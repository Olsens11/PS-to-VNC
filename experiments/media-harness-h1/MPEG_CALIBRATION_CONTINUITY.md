# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 08:44 EDT
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `7b2cff832b66bbb478bc16ef04f994dfd41ea48a`
- Implementation commit produced this session: `6b255836ac15f069f432e82a9697de24555f5447`
- Handoff/docs commit: the commit containing this file; verify with `git rev-parse HEAD` after fetch
- Local Pi dirty/staged/untracked state: **not proven by the GitHub repository connector in this run**. Do not infer that the Pi worktree is clean from the pushed branch. Before Pi-side build/deploy, reconcile `git status --short --branch` and any local experiment files against this handoff.

## Larger push objective

Prepare a hardware-testable H1 ELF that combines the already-qualified interaction path (visible RFB + mouse + keyboard/OSK/local UI), PCM audio, and MPEG presentation through a native PS2 calibration flow, while keeping calibration experiment-local and keeping the permanent MPEG compositor/presentation backend separate.

Required temporary calibration entry gesture: held `START+SELECT`.

During calibration:

- keep the RFB/PSTV protocol session healthy;
- finish any RFB server message already in progress to the existing complete-message scheduling boundary;
- issue no new framebuffer-update requests;
- present no new RFB visual updates;
- retain the last authoritative desktop image;
- suppress ordinary mouse/controller dispatch while physical controller polling continues;
- render calibration locally.

After accepted MPEG ownership begins, RFB may resume elsewhere, but from the **outer edge of the outer matte inward** RFB presentation is forbidden.

## Session objective

Establish the first experiment-local, multi-file calibration core without modifying `src/` or qualified controller/pad code, and leave a deterministic host contract suitable for the next H1 integration step.

## What changed

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_geometry.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_state.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_test.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- `experiments/media-harness-h1/mpeg_presentation_calibration/README.md`

No `src/` file was modified. No direct merge/alignment into `src/` was attempted.

## Preserved behavior/contracts

The portable core currently encodes:

- held `START+SELECT` as a one-shot level-chord entry gesture;
- center-preserving 16-pixel D-pad resize;
- one-pixel R1 movement;
- one-pixel R2 inner-matte mapping;
- one-pixel L2 outer-matte mapping with the accepted reversed horizontal/all-sides semantics;
- EDIT / CONTROLS / REVIEW modal state;
- X release/re-press requirement before final Review acceptance;
- post-exit release quarantine so calibration controls cannot leak into desktop input;
- permanent backend RFB suppression geometry as base MPEG rectangle expanded by the outer matte and clipped to the physical canvas.

The calibration module deliberately does not own pad polling, ordinary mouse/OSK routing, RFB parsing/socket state, MPEG decode, permanent composition, or PS2 GS primitives.

## Tests/evidence completed

Host test was compiled and executed before committing with:

`cc -std=c99 -O2 -Wall -Wextra -Werror -pedantic`

Result:

`MPEG_CALIBRATION_HOST_TEST=PASS`

The host test covers current defaults for a 704x462 canvas, START+SELECT entry, centered resize, one-pixel move, inner matte, outer matte, suppression rectangle derivation, Review release/re-press acceptance, and release-quarantine completion.

Repository evidence:

- implementation commit: `6b255836ac15f069f432e82a9697de24555f5447`
- parent/pushed authority used as base: `7b2cff832b66bbb478bc16ef04f994dfd41ea48a`

No PS2 ELF was built in this run. Therefore there is no new ELF SHA256/PT_LOAD identity yet.

## Proven vs unproven

### Proven in this session

- the portable calibration state/geometry contract compiles cleanly under strict host warnings;
- its current host contract test passes;
- calibration is isolated in the experimental directory;
- `src/` and qualified controller/pad source remain untouched;
- the outer-matte-to-RFB-suppression rule is represented as executable geometry logic rather than prose only.

### Still unproven

- native PS2 rendering of EDIT/CONTROLS/REVIEW;
- exact H1 coordinator wiring of START+SELECT ahead of ordinary Select->OSK routing;
- exact use of the qualified input-runtime suspend/rebase/resume seam;
- RFB request freeze/resume at the current complete-message boundary in the combined candidate;
- PCM + RFB + calibration + MPEG coexistence in one ELF;
- permanent compositor enforcement of the suppression rectangle;
- PS2 build/link success for the new module;
- ELF/PT_LOAD identity;
- any physical PS2 behavior or hardware qualification.

## Hardware-qualification boundary

Hardware authority remains the previously qualified interaction/CP2N lineage and subsequent evidence already recorded by the branch. This new calibration implementation is host-only evidence until a new PT_LOAD-changing candidate is built and then physically qualified. Do not describe the new module as hardware-qualified.

## Important reconciliation

Historical media-object composition notes that contemplated continuing RFB decoding/presentation beneath active MPEG do not govern this calibration experiment's newer ownership requirement. For this experiment:

1. calibration freezes new RFB visual work at a valid complete-message scheduling boundary while the transport/session remains healthy;
2. after accepted MPEG ownership starts, RFB may resume outside the accepted suppression rectangle;
3. the suppression rectangle begins at the outer edge of the outer matte.

Preserve the older historical note as evidence; do not rewrite history silently.

## Current push status

Milestone reached: **portable calibration core exists and is host-tested on the experimental branch**.

The push is not yet at a hardware-testable ELF. The next PT_LOAD-changing integration work remains.

## Single best next action

First reconcile the fetched branch with the actual Pi worktree (`git status --short --branch`) so no uncommitted CP2P/local work is overwritten. Then, using the current experimental CP2P/H1 source as authority and `src/` only as a reference, add an experiment-local adapter/renderer that:

1. observes normalized controller state and detects held START+SELECT before Select->OSK dispatch;
2. invokes the existing qualified input-runtime mouse-suspension/rebase/resume seam without modifying qualified controller internals;
3. requests RFB visual quiescence only at the existing complete-server-message scheduling boundary and stops new framebuffer-update requests while calibration owns foreground;
4. renders EDIT/CONTROLS/REVIEW locally on PS2;
5. publishes accepted calibration geometry atomically to the existing experimental MPEG presentation path;
6. applies the returned outer-matte suppression rectangle in the permanent experiment compositor rather than inside the calibration module;
7. builds one exact candidate ELF and records ELF SHA256, bytes, PT_LOAD SHA256/bytes, source HEAD, and checker results before any hardware claim.

If the Pi worktree contains newer unpushed CP2P files, those local files are authoritative for this integration step and must be reconciled rather than overwritten by the pushed CP2O-prep branch.
