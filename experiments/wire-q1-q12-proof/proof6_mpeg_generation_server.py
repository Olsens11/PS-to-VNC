#!/usr/bin/env python3
"""
Proof 6 Pi server: one MPEG generation inside one live Wire Session.

Q4 establishment and the compact RFB-channel control/status messages are
development apparatus. Product Transport on the PS2 owns the actual physical
Wire session, framing, logical MPEG/RFB queues, flow control and I/O ownership.

The server streams one generated MPEG-2 elementary stream only while MPEG credit
exists. When the PS2 reports a safe decoder stop, the server closes production,
proves no send is in flight, acknowledges retirement, and sends no further MPEG
DATA. The PS2 then drains residual accepted MPEG bytes and must complete a
logical-RFB round trip before the server closes the still-same Wire Session.
"""

from __future__ import annotations

import argparse
import hashlib
import select
import socket
import struct
import sys
from pathlib import Path

import proof1_wire_server as base

FRAME_DATA = 3
FRAME_CREDIT = 4

CHANNEL_RFB = 1
CHANNEL_MPEG2 = 4

CREDIT = struct.Struct(">I")
STOP_STATUS = struct.Struct(">4sIIII")
LIVE_STATUS = struct.Struct(">4sII")

MRET = b"MRET"
ROK = b"ROK!"
DONE = b"DONE"

MAX_PAYLOAD = 8192
EXPECTED_RFB_INITIAL_CREDIT = 64
EXPECTED_MPEG_INITIAL_CREDIT = 512 * 1024


class Proof6Error(RuntimeError):
    pass


def encode_frame(
    kind: int,
    channel: int,
    sequence: int,
    payload: bytes,
) -> bytes:
    return (
        base.HEADER.pack(
            base.MAGIC,
            base.WIRE_HEADER_VERSION,
            kind,
            channel,
            0,
            sequence,
            len(payload),
        )
        + payload
    )


class WirePeer:
    def __init__(self, sock: socket.socket) -> None:
        self.sock = sock
        self.tx_sequence = 1
        self.rx_sequence = 1
        self.rfb_credit = 0
        self.mpeg_credit = 0

    def send_data(self, channel: int, payload: bytes) -> None:
        if not payload or len(payload) > MAX_PAYLOAD:
            raise Proof6Error(f"invalid outbound payload length {len(payload)}")

        if channel == CHANNEL_RFB:
            if len(payload) > self.rfb_credit:
                raise Proof6Error(
                    f"RFB credit exhausted have={self.rfb_credit} need={len(payload)}"
                )
            self.rfb_credit -= len(payload)
        elif channel == CHANNEL_MPEG2:
            if len(payload) > self.mpeg_credit:
                raise Proof6Error(
                    f"MPEG credit exhausted have={self.mpeg_credit} need={len(payload)}"
                )
            self.mpeg_credit -= len(payload)
        else:
            raise Proof6Error(f"unsupported outbound channel {channel}")

        self.sock.sendall(
            encode_frame(
                FRAME_DATA,
                channel,
                self.tx_sequence,
                payload,
            )
        )
        self.tx_sequence += 1

    def receive_frame(self) -> tuple[int, int, bytes]:
        raw = base.read_exact(self.sock, base.HEADER_BYTES)
        magic, version, kind, channel, flags, sequence, length = base.HEADER.unpack(raw)

        if magic != base.MAGIC or version != base.WIRE_HEADER_VERSION or flags != 0:
            raise Proof6Error(
                "invalid frame header "
                f"magic={magic!r} version={version} flags={flags}"
            )
        if sequence != self.rx_sequence:
            raise Proof6Error(
                f"PS2 sequence mismatch expected={self.rx_sequence} got={sequence}"
            )
        if length > MAX_PAYLOAD:
            raise Proof6Error(f"oversize frame payload {length}")

        self.rx_sequence += 1
        return kind, channel, base.read_exact(self.sock, length)

    def accept_credit(self, channel: int, payload: bytes) -> int:
        if len(payload) != CREDIT.size:
            raise Proof6Error(
                f"invalid credit payload channel={channel} bytes={len(payload)}"
            )
        amount = CREDIT.unpack(payload)[0]
        if amount == 0:
            raise Proof6Error("zero credit")

        if channel == CHANNEL_RFB:
            self.rfb_credit += amount
        elif channel == CHANNEL_MPEG2:
            self.mpeg_credit += amount
        else:
            raise Proof6Error(f"credit for unsupported channel {channel}")

        return amount


def establish(listener: socket.socket) -> tuple[WirePeer, int]:
    connection, peer = listener.accept()
    connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    connection.settimeout(20.0)

    print(
        f"PROOF6_SESSION_PROVISIONAL peer={peer[0]}:{peer[1]}",
        flush=True,
    )

    wire_version, product_version = base.receive_hello(connection)
    if (wire_version, product_version) != (base.WIRE_VERSION, base.PRODUCT_VERSION):
        raise Proof6Error(
            "incompatible proof establishment "
            f"wire={wire_version} product={product_version}"
        )

    session_id = base.nonzero_session_id()
    base.send_result(connection, base.FRAME_ACCEPT, session_id)

    print(
        f"PROOF6_SESSION=ACTIVE proof_session_id={session_id}",
        flush=True,
    )
    return WirePeer(connection), session_id


