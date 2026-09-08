#!/usr/bin/env python3
"""
File synopsis:
    Narrow H1 Pi-side apparatus correction for normal resident-session teardown.

The first H1 hardware run proved that the PS2 sends a complete SESSION_RESULT
and then closes the connection while returning to the resident wait loop. The
base server originally treated every EOF as a reader failure, including that
normal post-result close.

This wrapper changes only that classification seam:
    * EOF before a complete SESSION_RESULT remains an error.
    * EOF after a parsed SESSION_RESULT is the expected connection boundary.

No CONFIG, producer, scheduling, MPEG, audio, credit, metadata, validation, or
PS2 behavior is changed.
"""

from __future__ import annotations

import h1_mux_server as base


_ORIGINAL_CHECK_READER = base.H1Session.check_reader


def _check_reader_allow_post_result_eof(self: base.H1Session) -> None:
    error = self.reader_error

    if (
        isinstance(error, EOFError)
        and self.result is not None
        and self.result_event.is_set()
    ):
        print(
            "H1_POST_RESULT_SOCKET_CLOSE=PASS "
            "classification=normal_session_boundary",
            flush=True,
        )
        self.reader_error = None
        return

    _ORIGINAL_CHECK_READER(self)


base.H1Session.check_reader = _check_reader_allow_post_result_eof


if __name__ == "__main__":
    raise SystemExit(base.main())
