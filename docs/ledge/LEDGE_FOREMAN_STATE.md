# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0026
RECORDED_AT=2026-09-20T18:33:18-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0025
SUPERSEDES_FOREMAN_STATE_REVISION=0025
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0026 independently accepts the completed
`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3` Reconstruction baton and advances
A004 into the single physical compositor / first synchronized MPEG presentation
/ shared-media-clock arm seam. It also accepts the worker's bounded dictionary
maintenance repair after independently confirming that the final change merely
adds Display to the known-safe reconciliation set and restores specialized
Diagnostics metadata exactly.

Revision 0025 independently accepts the completed
`A004-RFB-FREEZE-REFRESH-R2` Reconstruction baton and advances A004 into the
presentation-owned start/WAIT_FIRST_FRAME/first-frame ownership seam. The next
packet deliberately does not restore H1's direct active-stop behavior because
current Q7 requires retirement to remain a later ordered transition.

Revision 0024 is an execution-policy correction only. It adopts Reconstruction
Contract revision 0006, applies autonomous repository execution to the currently
active A004 P2 packet, and leaves that packet's engineering scope, required
behavior, C1-C9 acceptance criteria, invariants, non-goals, dependency ordering,
and worker role unchanged.

Revision 0023 independently accepts the completed
`A004-MPEG-CALIBRATION-CORE-R1` Reconstruction baton and advances A004 into
the next bounded owner seam: generic RFB freeze/request/full-refresh policy at
the already-proven RFB safe scheduling boundary.

## Current Foreman phase

`A004_P3_INTEGRATED__A004_SHARED_COMPOSITOR_FIRST_SYNC_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_PRESENTATION_SEAMS`

ARCHITECTURE_BLOCKER=NONE
A004_P1_FOREMAN_ACCEPTED=YES
A004_P2_FOREMAN_ACCEPTED=YES
A004_P3_FOREMAN_ACCEPTED=YES
A004_P4_ACTIVE=YES
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## A004 P1 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe`;
- final pre-log source/dictionary authority
  `094f07fff17801152bcd0d17ac69c570d8bfad27`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T171848-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed five commits after Foreman base `672f67eb...`:

- `ad00a7a07c15c8d56f89e5ac664e07507e50c312` —
  `feat(ui): add MPEG calibration core`;
- `51025be8553c3dad8b5a11002fb57f9bb7a5af62` —
  `docs(symbols): index MPEG calibration core`;
- `3d9f4a821f16d51a3d22597b4a539009aac7705f` —
  `refactor(ui): name MPEG calibration action enum`;
- `094f07fff17801152bcd0d17ac69c570d8bfad27` —
  `docs(symbols): bind calibration actions to named enum`;
- `b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe` —
  the required immutable Reconstruction work log.

Independent source review confirms:

- MPEG CALIBRATION is separate from DESKTOP CALIBRATION;
- `src/ui/mpeg_calibration.{c,h}` owns only local calibration value/state;
- base, inner-content/matte, and outer/suppression meanings remain distinct;
- width/height are bounded and 16-pixel aligned while x/y remain pixel precise;
- geometry calculations use widened arithmetic before bounded int32 storage;
- REVIEW accept remains unarmed while X/accept is held and requires release
  followed by a fresh press;
- successful acceptance copies one immutable caller-visible region and does not
  imply START, producer admission, decode, first presentation, clock arm, or
  MPEG visual ownership;
- cancel publishes no false accepted region and exit remains quarantined until
  all calibration actions are released;
- no RFB, Transport, MPEG decoder, Display, Pi, GS, or Application dependency
  was imported into the calibration owner;
- no new top-level source directory was created.

A004-P1-C1 through A004-P1-C9 are accepted as MET within the packet's bounded
source scope.

## Independent P1 evidence

GitHub Actions run `35539055477` at pickup HEAD
`b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe` completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected after already-coherent dictionaries.

These are machine/repository checks. They do not constitute physical PS2
qualification.

No physical hardware run was performed for A004 P1.

## Forward-authority correction for A004

`docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001 is a
2026-09-15 temporal audit tranche and remains historically truthful. It must not
be destructively rewritten merely because later architecture work refined
retirement semantics.

For forward reconstruction, Q7 in
`docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011 is governing
where it conflicts with the older audit's retirement wording.

Current Q7 requires:

1. MPEG retirement first closes new MPEG production/admission;
2. already accepted MPEG may drain;
3. RFB suppression may then be released so RFB can request, receive and rebuild
   the underlying desktop while valid retiring MPEG/mattes remain visible;
4. final safe MPEG retirement removes MPEG/mattes and reveals the already
   refreshing RFB desktop.

Therefore **active or retiring MPEG is not equivalent to a global RFB freeze**.
The global freeze reconstructed by P2 is for ownership gaps such as calibration
foreground and WAIT_FIRST_FRAME-style protection. Later presentation ownership
will use region-local suppression/composition while RFB framebuffer truth may
continue to advance.

## Why P2 is RFB flow protection before presentation ownership

Current clean RFB already owns:

- exact request serialization through
  `pstvnc_rfb_session_request_update()`;
- complete server-message parsing;
- a nonblocking live service boundary that returns IDLE only before consuming
  any byte of the next server message;
- parser-safe finite-session quiesce at that same complete-message boundary.

Current Application still drives a simple one-request/one-response cadence and
has no MPEG lifecycle.

Frozen H1 evidence separately recovered:

- a calibration RFB gate that blocks new requests and remote presentation while
  frozen without destroying the RFB/PSTV session;
- one outstanding framebuffer request at a time;
- an in-flight pre-freeze response still completing protocol ownership while
  visual publication remains suppressed;
- one coalesced FULL/nonincremental request after thaw before normal
  incremental cadence resumes.

Those facts form a small RFB-owned policy seam that can be reconstructed before
the later presentation owner exists. This avoids putting MPEG/calibration types
inside RFB and avoids inventing temporary Application callbacks.

## A004 P2 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `773cdd905cde5a0c38a1e732c10d4d5f2d123c46`;
- final pre-log source/dictionary authority
  `5aa84f1dc0e3017b88214e20637ab1a0605cc38a`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T175245-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed five commits after Foreman authority `aaa452899...`:

- `7e779fdb113ee37dfda1e60f4887c7d7c608037a` —
  generic RFB freeze/refresh flow policy plus focused host contract;
- `7600d77f158892d1b850c686371cb3c9e72d67b1` —
  tree-identical deterministic dictionary-reconciliation trigger;
- `a64b6427356ff1c2fcbc87f2e40cd126048b7589` —
  automated clean-symbol reconciliation;
- `5aa84f1dc0e3017b88214e20637ab1a0605cc38a` —
  RFB domain responsibility prose;
- `773cdd905cde5a0c38a1e732c10d4d5f2d123c46` —
  required immutable Reconstruction work log.

Independent source review confirms:

- `src/rfb/flow_policy.{c,h}` contains only generic RFB live-flow facts:
  global frozen/thawed state, one outstanding request, one pending FULL debt,
  request choice and publication permission;
- no MPEG, calibration, decoder, producer, GS, presentation-owner, Transport
  lifecycle or Application type/header is imported;
- `next_request()` is non-mutating, so merely inspecting policy cannot consume
  FULL debt;
- FULL debt is cleared only after the currently owed FULL request is recorded as
  successfully sent;
- freeze while one request is outstanding does not discard protocol ownership;
  the response may complete at the existing RFB complete-message boundary and
  clear that outstanding fact while visual publication remains blocked;
- repeated freeze/thaw intent before recovery advances coalesces, while a later
  genuine interval after recovery send creates distinct debt;
- existing strict initial-frame authority and parser/session source remain
  unchanged;
- current Q7 remains possible because active/retiring MPEG is not represented
  inside the global RFB freeze policy.

A004-P2-C1 through A004-P2-C9 are accepted as MET within the packet's bounded
source scope.

