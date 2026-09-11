# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 13:45 EDT run
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `69dfd7e416a2ea0355da521516b8d568344f870c`
- Pre-handoff implementation HEAD: `64a3c75365d0d9b436b3ebd8bbdb6a0d592807f6`
- The canonical handoff commit is the commit containing this file and is therefore the branch HEAD immediately after this update.
- Local Pi staged/unstaged/untracked state remains **not observable through the GitHub repository connector**. Do not infer a clean Pi worktree before any build/deploy. No unknown local work was reset, cleaned, discarded, or overwritten by this run.

## Larger push objective

Prepare a hardware-testable H1 ELF combining qualified visible RFB interaction, PCM audio, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Temporary entry gesture remains held `START+SELECT`.

During calibration, keep PSTV/RFB protocol state healthy, freeze new RFB visual/request work only through the existing complete-server-message scheduling boundary, retain the last authoritative desktop image, suppress ordinary mouse/controller/local-UI dispatch while physical controller polling continues, and render calibration locally. After accepted MPEG ownership begins, RFB may resume elsewhere but is forbidden from the outer edge of the outer matte inward.

## This session objective

Reduce the remaining H1 coordinator wiring surface without prematurely modifying the current coordinator or `src/`: compose the already-separated foreground bridge, safe-boundary RFB gate, render-plan access, and accepted-geometry access behind one experiment-local runtime boundary that the H1 coordinator can consume directly.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_runtime.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_runtime.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_runtime_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

Commits:

- `900c80d3e7773bfbb03613a42dcd7b4e1af30d78` — runtime interface
- `c1ba12acd5304ce6547b2866f43f19d601f61be0` — runtime implementation
- `83a824da8b4ea6bdb5579d4945e5a5018b0753d5` — runtime host contract test
- `64a3c75365d0d9b436b3ebd8bbdb6a0d592807f6` — host target integration

No `src/` file, qualified controller/input/local-UI/RFB implementation, parser/framing code, H1 interaction coordinator, PS2 graphics platform code, or permanent MPEG compositor was modified.

## Repository/source evidence reconciled this run

At session start the continuity guide and exact pushed branch history were re-read. The branch was still at `69dfd7e416a2ea0355da521516b8d568344f870c`, so there was no newer pushed calibration work to reconcile.

The current H1 coordinator and clean platform graphics seam were inspected again before the tranche:

- normalized controller state is routed in `h1_interaction_coordinator.c` immediately before `pstvnc_local_controller_route()`;
- the existing OSK path already demonstrates the qualified suspend -> neutralize published pointer -> rebase ownership sequence;
- final presentation converts the authoritative remote framebuffer to caller-owned GS pixels, prepares a caller-owned local overlay, then calls `pstvnc_ps2_graphics_present()`;
- `src/platform/ps2_graphics.h` accepts a complete desktop plus one generic local overlay and does not own the product meaning of that overlay.

Those facts support keeping calibration orchestration in the experiment-local runtime and a later experiment-local PS2 graphics adapter, rather than changing clean platform/controller owners.

## Runtime contract now represented in code

`h1_mpeg_calibration_runtime.*` composes, but does not merge, the existing calibration responsibilities:

- foreground controller ownership via `h1_mpeg_calibration_foreground`;
- RFB request/presentation freeze/thaw state via `h1_mpeg_calibration_rfb_gate`;
- active native calibration presentation facts via `h1_mpeg_calibration_render`;
- accepted geometry publication through a read-only committed-region accessor.

For each normalized controller observation the runtime:

1. delegates ownership transitions to the foreground bridge;
2. immediately mirrors the bridge's `freeze_rfb_visuals` fact into the RFB gate;
3. returns whether ordinary controller dispatch must be consumed;
4. exposes whether calibration has a visible modal screen.

The runtime does **not** itself issue RFB requests, parse RFB, draw GS pixels, poll libpad, route ordinary OSK/mouse actions, decode MPEG, or enforce permanent compositor clipping. Those remain with their existing/later owners.

On frozen -> thawed transition, the existing RFB gate still requires exactly one full/nonincremental refresh before normal incremental assumptions resume.

## Tests/checks and evidence boundary

Previously executed and recorded:

- `MPEG_CALIBRATION_HOST_TEST=PASS`

Committed but **not executed in this repository-only run**:

- `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_RENDER_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_RUNTIME_HOST_TEST=PASS` — not yet claimed

`Makefile.host` now builds/runs all six tests with `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`.

The new runtime host test is intended to verify:

- START+SELECT establishes the existing S/N/B foreground operation order;
- calibration entry closes both RFB request and remote-present gates;
- an active EDIT screen exposes a render plan;
- cancel remains frozen through release quarantine;
- full physical release resumes input, thaws both RFB gates, and produces exactly one full-refresh obligation;
- committed MPEG geometry is absent before acceptance.

No compiler, Pi execution, PS2 ELF build, deployment, or physical hardware observation occurred in this run. Therefore none of the newly added runtime behavior is execution-qualified yet.

