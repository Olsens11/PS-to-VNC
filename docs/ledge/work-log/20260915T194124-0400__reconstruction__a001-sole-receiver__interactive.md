# Interactive Reconstruction — A001 logical RFB runtime

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T19:41:24-04:00
COMPLETED_AT=2026-09-15T20:00:54-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=686780e89d5629ad13e18897d78e241b0b489266
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Continue the active A001 reconstruction as one sustained shift rather than treating each coherent source atom as an end-of-shift boundary. Consumed reconstruction contract revision 0002, immutable work-log contract revision 0001, global state revision 0010, audit state revision 0007/A001 disposition, reconstruction state revision 0005, current validation V003/V004 authority, clean architecture plus the ledge transport overlay, source naming/topology policy, and pinned forensic H1 authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The branch and reconstruction state were re-read between bounded repository writes. No competing committed reconstruction advance appeared during the shift. Unknown external Pi-local dirty work remained outside this GitHub-native mutation surface and was neither overwritten nor declared absent.

## Work performed

This shift continued through multiple safe checkpoints inside one A001 work item:

- added `src/transport/runtime.h` and `runtime.c` as the first higher Transport runtime above the already reconstructed physical stream and logical RFB storage;
- made that runtime own the CONFIG-sized RFB storage allocation, queue mutex, producer-driven activity event, receiver-completion event, receiver stack/configuration, credit state, and sole physical receiver thread;
- made the receiver runtime the sole higher caller of `pstvnc_transport_physical_stream_receive_frame()` and accept audit-authorized channel-1 DATA into Transport-owned storage without exposing the raw socket;
- reconstructed the protected activity-sequence plus armed-event rendezvous so logical RFB consumers can sleep until committed producer activity rather than blind polling;
- reconstructed initial/returned logical-RFB credit and outbound RFB fragmentation through the existing serialized physical send path;
- added explicit terminal residual discard that does not become parser-consumption credit;
- added `src/transport/quiesce.c` for the ordered Pi REQUEST -> PS2 BOUNDARY -> Pi COMMIT -> PS2 COMPLETE marker process while leaving the application/RFB owner responsible for deciding the complete-RFB-message safe boundary;
- kept resource reclamation fail-closed while a started receiver has not published completion;
- expanded `src/transport/SYMBOLS.md` for the principal runtime/channel/quiesce surface;
- added backend-independent `tests/unit/transport_rfb_channel_test.c`, wired it into the host `unit` aggregate, and synchronized the test Make/unit dictionaries;
- advanced reconstruction current state from revision 0005 to revision 0006.

## Forensic comparison / defects caught during the shift

Continuing past the first source checkpoint materially improved the result. Direct comparison with the pinned H1 logical-RFB implementation found two local reconstruction defects before handoff:

1. `rfb_channel.*` returns `0` for success, but the first `runtime.c` draft interpreted those results as booleans. The clean runtime was corrected before this shift ended.
2. the first exact-read runtime draft waited for the entire requested RFB parser span to be resident in the logical queue before consuming any bytes. That can deadlock when an exact parser read is larger than queue capacity: the queue fills, the Pi waits for credit, and the parser waits for bytes it cannot receive. The corrected implementation now consumes whatever committed bytes are available, returns credit only for those parser-consumed bytes, and waits on producer activity only when no current bytes remain. This matches the relevant H1 flow-control behavior without reproducing diagnostic scaffolding.

The historical post-session receive/mailbox poison defect is still explicitly unresolved. This shift did not add a generic timeout, nonblocking-recv workaround, or claim that explicit completion/quiesce source alone fixes the hardware/runtime symptom.

## Exact commits/files