GitHub Actions run `35540415181` at pickup HEAD completed SUCCESS.
Host evidence includes both `RFB_FLOW_POLICY_TEST=PASS` and
`transport_runtime_test: PASS`; project-check, strict dictionary, PS2 compile
and PS2 link/current-source reproducibility also passed.

The earlier behavior-head run retained another occurrence of the already-seen
timing-sensitive `transport_runtime_test` assertions at lines 787/789. No
Transport source changed in P2, the starting authority was green, later runs
passed unchanged source, and the final coherent head is green. This remains
test/infrastructure debt rather than an A004 P2 product defect.

No physical PS2 run was performed and P2 is not yet wired into product
Application runtime. Those facts remain explicit evidence boundaries.

## Why P3 is presentation ownership state before compositor/runtime wiring

Current clean authority now has:

- P1: one accepted immutable MPEG CALIBRATION region value;
- P2: one generic RFB freeze/request/full-refresh policy;
- A003: MPEG decoder ownership plus the settled rule that Presentation owns the
  first-real-presentation media-clock arm boundary;
- Platform: one existing PS2 graphics path whose synchronized flip is already
  the application-visible physical completion boundary.

Frozen H1 `h1_mpeg_presentation_owner` and `h1_mpeg_start_handoff` recover a
useful start-side ownership model: RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED,
with immutable region/suppression facts and stale-generation rejection.

Two H1 details must not be copied literally:

1. H1 minted presentation generations internally; current architecture treats
   MPEG run/generation identity as caller-owned exact-run authority. Presentation
   consumes that identity and must not create a competing generation namespace.
2. H1's active `stop()` returned directly from MPEG_OWNED to RFB_ONLY. Current
   Q7 supersedes that behavior: active MPEG retirement requires a later ordered
   retirement phase where RFB may rebuild underneath still-visible retiring
   MPEG before final reveal.

P3 therefore reconstructs only the start-side visible-ownership contract and
reserves actual physical composition/clock-arm and Q7 retirement for later
tranches.

## A004 P3 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `309a87851c17f65600ee1d647022cb8b6276baab`;
- final pre-log source/tooling authority
  `b8abd1a89f39710d99691e89e7e24ef0dcc75509`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T181426-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed seven commits after Foreman authority `2f16a2b9...`:

- `abe684c79cf49f856d3d72b3fe5523a815a4034b` —
  Display-owned MPEG visible-ownership core and host fixture;
- `d4035cce00d4bbef297b2018c1839767fdc2beed` —
  first tree-identical dictionary reconciliation trigger;
- `bfd2d0ffc54ef7100ea64b1fea65716a4bf04560` —
  temporary reconciliation-tool generalization after Display was found absent;
- `e6ebb4adf2280936f511612999a8793cd296c28b` —
  second tree-identical reconciliation trigger;
- `aa367034fa874d854209836e3133f45de5f68caa` —
  automated symbol reconciliation;
- `b8abd1a89f39710d99691e89e7e24ef0dcc75509` —
  bounded tooling correction preserving unsupported dictionary metadata;
- `309a87851c17f65600ee1d647022cb8b6276baab` —
  required immutable Reconstruction work log.

Independent source review confirms:

- `src/display/mpeg_presentation.{c,h}` is a pure Display/Presentation owner;
- it uses no UI/calibration, RFB, Transport, MPEG-decoder, Platform or
  media-clock API;
- one caller-supplied nonzero MPEG run/generation is copied verbatim and never
  minted or incremented by Presentation;
- one already-resolved `base / inner_content / suppression` geometry snapshot
  is copied immutably for the run;
- base geometry remains macroblock-sized while inner content is a symmetric
  presentation-local inset and suppression must contain the complete base;
- RFB_ONLY -> WAIT_FIRST_FRAME is the only arm transition;
- WAIT_FIRST_FRAME reports FROZEN_RFB_DESKTOP/global protection while explicitly
  not claiming MPEG visual ownership;
- only an exact-generation first-frame-presented event may promote to
  MPEG_OWNED, and duplicate/stale/wrong events fail closed;
- MPEG_OWNED reports COMPOSITED, releases the generic global-freeze requirement,
  and retains the exact generation/suppression snapshot;
