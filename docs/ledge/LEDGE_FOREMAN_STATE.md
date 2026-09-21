# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0034
RECORDED_AT=2026-09-21T06:26:30-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0033
SUPERSEDES_FOREMAN_STATE_REVISION=0033
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0034 independently accepts the completed
`A003-PS2-WORKER-EXECUTION-BINDING-R5` Reconstruction baton and advances to
the missing PS2-side MPEG generation-control relay through the already-governing
single physical-I/O Transport owner. R5 now supplies concrete aligned-memory,
independent synchronization, retained-event and dormancy-proven EE-thread
mechanics without choosing product worker values or acquiring lifecycle policy.

Revision 0033 independently accepts the completed
`A004-MAIN-THREAD-FRAME-CONSUMER-P7` Reconstruction baton and advances to the
remaining concrete PS2 execution binding for the accepted A003 decoder/worker
mechanics. P7 now owns the Application/main-thread one-frame bridge from an
exact R4 claim through P4/P6 presentation while preserving borrowed storage,
first-sync clock ordering and RETIRING drain semantics.

Revision 0032 independently accepts the completed
`A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4` Reconstruction baton, records work-log
contract revision 0007's exact immutable filename-stamp compatibility repair,
and advances into the bounded A004 main-thread frame-consumer seam. R4 now
provides an exact-generation MPEG worker with one borrowed frame slot whose
storage cannot be overwritten while AVAILABLE or CLAIMED.

Revision 0031 independently accepts the completed
`A003-PS2-LIBMPEG-BACKEND-R3` Reconstruction baton and advances A003 into the
MPEG-owned asynchronous worker / one-slot borrowed-frame rendezvous seam. R3
now instantiates the accepted synchronous decoder abstraction against the exact
pinned mature SMS libmpeg/IPU mechanism in the canonical clean product build
without importing GS, Presentation or Application ownership.

Revision 0030 independently accepts the completed
`A003-DECODED-PICTURE-HANDOFF-R2` Reconstruction baton and advances A003 into
the concrete PS2 SMS-libmpeg/IPU backend seam. R2 now exposes exactly one
completed decoded picture as an explicitly borrowed MPEG-owned value while
preserving the qualified safe-stop boundary and existing decoder-run semantics.

Current source inspection also confirms that no clean product implementation of
`pstvnc_mpeg_decoder_platform_ops_t` exists and the clean linked build does not
yet contain SMS libmpeg. Revision 0030 therefore reconstructs that synchronous
hardware backend before adding the asynchronous MPEG worker/rendezvous. This
avoids designing concurrency around a decoder abstraction that has not yet been
instantiated against the qualified PS2 mechanism.

Revision 0029 independently accepts the completed
`A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6` Reconstruction baton and returns to
the MPEG owner for the smallest prerequisite to the decoder-worker runtime
handoff: an explicit one-picture decoded-frame publication/step seam.

The completed R6 worker timed out after publishing its required immutable log.
A restarted Reconstruction pickup therefore emitted a second immutable NOOP log
only to confirm that R6 was already complete. That NOOP changed no product
source, tests, state or packet authority and is preserved as truthful chronology.

Revision 0028 independently accepts the completed
`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5` Reconstruction baton and advances A004
into the absolute common-clock video scheduler / presentation-drop policy seam.
R5 now provides exact-run RETIRING and REVEAL_PENDING phases plus a fail-closed
single-GS synchronized final reveal, while leaving producer-close,
decoder-safe-stop, Transport residual-finalization and RFB-restoration proof as
external Application obligations.

Revision 0027 independently accepts the completed
`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4` Reconstruction baton and advances A004
into the exact-run current-Q7 retirement/final-visible-handoff seam. P4 now
places P3/P4 presentation source directly in the canonical PS2 compile and
linked reproducibility manifests. Revision 0027 deliberately separates
retirement from absolute scheduler/drop work so a later runtime cannot acquire
a decoder-to-compositor path before a bounded safe visible retirement mechanism
exists.

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

`A003_R5_INTEGRATED__A003_MPEG_GENERATION_CONTROL_RELAY_RECONSTRUCTION_ACTIVE__PI_WIRE_CONTROL_OWNER_DEPENDENCY_QUEUED__APPLICATION_ACTIVATION_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

ARCHITECTURE_BLOCKER=NONE
A004_P1_FOREMAN_ACCEPTED=YES
A004_P2_FOREMAN_ACCEPTED=YES
A004_P3_FOREMAN_ACCEPTED=YES
A004_P4_FOREMAN_ACCEPTED=YES
A004_P5_FOREMAN_ACCEPTED=YES
A004_P6_FOREMAN_ACCEPTED=YES
A004_P7_FOREMAN_ACCEPTED=YES
A003_DECODED_PICTURE_STEP_FOREMAN_ACCEPTED=YES
A003_PS2_LIBMPEG_BACKEND_FOREMAN_ACCEPTED=YES
A003_MPEG_WORKER_FRAME_RENDEZVOUS_FOREMAN_ACCEPTED=YES
A003_PS2_WORKER_EXECUTION_BINDING_FOREMAN_ACCEPTED=YES
WORK_LOG_CONTRACT_REVISION_0007_ACTIVE=YES
A003_MPEG_GENERATION_CONTROL_RELAY_ACTIVE=YES
PI_WIRE_CONTROL_OWNER=DEPENDENCY_QUEUED
APPLICATION_ACTIVATION=DEPENDENCY_QUEUED
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

## A004 P4 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `a88718833fb15a8c6fdba668daea9c9b78c960d9`;
- final pre-log source/dictionary/build authority
  `37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T184357-0400__reconstruction__a004-presentation__interactive.md`.

A role-error NOOP log exists immediately before the Reconstruction range at
`2f234d140213ed79d37c6bc2c228f12dfae90fdf`. It changed no Foreman state,
packet, product source or planning authority. The immutable log contract requires
that historical record to remain preserved; it is not current Foreman authority.

The actual P4 Reconstruction range is:

- `75d5c87cd47a3d8b7108d790eed890de4bbaf7b9` —
  synchronized MPEG compositor/frame/Platform mechanism and direct build wiring;
- `bbf7e52f45ec577ccd9eeea688cd4023d87d075f` —
  restore executable mode on the strict PS2 compile script after a Git-write
  mode regression;
- `25081e22864db4f09c67c10dd7c4662cfa975f50` —
  deterministic dictionary-reconciliation trigger;
- `8f147edc97219df4b59c5118566a377f0a0bd0ab` —
  generated current clean dictionaries;
- `f0e1da106035063c63555f93d2b480c36306128c` —
  synchronized compositor ownership prose;
- `37bbd71ca5afaf3dfb63568e85f9d31dbf043d08` —
  preserve legal full-inner-matte geometry where resolved inner content may be
  zero-sized;
- `a88718833fb15a8c6fdba668daea9c9b78c960d9` —
  required immutable Reconstruction work log.

Independent source review confirms:

- `src/display/mpeg_frame.{c,h}` defines one decoder-independent
  caller-owned RGB16 16x16-macroblock surface and bounded deterministic detile;
- `src/display/mpeg_compositor.{c,h}` owns exact-run coordination but contains
  no gsKit/dmaKit, decoder, Transport, RFB or calibration call;
- `src/platform/ps2_graphics.{c,h}` remains the sole current GS/dmaKit owner;
- no second presenter, graphics semaphore or H1 C-file wrapper was introduced;
- P4 reuses one maximum-size MPEG VRAM allocation and one bounded aligned EE
  detile buffer rather than allocating permanent frame/run resources;
- the one render body orders desktop -> suppression -> MPEG -> inner matte ->
  local overlay -> queue -> synchronized flip;
- ordinary desktop/local-overlay refresh while MPEG remains retained redraws the
  same composite, so authoritative RFB truth can advance underneath the visible
  suppression footprint;
- the Platform synchronized result is populated only after
  `gsKit_sync_flip()`, with `GetTimerSystemTime()` sampled after that
  boundary in `kBUSCLK` ticks;
- Display rejects stale generation or source/base geometry mismatch before the
  Platform presenter is called;
- WAIT_FIRST_FRAME can arm an unarmed session media clock only from that
  post-sync timer observation and only then performs exact P3 promotion;
- a pre-existing session media epoch is preserved across a later MPEG run;
- MPEG_OWNED frames neither re-arm the epoch nor duplicate P3 promotion;
- failures after physical synchronization preserve truthful effects while
  returning failure rather than claiming a completed first-presentation
  transaction;
- no active clear/stop API exists in P4, preserving current-Q7 attachment;
- the final inner-matte correction correctly permits an accepted P1/P3 geometry
  whose inner content is zero width/height while keeping base/suppression
  positive and bounded.

A004-P4-C1 through A004-P4-C12 are accepted as MET within the bounded source and
machine-evidence scope.

### P4 evidence

Pickup-head GitHub Actions run `35543110959` completed SUCCESS.

Host evidence includes:

- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `transport_runtime_test: PASS`.

Canonical direct PS2 compile explicitly compiled:

- `src/display/mpeg_frame.c`;
- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_compositor.c`;
- `src/platform/ps2_graphics.c`;

