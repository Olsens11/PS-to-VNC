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

Compound START preparation is fail-closed. If a later preparation step fails
after suppression was installed, the exact suppression generation and usable
prepared START state are rolled back together while the generation high-water
remains stale/rejected.
"""

from __future__ import annotations

import json
from pathlib import Path

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

    def _rollback_compound_preparation(
        self,
        generation: int,
        *,
        suppression_installed: bool,
    ) -> None:
        """Remove all usable state for one failed START while preserving high-water."""

        rollback_error: BaseException | None = None

        if suppression_installed:
            try:
                self.suppression_bridge.retire_suppression_exact(generation)
            except BaseException as exc:
                rollback_error = exc

        prepared = self.peek_prepared()
        if prepared is not None and prepared.generation == int(generation):
            try:
                self.release_prepared_exact(generation)
            except BaseException as exc:
                if rollback_error is None:
                    rollback_error = exc

        self.capture_plan = None

        # These files are evidence of usable prepared state. A failed compound
        # transaction must not leave them implying that suppression/capture can
        # later be activated. Failure to remove evidence is itself fail-closed.
        for name in (
            "mpeg_start_prepared.json",
            "rfb_suppression_prepared.json",
            "mpeg_capture_prepared.json",
        ):
            try:
                (Path(self.session.evidence) / name).unlink(missing_ok=True)
            except BaseException as exc:
                if rollback_error is None:
                    rollback_error = exc

        if rollback_error is not None:
            raise base.ProtocolError(
                f"CP2P START generation {int(generation)} rollback failed"
            ) from rollback_error

    def _handle_start_frame(self, frame: base.Frame) -> None:
        try:
            super()._handle_start_frame(frame)
        except BaseException:
            prepared = self.peek_prepared()
            if prepared is not None:
                self._rollback_compound_preparation(
                    prepared.generation,
                    suppression_installed=False,
                )
            raise

        request = self.peek_prepared()
        if request is None:
            raise base.ProtocolError("validated MPEG START did not remain prepared")

        suppression_installed = False
        try:
            self.suppression_bridge.install_suppression(request)
            suppression_installed = True

            self.capture_plan = prepare_exact_capture_plan(
                request,
                desktop_width=self.suppression_bridge.desktop_width,
                desktop_height=self.suppression_bridge.desktop_height,
                display=self.session.display,
            )

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
            (Path(self.session.evidence) / "rfb_suppression_prepared.json").write_text(
                json.dumps(suppression_evidence, indent=2, sort_keys=True) + "\n"
            )

            capture_evidence = self.capture_plan.to_dict()
            capture_evidence["state"] = "prepared-producer-dormant"
            (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                json.dumps(capture_evidence, indent=2, sort_keys=True) + "\n"
            )
        except BaseException:
            self._rollback_compound_preparation(
                request.generation,
                suppression_installed=suppression_installed,
            )
            raise

        print(
            "H1_CP2P_RFB_SUPPRESSION_PREPARED="
            + json.dumps(suppression_evidence, sort_keys=True),
            flush=True,
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