- exact pending-start abort returns to RFB_ONLY without creating RFB
  FULL-refresh debt;
- there is deliberately no active-stop/retire API, so current-Q7 retirement
  remains attachable without an H1-style early reveal;
- the first-frame state transition itself performs no physical draw, upload,
  flip or media-clock arm and therefore does not manufacture hardware evidence.

A004-P3-C1 through A004-P3-C10 are accepted as MET within the packet's bounded
source scope.

### P3 dictionary/tooling repair acceptance

The worker's first behavior commit exposed a genuine integration defect: the
canonical deterministic reconciliation helper did not target `src/display`.

The temporary broad reconciliation then removed two maintained Diagnostics rows
for live GNU `__typeof__(sendto)` alias declarations. The worker correctly
rejected that collateral removal and narrowed the helper again.

Independent Foreman comparison confirms the final state:

- `src/diagnostics/SYMBOLS.md` is byte-identical to Foreman base
  (blob `5918063c0a27596ab558aa6160ec69856dd43e5c`);
- the generated portal again reports Diagnostics=69;
- Display now has complete coverage with 113 symbols;
- relative to the prior helper, the only functional target-set change is adding
  `Path("src/display")`;
- the workflow change only stages `src/display/SYMBOLS.md` during authorized
  reconciliation;
- specialized Diagnostics/framebuffer/UI dictionaries are not newly rewritten
  by the helper.

That final tooling repair is accepted as bounded integration work rather than
product behavior.

### P3 evidence

GitHub Actions run `35541637912` at pickup HEAD completed SUCCESS.

Host logs explicitly show:

- `MPEG_PRESENTATION_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `transport_runtime_test: PASS`;
- the remaining canonical host suite passing.

Project check, strict dictionary audit, broad PS2 compile, and linked
current-source reproducibility also passed.

Evidence limitation remains important: the current PS2 compile/link source
lists do not yet include the still-unwired new P3 translation unit. Those jobs
prove the existing linked product was not broken; the direct executable P3
evidence is the host fixture. No physical PS2 run was performed.

## Why P4 is the single physical compositor / first-sync seam

Current authority now has:

- P1 immutable accepted MPEG calibration meaning;
- P2 generic RFB freeze/request/full-refresh policy;
- P3 exact-run Presentation ownership state;
- A003 MPEG decoder ownership and safe-stop behavior;
- A002 one session-scoped shared media clock;
- one existing PS2 Platform graphics owner whose successful
  `pstvnc_ps2_graphics_present()` returns only after `gsKit_sync_flip()`.

Frozen H1 `h1_cumulative39_graphics` proves the required visible layering:

```text
remote desktop
-> black outer suppression footprint
-> MPEG base
-> black symmetric inner matte
-> local UI / OSK overlay
-> one GS queue / synchronized flip
```

H1 also proves that MPEG picture memory arrives in RGB16 macroblock order and
must be presented through the same physical GS owner. Its useful resource
behavior caches desktop/overlay state, reuses bounded video texture resources,
and returns video-present success only after synchronized physical presentation.

The clean reconstruction must **not** copy H1's structural shortcuts:

- no textual inclusion/wrapping of another `.c` graphics owner;
- no second GS presenter;
- no semaphore justified solely by competing graphics callers;
- no direct active `clear_video()` product stop that bypasses current Q7;
- no MPEG decoder worker directly taking GS ownership.

Current clean MPEG decode also exposes a real dependency for later integration:
the decoder owns its picture buffer but has no per-picture presentation handoff.
P4 therefore reconstructs the physical compositor contract and first-sync
semantics without yet wiring the MPEG worker or final Application transaction.

A003/A004 authority requires the shared media epoch to be armed by Presentation
at the first **real synchronized MPEG presentation boundary**, not at START,
producer admission, decode completion, upload, or a source-level state change.
P4 must make that boundary explicit and host-testable while preserving that only
a PS2 run can qualify the physical result.

## Active bounded Reconstruction packet

PACKET_ID=`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a004-presentation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
PI_LOCAL_USER_PROXY_REQUIRED=`NO`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
EXECUTION_SEAT_USE=`OPTIONAL_LOCAL_SURFACE_ONLY_WHEN_EXPLICITLY_REQUIRED`
WORKTREE_PREFLIGHT_REQUIRED=`CONDITIONAL_ON_EXPLICIT_PI_LOCAL_EXECUTION`
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`

### Objective

Reconstruct the clean **single-owner physical MPEG compositor and first
synchronized presentation seam** required by A004.

The result must preserve one PS2 GS/dmaKit owner, accept one stable decoded
RGB16 macroblock frame plus the exact P3 run snapshot, compose the qualified
desktop/suppression/MPEG/inner-matte/local-overlay order, expose the real
post-sync completion boundary, and attach the one session-scoped media-clock arm
plus P3 first-frame promotion only to the exact first successful synchronized
MPEG presentation.

This packet reconstructs mechanism and boundary contracts. It does **not** wire
the MPEG decoder worker, RFB/P2 runtime, calibration entry, START/producer
transaction, scheduler/drop, Q7 retirement, or final Application lifecycle.

### Required current authority / forensic trace

Read current at wake, including:

- Reconstruction Contract rev 0006;
- Foreman State rev 0026;
- A002 CONFIG/audio/common-clock audit rev 0001;
- A003 MPEG-generation audit rev 0001;
- A004 presentation/calibration audit rev 0001;
- Wire runtime decisions rev 0011, especially Q6/Q7;
- architecture overlay rev 0004;
- clean architecture and source-topology policy;
- `src/display/display.{c,h}`;
- `src/display/mpeg_presentation.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/media/clock.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- current local-UI presentation types only far enough to preserve final-overlay
  ordering;