and reported `CLEAN_PS2_COMPILE_CHECK=PASS`.

The linked clean build explicitly compiled/linked
`mpeg_frame.o`, `mpeg_presentation.o`, `mpeg_compositor.o` and
`ps2_graphics.o` into both reproducibility builds and reported
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Project check and strict dictionary audit also passed. No physical PS2/television
run occurred; visible layering, actual first-frame timing, A/V sync and endurance
remain hardware debt.

## Why P5 is retirement before scheduler/runtime wiring

P4 can now safely establish and retain an exact MPEG visual owner, while
ordinary desktop updates continue beneath the retained suppression layer.
However, current source intentionally has no bounded exact-run transition that
can retire that visual owner and reveal the rebuilt RFB desktop.

Current Q7 requires a real ordered retirement:

1. Application closes new MPEG production/admission;
2. already accepted MPEG may drain;
3. RFB refresh can proceed underneath still-visible MPEG/mattes;
4. only after the decoder/runtime reaches its safe stop boundary and the caller
   has sufficient underlay-restoration evidence may the final synchronized
   visible handoff remove MPEG/mattes and reveal RFB.

The production/Transport/decoder/RFB proofs are cross-domain Application facts.
Presentation must not invent them. What is missing now is the narrow
Presentation/Platform mechanism that can represent RETIRING, continue exact-run
drain presentation, seal further frame presentation before final reveal, and
perform one fail-closed synchronized reveal through the existing GS owner.

That retirement seam is a dependency for later decoder-worker handoff. Wiring
the decoder into P4 first would create a runtime that can start/retain MPEG but
has no bounded owner-correct visible retirement path.

Absolute scheduler/drop is also required A004 behavior, but it is independent
timing policy and remains the next later tranche rather than being bundled into
retirement.

