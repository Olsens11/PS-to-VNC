# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0003
RECORDED_AT=2026-09-16T05:55:54-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0022
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This is an interactive Foreman pass requested by the operator. It supersedes the stale revision 0002 planning baton while preserving committed product-source, audit, Validation, and Continuity authority. Scheduled Reconstruction A/B and scheduled Foreman seats are currently operator-paused; this state therefore assigns the next substantive A001 packet to the High-thinking interactive Reconstruction B substitute rather than asserting an automated wake.

## Current foreman phase

`A001_BEHAVIOR_CLOSURE_AND_VALIDATION_PREP`

Current branch authority before this Foreman write was `4cd9678a17dd11e3f3d377d922665ea2ed881c08`, global state revision 0022. The long interactive reconstruction continuation has already completed the RFB logical-stream migration, application/Transport descriptor adoption, deterministic V005 fatal convergence, pinned PSTV endpoint correction, live build/topology integration, direct-RFB socket-seam retirement, framing coverage, and a definition-level Transport dictionary. Validation independently records the V005 source shape as supported by static review, with canonical executable confirmation still pending.

The remaining A001 work is now split deliberately:

- Reconstruction owns remaining product-behavior proof/test-source and any source defects those tests expose.
- Foreman owns V004 dictionary/portal/topology/test-registration/build-evidence chores.
- Validation owns V003/V004/V005 disposition and independent execution.
- Physical qualification remains operator/hardware evidence.

## Configuration authority disposition

The previously tracked eight-field `pstvnc_transport_session_config_t` production-producer gap is **not an A001 reconstruction blocker**.

A001 owns the Transport/RFB consumer boundary and therefore correctly accepts a complete, already-validated Transport session value without manufacturing defaults. `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` assigns CONFIG decode/completeness/structural validation and production profile ownership to A002. That audit explicitly classifies the relevant RFB queue/credit, receiver-thread, and physical DATA-payload values as qualified-profile/tuning values supplied through the later configuration/profile mechanism.

Therefore:

- A001 must retain the explicit typed caller-supplied Transport subprofile and fail closed when no production CONFIG/profile producer exists.
- A001 must not invent fixed numeric defaults merely to make no-argument startup live.
- A002 will adopt/reconstruct the production configuration/profile producer and supply narrow owner-specific immutable values, including the Transport subset.
- The absence of that A002 producer does not prevent A001 source/host validation of the parameterized Transport/RFB lifecycle.

This disposition narrows, rather than erases, the evidence boundary: A001 can become `VALIDATION_READY` as a parameterized coherent tranche, while full production startup through negotiated CONFIG remains future A002 work.

## Goal packet — Interactive Reconstruction B substitute

TARGET_START=NEXT_INTERACTIVE_PICKUP
WORKER=interactive-b
SUBSTITUTES_FOR=recon-b
WORK_ITEM_KEY=a001-sole-receiver
GOAL_STATUS=ASSIGNED
ASSIGNING_HEAD=4cd9678a17dd11e3f3d377d922665ea2ed881c08
ASSIGNING_RECONSTRUCTION_STATE_REVISION=0007
ASSIGNING_GLOBAL_STATE_REVISION=0022

### Objective

Close the remaining A001 **product-behavior evidence seam** by adding deterministic host behavior fixtures around the actual Transport physical-stream/runtime implementation—not merely bridge stubs—and correct only reconstruction-owned defects those fixtures expose. Finish with a criterion-by-criterion A001 behavior map suitable for Foreman/Validation handoff. Do not begin A002 and do not perform Foreman-owned dictionary/portal/Makefile chores as substitute work.

This packet is intentionally larger than a microtask. Work through the coherent runtime proof seam in one interactive round as far as tool capacity safely permits, reserving enough capacity to create the required immutable round log.

### REQUIRED_DELIVERABLES

