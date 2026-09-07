#!/usr/bin/env python3
"""
File synopsis:
    EXP3 O1 one-shot MPEG fixture replay server.

This is deliberately not the final PS2VNC scheduler. It changes one boundary:
the exact qualified MPEG fixture is carried over TCP using the existing PSTV
version-1 DATA framing and reserved MPEG2 channel 4.

The PS2 buffers and validates the entire fixture before decoding, so O1 does
not test streaming queue depth, starvation, pacing, or A/V synchronization.
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

FRAME_DATA = 3
CHANNEL_MPEG2 = 4

HEADER = struct.Struct(">IBBBBII")

MAX_PAYLOAD = 8192

EXPECTED_SHA256 = (
    "5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f"
)
EXPECTED_BYTES = 8815372
EXPECTED_FRAMES = 1077
EXPECTED_FINAL_PAYLOAD = 780


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fixture", required=True)
    parser.add_argument("--listen", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5904)
    args = parser.parse_args()

    fixture_path = pathlib.Path(args.fixture)
    data = fixture_path.read_bytes()

    digest = hashlib.sha256(data).hexdigest()
    crc32 = zlib.crc32(data) & 0xFFFFFFFF

    print(f"O1_FIXTURE_PATH={fixture_path}", flush=True)
    print(f"O1_FIXTURE_SHA256={digest}", flush=True)
    print(f"O1_FIXTURE_BYTES={len(data)}", flush=True)
    print(f"O1_FIXTURE_CRC32={crc32:08x}", flush=True)

    if digest != EXPECTED_SHA256:
        raise SystemExit("fixture SHA-256 mismatch")

    if len(data) != EXPECTED_BYTES:
        raise SystemExit("fixture byte-count mismatch")

    full_frames, tail = divmod(
        len(data),
        MAX_PAYLOAD,
    )

    frame_count = (
        full_frames +
        (1 if tail else 0)
    )

    if frame_count != EXPECTED_FRAMES:
        raise SystemExit("fixture frame-count mismatch")

    if tail != EXPECTED_FINAL_PAYLOAD:
        raise SystemExit("fixture final-payload mismatch")

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
        (args.listen, args.port)
    )

    listener.listen(1)

    print(
        "O1_SERVER_READY=YES "
        f"listen={args.listen} "
        f"port={args.port} "
        f"expected_frames={EXPECTED_FRAMES}",
        flush=True,
    )

    connection, address = listener.accept()

    print(
        "O1_CLIENT_CONNECTED="
        f"{address[0]}:{address[1]}",
        flush=True,
    )

    start = time.monotonic()

    frames_sent = 0
    payload_bytes_sent = 0
    sequence = 0

    with connection:
        for offset in range(
            0,
            len(data),
            MAX_PAYLOAD,
        ):
            payload = data[
                offset:
                offset + MAX_PAYLOAD
            ]

            header = HEADER.pack(
                MAGIC,
                VERSION,
                FRAME_DATA,
                CHANNEL_MPEG2,
                0,
                sequence,
                len(payload),
            )

            connection.sendall(header)
            connection.sendall(payload)

            frames_sent += 1
            payload_bytes_sent += len(payload)
            sequence += 1

        connection.shutdown(
            socket.SHUT_WR
        )

        peer_closed = False

        connection.settimeout(10.0)

        try:
            while True:
                extra = connection.recv(1)

                if not extra:
                    peer_closed = True
                    break

        except socket.timeout:
            pass

    elapsed = (
        time.monotonic() -
        start
    )

    listener.close()

    if frames_sent != EXPECTED_FRAMES:
        raise SystemExit(
            "sent frame-count mismatch"
        )

    if payload_bytes_sent != EXPECTED_BYTES:
        raise SystemExit(
            "sent byte-count mismatch"
        )

    wire_bytes = (
        payload_bytes_sent +
        frames_sent * HEADER.size
    )

    mib_per_second = (
        payload_bytes_sent /
        (1024.0 * 1024.0) /
        elapsed
        if elapsed > 0
        else 0.0
    )

    print(
        "O1_REPLAY_SEND=PASS "
        f"frames={frames_sent} "
        f"payload_bytes={payload_bytes_sent} "
        f"header_bytes={frames_sent * HEADER.size} "
        f"wire_bytes={wire_bytes} "
        f"last_sequence={sequence - 1} "
        f"elapsed_seconds={elapsed:.6f} "
        f"payload_MiBps={mib_per_second:.6f} "
        f"peer_closed={'YES' if peer_closed else 'NO'}",
        flush=True,
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