## A004 P5 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `8ab034addb551899c4600c64063a1e1f7db7e557`;
- final pre-log source/dictionary authority
  `4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T192712-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed five commits after Foreman base
`3ce6b596a5fc0936c30ab7c31936e1a2c82c4675`:

- `05e3930a8c82a7f0c4b1080de44972028bb0fd75` —
  exact-run Presentation retirement and synchronized retained-video reveal;
- `6480038ae5ec69eae37a2af5e52ecd7972d107fa` —
  deterministic dictionary-reconciliation trigger;
- `3b8b9adc09a17fe373edf35b5416934e95e7cc8a` —
  automated current clean symbol reconciliation;
- `4367ed3b718c2740d3b3dcefb1ecd24ed93b9556` —
  maintained Display/Platform responsibility prose;
- `8ab034addb551899c4600c64063a1e1f7db7e557` —
  required immutable Reconstruction work log.

Independent source review confirms:

- Presentation now owns the exact visible lifecycle
  `RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED -> RETIRING ->
  REVEAL_PENDING -> RFB_ONLY`;
- only the exact nonzero current MPEG_OWNED generation may begin retirement;
- RETIRING retains the immutable run/geometry snapshot, remains COMPOSITED and
  visually MPEG-owned, and requires no generic global P2 freeze;
- exact RETIRING frames remain presentable through the P4 compositor so
  already-accepted data can drain;
- REVEAL_PENDING is a distinct exact-generation seal that keeps the snapshot and
  retained MPEG visual owner but rejects every further MPEG frame before
  Platform is called;
- ordinary desktop presentation continues updating the cached RFB desktop while
  retained retiring MPEG/suppression/mattes remain drawn above it;
- no RFB source/API or P2 FULL-refresh debt was imported into Presentation;
- no producer-close, decoder-safe-stop, Transport residual-discard or RFB-current
  fact is manufactured by P5;
- Platform remains the sole gsKit/dmaKit owner;
- `pstvnc_ps2_graphics_reveal_retained_video()` presents cached desktop plus
  current local overlay while omitting retained MPEG layers through the same
  render body and synchronized flip;
- retained `video_visible` and geometry facts are not cleared until that
  no-video frame has successfully crossed the synchronized boundary;
- reusable video texture/VRAM remains available after successful reveal;
- `pstvnc_mpeg_compositor_reveal_retired()` validates exact
  REVEAL_PENDING generation/snapshot, performs the Platform synchronized reveal,
  and only then commits logical RFB_ONLY;
- retirement reveal has no media-clock parameter and does not arm, clear or
  replace the session epoch;
- a later distinct MPEG run reuses the same already-armed session epoch;
- there is no generic `clear_video()` API and no ordinary
  MPEG_OWNED -> RFB_ONLY direct-stop shortcut.

A004-P5-C1 through A004-P5-C12 are accepted as MET within the bounded
source/machine-evidence scope.

### P5 integration/evidence chronology

Behavior commit run `35544856315` and reconciliation-trigger run
`35544918534` failed only the strict dictionary job because newly introduced
Display/Platform symbols had not yet been committed to their maintained
dictionaries. On both runs host-unit, project-check, direct PS2 compile and
linked PS2 build were already green; the second run's authorized reconciliation
job succeeded.

After the generated dictionaries and maintained responsibility prose landed,
pre-log run `35544987580` completed SUCCESS across all required gates.

Exact pickup-head run `35545125944` (run number 292) also completed SUCCESS:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on an already-coherent head.

Exact-head host logs explicitly report:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Exact-head strict PS2 compile directly compiles current
`src/display/mpeg_presentation.c`, `src/display/mpeg_compositor.c` and
`src/platform/ps2_graphics.c` and reports
`CLEAN_PS2_COMPILE_CHECK=PASS`.

The linked job reports both `ISSUE7_LINKED_BUILD=PASS` and
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2/television run occurred. Actual visible retirement, underlay
rebuild timing and hardware failure behavior remain qualification debt.

## Why P6 is absolute scheduler/drop before decoder-runtime wiring

P1-P5 now establish calibration meaning, generic RFB protection, exact visible
ownership, the single synchronized compositor/clock-arm boundary, and a bounded
current-Q7 visible retirement path.

The remaining presentation-policy dependency before connecting decoded frames to
that mechanism is the qualified absolute scheduler/drop behavior.

Frozen H1/P6 evidence establishes:

- source cadence is represented by a rational FPS, historically 30000/1001;
- frame deadlines are absolute offsets from the common media epoch rather than
  relative sleeps from the previous frame;
- late processing must not rebase the schedule and create timing drift;
- experimental NONE/ONE_VSYNC/TWO_VSYNC comparison modes are not product
  behavior.

Frozen P7 evidence further establishes the intended stale-presentation rule:

- every MPEG picture still advances decode/reference-picture state;
- stale handling suppresses presentation only;
- a picture at least one complete source-frame period late is eligible to be
  omitted from visible presentation;
- pictures less than one frame late remain presentable, avoiding ordinary
  VBlank quantization being treated as stale.

Current clean CONFIG intentionally does not yet carry H1's FPS/scheduler/drop
laboratory fields. R6 therefore reconstructs one narrow Display-owned immutable
timing-policy value and pure absolute scheduling decisions without broadening
the CONFIG wire contract. Profile plumbing belongs to later orchestration.

P4/P5 also settled one important modern constraint absent from the simple H1
single-run model: the session media epoch survives later MPEG runs. R6 must not
reset run-local picture ordinal 1 onto the original epoch and thereby classify a
later run as ancient. Instead the scheduler begins only after P4 has physically
presented/promoted the run's first frame. It derives one immutable run offset on
the existing session clock axis from that exact first-sync observation; this is
a run placement on the shared timeline, not a second epoch and never mutates the
session clock.

## A004 P6 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `352bf76f0535025e925b66a5e167bbf2e0e07d5c`;
- final R6 source/dictionary authority
  `65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`;
- completed immutable R6 Reconstruction log
  `docs/ledge/work-log/20260920T195955-0400__reconstruction__a004-presentation__interactive.md`;
- timeout/restart confirmation NOOP log
  `docs/ledge/work-log/20260920T202020-0400__reconstruction__a004-presentation__interactive.md`.

The second Reconstruction log exists because the original worker timed out after
it had already updated the completed R6 log. The restarted worker refreshed live
authority, proved the sole active packet had already been completed, changed no
product/state/planning authority and correctly refused to execute R6 again.

The substantive R6 range after Foreman base
`1845f06c67b95c131f1824920781379fc6a69cae` is:

- `02b71b0c1edd878faeafccc239592de8d27289d9` —
  Display-owned absolute MPEG presentation scheduler plus focused tests and
  direct build registration;
- `f356aee132e52ee3a6cc47e510a615c42c1eee98` —
  tree-identical noncanonical reconciliation trigger; no product/tree change;
- `b5e16bb22281ffd750fcdc573d3ebc8fb36b76bf` —
  canonical deterministic dictionary-reconciliation trigger;
- `66e312997ae7495a0a83926d141dfd3ae952bffa` —
  automated current clean dictionary reconciliation;
- `65fec4980fb1362716b5bc3ef6f1b73fdafacaf0` —
  maintained Display/Media responsibility prose;
- `64f74f12f2385e6595161c3f3f21cbd2e12a3525` —
  required completed R6 Reconstruction log;
- `352bf76f0535025e925b66a5e167bbf2e0e07d5c` —
  restart-confirmation NOOP log only.

Independent source review confirms:

- `src/display/mpeg_scheduler.{c,h}` owns only deterministic post-first-frame
  presentation timing policy and depends only on the common Media clock seam;
- the profile contains positive rational FPS plus explicit optional
  presentation-drop enable/threshold facts and does not restore H1 scheduler
  comparison modes or expand production CONFIG;
- initialization requires a nonzero caller generation and an already-armed
  session clock, consumes the caller's exact first-sync tick and never calls
  `pstvnc_media_clock_arm()`;
- ordinal 1 is rejected, preserving P4 as the sole first-frame physical
  presentation/clock-arm/promotion authority;
- each run stores one immutable
  `max(first_sync_tick - video_deadline_zero, 0)` placement offset on the
  existing session timeline, so later runs reuse rather than replace the one
  session epoch;
- the scheduler uses an absolute non-rebasing grid
  `run_base + (ordinal - 1) * frame_period`;
- decision inspection is pure and accepts an injected current tick rather than
  sleeping, reading a timer, waiting on VBlank or presenting;
- early frames classify WAIT, on-time/slightly-late frames PRESENT_NOW, and an
  enabled threshold-late frame DROP_PRESENTATION_ONLY;
- disabling drop leaves every otherwise-valid late frame presentable;
- stale/wrong run generations fail closed and the scheduler never mints or
  increments run identity;
- frame period, threshold, ordinal offset, run-base addition and final clock
  deadline calculations are bounded/saturating so far-future values cannot wrap
  into early deadlines;
- the only Media addition is read-only
  `pstvnc_media_clock_tick_rate()`, exposing an immutable initialization fact
  without exposing or mutating epoch/armed authority;
- decoder, compositor, P5 retirement, RFB, Transport and Application runtime
  call graphs remain unwired to the scheduler;
- the 30000/1001 and 1000-milliframe values remain qualification/reference test
  values rather than newly declared public CONFIG knobs.

A004-P6-C1 through A004-P6-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

### R6 CI chronology

Behavior-head run `35546545543` passed host-unit, direct PS2 compile and PS2
link, while project-check and strict dictionary correctly reported missing
dictionary/topology coverage for the new scheduler files.

The first tree-identical trigger run `35546616557` preserved that expected
failure because its noncanonical commit message did not satisfy the authorized
reconciliation predicate.

The canonical trigger run `35546710602` successfully ran dictionary
reconciliation. Its same-run project/dictionary jobs still observed the
pre-reconciliation trigger tree and therefore remained red; the automation
created the coherent dictionary commit afterward.

Final pre-log run `35546830313`, completed-log run `35546948240` (run 299)
and timeout/restart NOOP-head run `35547407928` (run 300) all completed
SUCCESS across the required ordinary gates.

Run 299 explicitly reports:

- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `transport_runtime_test: PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- direct `PS2_COMPILE=src/media/clock.c`;
- direct `PS2_COMPILE=src/display/mpeg_scheduler.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2 timing/drop/A-V run occurred; machine evidence does not qualify
real VBlank cadence, visible stale-frame omission or A/V synchronization.

## Why the next seam is decoder picture publication before worker/runtime wiring

R6 completes the deterministic presentation-policy surface, but current clean
`pstvnc_mpeg_decoder_run()` still privately loops over
`platform_ops.picture()` until terminal completion/stop/failure.

After each successful picture the decoder:

1. validates sequence/feed/result state;
2. increments `pictures_decoded`;
3. observes the safe-stop boundary;
4. immediately enters the next decoder call.

It exposes no public one-picture result containing the just-decoded picture
buffer, sequence geometry or ordinal.

Therefore a worker-to-Presentation runtime cannot yet be reconstructed without
one of two bad shortcuts:

- reaching into public decoder struct internals and racing the decoder-owned
  reusable picture buffer; or
- duplicating the decoder's qualified feed/sequence/safe-stop loop outside the
  decoder owner.

The next bounded dependency is consequently an MPEG-owned one-picture step /
decoded-picture publication seam. It must preserve the proven false-EOF
prevention and current `run()` behavior while making buffer lifetime explicit.

This packet deliberately does **not** add the MPEG worker thread, main-thread
frame mailbox, scheduler/compositor calls, concrete PS2 libmpeg backend,
START/RETIRE orchestration or final Application transaction. Those remain later
seams chosen from the returned source.

## A003 R2 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `b67a077c0a56c0b4db2fdcaf5a19ed21b70f440d`;
- final pre-log source/dictionary authority
  `ed5c03e9c3666d784d073b2922d8bff02d10e800`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T203600-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The worker landed five commits after Foreman base
`c88a9fcf0011eacc5c43bdc11184af29c320907f`:

- `bf6ec2bf2a870287b624de65b44c14835fed2e8f` —
  one-picture decoder handoff;
- `7a3a43274fae3d975f9450e10e20911f52f504e8` —
  deterministic dictionary-reconciliation trigger;
- `d1fd65633292952d1c12683433a06c4234a03085` —
  automated current-clean dictionary reconciliation;
- `ed5c03e9c3666d784d073b2922d8bff02d10e800` —
  fail-closed borrowed-picture output plus maintained MPEG responsibility prose;
- `b67a077c0a56c0b4db2fdcaf5a19ed21b70f440d` —
  required immutable Reconstruction work log.

Independent source review confirms:

- `pstvnc_mpeg_decoder_step()` owns at most one underlying platform
  `picture()` call and returns before a later call may begin;
- `PSTVNC_MPEG_DECODER_PICTURE_READY` is the sole successful publication
  result;
- the published `pstvnc_mpeg_decoded_picture_t` is a read-only borrowed view
  carrying only decoder-owned pixels, usable macroblock-backed byte count,
  allocation capacity, validated sequence width/height, bytes-per-pixel and
  decoder-local picture ordinal;
- no MPEG generation, Presentation or Application authority is carried in the
  decoded-picture value;
- the borrow is explicitly valid only until the next step on that decoder or
  decoder release, whichever occurs first;
- tests prove the next step reuses/overwrites the same decoder-owned picture
  allocation rather than manufacturing immutable caller storage;
- every call with a writable picture output zeroes that output before
  validation/terminal handling, so every non-PICTURE_READY result fails closed
  instead of leaving stale borrowed metadata consumable;
- sequence callback accounting now retains the exact macroblock-backed usable
  byte extent independently from maximum picture allocation capacity;
- ordinal 1 is observable as decoded data but no scheduler, clock, GS,
  compositor, visible-ownership or generation effect occurs in MPEG;
- stop already visible before a step prevents a new decoder call and returns
  STOPPED with no picture publication;
- stop requested during an active picture does not enter the libmpeg data
  callback as synthetic EOF: real Transport data continues, successful-picture
  accounting occurs once after decoder-call return, then STOPPED suppresses
  publication of that stopping boundary;
- real Transport exhaustion remains COMPLETE and decoder end without real
  exhaustion/owner stop remains UNEXPECTED_END;
- existing sequence, Transport, transfer, picture, synchronization and
  accounting result classes remain distinct;
- `pstvnc_mpeg_decoder_run()` now loops over the one-picture step and adds no
  second decoder loop or duplicate accounting;
- feed/payload/padded-transfer accounting remains owned by the existing feed
  callback, while picture accounting is incremented exactly once in the step;
- MPEG still imports no Display scheduler/compositor, Platform graphics, RFB,
  media clock or Application runtime dependency;
- channel-4 consumption remains exclusively through the public Transport MPEG
  activity/read/wait seam.

A003-R2-C1 through A003-R2-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

### R2 machine evidence

Final pre-log run `35548538065` (#305) completed SUCCESS.

Exact pickup-head run `35548659856` (#306) also completed SUCCESS:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on a coherent ordinary head.

Exact-head logs explicitly report:

- `mpeg_decoder_test: PASS`;
- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- direct `PS2_COMPILE=src/mpeg/decoder.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2 MPEG decode/start-stop test occurred and none is inferred.

