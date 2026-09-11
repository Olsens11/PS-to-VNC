# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-11 17:47 EDT / 21:47 UTC
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `dd6c5c8cc9aa9aae24976e8ef233aadbd487a9ce`
- Implementation/evidence HEAD before this handoff update: `a46c12c6a2728bb175f81e1b515f5eeac64f8a6d`
- The canonical handoff commit is the commit containing this file and is the branch HEAD immediately after this update.
- Repository connector evidence cannot report David's Pi worktree staged/unstaged/untracked state. Do not infer the Pi worktree is clean. No reset, clean, discard, overwrite, merge, or `src/` mutation was performed.

## Larger push objective

Produce a hardware-testable H1 ELF with qualified visible RFB interaction, PCM, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Required behavior remains: held `START+SELECT` temporary entry; accepted EDIT/CONTROLS/REVIEW UX and mapping; physical controller polling continues while ordinary mouse/controller/local-UI dispatch is suppressed; RFB protocol remains healthy while visual updates are frozen; thaw requires one full/nonincremental refresh; and the outer edge of the accepted outer matte is the permanent RFB-suppression perimeter inward once MPEG presentation owns the region.

## This session objective

Prepare the narrow experiment-local coordinator binding that can execute calibration foreground ownership against the current H1 input/RFB objects before ordinary `pstvnc_local_controller_route()` dispatch, without modifying `src/` or yet changing the live coordinator.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

Commits:

- `810d83e522de115b2c3ffae12cc4ec335786cc44` — add interaction-binding contract
- `ee3847918756e8ca459ebf1bc555a080bfff6b93` — implement interaction binding
- `dcf8a972b2e782d9239dbaef235706be21960022` — add binding host contract test
- `880318d06f2d4f752ebf673209629588925c516e` — add binding test to strict host suite
- `a46c12c6a2728bb175f81e1b515f5eeac64f8a6d` — de-duplicate the binding test source composition after the first CI failure

No `src/` file, qualified controller/input/local-UI code, live H1 interaction coordinator, RFB parser/session implementation, PS2 graphics owner, or permanent MPEG compositor was modified.

## Interaction-binding contract now represented

The binding owns no new domain semantics. It borrows pointers to the current H1 coordinator-owned input runtime, RFB session, published pointer coordinates/button state, pointer-message counter, and mouse-suspension fact. It adapts the existing calibration foreground operations to the current architecture:

1. `pstvnc_input_runtime_suspend_mouse_interpretation()`;
2. publish a neutral RFB pointer event if a click is currently published;
3. `pstvnc_input_runtime_rebase_suspended_mouse_state()` to the exact published coordinates with neutral buttons;
4. retain ownership through calibration release quarantine;
5. `pstvnc_input_runtime_resume_mouse_interpretation()` only after the calibration core proves full physical release.

The binding also owns one calibration runtime plus one calibration RFB-flow adapter and exposes the already-prepared generic `pstvnc_h1_rfb_flow_policy_t` for later use by `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()`.

Ordinary Select remains outside calibration unless START is simultaneously held; the host contract explicitly exercises that distinction.

## Tests/checks and exact evidence

### Preserved passing baseline from the previous tranche

Exact implementation HEAD `2baa3c98b51ff916d4c1d059da69366934acfd9e` passed workflow run `34646154111`, including the calibration core/adapter/foreground/RFB-gate/scheduler/flow/render/runtime host contracts and the existing H1 source/ownership checks. The pinned PS2 cumulative build also compiled the modified generic RFB runtime. That remains regression evidence only, not a calibration-enabled ELF.

### This run: preserved failures

First binding-suite attempt at HEAD `880318d06f2d4f752ebf673209629588925c516e`:

- workflow run `34650926359`
- host-preflight job `103432644159`
- `Test MPEG presentation calibration host contracts` failed.

Inspection found one definite build-composition defect in the new Makefile target: the initial `INTERACTION_BINDING_SOURCES` combined `RUNTIME_SOURCES` and `RFB_FLOW_SOURCES`, causing `h1_mpeg_calibration_rfb_gate.c` to be included twice. Commit `a46c12c6a2728bb175f81e1b515f5eeac64f8a6d` replaced that aggregate composition with an explicit unique source list.

Second attempt at exact HEAD `a46c12c6a2728bb175f81e1b515f5eeac64f8a6d`:

- workflow run `34650963508`
- host-preflight job `103432756951`
- `Test MPEG presentation calibration host contracts` still failed.

The GitHub repository connector exposes step status but not the step's stdout/stderr log body, so the exact remaining compiler/link/assertion text was not available in this session. Do not call the interaction-binding test proven. Preserve this as an unresolved failure rather than smoothing it over.

The same run's pinned `ps2-build` job `103432757320` was still in progress when this handoff was written and does not compile/link the new calibration binding into a candidate ELF anyway.

Evidence paths/IDs:

