#!/usr/bin/env python3
"""
File synopsis:
    EXP3 O2 credit-aware concurrent MPEG fixture sender.

The exact O1 fixture is retained. The Pi first sends a small connection-scoped
CONFIG selecting real PS2 queue capacity and receiver-thread resources. After
the PS2 echoes CONFIG and grants receiver CREDIT, the Pi sends the same fixed
8192-byte PSTV DATA/channel-4 frames used by O1, but only when sufficient real
receiver credit exists.

This sender intentionally remains a fixture replay tool, not the final mux or
ffmpeg producer. Its only purpose is to qualify concurrent network production
and SMS MPEG consumption with strong queue/starvation telemetry.
"""

from __future__ import annotations

import argparse
import hashlib
import pathlib
import socket
import struct
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

MAX_PAYLOAD = 8192

EXPECTED_SHA256 = (
    "5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f"
)
EXPECTED_BYTES = 8815372
EXPECTED_CRC32 = 0x4BDC7859
EXPECTED_FRAMES = 1077
EXPECTED_FINAL_PAYLOAD = 780

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
        raise RuntimeError(f"bad version {version}")

    if length > MAX_PAYLOAD:
        raise RuntimeError(f"payload length exceeds protocol maximum: {length}")

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
        raise ValueError("payload exceeds protocol maximum")

    header = HEADER.pack(
        MAGIC,
        VERSION,
        kind,
        channel,
        flags,
        sequence,
        len(payload),
    )

    connection.sendall(header)
    connection.sendall(payload)