## Why R3 is the concrete PS2 libmpeg backend before worker threading

R2 makes the synchronous decoder boundary suitable for a bounded asynchronous
rendezvous, but current clean product source still contains no concrete
implementation of `pstvnc_mpeg_decoder_platform_ops_t`.

The clean linked build presently compiles `src/mpeg/decoder.c` only. It does
not link SMS libmpeg and has no clean source that calls:

- `MPEG_Initialize()`;
- `MPEG_Picture()`;
- `MPEG_Destroy()`;
- `_MPEG_Set16(1)`;
- TO_IPU DMA submission;
- the qualified IPU/DMAC known-state preparation.

The only mature SMS libmpeg source currently lives under the EXP3 historical
vendor path. Its `UPSTREAM.txt` pins upstream SMS commit
`c1898094725ad750ec20e10cc148b39d7c8a9c65` and records the four copied files
as unchanged with Academic Free License 2.0 notices retained.

Frozen qualified evidence further shows that the clean split must preserve only
decoder hardware mechanisms from H1:

- establish the known IPU/DMAC state before `MPEG_Initialize()`;
- use the mature pinned SMS libmpeg implementation;
- use RGB16 through `_MPEG_Set16(1)`;
- submit padded feed data through ordinary TO_IPU DMA;
- translate libmpeg sequence width/height into the decoder-owned sequence
  callback and picture buffer;
- use `MPEG_Picture(NULL,...)` for the first call and the accepted decoder
  picture buffer for later calls as required by the mature API;
- wait for TO_IPU ownership before teardown and call `MPEG_Destroy()` only
  after decoder-call ownership has returned.

H1's GIF upload/draw packets, GS initialization, VBlank scheduling and
presentation work are explicitly not part of the clean decoder backend because
P4 owns the sole current GS/compositor path.

Building the concrete synchronous hardware backend before the worker also tests
whether the already-accepted decoder platform-ops seam is actually sufficient
against the qualified libmpeg API before introducing thread/rendezvous
complexity.

## A003 R3 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `358365601c384c772ef224fe17ae04adcc4d6f94`;
- final pre-log source/dictionary/build authority
  `16344f98329083802198b7b15437dda3055a6ec2`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T205400-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The worker landed six commits after Foreman base
`a2a608522edd43c102973ebfb162e7fe3952c117`:

- `18eb2d063c0f20970feb969a6d0771cc75512f4f` —
  concrete pinned PS2 SMS decoder backend plus product dependency/build wiring;
- `91eb3c67f684aef324296e995eb2b7b429c1e0cf` —
  hermetic pinned-SMS byte/provenance verification;
- `cbdd6bcad7222c73a1033df910aa3634f2ca3258` —
  deterministic dictionary-reconciliation trigger;
- `c739bf6e8cec4d2105cf1d94051436d8144012a2` —
  automated current-clean symbol reconciliation;
- `16344f98329083802198b7b15437dda3055a6ec2` —
  maintained MPEG responsibility prose;
- `358365601c384c772ef224fe17ae04adcc4d6f94` —
  required immutable Reconstruction work log.

Independent review confirms:

- `src/mpeg/ps2_decoder_backend.{c,h}` realizes every accepted decoder
  platform op without changing decoder ownership;
- backend ownership is limited to SMS binding, IPU/DMAC known-state preparation,
  callback translation, TO_IPU feed DMA, RGB16 selection and SMS
  picture/destroy mechanics;
- no Display, compositor, scheduler, Presentation, Platform graphics, Transport,
  RFB, media-clock or Application API enters the backend;
- D3/D4 quiesce plus IPU reset/BCLR is unconditional before SMS initialization;
- the obsolete reset-each-session toggle was not restored;
- SMS data delegates directly to the accepted decoder feed callback and cannot
  manufacture local-stop EOF;
