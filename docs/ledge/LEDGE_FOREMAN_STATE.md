# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0004
RECORDED_AT=2026-09-16T06:41:10-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0023
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman pass consumes the completed Reconstruction B substitute packet from revision 0003, the independent 06:20 Validation review, and Continuity global state revision 0023. It supersedes Foreman revision 0003 as the current planning/integration baton.

## Current foreman phase

`A001_FOREMAN_INTEGRATION_AND_VALIDATION_HANDOFF`

Pre-write branch authority for this state was `ec7ea35a90f11f43a79f6c6718b3f377c4909785` on `ledge/h1-all-guns`.

The interactive Reconstruction B substitute completed the entire revision-0003 C1-C8 behavior packet and recorded `FOREMAN_GOAL_RESULT=MET`. It added direct host behavior fixtures around the real Transport physical-stream/runtime implementations, found no A001 product-source defect requiring correction, and mapped all eight A001 audit behaviors to current source/tests. Validation independently reviewed the new fixtures through `bce128b5...`, opened no new product defect, and retained V003/V004/V005 formally OPEN pending canonical integration/execution.

Foreman therefore does **not** assign another Reconstruction behavior packet at this point. Reconstruction A/B and the interactive B substitute are on hold for A001 unless canonical/independent evidence exposes a concrete reconstruction-owned defect. Beginning A002 now would be premature because A001 still lacks the required Foreman/Validation handoff boundary.

## Foreman work completed this pass

### Canonical direct-fixture registration — COMPLETE AT SOURCE LEVEL

The two new direct behavior fixtures are now registered in `tests/Makefile` under the canonical `unit` target:

- `tests/unit/transport_physical_stream_test.c` -> `transport-physical-stream-unit`;
- `tests/unit/transport_runtime_test.c` -> `transport-runtime-unit`.

The physical-stream target links the current `src/transport/physical_stream.c` and `protocol.c` against the narrow host kernel/PS2IP seams. The runtime target links the current `runtime.c`, `quiesce.c`, `rfb_channel.c`, and `protocol.c`, adds the host kernel seam, and uses `-pthread` for the deterministic host thread/synchronization fixture.

Registration was performed through three commits because the first whole-file connector replacement accidentally changed one unrelated dependency line and the first correction also reordered one existing dependency. Both were immediately corrected. The net branch delta from pre-Foreman authority `d57c08220b7e613e6fd1d111c2702b3fa4d99b2e` through `ec7ea35a90f11f43a79f6c6718b3f377c4909785` is exactly one `tests/Makefile` modification with **51 additions and zero deletions**. No unrelated Makefile content remains changed.

Commits:

- `f23682487dcb715ae1326648349b0230cc2f8553` — initial direct-fixture registration;
- `26d1df03177f9ec1ca136b92b8b9299adb98dad6` — restore accidentally altered presentation dependency;
- `ec7ea35a90f11f43a79f6c6718b3f377c4909785` — restore original OSK dependency ordering.

The final net result is the intended registration only.

### Canonical execution — STILL PENDING

This connected Foreman surface cannot execute the repository checkout. A direct Git/network probe from the local container cannot resolve GitHub, and no GitHub Actions run exists for `ec7ea35...`. Therefore no canonical `make -C tests unit`, `scripts/check.sh`, strict/long dictionary audit, PS2DEV link, reproducibility, ELF/PT_LOAD, or hardware PASS is claimed here.

Reconstruction's previously recorded physical-stream/runtime/RFB-channel shadow executions remain useful supplemental evidence only.

## Configuration authority disposition

Unchanged from revision 0003: the missing production producer of `pstvnc_transport_session_config_t` is A002-owned configuration/profile work, not an A001 blocker and not license to invent defaults. A001 correctly retains the explicit caller-supplied validated Transport subprofile boundary.

## A001 behavior disposition

No residual A001-owned product-behavior obligation is presently identified after the C1-C8 packet.

Current source/test evidence covers the eight A001 audit behaviors: one physical PSTV owner/sole receiver; Transport sequence/framing authority; logical RFB byte delivery; parser-consumption versus residual-discard credit distinction; producer-driven activity; finite RFB REQUEST -> BOUNDARY -> COMMIT -> residual -> COMPLETE quiescence; RFB-owned complete-message parser boundaries; and application/presentation policy above parser/framebuffer authority.

That is a Reconstruction/Foreman behavior assessment, **not** a Validation PASS and not hardware qualification.

## Remaining Foreman integration queue

1. **V004 RFB dictionary — OPEN.** `src/rfb/SYMBOLS.md` still contains the deleted `rfb_io.h` socket seam and stale socket-bearing `rfb_session` vocabulary and does not fully inventory the current `src/rfb/bridge.*` definitions. It must be reconciled to current source.
2. **Generated symbol portal — OPEN.** `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` still omits `src/transport`. Regenerate only after the RFB dictionary is reconciled so the portal counts/status remain exact rather than hand-guessed.
3. **Transport dictionary — SOURCE COMPLETE / MACHINE PROOF PENDING.** `src/transport/SYMBOLS.md` is definition-level complete in committed source, but canonical Universal-Ctags strict/long execution remains pending.
4. **Topology/domain adoption — COMPLETE AT SOURCE LEVEL.** `src/transport` is already present in current source-topology policy and continuity checker; do not redo it.
5. **Direct-fixture registration — COMPLETE AT SOURCE LEVEL.** The two new direct fixtures are now part of the canonical Makefile target graph.
6. **Canonical machine evidence — PENDING_LOCAL.** Host unit suite, project check, dictionary strict/long, clean PS2DEV compile/link, reproducibility and exact ELF/PT_LOAD evidence require an execution-capable repository surface.
7. **Hardware — HARDWARE_PENDING.** No reconstructed-DUT PS2 qualification has occurred.

## Validation disposition carried forward

- V003 remains formally OPEN / `WAIT_FOR_COHERENT_TRANCHE`; current source is now behaviorally coherent by Reconstruction/Foreman evidence, but Validation owns promotion.
- V004 remains formally OPEN until the RFB dictionary/portal and canonical strict evidence are complete.
- V005 remains formally OPEN despite source-shape support and stronger lifecycle fixtures; Validation still requires canonical executable integration evidence before revisioning the finding.
- `VALIDATION_READY` remains unclaimed until the V004 integration gate is reconciled and the evidence boundary is explicit.

## Reconstruction assignment status

RECONSTRUCTION_A_PACKET=NONE
RECONSTRUCTION_B_PACKET=NONE
INTERACTIVE_B_PACKET=NONE
REASON=NO_IDENTIFIED_A001_PRODUCT_BEHAVIOR_GAP_AFTER_C1_C8;FOREMAN_V004_AND_MACHINE_EVIDENCE_REMAIN

Reconstruction workers must not repeat revision-0003 C1-C8, must not consume Foreman dictionary/portal chores as filler, and must not begin A002 without a fresh Foreman packet after the A001 handoff boundary. If canonical evidence exposes a concrete A001 defect, Foreman will issue a narrowly scoped fresh behavior packet against that evidence.

## Exact next Foreman pickup

Reconcile `src/rfb/SYMBOLS.md` against current RFB source and regenerate the source-symbol portal, then obtain canonical machine evidence on an execution-capable checkout. Once those results exist, hand the coherent A001 tranche to Validation for independent V003/V004/V005 disposition. If Validation finds no reconstruction-owned defect and the A001 handoff boundary is satisfied, only then may Foreman publish the first A002 packet.
