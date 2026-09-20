# Foreman shift — accept A004 P3 and activate shared compositor P4

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T18:27:44-04:00
COMPLETED_AT=2026-09-20T18:34:21-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=309a87851c17f65600ee1d647022cb8b6276baab
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed
`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3` Reconstruction baton,
independently verify the worker's presentation owner, generation semantics,
dictionary/tooling repair and CI evidence, then publish the next bounded A004
physical compositor/first-synchronized-frame/common-clock packet without
performing Reconstruction product behavior from the Foreman seat.

## Authority refreshed

Live branch authority at Foreman pickup was:

`309a87851c17f65600ee1d647022cb8b6276baab`

The branch was seven commits ahead of prior Foreman authority
`2f16a2b9280e41f7b4b74fac3b7f9d3bc420a612`.

Newest relevant Reconstruction record:

`docs/ledge/work-log/20260920T181426-0400__reconstruction__a004-presentation__interactive.md`

Foreman state was still revision 0025. The worker correctly did not self-accept
P3 or publish a later packet.

## Worker commit review

Reviewed the exact worker range:

- `abe684c79cf49f856d3d72b3fe5523a815a4034b` —
  `feat(display): add MPEG presentation ownership core`;
- `d4035cce00d4bbef297b2018c1839767fdc2beed` —
  first tree-identical deterministic dictionary trigger;
- `bfd2d0ffc54ef7100ea64b1fea65716a4bf04560` —
  initial dictionary-helper repair;
- `e6ebb4adf2280936f511612999a8793cd296c28b` —
  second tree-identical reconciliation trigger;
- `aa367034fa874d854209836e3133f45de5f68caa` —
  automated dictionary reconciliation;
- `b8abd1a89f39710d99691e89e7e24ef0dcc75509` —
  bounded tooling correction preserving unsupported dictionary metadata;
- `309a87851c17f65600ee1d647022cb8b6276baab` —
  required immutable Reconstruction log.

Final source/tooling changes before the log are limited to:

- `src/display/mpeg_presentation.{c,h}`;
- `tests/unit/mpeg_presentation_test.c`;
- `tests/Makefile`;
- `src/display/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `scripts/reconcile-source-dictionaries.py`;
- `.github/workflows/ledge-reconstruction.yml`.

No Application, UI calibration, RFB, Transport, MPEG decoder, Platform,
media-clock, framebuffer or input product source changed.

## Independent P3 behavior review

Foreman independently inspected the final P3 source/header, focused host test,
worker commit range, final symbol authority and current Q7 constraints.

Confirmed:

- visible MPEG ownership belongs to `src/display/`;
- Presentation imports none of UI/calibration, RFB, Transport, MPEG decoder,
  Platform graphics or media-clock ownership;
- one already-resolved base/inner-content/suppression value is copied as the
  exact run snapshot rather than becoming a second editable calibration store;
- base dimensions remain >=16 and 16-pixel aligned;
- inner content is contained by the base and has symmetric insets;
- suppression is nonempty and contains the complete base;
- rectangle-end arithmetic is widened before comparison;
- run generation is supplied by the caller, must be nonzero, is copied
  verbatim, and is never minted/incremented by Presentation;
- nested arm cannot replace a live exact-run snapshot;
- WAIT_FIRST_FRAME is a real protected ownership gap:
  FROZEN_RFB_DESKTOP, generic global-RFB protection required, MPEG visual
  ownership false;
- only the exact stored generation's first-frame-presented event promotes to
  MPEG_OWNED;
- duplicate, stale, wrong and zero generation promotion fails closed;
- MPEG_OWNED is COMPOSITED, no longer requests global RFB freeze, and retains
  the exact run/suppression snapshot;
- exact pending abort is legal only in WAIT_FIRST_FRAME and owns no RFB
  full-refresh debt;
- there is no P3 active-stop/retire API and therefore no H1-style early
  MPEG_OWNED -> RFB_ONLY reveal;
- the state-only first-frame event performs no draw/flip/upload and no
  `pstvnc_media_clock_arm()`, preserving the real physical boundary for P4.

A004-P3-C1 through A004-P3-C10 are Foreman accepted as MET.

## Dictionary/tooling repair review

The first P3 source commit correctly made Display dictionary coverage incomplete.
The canonical reconciliation helper itself then proved incomplete because
`src/display` was absent from its maintenance target set.

A temporary broad helper pass removed two maintained Diagnostics rows for live
GNU `__typeof__(sendto)` alias declarations. Foreman independently verified
that the worker did not preserve that collateral rewrite.

At final authority:

- `src/diagnostics/SYMBOLS.md` is identical to Foreman base, with blob
  `5918063c0a27596ab558aa6160ec69856dd43e5c`;
- portal Diagnostics count is restored to 69;
- Display reports 113 symbols;
- compared with the prior reconciliation helper, the functional target-set
  change is adding `Path("src/display")`;
- helper documentation now explicitly says only known-safe C/H dictionaries are
  mechanically rewritten;
- the workflow adds only `src/display/SYMBOLS.md` to the authorized generated
  reconciliation commit set.

The final tooling repair is accepted as bounded integration work. No product
runtime behavior is introduced by it.

## Independent CI evidence

Pickup-head workflow:

- GitHub Actions run `35541637912`, run number 278;
- head `309a87851c17f65600ee1d647022cb8b6276baab`;
- conclusion SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Host logs explicitly show:

- `MPEG_PRESENTATION_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `transport_runtime_test: PASS`;
- the remaining canonical host suite passing.

