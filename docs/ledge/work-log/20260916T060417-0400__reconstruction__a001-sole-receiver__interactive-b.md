# Ledge Reconstruction Shift — Interactive Reconstruction B substitute

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T06:04:17-04:00
COMPLETED_AT=2026-09-16T06:25:14-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive-b
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2c67e0b996b08a7535fe1340cde949a432794011
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
FOREMAN_GOAL_RESULT=MET

## Objective and authority consumed

Executed the fresh `a001-sole-receiver` C1-C8 behavior-closure packet assigned by `LEDGE_FOREMAN_STATE.md` revision `0003` as the interactive Reconstruction B substitute. The round stayed strictly inside A001: it added behavior-specific host fixture source around the actual Transport physical-stream/runtime implementations, corrected only test-harness seams needed to make those fixtures deterministic/host-compatible, re-read all eight A001 audit behaviors, and performed the authorized A001-only dead-seam census. No A002 audio/CONFIG implementation, A003+, V004 dictionary/portal work, or canonical Makefile registration was performed.

Authority consumed at start included branch HEAD `2c67e0b996b08a7535fe1340cde949a432794011`, Foreman revision `0003`, global Continuity state revision `0022`, Validation state revision `0005`, Validation findings revision `0004`, A001 audit `LEDGE_AUDIT_A001_TRANSPORT_RFB.md`, A002 CONFIG/audio audit revision `0001`, and this worker's immediately previous immutable handoff `20260915T233233-0400__reconstruction__a001-sole-receiver__interactive-b.md`.

During this round Validation independently advanced with commit `28bf86b19be9c97ec48bb0f913f340a33ea08196`, reviewing the new fixture source through `bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd`. That review opened no new product defect, recorded the new stop-retry/reclaim assertions as materially stronger C4/C5/V005 evidence, and retained V003/V004/V005 formal OPEN pending canonical integration/execution. The concurrent Validation commit is authority consumed here, not Reconstruction work attributed to this worker.

## Exact Reconstruction changes

This round changed only host behavior-test source/support. No product source was changed because the deterministic fixture construction and static review exposed no A001 product defect requiring correction.

Worker commits:

- `4e673f0f6606faa1a5330bc3d66cb063798e7508` — `test(a001): add deterministic host kernel seam`; added `tests/unit/transport_host_stubs/kernel.h` with the narrow EE semaphore/thread declarations needed by the direct Transport fixtures.
- `db8936e4e50d520f8916653fba06fe8b2b2eff7b` — `test(a001): add host PS2IP include seam`; added `tests/unit/transport_host_stubs/ps2ip.h` so current physical-stream source can compile against host socket declarations rather than PS2SDK headers.
- `7f30d457349bf75030910b0c28095ce9182a56c5` — `test(a001): exercise real physical PSTV stream`; added the direct `tests/unit/transport_physical_stream_test.c` fixture.
- `2afc25b9367dfeea896462ddce48340f1d312493` — `test(a001): exercise real Transport runtime behavior`; added the direct `tests/unit/transport_runtime_test.c` fixture.
- `b6f48b7a4f0f782e7b93a0df199865a3094f8b0e` — `test(a001): make physical fixture self-contained`; made the physical fixture explicitly consume the host kernel seam.
- `bce128b5b5e30bbb49a527bdfbe02c5c8ec371dd` — `test(a001): harden runtime lifecycle proof`; added failed-stop retry, pre-reclaim release-failure ownership, fresh-session reset, and self-contained host-kernel coverage.
- `3f45d95724c5294c587731eef15005bba659e42f` — `test(a001): isolate PS2 thread ABI pedantic warning`; documented and suppresses only the host `-Wpedantic` diagnostic inherent in PS2SDK's `ee_thread_t.func` `void *` entry representation. `-Wall`, `-Wextra`, and `-Werror` remain active; product source is untouched.

Net worker source additions from the round are `tests/unit/transport_physical_stream_test.c`, `tests/unit/transport_runtime_test.c`, and the two `tests/unit/transport_host_stubs/` headers.

## C1-C8 result

### C1 — MET

`transport_physical_stream_test.c` directly links the current `src/transport/physical_stream.c` + `protocol.c` behavior rather than a bridge stub. It covers successful adopt, pre-adoption semaphore failure without descriptor ownership, complete header+payload send, short/chunked exact send, serialized send-lock ownership, send failure without sequence advancement, complete receive, short/chunked receive, wrong receive sequence, corrupt PSTV protocol header, insufficient payload capacity, receive failure without progression, `shutdown(SHUT_RDWR)` without descriptor close, and final Transport-owned close/release/reset.

