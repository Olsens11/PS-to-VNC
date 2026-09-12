#!/usr/bin/env python3
"""
File synopsis:
    CP2P item-#5 Pi runner: visible RFB + optional PCM plus receive/validation of
    the PS2's immutable MPEG START request on the existing PSTV reader path.

This intentionally remains a pre-MPEG-production checkpoint. It derives from the
CP2O visible-RFB/PCM runner, attaches the channel-4 START receiver only after the
existing RFB adapter has installed its channel-1 receive shim, and therefore
preserves one physical PSTV socket and one H1 reader thread.

A valid START is retained as prepared generation state and written to evidence.
No RFB suppression, exact-region capture mutation, ffmpeg launch, MPEG DATA, or
new CONFIG profile is introduced here; those belong to later scoreboard items.
"""

from __future__ import annotations

import h1_mux_server_cumulative39_rfb_pcm_bridge as cp2o
from h1_cp2p_start_receiver import H1Cp2pStartReceiver, open_cp2p_start_receiver

base = cp2o.base
_ParentSession = cp2o.H1Cumulative39RfbPcmSession


class H1Cp2pStartReceiveSession(_ParentSession):
    """CP2O transport/presentation baseline plus inert CP2P START preparation."""

    def __init__(
        self,
        sock,
        profile: dict[str, int],
        evidence,
        duration: float,
        display: str,
    ) -> None:
        super().__init__(sock, profile, evidence, duration, display)
        self.cp2p_start_receiver: H1Cp2pStartReceiver | None = None

        try:
            # Parent construction has already opened/registered the RFB adapter.
            # Wrapping receive_frame here therefore composes START outside that
            # existing channel-1 shim while staying on the same reader call path.
            self.cp2p_start_receiver = open_cp2p_start_receiver(self)
        except BaseException:
            super().cleanup()
            raise

        print(
            "H1_CP2P_START_RECEIVER=ARMED "
            f"session_id={int(self.profile['session_id'])} "
            f"desktop={int(self.profile['desktop_width'])}x"
            f"{int(self.profile['desktop_height'])}",
            flush=True,
        )

    def cleanup(self) -> None:
        try:
            super().cleanup()
        finally:
            if self.cp2p_start_receiver is not None:
                self.cp2p_start_receiver.stop()
                self.cp2p_start_receiver = None


base.H1Session = H1Cp2pStartReceiveSession


if __name__ == "__main__":
    raise SystemExit(base.main())
