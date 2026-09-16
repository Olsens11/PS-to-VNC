# Ledge Validation Shift — A001 sole receiver

ROLE_KEY=validation
WORKER_KEY=validation
WORK_ITEM_KEY=a001-sole-receiver
STARTED_AT=2026-09-16T06:20:13-04:00
COMPLETED_AT=2026-09-16T06:23:30-04:00
STATUS=EVIDENCE_ADVANCED_NOT_VALIDATION_READY
STARTING_AUTHORITY=bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd
ENDING_AUTHORITY_PRE_WRITE=bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd
BRANCH=ledge/h1-all-guns
RECONSTRUCTION_CONTRACT_REVISION=0005
GLOBAL_STATE_REVISION=0022
FOREMAN_STATE_REVISION=0003
VALIDATION_FINDINGS_REVISION=0004
TEMPORAL_CLASS=IMMUTABLE_SHIFT_RECORD
TEMPORAL_SEMANTICS=POINT_IN_TIME

## Point-in-time review

Current authority contains new validation-relevant A001 behavior fixtures after the prior validation shift. Commit b6f48b7a4f0f782e7b93a0df199865a3094f8b0e makes the physical-stream fixture self-contained against the host kernel stub, and current HEAD bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd hardens the runtime lifecycle fixture. The latter now explicitly checks failed physical interrupt does not authorize reclaim, stop retry reissues the owned interrupt, receiver completion precedes reclaim, and a pre-reclaim ReferThreadStatus failure leaves Transport ownership retryable.

These are validation-relevant improvements to the evidence shape for Foreman packet C4/C5 and V005, but they are test-source changes rather than an independently executed canonical PASS on this surface.

## Checks / results

- re-read branch authority before write: HEAD bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd;
- reviewed current Foreman revision 0003 and its C1-C8 A001 behavior packet;
- reviewed current validation findings revision 0004 and prior immutable V005 shift;
- reviewed commits b6f48b7a4f0f782e7b93a0df199865a3094f8b0e and bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd;
- static inspection result: new runtime assertions materially strengthen deterministic fatal-stop/retry/reclaim evidence and remain validation fixtures, not DUT behavior changes;
- canonical tests/Makefile inspection result: the existing unit target registers transport_protocol_test, transport_rfb_channel_test, transport_bridge_test, and rfb_bridge_test, but does not yet register transport_physical_stream_test or transport_runtime_test;
- canonical `make -C tests unit`: PENDING_LOCAL on this validation surface;
- `scripts/check.sh`: PENDING_LOCAL;
- strict/long dictionary/topology checks: PENDING_LOCAL;
- PS2DEV link/reproducibility/exact ELF/PT_LOAD: PENDING_LOCAL;
- PS2 hardware qualification: HARDWARE_PENDING.

## Findings / disposition

Findings opened: none.
Findings closed: none.

V005 remains formally OPEN. Its reconstructed source resolution remains supported by prior static review, and the new runtime fixture materially improves the intended executable proof, but Validation does not infer PASS without canonical execution/integration evidence.

V004 remains OPEN and Foreman-owned for integration/evidence chores. In addition to the already-recorded RFB dictionary/generated-portal work, the two new physical/runtime fixtures now require canonical Makefile registration before their results can contribute to the canonical unit-suite claim.

V003 remains OPEN / WAIT_FOR_COHERENT_TRANCHE. A001 is NOT_VALIDATION_READY at this point-in-time authority.

## Owner handoffs / blockers

Reconstruction A/B: no new product-source defect is identified by this shift. Preserve the tested lifecycle semantics; complete the assigned behavior packet and report exact executed versus pending evidence.

Foreman: register `tests/unit/transport_physical_stream_test.c` and `tests/unit/transport_runtime_test.c` in the canonical host unit suite, finish V004 dictionary/portal work, and surface exact canonical machine results. This is integration/evidence plumbing, not a Reconstruction product-behavior correction.

Validation: next pickup is the completed C1-C8 worker handoff plus Foreman canonical registration/execution evidence. Independently judge runtime/physical fixture results, then revision-chain V003/V004/V005 only if the complete evidence supports it.

No PS2 hardware success is claimed. Unknown external/Pi-local dirty work remains outside connector visibility and was neither overwritten nor declared absent.
