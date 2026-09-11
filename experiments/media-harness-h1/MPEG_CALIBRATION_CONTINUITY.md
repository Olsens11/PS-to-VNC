# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-11 15:47 EDT / 19:47 UTC
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `961fcae90045357b76655d548b03589304640315`
- Implementation/evidence HEAD before this handoff update: `3e00278b0fe52531c4a9707753a59c9653e6885e`
- The canonical handoff commit is the commit containing this file and is the branch HEAD immediately after this update.
- Repository connector evidence cannot report a Pi worktree's staged/unstaged/untracked state. Do not infer that David's Pi worktree is clean. No reset, clean, discard, overwrite, merge, or `src/` mutation was performed.

## Larger push objective

Produce a hardware-testable H1 ELF with qualified visible RFB interaction, PCM, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Calibration requirements remain: held `START+SELECT` temporary entry; accepted EDIT/CONTROLS/REVIEW UX and mapping; physical controller polling continues but ordinary mouse/controller/local-UI dispatch is suppressed while calibration owns foreground; RFB protocol remains healthy while new RFB visual presentation is frozen; request scheduling stops at complete-server-message/IDLE boundaries; thaw requires one full/nonincremental refresh; and once MPEG presentation owns an accepted region, the outer edge of the outer matte is the permanent RFB suppression perimeter inward.

## This session objective

Make the next safe scheduling tranche concrete before touching the live PS2 RFB loop: model one-outstanding-request ownership and HOLD / INCREMENTAL / FULL decisions directly against the already host-proven calibration RFB gate, with regression coverage for the in-flight-response case.

## What changed this run

Added under `experiments/media-harness-h1/mpeg_presentation_calibration/`:

- `h1_mpeg_calibration_rfb_schedule.h`
- `h1_mpeg_calibration_rfb_schedule.c`
- `h1_mpeg_calibration_rfb_schedule_test.c`

Updated:

- `Makefile.host`

Implementation commit:

- `3e00278b0fe52531c4a9707753a59c9653e6885e` — `feat(h1): model calibration-safe RFB request scheduling`

No `src/` file, qualified controller/input/local-UI implementation, RFB parser/session runtime, platform graphics implementation, H1 coordinator, or permanent MPEG compositor was changed.

## Scheduler contract now represented in code

The new scheduler owns only one fact: whether a framebuffer request is currently outstanding. It owns no socket, parser, framebuffer, controller, renderer, or permanent-composition state.

At an already-safe RFB boundary:

- outstanding request -> `HOLD` without consuming a pending full-refresh obligation;
- no request + frozen calibration RFB gate -> `HOLD`;
- no request + thawed gate with one-shot refresh obligation -> `FULL`;
- no request + ordinary thawed gate -> `INCREMENTAL`.

After a request is actually transmitted, `request_sent()` marks it outstanding. When its complete framebuffer-update response finishes, `update_complete()` clears it even if calibration has frozen remote visual publication in the meantime. This is the required protocol/visual split for the case where START+SELECT claims calibration while an older incremental request is already waiting for server damage.

The scheduler deliberately does not itself decide whether a completed update is visually published; the existing calibration RFB gate remains the source of that presentation fact. It also does not issue the request. The live RFB session runtime remains the request/socket owner.

## Tests/checks and exact evidence

GitHub Actions run `34640522641` for exact implementation HEAD `3e00278b0fe52531c4a9707753a59c9653e6885e` started from the push.

Job `host-preflight` (`103398791449`) completed **success**. Its `Test MPEG presentation calibration host contracts` step completed **success** and now executes seven strict host contracts under the existing `-std=c99 -O2 -Wall -Wextra -Werror -pedantic` Makefile, including the new scheduler test.

New scheduler assertions covered by that passing target:

- ordinary thawed state chooses `INCREMENTAL`;
- repeated IDLE scheduling while a request is outstanding chooses `HOLD` and cannot manufacture a duplicate request;
- entering freeze with a request in flight preserves the outstanding request until its complete response is consumed;
- after that response completes, frozen state sends no replacement request;
- thaw produces exactly one `FULL` decision;
- IDLE while that full request is outstanding does not duplicate it;
- after full-response completion, scheduling returns to `INCREMENTAL`;
- a pending FULL obligation is not consumed merely because an older request is still outstanding.

The same host-preflight job passed the existing H1 CONFIG, source-ownership, CP2K/CP2L/CP2M/CP2N, logical RFB channel/credit, Pi bridge, reader integration, cumulative bridge, and operator-surface checks.

A separate local compiler smoke test of the gate+scheduler transition also produced `MPEG_CALIBRATION_RFB_SCHEDULE_HOST_TEST=PASS`; this is supplementary only. GitHub Actions above is the repository-authoritative execution evidence.

At handoff time, workflow run `34640522641` job `ps2-build` (`103398791975`) was still in progress. That job builds the existing RFB-prep/CP2K/CP2L/CP2M/CP2N lineage and does **not** yet build a calibration-enabled ELF, so its eventual result is not calibration-ELF qualification evidence.

Evidence paths/IDs:

- implementation commit `3e00278b0fe52531c4a9707753a59c9653e6885e`
- workflow run `34640522641`
- host-preflight job `103398791449`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_schedule.*`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff

No Pi execution, deployment, or physical PS2 observation occurred.

## Repository/source reconciliation this run

At session start the continuity guide was read first and branch `experiment/h1-rfb-mux-prep` was independently verified at exact pushed HEAD `961fcae90045357b76655d548b03589304640315`. No newer pushed calibration commit existed.

The current `h1_rfb_session_runtime.c/.h` were re-read before implementation. Live facts remain:

- application service runs only at complete server-message boundaries;
- after initial-frame service the runtime currently sends an incremental request directly;
- after each completed incremental update it currently services the application then sends another incremental request directly;
- IDLE can occur while an incremental request is already outstanding on a static desktop;
- IDLE application service may therefore observe calibration entry while the older request remains outstanding;
- the complete response to that old request must still be parsed/consumed but may need visual publication suppressed;
- no duplicate framebuffer request should be sent merely because IDLE application service ran again.

The pushed-branch/Pi-local discrepancy remains important: this pushed branch contains CP2O `h1_main_rfb_visible_interaction_pcm.c` with MPEG OFF and does not prove that later Pi-local MPEG/shared-compositor files are absent from David's worktree. Reconcile the Pi worktree before any deploy/build that could overwrite newer local work.

## Current executable / ELF / PT_LOAD identity

No calibration-enabled PS2 executable exists yet.

- calibration candidate ELF SHA256: **none**
- calibration candidate ELF bytes: **none**
- calibration candidate PT_LOAD SHA256/bytes: **none**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later recorded hardware evidence. Calibration through the new RFB scheduler is host-contract proven only.

## Proven vs unproven

### Proven now

- the complete experiment-local calibration policy stack through the runtime facade remains strict-host-test passing;
- the new RFB request scheduler is also strict-host-test passing;
- HOLD / INCREMENTAL / FULL semantics are executable rather than prose-only;
- one-outstanding-request ownership prevents IDLE from creating duplicate requests;
- the in-flight-response case is represented: freeze does not cancel protocol ownership, completion clears the outstanding fact, and frozen state issues no replacement request;
- thaw's one-shot full-refresh obligation survives an outstanding older request and is consumed only when the scheduler can actually choose `FULL`;
- calibration remains experiment-local and `src/` remains untouched;
- permanent RFB suppression from the outer edge of the outer matte remains a separate backend/compositor contract.

### Still unproven

- live use of this scheduler inside `h1_rfb_session_runtime`;
- live safe-boundary HOLD / incremental=1 / full=0 request issuance;
- suppression of visual publication for a response that completes while calibration owns foreground;
- coordinator binding immediately before ordinary `pstvnc_local_controller_route()`;
- live use of qualified suspend/neutralize/rebase/resume callbacks;
- native PS2 calibration overlay raster/presentation adapter;
- accepted geometry handoff into MPEG presentation;
- permanent compositor enforcement of the outer-matte RFB suppression perimeter;
- calibration-enabled PS2 compile/link/checkers;
- candidate ELF/PT_LOAD identity;
- physical PS2 qualification.

## Important design decisions / reconciliation

- Do not put START+SELECT into `src/ui/local_controller.*` or qualified pad code.
- Do not pause/disconnect the RFB parser/socket as calibration's freeze mechanism.
- Request ownership and visual-publication ownership are deliberately distinct.
- HOLD while `request_outstanding` must not consume the full-refresh obligation.
- `update_complete()` must run for an in-flight response even if its visual presentation is suppressed.
- The live RFB session runtime remains the owner that actually calls `pstvnc_rfb_session_request_update(session, incremental)`.
- Calibration rendering remains separate from permanent MPEG composition.
- Never align/merge into `src/` without David's explicit later permission.

## Current overall push status / milestone

Milestone reached: **the calibration policy stack now includes an execution-proven request-ownership scheduler for the exact static-desktop/in-flight-response problem that blocked safe live RFB wiring.**

The remaining path to a hardware-testable ELF is now: integrate that scheduler into the experiment-local H1 RFB runtime while preserving old entry points; bind coordinator calibration ownership/presentation gating; implement native PS2 calibration rendering; connect accepted geometry to MPEG presentation and outer-matte compositor suppression; then produce pinned PS2 build/checker/ELF/PT_LOAD evidence and perform physical qualification.

## Single best next action

Integrate the proven scheduler into the experiment-local `h1_rfb_session_runtime` without changing existing callers:

1. add an optional request-policy/presentation-policy entry point while keeping `run`, `run_with_presenter`, and `run_with_presenter_and_service` behavior unchanged by default;
2. initialize scheduler state after the initial full frame (no request outstanding);
3. replace direct incremental request issuance with one helper that maps `HOLD` -> no send, `INCREMENTAL` -> `pstvnc_rfb_session_request_update(..., 1)`, `FULL` -> `pstvnc_rfb_session_request_update(..., 0)`, and marks successful sends outstanding;
4. on complete update, clear outstanding ownership before presentation/service policy is evaluated;
5. at IDLE, never send another request while one is outstanding; if calibration entered during IDLE, keep parsing the old response when it eventually arrives;
6. expose a narrow presentation-allow callback so a completed in-flight response can update authoritative parser/framebuffer state while visual publication is suppressed during calibration;
7. add source/host contract coverage and require pinned PS2 compile success before moving on to native calibration rasterization.

Do not fold PS2 rasterization, MPEG decode, or permanent compositor clipping into that runtime patch. Before any Pi build/deploy, reconcile David's Pi worktree because its local dirty/unpushed state is not visible through this repository connection.