The direct source therefore covers framing, direction-local sequence authority, exact-I/O progression, fail-closed rejection, serialized-send ownership, and the V005 physical receive interrupt seam.

### C2 — MET

`transport_runtime_test.c` directly links the actual `src/transport/runtime.c` state machine, real `src/transport/rfb_channel.c`, and real `src/transport/quiesce.c`, with only PS2 kernel/physical-stream mechanisms controlled by the fixture. The sole receiver receives complete fake PSTV frames through the physical-stream mechanism seam; the test records maximum concurrent receive calls and receiver thread identity so a second physical receive owner cannot silently satisfy the assertions.

The fixture covers valid DATA/RFB dispatch plus wrong-kind and wrong-channel fail-closed convergence. It also verifies receiver completion, failed state, and empty/no-corrupt logical RFB state on invalid input.

### C3 — MET

The runtime fixture proves parser-consumption credit accumulation, configured batch return, flush-on-empty, real logical-RFB queue consumption, outbound logical-RFB fragmentation at `max_data_payload`, and send-failure propagation. Terminal residual discard is exercised after COMMIT and is asserted not to emit return credit or erase already earned parser-consumption credit.

Producer activity is tied to the runtime's activity-sequence/rendezvous path rather than a diagnostic counter or timer. Existing `transport_rfb_channel_test.c` independently covers commit generation, wraparound, exact-read atomicity, and residual-discard generation distinction.

### C4 — MET

The runtime fixture executes the Transport state sequence REQUEST -> externally initiated BOUNDARY -> residual DATA -> COMMIT -> exact residual snapshot/discard -> COMPLETE. Before test-only cleanup it asserts that this finite RFB quiesce path has not set `stop_requested`, has not invoked physical shutdown, and has not completed the receiver, keeping it distinct from application-local fatal abort.

Existing `rfb_async_framing_test.c` provides the RFB-owned half of the same invariant: quiesce is completed only at a complete-server-message safe boundary and the next server-message byte remains unread. The new runtime lifetime assertions additionally require receiver-done publication before thread termination/delete and physical release, including the post-done scheduling window where the waiter may observe a not-yet-dormant receiver.

### C5 — MET

Representative deterministic failures now include invalid config, semaphore creation failure, physical-adopt failure, initial-credit send failure, `CreateThread` failure, `StartThread` failure, physical shutdown failure, live-receiver release refusal, and pre-reclaim `ReferThreadStatus` failure.

A failed physical shutdown sets stop intent but does not prove completion or authorize reclaim. A second stop call must reissue the Transport-owned physical interrupt; `stop_requested` alone is not treated as permission to wait/reclaim. A failed thread-status proof leaves runtime/session resources initialized and owned for retry. Reusing the runtime object after a clean release asserts fresh stop/done/failed/quiesce state and new physical adoption/release rather than stale current-session authority.

No timeout, second receiver, unsafe adopted-descriptor close, or diagnostic progress counter was introduced as correctness authority.

### C6 — MET

The eight-field `pstvnc_transport_session_config_t` remains an explicit caller-supplied immutable value boundary. The tests use conspicuous test-only values only to exercise behavior; no production numeric defaults were introduced. Foreman revision `0003` and `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` assign production profile/CONFIG decode/completeness/validation and supply of the Transport subprofile to A002. The missing production producer is therefore recorded as later A002 work, not an A001 blocker and not “solved” in this round.

### C7 — MET

No A002 CONFIG/audio implementation, no A003+ behavior, and no repair of the separately inherited H1 next-session receive-poison defect was introduced. The fresh-session fixture proves only clean A001 runtime object/session authority after normal reconstructed release; it does not reinterpret that test as authority to alter the inherited H1 defect boundary.

Current A001 runtime dispatches the reconstructed RFB logical channel and fails unexpected/non-RFB frames closed. Future AUDIO/MPEG logical consumers remain later-tranche work; this round preserves the sole physical receiver/owner architecture rather than implementing those consumers early.

### C8 — MET

This immutable record distinguishes committed fixture source from actual supplemental execution and from canonical PENDING_LOCAL evidence. No shadow run is called a canonical unit-suite PASS.