## Evidence / artifact paths

Repository-visible evidence:

- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_*`
- `.../h1_mpeg_calibration_adapter.*`
- `.../h1_mpeg_calibration_foreground.*`
- `.../h1_mpeg_calibration_rfb_gate.*`
- `.../h1_mpeg_calibration_render.*`
- `.../h1_mpeg_calibration_runtime.*`
- corresponding host tests
- `.../Makefile.host`
- this handoff

No external evidence directory was created because no execution-capable checkout was available in this run.

## Current executable / ELF / PT_LOAD identity

No new PS2 executable was built.

- candidate ELF SHA256: **none yet**
- candidate ELF bytes: **none yet**
- candidate PT_LOAD SHA256/bytes: **none yet**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later recorded evidence. Calibration remains repository/source-contract evidence until a PT_LOAD-changing candidate is built and physically qualified.

## Proven vs unproven

### Proven from repository/source evidence

- calibration remains experiment-local;
- `src/` and qualified controller/input/local-UI/RFB/platform graphics code remain unchanged;
- START+SELECT meaning remains outside qualified local-controller code;
- portable state, normalized-controller mapping, foreground lifecycle, RFB scheduling gate, render-plan preparation, and runtime composition remain separately named responsibilities;
- the outer-matte suppression rectangle remains an executable calibration/backend contract separate from permanent MPEG/RFB composition;
- the H1 coordinator can now consume one experiment-local runtime boundary instead of knowing the internals of four calibration modules.

### Still unproven

- execution of adapter, foreground, RFB-gate, render, and runtime host tests on an execution-capable checkout;
- direct runtime binding immediately before `pstvnc_local_controller_route()`;
- live callback binding to qualified suspend/neutralize/rebase/resume operations;
- live safe-boundary application of runtime RFB gate decisions to actual framebuffer request generation and remote-frame presentation;
- PS2/GS adapter that rasterizes/draws the native calibration render plan over the frozen retained desktop;
- exact native visual fidelity on PS2;
- atomic accepted-geometry publication into MPEG presentation;
- permanent compositor enforcement of the outer-matte suppression rectangle;
- combined RFB + interaction + PCM + calibration + MPEG build/link;
- candidate ELF/PT_LOAD identity;
- physical PS2 behavior and hardware qualification.

## Important design decisions / reconciliation

- Do not put START+SELECT into `src/ui/local_controller.*`.
- Entry uses authoritative `buttons_down`, not paired edge assumptions.
- Calibration reuses the qualified input-runtime ownership lifecycle.
- RFB quiescence remains application scheduling policy at the existing safe complete-message boundary; calibration does not disconnect RFB or pause parser internals.
- Thaw requires one full/nonincremental refresh before normal incremental assumptions resume.
- Calibration and permanent MPEG presentation/composition remain separate owners.
- Native calibration rendering remains presentation facts plus a later experiment-local platform adapter; the clean PS2 graphics platform owner stays generic.
- The runtime is orchestration only. It deliberately does not become a second controller, RFB, graphics, or MPEG owner.

## Current overall push status / milestone

Milestone reached: **portable calibration core + normalized-controller adapter + foreground bridge + safe-boundary RFB gate + platform-neutral render plan + one experiment-local runtime composition boundary are represented on the experimental branch.**

This removes most policy plumbing from the eventual H1 coordinator patch. The main remaining functional layers before a hardware-testable ELF are now (1) execution of host contracts, (2) live H1 coordinator/RFB scheduling binding, (3) experiment-local PS2 calibration-overlay raster/presentation adapter, (4) accepted geometry handoff to MPEG presentation plus permanent suppression enforcement, and (5) combined PS2 build/identity evidence.

## Single best next action

On the next run, first re-read this file and branch state. If an execution-capable checkout is still unavailable, inspect the complete current H1 coordinator service/present functions and the current RFB worker request scheduling interfaces, then make the smallest coherent **experiment-local coordinator binding** that:

1. owns one `pstvnc_h1_mpeg_calibration_runtime_t` in the H1 coordinator;
2. services it immediately before ordinary `pstvnc_local_controller_route()`;
3. maps the runtime foreground callbacks to the existing suspend/neutralize/rebase/resume operations without changing their semantics;
4. skips ordinary local-controller routing whenever runtime result says `consume_controller_state`;
5. evaluates request/present permission only at the existing safe server-message/application scheduling boundary;
6. consumes the one-shot thaw full-refresh requirement there;
7. does not yet fold PS2 rasterization or permanent MPEG composition into the runtime.

If an execution-capable checkout is available first run:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Preserve exact stdout/stderr and do not proceed past a failing contract without recording the failure. Expected PASS markers are the core, adapter, foreground, RFB-gate, render, and runtime tests listed above.

After the coordinator binding compiles, the next separate tranche is the experiment-local PS2 graphics adapter for the render plan. Only after those integrate should the combined candidate ELF be built and its exact SHA256, byte count, and PT_LOAD identity recorded before any physical-hardware claim.
