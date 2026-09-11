# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-11 18:53 EDT / 22:53 UTC
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `d5db0e324fd229332645d87be1ed0bbc160801b6`
- Implementation/evidence HEAD before this handoff update: `908250ae93a8a0801189b0113360adc0e21ba08a`
- The canonical handoff commit containing this file is the branch HEAD immediately after this update.
- Remote repository state is clean by definition, but the repository connector cannot inspect David's Pi worktree staged/unstaged/untracked state. Treat Pi-local dirty state as **unknown**. No reset, clean, discard, overwrite, merge, or `src/` mutation was performed.

## Larger push objective

Produce a hardware-testable H1 ELF with qualified visible RFB interaction, PCM, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Required behavior remains: held `START+SELECT` temporary entry; accepted EDIT/CONTROLS/REVIEW UX and mapping; physical controller polling continues while ordinary mouse/controller/local-UI dispatch is suppressed; RFB protocol remains healthy while visual updates are frozen; thaw requires one full/nonincremental refresh; and the outer edge of the accepted outer matte is the permanent RFB-suppression perimeter inward once MPEG presentation owns the region.

## This session objective

Diagnose and repair the remaining interaction-binding host failure with exact evidence, require the full strict calibration host suite to return green, and stop before live coordinator insertion so a red contract is never integrated.

## What changed this run

Added:

- `.github/workflows/mpeg-calibration-host-diagnostic.yml` — runs the exact strict calibration host suite, tees stdout/stderr to `build/evidence/mpeg-calibration-host.log`, and preserves it as an Actions artifact even on failure.
- `experiments/media-harness-h1/mpeg_presentation_calibration/host_stubs/libpad.h` — host-only PS2SDK type shim containing only the `struct padButtonStatus` surface needed by the real `src/input/pad.h` header graph during ordinary Ubuntu host compilation.

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host` — only the interaction-binding host target now places `-Ihost_stubs` before the real `src/input` include directory.

Commits:

- `68234589a96a316eac7d66ab73a8a14132216672` — preserve exact MPEG calibration host diagnostic log
- `58814d024b34bc50b78f3faf75af24840bd149a1` — add host-only libpad type shim
- `908250ae93a8a0801189b0113360adc0e21ba08a` — use the host-only shim for the binding contract

No `src/` file, qualified controller/input/local-UI code, live H1 interaction coordinator, RFB parser/session implementation, PS2 graphics owner, or permanent MPEG compositor was modified.

## Root cause and reconciliation

The previous red interaction-binding contract was not a calibration logic/assertion defect. Exact preserved diagnostic output showed host compilation stopped transitively at:

`src/input/input_runtime.h -> src/input/pad.h -> <libpad.h>`

with `fatal error: libpad.h: No such file or directory` on ordinary Ubuntu CI. The interaction binding intentionally consumes the real current `pstvnc_input_runtime_t` interface, so replacing or weakening that production header contract would have been the wrong fix.

The repair is deliberately host-only. The shim supplies only the libpad structure declaration needed to parse the real source header graph; it emulates no libpad calls, pad behavior, controller semantics, or input-runtime behavior. Pinned PS2 builds continue to use the real PS2SDK `<libpad.h>` because their build paths do not add `host_stubs`.

## Tests/checks and exact evidence

### Failure captured exactly

Diagnostic run at `68234589a96a316eac7d66ab73a8a14132216672`:

- workflow run `34655663750` — **FAILURE**
- artifact `10285364214`, `mpeg-calibration-host-diagnostic`
- artifact digest `sha256:d117653ed678b0dd6ccab219d0aed9c4b96ef9f858231277209e49990decd0c3`
- exact log identified missing host `<libpad.h>` while compiling `h1_mpeg_calibration_interaction_binding_test`; earlier calibration targets had already built.

This preserves the failure rather than replacing it with a post-hoc description.

### Corrected strict host suite

Exact implementation HEAD `908250ae93a8a0801189b0113360adc0e21ba08a`:

- diagnostic workflow run `34655729670` — **SUCCESS**
- artifact `10285725159`, `mpeg-calibration-host-diagnostic`
- artifact digest `sha256:c4f1119f72f8395c46a5b34ea668e3b595ab6bda74fc9a27a3c4bb7c81d0b4e3`
- strict flags: `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`
- exact PASS markers:
  - `MPEG_CALIBRATION_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_RFB_SCHEDULE_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_RFB_FLOW_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_RENDER_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_RUNTIME_HOST_TEST=PASS`
  - `MPEG_CALIBRATION_INTERACTION_BINDING_HOST_TEST=PASS`

The ordinary branch workflow independently confirmed the same result:

- workflow run `34655729709` — **SUCCESS**
- host-preflight job `103447595405` — **SUCCESS**; `Test MPEG presentation calibration host contracts` succeeded, followed by all existing CP2N/RFB/mux/operator checks.
- ps2-build job `103447595609` — **SUCCESS**; cumulative H1 RFB-prep and CP2K/CP2L/CP2M/CP2N regression candidates compiled under the pinned PS2 toolchain and their existing artifacts were preserved.

Repository reconciliation also corrected one stale fact from the prior handoff: prior run `34650963508` ps2-build job `103432757320` ultimately completed **SUCCESS**. It was merely still running when that handoff was written.

These PS2 jobs remain regression-build evidence only. The current candidate makefiles do not yet link the calibration binding/runtime/renderer into a calibration-enabled ELF.

## Evidence paths / artifacts

- `.github/workflows/mpeg-calibration-host-diagnostic.yml`
- `experiments/media-harness-h1/mpeg_presentation_calibration/host_stubs/libpad.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding.[ch]`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding_test.c`
- failed diagnostic run/artifact: `34655663750` / `10285364214`
- green diagnostic run/artifact: `34655729670` / `10285725159`
- green full branch workflow: `34655729709`
- green host job: `103447595405`
- green pinned-PS2 regression job: `103447595609`
- this handoff