- implementation HEAD `a46c12c6a2728bb175f81e1b515f5eeac64f8a6d`
- failed first run `34650926359`, host job `103432644159`
- failed corrected-source-list run `34650963508`, host job `103432756951`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding.[ch]`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding_test.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff

No Pi execution, deployment, physical PS2 observation, or calibration-enabled ELF execution occurred.

## Repository/source reconciliation this run

At session start the continuity guide was read first. Remote branch authority independently reported `experiment/h1-rfb-mux-prep` at `dd6c5c8cc9aa9aae24976e8ef233aadbd487a9ce`, matching the preceding handoff.

The exact current `h1_interaction_coordinator.[ch]`, calibration runtime/foreground/adapter/flow contracts, `src/input/input_runtime.h`, and `src/rfb/rfb_session.h` were re-read before implementation.

Important live-source facts reconfirmed:

- normalized controller events enter `h1_interaction_service_controller_state()` immediately before `pstvnc_local_controller_route()`;
- existing OSK foreground entry already demonstrates the desired suspend -> neutralize -> rebase sequence;
- `h1_interaction_resume_desktop_mouse_if_ready()` would need to respect calibration ownership when the binding is actually inserted, so it cannot autonomously resume the mouse during calibration;
- the calibration-specific flow policy remains separate from the RFB parser/runtime and can be passed through the generic policy entry point;
- `src/` remains the reference model only and was not modified.

Remote repository APIs cannot expose David's Pi worktree dirty/staged/untracked state. Reconcile the Pi worktree before any build/deploy that could overwrite newer local MPEG/shared-compositor work.

## Current executable / ELF / PT_LOAD identity

No calibration-enabled PS2 executable exists yet.

- calibration candidate ELF SHA256: **none**
- calibration candidate ELF bytes: **none**
- calibration candidate PT_LOAD SHA256/bytes: **none**

## Proven vs unproven

### Proven from earlier green evidence

- portable calibration state/geometry and accepted UX mapping;
- normalized-controller adapter and START+SELECT level-chord ownership model;
- foreground ownership bridge and release quarantine contract;
- RFB visual/request gate, request scheduler, generic RFB flow-policy seam, and calibration flow adapter;
- render-plan/runtime contracts through the previous green implementation HEAD;
- existing generic RFB runtime changes compile in the pinned PS2 cumulative regression build;
- calibration remains experiment-local and `src/` untouched.

### Represented in code this run but not yet proven

- the new interaction binding's concrete adaptation of current H1 input/RFB/published-pointer state to the foreground callbacks;
- neutral-pointer publication accounting through the borrowed H1 pointer-message counter;
- ordinary Select pass-through at this new binding layer;
- binding-owned exposure of the prepared generic RFB flow policy.

### Still unproven / incomplete

- remaining interaction-binding host failure root cause and corrected green run;
- insertion of the binding into the live H1 interaction coordinator before ordinary local-controller routing;
- prevention of `h1_interaction_resume_desktop_mouse_if_ready()` from resuming during calibration ownership;
- live H1 main use of `run_with_flow_policy` and the binding's policy;
- native PS2 EDIT/CONTROLS/REVIEW raster/presentation adapter;
- accepted geometry handoff into MPEG presentation;
- permanent compositor enforcement of the outer-matte suppression perimeter;
- a PS2 ELF target actually linking the calibration integration/runtime/renderer objects;
- candidate ELF/PT_LOAD identity and physical PS2 qualification.

## Hardware-qualification boundary

Nothing from this session is hardware-qualified. Existing qualified controller/input ownership semantics were used as the reference model and left untouched. No calibration-enabled ELF has been built or run on PS2 hardware.

## Important design decisions / reconciliations

- Keep START+SELECT meaning above qualified controller/libpad owners.
- Keep calibration foreground mechanics in the experiment-local binding/runtime; do not add them to `src/ui/local_controller.*`.
- Reuse exact current input-runtime suspension/rebase/resume semantics instead of inventing a second mouse freeze mechanism.
- Neutralize only the successfully published remote button state and account for that RFB pointer message.
- Keep calibration RFB flow policy separate from parser/socket ownership.
- Do not wire a failing host contract into the live coordinator merely to make PT_LOAD progress.
- Native calibration rasterization remains separate from permanent MPEG composition.
- Never merge/align this experiment into `src/` without David's explicit later permission.

## Current overall push status / milestone

Milestone: **the coordinator-facing ownership adapter now exists as a first-class experiment-local module, but its new strict host contract is red and must be repaired before live coordinator insertion.**

The path to the hardware-testable ELF remains: obtain green interaction-binding evidence; insert the binding before ordinary local-controller routing and connect its flow policy to the RFB run; add native PS2 calibration rasterization; connect committed geometry to MPEG presentation and outer-matte suppression; then build/check/hash the candidate ELF/PT_LOAD before physical qualification.

## Single best next action

Diagnose and repair the exact remaining `h1_mpeg_calibration_interaction_binding_test` failure at current branch authority before touching the live coordinator. Use repository CI/check evidence or an execution-capable checkout to capture the compiler/link/assertion text, make the smallest correction, and require the entire strict calibration host suite to return green. Once green, the following tranche should insert one `pstvnc_h1_mpeg_calibration_interaction_binding_t` into the experiment-local H1 coordinator, call it immediately before `pstvnc_local_controller_route()`, prevent ordinary desktop-mouse auto-resume while calibration owns suspension, and pass its policy to `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()`—still without modifying `src/` or beginning PS2 calibration rasterization.
