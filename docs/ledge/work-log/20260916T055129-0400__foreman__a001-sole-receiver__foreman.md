# Ledge Reconstruction Foreman — Interactive A001 closure planning

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T05:51:29-04:00
COMPLETED_AT=2026-09-16T05:59:52-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=foreman
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=4cd9678a17dd11e3f3d377d922665ea2ed881c08
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

The operator explicitly requested that this interactive session act as Reconstruction Foreman rather than waking the scheduled Foreman. Scheduled Reconstruction A/B and scheduled Foreman seats were already operator-paused and were not modified by this shift.

Consumed current branch authority `4cd9678a17dd11e3f3d377d922665ea2ed881c08`, reconstruction contract revision 0005, work-log contract revision 0001, global state revision 0022, stale historical reconstruction state revision 0007, stale Foreman revision 0002, A001 audit authority, A002 CONFIG/audio-clock audit authority, current V003/V004/V005 findings/evidence, the completed interactive Reconstruction B continuation log, current Transport/RFB/application source, current canonical test Makefile, source-topology/dictionary policy, and the generated dictionary portal.

Unknown Pi-local state was not overwritten or declared absent. No scheduled automation state was changed.

## Planning result

Published `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0003 in commit `54566b16015fac5e2c7c6ea11d834e6baccbbed6`.

The new phase is `A001_BEHAVIOR_CLOSURE_AND_VALIDATION_PREP`. It consumes the substantial interactive reconstruction already committed instead of reissuing the obsolete RFB-session/application-migration packets from Foreman revision 0002.

The fresh bounded packet is assigned to `interactive-b` as the operator's High-thinking Reconstruction B substitute. Its objective is the remaining A001 product-behavior evidence seam: deterministic host fixtures around the actual Transport physical-stream and runtime implementation, with reconstruction-owned defect correction only when those fixtures expose one.

Acceptance criteria C1-C8 require direct physical-stream proof, direct runtime/sole-receiver proof, parser-consumed-versus-residual credit proof, activity/fragmentation/quiesce/lifetime proof, lifecycle/failure/repeated-fixture ownership proof, explicit A001/A002 CONFIG boundary, no A002 or inherited receive-poison repair, and a truthful immutable evidence log.

## CONFIG authority disposition

Resolved the long-standing planning ambiguity around `pstvnc_transport_session_config_t` without inventing product policy.

A001 owns the Transport/RFB consumer boundary and therefore keeps a complete explicit caller-supplied Transport session value. A002 audit authority owns CONFIG decode/completeness/structural validation and the production profile mechanism. The RFB queue/credit fields, receiver-thread stack/priority, and maximum DATA payload are classified by A002 as qualified-profile/tuning values rather than universal application defaults.

Therefore the absence of the future production producer is not an A001 reconstruction blocker. `pstvnc_app_run()` may remain fail-closed while A001 is validated parametrically; A002 later reconstructs the configuration/profile producer and supplies narrow owner-specific immutable values. No numeric default was chosen in this shift.

## Foreman integration/chore reconciliation

Repository inspection established the following current status:

- `src/transport/SYMBOLS.md` has already been expanded to definition-level `COVERAGE=COMPLETE` by `d21ccfe6f6e4015cdea1092ebd0ebd16b1dbd81a`; canonical Universal-Ctags long/strict proof remains PENDING_LOCAL.
- `src/rfb/SYMBOLS.md` is materially stale despite declaring `COVERAGE=COMPLETE`: it still contains rows for deleted `src/rfb/rfb_io.h`, old `socket_fd` helpers/fields/signatures, and lacks the current `src/rfb/bridge.c/.h` symbol family.
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` still omits `src/transport`, so generated portal synchronization remains open.
- `docs/development/source-topology.md` and `scripts/continuity-check.sh` already include `src/transport`; no duplicate topology adoption is required.
- `tests/Makefile` already registers the existing Transport protocol/RFB-channel/bridge and RFB-bridge fixtures. The new direct physical-stream/runtime fixtures belong to Reconstruction test source first and should be canonically registered by the Foreman after they exist.
- live Transport/RFB bridge clean-build topology is already wired; executable PS2DEV/build/reproducibility evidence remains PENDING_LOCAL.

No unsafe whole-file rewrite of the large stale RFB dictionary was attempted from incomplete/truncated connector rendering. This is not treated as a product-behavior blocker: the fresh Reconstruction packet can proceed independently, while canonical RFB dictionary/portal regeneration remains a Foreman integration chore for a surface that can derive/verify the complete generated result.

## Validation boundary

V003 remains formally OPEN / `WAIT_FOR_COHERENT_TRANCHE` under Validation ownership.

V004 remains formally OPEN pending RFB dictionary reconciliation, generated portal synchronization, and canonical long/complete/strict dictionary/topology evidence.

V005 remains formally OPEN pending independent canonical executable evidence, although Validation's immutable static review supports the reconstructed fatal-convergence source shape.

No `VALIDATION_READY`, full Validation PASS, PS2DEV build PASS, reproducibility/PT_LOAD qualification, or hardware PASS is claimed.

PENDING_LOCAL=canonical host unit suite; scripts/check.sh; Universal-Ctags long/complete/strict dictionary validation; clean PS2DEV compile/link; reproducibility/exact ELF/PT_LOAD evidence
HARDWARE_PENDING=YES

## Exact next pickup

INTERACTIVE RECONSTRUCTION B: consume Foreman state revision 0003 and current HEAD, then execute packet C1-C8 as one sustained A001 behavior-closure round. Reserve tool capacity for the required immutable round log. Do not begin A002 and do not substitute V004 dictionary/portal chores for the assigned behavior proof.

FOREMAN after that result: register any new physical/runtime fixtures canonically, finish/reconcile the stale RFB dictionary and generated portal through a complete verifiable source surface, obtain available canonical machine evidence, and determine whether any reconstruction-owned A001 behavior remains before independent Validation handoff.

VALIDATION: independently consume new canonical behavior execution and V004 evidence when available; findings/state remain Validation-owned.

Scheduled A/B/Foreman seats remain operator-paused unless the operator later chooses to return to automated reconstruction.