def receive_initial_credits(peer: WirePeer) -> None:
    seen_rfb = False
    seen_mpeg = False

    while not (seen_rfb and seen_mpeg):
        kind, channel, payload = peer.receive_frame()
        if kind != FRAME_CREDIT:
            raise Proof6Error(
                f"expected initial credit kind={kind} channel={channel}"
            )

        amount = peer.accept_credit(channel, payload)

        if channel == CHANNEL_RFB:
            if seen_rfb or amount != EXPECTED_RFB_INITIAL_CREDIT:
                raise Proof6Error(
                    f"unexpected RFB initial credit amount={amount}"
                )
            seen_rfb = True
            print(f"PROOF6_INITIAL_RFB_CREDIT={amount}", flush=True)
        elif channel == CHANNEL_MPEG2:
            if seen_mpeg or amount != EXPECTED_MPEG_INITIAL_CREDIT:
                raise Proof6Error(
                    f"unexpected MPEG initial credit amount={amount}"
                )
            seen_mpeg = True
            print(f"PROOF6_INITIAL_MPEG_CREDIT={amount}", flush=True)


def parse_stop_status(payload: bytes) -> tuple[int, int, int, int]:
    if len(payload) != STOP_STATUS.size:
        raise Proof6Error(
            f"unexpected RFB control size before stop {len(payload)}"
        )
    marker, decoded, presented, feed_calls, payload_bytes = STOP_STATUS.unpack(payload)
    if marker != b"MSTP":
        raise Proof6Error(f"unexpected pre-retire marker {marker!r}")
    if decoded < 90 or presented < 90 or feed_calls == 0:
        raise Proof6Error(
            "decoder metrics below proof threshold "
            f"decoded={decoded} presented={presented} feed_calls={feed_calls}"
        )
    return decoded, presented, feed_calls, payload_bytes


