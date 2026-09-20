# Reconstruction shift — A004 Presentation ownership / first-frame R3

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T18:14:26-04:00
COMPLETED_AT=2026-09-20T18:24:23-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2f16a2b9280e41f7b4b74fac3b7f9d3bc420a612
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute Foreman packet
`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3` only.

The bounded goal was to reconstruct the Presentation-owned start-side visible
ownership core:

`RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED`

The owner had to consume one already-resolved geometry snapshot and one
caller-supplied exact nonzero MPEG run/generation identity, keep those facts
immutable for the run, reject stale/wrong lifecycle events, distinguish the
WAIT_FIRST_FRAME ownership gap from real MPEG ownership, and leave current-Q7
retirement attachable without restoring H1 direct-stop behavior.

This shift did not begin the physical compositor/clock-arm tranche.

## Authority and execution classification

Consumed current authority including:

- Foreman state revision 0025;
- Reconstruction Contract revision 0006;
- work-log contract revision 0006 / log format revision 0001;
- Wire runtime decisions revision 0011, especially Q6/Q7;
- architecture overlay revision 0004;
- A003 MPEG-generation audit revision 0001;
- A004 presentation/calibration audit revision 0001;
- current clean architecture and source-topology/naming policy.

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No live-Pi observation or physical PS2 action was required by this packet.
Repository archaeology, writes, commits, dictionary maintenance, CI execution
and workflow/log inspection were performed through GitHub/GitHub Actions. No
user-proxy terminal work was requested.

Starting branch authority was:

`2f16a2b9280e41f7b4b74fac3b7f9d3bc420a612`

The starting authority's canonical workflow run `35540884626` was already
SUCCESS.

## Current clean source inspected

Inspected the current owners/seams needed to preserve domain boundaries:

- `src/display/display.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/ui/mpeg_calibration.{c,h}`;
- `src/rfb/flow_policy.{c,h}`;
- `src/media/clock.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- current Display symbol dictionary and canonical host-test registration.

Recovered current facts:

- Display already owns platform-neutral presentation-facing geometry/state above
  the PS2 graphics seam.
- UI calibration owns editable/accepted calibration meaning and already resolves
  distinct base, inner-content/matte and suppression rectangles.
- Platform owns the physical GS/dmaKit path and synchronized flip.
- P2 RFB policy owns generic global freeze/thaw/request/full-refresh debt.
- Media owns the common clock API, while A003/A004 require its arm callsite at
  the first real synchronized MPEG presentation boundary.
- MPEG decoder explicitly does not own compositor/clock policy.

These facts make a small pure `src/display/` owner the narrowest correct P3
placement. Display does not import the UI calibration type; Application can
later translate an accepted calibration result into the neutral Display value
at the orchestration boundary.

## Frozen H1 forensic trace

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Inspected:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.{c,h}`;
- its host test;
- `h1_mpeg_start_handoff.{c,h}`;
- its host test;
- calibration geometry helpers far enough to recover base/inner/suppression
  relationships;
- current audits far enough to distinguish historical direct stop from current
  Q7.

Recovered semantics:

- H1 already distinguished RFB_ONLY, WAIT_FIRST_FRAME and MPEG_OWNED.
- WAIT_FIRST_FRAME retained the previously visible RFB desktop rather than
  claiming MPEG ownership.
- exact-generation first-frame presentation promoted visible ownership.
- base geometry remained unchanged by inner/outer mattes;
  inner matte remained presentation-local;
  outer footprint became suppression geometry.
- H1 start handoff and presentation owner internally minted generations.
- H1 also exposed a direct active `stop()` from MPEG_OWNED to RFB_ONLY.

The last two mechanisms were deliberately **not** carried forward:
current Q6 leaves run/generation identity with the surrounding lifecycle owner,
and current Q7 requires retirement to close production, permit overlapped RFB
restoration underneath still-visible MPEG, then remove MPEG/mattes only at the
safe final boundary.

## Placement and value ownership

Added a pure Display-owned owner:

- `src/display/mpeg_presentation.h`
- `src/display/mpeg_presentation.c`

The public resolved value is:

- base rectangle — exact MPEG capture/presentation geometry;
- inner-content rectangle — presentation-local result of symmetric inner matte;
- suppression rectangle — outer visual-ownership/RFB-suppression footprint.

