# Ledge Reconstruction Shift — Interactive Reconstruction B continuation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T23:32:33-04:00
COMPLETED_AT=2026-09-16T03:48:16-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive-b
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=bbeefe1fed187a21d9c7721a81018978ce7108d8
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=YES

## Objective and authority consumed

This record closes the interactive Reconstruction B continuation that began at `2026-09-15T23:32:33-04:00` and was left without its required immutable shift record when the prior execution surface closed. The shift began while Foreman state revision `0002` and its A001 B packet were the operative assignment being continued from the immediately preceding interactive-B work. It remained on `a001-sole-receiver`, did not begin A002, and preserved the A001 audit requirements: one physical PSTV owner/sole receiver, logical RFB parsing outside Transport, parser consumption distinct from terminal residual discard, receiver completion before reclaim, and no guessed Transport CONFIG defaults.

During the long-running continuation, Validation and Continuity advanced independently on the same branch. Before this record was created, the worker re-read current branch authority at `95f3c6eff7e021282b553d77bcc744e88d80b361` and consumed Continuity global state revision `0020` through the immutable `20260916T033256-0400__continuity__global-reconciliation__continuity.md` record. That newer authority explicitly classifies Foreman state revision `0002` as stale and says A/B must not execute another behavior packet or substitute Foreman chores until a fresh Foreman packet is published. No newer Foreman packet exists at this close. This shift therefore self-pauses rather than extending stale authority.

No `FOREMAN_GOAL_RESULT` is recorded here because the currently authoritative Continuity baton explicitly forbids reporting one against an invented or stale packet. The criterion-level results from the earlier B packet remain historical evidence in the prior interactive-B record; this continuation records the actual additional A001 work and its present evidence boundary.

## Product behavior reconstructed

The major behavior work in this continuation was deterministic application-local fatal Transport convergence for Validation finding V005 and completion of the application ownership fixture.

The reconstructed path now gives Transport, not application/RFB, the mechanism that interrupts its owned blocking physical receive. An application-local post-adoption fatal path requests Transport abort; Transport publishes stop intent, interrupts its own PSTV socket receive, waits for the sole receiver's quiescent completion, proves the receiver thread dormant before receiver-visible resources are reclaimed, and only then releases the adopted physical/session resources. The application never directly closes a successfully adopted descriptor. Pre-adoption failure leaves caller close ownership intact. Server-driven finite RFB REQUEST -> BOUNDARY -> COMMIT -> residual discard -> COMPLETE remains a separate RFB-safe-boundary process and is not repurposed as the application fatal-abort primitive.

Relevant reconstruction commits include:

- `eb8ea2be140ae6c294fe4450a386b296e3442931` — `reconstruct(a001): add Transport-owned receive interrupt`;
- `6e210418571a1890ac78092281300a0fd36920af` — `reconstruct(a001): interrupt owned PSTV receive on fatal stop`;
- `e917079725fe5c92c3e18c92f11f7f93bca10943` — `test(a001): migrate application Transport lifecycle fixture`; the canonical `tests/unit/app_test.c` was moved to the configured application entry, descriptor-free RFB start, Transport open/abort stubs, explicit CONFIG forwarding checks, caller-owned pre-adoption close, post-adoption no-double-close, abort ordering, and no-argument fail-closed behavior;
- `0178e0543a13fac0216c54617de43cd924d03d51` — `test(a001): consolidate app Transport lifecycle coverage`; removed the weaker standalone `app_transport_lifecycle_test.c` after its useful coverage was incorporated into `app_test.c`;
- `98c243d4f1072ec3034d3474fbed3c5d224fe809` — `reconstruct(a001): prove receiver thread dormant before reclaim`; replaced bounded-delay-style lifetime assumptions with an explicit quiescent receiver-done / thread-dormancy fence and made stop retries reissue the Transport-owned receive interrupt rather than treating a prior stop flag as sufficient;
- `22b71c74f004a713be803be78b83b53ffb68e80f` — `reconstruct(a001): keep session active until Transport reclaim succeeds`;
- `dddf72eac52381cbe3c6ff55e4960b6229af91d1` — `reconstruct(a001): preserve retryable Transport ownership on release failure`; release semantics now distinguish unreclaimed Transport ownership from cleanup failure after authority has already been retired.

