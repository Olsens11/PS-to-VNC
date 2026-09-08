#!/usr/bin/env python3
"""
File synopsis:
    Pi-side producer for the resident H1 one-socket AUDIO + MPEG2 media harness.

The server owns one physical PSTV TCP connection. It performs CONFIG v2,
receives independent PS2 AUDIO/MPEG credits, captures the qualified P12 MPEG-2
video profile, optionally captures 48 kHz signed-16 stereo PCM, and schedules
both logical channels over that one socket.

The first hardware qualification should use P11_COMPAT_VIDEO_ONLY. The next
profile, P11_COMPAT_PLUS_PCM, changes only the addition of PCM audio on the same
mux. No separate PS2-facing audio socket exists in H1.
"""

from __future__ import annotations

import argparse
import json
import os
import signal
import socket
import struct
import subprocess
import sys
import threading
import time
import zlib
from pathlib import Path

THIS_DIR = Path(__file__).resolve().parent
AUDIO_PI_DIR = THIS_DIR.parent / "audio-transport" / "pi"
if str(AUDIO_PI_DIR) not in sys.path:
    sys.path.insert(0, str(AUDIO_PI_DIR))

import mux_daemon as audio_base  # type: ignore

from h1_profiles import (
    AUDIO_PCM,
    PROFILES,
    build_config_payload,
    profile_names,
    resolve_profile,
)

MAGIC = b"PSTV"
VERSION = 1
HEADER = struct.Struct(">4sBBBBII")
HEADER_BYTES = 16
MAX_PAYLOAD = 8192
HELLO_BYTES = 24
CREDIT_BYTES = 4

FRAME_HELLO = 1
FRAME_CONFIG = 2
FRAME_DATA = 3
FRAME_CREDIT = 4
FRAME_TELEMETRY = 5
FRAME_HEARTBEAT = 6
FRAME_ERROR = 7
FRAME_MEDIA_END = 8
FRAME_SESSION_RESULT = 9

CHANNEL_CONTROL = 0
CHANNEL_AUDIO = 2
CHANNEL_TELEMETRY = 3
CHANNEL_MPEG2 = 4

CONFIG_ACK_FLAG = 0x01

CAP_CONFIG_DYNAMIC = 1 << 4
CAP_MEDIA_MUX_DYNAMIC = 1 << 5
CAP_MPEG2_ES = 1 << 6
CAP_REPEAT_SESSIONS = 1 << 7
REQUIRED_CAPS = (
    CAP_CONFIG_DYNAMIC
    | CAP_MEDIA_MUX_DYNAMIC
    | CAP_MPEG2_ES
    | CAP_REPEAT_SESSIONS
)

MEDIA_END_VERSION = 1
MEDIA_END_BYTES = 64
RESULT_BYTES = 128
TELEMETRY_BYTES = 160

MPEG_SEQUENCE_END = b"\x00\x00\x01\xb7"
TERMINAL_SUFFIX = MPEG_SEQUENCE_END + b"\x00" * 12

VIDEO_SOURCE_SIZE = "704x462"
VIDEO_RATE = "30000/1001"
VIDEO_FILTER = "scale=608:416:flags=bicubic,format=yuv420p"
VIDEO_BITRATE = "4000000"
VIDEO_MAXRATE = "5000000"
VIDEO_VBV_BITS = "1835008"
VIDEO_GOP = "15"

STOP_REASON_FINITE_DURATION = 1


class ProtocolError(RuntimeError):
    pass


class Frame:
    def __init__(
        self,
        kind: int,
        channel: int,
        flags: int,
        sequence: int,
        payload: bytes,
    ) -> None:
        self.kind = kind
        self.channel = channel
        self.flags = flags
        self.sequence = sequence
        self.payload = payload


def read_exact(sock: socket.socket, count: int) -> bytes:
    data = bytearray()
    while len(data) < count:
        chunk = sock.recv(count - len(data))
        if not chunk:
            raise EOFError("socket closed")
        data.extend(chunk)
    return bytes(data)


def receive_frame(sock: socket.socket) -> Frame:
    raw = read_exact(sock, HEADER_BYTES)
    magic, version, kind, channel, flags, sequence, payload_length = HEADER.unpack(raw)

    if magic != MAGIC or version != VERSION:
        raise ProtocolError("invalid PSTV header identity")
    if payload_length > MAX_PAYLOAD:
        raise ProtocolError(f"payload length {payload_length} exceeds {MAX_PAYLOAD}")

    payload = read_exact(sock, payload_length) if payload_length else b""
    return Frame(kind, channel, flags, sequence, payload)