This is not a second calibration store. It is one immutable run snapshot of an
already-resolved value supplied by the later orchestration owner.

The owner contains:

- visible state;
- copied resolved geometry;
- copied caller-owned run generation;
- snapshot-valid fact.

It imports no UI, RFB, Transport, MPEG decoder, Platform or media-clock header.

## Generation ownership

`pstvnc_mpeg_presentation_arm()` requires one nonzero generation supplied by
the caller and copies it verbatim.

Presentation never allocates, increments or redefines generation identity.

Nested arm is rejected. Wrong, zero, stale and duplicate first-frame events are
rejected without replacing live state. Pending abort is similarly exact-run
guarded.

After pending abort, a later caller may arm a distinct generation. A stale event
from the aborted run cannot promote the new run.

## Visible ownership contract

Initial state:

- state = RFB_ONLY;
- mode = DIRECT_RFB;
- no MPEG visual ownership;
- no global RFB protection requirement;
- no live run snapshot.

Valid arm:

- only legal from RFB_ONLY;
- snapshots geometry and exact caller generation;
- enters WAIT_FIRST_FRAME;
- mode = FROZEN_RFB_DESKTOP;
- reports generic global RFB protection required;
- explicitly does not claim MPEG visual ownership.

Exact first-frame event:

- only legal for the stored nonzero generation while WAIT_FIRST_FRAME;
- transitions exactly once to MPEG_OWNED;
- mode = COMPOSITED;
- global RFB freeze/protection is no longer required;
- exact generation and suppression geometry remain intact for later composition
  and current-Q7 retirement;
- state transition itself performs no physical draw/flip/upload and no media
  clock arm.

Pending abort:

- only exact WAIT_FIRST_FRAME generation may abort;
- returns to RFB_ONLY;
- clears the pending run snapshot;
- owns no P2 FULL-refresh debt.

There is deliberately **no active-stop/retire API** in this P3 owner.

## Geometry validation

Because P3 consumes an already-resolved geometry rather than editable
calibration state, it validates consistency without inventing a second canvas or
configuration authority:

- base x/y are nonnegative;
- base width/height are at least 16 and 16-pixel aligned;
- arithmetic for rectangle ends is widened;
- inner content must remain inside base;
- inner left/right and top/bottom insets must be symmetric;
- suppression must be nonempty and contain the complete base footprint.

No independent canvas maximum was invented in Presentation; upstream resolved
geometry remains the source of desktop bounds.

## Behavior/test commit

`abe684c79cf49f856d3d72b3fe5523a815a4034b` —
`feat(display): add MPEG presentation ownership core`

Changed/added:

- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_presentation.h`;
- `tests/unit/mpeg_presentation_test.c`;
- `tests/Makefile`.

The focused fixture covers:

- initial RFB_ONLY state/mode;
- immutable copied geometry and caller generation;
- nested-arm rejection;
- WAIT_FIRST_FRAME frozen-desktop/global-protection semantics;
- zero/wrong exact-generation first-frame rejection;
- one exact promotion to MPEG_OWNED;
- COMPOSITED mode and release of global freeze requirement;
- retention of suppression/run snapshot after promotion;
- exact pending abort and stale abort rejection;
- stale old generation versus later run;
- representative geometry/alignment/suppression failures;
- explicit absence of a P3 active-stop path.

## Dictionary/tooling integration defect and repair

The first behavior-head checks correctly exposed missing dictionary coverage for
the two new Display files.

An initial tree-identical canonical reconciliation trigger was committed:

`d4035cce00d4bbef297b2018c1839767fdc2beed`

Its workflow proved the canonical reconciliation helper did not target
`src/display`; the helper's hard-coded maintenance set predated this P3 need.

A first tooling repair:

`bfd2d0ffc54ef7100ea64b1fea65716a4bf04560` —
`fix(tooling): reconcile all clean source dictionaries`

temporarily generalized the helper/workflow to all topology directories.

A second tree-identical trigger:

`e6ebb4adf2280936f511612999a8793cd296c28b`

allowed the repaired automation to run.

The automation generated:

`aa367034fa874d854209836e3133f45de5f68caa` —
`docs(symbols): reconcile current clean definitions`

It correctly added 96 Display definition rows, but broad reconciliation also
removed two existing Diagnostics dictionary rows for live GNU
`__typeof__(sendto)` linker-alias declarations. Current diagnostics source
still contains those declarations; the baseline strict dictionary audit had
accepted their maintained rows. That collateral removal was therefore not
accepted as P3 cleanup.

Final integration correction:

`b8abd1a89f39710d99691e89e7e24ef0dcc75509` —
`fix(tooling): preserve unsupported dictionary metadata`

This:

- restored `src/diagnostics/SYMBOLS.md` exactly to its pre-reconciliation
  content;
- restored the generated portal's Diagnostics count to 69;
- retained the 96 generated P3 Display rows, giving Display 113 symbols;
- refined the Display responsibility prose to name visible-ownership state;
- narrowed the deterministic helper to its prior safe maintenance set plus
  Display rather than mechanically rewriting specialized domains whose syntax
  its current matcher cannot safely preserve;
- registered `src/display/SYMBOLS.md` in the workflow's reconciliation commit
  set.

This tooling repair changes development infrastructure only, not product
runtime behavior.

## Final exact changed-path boundary

Comparison from Foreman base
`2f16a2b9280e41f7b4b74fac3b7f9d3bc420a612` through final pre-log source
authority
`b8abd1a89f39710d99691e89e7e24ef0dcc75509`
contains exactly eight changed paths:

- `.github/workflows/ledge-reconstruction.yml`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `scripts/reconcile-source-dictionaries.py`;
- `src/display/SYMBOLS.md`;
- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_presentation.h`;
- `tests/Makefile`;
- `tests/unit/mpeg_presentation_test.c`.