No Pi execution, deployment, physical PS2 observation, or calibration-enabled ELF execution occurred.

## Current executable / ELF / PT_LOAD identity

No calibration-enabled PS2 executable exists yet.

- calibration candidate ELF SHA256: **none**
- calibration candidate ELF bytes: **none**
- calibration candidate PT_LOAD SHA256/bytes: **none**

Existing CP2N/CP2O and other workflow ELFs are regression/reference artifacts only and must not be described as the calibration candidate.

## Proven vs unproven

### Proven through current green evidence

- portable calibration state/geometry and accepted EDIT/CONTROLS/REVIEW UX mapping;
- normalized-controller adapter and held START+SELECT entry ownership model;
- foreground suspend -> neutralize -> rebase -> release-quarantine -> resume contract;
- RFB visual/request gate, request scheduler, generic flow-policy seam, and calibration flow adapter;
- render-plan/runtime contracts;
- concrete interaction binding to the current H1-owned input runtime, RFB session, published pointer state, pointer-message counter, and suspension fact;
- neutral-pointer publication accounting, ordinary Select pass-through, full-release resume, and binding-owned RFB policy exposure;
- all nine strict calibration host contracts green at `908250ae...`;
- existing non-calibration H1 candidates still compile under the pinned PS2 toolchain;
- calibration remains experiment-local and `src/` untouched.

### Still unproven / incomplete

- insertion of the binding into the live experiment-local H1 interaction coordinator immediately before ordinary `pstvnc_local_controller_route()` dispatch;
- preventing `h1_interaction_resume_desktop_mouse_if_ready()` from autonomously resuming while calibration owns suspension;
- live H1 use of `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()` with the binding's policy;
- PS2 compilation of the actual coordinator-linked calibration objects;
- native PS2 EDIT/CONTROLS/REVIEW raster/presentation adapter;
- accepted geometry handoff into MPEG presentation;
- permanent compositor enforcement of the accepted outer-matte suppression perimeter;
- a PS2 ELF target actually linking calibration integration/runtime/renderer/compositor ownership;
- candidate ELF/PT_LOAD identity and physical PS2 qualification.

## Hardware-qualification boundary

Nothing added or repaired in this session is hardware-qualified. Existing qualified controller/input semantics remain the reference model and were not modified. The green pinned PS2 job proves only that the existing regression candidates still build; it does not prove a calibration-enabled executable because those makefiles do not yet link calibration.

## Important design decisions / reconciliations

- Keep START+SELECT meaning above qualified controller/libpad owners.
- Keep calibration foreground mechanics in the experiment-local binding/runtime; do not add them to `src/ui/local_controller.*` or other `src/` owners.
- Reuse the exact current input-runtime suspension/rebase/resume interfaces.
- Keep RFB flow policy separate from parser/socket ownership.
- Use a host-only type shim to make real source interfaces testable off-target; never substitute that shim into a PS2 build or emulate libpad semantics in the calibration module.
- Do not begin live coordinator insertion until the binding contract is green; that gate is now satisfied.
- Native calibration rasterization remains separate from permanent MPEG composition.
- Never merge/align this experiment into `src/` without David's explicit later permission.

## Current overall push status / milestone

Milestone: **the coordinator-facing calibration ownership binding is now strict-host proven against the current source interfaces, and the branch's ordinary host plus pinned-PS2 regression workflows are green. The push is ready for experiment-local live coordinator insertion.**

The remaining path to a hardware-testable ELF is: bind calibration ownership and RFB flow into the experiment-local H1 coordinator; prove that integration under host/pinned PS2 builds; add native PS2 calibration rasterization; connect accepted geometry to MPEG presentation and permanent outer-matte suppression; then build/check/hash the calibration candidate ELF/PT_LOAD before physical qualification.

## Single best next action

Insert one `pstvnc_h1_mpeg_calibration_interaction_binding_t` into the experiment-local H1 interaction coordinator and service it immediately before `pstvnc_local_controller_route()`. While its calibration runtime owns foreground, prevent `h1_interaction_resume_desktop_mouse_if_ready()` from independently resuming mouse interpretation. Pass `pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy()` into `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()` at the current H1 main/RFB run site. Add focused host/source-ownership checks for that wiring and require both the strict host suite and pinned PS2 compilation to remain green before beginning the separate native calibration rasterization tranche. Do not modify `src/`.