def encode_header(
    kind: int,
    channel: int,
    flags: int,
    sequence: int,
    payload_length: int,
) -> bytes:
    return HEADER.pack(
        MAGIC,
        VERSION,
        kind,
        channel,
        flags,
        sequence,
        payload_length,
    )


class ProducerBuffer:
    """Bounded process-stdout spool with deliberate upstream backpressure."""

    def __init__(
        self,
        name: str,
        process: subprocess.Popen[bytes],
        read_size: int,
        archive_path: Path,
        terminal_suffix: bytes = b"",
        maximum_buffer: int = 1024 * 1024,
    ) -> None:
        if process.stdout is None:
            raise RuntimeError(f"{name} process has no stdout pipe")

        self.name = name
        self.process = process
        self.stdout = process.stdout
        self.read_size = read_size
        self.archive_path = archive_path
        self.terminal_suffix = terminal_suffix
        self.maximum_buffer = maximum_buffer

        self.condition = threading.Condition()
        self.buffer = bytearray()
        self.eof = False
        self.error: BaseException | None = None
        self.bytes_produced = 0
        self.started_monotonic = time.monotonic()
        self.ended_monotonic = 0.0

        self.thread = threading.Thread(
            target=self._reader,
            name=f"h1-{name}-producer",
            daemon=True,
        )
        self.thread.start()

    def _append(self, data: bytes, archive) -> None:
        if not data:
            return

        archive.write(data)
        archive.flush()

        offset = 0
        while offset < len(data):
            with self.condition:
                while len(self.buffer) >= self.maximum_buffer:
                    self.condition.wait(timeout=0.05)

                room = self.maximum_buffer - len(self.buffer)
                part = data[offset : offset + room]
                self.buffer.extend(part)
                self.bytes_produced += len(part)
                offset += len(part)
                self.condition.notify_all()

    def _reader(self) -> None:
        try:
            with self.archive_path.open("wb") as archive:
                while True:
                    chunk = self.stdout.read(self.read_size)
                    if not chunk:
                        break
                    self._append(chunk, archive)

                if self.terminal_suffix:
                    self._append(self.terminal_suffix, archive)

        except BaseException as exc:
            self.error = exc
        finally:
            self.ended_monotonic = time.monotonic()
            with self.condition:
                self.eof = True
                self.condition.notify_all()

    def available(self) -> int:
        with self.condition:
            return len(self.buffer)

    def take(self, maximum: int, alignment: int = 1) -> bytes:
        with self.condition:
            count = min(maximum, len(self.buffer))
            count -= count % alignment
            if count == 0:
                return b""
            result = bytes(self.buffer[:count])
            del self.buffer[:count]
            self.condition.notify_all()
            return result

    def done_and_empty(self) -> bool:
        with self.condition:
            return self.eof and not self.buffer

    def check(self) -> None:
        if self.error is not None:
            raise RuntimeError(f"{self.name} producer failed") from self.error

    def stop(self) -> None:
        if self.process.poll() is not None:
            return
        try:
            os.killpg(self.process.pid, signal.SIGINT)
        except ProcessLookupError:
            pass

    def terminate(self) -> None:
        if self.process.poll() is None:
            try:
                os.killpg(self.process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass


class ChannelState:
    def __init__(self, channel: int, name: str) -> None:
        self.channel = channel
        self.name = name
        self.credit = 0
        self.frames_sent = 0
        self.bytes_sent = 0
        self.last_data_sequence = 0
        self.crc32 = 0


class H1Session:
    def __init__(
        self,
        sock: socket.socket,
        profile: dict[str, int],
        evidence: Path,
        duration: float,
        display: str,
    ) -> None:
        self.sock = sock
        self.profile = profile
        self.evidence = evidence
        self.duration = duration
        self.display = display

        self.send_lock = threading.Lock()
        self.condition = threading.Condition()
        self.next_tx_sequence = 1
        self.expected_rx_sequence = 1

        self.audio = ChannelState(CHANNEL_AUDIO, "audio")
        self.mpeg = ChannelState(CHANNEL_MPEG2, "mpeg")

        self.hello_event = threading.Event()
        self.config_ack_event = threading.Event()
        self.result_event = threading.Event()
        self.stop_event = threading.Event()

        self.reader_error: BaseException | None = None
        self.hello: dict[str, int] | None = None
        self.result: dict[str, int] | None = None
        self.last_telemetry: dict[str, int] | None = None

        self.config_payload = build_config_payload(profile)
        self.video_producer: ProducerBuffer | None = None
        self.audio_producer: ProducerBuffer | None = None
        self.audio_stop_requested = False

    def send_frame(
        self,
        kind: int,
        channel: int,
        payload: bytes = b"",
        flags: int = 0,
    ) -> int:
        if len(payload) > MAX_PAYLOAD:
            raise ProtocolError("attempted oversized PSTV payload")

        with self.send_lock:
            sequence = self.next_tx_sequence
            self.next_tx_sequence += 1
            self.sock.sendall(
                encode_header(kind, channel, flags, sequence, len(payload))
            )
            if payload:
                self.sock.sendall(payload)
            return sequence

    def _parse_hello(self, payload: bytes) -> dict[str, int]:
        if len(payload) != HELLO_BYTES:
            raise ProtocolError(f"HELLO length {len(payload)} != {HELLO_BYTES}")
        words = struct.unpack(">6I", payload)
        return {
            "capabilities": words[0],
            "max_payload": words[1],
            "reserved_0": words[2],
            "reserved_1": words[3],
            "reserved_2": words[4],
            "reserved_3": words[5],
        }

    def _parse_result(self, payload: bytes) -> dict[str, int]:
        if len(payload) != RESULT_BYTES:
            raise ProtocolError(f"RESULT length {len(payload)} != {RESULT_BYTES}")
        w = struct.unpack(">32I", payload)
        names = (
            "version",
            "session_id",
            "transport_error",
            "integrity_pass",
            "pictures_decoded",
            "pictures_displayed",
            "feed_calls",
            "payload_bytes_submitted",
            "dma_bytes_submitted",
            "deadline_misses",
            "max_deadline_late_ticks_lo",
            "producer_picture_starts",
            "audio_bytes_enqueued",
            "audio_bytes_consumed",
            "audio_bytes_played",
            "mpeg_bytes_enqueued",
            "mpeg_bytes_consumed",
            "mpeg_wait_events",
            "mpeg_wait_loops",
            "mpeg_wait_max_loops",
            "audio_credit_bytes_sent",
            "mpeg_credit_bytes_sent",
            "producer_audio_crc32",
            "ps2_audio_crc32",
            "producer_mpeg_crc32",
            "ps2_mpeg_crc32",
            "config_digest",
            "profile_id",
            "audio_mode",
            "video_mode",
            "producer_stop_reason",
            "diagnostic_word",
        )
        return dict(zip(names, w, strict=True))

    def _parse_telemetry(self, payload: bytes) -> dict[str, int]:
        if len(payload) != TELEMETRY_BYTES:
            raise ProtocolError(
                f"TELEMETRY length {len(payload)} != {TELEMETRY_BYTES}"
            )
        words = struct.unpack(">40I", payload)
        return {
            "version": words[0],
            "error": words[1],
            "session_id": words[2],
            "diagnostic_word": words[3],
            "audio_queue_current": words[8],
            "audio_queue_high_water": words[9],
            "audio_bytes_enqueued": words[10],
            "audio_bytes_consumed": words[11],
            "mpeg_queue_current": words[16],
            "mpeg_queue_high_water": words[17],
            "mpeg_bytes_enqueued": words[18],
            "mpeg_bytes_consumed": words[19],
            "mpeg_wait_events": words[21],
            "mpeg_wait_loops": words[22],
            "mpeg_wait_max_loops": words[23],
            "end_received": words[25],
            "receiver_done": words[26],
            "producer_audio_bytes": words[27],
            "producer_mpeg_bytes": words[28],
            "producer_picture_starts": words[29],
            "config_digest": words[30],
            "profile_id": words[31],
        }

    def reader(self) -> None:
        try:
            while not self.stop_event.is_set():
                frame = receive_frame(self.sock)

                if frame.sequence != self.expected_rx_sequence:
                    raise ProtocolError(
                        "PS2 sequence mismatch "
                        f"expected={self.expected_rx_sequence} "
                        f"actual={frame.sequence}"
                    )
                self.expected_rx_sequence += 1

                if frame.kind == FRAME_HELLO:
                    if frame.channel != CHANNEL_CONTROL or frame.flags != 0:
                        raise ProtocolError("invalid HELLO channel/flags")
                    self.hello = self._parse_hello(frame.payload)
                    if (self.hello["capabilities"] & REQUIRED_CAPS) != REQUIRED_CAPS:
                        raise ProtocolError(
                            "wrong PS2 ELF: required H1 media capabilities absent"
                        )
                    if self.hello["max_payload"] != MAX_PAYLOAD:
                        raise ProtocolError("PS2 max payload authority mismatch")
                    print("H1_PS2_HELLO=" + json.dumps(self.hello, sort_keys=True), flush=True)
                    self.hello_event.set()

                elif frame.kind == FRAME_CONFIG:
                    if (
                        frame.channel != CHANNEL_CONTROL
                        or frame.flags != CONFIG_ACK_FLAG
                        or frame.payload != self.config_payload
                    ):
                        raise ProtocolError("invalid H1 CONFIG ACK")
                    print(
                        "H1_CONFIG_ACK=PASS "
                        f"session_id={self.profile['session_id']} "
                        f"profile_id={self.profile['profile_id']}",
                        flush=True,
                    )
                    self.config_ack_event.set()

                elif frame.kind == FRAME_CREDIT:
                    if len(frame.payload) != CREDIT_BYTES:
                        raise ProtocolError("invalid CREDIT payload")
                    amount = struct.unpack(">I", frame.payload)[0]
                    if frame.channel == CHANNEL_AUDIO:
                        state = self.audio
                    elif frame.channel == CHANNEL_MPEG2:
                        state = self.mpeg
                    else:
                        raise ProtocolError("CREDIT on unsupported H1 channel")
                    with self.condition:
                        state.credit += amount
                        self.condition.notify_all()

                elif frame.kind == FRAME_TELEMETRY:
                    if frame.channel != CHANNEL_TELEMETRY:
                        raise ProtocolError("telemetry on wrong channel")
                    self.last_telemetry = self._parse_telemetry(frame.payload)
                    print(
                        "H1_PS2_TELEMETRY="
                        + json.dumps(self.last_telemetry, sort_keys=True),
                        flush=True,
                    )

                elif frame.kind == FRAME_SESSION_RESULT:
                    if frame.channel != CHANNEL_TELEMETRY:
                        raise ProtocolError("session result on wrong channel")
                    self.result = self._parse_result(frame.payload)
                    print(
                        "H1_PS2_RESULT=" + json.dumps(self.result, sort_keys=True),
                        flush=True,
                    )
                    self.result_event.set()

                elif frame.kind == FRAME_ERROR:
                    code = (
                        struct.unpack(">I", frame.payload)[0]
                        if len(frame.payload) == 4
                        else -1
                    )
                    raise ProtocolError(f"PS2 reported H1 transport error={code}")

                else:
                    raise ProtocolError(
                        f"unexpected PS2 frame kind={frame.kind} channel={frame.channel}"
                    )

        except BaseException as exc:
            if not self.stop_event.is_set():
                self.reader_error = exc
                self.result_event.set()
                self.hello_event.set()
                self.config_ack_event.set()
                with self.condition:
                    self.condition.notify_all()

    def check_reader(self) -> None:
        if self.reader_error is not None:
            raise RuntimeError("H1 PS2 reader failed") from self.reader_error

    def video_command(self) -> list[str]:
        return [
            "ffmpeg",
            "-hide_banner",
            "-loglevel",
            "warning",
            "-f",
            "x11grab",
            "-framerate",
            VIDEO_RATE,
            "-video_size",
            VIDEO_SOURCE_SIZE,
            "-i",
            self.display,
            "-t",
            f"{self.duration:.6f}",
            "-vf",
            VIDEO_FILTER,
            "-an",
            "-c:v",
            "mpeg2video",
            "-profile:v",
            "main",
            "-level:v",
            "main",
            "-pix_fmt",
            "yuv420p",
            "-b:v",
            VIDEO_BITRATE,
            "-maxrate:v",
            VIDEO_MAXRATE,
            "-minrate:v",
            "0",
            "-bufsize:v",
            VIDEO_VBV_BITS,
            "-g",
            VIDEO_GOP,
            "-bf",
            "0",
            "-r",
            VIDEO_RATE,
            "-f",
            "mpeg2video",
            "pipe:1",
        ]

    def audio_command(self) -> list[str]:
        monitor = audio_base.discover_default_sink_monitor()
        print(f"H1_AUDIO_CAPTURE_TARGET={monitor}", flush=True)
        return [
            "pw-record",
            "--target",
            monitor,
            "--rate",
            str(self.profile["audio_rate"]),
            "--format",
            "s16",
            "--channels",
            str(self.profile["audio_channels"]),
            "-",
        ]

    def start_producers(self) -> None:
        video_command = self.video_command()
        print("H1_VIDEO_COMMAND=" + json.dumps(video_command), flush=True)

        video_process = subprocess.Popen(
            video_command,
            stdout=subprocess.PIPE,
            stderr=None,
            bufsize=0,
            start_new_session=True,
        )
        self.video_producer = ProducerBuffer(
            "video",
            video_process,
            8192,
            self.evidence / "video.m2v",
            terminal_suffix=TERMINAL_SUFFIX,
        )

        if self.profile["audio_mode"] == AUDIO_PCM:
            audio_command = self.audio_command()
            print("H1_AUDIO_COMMAND=" + json.dumps(audio_command), flush=True)
            audio_process = subprocess.Popen(
                audio_command,
                stdout=subprocess.PIPE,
                stderr=None,
                bufsize=0,
                start_new_session=True,
            )
            self.audio_producer = ProducerBuffer(
                "audio",
                audio_process,
                8192,
                self.evidence / "audio.pcm",
            )

    def _send_from(
        self,
        producer: ProducerBuffer,
        state: ChannelState,
        quantum: int,
        alignment: int,
    ) -> bool:
        with self.condition:
            credit = state.credit

        if credit < alignment:
            return False

        available = producer.available()
        if available < alignment:
            return False

        count = min(quantum, credit, available, MAX_PAYLOAD)
        count -= count % alignment
        if count == 0:
            return False

        payload = producer.take(count, alignment)
        if not payload:
            return False

        sequence = self.send_frame(FRAME_DATA, state.channel, payload)

        with self.condition:
            if state.credit < len(payload):
                raise ProtocolError("credit accounting underflow")
            state.credit -= len(payload)

        state.frames_sent += 1
        state.bytes_sent += len(payload)
        state.last_data_sequence = sequence
        state.crc32 = zlib.crc32(payload, state.crc32) & 0xFFFFFFFF
        return True

    def schedule_media(self) -> None:
        if self.video_producer is None:
            raise RuntimeError("video producer not started")

        # Byte-weighted round robin approximates the 4 Mb/s video : 1.536 Mb/s
        # PCM source-rate relationship without coupling PS2 queue sizes.
        schedule = (CHANNEL_AUDIO, CHANNEL_MPEG2, CHANNEL_MPEG2, CHANNEL_MPEG2)
        schedule_index = 0

        while True:
            self.check_reader()
            self.video_producer.check()
            if self.audio_producer is not None:
                self.audio_producer.check()

            if (
                self.audio_producer is not None
                and self.video_producer.eof
                and not self.audio_stop_requested
            ):
                self.audio_stop_requested = True
                self.audio_producer.stop()
                print("H1_AUDIO_STOP_REQUEST=VIDEO_EOF", flush=True)

            video_done = self.video_producer.done_and_empty()
            audio_done = (
                self.audio_producer is None
                or self.audio_producer.done_and_empty()
            )
            if video_done and audio_done:
                return

            sent = False
            for _ in range(len(schedule)):
                channel = schedule[schedule_index]
                schedule_index = (schedule_index + 1) % len(schedule)

                if channel == CHANNEL_AUDIO:
                    if self.audio_producer is None:
                        continue
                    sent = self._send_from(
                        self.audio_producer,
                        self.audio,
                        MAX_PAYLOAD,
                        4,
                    )
                else:
                    sent = self._send_from(
                        self.video_producer,
                        self.mpeg,
                        MAX_PAYLOAD,
                        1,
                    )

                if sent:
                    break

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)

    def send_media_end(self) -> dict[str, int]:
        video_path = self.evidence / "video.m2v"
        video_bytes = video_path.read_bytes()

        picture_starts = video_bytes.count(b"\x00\x00\x01\x00")
        sequence_headers = video_bytes.count(b"\x00\x00\x01\xb3")
        sequence_ends = video_bytes.count(MPEG_SEQUENCE_END)

        words = [
            MEDIA_END_VERSION,
            self.profile["session_id"],
            self.audio.bytes_sent,
            self.audio.frames_sent,
            self.audio.last_data_sequence,
            self.audio.crc32,
            self.mpeg.bytes_sent,
            self.mpeg.frames_sent,
            self.mpeg.last_data_sequence,
            self.mpeg.crc32,
            picture_starts,
            sequence_headers,
            sequence_ends,
            STOP_REASON_FINITE_DURATION,
            0,
            0,
        ]

        payload = struct.pack(">16I", *words)
        if len(payload) != MEDIA_END_BYTES:
            raise AssertionError("H1 MEDIA_END payload size mismatch")

        metadata = {
            "session_id": self.profile["session_id"],
            "audio_bytes": self.audio.bytes_sent,
            "audio_frames": self.audio.frames_sent,
            "audio_last_sequence": self.audio.last_data_sequence,
            "audio_crc32": self.audio.crc32,
            "mpeg_bytes": self.mpeg.bytes_sent,
            "mpeg_frames": self.mpeg.frames_sent,
            "mpeg_last_sequence": self.mpeg.last_data_sequence,
            "mpeg_crc32": self.mpeg.crc32,
            "picture_starts": picture_starts,
            "sequence_headers": sequence_headers,
            "sequence_ends": sequence_ends,
            "stop_reason": STOP_REASON_FINITE_DURATION,
        }

        self.send_frame(FRAME_MEDIA_END, CHANNEL_CONTROL, payload)
        print("H1_MEDIA_END_SENT=" + json.dumps(metadata, sort_keys=True), flush=True)
        return metadata

    def validate_result(self, metadata: dict[str, int]) -> None:
        self.check_reader()
        if self.result is None:
            raise ProtocolError("PS2 session result missing")

        result = self.result
        failures: list[str] = []

        checks = {
            "version": (result["version"], 1),
            "session_id": (result["session_id"], self.profile["session_id"]),
            "transport_error": (result["transport_error"], 0),
            "integrity_pass": (result["integrity_pass"], 1),
            "profile_id": (result["profile_id"], self.profile["profile_id"]),
            "audio_mode": (result["audio_mode"], self.profile["audio_mode"]),
            "video_mode": (result["video_mode"], self.profile["video_mode"]),
            "producer_picture_starts": (
                result["producer_picture_starts"],
                metadata["picture_starts"],
            ),
            "producer_mpeg_crc32": (
                result["producer_mpeg_crc32"],
                metadata["mpeg_crc32"],
            ),
            "ps2_mpeg_crc32": (
                result["ps2_mpeg_crc32"],
                metadata["mpeg_crc32"],
            ),
            "mpeg_bytes_enqueued": (
                result["mpeg_bytes_enqueued"],
                metadata["mpeg_bytes"],
            ),
            "mpeg_bytes_consumed": (
                result["mpeg_bytes_consumed"],
                metadata["mpeg_bytes"],
            ),
            "producer_stop_reason": (
                result["producer_stop_reason"],
                STOP_REASON_FINITE_DURATION,
            ),
        }

        if self.profile["audio_mode"] == AUDIO_PCM:
            checks.update(
                {
                    "producer_audio_crc32": (
                        result["producer_audio_crc32"],
                        metadata["audio_crc32"],
                    ),
                    "ps2_audio_crc32": (
                        result["ps2_audio_crc32"],
                        metadata["audio_crc32"],
                    ),
                    "audio_bytes_enqueued": (
                        result["audio_bytes_enqueued"],
                        metadata["audio_bytes"],
                    ),
                    "audio_bytes_consumed": (
                        result["audio_bytes_consumed"],
                        metadata["audio_bytes"],
                    ),
                }
            )

        for name, (actual, expected) in checks.items():
            if actual != expected:
                failures.append(f"{name}:actual={actual}:expected={expected}")

        summary = {
            "profile": self.profile,
            "metadata": metadata,
            "result": result,
            "validation_failures": failures,
        }
        (self.evidence / "summary.json").write_text(
            json.dumps(summary, indent=2, sort_keys=True) + "\n"
        )

        if failures:
            raise ProtocolError("H1 result validation failed: " + "; ".join(failures))

        print(
            "H1_SESSION_VALIDATION=PASS "
            f"profile_id={self.profile['profile_id']} "
            f"session_id={self.profile['session_id']} "
            f"decoded={result['pictures_decoded']} "
            f"displayed={result['pictures_displayed']} "
            f"deadline_misses={result['deadline_misses']} "
            f"mpeg_wait_events={result['mpeg_wait_events']} "
            f"audio_bytes_played={result['audio_bytes_played']}",
            flush=True,
        )

    def run(self) -> None:
        reader = threading.Thread(target=self.reader, name="h1-ps2-reader", daemon=True)
        reader.start()

        if not self.hello_event.wait(timeout=10.0):
            raise ProtocolError("timed out waiting for PS2 HELLO")
        self.check_reader()

        self.send_frame(FRAME_CONFIG, CHANNEL_CONTROL, self.config_payload)
        print(
            "H1_CONFIG_SENT=" + json.dumps(self.profile, sort_keys=True),
            flush=True,
        )

        if not self.config_ack_event.wait(timeout=10.0):
            raise ProtocolError("timed out waiting for exact CONFIG ACK")
        self.check_reader()

        self.start_producers()
        self.schedule_media()

        metadata = self.send_media_end()

        if not self.result_event.wait(timeout=60.0):
            raise ProtocolError("timed out waiting for H1 PS2 SESSION_RESULT")
        self.check_reader()
        self.validate_result(metadata)

        self.stop_event.set()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.sock.close()
        reader.join(timeout=1.0)

    def cleanup(self) -> None:
        self.stop_event.set()
        for producer in (self.audio_producer, self.video_producer):
            if producer is not None:
                producer.terminate()
        try:
            self.sock.close()
        except OSError:
            pass