No Application, UI calibration, RFB, Transport, MPEG decoder, Platform,
media-clock, framebuffer or input product source changed.

Final static boundary inspection confirms the new Presentation owner contains:

- no UI/calibration include;
- no RFB include;
- no Transport include/API call;
- no MPEG decoder include/API call;
- no Platform/PS2 graphics include/API call;
- no media-clock include/API call;
- no `pstvnc_ps2_graphics_present()`;
- no `pstvnc_media_clock_arm()`;
- no active presentation stop/retire API.

## CI / verification evidence

### Baseline

Run `35540884626` at starting authority `2f16a2b928...`:
SUCCESS.

### First P3 behavior head

Run `35541244511` at
`abe684c79cf49f856d3d72b3fe5523a815a4034b`:

- host-unit — PASS;
- `MPEG_PRESENTATION_TEST=PASS`;
- `transport_runtime_test: PASS`;
- ps2-compile repository gate — PASS;
- ps2-link/current-source reproducibility repository gate — PASS;
- project-check — FAIL only because the two new Display files did not yet have
  local dictionary coverage;
- dictionary-long — FAIL for the same 96 missing definitions / file coverage;
- dictionary-reconcile — SKIPPED as expected on a normal behavior commit.

### First reconciliation trigger

Run `35541282950` at `d4035cce...` exposed the canonical tooling defect:
dictionary-reconcile failed because the helper did not target Display.

No product behavior was changed in response.

### Repaired reconciliation trigger

Run `35541407998` at `e6ebb4ad...` used the generalized helper.
Its reconciliation step completed and strict dictionary validation passed,
reporting:

- Display: before=17, removed=0, added=96;
- Diagnostics: before=69, removed=2, added=0;
- all other domains: no changes.

The Diagnostics removals were independently reviewed and rejected as collateral
metadata loss; they were restored by `b8abd1...`.

### Final coherent source/tooling head

Run `35541518936` at
`b8abd1a89f39710d99691e89e7e24ef0dcc75509`:
SUCCESS.

Observed jobs:

- host-unit — PASS, including:
  - `MPEG_PRESENTATION_TEST=PASS`;
  - `transport_runtime_test: PASS`;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile repository gate — PASS;
- ps2-link/current-source reproducibility repository gate — PASS;
- dictionary-reconcile — SKIPPED as expected on the coherent normal commit.

### PS2 evidence classification

The repository's broad PS2 compile/link workflows passed, but their current
stage-local source lists do not directly compile/link this newly added, still
unwired P3 module. They establish that the existing current product build was
not broken by the P3 repository changes; they are **not** direct PS2 execution
or hardware evidence for `mpeg_presentation.c`.

Direct executable evidence for the new P3 owner is its canonical host unit
fixture.

No physical PS2 run was performed or claimed.

