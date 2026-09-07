#!/usr/bin/env python3
"""
File synopsis:
    EXP3 O3 finite live-X11 MPEG producer and credit-aware PSTV sender.

The server waits for the PS2, negotiates the real queue allocation, then starts
ffmpeg against X11 :0. MPEG-2 elementary bytes are archived and sent to PSTV
DATA/channel 4 as they are produced. No complete-stream prebuffer exists.

At ffmpeg EOF the Pi sends one terminal TELEMETRY frame containing the exact
byte count, DATA-frame count, CRC32, and MPEG start-code counts actually seen.
The PS2 does not treat a temporarily empty queue as EOF before that frame.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import shlex
import socket
import struct
import subprocess
import time
import zlib

MAGIC = 0x50535456
VERSION = 1

FRAME_CONFIG = 2
FRAME_DATA = 3
FRAME_CREDIT = 4
FRAME_TELEMETRY = 5

CHANNEL_CONTROL = 0
CHANNEL_TELEMETRY = 3
CHANNEL_MPEG2 = 4

CONFIG_ACK_FLAG = 0x01

HEADER = struct.Struct(">IBBBBII")
U32 = struct.Struct(">I")
CONFIG = struct.Struct(">IIII")
END = struct.Struct(">IIIIIIII")

MAX_PAYLOAD = 8192

RESULT_WORD_NAMES = (
    "version",
    "error",
    "queue_capacity",
    "queue_current",
    "queue_high_water",
    "frames_received",
    "bytes_received",
    "last_data_sequence",
    "crc32",
    "bytes_consumed",
    "feed_wait_events",
    "feed_wait_loops",
    "feed_wait_max_loops",
    "credit_frames_sent",
    "credit_bytes_sent",
    "pictures_decoded",
    "pictures_displayed",
    "feed_calls",
    "payload_bytes_submitted",
    "dma_bytes_submitted",
    "receiver_done",
    "integrity_pass",
    "end_received",
    "producer_bytes",
    "producer_frames",
    "producer_last_data_sequence",
    "producer_crc32",
    "producer_picture_starts",
    "producer_sequence_headers",
    "producer_sequence_ends",
)


def read_exact(connection: socket.socket, count: int) -> bytes:
    parts: list[bytes] = []
    remaining = count

    while remaining:
        chunk = connection.recv(remaining)
        if not chunk:
            raise RuntimeError(
                f"peer closed with {remaining} bytes still required"
            )
        parts.append(chunk)
        remaining -= len(chunk)

    return b"".join(parts)


def recv_frame(connection: socket.socket) -> tuple[int, int, int, int, bytes]:
    wire = read_exact(connection, HEADER.size)
    magic, version, kind, channel, flags, sequence, length = HEADER.unpack(wire)

    if magic != MAGIC:
        raise RuntimeError(f"bad magic 0x{magic:08x}")
    if version != VERSION:
        raise RuntimeError(f"bad PSTV version {version}")
    if length > MAX_PAYLOAD:
        raise RuntimeError(f"payload length exceeds PSTV maximum: {length}")

    payload = read_exact(connection, length) if length else b""
    return kind, channel, flags, sequence, payload


def send_frame(
    connection: socket.socket,
    *,
    kind: int,
    channel: int,
    flags: int,
    sequence: int,
    payload: bytes,
) -> None:
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload exceeds PSTV maximum")

    connection.sendall(
        HEADER.pack(
            MAGIC,
            VERSION,
            kind,
            channel,
            flags,
            sequence,
            len(payload),
        )
    )
    if payload:
        connection.sendall(payload)


def receive_credit_or_result(
    connection: socket.socket,
    *,
    expected_ps2_sequence: int,
) -> tuple[int, int, dict[str, int] | None]:
    kind, channel, flags, sequence, payload = recv_frame(connection)

    if sequence != expected_ps2_sequence:
        raise RuntimeError(
            "PS2 sequence mismatch "
            f"expected={expected_ps2_sequence} actual={sequence}"
        )

    expected_ps2_sequence += 1

    if (
        kind == FRAME_CREDIT
        and channel == CHANNEL_MPEG2
        and flags == 0
        and len(payload) == 4
    ):
        credit = U32.unpack(payload)[0]
        if credit == 0:
            raise RuntimeError("zero receiver credit is invalid")
        return expected_ps2_sequence, credit, None

    if (
        kind == FRAME_TELEMETRY
        and channel == CHANNEL_TELEMETRY
        and flags == 0
        and len(payload) == len(RESULT_WORD_NAMES) * 4
    ):
        words = struct.unpack(
            ">" + "I" * len(RESULT_WORD_NAMES),
            payload,
        )
        return (
            expected_ps2_sequence,
            0,
            dict(zip(RESULT_WORD_NAMES, words, strict=True)),
        )

    raise RuntimeError(
        "unexpected PS2 frame "
        f"kind={kind} channel={channel} flags={flags} "
        f"sequence={sequence} length={len(payload)}"
    )


class StartCodeCounter:
    def __init__(self) -> None:
        self.tail = b""
        self.picture_starts = 0
        self.sequence_headers = 0
        self.sequence_ends = 0

    def add(self, payload: bytes) -> None:
        combined = self.tail + payload
        self.picture_starts += combined.count(b"\x00\x00\x01\x00")
        self.sequence_headers += combined.count(b"\x00\x00\x01\xb3")
        self.sequence_ends += combined.count(b"\x00\x00\x01\xb7")
        self.tail = combined[-3:]


def read_full_payload(stream) -> bytes:
    parts: list[bytes] = []
    remaining = MAX_PAYLOAD

    while remaining:
        chunk = stream.read(remaining)
        if not chunk:
            break
        parts.append(chunk)
        remaining -= len(chunk)

    return b"".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5904)
    parser.add_argument("--queue-capacity", type=int, default=32768)
    parser.add_argument("--receiver-stack", type=int, default=16384)
    parser.add_argument("--receiver-priority", type=int, default=63)
    parser.add_argument("--display", default=":0.0")
    parser.add_argument("--source-size", default="704x462")
    parser.add_argument("--duration", type=float, default=20.0)
    parser.add_argument("--bitrate", default="4M")
    parser.add_argument("--gop", type=int, default=15)
    parser.add_argument("--archive", required=True)
    parser.add_argument("--ffmpeg-log", required=True)
    args = parser.parse_args()

    if args.queue_capacity < MAX_PAYLOAD:
        raise SystemExit("queue capacity must be at least one maximum DATA frame")
    if args.queue_capacity % MAX_PAYLOAD:
        raise SystemExit("queue capacity must be an exact multiple of 8192")
    if args.receiver_stack < 256 or args.receiver_stack % 16:
        raise SystemExit("receiver stack must be >=256 and a multiple of 16")
    if not 1 <= args.receiver_priority <= 127:
        raise SystemExit("receiver priority must be in the EE 1..127 domain")
    if args.duration <= 0:
        raise SystemExit("duration must be positive")
    if args.gop <= 0:
        raise SystemExit("GOP must be positive")

    archive_path = pathlib.Path(args.archive)
    ffmpeg_log_path = pathlib.Path(args.ffmpeg_log)
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    ffmpeg_log_path.parent.mkdir(parents=True, exist_ok=True)

    config_payload = CONFIG.pack(
        1,
        args.queue_capacity,
        args.receiver_stack,
        args.receiver_priority,
    )

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        "O3_SERVER_READY=YES "
        f"listen={args.listen} port={args.port} "
        f"queue_capacity={args.queue_capacity} "
        f"receiver_stack={args.receiver_stack} "
        f"receiver_priority={args.receiver_priority} "
        f"display={args.display} source_size={args.source_size} "
        f"duration={args.duration} bitrate={args.bitrate} gop={args.gop}",
        flush=True,
    )

    connection, address = listener.accept()
    connection.settimeout(120.0)

    print(f"O3_CLIENT_CONNECTED={address[0]}:{address[1]}", flush=True)

    pi_sequence = 0
    expected_ps2_sequence = 0

    send_frame(
        connection,
        kind=FRAME_CONFIG,
        channel=CHANNEL_CONTROL,
        flags=0,
        sequence=pi_sequence,
        payload=config_payload,
    )
    pi_sequence += 1

    kind, channel, flags, sequence, payload = recv_frame(connection)

    if sequence != expected_ps2_sequence:
        raise RuntimeError(
            "CONFIG ACK sequence mismatch "
            f"expected={expected_ps2_sequence} actual={sequence}"
        )
    expected_ps2_sequence += 1

    if (
        kind != FRAME_CONFIG
        or channel != CHANNEL_CONTROL
        or flags != CONFIG_ACK_FLAG
        or payload != config_payload
    ):
        raise RuntimeError("PS2 CONFIG ACK did not echo the accepted profile")

    print("O3_CONFIG_ACK=PASS", flush=True)

    expected_ps2_sequence, available_credit, result = receive_credit_or_result(
        connection,
        expected_ps2_sequence=expected_ps2_sequence,
    )

    if result is not None:
        raise RuntimeError("received terminal result before initial credit")
    if available_credit != args.queue_capacity:
        raise RuntimeError(
            "initial credit differs from configured allocation: "
            f"credit={available_credit} config={args.queue_capacity}"
        )

    print(f"O3_INITIAL_CREDIT={available_credit}", flush=True)

    ffmpeg_command = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-f",
        "x11grab",
        "-framerate",
        "30000/1001",
        "-video_size",
        args.source_size,
        "-i",
        f"{args.display}+0,0",
        "-t",
        str(args.duration),
        "-vf",
        "pad=704:480:0:0:black,format=yuv420p",
        "-an",
        "-c:v",
        "mpeg2video",
        "-b:v",
        args.bitrate,
        "-g",
        str(args.gop),
        "-bf",
        "0",
        "-f",
        "mpeg2video",
        "pipe:1",
    ]

    print(f"O3_FFMPEG_COMMAND={shlex.join(ffmpeg_command)}", flush=True)

    digest = hashlib.sha256()
    crc32 = 0
    start_codes = StartCodeCounter()

    frames_sent = 0
    payload_bytes_sent = 0
    credit_frames_received = 1
    credit_bytes_received = available_credit
    credit_wait_events = 0
    credit_wait_seconds = 0.0
    max_credit_wait_seconds = 0.0

    producer_start = time.monotonic()

    with (
        archive_path.open("wb") as archive,
        ffmpeg_log_path.open("wb") as ffmpeg_log,
    ):
        process = subprocess.Popen(
            ffmpeg_command,
            stdout=subprocess.PIPE,
            stderr=ffmpeg_log,
            bufsize=0,
            env={**os.environ, "DISPLAY": args.display.split(".")[0]},
        )

        if process.stdout is None:
            raise RuntimeError("ffmpeg stdout pipe was not created")

        while True:
            payload = read_full_payload(process.stdout)
            if not payload:
                break

            archive.write(payload)
            digest.update(payload)
            crc32 = zlib.crc32(payload, crc32)
            start_codes.add(payload)

            while available_credit < len(payload):
                wait_start = time.monotonic()
                credit_wait_events += 1

                expected_ps2_sequence, credit, premature_result = (
                    receive_credit_or_result(
                        connection,
                        expected_ps2_sequence=expected_ps2_sequence,
                    )
                )

                waited = time.monotonic() - wait_start
                credit_wait_seconds += waited
                max_credit_wait_seconds = max(max_credit_wait_seconds, waited)

                if premature_result is not None:
                    raise RuntimeError(
                        "PS2 sent terminal result before ffmpeg stream ended"
                    )

                available_credit += credit
                credit_frames_received += 1
                credit_bytes_received += credit

            send_frame(
                connection,
                kind=FRAME_DATA,
                channel=CHANNEL_MPEG2,
                flags=0,
                sequence=pi_sequence,
                payload=payload,
            )

            pi_sequence += 1
            available_credit -= len(payload)
            frames_sent += 1
            payload_bytes_sent += len(payload)

        process.stdout.close()
        ffmpeg_return = process.wait()
        archive.flush()

    producer_elapsed = time.monotonic() - producer_start
    crc32 &= 0xFFFFFFFF
    sha256 = digest.hexdigest()

    if ffmpeg_return != 0:
        raise RuntimeError(f"ffmpeg exited with status {ffmpeg_return}")
    if payload_bytes_sent == 0 or frames_sent == 0:
        raise RuntimeError("ffmpeg produced no MPEG payload")
    if start_codes.picture_starts == 0:
        raise RuntimeError("live MPEG stream contains no picture start codes")

    last_data_sequence = pi_sequence - 1

    print(
        "O3_LIVE_STREAM_SEND=PASS "
        f"frames={frames_sent} payload_bytes={payload_bytes_sent} "
        f"last_data_sequence={last_data_sequence} "
        f"sha256={sha256} crc32={crc32:08x} "
        f"picture_starts={start_codes.picture_starts} "
        f"sequence_headers={start_codes.sequence_headers} "
        f"sequence_ends={start_codes.sequence_ends} "
        f"producer_elapsed_seconds={producer_elapsed:.6f} "
        f"payload_MiBps={payload_bytes_sent / (1024 * 1024) / producer_elapsed:.6f} "
        f"credit_frames_received={credit_frames_received} "
        f"credit_bytes_received={credit_bytes_received} "
        f"credit_wait_events={credit_wait_events} "
        f"credit_wait_seconds={credit_wait_seconds:.6f} "
        f"max_credit_wait_seconds={max_credit_wait_seconds:.6f}",
        flush=True,
    )

    end_payload = END.pack(
        1,
        payload_bytes_sent,
        frames_sent,
        last_data_sequence,
        crc32,
        start_codes.picture_starts,
        start_codes.sequence_headers,
        start_codes.sequence_ends,
    )

    send_frame(
        connection,
        kind=FRAME_TELEMETRY,
        channel=CHANNEL_TELEMETRY,
        flags=0,
        sequence=pi_sequence,
        payload=end_payload,
    )
    pi_sequence += 1

    print(
        f"O3_END_METADATA_SENT=PASS sequence={pi_sequence - 1}",
        flush=True,
    )

    terminal_result: dict[str, int] | None = None
    result_wait_start = time.monotonic()

    while terminal_result is None:
        expected_ps2_sequence, credit, terminal_result = receive_credit_or_result(
            connection,
            expected_ps2_sequence=expected_ps2_sequence,
        )

        if credit:
            available_credit += credit
            credit_frames_received += 1
            credit_bytes_received += credit

    result_wait_seconds = time.monotonic() - result_wait_start
    print(f"O3_RESULT_WAIT_SECONDS={result_wait_seconds:.6f}", flush=True)

    for name in RESULT_WORD_NAMES:
        value = terminal_result[name]
        if name in ("crc32", "producer_crc32"):
            print(f"O3_RESULT_{name.upper()}={value:08x}", flush=True)
        else:
            print(f"O3_RESULT_{name.upper()}={value}", flush=True)

    expected = terminal_result

    if expected["version"] != 1:
        raise RuntimeError("unexpected O3 result version")
    if expected["error"] != 0:
        raise RuntimeError(f"PS2 live stream error={expected['error']}")
    if expected["queue_capacity"] != args.queue_capacity:
        raise RuntimeError("PS2 result queue capacity differs from CONFIG")
    if expected["queue_current"] != 0:
        raise RuntimeError("terminal PS2 queue is not empty")
    if expected["frames_received"] != frames_sent:
        raise RuntimeError("PS2 DATA frame count mismatch")
    if expected["bytes_received"] != payload_bytes_sent:
        raise RuntimeError("PS2 received-byte count mismatch")
    if expected["last_data_sequence"] != last_data_sequence:
        raise RuntimeError("PS2 last DATA sequence mismatch")
    if expected["crc32"] != crc32:
        raise RuntimeError("PS2 live CRC32 mismatch")
    if expected["bytes_consumed"] != payload_bytes_sent:
        raise RuntimeError("decoder did not consume exact live byte count")
    if expected["receiver_done"] != 1 or expected["end_received"] != 1:
        raise RuntimeError("PS2 did not reach terminal live-stream state")
    if expected["integrity_pass"] != 1:
        raise RuntimeError("PS2 live terminal integrity contract failed")

    producer_pairs = (
        ("producer_bytes", payload_bytes_sent),
        ("producer_frames", frames_sent),
        ("producer_last_data_sequence", last_data_sequence),
        ("producer_crc32", crc32),
        ("producer_picture_starts", start_codes.picture_starts),
        ("producer_sequence_headers", start_codes.sequence_headers),
        ("producer_sequence_ends", start_codes.sequence_ends),
    )

    for name, value in producer_pairs:
        if expected[name] != value:
            raise RuntimeError(
                f"PS2 terminal producer metadata mismatch: {name}"
            )

    if expected["pictures_displayed"] != expected["pictures_decoded"]:
        raise RuntimeError("decoded/displayed live picture counts differ")

    decoder_delta = start_codes.picture_starts - expected["pictures_decoded"]
    print(f"O3_DECODER_RETURN_DELTA={decoder_delta}", flush=True)

    if decoder_delta not in (0, 1):
        raise RuntimeError(
            "live SMS returned-picture count differs from coded count by more than one"
        )

    print("O3_RESULT_CONTRACT=PASS", flush=True)

    connection.close()
    listener.close()

    decode = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", str(archive_path), "-f", "null", "-"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        check=False,
    )

    if decode.returncode != 0:
        raise RuntimeError(
            "archived live MPEG failed Pi software decode: "
            + decode.stderr.decode("utf-8", errors="replace")
        )

    print("O3_ARCHIVE_PI_DECODE=PASS", flush=True)
    print(f"O3_ARCHIVE_PATH={archive_path}", flush=True)
    print(f"O3_FFMPEG_LOG={ffmpeg_log_path}", flush=True)
    print("O3_OVERALL_PI_EVIDENCE=PASS", flush=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