## A001 eight-behavior closure map

1. **One physical PSTV stream; serialized sends; sole receive owner.** Source: `src/transport/physical_stream.c/.h`, `src/transport/runtime.c/.h`. Tests: new `transport_physical_stream_test.c`, new `transport_runtime_test.c`, existing `transport_protocol_test.c`. Current A001 dispatch target is logical RFB; later AUDIO/MPEG consumers attach under their own tranches without gaining physical receive authority.
2. **Sequence authority belongs to Transport and advances only on complete physical frames.** Source: `physical_stream.c` direction-local `next_send_sequence` / `expected_receive_sequence`. Tests: new physical-stream complete/failure/wrong-sequence cases plus `transport_protocol_test.c` header/framing checks.
3. **Only Transport receiver reads physical bytes; complete RFB payloads become logical bytes; RFB parser reads the logical stream.** Source: `runtime.c` receiver/accept path, `rfb_channel.c`, `src/transport/bridge.c`, `src/rfb/bridge.c`, `src/rfb/rfb_session.c`. Tests: new runtime sole-receiver fixture, `transport_rfb_channel_test.c`, `rfb_bridge_test.c`, RFB session/framing tests.
4. **RFB credit represents parser consumption, not terminal discard.** Source: `runtime.c` consumed-credit path plus `quiesce.c`/`rfb_channel.c` residual-discard path. Tests: new runtime batch/flush/residual assertions and existing `transport_rfb_channel_test.c` residual-generation test.
5. **RFB availability/activity is producer-driven rather than blind timer polling.** Source: `runtime.c` `activity_sequence`, armed activity event, snapshot/wait pair. Tests: new runtime activity/dispatch fixture plus existing channel generation tests. Canonical execution of the full new runtime fixture remains pending registration.
6. **Finite RFB quiesce is ordered and separate from fatal abort; complete-message boundary selection remains outside Transport.** Source: `runtime.c`, `quiesce.c`, Transport/RFB bridges, `rfb_session.c`. Tests: new runtime quiesce/lifetime case, `rfb_bridge_test.c`, `rfb_async_framing_test.c` complete-message boundary cases.
7. **RFB parser completes protocol messages and never benignly yields mid-message.** Source: `src/rfb/rfb_session.c`. Tests: `rfb_async_framing_test.c` truncated CutText/ColorMap, unsupported-message, Bell boundary, no-mid-message-yield, and boundary-quiesce cases plus existing session/initial-frame tests.
8. **Application flow/presentation policy remains above parser/framebuffer authority.** Source: `src/app.c` services semantic input, advances RFB receive/parser state, presents only when the authoritative framebuffer is dirty/local UI requires presentation, and issues the next incremental update independently of whether the prior update required representation. Tests: `app_test.c`, including `test_clean_update_does_not_represent`, invalid-live-frame-before-presentation, idle receive, input publication, and lifecycle cases.

No remaining **A001-owned product-behavior obligation** was identified by this packet. Later AUDIO/MPEG consumers and production CONFIG/profile production are explicitly later-tranche responsibilities; V004 registration/dictionary/portal work is Foreman-owned evidence integration.

## Executed evidence versus PENDING_LOCAL

Actually executed on this interactive surface:

1. Supplemental physical-stream shadow assembled from the exact connector-read current physical-stream/protocol logic and deterministic local kernel/socket seam:
   `cc -std=c99 -Wall -Wextra -Werror -I/mnt/data/a001_shadow/stubs -I/mnt/data/a001_shadow /mnt/data/a001_shadow/shadow.c /mnt/data/a001_shadow/transport/physical_stream.c /mnt/data/a001_shadow/transport/protocol.c -o /mnt/data/a001_shadow/shadow`
   Result: `physical_stream_shadow: PASS`.
2. Supplemental runtime shadow assembled from the exact connector-read current `runtime.c`, `quiesce.c`, `rfb_channel.c`, and `protocol.c` logic plus deterministic local mechanism stubs:
   `cc -std=c99 -Wall -Wextra -Werror -I/mnt/data/a001_runtime_shadow/stubs -I/mnt/data/a001_runtime_shadow -pthread /mnt/data/a001_runtime_shadow/shadow.c /mnt/data/a001_runtime_shadow/transport/runtime.c /mnt/data/a001_runtime_shadow/transport/quiesce.c /mnt/data/a001_runtime_shadow/transport/rfb_channel.c /mnt/data/a001_runtime_shadow/transport/protocol.c -o /mnt/data/a001_runtime_shadow/shadow`
   Result: `runtime_shadow: PASS`. An earlier compile attempt failed only because the ad-hoc local shadow stub had omitted `THS_RUNNING`; adding that missing test-stub constant allowed the unchanged current product logic to compile/run.