- `7251e16359779e6a1c0535a93ea8a12f80c235f6` — add `src/transport/runtime.h` synchronized receiver-runtime contract.
- `7546f1c7b5e85a46f033f3acb9066d52e9ddff69` — retain receiver thread stack/priority configuration in runtime state.
- `b1f250cc62aa5207af13e04bedbd521df3c6f476` — add initial `src/transport/runtime.c` sole receiver/RFB runtime.
- `083d40bbce47d31609cf676a3377920962c54918` — correct logical-channel success-result interpretation and stack-size validation.
- `ad3462772756a7980e8caacba29371ec6a00ecd8` — define terminal logical-RFB residual discard.
- `4058a9bcb0d2fcc906a843c1faf5511b55f91159` — implement exact terminal residual discard without producer activity/credit.
- `a746f5dd8f31ffce375fe91c4d60d1c67c397af8` — define explicit ordered RFB quiesce operations.
- `bd26266ce3aa003ef1ce85f7adbb374da48766d6` — add `src/transport/quiesce.c` ordered REQUEST/BOUNDARY/COMMIT/COMPLETE implementation.
- `ef9b5b617ca20074de1275b376b707c507f45f9d` — expose incremental logical-RFB read primitive.
- `c2f59c87bc00568c58811f9bbf1681ff5eb839b6` — implement circular incremental parser consumption.
- `e8d768ed75b0d024fde26a06086f59226da35b7e` — change runtime exact reads to stream committed bytes and return parser-earned credit incrementally.
- `ee81bb0d190840f68c629d9f6fef6b0b12cb350c` — index principal runtime/channel/quiesce symbols.
- `b2860b2123879e1a347766029d1e2c6c27b283cd` — add logical-RFB channel host regression test.
- `a7f2eec2fd9cac98c04c3b31e6c3d4d8ec093bf6` — wire transport channel test into `tests/Makefile` unit aggregate.
- `99320e79368b6c98f0fdb58ab35f00a0925a715d` — advance `LEDGE_RECONSTRUCTION_STATE` to revision 0006.
- `1d600d1098fb838e1f583fa28f51eb7d96566a69` — synchronize test Make-target dictionary.
- `6e6441e825114987c39f8fdd0816801ea2268a43` — index newly written transport channel unit-test symbols.

## Checks / evidence

PASS by repository/static inspection only:

- one reconstructed higher runtime is the sole caller of the physical receive primitive;
- physical socket authority remains inside Transport;
- channel-1 storage remains independent of RFB parser/application policy;
- activity wait is event/sequence driven rather than timer polling;
- terminal residual discard is explicitly distinct from parser consumption;
- outbound logical writes reuse physical-stream serialized send;
- test source covers capacity rejection, circular wraparound, incremental reads, short exact-read atomicity, activity generation, and exact residual discard;
- test Make surface and its local dictionary were synchronized in this shift.

PENDING_LOCAL / not claimed PASS:

- actual compilation of `runtime.*`, `quiesce.c`, and changed channel source against PS2DEV;
- execution of `transport_rfb_channel_test` and the full host unit suite;
- canonical `scripts/check.sh`;
- strict definition-level source-dictionary check and generated portal refresh; V004 remains OPEN and the generated product portal still omits `src/transport`;
- clean-product topology/build integration; inspection confirmed the ordinary canonical `scripts/build.sh` remains historical/B4A-oriented and is not evidence that this new source is linked;
- exact clean ELF/PT_LOAD identity/reproducibility evidence;
- any PS2 hardware execution or qualification.

A001 remains IN_PROGRESS and is not `VALIDATION_READY`.

## Known blockers / remaining work

There is no persistent source-design blocker to continued A001 reconstruction. The next real boundary is integration/lifecycle evidence rather than lack of work.

Remaining A001 work includes public Transport/RFB bridge and session lifecycle integration, coherent close/error convergence around the blocking sole receiver, build/topology/generated-dictionary integration, executable host/canonical checks, and exact build/PT_LOAD evidence. The provisional `src/transport/transport.h` public interface is not yet wired to `runtime.*`; do not manufacture default CONFIG values merely to make that interface compile.

V003 remains OPEN waiting for a coherent tranche. V004 remains OPEN despite substantial local indexing because generated portal and definition-level completeness have not been proven.

## State/contract revisions

Consumed: reconstruction contract 0002; work-log protocol 0001; global state 0010; audit state 0007; reconstruction state 0005; current validation V003/V004 authority.

Produced: reconstruction state revision 0006 and this immutable shift record.

## Exact next pickup

Continue `a001-sole-receiver` from reconstruction state 0006, `runtime.*`, `quiesce.c`, and `transport_rfb_channel_test.c`. On an execution-capable surface first compile/run the backend-independent transport channel host test and relevant canonical checks, then resolve any defects without weakening the audited semantics. Continue with the public Transport/RFB bridge and session lifecycle so RFB uses the logical stream without raw-socket authority and teardown cannot reclaim receiver-visible resources before explicit receiver completion. Keep complete-RFB-message safe-boundary policy outside Transport, keep the historical receiver-poison defect visible, and resolve V004 plus topology/build integration before any `VALIDATION_READY` handoff.

The shift stops here at a coherent handoff with Reconstruction B scheduled for 20:10, rather than beginning another overlapping mutation window.