- SMS sequence rejects invalid signed dimensions, delegates allocation/bounds to
  the decoder sequence callback, and captures only its decoder-owned picture
  pointer/capacity;
- `_MPEG_Set16(1)` selects the qualified RGB16 path;
- feed submission requires exact aligned decoder transfer bytes, waits TO_IPU,
  and submits exact QWC without Transport reads, repadding or accounting policy;
- first picture uses `MPEG_Picture(NULL,...)`; later pictures use the exact
  decoder-owned sequence buffer;
- SMS PTS scratch remains backend-local and carries no common-clock authority;
- destroy waits TO_IPU and calls `MPEG_Destroy()` once per initialized interval;
- release rejects initialized state, clears only backend-owned state and requires
  fresh known-state preparation on the next acquisition;
- mature SMS process-global state is represented honestly by one active backend
  reservation, so overlapping acquisition fails closed.

Stable product dependency `vendor/sms-libmpeg/` is Git-blob identical to the
pinned EXP3 reference:

- `UPSTREAM.txt` — `70ca1542e3c89f471a5994fbbbb87baf61c2d375`;
- `include/libmpeg.h` — `ee2195b52dc3a7112537046426a80aa0e603fa6c`;
- `include/libmpeg_internal.h` —
  `c2f80a9104380634c3ef6749baa2e5f0acb5c13e`;
- `src/libmpeg.c` — `f9e5f11689fa6ed3759365249c2d7cfb7335e2fb`;
- `src/libmpeg_core.S` —
  `93638fd62e58bfac8c6ed1c5fc84ef119d318438`.

Pinned upstream remains
`c1898094725ad750ec20e10cc148b39d7c8a9c65`; upstream authorship and Academic
Free License 2.0 notices remain intact and no third-party source was edited.

Canonical current-source build now directly contains backend + SMS C + SMS
assembly. SMS C/assembly use the dedicated qualified flags
`-D_EE -O2 -G8192 -mgpopt -mno-abicalls -Wall -mno-check-zero-division`
rather than ordinary product `-G0`.

A003-R3-C1 through A003-R3-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

### R3 machine evidence

