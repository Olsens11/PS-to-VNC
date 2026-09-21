# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0035
RECORDED_AT=2026-09-21T07:48:11-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0034
SUPERSEDES_FOREMAN_STATE_REVISION=0034
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0035 independently accepts the completed
A003-MPEG-GENERATION-CONTROL-RELAY-R6 Reconstruction baton and advances to a
configuration-owned MPEG runtime profile authority before any live Application
activation. R6 now supplies exact session-bound START/RETIRE control relay
mechanics through the sole Transport physical-I/O owner, with one bounded
RETIRE-completion handoff and no Transport-owned generation business policy.
Physical qualification remains pending.

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

`A003_R6_INTEGRATED__A003_MPEG_RUNTIME_PROFILE_AUTHORITY_RECONSTRUCTION_ACTIVE__PI_WIRE_CONTROL_OWNER_DEPENDENCY_QUEUED__APPLICATION_ACTIVATION_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

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
A003_MPEG_GENERATION_CONTROL_RELAY_FOREMAN_ACCEPTED=YES
A003_MPEG_RUNTIME_PROFILE_AUTHORITY_ACTIVE=YES
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

## A003 R6 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD 6b37d60690be15754d9f0bf1e47f4081f16b1bef;
- final pre-log R6 source/dictionary authority
  31db24ad56515d09a0218b1d9e20718c7bcf9662;
- immutable Reconstruction log
  docs/ledge/work-log/20260921T071223-0400__reconstruction__a003-mpeg-generation__interactive.md;
- exact prior Foreman base
  a646b49ed8200ee127dd96f37d4e3359145c6607.

The substantive R6 source range is a linear eleven-commit descendant of that
Foreman base, followed by the immutable worker-log commit at current HEAD. The
range touches only Transport source/tests, generated/current dictionaries and
Transport ownership documentation. No Application, MPEG decoder/worker,
Display/Presentation, RFB, audio, configuration or Pi product source entered R6.

Independent review confirms:

- public START, RETIRE and nonblocking RETIRE-completion operations require one
  current opaque Transport access authority;
- stale Session-A access is rejected before it can send through or consume
  Session-B state;
- exact START and RETIRE payload codecs remain shared from Transport protocol
  authority rather than duplicated;
- START is kind 11 / control channel 0 / flags 0 / exact 44-byte v1 payload;
- RETIRE is kind 10 / control channel 0 / flags 0 / exact 12-byte v1 payload;
- both outbound controls enter the existing synchronous
  pstvnc_transport_runtime_submit_frame() rendezvous;
- within current Transport product source, the private runtime I/O owner remains
  the sole caller of the physical framed-send primitive;
- inbound exact RETIRE is decoded into one synchronized session-local pending
  completion slot;
- malformed RETIRE and inbound START remain protocol failures;
- the completion slot cannot be overwritten, take is nonblocking and one-shot,
  and a second take returns WOULD_BLOCK;
- Transport never compares the completion against an Application pending-run
  identity;
- RETIRE receipt does not publish MPEG producer_done, synthesize EOF, discard
  residual MPEG bytes, return retirement credit or reopen a successor run;
- DATA/channel4 remains opaque MPEG media even when its 44 bytes happen to have
  START-shaped contents;
- the obsolete direct physical-stream MPEG START convenience helper is removed,
  so product callers cannot bypass the sole-I/O runtime through that surface;
- existing RFB/AUDIO/MPEG DATA mechanics remain outside the new control meaning.

A003-R6-C1 through A003-R6-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

### R6 machine evidence

Canonical final pre-log workflow:

35593508080 at 31db24ad56515d09a0218b1d9e20718c7bcf9662 — SUCCESS.

Observed canonical jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

This is source/host/repository/PS2 compile-link evidence only. No independent
Validation run consumed R6, no physical PS2/Pi generation-control run occurred,
and no machine result is promoted to hardware qualification.

## Why R7 is runtime-profile authority before Application activation

The reconstructed owner mechanisms are intentionally parameteric:

- Transport's MPEG channel requires caller-supplied queue/credit values;
- the MPEG decoder requires caller-supplied resource/feed/alignment bounds;
- the MPEG worker requires caller-supplied stack bytes and priority;
- the PS2 worker binding requires caller-supplied bounded join-observation
  delay/count;
- the Display scheduler requires caller-supplied source-rate/drop policy.

R3-R6/P6 deliberately refused to manufacture these values because mechanism
owners are not configuration policy owners. Current clean CONFIG still carries
only the A001/A002 production subset and does not contain H1's 61-field
laboratory surface.

Repository evidence is sufficient to select one narrow current MPEG runtime
profile without reopening the broad lab protocol:

- the 600-second all-guns event-wake hardware confirmation used MPEG queue and
  initial credit 524288 bytes with 8192-byte credit batches;
- the mature H1 video profile and decoder mechanism used a 704x480 maximum,
  RGB16/two-byte pixels, 2048-byte feed payload, 16-byte transfer alignment and
  64-byte backing-buffer alignment;
- the hardware-proven safe-stop worker used a 64-KiB EE stack, priority 67 and
  a fail-closed 1000-us by 3000-observation dormancy wait;
- the qualified presentation path is absolute common-clock scheduling at
  30000/1001 source cadence; the current all-guns profile leaves lateness-drop
  disabled, so drop threshold remains zero until a separately qualified product
  profile elects to enable it.

Those historical values are evidence for selecting the initial clean profile;
they are not a claim that reconstructed source is physically qualified.

R7 therefore makes the selection explicit in Configuration ownership while
reusing the already-defined narrow owner value types. It does not extend the
session CONFIG wire with laboratory knobs, does not add a generic settings
object, and does not activate any run.

The Pi product Wire runtime remains a required downstream dependency. It is not
chosen as R7 because the current ledge has no mature product Pi Wire
server/service/source topology; combining that topology decision, Q4 product
establishment, control ownership and MPEG producer lifecycle in one immediate
packet would be broader than the already-defined value-authority seam.

## Active bounded Reconstruction packet

PACKET_ID=A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7
PACKET_STATUS=ACTIVE
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
ASSIGNING_BASE_HEAD=REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE
FOREMAN_DECISION_BASE=6b37d60690be15754d9f0bf1e47f4081f16b1bef

### Objective

Reconstruct one narrow Configuration-owned selected MPEG runtime profile that
supplies the already-accepted Transport, MPEG decoder/worker/PS2 binding and
Display scheduler public value types with explicit evidence-selected values.

R7 is value authority only. It must not widen the current CONFIG wire into H1's
laboratory parameter surface and must not wire a live Application MPEG run.

### Required authority

Read current at wake, including:

- AGENTS.md and CONTRIBUTING.md;
- Clean Architecture and current source-topology/naming/lifecycle documents;
- Reconstruction Contract rev 0006 and work-log contract rev 0007;
- Foreman State rev 0035;
- Wire Runtime Decisions rev 0011 and Architecture Overlay rev 0004;
- A002 CONFIG/audio/clock audit;
- A003 MPEG audit and A004 presentation/calibration audit;
- current public value types in:
  - src/transport/transport.h;
  - src/mpeg/decoder.h;
  - src/mpeg/worker.h;
  - src/mpeg/ps2_worker_runtime.h;
  - src/display/mpeg_scheduler.h;
- current src/config profile ownership and tests;
- frozen H1 value evidence, especially:
  - h1_profiles.py;
  - h1_video_runtime.c;
  - h1_cp2p_mpeg_worker.c;
  - CP2P_MPEG_EVENT_WAKE_HARDWARE_RESULT.md;
  - CP2P_MPEG_SAFE_STOP_LIFECYCLE.md;
- current R3/R4/R5/P6 Foreman acceptance sections so mechanism ownership is not
  accidentally moved into Configuration.

### Required behavior

1. Add one small Configuration-owned MPEG runtime-profile authority. Prefer a
   focused config module rather than enlarging the general CONFIG decoder with
   unrelated implementation knobs.
2. Reuse the existing public owner value types. Do not duplicate Transport,
   decoder, worker, PS2 worker-runtime or Display scheduler structures merely to
   rename their fields.
3. Publish exactly one current selected profile with these Transport MPEG
   channel values:
   - queue_capacity = 524288;
   - initial_credit_bytes = 524288;
   - credit_batch_bytes = 8192;
   - credit_flush_on_empty = 1;
   - credit_return_enabled = 1.
4. Publish exactly one current decoder profile with:
   - max_width = 704;
   - max_height = 480;
   - bytes_per_pixel = 2;
   - feed_payload_capacity = 2048;
   - transfer_alignment = 16;
   - buffer_alignment = 64.
5. Publish exactly one current MPEG worker profile with:
   - worker_stack_bytes = 65536;
   - worker_priority = 67.
6. Publish exactly one current PS2 worker-runtime policy with:
   - join_poll_delay_us = 1000;
   - join_poll_max_count = 3000.
   This is a bounded fail-closed dormancy observation policy, not permission to
   treat timeout as successful retirement or to force-delete a live thread.
7. Publish exactly one current Display scheduler profile with:
   - fps_numerator = 30000;
   - fps_denominator = 1001;
   - drop_enabled = 0;
   - drop_threshold_milliframes = 0.
   Preserve the existing absolute scheduler implementation. Do not resurrect
   scheduler-comparison modes.
8. Do not add MPEG queue/feed/thread/scheduler tuning fields to the current
   session CONFIG wire. The selected runtime profile is internal product
   configuration authority, not a user/network sweep surface.
9. Do not productize H1 MPEG_EMPTY_DELAY_US, VIDEO_IPU_RESET_EACH_SESSION,
   stage markers/holds, profile IDs, allocation-order experimentation,
   scheduler-mode selection, socket-buffer knobs or arbitrary drop tuning.
10. Do not create a second geometry authority. Active MPEG calibration/region,
    START geometry and Presentation base/inner/outer regions stay with their
    current owners. The 704x480 value is a decoder resource bound, not active
    presentation geometry.
11. Keep video presentation offset in the existing media-clock/session profile
    authority; R7 does not duplicate it.
12. Keep MPEG start-target/prefill polling knobs out of the clean selected
    profile unless current clean source has an actual owner/API requiring them.
    Current event-driven decode starvation and START ordering must not acquire a
    timer-poll correctness dependency merely to preserve H1 fields.
13. No module may silently substitute a different local default for an R7 value.
    Later Application wiring must be able to obtain the selected profile and
    pass its narrow owner values explicitly.
14. Add focused host tests that assert the exact selected values, prove the
    profile is immutable/copy-safe as appropriate, and prove no CONFIG-wire
    field-count/schema expansion occurred.
15. Preserve clean source synopsis, naming, dictionary and topology contracts;
    update direct build/test manifests for any new maintained source.
16. Preserve the current product call graph: no Transport session open, decoder
    start, worker start, Presentation activation, scheduler initialization,
    START/RETIRE send, Pi service or Application loop wiring may be introduced
    by R7.

### Acceptance criteria

- A003-R7-C1 CONFIG_OWNS_SELECTED_MPEG_PROFILE
- A003-R7-C2 EXACT_TRANSPORT_VALUES
- A003-R7-C3 EXACT_DECODER_VALUES
- A003-R7-C4 EXACT_WORKER_EXECUTION_VALUES
- A003-R7-C5 EXACT_SCHEDULER_VALUES
- A003-R7-C6 EXISTING_OWNER_TYPES_REUSED
- A003-R7-C7 CONFIG_WIRE_NOT_BROADENED
- A003-R7-C8 LAB_KNOBS_NOT_PRODUCTIZED
- A003-R7-C9 GEOMETRY_OWNERSHIP_PRESERVED
- A003-R7-C10 NO_LIVE_ACTIVATION
- A003-R7-C11 FOCUSED_TESTS_AND_STRICT_DICTIONARIES
- A003-R7-C12 CLEAN_BUILD_EVIDENCE

All must be MET for Foreman acceptance.

### Invariants

- Configuration selects values; mechanism owners continue to validate and use
  only their own narrow types.
- Transport does not acquire decoder/Presentation/Application policy.
- MPEG does not acquire Transport queue internals or Display policy.
- Display scheduler remains pure policy over an already-armed media clock.
- Application remains the future cross-domain lifecycle coordinator.
- Wire Session identity and MPEG generation identity remain distinct.
- R7 does not alter current Q7 retirement ordering.
- Hardware evidence for H1 values is provenance, not reconstructed-code
  hardware qualification.

### Explicit non-goals

Do not implement in R7:

- a Pi product Wire server/service;
- Q4 product establishment;
- Pi START handling or prepared-generation state;
- Pi producer launch/emission/RETIRE cleanup;
- Application generation allocation;
- MPEG calibration or active-region policy;
- RFB suppression/restoration;
- decoder/worker startup;
- P7 live-loop wiring;
- START/RETIRE runtime calls;
- producer_done publication;
- residual-byte/credit finalization;
- Q7 retirement orchestration;
- Wire-loss MPEG recovery;
- new display-mode/profile selection UI;
- physical PS2/Pi qualification.

### Evidence required

Worker return must identify:

- exact source/test/dictionary/build commits;
- selected profile public seam and owner-type reuse;
- exact values for each owner subprofile;
- proof that current CONFIG wire field count/schema is unchanged;
- proof no live activation call graph was added;
- focused host-test results;
- canonical project-check/dictionary result;
- direct PS2 compile evidence for new maintained C where applicable;
- linked/current-source reproducibility result;
- explicit machine-vs-hardware evidence boundary.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=reconstruction;
- WORK_ITEM_KEY=a003-mpeg-generation;
- WORKER_KEY=interactive.

Stop after R7. Do not begin Pi product Wire ownership or Application activation
in the same shift.

## Deferred dependency graph

After accepted R7:

1. define the smallest mature Pi product Wire runtime/control-owner foundation
   needed for provisional Q4 establishment, one physical session/I/O owner and
   exact START/RETIRE control ownership without importing the H1 monolith;
2. add Pi MPEG producer admission/retirement behind that owner, preserving exact
   run fencing and the R6 completion contract;
3. reconstruct bounded Application MPEG activation only after Pi control and
   selected runtime values both exist:
   accepted MPEG region -> run fence -> Presentation arm -> local worker/backend
   start -> exact START -> producer admission -> P7 service;
4. reconstruct current-Q7 retirement/failure:
   close new producer/admission -> RETIRING -> accepted MPEG drains while RFB
   rebuilds underneath -> safe worker/decoder stop -> exact RETIRE completion ->
   Transport producer_done/residual/credit finalization -> seal -> synchronized
   reveal;
5. integrate Wire-loss containment, repeated-run/repeated-session behavior and
   final shutdown/re-admission;
6. perform exact-source physical qualification on the reconstructed product
   path and final ELF.

## Hardware qualification debt

HARDWARE_PENDING=A003 R3/R4/R5/R6/R7 reconstructed MPEG runtime; one-run and repeated-run stale fencing; Wire-loss during MPEG; current-Q7 overlapped RFB restoration; A004 presentation-visible geometry/matte/suppression/first-frame timing; final product Q4/Wire Pi service; all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle

## Foreman next pickup

Consume A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7. Independently verify exact
selected values, no CONFIG-wire expansion, no duplicated owner types, no live
activation and canonical machine evidence. Then choose the smallest Pi
Wire/control owner foundation; do not skip directly to Application orchestration.

Do not execute the active packet from the Foreman seat.