The same continuation also reconciled the physical connection seam to the PSTV transport endpoint/call identity rather than the obsolete direct-VNC identity. `src/platform/ps2_network.c/.h` and the application call chain use the H1-authoritative PSTV endpoint semantics (`192.168.50.1:5902`) without creating a second RFB socket owner.

No timeout, second receiver, application close of an adopted descriptor, hidden receive-poison workaround, or fabricated queue/credit/thread/payload default was introduced.

## Build, test, topology, and dead-seam integration performed before the authority stop

The continuation also made the already-reconstructed A001 source part of the clean build/test topology and removed a dead direct-RFB socket declaration seam. These integration edits were performed before the later Continuity revision `0020` clarified that remaining V004 chores belong to the Foreman/integration lane.

Exact commits include:

- `ed54ceb94fff39082e5f77950f3091c1ece47948` — `build(a001): wire Transport into clean linked topology`; added the active input/UI/Transport objects and dependencies to `mk/issue7-clean.mk` and removed obsolete direct-RFB header dependencies;
- `18be234c5d370bbf1ebc3994b1ff8e8f0c8acf18` — `build(a001): compile live Transport topology`; added the active Transport/input/UI source set and Transport include path to `scripts/check-clean-ps2-compile.sh`;
- `87b78d00f19f67054c2bd24994a9403834d7a384` — `build(a001): link RFB logical bridge`; corrected the linked object topology so `src/rfb/bridge.c` is present with `rfb_session.c`;
- `48fcd8175cf3ca6ddb5d842111b907b240c906f3` — `build(a001): compile RFB logical bridge`; included the RFB logical bridge in the strict clean PS2 compile source topology;
- `d9fa251e18e2e2796a4d98739fd1bf44ce0bcede` — `docs(a001): adopt Transport clean source domain`; registered `src/transport` in current source-topology policy;
- `a83470607bd3fd03e37bc5d5617f59d037dce75b` — `docs(a001): map Transport clean source ownership`; updated the living file/service map;
- `ee74145b9ccd4a7792e638ad9711172e428f2620` — `docs(a001): register Transport topology contract`; added `src/transport` to the current topology checker directory set;
- `4a1ebec4d818333c22458dc75fba4c3ea231a7f1` — added deterministic PSTV protocol/header framing-contract test source;
- `dbd36682b88e15a2f081ed3a498a2cf39551b73a` — registered the framing test and reconciled the migrated RFB fixture prerequisites with the logical bridge/Transport include topology;
- `f4410230a43178711936090ab5f023c5f3c693b9` — `reconstruct(a001): retire direct RFB socket seam`; deleted the now-dead `src/rfb/rfb_io.h` declaration-only socket seam;
- `d21ccfe6f6e4015cdea1092ebd0ebd16b1dbd81a` — `docs(a001): complete Transport symbol inventory`; expanded `src/transport/SYMBOLS.md` to definition-level coverage derived from the exact branch source with an independent compiler-AST inventory pass.

Concurrent Validation/Continuity commits interleaved with this worker's commits on the shared branch. They are current authority consumed by this shift but are not attributed here as Reconstruction work.

## Evidence and exact result boundary

Supplemental executable sanity evidence obtained during the continuation:

- a host-compiled shadow of the current `transport_bridge_test` against the current bridge API passed, including stop -> wait -> release / V005 lifecycle assertions;
- a host-compiled shadow of the new PSTV protocol/framing fixture passed under strict C99 warning settings and printed `transport protocol tests passed`.

These runs are supplemental evidence only. They were not the repository's canonical `make -C tests unit` execution and are not promoted to a canonical host-suite PASS.

