#!/usr/bin/env python3
"""
File synopsis:
    Experimental Pi-side policy owner for the PS2VNC dedicated-link mux.

The daemon accepts one PS2 transport connection, forwards the logical RFB
channel to an isolated local X0tigervnc provider, captures 48-kHz signed
16-bit stereo PCM from the PipeWire default-sink monitor, and schedules both
payload streams over the one PS2-facing transport connection.

The PS2 supplies mechanisms and truthful receiver credits.

This daemon owns experimental policy:
    - quanta;
    - channel weights;
    - policy windows;
    - batching;
    - host-side buffering;
    - telemetry cadence.

Changing those values requires no new PS2 ELF.

Context:
    experiments/audio-transport/README.md
"""

from __future__ import annotations

import argparse
import array
import json
import re
import socket
import struct
import subprocess
import sys
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, Dict, Optional


MAGIC = 0x50535456
VERSION = 1
HEADER_SIZE = 16
MAX_PAYLOAD = 8192

FRAME_HELLO = 1
FRAME_CONFIG = 2
FRAME_DATA = 3
FRAME_CREDIT = 4
FRAME_TELEMETRY = 5
FRAME_HEARTBEAT = 6
FRAME_ERROR = 7

CHANNEL_CONTROL = 0
CHANNEL_RFB = 1
CHANNEL_AUDIO = 2
CHANNEL_TELEMETRY = 3
CHANNEL_MPEG2 = 4

HELLO_PAYLOAD_SIZE = 24
CREDIT_PAYLOAD_SIZE = 4
TELEMETRY_PAYLOAD_SIZE = 96

AUDIO_FRAME_BYTES = 4

HEADER_STRUCT = struct.Struct(">IBBBBII")


class ProtocolError(RuntimeError):
    pass


def read_exact(stream: socket.socket, count: int) -> bytes:
    chunks = bytearray()

    while len(chunks) < count:
        chunk = stream.recv(count - len(chunks))

        if not chunk:
            raise EOFError("peer closed transport")

        chunks.extend(chunk)

    return bytes(chunks)


@dataclass(frozen=True)
class Frame:
    kind: int
    channel: int
    flags: int
    sequence: int
    payload: bytes


def encode_frame(frame: Frame) -> bytes:
    if len(frame.payload) > MAX_PAYLOAD:
        raise ProtocolError("payload exceeds transport maximum")

    header = HEADER_STRUCT.pack(
        MAGIC,
        VERSION,
        frame.kind,
        frame.channel,
        frame.flags,
        frame.sequence,
        len(frame.payload),
    )

    return header + frame.payload


def receive_frame(stream: socket.socket) -> Frame:
    raw_header = read_exact(stream, HEADER_SIZE)

    (
        magic,
        version,
        kind,
        channel,
        flags,
        sequence,
        payload_length,
    ) = HEADER_STRUCT.unpack(raw_header)

    if magic != MAGIC:
        raise ProtocolError("bad transport magic")

    if version != VERSION:
        raise ProtocolError(
            f"unsupported transport version {version}"
        )

    if payload_length > MAX_PAYLOAD:
        raise ProtocolError("transport payload too large")

    payload = (
        read_exact(stream, payload_length)
        if payload_length
        else b""
    )

    return Frame(
        kind=kind,
        channel=channel,
        flags=flags,
        sequence=sequence,
        payload=payload,
    )


@dataclass
class ChannelState:
    name: str
    channel_id: int
    quantum: int
    weight: int
    policy_window: int
    host_buffer_limit: int
    alignment: int = 1

    data: bytearray = field(default_factory=bytearray)

    initial_credit_seen: bool = False
    physical_capacity: int = 0
    available_credit: int = 0
    outstanding: int = 0

    bytes_received_credit: int = 0
    bytes_sent: int = 0
    high_water: int = 0

    def add_credit(self, amount: int) -> None:
        if amount <= 0:
            raise ProtocolError(
                f"{self.name}: invalid credit {amount}"
            )

        if not self.initial_credit_seen:
            self.initial_credit_seen = True
            self.physical_capacity = amount
        else:
            self.outstanding = max(
                0,
                self.outstanding - amount,
            )

        self.available_credit += amount
        self.bytes_received_credit += amount

        if (
            self.physical_capacity
            and self.available_credit
            > self.physical_capacity
        ):
            raise ProtocolError(
                f"{self.name}: credit exceeds physical capacity"
            )

    def append(self, payload: bytes) -> None:
        if self.alignment > 1:
            if len(payload) % self.alignment:
                raise ProtocolError(
                    f"{self.name}: unaligned producer payload"
                )

        self.data.extend(payload)

        if len(self.data) > self.high_water:
            self.high_water = len(self.data)

    def send_budget(self) -> int:
        policy_room = max(
            0,
            self.policy_window - self.outstanding,
        )

        budget = min(
            len(self.data),
            self.available_credit,
            self.quantum,
            policy_room,
            MAX_PAYLOAD,
        )

        if self.alignment > 1:
            budget -= budget % self.alignment

        return budget

    def take_for_send(self, count: int) -> bytes:
        payload = bytes(self.data[:count])
        del self.data[:count]

        self.available_credit -= count
        self.outstanding += count
        self.bytes_sent += count

        return payload


