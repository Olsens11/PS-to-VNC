#!/usr/bin/env python3
"""
File synopsis:
    Replays an exact archived O3 MPEG elementary stream through the existing
    live PSTV protocol while optionally appending terminal zero bytes.

Purpose:
    This is a narrow EOF-boundary diagnostic. It reuses the same PS2 O3 ELF,
    queue capacity, decoder path, framing, credit flow, and terminal metadata.
    The only intentional variable is the number of zero bytes appended after
    the archived MPEG payload before terminal metadata is sent.
"""

from __future__ import annotations

import argparse
import pathlib
import socket
import struct
import time
import zlib

import network_live_stream_server as live


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--append-zero-bytes", type=int, default=0)
    parser.add_argument("--listen", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5904)
    parser.add_argument("--queue-capacity", type=int, default=32768)
    parser.add_argument("--receiver-stack", type=int, default=16384)
    parser.add_argument("--receiver-priority", type=int, default=63)
    args = parser.parse_args()

    if args.append_zero_bytes < 0 or args.append_zero_bytes > 64:
        raise SystemExit("append-zero-bytes must be in 0..64")
    if args.queue_capacity < live.MAX_PAYLOAD:
        raise SystemExit("queue capacity must be at least one maximum DATA frame")
    if args.queue_capacity % live.MAX_PAYLOAD:
        raise SystemExit("queue capacity must be an exact multiple of 8192")
    if args.receiver_stack < 256 or args.receiver_stack % 16:
        raise SystemExit("receiver stack must be >=256 and a multiple of 16")
    if not 1 <= args.receiver_priority <= 127:
        raise SystemExit("receiver priority must be in the EE 1..127 domain")

    input_path = pathlib.Path(args.input)
    original = input_path.read_bytes()
    payload = original + (b"\x00" * args.append_zero_bytes)

    if not original:
        raise SystemExit("input archive is empty")

    counter = live.StartCodeCounter()
    counter.add(payload)

    crc32 = zlib.crc32(payload) & 0xFFFFFFFF

    chunks = [
        payload[offset : offset + live.MAX_PAYLOAD]
        for offset in range(0, len(payload), live.MAX_PAYLOAD)
    ]

    config_payload = live.CONFIG.pack(
        1,
        args.queue_capacity,
        args.receiver_stack,
        args.receiver_priority,
    )

    print(
        "O3_EOF_REPLAY_SOURCE "
        f"input={input_path} original_bytes={len(original)} "
        f"append_zero_bytes={args.append_zero_bytes} "
        f"wire_payload_bytes={len(payload)} "
        f"mod16={len(payload) % 16} mod2048={len(payload) % 2048} "
        f"frames={len(chunks)} crc32={crc32:08x} "
        f"picture_starts={counter.picture_starts} "
        f"sequence_headers={counter.sequence_headers} "
        f"sequence_ends={counter.sequence_ends}",
        flush=True,
    )

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        "O3_EOF_REPLAY_SERVER_READY=YES "
        f"listen={args.listen} port={args.port} "
        f"queue_capacity={args.queue_capacity}",
        flush=True,
    )

    connection, address = listener.accept()
    connection.settimeout(120.0)

    print(f"O3_EOF_REPLAY_CLIENT_CONNECTED={address[0]}:{address[1]}", flush=True)

    pi_sequence = 0
    expected_ps2_sequence = 0

    live.send_frame(
        connection,
        kind=live.FRAME_CONFIG,
        channel=live.CHANNEL_CONTROL,
        flags=0,
        sequence=pi_sequence,
        payload=config_payload,
    )
    pi_sequence += 1

    kind, channel, flags, sequence, ack_payload = live.recv_frame(connection)

    if sequence != expected_ps2_sequence:
        raise RuntimeError(
            "CONFIG ACK sequence mismatch "
            f"expected={expected_ps2_sequence} actual={sequence}"
        )
    expected_ps2_sequence += 1

    if (
        kind != live.FRAME_CONFIG
        or channel != live.CHANNEL_CONTROL
        or flags != live.CONFIG_ACK_FLAG
        or ack_payload != config_payload
    ):
        raise RuntimeError("PS2 CONFIG ACK did not echo the accepted profile")

    print("O3_EOF_REPLAY_CONFIG_ACK=PASS", flush=True)

    expected_ps2_sequence, available_credit, result = live.receive_credit_or_result(
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

    print(f"O3_EOF_REPLAY_INITIAL_CREDIT={available_credit}", flush=True)

    credit_frames_received = 1
    credit_bytes_received = available_credit
    credit_wait_events = 0
    credit_wait_seconds = 0.0
    max_credit_wait_seconds = 0.0

    send_start = time.monotonic()

    for chunk in chunks:
        while available_credit < len(chunk):
            wait_start = time.monotonic()
            credit_wait_events += 1

            expected_ps2_sequence, credit, premature_result = (
                live.receive_credit_or_result(
                    connection,
                    expected_ps2_sequence=expected_ps2_sequence,
                )
            )

            waited = time.monotonic() - wait_start
            credit_wait_seconds += waited
            max_credit_wait_seconds = max(max_credit_wait_seconds, waited)

            if premature_result is not None:
                raise RuntimeError("PS2 sent terminal result before replay ended")

            available_credit += credit
            credit_frames_received += 1
            credit_bytes_received += credit

        live.send_frame(
            connection,
            kind=live.FRAME_DATA,
            channel=live.CHANNEL_MPEG2,
            flags=0,
            sequence=pi_sequence,
            payload=chunk,
        )

        pi_sequence += 1
        available_credit -= len(chunk)

    send_elapsed = time.monotonic() - send_start
    last_data_sequence = pi_sequence - 1

    print(
        "O3_EOF_REPLAY_SEND=PASS "
        f"frames={len(chunks)} payload_bytes={len(payload)} "
        f"last_data_sequence={last_data_sequence} "
        f"elapsed_seconds={send_elapsed:.6f} "
        f"credit_frames_received={credit_frames_received} "
        f"credit_bytes_received={credit_bytes_received} "
        f"credit_wait_events={credit_wait_events} "
        f"credit_wait_seconds={credit_wait_seconds:.6f} "
        f"max_credit_wait_seconds={max_credit_wait_seconds:.6f}",
        flush=True,
    )

    end_payload = live.END.pack(
        1,
        len(payload),
        len(chunks),
        last_data_sequence,
        crc32,
        counter.picture_starts,
        counter.sequence_headers,
        counter.sequence_ends,
    )

    live.send_frame(
        connection,
        kind=live.FRAME_TELEMETRY,
        channel=live.CHANNEL_TELEMETRY,
        flags=0,
        sequence=pi_sequence,
        payload=end_payload,
    )
    pi_sequence += 1

    print(
        f"O3_EOF_REPLAY_END_METADATA_SENT=PASS sequence={pi_sequence - 1}",
        flush=True,
    )

    terminal_result: dict[str, int] | None = None
    result_wait_start = time.monotonic()

    while terminal_result is None:
        expected_ps2_sequence, credit, terminal_result = live.receive_credit_or_result(
            connection,
            expected_ps2_sequence=expected_ps2_sequence,
        )

        if credit:
            available_credit += credit
            credit_frames_received += 1
            credit_bytes_received += credit

    result_wait_seconds = time.monotonic() - result_wait_start
    print(f"O3_EOF_REPLAY_RESULT_WAIT_SECONDS={result_wait_seconds:.6f}", flush=True)

    for name in live.RESULT_WORD_NAMES:
        value = terminal_result[name]
        if name in ("crc32", "producer_crc32"):
            print(f"O3_EOF_REPLAY_RESULT_{name.upper()}={value:08x}", flush=True)
        else:
            print(f"O3_EOF_REPLAY_RESULT_{name.upper()}={value}", flush=True)

    expected_pairs = (
        ("queue_capacity", args.queue_capacity),
        ("frames_received", len(chunks)),
        ("bytes_received", len(payload)),
        ("last_data_sequence", last_data_sequence),
        ("crc32", crc32),
        ("bytes_consumed", len(payload)),
        ("producer_bytes", len(payload)),
        ("producer_frames", len(chunks)),
        ("producer_last_data_sequence", last_data_sequence),
        ("producer_crc32", crc32),
        ("producer_picture_starts", counter.picture_starts),
        ("producer_sequence_headers", counter.sequence_headers),
        ("producer_sequence_ends", counter.sequence_ends),
    )

    if terminal_result["version"] != 1:
        raise RuntimeError("unexpected O3 result version")
    if terminal_result["error"] != 0:
        raise RuntimeError(f"PS2 replay stream error={terminal_result['error']}")
    if terminal_result["queue_current"] != 0:
        raise RuntimeError("terminal PS2 queue is not empty")
    if terminal_result["receiver_done"] != 1 or terminal_result["end_received"] != 1:
        raise RuntimeError("PS2 did not reach terminal replay state")
    if terminal_result["integrity_pass"] != 1:
        raise RuntimeError("PS2 replay terminal integrity contract failed")

    for name, value in expected_pairs:
        if terminal_result[name] != value:
            raise RuntimeError(
                f"PS2 replay result mismatch: {name} "
                f"expected={value} actual={terminal_result[name]}"
            )

    if terminal_result["pictures_displayed"] != terminal_result["pictures_decoded"]:
        raise RuntimeError("decoded/displayed replay picture counts differ")

    decoder_delta = counter.picture_starts - terminal_result["pictures_decoded"]
    print(f"O3_EOF_REPLAY_DECODER_RETURN_DELTA={decoder_delta}", flush=True)

    if decoder_delta not in (0, 1):
        raise RuntimeError(
            "replay SMS returned-picture count differs from coded count by more than one"
        )

    print("O3_EOF_REPLAY_RESULT_CONTRACT=PASS", flush=True)
    print("O3_EOF_REPLAY_OVERALL_PI_EVIDENCE=PASS", flush=True)

    connection.close()
    listener.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
