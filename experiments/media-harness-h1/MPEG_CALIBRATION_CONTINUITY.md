# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 10:44 EDT run
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `d6adaae4e59ad68b1ca4bdb3dbfb0f5223b02e4a`
- Calibration-core implementation commit: `6b255836ac15f069f432e82a9697de24555f5447`
- Controller-adapter tranche commits already present:
  - `0d6bb3ef07af4aac10586c1ee743ae1a44c577a3` — adapter header
  - `686de8cf8d31329970a54e371eaa98c1c98b688f` — adapter implementation
  - `b8af43c087df4f002b7f24c73fc28638e5a28b31` — adapter host test
  - `514dacbc52f66c7bff43c6a94cf7b851dd139610` — adapter host target
- Foreground-ownership tranche commits in this run:
  - `afab6a7bcc6a17846faef9c1ad0a2704865c55ae` — foreground bridge interface
  - `64af87a2326690eb8dcea9a7ec9855cfde2d1441` — foreground bridge implementation
  - `9589d1fdeaca6497765fa7a33dd88ac5b0fb3e60` — foreground ownership host test
  - `8c2d82bf5e3c27e185607bb4c77751ad05bfd98a` — foreground host target
- Handoff/docs commit: the commit containing this file; verify branch HEAD after fetch.
- Repository-visible pushed state was inspected before writes. Local Pi staged/unstaged/untracked state remains **not observable through the GitHub connector**; do not infer a clean Pi worktree from the pushed branch. Before any Pi build/deploy, reconcile the actual Pi worktree and preserve any newer local CP2P work.

## Larger push objective

Prepare a hardware-testable H1 ELF combining the already-qualified visible-RFB interaction path (mouse + keyboard/OSK/local UI), PCM audio, and MPEG presentation through a native PS2 calibration flow. Calibration stays experiment-local; the permanent MPEG compositor/presentation backend remains separate.

Temporary calibration entry gesture: held `START+SELECT`.

During calibration:

- keep the PSTV/RFB protocol session healthy;
- finish any RFB server message already in progress to the existing complete-message scheduling boundary;
- issue no new framebuffer-update requests;
- present no new RFB visual updates;
- retain the last authoritative desktop image;
- suppress ordinary mouse/controller/local-UI dispatch while physical controller polling continues;
- render calibration locally.

After accepted MPEG ownership starts, RFB may resume elsewhere, but from the **outer edge of the outer matte inward** RFB presentation is forbidden.

## This session objective

Advance from the normalized-controller adapter to an experiment-local foreground-ownership bridge that represents the already-qualified input-runtime ownership sequence and RFB-freeze lifetime without modifying `src/`, qualified controller code, RFB parser/framing, or permanent MPEG composition.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

No `src/` file was modified. No qualified controller/local-UI/input-runtime/RFB implementation was modified.

## Repository/source evidence reconciled this run

The exact pushed H1 coordinator was re-read before implementation.

Relevant current authority:

- `h1_interaction_service_controller_state()` routes normalized `pstvnc_controller_state_t` observations through `pstvnc_local_controller_route()`; this remains the narrow interception seam for the next wiring patch.
- `h1_interaction_open_osk()` already establishes the qualified higher-level ownership order:
  1. `pstvnc_input_runtime_suspend_mouse_interpretation()`;
  2. neutralize any successfully published remote click state;
  3. `pstvnc_input_runtime_rebase_suspended_mouse_state()`.
- `h1_interaction_resume_desktop_mouse_if_ready()` resumes the input runtime only after the higher-level foreground owner is ready.
- the H1 coordinator service callback is invoked at the RFB safe-server-message boundary, which remains the correct place to honor framebuffer-request/visual quiescence.
- `src/input/input_runtime.h` explicitly states that mouse suspension keeps physical libpad polling live, discards pre-boundary semantic input, requires published-pointer rebase, and must not resume until the higher-level foreground owner proves its physical-release quarantine complete.

No contradiction was found between the continuity handoff and current pushed coordinator/input-runtime authority.

## Foreground bridge contract now represented in code

`h1_mpeg_calibration_foreground.*` composes the existing adapter with four narrow higher-level operations supplied by the H1 coordinator:

1. suspend mouse interpretation;
2. neutralize the last successfully published remote pointer-button state;
3. rebase the suspended local mouse interpreter to the exact published pointer state;
4. resume mouse interpretation only after calibration release quarantine completes.

The bridge deliberately does **not** own libpad, RFB framing, framebuffer requests, GS rendering, local UI, MPEG decode, or permanent composition.

On calibration entry it establishes `rfb_visuals_frozen` before invoking the ownership callbacks, then performs `suspend -> neutralize -> rebase` in the qualified order.

On accept/cancel it keeps ordinary input and RFB visual ownership suppressed until the portable calibration core reports `release_quarantine_complete`. Only then may the bridge call resume and clear the RFB-freeze fact.

Callback failure is fail-closed: any suspension/freeze facts already established remain asserted rather than silently returning ordinary desktop ownership.

## Tests/checks and exact evidence boundary

### Previously proven calibration-core evidence

Previously recorded strict host execution:

`MPEG_CALIBRATION_HOST_TEST=PASS`

### Adapter test

The adapter test source and Makefile target remain committed, but this repository-only run still cannot execute the host compiler. Therefore `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS` is **not claimed** here.

### New foreground host test

`h1_mpeg_calibration_foreground_test.c` now covers source-level contracts for:

