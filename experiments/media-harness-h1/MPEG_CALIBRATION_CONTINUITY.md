# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 12:49 EDT run
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `efb24d51abc80583d9d3bd4fc7fb4da2d75e0816`
- Pre-handoff implementation HEAD: `cd107a502f759224ce1ee3c287ce45119d0cc2c8`
- Local Pi staged/unstaged/untracked state remains **not observable through the GitHub repository connector**. Repository-visible pushed state is authoritative for this run; do not infer a clean Pi worktree before any build/deploy.

## Larger push objective

Prepare a hardware-testable H1 ELF combining qualified visible RFB interaction, PCM audio, MPEG presentation, and native PS2 MPEG-presentation calibration while keeping calibration experiment-local and separate from the permanent MPEG compositor/presentation backend.

Temporary entry gesture remains held `START+SELECT`.

During calibration, keep PSTV/RFB protocol state healthy, freeze new RFB visual/request work only at the existing complete-server-message boundary, retain the last authoritative desktop image, suppress ordinary mouse/controller/local-UI dispatch while physical controller polling continues, and render calibration locally. After accepted MPEG ownership begins, RFB may resume elsewhere but is forbidden from the outer edge of the outer matte inward.

## This session objective

Advance native calibration rendering without prematurely binding PS2/GS primitives into calibration state or touching qualified `src/` code. Mirror the clean OSK renderer ownership pattern: calibration state produces a caller-consumable presentation description; platform drawing remains a later H1 adapter.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_render.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_render.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_render_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

Commits:

- `366dcad1b3b48c7bb9ed459dccce9820005bfd20` — render-plan interface
- `a2e09ff899e9d9bb3677484342f70378f4e3e630` — render-plan implementation
- `17818078f128315a4600a1e38b06bb3340b9a9f7` — render-plan host contract test
- `cd107a502f759224ce1ee3c287ce45119d0cc2c8` — host target integration

No `src/` file, qualified controller/input/local-UI/RFB implementation, parser/framing code, or permanent MPEG compositor was modified.

## Repository/source evidence reconciled this run

The current H1 coordinator and clean OSK renderer contract were re-read before implementation.

Relevant current facts:

- H1 currently converts the remote framebuffer to GS pixels, prepares caller-owned local UI overlay pixels, then calls the platform graphics presenter;
- clean `osk_render` is explicitly platform-neutral: it consumes domain state and writes/produces caller-owned presentation facts without owning controller, RFB, GS, foreground, or presentation policy;
- therefore calibration rendering should first expose presentation facts, then a separate H1 platform adapter may translate those facts to PS2 drawing primitives.

This avoids making calibration state itself the permanent compositor or a PS2 graphics owner.

## Render contract now represented in code

`h1_mpeg_calibration_render.*` translates active calibration state into a platform-neutral render plan containing:

- current calibration region;
- derived outer-matte RFB suppression rectangle;
- modal screen kind: EDIT / CONTROLS / REVIEW;
- native calibration title/body/footer strings reflecting the accepted controls;
- Review text explicitly states the outer-matte/RFB ownership contract and X release/re-press acceptance rule.

Inactive calibration produces no render plan.

The render plan does **not** own GS memory, pixel conversion, RFB presentation, MPEG presentation, controller interpretation, or foreground transitions.

## Tests/checks and evidence boundary

Previously executed and recorded:

- `MPEG_CALIBRATION_HOST_TEST=PASS`

Committed but **not executed in this repository-only run**:

- `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS` — not yet claimed
- `MPEG_CALIBRATION_RENDER_HOST_TEST=PASS` — not yet claimed

`Makefile.host` now builds/runs all five tests with `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`.

No compiler, Pi execution, PS2 ELF build, deployment, or physical hardware observation occurred in this run.

## Evidence / artifact paths

Repository-visible evidence:

- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_*`
- `.../h1_mpeg_calibration_adapter.*`
- `.../h1_mpeg_calibration_foreground.*`
- `.../h1_mpeg_calibration_rfb_gate.*`
- `.../h1_mpeg_calibration_render.*`
- corresponding host tests
- `.../Makefile.host`
- this handoff

## Current executable / ELF / PT_LOAD identity

No new PS2 executable was built.

- candidate ELF SHA256: **none yet**
- candidate ELF bytes: **none yet**
- candidate PT_LOAD SHA256/bytes: **none yet**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later recorded evidence. Calibration remains repository/source-contract evidence until a PT_LOAD-changing candidate is built and physically qualified.

## Proven vs unproven

### Proven from repository/source evidence

- calibration remains experiment-local;
- `src/` and qualified controller/input/local-UI/RFB code remain unchanged;
- START+SELECT meaning remains outside qualified local-controller code;
- normalized-controller adapter, foreground-ownership bridge, and safe-boundary RFB gate remain separated by responsibility;
- outer-matte suppression geometry remains executable and separate from permanent MPEG/RFB composition;
- calibration now has an explicit platform-neutral native render-plan boundary modeled after the clean UI renderer ownership style.

### Still unproven

- execution of adapter, foreground, RFB-gate, and render host tests on an execution-capable checkout;
- direct coordinator wiring before `pstvnc_local_controller_route()`;
- live binding to qualified suspend/neutralize/rebase/resume callbacks;
- live binding of freeze state into actual framebuffer-request and remote-presentation scheduling;
- PS2/GS adapter that draws the render plan over the frozen desktop;
- exact native visual fidelity on PS2;
- atomic accepted-geometry publication into MPEG presentation;
- permanent compositor enforcement of the suppression rectangle;
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
- Native calibration rendering follows the clean renderer pattern: prepare presentation facts first, bind platform drawing separately.

## Current overall push status / milestone

Milestone reached: **portable calibration core + normalized-controller adapter + foreground bridge + safe-boundary RFB gate + platform-neutral native calibration render-plan contract are represented on the experimental branch.**

The remaining path to a hardware-testable ELF is now primarily live H1 integration: execute host contracts, wire coordinator ownership/RFB scheduling, add the PS2 graphics adapter for the render plan, publish accepted geometry to the existing MPEG presentation boundary, then build and identify the candidate ELF.

## Single best next action

On an execution-capable checkout first run:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Preserve exact output. Expected PASS lines are the core, adapter, foreground, RFB-gate, and render host tests listed above.

If all pass, the next coherent source tranche is the experiment-local H1 coordinator/platform integration:

1. service the foreground bridge immediately before ordinary local-controller routing;
2. bind its callbacks to existing suspend, pointer-neutralization, rebase, and resume operations;
3. consume calibration-owned controller observations before OSK/local-controller routing;
4. apply foreground `freeze_rfb_visuals` to the safe-boundary RFB gate;
5. suppress actual framebuffer request generation and remote-frame presentation while frozen without stopping protocol service;
6. on thaw issue the gate's one-shot nonincremental refresh;
7. when calibration is active, prepare the new render plan and translate it in a separate H1 PS2 graphics adapter over the retained desktop;
8. keep accepted-geometry publication and permanent MPEG compositor enforcement as separate interfaces rather than moving them into calibration state.

After that compiles, build the combined candidate and record exact ELF SHA256, byte count, and PT_LOAD identity before any hardware claim.
