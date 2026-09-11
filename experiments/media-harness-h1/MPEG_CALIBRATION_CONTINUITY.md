# MPEG calibration continuity handoff

## Session identity

- Session timestamp: 2026-09-11 11:44 EDT run
- Branch: `experiment/h1-rfb-mux-prep`
- Session-start pushed HEAD: `22a29413e57b8bdf48d212d31b946e0f8b783a7f`
- Pre-handoff implementation HEAD: `ad567fbe2a30e1f80e4c535a2f425d4b761eb727`
- Calibration core: `6b255836ac15f069f432e82a9697de24555f5447`
- Controller-adapter tranche: `0d6bb3ef07af4aac10586c1ee743ae1a44c577a3`, `686de8cf8d31329970a54e371eaa98c1c98b688f`, `b8af43c087df4f002b7f24c73fc28638e5a28b31`, `514dacbc52f66c7bff43c6a94cf7b851dd139610`
- Foreground-ownership tranche: `afab6a7bcc6a17846faef9c1ad0a2704865c55ae`, `64af87a2326690eb8dcea9a7ec9855cfde2d1441`, `9589d1fdeaca6497765fa7a33dd88ac5b0fb3e60`, `8c2d82bf5e3c27e185607bb4c77751ad05bfd98a`
- RFB-gate tranche this run:
  - `689e9bb5e746e033f2a0243e08f7861588f5d5c4` — RFB gate interface
  - `f22aec4bf4794fd34f3180d396c0f5bc2650ba72` — RFB gate implementation
  - `dd79e31e48811f894fbddc8c1d00f54e68c2fc7a` — RFB gate host-contract test
  - `ad567fbe2a30e1f80e4c535a2f425d4b761eb727` — host target
- Two transient coordinator-header commits (`cea121ba...` then `ac8dd36f...`) intentionally cancel each other. Net coordinator header content is unchanged; no incomplete coordinator field/API was left behind.
- Handoff/docs commit: the commit containing this file; verify final branch HEAD after fetch.
- Repository-visible pushed state was inspected before writes. Local Pi staged/unstaged/untracked state is not observable through the GitHub connector, so do not infer a clean Pi worktree before build/deploy.

## Larger push objective

Prepare a hardware-testable H1 ELF combining qualified visible RFB interaction, PCM audio, and MPEG presentation with native PS2 calibration. Calibration remains experiment-local and separate from the permanent MPEG compositor/presentation backend.

Temporary entry gesture: held `START+SELECT`.

During calibration:

- keep PSTV/RFB protocol state healthy;
- finish any in-progress RFB server message to the existing complete-message scheduling boundary;
- issue no new framebuffer-update requests;
- present no new RFB visual updates;
- retain the last authoritative desktop image;
- suppress ordinary mouse/controller/local-UI dispatch while physical controller polling continues;
- render calibration locally.

After accepted MPEG ownership begins, RFB may resume elsewhere, but from the **outer edge of the outer matte inward** RFB presentation is forbidden.

## This session objective

Advance the RFB-quiesce side of the integration without touching parser/framing internals or qualified `src/` code. Isolate a deterministic experiment-local gate that can be evaluated only at the existing safe server-message boundary.

## What changed this run

Added:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_gate.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_gate.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_gate_test.c`

Updated:

- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`

No `src/` file, qualified controller/input/local-UI module, RFB parser/session implementation, or permanent MPEG compositor implementation was modified.

## Repository/source evidence reconciled this run

The exact pushed H1 coordinator was re-read before implementation.

Current authority still shows:

- normalized controller observations reach `h1_interaction_service_controller_state()` immediately before `pstvnc_local_controller_route()`;
- `h1_interaction_open_osk()` already owns the qualified `suspend -> neutralize -> rebase` sequence;
- the coordinator service callback is the application-service seam associated with the RFB safe-server-message boundary;
- the existing input-runtime suspension keeps physical controller sampling alive while mouse interpretation is suspended.

The attempted direct coordinator-header wiring was deliberately backed out in the same run because repository-only access cannot compile the resulting full coordinator/ELF integration. The net header is unchanged. This preserves a clean boundary until the new gate and existing foreground bridge can be compiled together on an execution-capable checkout.

## RFB gate contract now represented in code

`h1_mpeg_calibration_rfb_gate.*` owns scheduling policy only. It does not own sockets, parser/framing, framebuffer storage, GS rendering, or MPEG composition.

At a complete-server-message scheduling boundary:

- `frozen=1` means issue no new RFB framebuffer requests and present no new remote visual updates;
- repeated frozen observations are idempotent;
- `frozen->thawed` records exactly one required full refresh;
- the one-shot refresh is withheld while frozen and consumed exactly once after thaw.

The full-refresh-on-thaw rule deliberately avoids depending on incremental dirty-region history that may have accumulated while calibration retained the last authoritative displayed desktop.

## Tests/checks and exact evidence boundary

Previously recorded strict host execution:

- `MPEG_CALIBRATION_HOST_TEST=PASS`

Committed but not executed in this repository-only run:

- adapter host test (`MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS` not yet claimed);
- foreground host test (`MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS` not yet claimed);
- new RFB gate host test (`MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS` not yet claimed).

`Makefile.host` now builds/runs all four tests with:

`-std=c99 -O2 -Wall -Wextra -Werror -pedantic`

No compiler, Pi execution, ELF build, deployment, or physical PS2 observation occurred in this run.

## Evidence / artifact paths

Repository-visible evidence:

- `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_*`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_adapter.{h,c}`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_foreground.{h,c}`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_gate.{h,c}`
- corresponding host tests
- `experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host`
- this handoff

## Current ELF / PT_LOAD identity

No new PS2 ELF was built.

- candidate ELF SHA256: **none yet**
- candidate ELF bytes: **none yet**
- candidate PT_LOAD SHA256/bytes: **none yet**

Hardware authority remains with the previously qualified interaction/CP2N lineage and later recorded evidence. Calibration remains source/host-contract evidence until a new PT_LOAD-changing candidate is built and physically qualified.

## Proven vs unproven

### Proven from repository/source evidence

- calibration remains experiment-local;
- `src/` and qualified controller/input/local-UI/RFB implementations remain unchanged;
- START+SELECT meaning stays outside qualified `local_controller`;
- normalized controller facts feed a separate calibration adapter;
- an explicit foreground bridge models the qualified input suspension/rebase/resume lifecycle and release quarantine;
- an explicit RFB gate now models request/presentation freeze and one-shot full refresh on thaw;
- RFB freeze remains application scheduling policy rather than parser/framing behavior;
- outer-matte suppression geometry remains separate from permanent MPEG/RFB composition.

### Still unproven

- execution of adapter, foreground, and RFB-gate host tests on an execution-capable checkout;
- direct coordinator call-site wiring before `pstvnc_local_controller_route()`;
- binding foreground callbacks to live coordinator suspend/neutralize/rebase/resume functions;
- binding foreground freeze state to the RFB gate at the actual safe-server-message boundary;
- suppressing actual framebuffer request generation and remote presentation through that gate;
- native PS2 EDIT/CONTROLS/REVIEW renderer;
- atomic accepted-geometry publication to MPEG presentation;
- permanent compositor enforcement of the outer-matte suppression rectangle;
- combined RFB + interaction + PCM + calibration + MPEG build/link;
- candidate ELF/PT_LOAD identity;
- physical PS2 behavior and hardware qualification.

## Important design decisions / reconciliation

- Do not put START+SELECT into `src/ui/local_controller.*`.
- Entry uses authoritative `buttons_down`, not paired press edges.
- Calibration reuses the qualified input-runtime ownership lifecycle rather than inventing another controller mechanism.
- RFB quiescence is evaluated only at an existing complete-server-message scheduling boundary.
- Calibration does not disconnect RFB as its normal freeze mechanism.
- Thaw requires one full/nonincremental desktop refresh before returning to ordinary incremental assumptions.
- Historical notes allowing continued RFB under MPEG remain historical. Current experiment policy freezes new RFB visual work throughout calibration, then permits RFB elsewhere after accepted MPEG ownership while forbidding presentation from the outer edge of the outer matte inward.
- Calibration and permanent MPEG presentation/composition remain separate owners.

## Current overall push status / milestone

Milestone reached: **portable calibration core + normalized-controller adapter + foreground-ownership bridge + isolated safe-boundary RFB gate are represented on the experimental branch.**

The remaining path to a hardware-testable ELF is increasingly integration-specific rather than behavioral: compile the four host contracts, bind foreground ownership at the coordinator call site, bind the RFB gate at the live worker scheduling boundary, add native calibration rendering, then build/identify the PT_LOAD-changing ELF.

## Single best next action

On an execution-capable checkout, run exactly:

`make -C experiments/media-harness-h1/mpeg_presentation_calibration -f Makefile.host clean test`

Preserve the exact output. Expected PASS lines:

- `MPEG_CALIBRATION_HOST_TEST=PASS`
- `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS`
- `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS`

If all four pass, perform one experiment-local wiring tranche against the exact current checkout:

1. add foreground bridge + RFB gate state to H1 experiment coordinator/runtime state;
2. service the foreground bridge immediately before ordinary `pstvnc_local_controller_route()` when desktop/calibration owns the controller observation;
3. bind its callbacks to the existing coordinator suspend, pointer-neutralization, rebase, and resume lifecycle;
4. consume calibration-owned controller observations before ordinary local-controller/OSK routing;
5. at the existing complete-server-message application-service boundary, copy the foreground `freeze_rfb_visuals` fact into the RFB gate;
6. when gate is frozen, do not generate a new framebuffer request and do not invoke remote-frame presentation; keep session/protocol service alive;
7. when gate thaws, consume its one-shot full-refresh requirement and issue a nonincremental framebuffer update request before returning to normal incremental request policy;
8. do not pause parser internals, do not disconnect RFB, and do not modify `src/`.

After that compiles/tests, the next PT_LOAD-changing ledge is native PS2 calibration rendering plus build integration, followed immediately by exact ELF/PT_LOAD identity generation before any hardware claim.