def receive_credit_or_result(
    connection: socket.socket,
    *,
    expected_ps2_sequence: int,
) -> tuple[int, int, dict[str, int] | None]:
    kind, channel, flags, sequence, payload = recv_frame(connection)

    if sequence != expected_ps2_sequence:
        raise RuntimeError(
            f"PS2 sequence mismatch expected={expected_ps2_sequence} actual={sequence}"
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

        result = dict(zip(RESULT_WORD_NAMES, words, strict=True))

        return expected_ps2_sequence, 0, result

    raise RuntimeError(
        "unexpected PS2 frame "
        f"kind={kind} channel={channel} flags={flags} "
        f"sequence={sequence} length={len(payload)}"
    )


def main() -> int:
    parser = argparse.ArgumentParser()

    parser.add_argument("--fixture", required=True)
    parser.add_argument("--listen", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5904)

    parser.add_argument(
        "--queue-capacity",
        type=int,
        default=8192,
        help=(
            "exact PS2 MPEG ring allocation; must be a positive multiple of 8192"
        ),
    )

    parser.add_argument(
        "--receiver-stack",
        type=int,
        default=16384,
    )

    parser.add_argument(
        "--receiver-priority",
        type=int,
        default=63,
    )

    args = parser.parse_args()

    if args.queue_capacity < MAX_PAYLOAD:
        raise SystemExit("queue capacity must be at least one maximum DATA frame")

    if args.queue_capacity % MAX_PAYLOAD:
        raise SystemExit("queue capacity must be an exact multiple of 8192")

    if args.receiver_stack < 256 or args.receiver_stack % 16:
        raise SystemExit("receiver stack must be >=256 and 16-byte aligned in size")

    if not 1 <= args.receiver_priority <= 127:
        raise SystemExit("receiver priority must be in the EE 1..127 domain")

    fixture_path = pathlib.Path(args.fixture)
    data = fixture_path.read_bytes()

    digest = hashlib.sha256(data).hexdigest()
    crc32 = zlib.crc32(data) & 0xFFFFFFFF

    print(f"O2_FIXTURE_PATH={fixture_path}", flush=True)
    print(f"O2_FIXTURE_SHA256={digest}", flush=True)
    print(f"O2_FIXTURE_BYTES={len(data)}", flush=True)
    print(f"O2_FIXTURE_CRC32={crc32:08x}", flush=True)

    if digest != EXPECTED_SHA256:
        raise SystemExit("fixture SHA-256 mismatch")

    if len(data) != EXPECTED_BYTES:
        raise SystemExit("fixture byte-count mismatch")

    if crc32 != EXPECTED_CRC32:
        raise SystemExit("fixture CRC32 mismatch")

    full_frames, tail = divmod(len(data), MAX_PAYLOAD)

    frame_count = full_frames + (1 if tail else 0)

    if full_frames != 1076 or tail != EXPECTED_FINAL_PAYLOAD:
        raise SystemExit("fixture DATA framing shape changed")

    if frame_count != EXPECTED_FRAMES:
        raise SystemExit("fixture frame count changed")

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
        "O2_SERVER_READY=YES "
        f"listen={args.listen} "
        f"port={args.port} "
        f"queue_capacity={args.queue_capacity} "
        f"receiver_stack={args.receiver_stack} "
        f"receiver_priority={args.receiver_priority}",
        flush=True,
    )

    connection, address = listener.accept()

    print(
        f"O2_CLIENT_CONNECTED={address[0]}:{address[1]}",
        flush=True,
    )

    connection.settimeout(30.0)

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
            f"CONFIG ACK sequence mismatch expected={expected_ps2_sequence} actual={sequence}"
        )

    expected_ps2_sequence += 1

    if (
        kind != FRAME_CONFIG
        or channel != CHANNEL_CONTROL
        or flags != CONFIG_ACK_FLAG
        or payload != config_payload
    ):
        raise RuntimeError("PS2 CONFIG ACK did not echo the exact accepted profile")

    print("O2_CONFIG_ACK=PASS", flush=True)

    expected_ps2_sequence, available_credit, result = receive_credit_or_result(
        connection,
        expected_ps2_sequence=expected_ps2_sequence,
    )

    if result is not None:
        raise RuntimeError("received result before initial credit")

    if available_credit != args.queue_capacity:
        raise RuntimeError(
            "initial credit does not equal actual configured queue capacity: "
            f"credit={available_credit} config={args.queue_capacity}"
        )

    print(
        f"O2_INITIAL_CREDIT={available_credit}",
        flush=True,
    )

    send_start = time.monotonic()

    frames_sent = 0
    payload_bytes_sent = 0

    credit_frames_received = 1
    credit_bytes_received = available_credit
    credit_wait_events = 0
    credit_wait_seconds = 0.0
    max_credit_wait_seconds = 0.0

    for offset in range(0, len(data), MAX_PAYLOAD):
        payload = data[offset : offset + MAX_PAYLOAD]

        while available_credit < len(payload):
            wait_start = time.monotonic()
            credit_wait_events += 1

            expected_ps2_sequence, credit, premature_result = receive_credit_or_result(
                connection,
                expected_ps2_sequence=expected_ps2_sequence,
            )

            waited = time.monotonic() - wait_start
            credit_wait_seconds += waited
            max_credit_wait_seconds = max(max_credit_wait_seconds, waited)

            if premature_result is not None:
                raise RuntimeError("PS2 sent terminal result before fixture transmission ended")

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

    send_elapsed = time.monotonic() - send_start

    if frames_sent != EXPECTED_FRAMES:
        raise RuntimeError("sent DATA frame count mismatch")

    if payload_bytes_sent != EXPECTED_BYTES:
        raise RuntimeError("sent payload byte count mismatch")

    print(
        "O2_STREAM_SEND=PASS "
        f"frames={frames_sent} "
        f"payload_bytes={payload_bytes_sent} "
        f"last_data_sequence={pi_sequence - 1} "
        f"elapsed_seconds={send_elapsed:.6f} "
        f"payload_MiBps={payload_bytes_sent / (1024 * 1024) / send_elapsed:.6f} "
        f"credit_frames_received={credit_frames_received} "
        f"credit_bytes_received={credit_bytes_received} "
        f"credit_wait_events={credit_wait_events} "
        f"credit_wait_seconds={credit_wait_seconds:.6f} "
        f"max_credit_wait_seconds={max_credit_wait_seconds:.6f}",
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

    print(
        f"O2_RESULT_WAIT_SECONDS={result_wait_seconds:.6f}",
        flush=True,
    )

    for name in RESULT_WORD_NAMES:
        value = terminal_result[name]

        if name == "crc32":
            print(f"O2_RESULT_{name.upper()}={value:08x}", flush=True)
        else:
            print(f"O2_RESULT_{name.upper()}={value}", flush=True)

    if terminal_result["version"] != 1:
        raise RuntimeError("unexpected result version")

    if terminal_result["queue_capacity"] != args.queue_capacity:
        raise RuntimeError("PS2 result queue capacity differs from CONFIG")

    if terminal_result["frames_received"] != EXPECTED_FRAMES:
        raise RuntimeError("PS2 frame count mismatch")

    if terminal_result["bytes_received"] != EXPECTED_BYTES:
        raise RuntimeError("PS2 received-byte mismatch")

    if terminal_result["last_data_sequence"] != EXPECTED_FRAMES:
        raise RuntimeError("PS2 last DATA sequence mismatch")

    if terminal_result["crc32"] != EXPECTED_CRC32:
        raise RuntimeError("PS2 CRC32 mismatch")

    if terminal_result["bytes_consumed"] != EXPECTED_BYTES:
        raise RuntimeError("decoder did not consume exact fixture byte count")

    if terminal_result["queue_current"] != 0:
        raise RuntimeError("terminal queue is not empty")

    if terminal_result["receiver_done"] != 1:
        raise RuntimeError("receiver did not reach terminal state")

    if terminal_result["integrity_pass"] != 1:
        raise RuntimeError("PS2 terminal integrity contract failed")

    if terminal_result["error"] != 0:
        raise RuntimeError(f"PS2 stream error={terminal_result['error']}")

    if terminal_result["pictures_decoded"] != 598:
        raise RuntimeError("unexpected SMS returned-picture count")

    if terminal_result["pictures_displayed"] != 598:
        raise RuntimeError("unexpected SMS displayed-picture count")

    print("O2_RESULT_CONTRACT=PASS", flush=True)

    connection.close()
    listener.close()

    print("O2_OVERALL_PI_EVIDENCE=PASS", flush=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
