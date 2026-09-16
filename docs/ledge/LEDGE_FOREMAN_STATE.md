# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0008
RECORDED_AT=2026-09-16T09:27:54-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0003
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0024
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed first A002 Reconstruction round, the concurrent provisional Validation review of that bounded CONFIG/profile tranche, and the Foreman-owned integration/evidence work performed after Reconstruction returned the baton. The live branch was re-read immediately before this state write at `ecebe6b0f9bff3a71f0587566cdf7b91314eb6ea`.

Global work state revision 0024 and Reconstruction state revision 0007 are point-in-time snapshots older than the completed A002 CONFIG/profile work. They remain temporally valid for what they recorded and are not rewritten here.

## Current foreman phase

`A002_MEDIA_CLOCK_PACKET_ISSUED`

The A002 CONFIG/profile foundation packet is complete from Reconstruction's side. Independent Validation has reviewed that bounded source tranche provisionally and opened no product-source finding. Foreman integration has registered its host fixture, linked its sources into the clean PS2 build graph, reconciled the clean-source dictionary/topology surfaces, and obtained positive host/link/reproducibility evidence. A final canonical workflow pass on the fully reconciled settled tree is expected from this state write and remains evidence work rather than a Reconstruction blocker.

A001 physical PS2 qualification remains `HARDWARE_PENDING`. Nothing in this state converts machine evidence into physical qualification.

## Reconstruction result consumed

WORK_ITEM_KEY=`a002-audio-clock`
WORKER=`interactive`
WORKER_STARTING_HEAD=`91eb25bcd69183e4d0ac2ec216e2f5a630cc9628`
WORKER_LOG_COMMIT=`150ded8e6f82cc154a994ec7a8c443c09e23e660`

The worker's substantive A002 CONFIG/profile commits were independently inspected rather than accepted from its prose summary:

- `57abc3d85e7ef09e042bec08912d76683ab046d5` — bounded session/profile authority;
- `d9fd5c0fef51fa6ea07a8ff029b5299caa095133` — versioned decode and validation;
- `1d4bec9fa9178623754b89ee21ca7adcfedff027` — behavior-specific decoder/profile tests;
- `f2992a472fd685c0a25591d1b8cba3018cee1ab3` / `73ee94367466ff6c6a7790f7aec901c6086af571` — clean config-text declaration/implementation hygiene;
- `5f53802469e3297ff12f7be25cfe7cd5247ce366` / `8c7a6c5790a4a2d125a37ffb187960d8bbe1e5d` — config dictionary/topology source-side adoption;
- `cff9ca4582d8f02305b97726fdf338e0716f59e5` / `3f4d2480b825739b8cf425ddec5e98fd489ad3cf` / `def52043730f3ef3c8d56d67b7291abbcb130930` — raw representability, mandatory Transport provenance, and boundary tests;
- `0882101bd19c92f45b06afbaba39e02f48b2f8e4` introduced incidental living-map formatting noise and `0cd60129977f4f4fac1c55b8f9e012345139ee5c` self-corrected it before the worker logged the round.

Files directly inspected included `src/config/profile.h`, `src/config/profile.c`, `src/config/text.h`, `src/config/text.c`, `src/config/SYMBOLS.md`, `tests/unit/config_profile_test.c`, `docs/development/source-topology.md`, and `docs/reference/FILE_AND_SERVICE_MAP.md`.

### Prior packet disposition

C1 `MET` — one explicit config-owned decoded session/profile aggregate exists with narrow composition, existing Transport, PCM, and media-clock subprofiles rather than an H1-sized god object.

C2 `MET` — all eight A001 Transport values are mandatory explicit profile authority; absent values are rejected rather than defaulted.

C3 `MET` — the decoder is side-effect free and rejects unsupported version, malformed exact shape, unknown identity, duplicates, missing mandatory fields, and structural-invalid values before publication.

C4 `MET` — stable A002 PCM/common-clock facts are represented, including exact signed offset interpretation, without importing A003 MPEG/presentation tuning.

C5 `MET` — Transport, future audio, and future common-clock consumers can consume narrow immutable subprofiles.

C6 `MET` — deterministic host-test source covers valid decode and the required negative/boundary cases.

C7 `MET` — current A001 runtime behavior remains unchanged; the application remains fail-closed and the Transport/RFB ownership model was not redesigned.

