# Reconstruction corrective shift — A003 Pi RFB attachment quiesce R13 corrective A

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T20:27:58-04:00
COMPLETED_AT=2026-09-21T20:37:22-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e71c44c45904738b25b6c5596446db49d64b6f57
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only active Foreman corrective packet:

`A003-PI-RFB-ATTACHMENT-QUIESCE-R13-CORRECTIVE-A`

The corrective is deliberately narrow. It closes only the R13 completion
defects independently identified by the Foreman:

1. make `request_quiesce()` wake an otherwise-blocked sole Wire owner without
   relying on PS2/provider/network traffic;
2. keep the quiesce caller incapable of sending Wire bytes or advancing Wire
   sequence;
3. prove REQUEST -> BOUNDARY -> COMMIT -> COMPLETE from the real local wake;
4. retire the wake state with the attachment and give Session B fresh wake state;
5. remove the artificial post-request PS2 CREDIT from the quiesce proof;
6. remove premature Reconstruction self-acceptance wording;
7. emit exactly one truthful immutable Reconstruction log for this corrective
   shift.

This shift does not redesign the otherwise-provisional R13 provider attachment.

## Authority recovered at pickup

Truthful corrective-shift start:

`2026-09-21T20:27:58-04:00`

Live pickup authority:

`e71c44c45904738b25b6c5596446db49d64b6f57`

Current Foreman State assigned:

- `PACKET_ID=A003-PI-RFB-ATTACHMENT-QUIESCE-R13-CORRECTIVE-A`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- `FOREMAN_DECISION_BASE=44497ff08432ccee900d10bc6fff03c72a6ab1b3`.

The latest Foreman immutable log was:

`docs/ledge/work-log/20260921T202140-0400__foreman__a003-mpeg-generation__interactive.md`

It explicitly withheld R13 acceptance and required this corrective.

## Provisional R13 history already present at pickup

This corrective shift did **not** begin from the original R13 assigning base.

The repository already contained the prior provisional R13 attempt:

1. `6fad7700b23444094b75a14f6c72bcbefd0ab03f`
   — `feat(pi): add lazy RFB attachment quiesce`;
2. `17318f5ebfac043c93e8ad0a4f18d39b5d494afa`
   — `tooling(symbols): run deterministic dictionary reconciliation`;
3. `9f2b20dc51c2edd7f34b17189257c8867e33bd8a`
   — `docs(symbols): reconcile current clean definitions`;
4. `643a37492192faeeadaee2268f6c9840732ce0a4`
   — `docs(pi): record R13 attachment lifecycle`;
5. `44497ff08432ccee900d10bc6fff03c72a6ab1b3`
   — `test(pi): close unused R13 idle provider fixture`.

That earlier Reconstruction attempt had **no immutable Reconstruction shift
log**. The newest Reconstruction log before the Foreman review remained the R12
record.

This corrective log does not backfill fake timestamps for the prior attempt.
It records those provisional commits as repository history and uses only this
corrective shift's truthful STARTED_AT / COMPLETED_AT.

## Foreman-found blocking defect

The provisional R13 source changed:

`RUNNING -> REQUEST_PENDING`

inside `RfbAttachment.request_quiesce()`, but the sole
`WireConnectionOwner` could already be blocked indefinitely in `select()`
waiting only on:

- the physical Wire socket;
- provider read readiness;
- provider write readiness;
- an in-progress provider-connect socket.

A state change alone is not descriptor readiness.

Therefore a clean request on an otherwise idle session could remain pending
forever.

The old integration test masked this defect by doing:

`request_quiesce()`

and then sending an unrelated additional PS2 RFB CREDIT solely to make the
physical Wire socket readable.

That test proved marker ordering only after unrelated peer traffic happened.
It did not prove that the public quiesce seam could actually cause REQUEST to
leave the Pi.

## Corrective wake ownership

The correction adds one private wake pair to each `RfbAttachment`.

At attachment construction:

- one `socket.socketpair()` is created;
- both ends are nonblocking;
- the read endpoint is exposed only as a readiness identity for the sole Wire
  owner;
- the write endpoint remains private to the attachment request seam;
- an attachment-local `threading.RLock` protects the cross-thread request/wake
  state.

This pair is an RFB/session-local notification primitive only.

It carries no Wire frame, no provider payload, and no product RFB bytes.

It is not a second transport connection.

## Corrected request_quiesce() behavior

`request_quiesce()` now:

1. acquires the narrow attachment-local quiesce lock;
2. validates that the attachment is exactly `RUNNING`;
3. verifies that the Session-local wake writer is still live;
4. publishes `REQUEST_PENDING`;
5. sends one local byte `b"Q"` to the private wake writer;
6. returns.