- ordinary Select remaining outside calibration ownership;
- held START+SELECT entering calibration even when only Select has a fresh edge;
- exact entry callback order `suspend -> neutralize -> rebase`;
- controller consumption and RFB freeze on the entry sample;
- cancel remaining frozen/consumed while Circle is physically held;
- full physical release being the sole resume boundary;
- resume occurring after release quarantine completion;
- entry-transition failure leaving suspension/RFB freeze asserted rather than failing open.

`Makefile.host` now builds/runs core, adapter, and foreground tests with:

`-std=c99 -O2 -Wall -Wextra -Werror -pedantic`

**Execution boundary:** no compiler or Pi execution occurred in this repository-only run, so `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS` is **not claimed yet**.

## Evidence / artifact paths

Repository-visible evidence:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground_test.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.{h,c}`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff file

No new ELF, PT_LOAD manifest, execution log, deployment, or physical observation was produced in this run.

## Current ELF / PT_LOAD identity

No new PS2 ELF was built in this run.

- new candidate ELF SHA256: **none yet**
- new candidate ELF bytes: **none yet**
- new candidate PT_LOAD SHA256/bytes: **none yet**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later recorded evidence. Calibration work remains source/host-contract evidence until a new PT_LOAD-changing candidate is built and physically qualified.

## Proven vs unproven

### Proven from repository/source evidence

- calibration remains experiment-local;
- no `src/` or qualified controller/input/local-UI/RFB implementation was changed;
- START+SELECT meaning remains outside qualified `local_controller`;
- normalized controller facts feed the calibration adapter without PS2SDK button vocabulary;
- a separate foreground-ownership bridge now models the exact qualified input suspension/rebase/resume responsibilities;
- ordinary Select remains pass-through when START is not simultaneously held;
- calibration ownership includes release quarantine;
- RFB visual freeze is modeled across the same ownership lifetime and fails closed on transition failure;
- permanent outer-matte suppression geometry remains separate from permanent MPEG/RFB composition.

### Still unproven

- compilation/execution of adapter and foreground host tests on an execution-capable checkout;
- direct call-site wiring immediately before `pstvnc_local_controller_route()`;
- binding the bridge callbacks to the actual coordinator's existing suspend/neutralize/rebase/resume functions;
- propagation of the bridge `freeze_rfb_visuals` fact into actual framebuffer-request generation at the existing safe-server-message boundary;
- native PS2 EDIT/CONTROLS/REVIEW renderer;
- atomic publication of accepted geometry to MPEG presentation;
- permanent compositor enforcement of the outer-matte suppression rectangle;
- combined RFB + mouse + OSK/keyboard + PCM + calibration + MPEG build/link;
- candidate ELF/PT_LOAD identity;
- physical PS2 behavior or hardware qualification.

## Important design decisions / reconciliation

- Do not put START+SELECT into `src/ui/local_controller.*`; temporary calibration meaning stays in experiment composition.
- Entry uses authoritative `buttons_down`, not paired press edges.
- The foreground bridge mirrors the existing qualified OSK ownership sequence rather than creating a new input-runtime mechanism.
- RFB freeze is an application/coordinator ownership fact; the bridge must not pause inside RFB parser/framing internals.
- Historical notes allowing continued RFB under MPEG remain historical. Current experiment policy is:
  1. freeze new RFB visual work throughout calibration while protocol state remains healthy;
  2. after accepted MPEG ownership, permit RFB elsewhere;
  3. forbid RFB presentation from the outer edge of the outer matte inward.
- Calibration and permanent MPEG presentation/composition remain separate owners.

## Current overall push status / milestone

Milestone reached: **portable calibration core + normalized-controller adapter + explicit foreground-ownership bridge are represented on the experimental branch.**

This removes ambiguity about how calibration should reuse the already-qualified input ownership lifecycle. The push is still not at a hardware-testable ELF because the bridge has not yet been bound to the live H1 coordinator/RFB request path and no native calibration renderer exists.

## Single best next action

On an execution-capable checkout, first run:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Preserve the exact output. Required expected lines are:

- `MPEG_CALIBRATION_HOST_TEST=PASS`
- `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS`
- `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS`

If those pass, perform the next experiment-local coordinator wiring tranche against the exact current branch:

1. add one `pstvnc_h1_mpeg_calibration_foreground_t` to H1 interaction-coordinator experiment state and initialize it with `PSTVNC_DISPLAY_WIDTH/HEIGHT`;
2. service it at the start of `h1_interaction_service_controller_state()`, immediately before `pstvnc_local_controller_route()`;
3. bind foreground callbacks to the coordinator's existing input-runtime suspension, pointer-neutralization, rebase, and resume seams rather than duplicating controller/input semantics;
4. if the foreground result consumes the observation, skip ordinary local-controller/OSK routing for that sample;
5. expose the bridge's current `freeze_rfb_visuals` fact to `pstvnc_h1_interaction_coordinator_service()` and then to the actual RFB framebuffer-request scheduling path at the existing complete-message boundary;
6. do not implement parser-internal pausing and do not modify `src/`;
7. keep native calibration rendering and permanent compositor clipping as separate subsequent modules unless they can be added without broadening this ownership patch.

After that wiring compiles/tests, the next PT_LOAD-changing ledge is the native PS2 calibration renderer plus build integration, followed by exact ELF/PT_LOAD identity before any hardware claim.
