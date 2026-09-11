# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-11 14:49 EDT / 18:49 UTC
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `a91f5b3fa0e4df25c6e967409445dde5e0058077`
- Evidence/source HEAD before this handoff update: `83f4bba0552c72dc29df1d5f8a7b058b4173c4ce`
- The canonical handoff commit is the commit containing this file and is the pushed branch HEAD immediately after this update.
- GitHub branch evidence has no staged/unstaged/untracked state. The Pi worktree's local staged/unstaged/untracked state is not observable through the repository connector in this run; do not infer it is clean. No reset, clean, discard, overwrite, merge, or `src/` mutation was performed.

## Larger push objective

Produce a hardware-testable H1 ELF with qualified visible RFB interaction, PCM, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Calibration requirements remain: held `START+SELECT` temporary entry; accepted EDIT/CONTROLS/REVIEW UX and mapping; physical controller polling continues but ordinary mouse/controller/local-UI dispatch is suppressed while calibration owns foreground; RFB protocol remains healthy while new RFB visual presentation is frozen; request scheduling stops at the existing complete-server-message boundary; thaw requires one full/nonincremental refresh; and once MPEG presentation owns an accepted region, the outer edge of the outer matte is the permanent RFB suppression perimeter inward.

## This session objective

Close the outstanding execution-evidence gap for the already-committed calibration core, adapter, foreground bridge, RFB gate, render-plan contract, and runtime facade before making live H1 coordinator/RFB-scheduler changes.

## What changed this run

Updated `.github/workflows/h1-rfb-mux-prep.yml` so the existing branch CI runs:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Commit:

- `83f4bba0552c72dc29df1d5f8a7b058b4173c4ce` — `ci(h1): execute MPEG calibration host contracts`

No product/experiment C source was changed in this tranche. No `src/` file, qualified controller/input/local-UI implementation, RFB parser, graphics platform owner, or permanent MPEG compositor was changed.

## Tests/checks and exact evidence

GitHub Actions run `34635378219`, job `host-preflight` (`103381859183`), for exact HEAD `83f4bba0552c72dc29df1d5f8a7b058b4173c4ce` completed successfully.

The new step `Test MPEG presentation calibration host contracts` completed `success` at 2026-09-11T18:48:50Z. The Makefile compiles/runs all six contracts under `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`:

- portable calibration core test — PASS by combined step success
- normalized-controller adapter test — PASS by combined step success
- foreground ownership bridge test — PASS by combined step success
- RFB freeze/thaw gate test — PASS by combined step success
- render-plan contract test — PASS by combined step success
- runtime facade test — PASS by combined step success

The same host-preflight job also passed the existing H1 CONFIG, RFB mux ownership, headless coordinator, CP2K presentation, CP2L mouse input, CP2M keyboard, CP2N real-module interaction, logical RFB channel/credit, Pi byte bridge, reader integration, cumulative bridge, and operator-surface checks.

The workflow's pinned-toolchain `ps2-build` job was still in progress when this handoff evidence was written. It builds the existing RFB-prep/CP2K/CP2L/CP2M/CP2N lineage only; it does **not** yet build a calibration ELF, so its completion would not constitute calibration ELF evidence.

Evidence URLs/paths:

- GitHub Actions run: `34635378219`
- host-preflight job: `103381859183`
- `.github/workflows/h1-rfb-mux-prep.yml`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- calibration module/test files under `experiments/media-harness-h1/mpeg_presentation_calibration/`
- this handoff

No Pi execution, deployment, or physical PS2 observation occurred.

## Repository/source reconciliation this run

The exact current pushed H1 coordinator and RFB runtime were re-read before deciding the next integration shape.

Important live facts:

- normalized controller state is still handled immediately before `pstvnc_local_controller_route()` in `h1_interaction_coordinator.c`;
- the existing OSK path remains the qualified reference for suspend -> neutralize published pointer -> rebase -> later resume;
- `h1_rfb_session_runtime.c` invokes application service only at complete server-message boundaries;
- after that service callback it currently sends the next incremental framebuffer request unconditionally;
- an incremental request can already be outstanding while the server is idle;
- therefore calibration entry may need to suppress presentation of one already-in-flight response, then prevent subsequent requests, while leaving parsing/session state healthy;
- on thaw, the scheduler needs a one-shot full/nonincremental request before returning to incremental requests.