The requester still does **not**:

- call `sendall()` on the physical Wire connection;
- call physical Wire `recv()`;
- encode a Wire frame;
- call `WireConnectionOwner._send_active_frame()`;
- read or modify `next_send_sequence`;
- become a second Wire reader/writer.

Repository-boundary host evidence explicitly checks that
`pi/rfb_attachment.py` contains no `sendall(`, no
`next_send_sequence`, and no `encode_channel_frame` ownership.

## Sole Wire owner wake handling

`WireConnectionOwner._wait_with_rfb_attachment()` now adds the current
attachment's wake reader to the same blocking `select()` readiness set.

There is no timeout argument and no polling sleep.

When the local wake reader becomes readable, the owner:

1. calls `attachment.acknowledge_quiesce_wake()`;
2. drains only the private notification bytes;
3. calls its existing `_flush_rfb_attachment_output()`;
4. sees `REQUEST_PENDING`;
5. serializes the exact zero-length channel-1 REQUEST through
   `_send_active_frame()`;
6. advances Wire sequence only in the existing Wire-owner send path;
7. calls `attachment.confirm_request_sent()`.

Thus the correction interrupts the blocking wait without transferring any
physical Wire authority to the caller.

## Synchronization boundary

The corrective adds only synchronization earned by the real cross-thread seam.

`threading.RLock` protects:

- the public REQUEST transition;
- the private wake descriptors;
- request-marker visibility/confirmation;
- failure/close interaction with wake retirement.

The remaining attachment/provider machinery stays under the existing single
Wire-owner execution context.

No callback bus, event registry, project-wide lifecycle manager, generic rider
notification system or timeout loop was introduced.

## Wake retirement / Session-B freshness

The wake pair is attachment-owned and is retired when attachment authority ends.

The corrective closes it on:

- RFB-local attachment failure;
- COMPLETE / attachment STOPPED;
- final Session attachment close/retirement.

Once retired:

- `quiesce_wake_reader` returns no live descriptor;
- old Session-A wake descriptors are closed;
- they cannot become readiness authority for a later Wire Session.

The existing Session-A / Session-B integration test was strengthened to prove:

- Session A begins with a live wake reader;
- final Session-A retirement closes that reader;
- Session A then exposes no wake reader;
- Session B constructs a different fresh wake reader;
- Session B starts with fresh attachment/Relay/credit/quiesce state.

## Independent idle REQUEST proof

Added focused integration test:

`test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic`

Test sequence:

1. establish Q4 Wire Session;
2. use one ordinary first RFB CREDIT only to perform the accepted R13 lazy
   provider attachment;
3. receive the finite initial reverse-capacity grant;
4. leave both Wire and provider sides otherwise idle;
5. call `request_quiesce()` from a separate caller thread;
6. send **no additional PS2 CREDIT**;
7. send **no provider bytes**;
8. close neither endpoint;
9. use no timeout poll in product source;
10. receive exactly one Pi->PS2 zero-length RFB DATA REQUEST;
11. prove REQUEST sequence is correct;
12. prove state advances to `WAIT_BOUNDARY`;
13. prove only one request marker was counted;
14. prove provider remains open;
15. prove no duplicate REQUEST appears;
16. prove no provider byte was emitted;
17. only after that independent wake proof, send BOUNDARY;
18. observe provider retirement before COMMIT;
19. receive COMMIT;
20. send COMPLETE;
21. observe attachment STOPPED, wake reader retired and containing Wire still
    alive.

This directly proves the missing Foreman criterion: clean quiesce intent causes
REQUEST to leave the Pi with no unrelated peer/provider traffic.

## Existing lifecycle proof corrected

The existing:

`test_request_boundary_drains_and_retires_before_commit_complete`

no longer sends the artificial post-request PS2 CREDIT.

Its peer sequence numbers were corrected accordingly.

It still proves:

`REQUEST -> BOUNDARY -> drain accepted provider writes -> provider close -> COMMIT -> COMPLETE`

and still verifies:

- provider->PS2 reads may continue before BOUNDARY;
- no provider read occurs after BOUNDARY;
- queued PS2->provider bytes drain before retirement;
- provider is closed before COMMIT;
- no premature replacement credit appears during retirement;
- COMMIT follows retirement;
- no ordinary post-COMMIT attachment traffic appears;
- COMPLETE stops only RFB;
- Wire remains alive.

The provider-write-failure quiesce test was also corrected to remove its
artificial CREDIT wake while preserving RFB-local failure containment.