def parse_override(text: str) -> tuple[str, int]:
    if "=" not in text:
        raise argparse.ArgumentTypeError("override must be NAME=INTEGER")
    name, raw = text.split("=", 1)
    try:
        value = int(raw, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    return name, value


def default_session_id() -> int:
    value = (time.time_ns() ^ (os.getpid() << 16)) & 0xFFFFFFFF
    return value if value != 0 else 1


def build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--profile",
        choices=profile_names(),
        default="P11_COMPAT_VIDEO_ONLY",
    )
    parser.add_argument("--session-id", type=lambda value: int(value, 0))
    parser.add_argument("--duration", type=float, default=24.0)
    parser.add_argument("--display", default=os.environ.get("DISPLAY", ":0.0"))
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument("--evidence")
    parser.add_argument("--override", action="append", default=[], type=parse_override)
    parser.add_argument("--validate-only", action="store_true")
    return parser


def main() -> int:
    args = build_argument_parser().parse_args()

    if args.duration <= 0:
        raise SystemExit("--duration must be positive")

    session_id = args.session_id if args.session_id is not None else default_session_id()
    overrides = dict(args.override)
    profile = resolve_profile(args.profile, session_id, overrides)
    config_payload = build_config_payload(profile)

    evidence = (
        Path(args.evidence)
        if args.evidence
        else Path.home()
        / "ps2vnc-evidence"
        / f"h1-{args.profile.lower()}-{time.strftime('%Y%m%dT%H%M%SZ', time.gmtime())}"
    )

    print(
        "H1_PI_PROFILE=" + json.dumps(profile, sort_keys=True),
        flush=True,
    )
    print(f"H1_CONFIG_PAYLOAD_BYTES={len(config_payload)}", flush=True)
    print(f"H1_EVIDENCE={evidence}", flush=True)

    if args.validate_only:
        command_session = object.__new__(H1Session)
        command_session.duration = args.duration
        command_session.display = args.display
        command_session.profile = profile
        video_command = H1Session.video_command(command_session)
        print("H1_VALIDATE_VIDEO_COMMAND=" + json.dumps(video_command), flush=True)
        print("H1_PI_MUX_SERVER_VALIDATE=PASS", flush=True)
        return 0

    evidence.mkdir(parents=True, exist_ok=False)
    (evidence / "profile.json").write_text(
        json.dumps(profile, indent=2, sort_keys=True) + "\n"
    )

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        f"H1_LISTENING={args.listen}:{args.port} "
        f"profile={args.profile} session_id={session_id}",
        flush=True,
    )

    connection, address = listener.accept()
    listener.close()
    connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    print(f"H1_PS2_CONNECTED={address[0]}:{address[1]}", flush=True)

    session = H1Session(
        connection,
        profile,
        evidence,
        args.duration,
        args.display,
    )

    try:
        session.run()
    finally:
        session.cleanup()

    print(
        "H1_PI_MUX_SESSION=PASS "
        f"profile={args.profile} session_id={session_id}",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