3. Supplemental exact-logic RFB-channel execution under the repository warning profile:
   `cc -O2 -std=c99 -Wall -Wextra -Werror -pedantic -I/mnt/data/a001_channel_exact /mnt/data/a001_channel_exact/test.c /mnt/data/a001_channel_exact/rfb_channel.c -o /mnt/data/a001_channel_exact/test`
   Result: `transport_rfb_channel_test: PASS`.
4. Host compiler probe confirmed that PS2SDK's required `ee_thread_t.func` function-pointer/object-pointer representation is rejected by `-pedantic -Werror`, and that the test-only kernel shim's narrowly scoped `-Wpedantic` suppression permits that ABI representation while retaining the other warning classes as errors.

These are supplemental executions, not the canonical repository suite. The full committed `tests/unit/transport_physical_stream_test.c` and `tests/unit/transport_runtime_test.c` are not yet registered in `tests/Makefile`; Foreman revision `0003` reserves that registration to Foreman. The current Makefile was reread and does already append the pre-existing `transport_protocol_test`, `transport_rfb_channel_test`, `transport_bridge_test`, and `rfb_bridge_test` to `unit`; the two new direct fixtures are the registration delta.

PENDING_LOCAL:

- canonical registration and execution of the two new direct fixtures;
- canonical `make -C tests unit` including those fixtures;
- `scripts/check.sh`;
- strict/long Universal-Ctags dictionary evidence and generated portal synchronization (Foreman/V004 lane);
- clean PS2DEV compile/link of the integrated tranche;
- reproducibility/exact ELF/PT_LOAD evidence.

HARDWARE_PENDING=YES. No PS2 hardware PASS is claimed.

## Validation and findings disposition

Concurrent Validation commit `28bf86b19be9c97ec48bb0f913f340a33ea08196` independently reviewed the new source through `bce128b5...`, opened no product-source defect, and explicitly handed canonical registration/execution to Foreman. V005 therefore remains formally OPEN despite stronger source + supplemental execution evidence. V004 remains formally OPEN for Foreman-owned dictionary/portal/test-registration/canonical evidence. V003 remains formally OPEN / `WAIT_FOR_COHERENT_TRANCHE` until Foreman integrates evidence and Validation independently promotes/revises the tranche.

A point-in-time integration note for Foreman/Validation: the concurrent Validation log added by `28bf86b...` uses path `docs/ledge/work-log/2026-09-16T06-20-13-04-00__validation__a001-sole-receiver__validation.md`, which does not visually match work-log contract revision `0001`'s punctuation-free timestamp filename grammar. Reconstruction did not modify another worker's immutable record; Foreman/Validation should let `work-log-check.py` determine whether this is a canonical-check issue and record any correction in a later immutable entry rather than rewriting history.

## A001-only stretch dead-seam census

After C1-C8 were met, current `src/rfb`, `src/platform/ps2_network.c`, `src/rfb/bridge.c`, and `src/transport/bridge.c` were reread. The deleted `rfb_io.h` socket seam has not reappeared; RFB crosses only the logical bridge; platform networking owns connection creation/close only before Transport adoption; Transport bridge owns active session/logical-RFB lifecycle. No additional behaviorally dead A001 adapter was proven safe to remove, so no speculative deletion was made.

## Exact next pickup

Foreman owns the next action under revision `0003`: consume this `FOREMAN_GOAL_RESULT=MET` C1-C8 handoff, register `transport_physical_stream_test.c` and `transport_runtime_test.c` canonically (including the host stub include path and pthread requirement), finish V004 RFB dictionary + generated portal work, and obtain exact canonical machine evidence. The host kernel shim already contains the narrow PS2 thread-ABI pedantic accommodation needed by the runtime target.

Validation then independently consumes the canonically executed behavior fixtures and Foreman evidence to revision-chain V003/V004/V005 if warranted. Reconstruction has no newly identified A001 product correction from this packet. Do not begin A002 until the Foreman/Validation handoff boundary authorizes it.