## No timeout-poll workaround

The corrective does not change the owner to periodic polling.

Product source continues to use blocking readiness:

`select.select(read_wait, write_wait, exception_wait)`

with the added wake descriptor in `read_wait`.

No arbitrary fourth timeout argument was added.

No `time.sleep()` loop was added to `pi/wire_server.py`.

The wake edge itself is bounded and session-local.

## Premature acceptance wording corrected

Before this corrective, Architecture Overlay revision 0005 said it:

`adds the accepted R13 reconstruction mechanism`

before the Foreman had accepted R13.

Corrected wording now says revision 0005:

`records the provisional R13 reconstructed mechanism ... pending independent Foreman acceptance`

Foreman acceptance remains Foreman-owned.

The maintained Pi/architecture authority was also updated to describe the
corrected private wake ownership without claiming live/hardware qualification.

## Dictionary reconciliation

Corrective source added new Pi definitions/fields for the wake seam.

Strict dictionary validation therefore correctly reported 19 missing generated
Pi symbols on the first corrective source head.

No generated dictionary was manually edited.

Exact deterministic reconciliation trigger:

`7b7fd7e53dc5557deebb4aac288b7a8cd663ac10`

message:

`tooling(symbols): run deterministic dictionary reconciliation`

Automation produced:

`4718c854ccec3a550774163f70c084ec0d1c78d7`

`docs(symbols): reconcile current clean definitions`

Final portal authority:

- `pi` — COMPLETE — 435 definitions;
- `src/transport` — COMPLETE — 1247 definitions.

## Corrective commit chronology

Corrective pickup authority:

`e71c44c45904738b25b6c5596446db49d64b6f57`

Corrective commits:

1. `0ddf94a8db608361cfb1164bb286d06c5eaae3b6`
   — `fix(pi): add session-local RFB quiesce wake`;
2. `06ce1c2306eba4fd9860eff159f6d0f7c3bad468`
   — `fix(pi): wake sole Wire owner for RFB REQUEST`;
3. `27386ef4f31687bf514d9afd534be90730af0767`
   — `test(pi): prove independent RFB quiesce wake`;
4. `edd2812ab4019dd4be74218a67593cec8cf00a74`
   — `docs(ledge): remove premature R13 acceptance`;
5. `7b7fd7e53dc5557deebb4aac288b7a8cd663ac10`
   — deterministic dictionary reconciliation trigger;
6. `4718c854ccec3a550774163f70c084ec0d1c78d7`
   — automated generated dictionary reconciliation;
7. `dab77bfd77bf0f9ab6582504b47230dab19e67b1`
   — `docs(pi): explain session-local R13 wake`;
8. `1d85a5ea13b83b6030af31333edfb7103414fff4`
   — `docs(pi): document independent R13 quiesce wake`;
9. `534d2b88bf2e775e55dd865f616f6a7a5e538155`
   — `docs(ledge): record corrected R13 wake ownership`.

## Exact corrective changed paths

Compared with corrective pickup authority
`e71c44c45904738b25b6c5596446db49d64b6f57`, final pre-log authority changes
only:

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/pi/README.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `pi/rfb_attachment.py`;
- `pi/wire_server.py`;
- `tests/unit/pi_rfb_attachment_test.py`.

No `src/` path changed.

No `mk/` path changed.

No PS2 product source or PS2 build input changed.

## Final machine evidence

Final pre-log authority:

`534d2b88bf2e775e55dd865f616f6a7a5e538155`

Canonical workflow:

`35672554566`

Final workflow result:

`SUCCESS`

The first host attempt on this exact head hit the known unchanged
`transport_runtime_test` scheduling race:

- `transport_runtime_test.c:862`;
- `transport_runtime_test.c:864`;
- fake receive-call visibility preceded queue/activity publication.

R13 corrective changes no `src/` or `mk/` path.

Only the failed host job was rerun on the exact same commit.

Workflow attempt 2 passed all gates:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on the already-reconciled head.

Corrective host evidence includes:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- `test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic ... ok`;
- `test_request_boundary_drains_and_retires_before_commit_complete ... ok`;
- `test_session_b_constructs_fresh_attachment_state ... ok`;
- `test_quiesce_wake_has_no_second_wire_sender_or_timeout_polling ... ok`;
- all 12 current `pi_rfb_attachment_test.py` cases PASS;
- all 18 current Pi Wire/Relay tests PASS;
- all 8 R11 native-provider tests PASS;
- all 9 R12 internal-provider tests PASS.

