# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 09:42 EDT run
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `210cfd0b0e79985a262d0d06e9dba726a3fbd94c`
- Calibration-core implementation commit: `6b255836ac15f069f432e82a9697de24555f5447`
- Adapter tranche commits in this run:
  - `0d6bb3ef07af4aac10586c1ee743ae1a44c577a3` — adapter header
  - `686de8cf8d31329970a54e371eaa98c1c98b688f` — adapter implementation
  - `b8af43c087df4f002b7f24c73fc28638e5a28b31` — adapter host test
  - `514dacbc52f66c7bff43c6a94cf7b851dd139610` — host Makefile target
- Handoff/docs commit: the commit containing this file; verify branch HEAD after fetch.
- Local Pi dirty/staged/untracked state: **not observable through the repository connector in this run**. The connected branch state is authoritative for pushed evidence only. Before any Pi build/deploy, reconcile the actual Pi worktree status and preserve any newer local CP2P work.

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

Add the experiment-local adapter between the already-qualified normalized controller-state vocabulary and the portable calibration module, without changing `src/`, libpad/controller ownership, or the existing local-controller implementation.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

No `src/` file was modified. No direct merge/alignment into `src/` was attempted.

## Adapter contract now represented in code

The adapter:

1. accepts only `pstvnc_controller_state_t`, the existing project-owned normalized physical-controller fact type;
2. maps those button facts into the portable calibration vocabulary without importing PS2SDK/PAD constants;
3. detects calibration entry from authoritative `buttons_down` so held START+SELECT works even when a connection-epoch sample deliberately manufactures no press edges;
4. leaves ordinary Select alone when START is not simultaneously held, preserving normal Select->OSK routing;
5. consumes the START+SELECT entry observation before it can also become ordinary local-controller/OSK behavior;
6. consumes every controller observation while calibration owns foreground, including the post-exit release quarantine;
7. exposes `freeze_rfb_visuals` for exactly the same ownership interval, including the entry sample and release-quarantine sample;
8. does not itself touch RFB parsing, framebuffer requests, sockets, GS rendering, permanent MPEG composition, or input-runtime ownership.

The coordinator/RFB worker must honor `freeze_rfb_visuals` only at its already-existing complete-server-message scheduling boundary; the adapter does not invent a second parser boundary.

## Current repository evidence inspected

The pushed branch started this run at `210cfd0b0e79985a262d0d06e9dba726a3fbd94c`.

Current H1 coordinator evidence confirms:

- controller-state events reach `h1_interaction_service_controller_state()` as normalized `pstvnc_controller_state_t` facts;
- ordinary local-controller routing occurs there, so an experiment-local calibration interception immediately before `pstvnc_local_controller_route()` is the narrow ownership seam;
- `h1_interaction_open_osk()` already uses `pstvnc_input_runtime_suspend_mouse_interpretation()`, pointer neutralization and `pstvnc_input_runtime_rebase_suspended_mouse_state()`;
- `h1_interaction_resume_desktop_mouse_if_ready()` already owns the qualified resume seam;
- the H1 coordinator service callback runs at the RFB safe-server-message boundary, which is the correct location for later framebuffer-request quiesce/resume policy.

This supports adapter-first integration without modifying qualified controller internals.

## Tests/checks and exact results

### Previously proven calibration-core host evidence

The calibration core was previously compiled with strict host warnings and passed:

`MPEG_CALIBRATION_HOST_TEST=PASS`

### New adapter test added this run

`h1_mpeg_calibration_adapter_test.c` covers:

- ordinary Select pass-through when START is not held;
- level-chord START+SELECT entry when only one button has a fresh edge;
- consumption/freezing while calibration is active;
- cancel sample consumption;
- continued ownership while Circle remains physically held;
- release-quarantine completion on the full-release sample;
- ordinary ownership returning on the following neutral sample.

`Makefile.host` now builds/runs both the core test and adapter test with:

`-std=c99 -O2 -Wall -Wextra -Werror -pedantic`

**Important evidence boundary:** the repository connector used in this run cannot execute the Pi/host compiler, so `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS` is **not claimed yet**. The test source/target is committed; execution remains the first verification action on an execution-capable checkout.

