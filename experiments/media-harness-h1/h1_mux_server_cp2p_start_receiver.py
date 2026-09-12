#!/usr/bin/env python3
"""
File synopsis:
    CP2P Pi runner for items #5/#6/#7: visible RFB + optional PCM, immutable
    MPEG START receive/validation, generation-scoped Pi-side RFB suppression,
    and exact START-derived capture-geometry preparation.

The runner preserves one physical PSTV socket and one H1 reader thread. It
substitutes a CP2P-only upstream RFB bridge while leaving the qualified CP2O
bridge implementation untouched. START first becomes immutable prepared state,
then installs one pending suppression generation, then prepares the exact
capture plan from that same immutable START.

Suppression becomes active immediately before the first NEW
FramebufferUpdateRequest after START, so any older request already outstanding
at calibration acceptance may finish normally while PS2 presentation remains
frozen in WAIT_FIRST_FRAME.

MPEG production is still dormant here. The FFmpeg command is prepared and saved
to evidence but no process or MPEG DATA is started; producer activation remains
item #8 and must consume this exact generation only after suppression is ready.
"""

from __future__ import annotations

import json

import h1_mux_server_cumulative39_rfb_pcm_bridge as cp2o
from h1_cp2p_capture_geometry import H1Cp2pCapturePlan, prepare_exact_capture_plan
from h1_cp2p_rfb_suppression import (
    H1Cp2pRfbPiBridge,
    open_cp2p_rfb_session_adapter,
)
from h1_cp2p_start_receiver import H1Cp2pStartReceiver

# CP2O imported its opener into module scope. Replace that one experiment-local
# construction seam before ParentSession.__init__ runs; CP2O source and its
# default bridge remain unchanged for every non-CP2P runner.
cp2o.open_rfb_session_adapter = open_cp2p_rfb_session_adapter

base = cp2o.base
_ParentSession = cp2o.H1Cumulative39RfbPcmSession


class H1Cp2pSuppressionStartReceiver(H1Cp2pStartReceiver):
    """START authority plus ordered suppression and exact capture preparation."""

    def __init__(self, session, suppression_bridge: H1Cp2pRfbPiBridge) -> None:
        super().__init__(session)
        self.suppression_bridge = suppression_bridge
        self.capture_plan: H1Cp2pCapturePlan | None = None

    def _handle_start_frame(self, frame: base.Frame) -> None:
        super()._handle_start_frame(frame)
        request = self.peek_prepared()
        if request is None:
            raise base.ProtocolError("validated MPEG START did not remain prepared")

        try:
            self.suppression_bridge.install_suppression(request)
            self.capture_plan = prepare_exact_capture_plan(
                request,
                desktop_width=self.suppression_bridge.desktop_width,
                desktop_height=self.suppression_bridge.desktop_height,
                display=self.session.display,
            )
        except BaseException:
            # The generation number remains stale/high-water in item-#5 state,
            # but no failed setup may remain prepared for later producer start.
            self.release_prepared_exact(request.generation)
            self.capture_plan = None
            raise

        suppression_evidence = {
            "generation": int(request.generation),
            "desktop_width": int(self.suppression_bridge.desktop_width),
            "desktop_height": int(self.suppression_bridge.desktop_height),
            "raw_pixel_budget_bytes": int(
                self.suppression_bridge.desktop_raw_byte_budget
            ),
            "suppression_x": int(request.suppression_x),
            "suppression_y": int(request.suppression_y),
            "suppression_width": int(request.suppression_width),
            "suppression_height": int(request.suppression_height),
            "state": "pending-next-new-rfb-update-request",
        }
        (self.session.evidence / "rfb_suppression_prepared.json").write_text(
            json.dumps(suppression_evidence, indent=2, sort_keys=True) + "\n"
        )
        print(
            "H1_CP2P_RFB_SUPPRESSION_PREPARED="
            + json.dumps(suppression_evidence, sort_keys=True),
            flush=True,
        )

        capture_evidence = self.capture_plan.to_dict()
        capture_evidence["state"] = "prepared-producer-dormant"
        (self.session.evidence / "mpeg_capture_prepared.json").write_text(
            json.dumps(capture_evidence, indent=2, sort_keys=True) + "\n"
        )
        print(
            "H1_CP2P_MPEG_CAPTURE_PREPARED="
            + json.dumps(capture_evidence, sort_keys=True),
            flush=True,
        )


class H1Cp2pStartReceiveSession(_ParentSession):
    """CP2O baseline plus START, Pi suppression, and exact capture preparation."""

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

        adapter = self.rfb_session_adapter
        if adapter is None or not isinstance(adapter.bridge, H1Cp2pRfbPiBridge):
            super().cleanup()
            raise base.ProtocolError("CP2P runner did not acquire suppression-capable RFB bridge")

        try:
            # Parent construction has already opened/registered the CP2P RFB
            # adapter. START wraps the same reader call chain outside channel 1.
            self.cp2p_start_receiver = H1Cp2pSuppressionStartReceiver(
                self,
                adapter.bridge,
            )
            self.cp2p_start_receiver.start()
        except BaseException:
            super().cleanup()
            raise

        print(
            "H1_CP2P_START_RECEIVER=ARMED "
            f"session_id={int(self.profile['session_id'])} "
            f"desktop={int(self.profile['desktop_width'])}x"
            f"{int(self.profile['desktop_height'])} "
            f"raw_budget={adapter.bridge.desktop_raw_byte_budget}",
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
