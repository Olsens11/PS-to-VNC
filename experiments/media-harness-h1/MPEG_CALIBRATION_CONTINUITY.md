# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-11 16:50 EDT / 20:50 UTC
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `c2f855e88e338e9e7096c695f72999511d8a9915`
- Implementation/evidence HEAD before this handoff update: `2baa3c98b51ff916d4c1d059da69366934acfd9e`
- The canonical handoff commit is the commit containing this file and is the branch HEAD immediately after this update.
- Repository connector evidence cannot report David's Pi worktree staged/unstaged/untracked state. Do not infer the Pi worktree is clean. No reset, clean, discard, overwrite, merge, or `src/` mutation was performed.

## Larger push objective

Produce a hardware-testable H1 ELF with qualified visible RFB interaction, PCM, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Calibration requirements remain: held `START+SELECT` temporary entry; accepted EDIT/CONTROLS/REVIEW UX and mapping; physical controller polling continues but ordinary mouse/controller/local-UI dispatch is suppressed while calibration owns foreground; RFB protocol remains healthy while new RFB visual presentation is frozen; request scheduling stops at complete-server-message/IDLE boundaries; thaw requires one full/nonincremental refresh; and once MPEG presentation owns an accepted region, the outer edge of the outer matte is the permanent RFB suppression perimeter inward.

## This session objective

Integrate the already host-proven HOLD / INCREMENTAL / FULL scheduler into the experiment-local H1 RFB session runtime through an optional generic flow-policy seam, without making the RFB runtime calibration-aware or changing the behavior of existing callers.

## What changed this run

Modified experiment-local RFB runtime:

- `experiments/media-harness-h1/h1_rfb_session_runtime.h`
- `experiments/media-harness-h1/h1_rfb_session_runtime.c`

Added calibration-specific adapter/test:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_flow.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_flow.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_flow_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

Contents-API commits in this tranche:

- `63e29dea91b6d9734dd36cd875290e5285e30815` — `feat(h1): add optional RFB flow policy seam`
- `e6fec996464e5b87c4f3601eb74823c94a23f279` — `feat(h1): honor optional RFB flow policy at safe boundaries`
- `ecab04e811ed099cb53aec7a289e6bb031716c71` — `feat(h1): adapt calibration scheduler to RFB flow policy`
- `c04dd0fb6c2d59d48a6113791e45a4327f58e973` — `feat(h1): implement calibration RFB flow adapter`
- `ec048bbbbe792aedfb5131526843ce5e519c9bdd` — `refactor(h1): keep RFB flow adapter on gate contract`
- `d9917abf6c73e76de52f71d9398e9167e691aefa` — `test(h1): cover calibration RFB flow adapter`
- `6640d4fe5c85edfe50c5ff7b26b040f633283f1f` — `test(h1): add calibration RFB flow adapter contract`
- `2baa3c98b51ff916d4c1d059da69366934acfd9e` — `fix(h1): use current src topology for RFB flow test`

No `src/` file, qualified controller/input/local-UI implementation, parser, permanent MPEG compositor, or platform graphics implementation was modified.

## RFB flow contract now represented in live experiment code

The H1 RFB runtime remains generic. Existing `run`, `run_with_presenter`, and `run_with_presenter_and_service` entry points pass no flow policy and retain their prior request/presentation cadence.

A new optional `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()` accepts a narrow generic policy with four facts/actions:

- choose `HOLD`, `INCREMENTAL`, or `FULL` at an already-safe boundary;
- acknowledge a successfully transmitted request;
- acknowledge a complete framebuffer-update response;
- decide whether a completed dirty framebuffer may be visually published.

With a policy present:

- `HOLD` sends no framebuffer request;
- `INCREMENTAL` maps to `pstvnc_rfb_session_request_update(..., 1)`;
- `FULL` maps to `pstvnc_rfb_session_request_update(..., 0)`;
- IDLE evaluates policy only for policy-enabled callers, so the scheduler can retain an outstanding request or issue the first post-thaw full refresh without altering default caller behavior;
- a complete update clears request ownership before visual-publication policy is consulted;
- a response requested before calibration entry can therefore be fully parsed/applied to authoritative framebuffer state while its visual publication is suppressed;
- application service and quiesce ordering remain at the existing complete-message/IDLE boundaries.