def run(args: argparse.Namespace) -> int:
    stream_path = Path(args.stream)
    stream = stream_path.read_bytes()
    if not stream:
        raise Proof6Error("MPEG stream is empty")

    stream_sha = hashlib.sha256(stream).hexdigest()

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(f"PROOF6_LISTENING={args.listen}:{args.port}", flush=True)
    print(f"PROOF6_STREAM={stream_path}", flush=True)
    print(f"PROOF6_STREAM_BYTES={len(stream)}", flush=True)
    print(f"PROOF6_STREAM_SHA256={stream_sha}", flush=True)

    peer: WirePeer | None = None

    try:
        peer, session_id = establish(listener)
        receive_initial_credits(peer)

        offset = 0
        mpeg_bytes_sent = 0
        mpeg_frames_sent = 0
        mpeg_credit_returned = 0
        stop_metrics: tuple[int, int, int, int] | None = None

        while stop_metrics is None:
            readable, _, _ = select.select([peer.sock], [], [], 0.0)
            if readable:
                kind, channel, payload = peer.receive_frame()

                if kind == FRAME_CREDIT:
                    amount = peer.accept_credit(channel, payload)
                    if channel == CHANNEL_MPEG2:
                        mpeg_credit_returned += amount
                    continue

                if kind == FRAME_DATA and channel == CHANNEL_RFB:
                    stop_metrics = parse_stop_status(payload)
                    decoded, presented, feed_calls, payload_bytes = stop_metrics
                    print(
                        "PROOF6_DECODER_SAFE_STOP_REPORT "
                        f"decoded={decoded} presented={presented} "
                        f"feed_calls={feed_calls} payload_bytes={payload_bytes}",
                        flush=True,
                    )
                    break

                raise Proof6Error(
                    f"unexpected PS2 frame before stop "
                    f"kind={kind} channel={channel} payload={payload.hex()}"
                )

            if peer.mpeg_credit == 0:
                kind, channel, payload = peer.receive_frame()
                if kind == FRAME_CREDIT:
                    amount = peer.accept_credit(channel, payload)
                    if channel == CHANNEL_MPEG2:
                        mpeg_credit_returned += amount
                    continue
                if kind == FRAME_DATA and channel == CHANNEL_RFB:
                    stop_metrics = parse_stop_status(payload)
                    decoded, presented, feed_calls, payload_bytes = stop_metrics
                    print(
                        "PROOF6_DECODER_SAFE_STOP_REPORT "
                        f"decoded={decoded} presented={presented} "
                        f"feed_calls={feed_calls} payload_bytes={payload_bytes}",
                        flush=True,
                    )
                    break
                raise Proof6Error(
                    f"unexpected frame while waiting for MPEG credit "
                    f"kind={kind} channel={channel}"
                )

            if offset >= len(stream):
                raise Proof6Error(
                    "test stream exhausted before PS2 requested generation stop"
                )

            count = min(
                MAX_PAYLOAD,
                peer.mpeg_credit,
                len(stream) - offset,
            )
            peer.send_data(
                CHANNEL_MPEG2,
                stream[offset : offset + count],
            )
            offset += count
            mpeg_bytes_sent += count
            mpeg_frames_sent += 1

        if stop_metrics is None:
            raise Proof6Error("missing stop metrics")

        # No further MPEG DATA is emitted after this point. This Python loop is
        # the proof-local producer owner, so reaching here is also the in-flight
        # send fence for this single-threaded apparatus.
        print(
            "PROOF6_MPEG_PRODUCTION_CLOSED=YES "
            f"bytes_sent={mpeg_bytes_sent} frames_sent={mpeg_frames_sent} "
            f"credit_returned_before_stop={mpeg_credit_returned}",
            flush=True,
        )

        peer.send_data(CHANNEL_RFB, MRET)
        print("PROOF6_RETIRE_ACK_SENT=MRET", flush=True)

        post_stop_mpeg_credit = 0
        live_metrics: tuple[int, int] | None = None

        while live_metrics is None:
            kind, channel, payload = peer.receive_frame()

            if kind == FRAME_CREDIT:
                amount = peer.accept_credit(channel, payload)
                if channel == CHANNEL_MPEG2:
                    post_stop_mpeg_credit += amount
                continue

            if kind == FRAME_DATA and channel == CHANNEL_RFB:
                if len(payload) != LIVE_STATUS.size:
                    raise Proof6Error(
                        f"unexpected post-retire RFB status size {len(payload)}"
                    )
                marker, residual_discarded, stop_in_picture = LIVE_STATUS.unpack(payload)
                if marker != b"LIVE":
                    raise Proof6Error(
                        f"unexpected post-retire marker {marker!r}"
                    )
                if stop_in_picture != 1:
                    raise Proof6Error(
                        "PS2 did not witness stop request during MPEG_Picture"
                    )
                live_metrics = (residual_discarded, stop_in_picture)
                print(
                    "PROOF6_RESIDUAL_FINALIZED "
                    f"discarded_bytes={residual_discarded} "
                    f"mpeg_credit_after_stop={post_stop_mpeg_credit} "
                    f"stop_requested_during_picture={stop_in_picture}",
                    flush=True,
                )
                break

            raise Proof6Error(
                f"unexpected post-retire frame "
                f"kind={kind} channel={channel} payload={payload.hex()}"
            )

        peer.send_data(CHANNEL_RFB, ROK)
        print("PROOF6_POST_MPEG_RFB_REPLY_SENT=ROK!", flush=True)

        while True:
            kind, channel, payload = peer.receive_frame()

            if kind == FRAME_CREDIT:
                peer.accept_credit(channel, payload)
                continue

            if kind == FRAME_DATA and channel == CHANNEL_RFB and payload == DONE:
                break

            raise Proof6Error(
                f"unexpected final frame "
                f"kind={kind} channel={channel} payload={payload!r}"
            )

        decoded, presented, feed_calls, payload_bytes = stop_metrics
        residual_discarded, stop_in_picture = live_metrics

        print("PROOF6_POST_MPEG_RFB_ROUNDTRIP=PASS", flush=True)
        print(
            "PROOF6_ONE_GENERATION=PASS "
            f"proof_session_id={session_id} "
            f"decoded={decoded} presented={presented} "
            f"feed_calls={feed_calls} decoder_payload_bytes={payload_bytes} "
            f"mpeg_bytes_sent={mpeg_bytes_sent} "
            f"residual_discarded={residual_discarded} "
            f"stop_during_picture={stop_in_picture}",
            flush=True,
        )

        try:
            peer.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        peer.sock.close()
        peer = None
        return 0

    except (EOFError, OSError, base.ProofError, Proof6Error) as exc:
        print(f"PROOF6_ONE_GENERATION=FAIL detail={exc}", flush=True)
        return 2

    finally:
        if peer is not None:
            try:
                peer.sock.close()
            except OSError:
                pass
        listener.close()


def self_test() -> int:
    payload = STOP_STATUS.pack(b"MSTP", 90, 90, 123, 456789)
    marker, decoded, presented, feed_calls, payload_bytes = STOP_STATUS.unpack(payload)

    assert marker == b"MSTP"
    assert decoded == 90
    assert presented == 90
    assert feed_calls == 123
    assert payload_bytes == 456789

    live = LIVE_STATUS.pack(b"LIVE", 4096, 1)
    assert LIVE_STATUS.unpack(live) == (b"LIVE", 4096, 1)

    frame = encode_frame(FRAME_DATA, CHANNEL_MPEG2, 7, b"abc")
    values = base.HEADER.unpack(frame[: base.HEADER_BYTES])
    assert values[0] == base.MAGIC
    assert values[1] == base.WIRE_HEADER_VERSION
    assert values[2] == FRAME_DATA
    assert values[3] == CHANNEL_MPEG2
    assert values[5] == 7
    assert values[6] == 3
    assert frame[base.HEADER_BYTES :] == b"abc"

    print("PROOF6_MPEG_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument("--stream")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    if not args.stream:
        parser.error("--stream is required unless --self-test is used")

    return run(args)


if __name__ == "__main__":
    sys.exit(main())