C8 `MET` — no PCM worker, AUDSRV runtime, shared-clock wait/arming runtime, MPEG/video reconstruction, or hardware claim was introduced.

No worker criterion is `BLOCKED`. No product-behavior correction packet is required. The one detected worker scope drift was formatting-only and was self-corrected before handoff.

## Concurrent Validation consumed

Validation commit `9ab326547cd0c74294a76d048e60703609aa8ca0` independently reviewed the bounded A002 CONFIG/profile source while Foreman integration was moving. It recorded `REVIEWED_SOURCE_PROVISIONAL`, found no new product-source defect, opened no finding, and explicitly handed remaining canonical registration/dictionary/build/reproducibility evidence to Foreman. No Validation state/findings revision was produced.

This Foreman state does not convert that provisional review into independent Validation PASS.

## Foreman-owned integration and evidence

The following non-behavioral integration work was completed after the worker returned:

- `e9852403b23b90420f8217dae5374167dc97579d` — registered `config_profile_test` in the canonical host `unit` target;
- `adde0646b5f2f5f27790b168ca5dd8bb0fc84603` — linked `src/config/profile.c` and `src/config/text.c` into the clean PS2 build graph;
- `de2cf98d5c336c1d62e931833af128f90ace14c7` — adopted `src/config` in the clean topology checker; its only incidental extra diff is nonsemantic blank-line formatting in that checker;
- `72fce0ee508767a056f7006ed882c399fc0db301` — allowed the reconciliation workflow to stage `src/config/SYMBOLS.md`;
- `54d53adecd80ea34b4fe922ee4d564c2a578bb72` — extended the deterministic dictionary reconciler to the current `src/config` domain;
- generated child `ecebe6b0f9bff3a71f0587566cdf7b91314eb6ea` — reconciled eight CONFIG public type rows to the validator's canonical `type` kind and regenerated the source-dictionary portal with `src/config` COMPLETE.

The initial standard long-dictionary/project-check jobs exposed the eight type-kind documentation mismatches. That was a Foreman-owned documentation/integration defect, not a DUT behavior defect. The deterministic reconciliation job then reported:

- `DICTIONARY_RECONCILED=src/config before=133 removed=8 added=8`;
- no changes in the other selected reconciliation directories;
- `SOURCE_DICTIONARY_CHECK_MODE=LONG`;
- `SOURCE_DICTIONARIES=PASS`;
- `git diff --check` success before the generated reconciliation commit.

Canonical host evidence at integration authority `54d53adecd80ea34b4fe922ee4d564c2a578bb72` reports the complete host unit job `PASS`, including `config_profile_test: PASS`.

The PS2 linked-build job at the same authority explicitly compiled both `src/config/profile.c` and `src/config/text.c`, linked them into the current clean ELF twice, and reported:

- `ISSUE7_LINKED_BUILD=PASS`;
- `ISSUE7_RUNTIME_IDENTITY_LINKED=YES`;
- pristine ELF SHA256 `89b9b787684d0d607f2124f34aeeff214f9c5bd97a24990270ce5285c029532f` on both builds;
- `PT_LOAD_SEGMENTS=1`;
- PT_LOAD SHA256 `faa255c022eb44423c2e5a9b1bae19f60e7ec9da319cd533e0ea1eeb7d227d65`;
- `PT_LOAD_BYTES=414728`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The separate historical Issue #7 strict compile job also passed, but because that stage-specific script predates A002 and does not itself enumerate the new config sources, A002 CONFIG compile/link evidence comes from the linked build above, not from overclaiming the stage-specific compile job.

At the instant of this state snapshot, a standard `scripts/check.sh` / ordinary long-dictionary run has not yet completed on the post-reconciliation `ecebe6b0...` tree. This state commit is intended to trigger that settled-tree workflow. Foreman must inspect its result before closing the immutable round log.

PENDING_LOCAL=settled-tree canonical workflow result after this state write
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; no A002 physical qualification is claimed

## A002 dependency conclusion

The next coherent A002 dependency is the shared session media-clock contract. The CONFIG/profile foundation now gives it immutable `epoch_lead_us`, signed audio presentation offset, and signed neutral video presentation offset. A002 audit revision 0001 establishes one common session epoch, publication ordering, signed deadline behavior, startup-before-presentation semantics, and stop/failure behavior. Those semantics can be reconstructed and host-tested without selecting the later A003 MPEG/video presentation callsite or starting the PCM/AUDSRV worker.