The new `h1_mpeg_calibration_rfb_flow.*` adapter connects this generic seam to the existing calibration RFB gate and request scheduler. The RFB runtime itself contains no calibration-specific state or START+SELECT logic.

## Tests/checks and exact evidence

### Preserved failure evidence

The first final-head attempt exposed a real branch-topology mistake rather than a behavioral failure:

- workflow run `34646069635`
- host-preflight job `103417044393`
- failing step: `Test MPEG presentation calibration host contracts`
- exact compiler failure: `../h1_rfb_session_runtime.h:19:10: fatal error: framebuffer.h: No such file or directory`

Cause: the first host-test include path assumed a flat `src/`; repository inspection confirmed this branch uses the current layered topology, including `src/framebuffer/framebuffer.h`, `src/rfb/...`, and `src/input/...`.

Fix: commit `2baa3c98b51ff916d4c1d059da69366934acfd9e` changed the test include paths only to the current branch topology. No `src/` file was changed.

### Passing repository-authoritative evidence

GitHub Actions run `34646154111` for exact implementation HEAD `2baa3c98b51ff916d4c1d059da69366934acfd9e`:

- host-preflight job `103417325493` completed **success**;
- `Test MPEG presentation calibration host contracts` completed **success**;
- CP2N real-module interaction contract completed **success**;
- CONFIG, RFB mux ownership, headless coordinator, CP2K presentation, CP2L input, CP2M keyboard, keyboard chord, cumulative activation, logical RFB channel/credit, Pi bridge, reader integration, cumulative bridge, operator profile, and operator-surface checks all completed **success**.

The same run's pinned `ps2-build` job `103417325520` successfully completed its cumulative H1 RFB-prep/CP2K/CP2L/CP2M/CP2N compile step against the modified generic RFB runtime. Artifact-preservation steps were still finishing when this handoff was written. This is useful regression compile evidence for the generic runtime change, but it is **not** a calibration-enabled ELF build because the new calibration flow adapter/runtime is not yet bound into an ELF target.

Evidence paths/IDs:

- implementation/evidence HEAD `2baa3c98b51ff916d4c1d059da69366934acfd9e`
- passing workflow run `34646154111`
- passing host-preflight job `103417325493`
- pinned ps2-build job `103417325520`
- preserved failing workflow run `34646069635`
- preserved failing host-preflight job `103417044393`
- `experiments/media-harness-h1/h1_rfb_session_runtime.[ch]`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_flow.*`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff

No Pi execution, deployment, physical PS2 observation, or calibration-enabled ELF execution occurred.

## Repository/source reconciliation this run

At session start the continuity guide was read first and branch `experiment/h1-rfb-mux-prep` was independently verified at exact pushed HEAD `c2f855e88e338e9e7096c695f72999511d8a9915`.

The live experiment-local `h1_rfb_session_runtime.[ch]`, calibration gate/scheduler/runtime, and branch `src/` topology were re-read before and during implementation.

Important reconciliations:

- request ownership and visual-presentation ownership remain deliberately distinct;
- default RFB entry points must not start issuing requests from IDLE merely because the policy-capable path can do so;
- the policy-enabled IDLE path is safe because its scheduler explicitly tracks whether a request is outstanding;
- complete response accounting occurs before visual publication so protocol state remains healthy during calibration freeze;
- this branch's clean-source model is directory-layered (`src/framebuffer`, `src/rfb`, `src/input`), and host test paths now reflect that current authority;
- the pushed-branch/Pi-local discrepancy remains important: pushed GitHub state does not prove later Pi-local MPEG/shared-compositor files are absent. Reconcile David's Pi worktree before any deploy/build that could overwrite newer local work.

## Current executable / ELF / PT_LOAD identity

No calibration-enabled PS2 executable exists yet.

- calibration candidate ELF SHA256: **none**
- calibration candidate ELF bytes: **none**
- calibration candidate PT_LOAD SHA256/bytes: **none**

The pinned PS2 job compiled existing RFB-prep through CP2N targets against the generic runtime change, but those are not calibration candidates.

## Proven vs unproven

### Proven now

- the calibration policy stack through scheduler remains strict-host-test passing;
- the generic H1 RFB runtime now has a host-regression-checked optional request/presentation flow seam while preserving existing public entry points;
- the calibration-specific flow adapter is strict-host-test passing;
- HOLD / incremental / full request decisions map cleanly onto generic RFB runtime policy;
- an in-flight response can clear request ownership while visual presentation is denied;
- thaw still yields exactly one full-refresh decision before incremental cadence resumes;
- the modified generic RFB runtime passes existing source/ownership contracts and compiles in the existing pinned PS2 cumulative build step;
- calibration remains experiment-local and `src/` remains untouched;
- permanent RFB suppression from the outer edge of the outer matte remains a separate backend/compositor contract.

### Still unproven

- actual H1 main/coordinator use of `run_with_flow_policy`;
- START+SELECT interception in the live experiment coordinator immediately before ordinary local-controller routing;
- live qualified suspend/neutralize/rebase/resume callbacks during calibration;
- live freeze/thaw behavior against a real VNC server and in-flight response;
- native PS2 calibration EDIT/CONTROLS/REVIEW raster/presentation adapter;
- accepted geometry handoff into MPEG presentation;
- permanent compositor enforcement of the outer-matte RFB suppression perimeter;
- a PS2 ELF target that actually links the calibration runtime/flow/renderer objects;
- calibration candidate ELF/PT_LOAD identity;
- physical PS2 qualification.

## Important design decisions / reconciliation

- Keep the RFB session runtime generic; calibration policy belongs in the experiment-local adapter.
- Do not put START+SELECT into `src/ui/local_controller.*` or qualified pad code.
- Do not pause/disconnect the RFB parser/socket as calibration's freeze mechanism.
- Do not cancel an already-outstanding RFB request on calibration entry.
- `update_complete` must discharge protocol ownership before presentation permission is evaluated.
- Policy-enabled IDLE may evaluate request scheduling; legacy/default paths retain prior IDLE behavior.
- Native calibration rendering remains separate from permanent MPEG composition.
- Never align/merge into `src/` without David's explicit later permission.

## Current overall push status / milestone

Milestone reached: **the previously host-proven calibration RFB scheduler is now connected through a generic, regression-checked experiment-local RFB session flow-policy seam, including in-flight-response visual suppression and one-shot post-thaw full-refresh semantics.**

The remaining path to a hardware-testable ELF is now dominated by live experiment composition rather than request-policy design: bind calibration ownership into the H1 coordinator/main path, implement native PS2 calibration presentation, connect accepted geometry to MPEG presentation and outer-matte compositor suppression, then create an ELF target that actually links those calibration objects and record pinned build/checker/ELF/PT_LOAD evidence before physical qualification.

## Single best next action

Bind the host-proven calibration runtime and RFB flow adapter into the current experiment-local H1 coordinator/main composition without touching `src/`:

1. instantiate one calibration runtime plus `pstvnc_h1_mpeg_calibration_rfb_flow_t` alongside existing H1 interaction state;
2. service normalized controller state immediately before ordinary `pstvnc_local_controller_route()`, so held START+SELECT can consume calibration-owned observations before Select reaches OSK or mouse/local UI;
3. implement foreground callbacks using the already-existing experiment-local access to the qualified input-runtime suspend -> neutralize pointer -> rebase -> release-quarantine -> resume sequence;
4. pass the prepared generic policy to `pstvnc_h1_rfb_session_runtime_run_with_flow_policy()`;
5. add a source/integration contract test proving ordinary controller dispatch is suppressed while calibration owns foreground and that legacy non-calibration RFB entry points remain available;
6. add/update a pinned PS2 build target that actually compiles/links these calibration integration objects before beginning native calibration rasterization.

Do not fold PS2 rasterization, MPEG decoding, or permanent compositor clipping into that coordinator-binding tranche. Before any Pi build/deploy, reconcile David's Pi worktree because its local dirty/unpushed state is not visible through this repository connection.