1. **Physical PSTV stream behavior fixture.** Add deterministic host test source for `src/transport/physical_stream.*` with PS2/socket/synchronization dependencies stubbed at the narrow mechanism boundary. Exercise complete header+payload send/receive behavior, direction-local sequence progression, short/exact-I/O failure, sequence/protocol rejection, serialized-send ownership, and Transport-owned I/O interruption used by fatal abort.
2. **Transport runtime behavior fixture.** Add deterministic host test source for `src/transport/runtime.*` sufficient to exercise the real runtime state machine with controlled physical-stream/kernel primitives. Cover sole-receiver dispatch to logical RFB, invalid/unexpected frame failure, producer-driven activity wake semantics, parser-consumption credit return, outbound logical-RFB fragmentation, quiesce REQUEST/BOUNDARY/COMMIT/residual/COMPLETE ordering, and stop/receiver-completion lifetime ordering.
3. **Credit/residual proof.** Explicitly demonstrate that parser-consumed RFB bytes may earn return credit while terminal residual discard does not. Include batch/flush-on-empty behavior and failure propagation where practical.
4. **Lifecycle/failure proof.** Cover representative initialization/start/stop/release failures and prove no receiver-visible allocation is reclaimed before completion/dormancy. Repeated clean initialize/release of the fixture must not inherit stale session authority. Do not reinterpret this as authority to fix the separately inherited H1 next-session receive-poison defect.
5. **Behavior correction if required.** If a deterministic fixture exposes a reconstruction-owned A001 defect, fix it coherently in product source and add the regression assertion in the same round. Do not weaken tests or add timeouts to make a failure disappear.
6. **A001 behavior closure map.** Before ending, re-read all eight required behaviors in `LEDGE_AUDIT_A001_TRANSPORT_RFB.md` and map each to current committed source plus behavior-specific tests. Identify any actual product-behavior obligation still unmet after this packet.

### ACCEPTANCE_CRITERIA

C1. Host-test source directly exercises the current physical-stream implementation sufficiently to prove PSTV framing/sequence/exact-I/O ownership and fail-closed progression; bridge-only stubs are not counted as this proof.
C2. Host-test source directly exercises the current Transport runtime sufficiently to prove there is one physical receive owner/dispatch path and that invalid frame/order/error conditions converge fail closed.
C3. Logical RFB queue/credit behavior proves parser-consumed versus residual-discard accounting, producer-driven activity, and outbound fragmentation without introducing RFB framing into Transport.
C4. Deterministic quiesce/lifetime tests prove REQUEST -> externally selected complete-message BOUNDARY -> COMMIT -> exact residual handling -> COMPLETE remains distinct from application-local fatal abort, and receiver completion/dormancy precedes reclamation.
C5. Representative initialization/start/stop/release failure paths and a fresh subsequent fixture session show unambiguous resource/session ownership. No guessed timeout, second receiver, unsafe close, or diagnostic counter is used as correctness authority.
C6. The eight-field Transport session value remains explicit/caller-supplied. The missing production CONFIG producer is recorded as A002-owned profile/configuration work, not “solved” with constants and not treated as an A001 blocker.
C7. No A002 audio/config implementation, A003+, or silent repair of the inherited H1 receive-poison defect is introduced.
C8. The immutable round log records exact tests actually executed versus `PENDING_LOCAL`; no canonical PASS is inferred from shadow fixtures.

### INVARIANTS / NON_GOALS

- One physical PSTV owner and sole receiver remain Transport.
- RFB owns RFB parsing and complete-server-message safe-boundary selection.
- Transport owns byte/framing/sequence/logical-channel mechanics, not RFB protocol semantics.
- Parser consumption and terminal residual discard remain different accounting events.
- V005 fatal abort remains distinct from finite server-driven RFB quiescence.
- `pstvnc_app_run()` may remain fail-closed until A002 supplies production profile authority; do not make A001 depend on invented live startup values.
- Do not update `SYMBOLS.md`, generated dictionary portals, source-topology allowlists, or canonical Makefile registrations merely as filler; those are Foreman chores unless a product-source change requires a minimal same-change dependency correction.
- Do not begin A002.