H1's `h1_media_clock.*` is evidence, not an implementation template. Its proven semantics include initialization to unarmed, one-shot common-epoch publication, signed offset deadline calculation with saturation, stop-aware waits, and explicit timer/delay failure. Its historical `volatile`/`EE_SYNCL` mechanism must not be copied blindly; the clean owner needs an explicit synchronization contract appropriate to clean architecture and deterministic testing.

## Fresh interactive Reconstruction packet

WORK_ITEM_KEY=`a002-audio-clock`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`ecebe6b0f9bff3a71f0587566cdf7b91314eb6ea`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md:0001`
ASSIGNING_VALIDATION_STATE=`0006`
ASSIGNING_VALIDATION_REVIEW=`9ab326547cd0c74294a76d048e60703609aa8ca0`

### Objective

Reconstruct A002's clean session-scoped common media-clock owner and its deterministic deadline/wait semantics using the immutable media-clock profile produced by the completed CONFIG foundation. Establish one safely published epoch shared by audio and neutral video timing, exact signed/saturating deadline math, and a testable stop/error-aware wait contract without beginning PCM/AUDSRV runtime or selecting A003 MPEG/presentation callsites.

This is a substantial timing/synchronization packet, not merely a helper-function exercise. It should establish the complete reusable A002 common-clock boundary that later PCM and video-side owners can consume.

### Required deliverables

1. **Session-scoped common-clock owner.** Add the smallest coherent clean owner for one session media epoch. It must consume immutable A002 media-clock profile values and must not turn config, app, or Transport into a mutable timing god object.

2. **One-shot synchronized publication.** Establish a clear unarmed -> armed transition in which the epoch value is fully published before observers can see the armed state. Concurrent observation must not permit `armed` with a stale/partial epoch. Repeated arm attempts must be idempotent and must not silently move an already-published session epoch.

3. **Pure signed/saturating deadline math.** Provide deterministic owner-level logic for deriving audio and neutral-video presentation deadlines from the same epoch plus their signed offsets. Required semantics include:
   - unarmed deadline is unavailable/fail-closed rather than fabricated;
   - negative signed offset underflow saturates to zero;
   - positive signed offset overflow saturates to `UINT64_MAX`;
   - any additional deadline/tick addition also saturates rather than wraps;
   - exact `int32_t` signed offset meaning from CONFIG is preserved.

4. **Clock/wait abstraction.** Separate pure timing math/state from the PS2 timer/delay mechanism sufficiently for deterministic host testing. Use a narrow abstraction or injected observer where needed. Do not copy H1's `volatile` + fence implementation merely because it existed historically.

5. **Stop- and failure-aware wait contract.** Reconstruct the A002 wait semantics needed by later consumers without implementing those consumers. A wait must never report deadline-ready before the common epoch is armed and the computed deadline is reached; it must honor session stop/cancel, surface timer/read/delay failures explicitly, and deliberately prevent a zero-poll configuration from becoming an accidental busy spin.

6. **Behavior-specific deterministic test source.** Add host-test source proving at minimum:
   - initialized/unarmed state;
   - one successful arm and idempotent repeated arm;
   - correct publication/observation ordering under a controlled concurrent or equivalent deterministic witness;
   - one shared epoch yielding independent audio/video deadlines;
   - positive and negative normal offsets;
   - `INT32_MIN` / `INT32_MAX` boundary interpretation;
   - underflow saturation to zero;
   - overflow saturation to `UINT64_MAX`, including additional-delay/tick overflow;
   - unarmed deadline rejection;
   - stop-before-ready behavior;
   - timer/delay failure propagation;
   - zero-poll handling without an unbounded busy-spin path.

7. **Source-writing/topology discipline.** Obey current clean source naming/synopsis/dictionary/topology policy. If a new clean domain is genuinely warranted, perform the Reconstruction-owned source-side directory/dictionary adoption required by policy. Do not perform generated portal or canonical test/build registration work that belongs to Foreman.

8. **Preserve audit boundaries.** Do not wire a video decoder/presentation callsite, choose an MPEG arm point, implement late-frame drop/repeat policy, begin PCM ingestion/playback, initialize or teardown AUDSRV/LIBSD, or claim hardware qualification. Narrow type-level hooks needed so future A002 consumers can depend on the clock are allowed; runtime media consumers are not.

### Acceptance criteria

M1. One session-scoped common media-clock owner exists with no cross-domain mutable global/config ownership leak.

M2. Epoch publication is one-shot/idempotent and synchronization-safe: a reader cannot observe armed state without the matching fully published epoch.

M3. Audio and neutral-video deadlines derive from one shared epoch using exact signed offsets and saturating arithmetic; unarmed state never fabricates a usable deadline.

M4. Timing state/math is separated from platform timer/delay primitives sufficiently for deterministic host tests and future PS2 integration.

M5. The wait contract is stop-aware, failure-aware, and cannot accidentally busy-spin on zero polling cadence.

M6. Behavior-specific host tests prove the required state, synchronization/publication, signed-boundary, saturation, stop, failure, and zero-poll semantics rather than only happy-path arithmetic.

M7. Existing A001 Transport/RFB behavior and completed A002 CONFIG/profile behavior remain stable; no missing timing value is replaced with an application default.

M8. No PCM/AUDSRV runtime and no A003 MPEG/presentation behavior is introduced.

### Preserved invariants / non-goals

- The completed CONFIG/profile boundary and all eight explicit A001 Transport session values remain immutable caller-supplied authority.
- One physical PSTV connection and one Transport-owned sole receiver remain unchanged.
- There is one common session media epoch, not independent audio/video origins.
- Audio may eventually prefill before the epoch/deadline, but this packet does not implement PCM consumption and nothing may redefine presentation-ready as merely buffer-ready.
- Do not select the A003 video qualification/arm callsite from the Foreman packet; reconstruct the reusable A002 clock owner only.
- Do not implement MPEG generation/decoding/presentation, frame lateness policy, or decoder lifecycle.
- Do not add `audsrv_quit()` or any per-session AUDSRV teardown policy here.
- Do not silently repair the inherited H1 next-session receive-poison defect.
- A001 physical status remains `HARDWARE_PENDING`.

### Blocker burden

Lack of physical hardware is not a blocker for this packet because the clock state/math and platform observer boundary are intentionally host-testable.

Uncertainty about the exact future A003 video callsite is not a blocker; do not choose that callsite. Likewise, absence of the future PCM worker is not a blocker; expose the clock/wait contract it will consume.

If PS2SDK synchronization primitive selection cannot yet be proven without prematurely wiring runtime media consumers, establish the narrow synchronization/platform boundary plus deterministic contract tests and leave only the concrete PS2 binding explicitly `PENDING_LOCAL` rather than weakening publication semantics or guessing at a cross-domain owner.

After any blocked subpart, explicitly continue all independent M1-M8 work that remains safely authorized.

### Fallback / stretch

If M1-M8 are genuinely complete early, the only authorized stretch is a pure A002 startup-reservoir policy/value state and deterministic tests that distinguish `reservoir-ready` from `clock/deadline-ready`. Do not start a PCM worker, consume Transport audio DATA, call AUDSRV, or expand into A003.

## Assignment status

RECONSTRUCTION_A_PACKET=NONE
RECONSTRUCTION_B_PACKET=NONE
INTERACTIVE_PACKET=A002_MEDIA_CLOCK_FOUNDATION
A002_PACKET=ACTIVE
A002_CONFIG_PROFILE_RECONSTRUCTION=MET
A002_CONFIG_PROFILE_VALIDATION=REVIEWED_SOURCE_PROVISIONAL
A001_MACHINE_SOURCE_STATUS=PASS
A001_HARDWARE_STATUS=HARDWARE_PENDING
SCHEDULED_A_B_FOREMAN_STATUS=OPERATOR_PAUSED_UNCHANGED

## Exact next pickup

The High-thinking interactive Reconstruction worker should consume this packet from current repository authority and execute M1-M8 as one sustained round, leaving exactly one canonical immutable `reconstruction/a002-audio-clock/interactive` work log before returning the baton.

Foreman should then inspect every produced timing commit/test and perform only the non-behavioral registration/build/dictionary/evidence work. Independent Validation may separately re-review the bounded CONFIG/profile machine evidence after the settled workflow completes; that review does not require pausing the authorized A002 media-clock reconstruction packet.