The intermediate dictionary failures are retained as evidence of the integration
tooling defect and its repair rather than hidden.

No physical PS2 run occurred.

The current strict PS2 compile/link manifests still omit the new unwired P3
translation unit. Their green result proves no regression to the current linked
product, not direct PS2 compilation/linkage of P3. P4 explicitly closes that
evidence gap for presentation mechanism source it adopts.

## Next-seam archaeology

Foreman inspected current:

- `src/display/display.{c,h}`;
- `src/display/mpeg_presentation.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/platform/ps2_system.{c,h}`;
- `src/media/clock.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- current Application presentation path;
- clean architecture;
- A002/A003/A004 audits and current Q7;
- canonical PS2 compile/link manifests.

Foreman also inspected frozen H1 authority:

- `h1_cumulative39_graphics.{c,h}`;
- `h1_video_runtime.{c,h}`;
- `h1_media_clock.{c,h}`;
- `MEDIA_OBJECT_COMPOSITION_NOTES.md`.

Recovered qualified physical behavior:

- one graphics owner composes the latest remote desktop, black outer
  suppression, MPEG base, black symmetric inner matte and local overlay;
- MPEG decoder picture memory is RGB16 16x16 macroblock-order data;
- H1 detiled that data into one reusable linear video texture;
- remote desktop may continue changing outside/under suppression while MPEG is
  visible;
- local UI/OSK remains the final overlay;
- video presentation success is only returned after synchronized physical
  presentation;
- bounded video texture resources are reused across frames/generations;
- the qualified first real video boundary owns common-media-clock arming.

Historical H1 structure is not copied as authority:

- its graphics wrapper textually included another graphics C file;
- it used a semaphore because multiple callers competed for GS;
- it had an active direct `clear_video()` path;
- its video worker itself participated directly in GS calls.

Clean reconstruction instead retains the one existing
`src/platform/ps2_graphics` owner and the architecture rule that
Application/Presentation owns cross-domain side effects.

## Current seam gap

The current Platform graphics API knows only desktop plus optional local
overlay. It has no MPEG texture/matte/suppression mechanism.

The current clean MPEG decoder owns the decoded picture buffer and sequence
geometry but exposes no per-picture presentation handoff. P4 must therefore
define a neutral decoded-frame contract and physical compositor mechanism
without prematurely making the decoder worker a second GS caller.

The current media clock already accepts an explicit
`pstvnc_media_clock_arm(clock, observed_now_tick)` and is session-scoped.
P4 therefore needs a narrow physical synchronized-completion/timer observation
seam rather than a new clock implementation or a per-MPEG-run epoch.

## Packet issued

Foreman state revision 0026 publishes:

`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`

The packet requires:

- one existing Platform GS owner;
- explicit RGB16 macroblock frame semantics;
- exact P3 generation/geometry fencing;
- desktop -> suppression -> MPEG -> inner matte -> local overlay ordering;
- bounded reusable video presentation resources;
- a real post-`gsKit_sync_flip` completion/timer fact;
- one session-clock first arm only at exact first synchronized MPEG
  presentation;
- exact P3 first-frame promotion only after that same successful boundary;
- stable subsequent composited frames without rearm/repromotion;
- no direct active clear/stop shortcut under Q7;
- direct canonical PS2 compile/link coverage for adopted P3/P4 presentation
  mechanism source.

It explicitly excludes decoder-worker handoff, scheduler/drop, Q7 retirement,
P2 runtime wiring, calibration runtime wiring, START/producer transaction,
final Application orchestration and hardware qualification.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
A004_P3_SOURCE_REVIEW=PASS
A004_P3_C1_C10_FOREMAN_ACCEPTANCE=PASS
P3_TOOLING_REPAIR_REVIEW=PASS
A004_P3_FINAL_CI=PASS
CURRENT_Q7_COMPATIBILITY=PASS
NEXT_OWNER_SEAM=SINGLE_PHYSICAL_COMPOSITOR_FIRST_SYNC
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## State / contract revisions

Consumed:

- Foreman State revision 0025
- Reconstruction Contract revision 0006
- work-log contract revision 0006
- Wire runtime decisions revision 0011
- architecture overlay revision 0004
- A002 audit revision 0001
- A003 audit revision 0001
- A004 audit revision 0001

Produced:

- Foreman State revision 0026
- packet `A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
HARDWARE_PENDING=YES_FOR_LATER_A004_VISUAL_AV_TIMING_AND_ENDURANCE_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

## Next pickup

Interactive Reconstruction executes
`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`, emits exactly one immutable
`a004-presentation` Reconstruction log, and returns the baton to Foreman.

Do not begin scheduler/drop, current-Q7 retirement, decoder-worker handoff or
final Application orchestration in the same worker shift.
