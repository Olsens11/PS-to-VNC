#!/usr/bin/env python3
"""
File synopsis:
    Disposable CP2P runner layered on the priority-67 persistent MPEG witness.

Purpose:
    Preserve the parent Pi transport/RFB/suppression behavior while decoding the
    DAxxNNNN stage family emitted from inside the PS2 cancellable MPEG queue-read
    loop. The low 16 bits are that call's empty-queue wait-loop counter, allowing
    a repeatedly waking reader to be distinguished from a reader that stops at
    one blocking boundary.

No Pi scheduling, queue, producer, RFB, suppression, retirement, or media policy
is changed.
"""

from __future__ import annotations

import h1_mux_server_cp2p_mpeg_persistent_witness as persistent

base = persistent.base

_parent_decode_stage = persistent.tx.stall._decode_stage

_MPEG_READ_INTERNAL_STAGE_NAMES = {
    0x01: "MPEG_READ_ENTER",
    0x02: "MPEG_READ_LOOP_ENTER",
    0x03: "MPEG_READ_BEFORE_QUEUE_WAIT_SEMA",
    0x04: "MPEG_READ_QUEUE_WAIT_SEMA_RETURN",
    0x05: "MPEG_READ_QUEUE_SAMPLED",
    0x06: "MPEG_READ_BEFORE_QUEUE_READ",
    0x07: "MPEG_READ_QUEUE_READ_RETURN",
    0x08: "MPEG_READ_BEFORE_QUEUE_SIGNAL_SEMA",
    0x09: "MPEG_READ_QUEUE_SIGNAL_SEMA_RETURN",
    0x0A: "MPEG_READ_BEFORE_CREDIT_RETURN",
    0x0B: "MPEG_READ_CREDIT_RETURN",
    0x0C: "MPEG_READ_RETURN_SUCCESS",
    0x0D: "MPEG_READ_BEFORE_EMPTY_DELAY",
    0x0E: "MPEG_READ_EMPTY_DELAY_RETURN",
    0x0F: "MPEG_READ_CANCELLED",
    0x10: "MPEG_READ_END_RECEIVED_EMPTY",
    0xFF: "MPEG_READ_ERROR",
}


def _decode_internal_mpeg_stage(word: int) -> str:
    word &= 0xFFFFFFFF
    if (word & 0xFF000000) == 0xDA000000:
        operation = (word >> 16) & 0xFF
        wait_loops = word & 0xFFFF
        name = _MPEG_READ_INTERNAL_STAGE_NAMES.get(
            operation,
            f"MPEG_READ_INTERNAL_UNKNOWN_0x{operation:02X}",
        )
        return f"{name}[wait_loops={wait_loops}]"
    return _parent_decode_stage(word)


# The parent persistent-witness session resolves this decoder dynamically from
# tx.stall. Replace only that presentation function; all session behavior and
# evidence plumbing remain the parent implementation.
persistent.tx.stall._decode_stage = _decode_internal_mpeg_stage

base.H1Session = persistent.H1Cp2pMpegPersistentWitnessSession


if __name__ == "__main__":
    raise SystemExit(base.main())