## Evidence / artifact paths

Repository-visible evidence paths:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter_test.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff file

No new ELF or execution log was produced in this run.

## Current ELF / PT_LOAD identity

No new PS2 ELF was built in this run. Therefore:

- new candidate ELF SHA256: **none yet**
- new candidate ELF bytes: **none yet**
- new candidate PT_LOAD SHA256/bytes: **none yet**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later evidence already recorded by the project; this calibration work remains host/source evidence only.

## Proven vs unproven

### Proven from repository/source evidence

- calibration remains experiment-local;
- `src/` and qualified controller/pad source were not changed by this tranche;
- the adapter uses the existing normalized controller vocabulary rather than PS2SDK constants;
- START+SELECT interception is now represented as an isolated experiment adapter rather than a new gesture inside the qualified local-controller module;
- ordinary Select can remain available to existing OSK routing outside calibration;
- the adapter explicitly models controller-dispatch suppression and RFB-visual-freeze ownership through release quarantine;
- the permanent outer-matte suppression geometry remains owned by the calibration core and separate from permanent composition.

### Still unproven

- compilation/execution of the new adapter host test;
- exact coordinator wiring of the adapter before `pstvnc_local_controller_route()`;
- suspension/rebase/resume calls driven by adapter enter/exit effects;
- RFB framebuffer-request freeze/resume at the existing safe-server-message boundary;
- native PS2 EDIT/CONTROLS/REVIEW renderer;
- atomic publication of accepted geometry to the MPEG presentation path;
- permanent compositor enforcement of the outer-matte suppression rectangle;
- combined RFB + mouse + OSK/keyboard + PCM + calibration + MPEG build/link;
- new ELF/PT_LOAD identity;
- physical PS2 behavior or hardware qualification.

## Important design decisions / reconciliation

- Do not add START+SELECT to `src/ui/local_controller.*`; that qualified module remains unchanged. The experiment adapter owns this temporary gesture.
- Detect the entry chord using `buttons_down`, not an assumption that START and SELECT press edges arrive simultaneously.
- The calibration adapter may state *that* RFB visuals must freeze, but it must not own RFB parser/request mechanics.
- Historical notes that allowed continuing RFB beneath MPEG remain historical evidence. Current experiment policy is:
  1. freeze new RFB visual work during calibration at the existing complete-server-message scheduling boundary while transport/session remains healthy;
  2. after accepted MPEG ownership, allow RFB elsewhere;
  3. forbid RFB presentation from the outer edge of the outer matte inward.
- Calibration and the permanent MPEG presentation/compositor backend remain separate modules.

## Current overall push status / milestone

Milestone reached: **portable calibration core + normalized-controller adapter are now represented on the experimental branch.**

The push is not yet at a hardware-testable ELF. The next integration ledge is coordinator ownership wiring, followed by RFB quiesce and native rendering.

## Single best next action

On an execution-capable checkout, first run the updated portable tests exactly:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Preserve the exact output. If both core and adapter tests pass, wire `pstvnc_h1_mpeg_calibration_adapter_service_controller()` into the experiment-local H1 interaction coordinator immediately before ordinary `pstvnc_local_controller_route()`.

That coordinator patch should, in one coherent tranche:

1. add the adapter to the H1 coordinator state and initialize it with the actual display dimensions;
2. service calibration first for every normalized controller-state observation;
3. if `consume_controller_state` is true, do not route that observation through ordinary local-controller/OSK behavior;
4. on `enter_foreground`, invoke the existing input-runtime suspension + pointer neutralization + rebase pattern already used for OSK ownership, without changing qualified controller code;
5. keep normal desktop mouse interpretation suspended through calibration release quarantine;
6. expose/propagate `freeze_rfb_visuals` to the current safe-server-message-boundary service path, but do not yet invent parser-level pause logic;
7. leave native calibration rendering and permanent compositor clipping as separate subsequent modules unless the current tranche can prove them without broadening ownership.

After that patch: run strict host checks, build the exact PS2 candidate only when all required current CP2P source is reconciled, and record source HEAD, ELF SHA256/bytes and PT_LOAD SHA256/bytes before any hardware claim.