- current clean PS2 compile/link manifests.

Trace frozen H1 commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, at minimum:

- `experiments/media-harness-h1/h1_cumulative39_graphics.{c,h}`;
- `h1_video_runtime.{c,h}` around sequence picture layout, upload/draw and
  first synchronization;
- `h1_media_clock.{c,h}`;
- `MEDIA_OBJECT_COMPOSITION_NOTES.md`;
- P3's already-traced ownership/start evidence only as needed to bind exact run
  state.

Historical wrapper/semaphore/direct-clear shapes are evidence, not current
product authority.

### Required behavior

1. **One physical GS owner.** Evolve the existing Platform graphics mechanism;
   do not create a competing GS/dmaKit owner, include another graphics `.c`
   file, or give MPEG decoder/runtime direct GS ownership.
2. **Neutral decoded-frame contract.** Define the smallest stable value needed
   to describe one decoded RGB16 macroblock-order MPEG picture. It must carry
   explicit source width/height/capacity facts and be consumable by
   Display/Presentation without importing MPEG decoder private state.
3. **Exact geometry/run fencing.** A video frame may be physically composed only
   against the exact current P3 snapshot/generation. Source dimensions and the
   P3 base rectangle must be mutually consistent; stale generation or geometry
   mismatch fails before physical presentation.
4. **Macroblock interpretation.** Preserve the qualified RGB16 16x16 macroblock
   picture layout. Detile or otherwise adapt it through one clearly owned,
   bounded mechanism suitable for the existing GS owner. Do not silently
   reinterpret macroblock memory as a linear desktop texture.
5. **Single composition order.** While MPEG is visible, one physical frame is:
   remote desktop -> black suppression footprint -> MPEG base -> black symmetric
   inner matte -> local UI/OSK overlay -> one GS queue/synchronized flip.
6. **RFB truth versus suppression.** Remote desktop storage may advance while
   MPEG is owned. Composition prevents remote pixels from becoming visible
   inside the exact suppression footprint; suppression is not a second
   framebuffer truth.
7. **Local overlay remains final.** Existing local UI/OSK presentation remains
   above MPEG/mattes and does not become MPEG state.
8. **Resource discipline.** Keep video presentation resources bounded and
   reusable. Do not leak VRAM or allocate a new permanent texture per frame/run.
   Do not require re-uploading an unchanged full desktop for every MPEG frame
   merely to reproduce H1 mechanically when the single GS owner can safely
   retain current texture state.