## A004 P3 worker acceptance disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A004-P3-C1 OWNER_BOUNDARY — MET

Visible MPEG ownership resides in `src/display/`. No forbidden domain
dependency was introduced.

### A004-P3-C2 RESOLVED_GEOMETRY — MET

One immutable Display-owned start snapshot preserves base, inner-content/matte
and suppression meanings. It is a resolved run snapshot rather than a second
editable calibration owner/store.

### A004-P3-C3 GENERATION_FENCE — MET

Generation is caller-supplied and nonzero. Presentation never mints it.
Nested arm, zero/wrong/stale/duplicate lifecycle events fail closed.

### A004-P3-C4 WAIT_FIRST_FRAME — MET

Valid arm enters a real WAIT_FIRST_FRAME gap that explicitly reports frozen RFB
desktop/global protection and does not report MPEG visual ownership.

### A004-P3-C5 FIRST_FRAME_PROMOTION — MET

Only the exact stored generation's explicit first-frame-presented event promotes
WAIT_FIRST_FRAME to MPEG_OWNED, exactly once.

This is a source-level event contract, not proof that a frame physically reached
a television.

### A004-P3-C6 COMPOSITED_MODE — MET

MPEG_OWNED reports COMPOSITED, no longer requires the global P2 freeze, and
retains exact generation/suppression geometry so RFB truth may later advance
under presentation-local suppression.

### A004-P3-C7 ABORT_PENDING — MET

Only exact WAIT_FIRST_FRAME abort returns to RFB_ONLY and clears the pending
snapshot. Stale aborts fail closed. Presentation creates no RFB refresh debt.

### A004-P3-C8 Q7_RETIREMENT_GUARD — MET

H1's direct MPEG_OWNED -> RFB_ONLY stop was not reconstructed. Active ownership
has no stop/retire API in P3, so the exact run/suppression snapshot cannot be
discarded by an early direct-stop path. Current-Q7 retirement remains a later
ordered process.

### A004-P3-C9 CLOCK_PHYSICAL_BOUNDARY — MET

P3 calls no graphics, decoder, Transport or clock API. Its public contract says
the promotion event belongs to the later mechanism only after the exact first
MPEG frame crosses the real synchronized physical presentation boundary.

No state-only operation arms the media clock or claims physical presentation.

### A004-P3-C10 CLEAN_INTEGRATION — MET

Focused P3 host tests, full host suite, project check and strict dictionary
audit pass on the final coherent head. Broad repository PS2 compile/link checks
also pass with the evidence limitation above. No hardware qualification is
claimed.

## Ambiguities deliberately preserved

- P3 does not choose or allocate MPEG run generations.
- P3 does not decide how UI calibration is converted into the neutral Display
  geometry value; later Application orchestration owns cross-domain translation.
- P3 does not own the P2 freeze/thaw callsite or FULL-refresh debt.
- P3 does not decide exact physical GS texture/compositor representation.
- P3 does not define the media-clock arm callsite beyond preserving that it must
  be attached to the future real first synchronized physical presentation.
- P3 does not define active retirement, overlapped RFB restoration, final reveal,
  scheduler or drop policy.
- P3 does not invent a canvas maximum after receiving already-resolved geometry.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A004_P3_REQUIRED_REPOSITORY_AND_HOST_GATES

P3_PRODUCT_RUNTIME_WIRING=NOT_IN_PACKET
P3_PHYSICAL_COMPOSITOR=NOT_IMPLEMENTED
P3_MEDIA_CLOCK_ARM_CALLSITE=NOT_IMPLEMENTED
P3_Q7_ACTIVE_RETIREMENT=NOT_IMPLEMENTED
P3_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
P3_PHYSICAL_PS2_RUNTIME=NOT_RUN_NOT_CLAIMED
HARDWARE_PENDING=YES_FOR_LATER_A004_VISUAL_AND_RUNTIME_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log source/tooling authority
`b8abd1a89f39710d99691e89e7e24ef0dcc75509` plus this immutable log commit.

Foreman should independently inspect P3 source, caller-generation semantics,
the absence of direct active-stop behavior, final CI evidence, and the bounded
dictionary-tooling repair, then decide the next A004 physical shared
compositor/first-synchronized-frame/media-clock-arm packet.

This Reconstruction worker must not begin that next tranche in this shift.