Project/dictionary evidence includes:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS records=169 grandfathered=9 format_compat=2 stamp_compat=1`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

PS2 regression evidence:

- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These are unchanged-source regression checks only. This corrective authorizes
and claims no new PS2 PT_LOAD tranche.

## Original A003 R13 worker dispositions

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R13-C1 RFB_ATTACHMENT_PRODUCT_OWNER — MET

Maintained `pi/rfb_attachment.py` owns one session-scoped provider attachment
and its local lifecycle. The corrective does not move ownership elsewhere.

### A003-R13-C2 IDLE_WIRE_DOES_NOT_START_PROVIDER — MET

The existing idle-Wire integration proof remains green. Attachment construction
and Wire ACTIVE alone produce zero provider connect calls.

### A003-R13-C3 NONBLOCKING_LAZY_INTERNAL_CONNECT — MET

First valid nonzero RFB CREDIT remains the lazy edge. Provider connection uses
nonblocking `connect_ex()` plus readiness / `SO_ERROR`; a pending connect
does not block Wire EOF/session progress.

### A003-R13-C4 EXPLICIT_FINITE_FLOW_CONFIGURATION — MET

`RfbFlowConfig` still requires explicit finite provider-read credit limit,
provider-write capacity and max DATA payload. No installed-daemon tuning
defaults were introduced.

### A003-R13-C5 R10_RELAY_COMPOSED_ONLY_AFTER_CONNECT — MET

The R10 Relay is still created exactly once only after provider connection
success. Pending connection does not publish reverse capacity.

### A003-R13-C6 SOLE_WIRE_IO_OWNER_PRESERVED — MET

The attachment requester signals only a private socketpair. It has no physical
Wire send/recv/sequence authority. `WireConnectionOwner` remains the sole
physical Wire reader/writer and the sole serializer of REQUEST.

### A003-R13-C7 ORDERED_REQUEST_BOUNDARY_COMMIT_COMPLETE — MET

The corrected independent-wake proof emits REQUEST with no unrelated peer or
provider traffic, then proves exact:

`REQUEST -> BOUNDARY -> provider-write drain -> provider close -> COMMIT -> COMPLETE`.

The old artificial post-request PS2 CREDIT was removed.

### A003-R13-C8 PROVIDER_IO_RETIRED_BEFORE_COMMIT — MET

After BOUNDARY, new provider reads stop; accepted provider writes drain; provider
I/O is closed before COMMIT is serialized.

### A003-R13-C9 PROVIDER_FAILURE_RFB_LOCAL — MET

Connect failure, provider EOF/write failure and local lifecycle failure remain
contained to RFB attachment state in the accepted host cases. Wire remains alive
until its own EOF/error.

### A003-R13-C10 SESSION_SCOPED_COMPLETE_STOP_NONRESUME — MET

COMPLETE stops the attachment while Wire remains alive. Session A's wake
descriptors are closed; Session B constructs a distinct fresh attachment,
Relay/credit/quiesce state and wake pair.

### A003-R13-C11 DEFAULT_SERVICE_NO_UNVALIDATED_AUTO_ATTACH — MET

The installed Wire service still supplies no attachment factory and no flow
profile. No automatic provider connection or service composition was added.

### A003-R13-C12 CLEAN_EVIDENCE_AND_CLAIM_BOUNDARY — MET

The required immutable corrective log is emitted here, premature self-acceptance
wording is removed, strict repository gates are green and live/hardware claims
remain bounded.

## Corrective acceptance dispositions

### R13-CA1 QUIESCE_REQUEST_WAKE_INDEPENDENT_OF_PEER_TRAFFIC — MET

A clean request wakes the Wire owner through the private local socketpair and
causes REQUEST to appear without extra PS2 CREDIT, provider bytes, endpoint
closure or timeout polling.

### R13-CA2 SOLE_WIRE_SENDER_PRESERVED — MET

The caller writes only to the private wake socket. Only
`WireConnectionOwner._send_active_frame()` serializes REQUEST/Wire frames and
advances Wire sequence.

### R13-CA3 NO_TIMEOUT_POLLING_WORKAROUND — MET

The owner retains blocking `select()` readiness. The wake reader is an
additional descriptor; no arbitrary timeout or polling loop was added.

### R13-CA4 WAKE_STATE_SESSION_SCOPED_AND_RETIRED — MET

Each attachment creates its own wake pair. It closes on failure, STOPPED /
COMPLETE and final session close. Session B receives a fresh pair.

### R13-CA5 QUIESCE_TEST_HAS_NO_ARTIFICIAL_CREDIT_WAKE — MET

The artificial post-`request_quiesce()` PS2 CREDIT was removed from the
ordered quiesce integration proof and the provider-write-failure proof.
REQUEST is received directly from the real local wake path.

### R13-CA6 PREMATURE_ACCEPTANCE_WORDING_REMOVED — MET

Architecture Overlay revision 0005 now describes provisional reconstructed R13
source pending independent Foreman acceptance.

### R13-CA7 TRUTHFUL_IMMUTABLE_RECONSTRUCTION_LOG_EMITTED — MET

This is the one immutable Reconstruction record for the corrective shift. It
uses the actual corrective timestamps and explicitly records the earlier
unlogged provisional R13 history rather than inventing a backfilled log.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_R13_CORRECTIVE_REQUIRED_REPOSITORY_MACHINE_GATES
PROVISIONAL_R13_SOURCE_EXISTED_AT_PICKUP=YES
PRIOR_R13_RECONSTRUCTION_LOG_EXISTED_AT_PICKUP=NO
FOREMAN_FOUND_QUIESCE_WAKE_DEFECT=YES
SESSION_LOCAL_QUIESCE_WAKE_IMPLEMENTED=YES
QUIESCE_CALLER_SENDS_PHYSICAL_WIRE=NO
QUIESCE_CALLER_ADVANCES_WIRE_SEQUENCE=NO
SOLE_WIRE_SENDER_PRESERVED=YES
TIMEOUT_POLLING_WORKAROUND=NO
ARTIFICIAL_POST_REQUEST_PS2_CREDIT_IN_PROOF=NO
REQUEST_BOUNDARY_COMMIT_COMPLETE_HOST_PROOF=YES
WAKE_SESSION_A_TO_SESSION_B_REUSE=NO
DEFAULT_WIRE_SERVICE_AUTO_ATTACH=NO
SRC_CHANGED=NO
MK_CHANGED=NO
NEW_PS2_PT_LOAD_CLAIM=NO
CONFIG_FRAME_PRODUCT_DELIVERY=NOT_IMPLEMENTED
SHARED_RFB_ON_OFF_FLOW_PROFILE_OWNER=NOT_IMPLEMENTED
AUTOMATIC_RFB_RECONNECT_RETRY_BACKOFF=NOT_IMPLEMENTED
PROVIDER_FAILURE_TO_PS2_RFB_RESTART_POLICY=NOT_IMPLEMENTED
SECOND_RFB_INSTANCE_IN_SAME_WIRE_SESSION=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_RESTART_POLICY=NOT_IMPLEMENTED
RFB_PRESENTATION_INPUT_CHANGES=NOT_IMPLEMENTED
LIVE_PI_STAGING=NOT_RUN_NOT_CLAIMED
LIVE_INTERNAL_PROVIDER_ACTIVATION=NOT_RUN_NOT_CLAIMED
PHYSICAL_PS2_PI_RFB_ATTACHMENT_QUIESCE=NOT_RUN_NOT_CLAIMED
PI_AUDIO_WIRE_RELAY=NOT_IMPLEMENTED
PI_MPEG_DATA_CONTROL_PRODUCER=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`534d2b88bf2e775e55dd865f616f6a7a5e538155`

plus this immutable Reconstruction corrective log commit.

Foreman should independently verify:

- the prior provisional R13 attempt really had no immutable Reconstruction log;
- the Foreman-found wake defect is repaired by a per-attachment local
  notification primitive;
- `request_quiesce()` still cannot send Wire or advance sequence;
- `WireConnectionOwner` alone drains wake and serializes REQUEST;
- clean idle request emits exactly one REQUEST without peer/provider traffic;
- no timeout polling is used;
- exact REQUEST -> BOUNDARY -> drain -> provider close -> COMMIT -> COMPLETE;
- no provider read after BOUNDARY;
- provider retirement precedes COMMIT;
- COMPLETE leaves Wire alive;
- wake descriptors are retired and Session B gets fresh state;
- artificial post-request CREDIT is absent from proof;
- premature acceptance wording is gone;
- original R13 C1-C12 and corrective CA1-CA7 are all supported;
- strict repository/dictionary/topology/host/PS2 build regressions are green;
- live/hardware/non-goal claim boundaries remain intact.

The Foreman, not this Reconstruction worker, chooses the next packet.

This worker stops after
`A003-PI-RFB-ATTACHMENT-QUIESCE-R13-CORRECTIVE-A` and does not begin shared
CONFIG/session composition, Application RFB orchestration, provider retry,
AUDIO/MPEG work, Q7 retirement/restoration or physical qualification.
