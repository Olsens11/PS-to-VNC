#!/usr/bin/env python3
"""
File synopsis:
    P7 finite archive replay wrapper that reuses the exact generic sequence-end
    replay apparatus but accepts the one validator failure expected when stale
    GS presentations are intentionally dropped.

Purpose:
    The underlying replay server validates transport integrity before it checks
    that pictures_displayed == pictures_decoded. P7 intentionally violates only
    that display-count equality while still requiring every SMS-returned picture
    to decode. This wrapper catches exactly that legacy validator boundary and
    leaves every other RuntimeError fatal.

    P7 harvest must independently require decoded == 598 for the exact 600-
    coded-picture archive and must reconcile the P7 UDP frames_dropped counter
    with decoded - displayed.
"""

from __future__ import annotations

import network_stream_sequence_end_replay_server as generic


EXPECTED_DROP_VALIDATOR_ERROR = "decoded/displayed replay picture counts differ"


def main() -> int:
    try:
        return generic.main()
    except RuntimeError as exc:
        if str(exc) != EXPECTED_DROP_VALIDATOR_ERROR:
            raise

        print(
            "P7_DROP_REPLAY_LEGACY_DISPLAY_EQUALITY_BOUNDARY=EXPECTED",
            flush=True,
        )
        print(
            "P7_DROP_REPLAY_TRANSPORT_VALIDATION_REACHED_BEFORE_BOUNDARY=YES",
            flush=True,
        )
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