A pushed-branch/local-worktree discrepancy is also explicit: the pushed branch currently contains `h1_main_rfb_visible_interaction_pcm.c` (CP2O, MPEG OFF) and does **not** contain the later `h1_main_rfb_visible_interaction_pcm_mpeg.c` / shared MPEG compositor names referenced by earlier Pi-local development context. Do not silently assume those Pi-local/unpushed files exist in the pushed branch. Reconcile the Pi worktree before any deployment or build that could overwrite/localize newer work.

## Current executable / ELF / PT_LOAD identity

No calibration-enabled PS2 executable exists yet.

- calibration candidate ELF SHA256: **none**
- calibration candidate ELF bytes: **none**
- calibration candidate PT_LOAD SHA256/bytes: **none**

The existing CI PS2 job continues to build the older qualified/unqualified baseline lineage only. Hardware authority remains with the previously qualified CP2N/interaction lineage and later recorded hardware evidence; the calibration path is host-contract proven but not yet PS2-link or hardware proven.

## Proven vs unproven

### Proven now

- all six calibration host contracts compile and execute successfully under strict host warnings-as-errors;
- calibration remains experiment-local and `src/` remains untouched;
- START+SELECT semantic entry, accepted geometry/control behavior, release quarantine, foreground ownership lifecycle, RFB gate behavior, render-plan preparation, and runtime composition are covered by passing host contracts;
- outer-matte-derived suppression geometry remains an executable backend contract separate from permanent composition;
- current RFB scheduling provides the required complete-message application boundary for calibration policy insertion.

### Still unproven

- live H1 coordinator binding before ordinary `pstvnc_local_controller_route()`;
- live use of the qualified suspend/neutralize/rebase/resume functions through the calibration foreground callbacks;
- safe-boundary suppression of new framebuffer requests after calibration entry;
- suppression of visual publication for an already-outstanding RFB request that completes during calibration;
- one-shot full/nonincremental framebuffer refresh on thaw in the real RFB loop;
- native PS2 calibration overlay raster/presentation adapter;
- accepted geometry handoff into MPEG presentation;
- permanent compositor enforcement of the outer-matte RFB suppression perimeter;
- calibration-enabled PS2 compile/link and checker results;
- candidate ELF/PT_LOAD identity;
- physical PS2 qualification.

## Important design decisions / reconciliation

- Do not put START+SELECT into `src/ui/local_controller.*` or qualified pad code.
- Calibration may claim foreground only from the ordinary desktop path, but once claimed it must continue receiving normalized physical-controller observations through release quarantine.
- Do not pause/disconnect the RFB parser/socket as the calibration mechanism.
- If a framebuffer request was already outstanding when calibration claims foreground, consume the resulting complete server message to keep protocol state healthy but suppress its visual publication; then issue no new request while frozen.
- The RFB scheduler, not the calibration state machine, owns request issuance.
- Thaw's full-refresh obligation must be represented at the RFB safe-boundary scheduler, not hidden in the parser.
- Calibration rendering remains separate from the permanent MPEG compositor/presenter.
- Never align/merge into `src/` without David's explicit later permission.

## Current overall push status / milestone

Milestone reached: **the entire experiment-local calibration policy stack through the runtime facade is now host-execution proven.**

The remaining path to a hardware-testable ELF is predominantly live integration: safe-boundary RFB request policy + H1 coordinator ownership binding, native PS2 calibration rendering, accepted-region MPEG/presentation binding with outer-matte suppression, then pinned PS2 build/checkers/ELF identity and physical qualification.

## Single best next action

Implement the smallest experiment-local live scheduling/binding tranche, preserving existing callers:

1. extend `h1_rfb_session_runtime` with an optional safe-boundary framebuffer-request policy seam while preserving all existing run entry points unchanged;
2. represent three scheduler decisions: HOLD, INCREMENTAL, FULL;
3. explicitly track whether a framebuffer request is outstanding so IDLE servicing cannot manufacture duplicate requests;
4. after calibration entry, allow an already-outstanding response to finish but suppress its presentation through the coordinator's calibration runtime gate;
5. when no request is outstanding, consult the policy only at the existing complete-message/IDLE boundary; HOLD sends nothing, INCREMENTAL sends the normal incremental request, FULL sends one nonincremental request;
6. bind the coordinator's calibration runtime immediately before ordinary local-controller routing, using the existing suspend/neutralize/rebase/resume semantics and consuming ordinary routing while calibration owns foreground/release quarantine;
7. add host/source contract tests for the scheduler semantics before touching native PS2 rendering.

Do not fold PS2 calibration rasterization or permanent MPEG composition into that scheduling tranche. Before any Pi build/deploy, first reconcile the Pi worktree against this pushed branch because its local dirty/unpushed state is not visible here.