9. **Real synchronized completion seam.** A successful MPEG composite must
   distinguish upload/draw submission from completion of the synchronized
   physical presentation. Expose a narrow post-sync result/fact and, where
   needed for clock arm, the exact platform timer observation associated with
   that boundary. Timing primitive ownership remains Platform-facing.
10. **First-sync clock arm.** For the exact P3 WAIT_FIRST_FRAME run, only after
    the first MPEG composite has successfully crossed the synchronized boundary:
    - arm the one session-scoped media clock from that boundary observation;
    - then record the exact P3 first-frame-presented promotion.
    START, decode success, picture availability, detile, texture upload, queue
    submission or an unsynchronized draw must not arm or promote.
11. **One session epoch, not one epoch per run.** If the session media clock is
    already armed, a later MPEG run must preserve the existing session epoch;
    it must not manufacture a second timing origin. The exact first synchronized
    frame still performs its P3 run promotion.
12. **Failure is fail-closed.** Invalid/stale frame, compositor failure, timer
    observation failure, clock-arm failure, or first-frame promotion failure
    must not report successful first presentation. Preserve enough state for the
    later Application owner to contain/retire the run safely.
13. **Subsequent-frame stability.** After P3 is MPEG_OWNED, further exact-run
    frames use COMPOSITED presentation without re-promoting ownership or
    re-arming/replacing the session clock.
14. **No early current-Q7 reveal.** Do not add an active
    MPEG_OWNED -> RFB_ONLY clear/stop shortcut. Do not clear the P3 active
    snapshot or reveal the suppression area merely because a stop is requested.
    Retirement/overlapped RFB restoration/final reveal remains later work.
15. **No premature cross-domain runtime wiring.** Do not yet make
    `pstvnc_mpeg_decoder_run()` call the compositor, create a frame queue,
    wire P2 freeze/thaw, modify calibration/input flow, send START/RETIRE, or
    perform final Application orchestration.
16. **Direct PS2 build coverage.** Any P3/P4 source that is now part of the
    physical compositor mechanism must be added to the canonical strict PS2
    compile and linked clean-build manifests so successful PS2 jobs genuinely
    compile/link that code. Preserve reproducible whole-ELF/PT_LOAD checks.

### Placement / shape guidance

Keep semantic presentation policy in `src/display/` and hardware GS/dmaKit
mechanism in the existing `src/platform/ps2_graphics.{c,h}` seam.

Do not create a new top-level presentation directory. A small Display-owned
compositor/coordinator module is acceptable if it makes the first-sync/clock/P3
boundary testable without moving raw GS ownership out of Platform.

A narrow Platform frame/result value is acceptable where necessary to report
successful synchronized completion and timer observation, but Platform must not
learn MPEG lifecycle/generation meaning.

Do not let Display import UI calibration internals; P3 neutral geometry remains
the presentation value.

### Acceptance criteria

- `A004-P4-C1 SINGLE_GS_OWNER`: all desktop/MPEG/matte/local-overlay physical
  drawing converges through the one existing Platform graphics owner.
- `A004-P4-C2 FRAME_CONTRACT`: one bounded RGB16 macroblock frame contract is
  explicit, validated and independent of MPEG decoder private state.
- `A004-P4-C3 EXACT_RUN_GEOMETRY`: stale generation and source/base mismatch
  are rejected before draw; P3 snapshot is the run geometry authority.
- `A004-P4-C4 COMPOSITION_ORDER`: desktop -> suppression -> MPEG -> inner
  matte -> local overlay -> one synchronized presentation is structurally
  enforced.
- `A004-P4-C5 SUPPRESSION_TRUTH`: RFB authoritative pixels remain separate
  from visible suppression; updates underneath MPEG remain representable.
- `A004-P4-C6 RESOURCE_DISCIPLINE`: bounded reusable video resources and
  sensible cached desktop/overlay use avoid per-frame/per-run resource leaks or
  unnecessary full-desktop upload as a correctness requirement.