class SmoothWeightedChooser:
    def __init__(
        self,
        channels: Dict[int, ChannelState],
    ) -> None:
        self.channels = channels
        self.current = {
            channel_id: 0
            for channel_id in channels
        }

    def choose(self) -> ChannelState:
        total = 0
        best_id: Optional[int] = None

        for channel_id, channel in self.channels.items():
            weight = max(1, channel.weight)
            total += weight
            self.current[channel_id] += weight

            if (
                best_id is None
                or self.current[channel_id]
                > self.current[best_id]
            ):
                best_id = channel_id

        assert best_id is not None

        self.current[best_id] -= total
        return self.channels[best_id]


class VolumeMirror:
    def __init__(self, enabled: bool) -> None:
        self.enabled = enabled
        self.gain = 1.0
        self.next_refresh = 0.0

    def refresh_if_due(self) -> None:
        if not self.enabled:
            self.gain = 1.0
            return

        now = time.monotonic()

        if now < self.next_refresh:
            return

        self.next_refresh = now + 0.25

        result = subprocess.run(
            [
                "wpctl",
                "get-volume",
                "@DEFAULT_AUDIO_SINK@",
            ],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        match = re.search(
            r"Volume:\s*([0-9.]+)",
            result.stdout,
        )

        if match is None:
            raise RuntimeError(
                "unable to parse wpctl sink volume"
            )

        gain = float(match.group(1))

        if "[MUTED]" in result.stdout:
            gain = 0.0

        self.gain = gain

    def apply(self, payload: bytes) -> bytes:
        self.refresh_if_due()

        if self.gain == 1.0:
            return payload

        if self.gain == 0.0:
            return bytes(len(payload))

        samples = array.array("h")
        samples.frombytes(payload)

        if sys.byteorder != "little":
            samples.byteswap()

        for index, sample in enumerate(samples):
            scaled = int(sample * self.gain)

            if scaled > 32767:
                scaled = 32767
            elif scaled < -32768:
                scaled = -32768

            samples[index] = scaled

        if sys.byteorder != "little":
            samples.byteswap()

        return samples.tobytes()


def discover_default_sink_monitor() -> str:
    result = subprocess.run(
        [
            "wpctl",
            "inspect",
            "@DEFAULT_AUDIO_SINK@",
        ],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    for raw_line in result.stdout.splitlines():
        line = raw_line.strip()

        match = re.search(
            r'node\.name\s*=\s*"([^"]+)"',
            line,
        )

        if match is not None:
            return match.group(1) + ".monitor"

    raise RuntimeError(
        "default sink node.name not found"
    )


def start_audio_capture(
    rate: int,
    channels: int,
) -> subprocess.Popen[bytes]:
    target = discover_default_sink_monitor()

    command = [
        "pw-record",
        "--target",
        target,
        "--rate",
        str(rate),
        "--format",
        "s16",
        "--channels",
        str(channels),
        "-",
    ]

    print(
        "AUDIO_CAPTURE_TARGET="
        + target,
        flush=True,
    )

    return subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        bufsize=0,
    )


def parse_telemetry(payload: bytes) -> dict:
    if len(payload) != TELEMETRY_PAYLOAD_SIZE:
        raise ProtocolError(
            "invalid telemetry payload length"
        )

    values = struct.unpack(
        ">" + ("I" * 24),
        payload,
    )

    names = [
        "version",
        "runtime_error",
        "frames_received",
        "payload_bytes_received",
        "receiver_loop_count",
        "last_received_sequence",
        "last_sent_sequence",
        "rfb_bytes_enqueued",
        "rfb_bytes_consumed",
        "rfb_bytes_sent",
        "rfb_queue_current",
        "rfb_queue_high_water",
        "rfb_credit_bytes_sent",
        "rfb_poll_calls",
        "rfb_read_calls",
        "audio_bytes_enqueued",
        "audio_bytes_consumed",
        "audio_queue_current",
        "audio_queue_high_water",
        "audio_credit_bytes_sent",
        "audio_read_calls",
        "audio_chunks_played",
        "audio_bytes_played",
        "reserved",
    ]

    return dict(zip(names, values))


class MuxSession:
    def __init__(
        self,
        ps2_socket: socket.socket,
        config: dict,
    ) -> None:
        self.ps2 = ps2_socket
        self.config = config

        self.stop_event = threading.Event()
        self.condition = threading.Condition()

        self.send_lock = threading.Lock()

        self.tx_sequence = 1
        self.expected_rx_sequence = 1

        self.ps2_hello: Optional[dict] = None
        self.last_telemetry: Optional[dict] = None

        self.channels = {
            CHANNEL_RFB: ChannelState(
                name="rfb",
                channel_id=CHANNEL_RFB,
                quantum=int(
                    config["rfb_quantum"]
                ),
                weight=int(
                    config["rfb_weight"]
                ),
                policy_window=int(
                    config["rfb_policy_window"]
                ),
                host_buffer_limit=int(
                    config["rfb_host_buffer"]
                ),
            ),
            CHANNEL_AUDIO: ChannelState(
                name="audio",
                channel_id=CHANNEL_AUDIO,
                quantum=int(
                    config["audio_quantum"]
                ),
                weight=int(
                    config["audio_weight"]
                ),
                policy_window=int(
                    config["audio_policy_window"]
                ),
                host_buffer_limit=int(
                    config["audio_host_buffer"]
                ),
                alignment=AUDIO_FRAME_BYTES,
            ),
        }

        self.chooser = SmoothWeightedChooser(
            self.channels
        )

        self.rfb: Optional[socket.socket] = None
        self.audio_process: Optional[
            subprocess.Popen[bytes]
        ] = None

        self.threads: list[threading.Thread] = []

    def send_frame(
        self,
        kind: int,
        channel: int,
        payload: bytes = b"",
        flags: int = 0,
    ) -> None:
        with self.send_lock:
            frame = Frame(
                kind=kind,
                channel=channel,
                flags=flags,
                sequence=self.tx_sequence,
                payload=payload,
            )

            self.ps2.sendall(
                encode_frame(frame)
            )

            self.tx_sequence += 1

    def enqueue(
        self,
        channel_id: int,
        payload: bytes,
    ) -> None:
        channel = self.channels[channel_id]

        with self.condition:
            while (
                len(channel.data) + len(payload)
                > channel.host_buffer_limit
            ):
                if self.stop_event.is_set():
                    return

                self.condition.wait(0.1)

            channel.append(payload)
            self.condition.notify_all()

    def ps2_reader(self) -> None:
        assert self.rfb is not None

        while not self.stop_event.is_set():
            frame = receive_frame(self.ps2)

            if frame.sequence != self.expected_rx_sequence:
                raise ProtocolError(
                    "PS2 sequence mismatch: "
                    f"expected={self.expected_rx_sequence} "
                    f"actual={frame.sequence}"
                )

            self.expected_rx_sequence += 1

            if frame.kind == FRAME_HELLO:
                if (
                    frame.channel != CHANNEL_CONTROL
                    or len(frame.payload)
                    != HELLO_PAYLOAD_SIZE
                ):
                    raise ProtocolError(
                        "invalid PS2 HELLO"
                    )

                (
                    capabilities,
                    max_payload,
                    rfb_capacity,
                    audio_capacity,
                    audio_rate,
                    audio_frame_bytes,
                ) = struct.unpack(
                    ">IIIIII",
                    frame.payload,
                )

                self.ps2_hello = {
                    "capabilities": capabilities,
                    "max_payload": max_payload,
                    "rfb_capacity": rfb_capacity,
                    "audio_capacity": audio_capacity,
                    "audio_rate": audio_rate,
                    "audio_frame_bytes":
                        audio_frame_bytes,
                }

                print(
                    "PS2_HELLO="
                    + json.dumps(
                        self.ps2_hello,
                        sort_keys=True,
                    ),
                    flush=True,
                )

            elif frame.kind == FRAME_CREDIT:
                if (
                    frame.channel
                    not in self.channels
                    or len(frame.payload)
                    != CREDIT_PAYLOAD_SIZE
                ):
                    raise ProtocolError(
                        "invalid CREDIT frame"
                    )

                amount = struct.unpack(
                    ">I",
                    frame.payload,
                )[0]

                with self.condition:
                    self.channels[
                        frame.channel
                    ].add_credit(amount)

                    self.condition.notify_all()

            elif frame.kind == FRAME_DATA:
                if frame.channel != CHANNEL_RFB:
                    raise ProtocolError(
                        "PS2 DATA on unsupported channel"
                    )

                self.rfb.sendall(frame.payload)

            elif frame.kind == FRAME_TELEMETRY:
                if (
                    frame.channel
                    != CHANNEL_TELEMETRY
                ):
                    raise ProtocolError(
                        "telemetry on wrong channel"
                    )

                telemetry = parse_telemetry(
                    frame.payload
                )

                self.last_telemetry = telemetry

                print(
                    "PS2_TELEMETRY="
                    + json.dumps(
                        telemetry,
                        sort_keys=True,
                    ),
                    flush=True,
                )

            elif frame.kind == FRAME_ERROR:
                raise ProtocolError(
                    "PS2 reported transport error"
                )

            else:
                raise ProtocolError(
                    f"unexpected PS2 frame kind "
                    f"{frame.kind}"
                )

    def rfb_reader(self) -> None:
        assert self.rfb is not None

        while not self.stop_event.is_set():
            payload = self.rfb.recv(8192)

            if not payload:
                raise EOFError(
                    "isolated RFB provider closed"
                )

            self.enqueue(
                CHANNEL_RFB,
                payload,
            )

    def audio_reader(self) -> None:
        assert self.audio_process is not None
        assert self.audio_process.stdout is not None

        mirror = VolumeMirror(
            bool(
                self.config[
                    "mirror_sink_volume"
                ]
            )
        )

        while not self.stop_event.is_set():
            payload = self.audio_process.stdout.read(
                int(
                    self.config[
                        "audio_quantum"
                    ]
                )
            )

            if not payload:
                raise EOFError(
                    "pw-record stopped"
                )

            remainder = (
                len(payload)
                % AUDIO_FRAME_BYTES
            )

            if remainder:
                payload = payload[:-remainder]

            if not payload:
                continue

            payload = mirror.apply(payload)

            self.enqueue(
                CHANNEL_AUDIO,
                payload,
            )

    def guarded_thread(
        self,
        name: str,
        target,
    ) -> threading.Thread:
        def runner() -> None:
            try:
                target()
            except Exception as exc:
                if not self.stop_event.is_set():
                    print(
                        f"THREAD_ERROR name={name} "
                        f"error={exc!r}",
                        flush=True,
                    )

                self.stop_event.set()

                with self.condition:
                    self.condition.notify_all()

        thread = threading.Thread(
            name=name,
            target=runner,
            daemon=True,
        )

        return thread

    def scheduler(self) -> None:
        telemetry_interval = float(
            self.config[
                "telemetry_interval_seconds"
            ]
        )

        next_telemetry = (
            time.monotonic()
            + telemetry_interval
        )

        self.send_frame(
            FRAME_HELLO,
            CHANNEL_CONTROL,
        )

        while not self.stop_event.is_set():
            now = time.monotonic()

            if (
                telemetry_interval > 0
                and now >= next_telemetry
            ):
                self.send_frame(
                    FRAME_HEARTBEAT,
                    CHANNEL_CONTROL,
                )

                next_telemetry = (
                    now + telemetry_interval
                )

            sent_something = False

            for _ in range(
                max(
                    1,
                    sum(
                        max(1, channel.weight)
                        for channel
                        in self.channels.values()
                    ),
                )
            ):
                channel = self.chooser.choose()

                with self.condition:
                    budget = channel.send_budget()

                    if budget == 0:
                        continue

                    payload = channel.take_for_send(
                        budget
                    )

                    self.condition.notify_all()

                self.send_frame(
                    FRAME_DATA,
                    channel.channel_id,
                    payload,
                )

                sent_something = True
                break

            if sent_something:
                continue

            with self.condition:
                timeout = 0.05

                if telemetry_interval > 0:
                    timeout = min(
                        timeout,
                        max(
                            0.001,
                            next_telemetry
                            - time.monotonic(),
                        ),
                    )

                self.condition.wait(timeout)

    def run(self) -> None:
        self.rfb = socket.create_connection(
            (
                str(self.config["rfb_host"]),
                int(self.config["rfb_port"]),
            ),
            timeout=5.0,
        )

        self.rfb.settimeout(None)

        self.audio_process = start_audio_capture(
            int(self.config["audio_rate"]),
            int(self.config["audio_channels"]),
        )

        self.threads = [
            self.guarded_thread(
                "ps2-reader",
                self.ps2_reader,
            ),
            self.guarded_thread(
                "rfb-reader",
                self.rfb_reader,
            ),
            self.guarded_thread(
                "audio-reader",
                self.audio_reader,
            ),
        ]

        for thread in self.threads:
            thread.start()

        try:
            self.scheduler()
        finally:
            self.stop_event.set()

            with self.condition:
                self.condition.notify_all()

            try:
                self.ps2.shutdown(
                    socket.SHUT_RDWR
                )
            except OSError:
                pass

            try:
                self.rfb.shutdown(
                    socket.SHUT_RDWR
                )
            except OSError:
                pass

            self.ps2.close()
            self.rfb.close()

            if self.audio_process is not None:
                self.audio_process.terminate()

                try:
                    self.audio_process.wait(
                        timeout=2.0
                    )
                except subprocess.TimeoutExpired:
                    self.audio_process.kill()
                    self.audio_process.wait()

            for thread in self.threads:
                thread.join(timeout=1.0)


def load_config(path: Path) -> dict:
    return json.loads(
        path.read_text(
            encoding="utf-8"
        )
    )


def validate_config(config: dict) -> None:
    required_positive = [
        "listen_port",
        "rfb_port",
        "rfb_quantum",
        "audio_quantum",
        "rfb_weight",
        "audio_weight",
        "rfb_policy_window",
        "audio_policy_window",
        "rfb_host_buffer",
        "audio_host_buffer",
        "audio_rate",
        "audio_channels",
        "audio_sample_bytes",
    ]

    for name in required_positive:
        if int(config[name]) <= 0:
            raise ValueError(
                f"{name} must be positive"
            )

    if int(config["rfb_quantum"]) > MAX_PAYLOAD:
        raise ValueError(
            "rfb_quantum exceeds protocol maximum"
        )

    if int(config["audio_quantum"]) > MAX_PAYLOAD:
        raise ValueError(
            "audio_quantum exceeds protocol maximum"
        )

    if (
        int(config["audio_quantum"])
        % AUDIO_FRAME_BYTES
    ):
        raise ValueError(
            "audio_quantum must preserve "
            "S16-stereo frame alignment"
        )


def serve(config: dict) -> None:
    validate_config(config)

    listener = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM,
    )

    listener.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1,
    )

    listener.bind(
        (
            str(config["listen_host"]),
            int(config["listen_port"]),
        )
    )

    listener.listen(1)

    print(
        "MUX_LISTEN="
        f"{config['listen_host']}:"
        f"{config['listen_port']}",
        flush=True,
    )

    while True:
        connection, peer = listener.accept()

        print(
            "PS2_CONNECTED="
            f"{peer[0]}:{peer[1]}",
            flush=True,
        )

        try:
            MuxSession(
                connection,
                config,
            ).run()

        except Exception as exc:
            print(
                "SESSION_ERROR="
                + repr(exc),
                flush=True,
            )

            try:
                connection.close()
            except OSError:
                pass

        print(
            "PS2_SESSION_ENDED=YES",
            flush=True,
        )


def main() -> int:
    parser = argparse.ArgumentParser()

    default_config = (
        Path(__file__).with_name(
            "default-config.json"
        )
    )

    parser.add_argument(
        "--config",
        type=Path,
        default=default_config,
    )

    parser.add_argument(
        "--check-config",
        action="store_true",
    )

    args = parser.parse_args()

    config = load_config(args.config)
    validate_config(config)

    if args.check_config:
        print("CONFIG_CHECK=PASS")
        return 0

    serve(config)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