### EXPECTED_EVIDENCE

Behavior-specific host fixture source and, where executable on the interactive surface, exact compile/run commands and output. Supplemental shadow-host execution is useful but must be labeled supplemental. Canonical `make -C tests unit`, `scripts/check.sh`, source-dictionary strict/long checks, PS2DEV build, reproducibility/PT_LOAD, and hardware evidence remain separate unless genuinely executed.

### BLOCKER_CONDITIONS

`BLOCKED` requires the higher burden already established for Reconstruction workers: exhaust materially different repository/tool routes, current audit/H1/source authority, and independent acceptance criteria, then reevaluate. Missing Universal Ctags, Foreman-owned V004 chores, or missing A002 production CONFIG producer are **not blockers for this behavior packet**.

A genuine blocker is limited to contradictory newer authority, an actual same-source collision, inability to construct a deterministic behavior seam without exposing/restructuring a separately owned component boundary, or evidence that an A001 audit invariant itself requires redesign beyond this packet.

### FALLBACK_OR_STRETCH

If C1-C8 are met early, perform a final product-source dead-seam/call-chain census for A001 only and remove behaviorally dead Transport/RFB adapter code only when current call/reference evidence proves it unreachable and removal does not cross into A002. Otherwise leave exact residual behavior evidence for the next Foreman/Validation pickup.

## Foreman chore queue / current status

1. **V004 Transport dictionary:** materially complete in `d21ccfe6f6e4015cdea1092ebd0ebd16b1dbd81a`; canonical Universal-Ctags strict/long proof remains pending.
2. **V004 RFB dictionary:** OPEN. `src/rfb/SYMBOLS.md` still describes the deleted `rfb_io.h` socket seam and pre-migration socket-bearing RFB session vocabulary and does not fully inventory `src/rfb/bridge.*`; it must be reconciled to current source.
3. **Generated dictionary portal:** OPEN. `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` omits `src/transport`; regenerate after RFB dictionary reconciliation so counts/status are exact.
4. **Topology/domain adoption:** source-topology policy and continuity checker already include `src/transport`; no duplicate adoption work is needed.
5. **Canonical test registration:** existing `tests/Makefile` registers `transport_protocol_test`, `transport_rfb_channel_test`, `transport_bridge_test`, and `rfb_bridge_test`; register the new physical/runtime behavior fixtures after Reconstruction creates them.
6. **Build topology:** clean linked and strict PS2 compile manifests already include live Transport/RFB bridge topology; retain as current evidence, executable build still PENDING_LOCAL.
7. **CONFIG preflight:** resolved for tranche ownership as above: production configuration/profile producer is A002-owned; A001 keeps only the explicit validated Transport subprofile consumer boundary.
8. **Canonical machine evidence:** `make -C tests unit`, `scripts/check.sh`, long/complete/strict dictionary/topology checks, PS2DEV link, reproducibility/exact ELF/PT_LOAD remain PENDING_LOCAL until actually executable.

## Validation disposition carried forward

- V003 remains formally OPEN / `WAIT_FOR_COHERENT_TRANCHE`; current source is far beyond the original interface-only evidence but Validation owns revision/closure.
- V004 remains formally OPEN pending RFB dictionary/portal/canonical strict evidence.
- V005 remains formally OPEN pending independent canonical executable lifecycle evidence, although independent Validation static review records `RESOLUTION_SUPPORTED_BY_STATIC_REVIEW` / provisional source-shape PASS.
- No A001 `VALIDATION_READY`, full PASS, PT_LOAD qualification, or hardware PASS is claimed by this Foreman state.

## Exact next Foreman pickup

Consume the interactive worker's criterion-level C1-C8 result. Register any new physical/runtime behavior fixtures canonically, finish RFB dictionary + generated portal, obtain whatever canonical machine evidence is available, and then determine whether any reconstruction-owned behavior remains before promoting the tranche for independent Validation. Do not schedule or authorize A002 until A001 reaches the appropriate handoff boundary.