Final coherent pre-log run `35550113167` (#312) completed SUCCESS after an
unchanged-source retry of the known timing-sensitive Transport fixture.

Exact pickup-head run `35550373481` (#313) completed SUCCESS first attempt.

Exact-head evidence includes:

- `transport_runtime_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `PS2_COMPILE=src/mpeg/ps2_decoder_backend.c`;
- four `SMS_VENDOR_IDENTITY_PASS` checks;
- `SMS_VENDOR_PROVENANCE_PASS=c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- direct SMS C and assembly compilation;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- two linked builds with the SMS dedicated flags;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Historical unchanged-SMS assembler/link warnings remain build observations only.
No physical PS2 SMS decode, IPU reset, TO_IPU DMA, repeated-session or
stop/relaunch qualification occurred or is inferred.

## Why R4 is worker / one-slot borrowed-frame rendezvous before Display runtime

R2 established that a decoded frame is borrowed decoder storage and may be
overwritten by the next decoder step. R3 now proves that decoder abstraction is
realizable against the qualified PS2 backend.

Direct decoder-worker calls into A004 would violate the owner split by moving
Presentation/GS/timing responsibility into MPEG. Conversely, handing a raw
borrowed pointer to the main thread while allowing the decoder to run ahead would
violate R2's lifetime contract.

The smallest missing seam is therefore an MPEG-owned asynchronous worker with
one synchronized frame slot. The worker decodes one frame, publishes that exact
borrowed value, and cannot enter another decoder step until the consumer releases
or explicitly discards the exact frame.

## A003 R4 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `4d2139d0b2443e65e5e3884b3ac5558bb1a0ddad`;
- final pre-log source/dictionary/build authority
  `e56d9cb47439ef60c82e3dd78328d7ea6732995e`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260921T020723-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The worker landed five commits after Foreman base
`3bbbd8ecbfa54a1cd155c1195e990ac4101a33b2`:

- `51f45cdb59cd61565246159e1915f002b0c7de2d` —
  one-slot asynchronous MPEG decoder worker, focused tests and build wiring;
- `785ec520812a9c7e07862717a4fab87febc6c623` —
  deterministic dictionary-reconciliation trigger;
- `d63e8af665a689acf38e3ce58f6cce62f3bf61a1` —
  generated current-clean MPEG dictionary reconciliation;
- `e56d9cb47439ef60c82e3dd78328d7ea6732995e` —
  maintained MPEG ownership prose;
- `4d2139d0b2443e65e5e3884b3ac5558bb1a0ddad` —
  immutable Reconstruction log whose filename mistake is preserved through
  work-log contract revision 0007.

Independent source review confirms:

- `src/mpeg/worker.{c,h}` owns the asynchronous decoder worker and one
  exact-generation borrowed-frame slot;
- worker start copies one nonzero caller generation verbatim and never mints or
  increments generation identity;
- after start, decoder initialize, every decoder step and decoder release occur
  only on the worker thread;
- owner-thread stop uses the decoder's accepted synchronized stop request and
  never changes callback EOF/Transport semantics;
- the synchronized slot lifecycle is exactly
  `EMPTY -> AVAILABLE -> CLAIMED -> EMPTY`;
- PICTURE_READY copies only R2 borrowed-picture metadata; no full RGB16 frame is
  copied and no Display type enters MPEG;
- worker cannot enter another decoder step while the slot is AVAILABLE or
  CLAIMED;
- nonblocking claim changes only AVAILABLE -> CLAIMED and returns the exact run,
  picture metadata and ordinal-derived claim token;
- release requires exact run plus exact claimed picture ordinal, clears the slot
  exactly once and signals the retained event;
- wrong generation, zero/wrong token, duplicate release and non-AVAILABLE claim
  fail closed;
- the event contract explicitly retains signal-before-wait and the worker
  rechecks synchronized stop/slot predicates after every wake, eliminating a
  correctness-critical poll timer;
- stop discards an unclaimed AVAILABLE frame but never invalidates CLAIMED
  storage;
- a claimed frame pins decoder storage until exact release, after which stop can
  proceed to decoder teardown;
- stop already visible before decoding prevents a picture call;
- stop during an active picture preserves R2's real Transport/feed semantics,
  accounts the completed picture once and suppresses caller publication;
- decoder COMPLETE, STOPPED and failure outcomes remain distinct and carry exact
  decoder report/release result;
- a previously published frame cannot mask a later decoder failure;
- join refuses non-empty slot and unfinished worker state, and is the final
  worker-write visibility fence;
- release requires join before thread destruction/stack reclamation;
- no Display scheduler/compositor/Presentation, Platform graphics, RFB,
  Application, media-clock or Transport-private dependency entered the worker.

A003-R4-C1 through A003-R4-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

### R4 machine evidence

Exact final pre-log source run:

`35566964942` — run #318 — SUCCESS first attempt.

It proves:

- host-unit PASS;
- `MPEG_WORKER_TEST=PASS`;
- `mpeg_decoder_test: PASS`;
- existing scheduler/compositor/RFB/Transport host suites PASS;
- project-check PASS;
- complete strict dictionaries PASS;
- direct R5900 compile including `src/mpeg/worker.c` PASS;
- SMS dedicated compile preservation PASS;
- clean linked build PASS;
- current-source linked reproducibility PASS.

The immutable log head run
`35567149530` (#319) has all host/dictionary/PS2 compile/link jobs green and
project-check red for one exact reason only:

`20260921T020723-0400...: filename stamp 20260921T020723-0400 != STARTED_AT-derived 20260921T015200-0400`.

No product/source regression is inferred from that policy-format failure.

### Immutable log compatibility repair

The malformed log truthfully records:

`STARTED_AT=2026-09-21T01:52:00-04:00`

and:

`COMPLETED_AT=2026-09-21T02:07:23-04:00`.

Its already-frozen filename incorrectly used the completion stamp:

`20260921T020723-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The governing immutability rule prohibits renaming, replacing, rewriting or
duplicating that record. Work-log contract revision 0007 therefore adds one
exact path-specific filename-stamp exception. Unlike the older broad grandfather
set, every other canonical metadata/schema/status/key check remains active for
this file, and both its frozen filename stamp and truthful STARTED_AT are pinned
exactly in the checker.

No generalized timestamp mismatch, role/date pattern or completion-time naming
rule is accepted.

## Why A004 P7 is the next seam

R4 now safely publishes exactly one borrowed frame to the main owner without
decoder run-ahead. A004 P3-P6 already provide exact-run visible ownership,
first-synchronized presentation, retirement-visible state and absolute
post-first-frame scheduler/drop policy.

The missing seam is the narrow **main-thread per-frame consumer** that translates
one exact R4 claim into A004 presentation work. Clean architecture assigns this
cross-domain coordination to Application/main-thread ownership; MPEG must not
call Display directly and Display must not own the decoder worker.

P7 deliberately stops before START/producer lifecycle and Q7 retirement
orchestration. It only owns one-frame claim -> validate/map -> first-frame or
scheduler decision -> present/drop -> exact release behavior.

## A004 P7 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `0d718851860ad15ab525ea0485c27165859f7be3`;
- final pre-log source/test/dictionary/build authority
  `d0c9bcb99725bfdaf6b07c5396a0fa1255039225`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260921T022804-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed six commits after Foreman base
`a04c775cf0bff2d9b89a01f1764c769ef59fe1a9`:

- `c9141fe436ab099383e3f5db84eacacfd3802720` —
  Application-owned MPEG frame consumer plus focused tests/build wiring;
- `8d0110fab005010173b0f0c547128f435992bc35` —
  deterministic dictionary-reconciliation trigger;
- `6f2844d92789500f6b9893f245c9228343bf5267` —
  generated current-clean root dictionary reconciliation;
- `5fb13033e83e2d7626af6f9596b59018e439ec5c` —
  maintained Application ownership/topology prose;
- `d0c9bcb99725bfdaf6b07c5396a0fa1255039225` —
  additional scheduler-decision containment test closure;
- `0d718851860ad15ab525ea0485c27165859f7be3` —
  required immutable Reconstruction work log.

Independent review confirms:

- `src/app_mpeg_frame.{c,h}` is an Application/main-thread coordinator, not an
  MPEG or Display mechanism owner;
- init requires one nonzero caller generation, exact WAIT_FIRST_FRAME
  Presentation generation, one R4 worker accepting that generation, an existing
  session clock and an immutable caller scheduler profile;
- generation is copied verbatim and never minted, incremented or rebased;
- one service call claims at most one worker frame; FRAME_UNAVAILABLE is
  ordinary IDLE and may expose only synchronized `worker_finished`, never
  worker terminal reinterpretation;
- the claimed R2/R4 borrow maps zero-copy into the neutral RGB16 macroblock
  surface only after pointer/alignment/bpp/usable-byte/capacity/geometry
  validation;
- the claim token must equal the exact picture ordinal and ordinal continuity is
  strict: first consumable picture 1, then exactly +1;
- ordinal 1 bypasses P6 and reaches only the P4 compositor while Presentation is
  WAIT_FIRST_FRAME;
- P7 never directly arms the clock or promotes Presentation; successful first
  service requires truthful P4 synchronized + first-frame-promoted effects;
- P6 initializes exactly once only after that successful physical first sync,
  using P4's observed synchronized tick and the existing session clock;
- later exact frames use the unchanged P6 absolute WAIT/PRESENT/DROP decision;
- WAIT retains the exact CLAIMED R4 borrow, returns the absolute deadline and
  performs no sleep, spin, VBlank wait, release or second claim;
- PRESENT uses the sole P4 compositor and releases the exact R4 claim only after
  successful synchronized presentation;
- DROP performs no compositor call and releases only the presentation claim;
  decoder/Transport/generation/Presentation ownership is unchanged;
- RETIRING remains legal for accepted later frames through the same P6/P4 path;
- P7 does not begin, seal or reveal retirement;
- REVEAL_PENDING/RFB_ONLY block new claims, and an unexpectedly sealed state
  with an already-held claim enters borrow-safe containment rather than drawing;
- mapping, ordinal, scheduler-init/decision, compositor and sealed-state
  failures request exact R4 stop before intentionally releasing a held claim;
- a stop failure leaves the claim pinned and publicly visible;
- a release failure preserves outstanding-claim state and faults the consumer;
- compositor effects are copied truthfully even when a later clock/promotion
  failure occurs after physical synchronization;
- P7 does not join/destroy the worker or reinterpret decoder terminal outcome;
- no Transport-private, RFB, direct GS/dmaKit, producer START/RETIRE, P5
  transition, clock-arm, sleep/delay or generation-allocation ownership entered
  the module;
- the root Application placement is explicitly admitted by source-topology
  authority rather than accidental source flattening.

A004-P7-C1 through A004-P7-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

### P7 machine evidence

Final coherent pre-log source authority:

`d0c9bcb99725bfdaf6b07c5396a0fa1255039225`

Workflow:

`35569493813` — run #325 — SUCCESS first attempt.

Observed evidence includes:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- project-check PASS;
- complete strict dictionaries PASS;
- `PS2_COMPILE=src/app_mpeg_frame.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log head run:

`35569778609` — run #326 — SUCCESS first attempt across all required jobs.

No physical PS2 MPEG presentation, deadline timing, decoder-worker concurrency
or retirement run occurred or is inferred.

## Why the next packet is concrete PS2 worker execution binding

P7 completes the main-thread per-frame bridge, but current clean source still has
no concrete provider for the injected R2/R4 execution mechanics:

- decoder memory allocation/alignment;
- decoder synchronization;
- worker memory allocation/alignment;
- worker EE thread create/start/join/destroy;
- worker synchronization;
- the retained signal-before-wait frame-slot event.

A recursive current-source inspection finds no clean MPEG call site that creates
those operation tables and no PS2 worker/thread/event binding. The existing
input runtime and frozen H1 worker independently prove the relevant EE kernel
mechanics—`CreateThread`, `StartThread`, `ReferThreadStatus`,
`DeleteThread`, `DelayThread`, `CreateSema`, `WaitSema`,
`SignalSema`, `ExitThread` and `_gp`—but H1's experimental stack,
priority and join polling constants are not product defaults.

R5 therefore binds only these already-designed execution contracts to PS2
mechanics. It does not activate an MPEG run or decide Application lifecycle.

## A003 R5 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `8b198603ace2f7a34bb4dce5753da385aa08b348`;
- final pre-log source/dictionary/build authority
  `99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260921T054912-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The worker landed five commits after Foreman base
`f3f69cb9d659b1d8d31e2d0d40b6d9b9a22f1664`:

- `d361baef50fc0eece2737e5147256d863d162aff` —
  concrete MPEG-owned PS2 worker execution binding plus build wiring;
- `f03e51ea05137060fa2c73350a18ce13efc2be30` —
  deterministic dictionary-reconciliation trigger;
- `6cb92ae8d8a48da96b24587e67c593936e1fd27c` —
  generated current-clean MPEG dictionary reconciliation;
- `99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2` —
  maintained MPEG responsibility prose;
- `8b198603ace2f7a34bb4dce5753da385aa08b348` —
  required immutable Reconstruction work log.

Independent review confirms:

- `src/mpeg/ps2_worker_runtime.{c,h}` is one explicit MPEG-owned mechanism
  object rather than a process-global singleton or generic Platform-thread
  framework;
- initialization requires explicit nonzero caller join-poll delay/count and
  introduces no stack, priority or join-time defaults;
- decoder and worker allocation callbacks pass the exact requested byte count
  and power-of-two alignment to `memalign()`, verify returned alignment, and
  pair allocations with `free()`;
- decoder and worker/slot synchronization use distinct count-1 semaphores;
- the frame-slot event is a third count-0/max-1 semaphore, retaining one
  signal-before-wait token while allowing redundant signals to coalesce;
- event wait is blocking `WaitSema()` and contains no DelayThread polling;
- thread create receives the exact R4 stack pointer, byte count and priority and
  sets `gp_reg = &_gp`;
- the private EE trampoline calls only the saved portable R4 entry/argument and
  then explicitly calls `ExitThread()`;
- one binding has exactly one active thread slot; overlapping create and stale
  thread IDs fail closed;
- join treats R4's `worker_finished` only as portable state and independently
  requires `ReferThreadStatus() == THS_DORMANT`;
- join polling is bounded solely by caller values; H1's historical 1-ms /
  3000-loop values were not restored;
- no `TerminateThread()` or other force-termination path exists;
- `DeleteThread()` is reachable only after dormancy was already proven or a
  final direct `ReferThreadStatus()` proves THS_DORMANT;
- StartThread-failure cleanup also refuses to delete unless dormant state can be
  proven, preserving ownership on failure;
- worker release ordering remains compatible with the binding fence:
  R4 requires EMPTY slot + worker_finished before join, joins the EE thread,
  destroys the dormant thread, then frees its stack; binding release refuses
  while either the thread slot or any binding allocation remains live;
- partial semaphore initialization/deletion failures preserve resource IDs and
  `resources_owned` rather than falsely reporting successful teardown;
- no decoder-step/libmpeg/IPU, Display/P7/scheduler/compositor, media-clock,
  producer, Transport, RFB, GS or generation-allocation behavior entered the
  adapter.

A003-R5-C1 through A003-R5-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

### R5 machine evidence

Final coherent pre-log source authority:

`99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`

Workflow:

`35586198869` — run #331 — final attempt SUCCESS.

The first attempt's only red canonical job was the known timing-sensitive
Transport runtime host fixture. No Transport source changed. Its unchanged
retry passed.

Final observed evidence includes:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `PS2_COMPILE=src/mpeg/ps2_worker_runtime.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log head workflow:

`35586567057` — run #332 — final attempt SUCCESS.

Attempt 1 again showed the known unchanged Transport-runtime cascade beginning
at lines 787/789 and dependent quiesce assertions; every non-host job passed.
The identical-commit host rerun passed. No R5 source regression is inferred.

No physical PS2 worker-thread, semaphore/event, decoder or presentation run
occurred or is inferred.

## Why R6 is the MPEG generation-control relay before Application activation

Current clean source now has the local decoder/worker/presentation mechanics
needed for a run, but the cross-Wire control path is not yet owner-correct:

- `src/transport/protocol.{c,h}` already preserves exact START kind 11 /
  control channel 0 / flags 0 / 44-byte v1 and RETIRE kind 10 / control channel
  0 / flags 0 / 12-byte v1 representations;
- the current sole physical-I/O runtime owns generic serialized outbound work
  through `pstvnc_transport_runtime_submit_frame()`;
- the public Transport bridge exposes no session-bound START or RETIRE request;
- the receiver currently dispatches only DATA frames, so an otherwise valid Pi
  RETIRE completion kind 10 would be classified as unaccepted and fail the Wire
  session;
- the older inline physical-stream MPEG START helper predates the governing
  single-I/O architecture and must not become an Application bypass around the
  Transport I/O owner.

Frozen exact-retirement evidence establishes that the Pi completion is the
identical RETIRE payload echoed back only after exact remote cleanup; direction
plus the higher owner's pending-run state distinguishes request from completion.

Transport should therefore relay the exact control envelope and preserve
session validity, while Application later owns active-run/generation meaning and
the ordered lifecycle transaction.

## Active bounded Reconstruction packet

PACKET_ID=`A003-MPEG-GENERATION-CONTROL-RELAY-R6`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a003-mpeg-generation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
PI_LOCAL_USER_PROXY_REQUIRED=`NO`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
EXECUTION_SEAT_USE=`OPTIONAL_LOCAL_SURFACE_ONLY_WHEN_EXPLICITLY_REQUIRED`
WORKTREE_PREFLIGHT_REQUIRED=`CONDITIONAL_ON_EXPLICIT_PI_LOCAL_EXECUTION`
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`

### Objective

Reconstruct the PS2-side Transport-owned MPEG **generation-control relay** so
Application can later send exact START/RETIRE requests and consume the exact Pi
RETIRE completion without bypassing the sole physical-I/O owner.

R6 owns only Wire envelope/relay mechanics and bounded control storage. It must
not decide which generation is active, start/stop a decoder or producer, mark
MPEG producer completion, finalize residual bytes/credit, or perform
Presentation/RFB/Application lifecycle.

### Required authority

Read current at wake, including:

- Reconstruction Contract rev 0006;
- work-log contract rev 0007;
- Foreman State rev 0034;
- Wire Runtime Decisions rev 0011 and Architecture Overlay rev 0004;
- `src/transport/protocol.{c,h}`;
- `src/transport/runtime.{c,h}`;
- `src/transport/bridge.{c,h}`;
- `src/transport/physical_stream.{c,h}`;
- current Transport runtime/bridge/protocol tests;
- A003 audit rev 0001 as historical semantic evidence where not superseded;
- frozen exact RETIRE control evidence:
  - `experiments/media-harness-h1/h1_cp2p_retirement_control.py`;
  - its focused tests;
  - frozen H1 Transport retire begin/poll/finalize only as mechanism evidence.

Governing Q1-Q12 ownership supersedes old direct physical-send structure.

### Required behavior

1. **Transport-owned relay, Application-owned meaning.** Add public Transport
   bridge seams for exact MPEG START request, RETIRE request, and nonblocking
   receipt of one Pi RETIRE completion. Do not put active-generation business
   state into Transport.
2. **Session-bound authority.** Every new public operation requires one current
   `pstvnc_transport_access_t`; stale Session-A authority must neither send
   through nor consume control state belonging to Session B.
3. **Sole physical-I/O send.** START and RETIRE requests must be encoded with the
   accepted protocol codecs and enter the existing
   `pstvnc_transport_runtime_submit_frame()` outbound rendezvous. Application
   or MPEG must never call physical-stream framed send directly.
4. **Exact START envelope.** Outbound START is kind 11 / channel 0 / flags 0 /
   exact 44-byte v1 payload. The relay may validate codec/envelope structure but
   does not decide calibration acceptance or active-generation policy.
5. **Exact RETIRE envelope.** Outbound RETIRE is kind 10 / channel 0 / flags 0 /
   exact 12-byte v1 payload.
6. **Inbound direction contract.** On PS2, inbound kind-10 exact RETIRE is the Pi
   completion/ACK candidate. Inbound START is not valid PS2-side traffic and
   remains a protocol failure.
7. **Exact inbound validation.** The sole receiver accepts RETIRE completion only
   with exact kind/channel/flags/payload length and a successful v1 payload
   decode. Malformed control fails the Transport session rather than being
   reinterpreted as media.
8. **One bounded completion slot.** Transport owns at most one pending decoded
   RETIRE-completion value for the active Wire Session, protected by
   Transport-owned synchronization. No overwrite and no unbounded queue.
9. **Nonblocking take.** The public bridge returns WOULD_BLOCK when no completion
   is pending; success copies one exact decoded completion and consumes it once.
   Duplicate take returns WOULD_BLOCK.
10. **No generation semantic comparison.** Transport does not decide whether the
    completion's session/generation matches Application's pending MPEG run.
    Application later performs that exact-run check.
11. **No producer terminal side effect.** Receiving a RETIRE completion does
    **not** call `pstvnc_transport_mpeg_mark_producer_done()` in R6 and does
    not synthesize decoder EOF. That ordered semantic belongs to the later
    Application retirement transaction.
12. **No residual/credit finalization.** R6 does not discard queued MPEG bytes,
    return retirement residual credit, reopen a successor run, or otherwise
    implement Q7 finalization.
13. **Retire slot is session-local.** New Transport session initialization begins
    with no pending completion; old-session completion cannot leak into the next
    Wire Session.
14. **No direct START bypass remains.** Remove, retire, or otherwise make
    unavailable to product callers the old
    `pstvnc_transport_physical_stream_send_mpeg_start()` convenience path so
    the accepted single-I/O owner is the only production send path.
15. **Pure MPEG DATA preserved.** Nonempty DATA/channel4 remains opaque MPEG
    media regardless of payload length/content. In particular an exact 44-byte
    START-shaped media payload remains media.
16. **Existing riders unchanged.** RFB/AUDIO/MPEG DATA flow, credit, receiver
    progress and Transport failure behavior remain unchanged outside the new
    exact control path.
17. **No Pi implementation.** Do not promote the frozen Python Pi coordinator or
    invent a production Pi Wire server in this packet. R6 is PS2-side relay
    completion only.
18. **No Application/MPEG lifecycle.** Do not arm Presentation, allocate a run
    generation, start R5/R4, invoke P7, start/retire a Pi producer, release RFB
    suppression or execute P5/Q7 lifecycle.
19. **Canonical evidence.** Add focused host tests proving exact outbound
    envelopes travel through runtime's sole outbound owner, exact inbound RETIRE
    completion one-shot behavior, malformed/inbound-START rejection, stale
    Transport-access fencing, fresh-session usability, pure 44-byte MPEG DATA
    classification, and unchanged rider behavior. Preserve direct PS2 compile,
    linked reproducibility and strict dictionaries.

### Shape guidance

Prefer extending the earned Transport bridge/runtime rather than creating a
second control subsystem.

A small runtime field set may contain:

- one pending `pstvnc_mpeg_retire_payload_t`;
- one pending flag;
- one dedicated control semaphore if needed for concurrent receiver/main access.

A public bridge surface may use names equivalent to:

- `pstvnc_transport_mpeg_send_start(...)`;
- `pstvnc_transport_mpeg_send_retire(...)`;
- `pstvnc_transport_mpeg_take_retire_completion(...)`.

Names are worker-owned. Keep the payload representation shared from
`transport/protocol.h`; do not duplicate the wire struct.

### Acceptance criteria

- `A003-R6-C1 SOLE_IO_CONTROL_SEND`
- `A003-R6-C2 EXACT_START_ENVELOPE`
- `A003-R6-C3 EXACT_RETIRE_ENVELOPE`
- `A003-R6-C4 INBOUND_RETIRE_COMPLETION`
- `A003-R6-C5 ONE_SLOT_NONBLOCKING_TAKE`
- `A003-R6-C6 SESSION_BOUND_ACCESS`
- `A003-R6-C7 NO_GENERATION_BUSINESS_STATE`
- `A003-R6-C8 NO_SYNTHETIC_PRODUCER_DONE`
- `A003-R6-C9 PURE_MPEG_DATA`
- `A003-R6-C10 NO_DIRECT_PHYSICAL_BYPASS`
- `A003-R6-C11 EXISTING_RIDERS_PRESERVED`
- `A003-R6-C12 CLEAN_EVIDENCE`

All must be MET for Foreman acceptance.

### Explicit non-goals

Do not implement in R6:

- Pi production Wire server/control receiver;
- Pi START validation/prepared-generation owner;
- Pi producer launch/emission admission;
- Pi RETIRE cleanup/ACK implementation;
- Application run generation allocation;
- Presentation arm from calibration;
- R5/R4 worker startup;
- P7 live-loop service;
- producer-done publication;
- residual-byte discard/credit finalization;
- RFB suppression/restoration;
- P5 retirement/reveal;
- successor generation startup;
- Wire Q4 establishment;
- physical MPEG qualification.

### Worker return

Return exact source/test/build/dictionary commits, public control-relay API,
runtime control-slot ownership/synchronization, outbound sole-I/O proof, exact
inbound validation, one-shot completion behavior, session-bound stale-access
proof, direct physical-helper disposition, C1-C12 disposition, canonical CI
evidence, and all non-claims/gaps.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a003-mpeg-generation`;
- WORKER_KEY=`interactive`.

Do not begin Pi product ownership or Application activation in the same shift.

## Deferred dependency graph

Expected remaining work after active A003 R6 is:

1. inspect the completed PS2 control relay against the still-absent mature Pi
   product Wire-server/control owner and choose the smallest owner-correct Pi
   implementation packet required to receive START, own exact remote producer
   admission/retirement and echo RETIRE completion without restoring H1
   monolithic coordinator structure;
2. reconcile the remaining validated-value authority for MPEG queue/credit,
   decoder resource/feed bounds, worker stack/priority/join policy and scheduler
   profile before any live Application activation; do not invent defaults;
3. reconstruct the bounded Application MPEG activation transaction only after
   both cross-Wire control sides and required caller values exist:
   accepted region -> exact run fence -> Presentation arm -> worker/backend
   start -> exact START -> producer admission -> P7 service;
4. reconstruct current-Q7 retirement/failure transaction:
   close new producer/admission -> begin RETIRING -> allow accepted drain while
   RFB rebuilds underneath -> safe worker/decoder stop -> RETIRE completion ->
   Transport producer-done/residual/credit finalization -> seal -> synchronized
   reveal;
5. integrate Wire-loss/failure containment and final shutdown/restart behavior,
   then perform physical qualification on the exact product ELF.

Foreman must choose the exact next seam from returned A003 R6 source rather than
pre-authorizing later implementation.

This is planning only, not worker authority to pre-implement later work.

## Hardware qualification debt

HARDWARE_PENDING=A004 visual geometry/matte/suppression/first-frame qualification; MPEG one-run/repeated-run stale fencing and Wire-loss behavior; current-Q7 overlapped RFB restoration; R1 product runtime where later required; final product Q4 establishment; exact product Wire service/runtime; all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A003-MPEG-GENERATION-CONTROL-RELAY-R6` Reconstruction baton,
independently verify sole-I/O START/RETIRE send, exact inbound RETIRE completion,
one-slot one-shot delivery, session-bound stale-access fencing, pure MPEG DATA
classification, absence of producer-done/generation-business side effects and
removal of the direct physical START bypass. Then choose the next owner-correct
Pi/control or validated-value prerequisite before Application activation.

Do not execute the packet from the Foreman seat.