Validation's immutable `20260916T002029-0400__validation__v005-fatal-teardown__validation.md` independently performed static review after the reconstruction and recorded that the V005 missing-cause source defect appears reconstructed: application fatal teardown calls `pstvnc_transport_session_abort()`, Transport causes its sole receiver to finish, receiver completion precedes reclamation, the application does not directly close the adopted descriptor, and finite RFB quiescence remains separate. Validation classified the V005 source shape provisionally PASS but left executable evidence PENDING_LOCAL. Later Validation reviews did not obtain the missing canonical executable evidence, so the formal finding register still says V005 OPEN.

PENDING_LOCAL remains:

- canonical `make -C tests unit` / full host fixture execution;
- `scripts/check.sh`;
- Universal-Ctags strict/long source-dictionary validation;
- clean PS2DEV compile and linked build;
- reproducibility/exact ELF identity and PT_LOAD comparison evidence.

HARDWARE_PENDING=YES. No PS2 hardware PASS is claimed by this shift.

## V004 / dictionary state

V004 is not closed by this shift. `src/transport/SYMBOLS.md` was materially expanded, but the canonical Universal-Ctags definition audit was unavailable on this execution surface. More importantly, `src/rfb/SYMBOLS.md` remains stale relative to the migrated logical bridge: it still carries entries for the deleted `rfb_io.h` / old socket-shaped session vocabulary and does not yet fully describe the current bridge/session definitions. `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` therefore also remains pending canonical regeneration/verification.

The worker does not mark either dictionary `COMPLETE` by inference and does not claim `scripts/check.sh` PASS. After Continuity revision `0020` was consumed, no further V004 integration edits were made because current authority assigns those chores to the fresh Foreman/integration path rather than to an A/B worker operating under a stale packet.

## Transport CONFIG authority gap

The eight-field `pstvnc_transport_session_config_t` boundary remains deliberately caller-supplied and explicit. Forensic H1 authority established that queue capacity, initial/batched credit, flush/return policy, receiver thread stack/priority, and maximum DATA payload were supplied dynamically per session by the Pi CONFIG exchange rather than being universal fixed defaults. Current clean source still lacks an adopted production producer for all eight values.

Accordingly the no-argument production `pstvnc_app_run()` remains fail-closed instead of manufacturing numeric policy. Tests use conspicuous test-only sentinels solely to prove forwarding/ownership. Live production startup remains blocked on recovering/adopting an explicit clean configuration authority; this is not worked around here.

## Current findings / disposition

- A001 remains `RECONSTRUCTING`; it is not `VALIDATION_READY`.
- V003 remains OPEN pending coherent A001 completion/evidence.
- V004 remains OPEN pending Foreman-owned dictionary/portal/topology/canonical-check completion.
- V005 remains formally OPEN pending independent executable evidence/finding-register revision, although independent Validation static review supports the reconstructed source resolution.
- A002-A006 were not started; this shift stayed on A001 only.
- no hardware qualification was performed.

## State/contract revisions and authority transition

Consumed work-log contract revision `0001`, A001 audit authority, the stale historical Foreman state revision `0002`, and later Continuity global state revision `0020`. Reconstruction state revision `0007` and Foreman state revision `0002` are now explicitly stale relative to the committed A001 source and later continuity movement.

The round's missing immutable record is being repaired here without inventing a second `STARTED_AT`: the original `2026-09-15T23:32:33-04:00` is preserved. This file is the one shift record for that continuation.

## Exact next pickup

SELF_PAUSED pending a fresh Foreman packet.

At the next Reconstruction B pickup:

1. re-read current branch, global state, newest Foreman state/packet, Validation findings/state, and newest immutable cross-lane logs;
2. execute only the fresh bounded A001 packet actually assigned to Reconstruction B; do not infer another packet from this log or from stale Foreman revision `0002`;
3. do not begin A002 while A001 remains reconstructing;
4. leave V004 dictionary/portal/canonical-check chores to the Foreman/integration lane unless a fresh packet explicitly assigns behavior-specific work touching those surfaces;
5. leave formal finding closure to Validation and hardware qualification to operator evidence;
6. preserve the unresolved production Transport CONFIG producer gap without guessing defaults.

Until that fresh assignment exists, there is no authorized additional A/B behavior work in this round.