- `A004-P4-C7 SYNC_BOUNDARY`: physical success is reported only after the one
  synchronized presentation boundary; upload/queue submission is not promoted
  to physical completion.
- `A004-P4-C8 SESSION_CLOCK_ARM`: only the first real synchronized MPEG
  presentation can arm an unarmed session media epoch; already-armed epoch is
  preserved across later runs.
- `A004-P4-C9 FIRST_FRAME_PROMOTION`: exact P3 WAIT_FIRST_FRAME promotion
  occurs only after successful synchronized presentation and never on
  START/decode/upload alone.
- `A004-P4-C10 SUBSEQUENT_FRAME_STABILITY`: exact MPEG_OWNED frames neither
  re-arm/replace the clock nor duplicate first-frame promotion.
- `A004-P4-C11 Q7_GUARD`: no active direct clear/stop reveals RFB or destroys
  current suppression; Q7 retirement remains a later ordered seam.
- `A004-P4-C12 CLEAN_DIRECT_BUILD_EVIDENCE`: focused host contracts, complete
  host suite, project/dictionary checks, and canonical PS2 compile/link
  manifests directly cover the new physical-compositor source. Hardware
  qualification remains explicitly separate.

### Explicit non-goals

Do not implement in P4:

- MPEG decoder worker -> presentation handoff/queue/thread integration;
- final Application MPEG activation transaction;
- P1 calibration entry/accept wiring;
- P2 RFB freeze/thaw runtime wiring;
- Transport START/RETIRE or Pi producer activation;
- absolute video scheduler or lateness/drop policy;
- current-Q7 retirement/overlapped RFB rebuild/final reveal;
- A005 general input composition;
- hardware visual/A/V/endurance qualification.

### Worker return

Return:

- exact current/frozen evidence inspected;
- exact source/tooling/build/test files and commits changed;
- one-GS-owner shape and why no semaphore/parallel presenter was introduced;
- decoded-frame layout/ownership decision;
- synchronized completion/timer observation contract;
- session-clock first-arm and later-run behavior;
- P3 first-frame promotion integration at the mechanism boundary;
- A004-P4-C1 through C12 disposition;
- focused/full/PS2 compile/link/dictionary evidence;
- explicit confirmation that no H1 direct active-clear/stop shortcut was added;
- evidence gaps and hardware non-claims;
- exact next dependency/baton point.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`
- WORK_ITEM_KEY=`a004-presentation`
- WORKER_KEY=`interactive`

Do not begin scheduler/drop, Q7 retirement, decoder-worker handoff, or final
Application orchestration in the same shift.

## Deferred dependency graph

Expected remaining A004 work after active P4 is:

1. absolute video scheduler/drop plus current-Q7 active-retirement state,
   overlapped RFB restoration and final visible handoff;
2. explicit decoded-frame handoff/runtime integration from the MPEG worker to
   the sole presentation owner if P4 source evidence shows that seam deserves
   its own bounded packet;
3. final Application orchestration consuming A003 plus P1/P2/P3/P4 public
   seams, including activation/abort/failure/shutdown ordering.

Foreman must choose the exact next seam from returned P4 source rather than
pre-authorizing those later tranches.

This is planning only, not worker authority to pre-implement later work.

## Hardware qualification debt

HARDWARE_PENDING=A004 visual geometry/matte/suppression/first-frame qualification; MPEG one-run/repeated-run stale fencing and Wire-loss behavior; current-Q7 overlapped RFB restoration; R1 product runtime where later required; final product Q4 establishment; exact product Wire service/runtime; all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A004-SHARED-COMPOSITOR-FIRST-SYNC-R4` Reconstruction baton,
independently verify that one physical GS owner remains authoritative, that
first-sync/clock/P3 promotion is tied to the actual synchronized boundary, that
the canonical PS2 build directly covers the new mechanism, and that current Q7
remains attachable without an active direct-clear shortcut. Then choose the next
bounded scheduler/retirement/runtime seam from actual source.

Do not execute the packet from the Foreman seat.
