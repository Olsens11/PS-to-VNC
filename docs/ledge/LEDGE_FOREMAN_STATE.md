# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0058
RECORDED_AT=2026-09-24T18:53:18-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0057
SUPERSEDES_FOREMAN_STATE_REVISION=0057
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0058 independently accepts `A003-APPLICATION-MPEG-LIVE-SERVICE-R22`
at final source authority `345e726effee8f01d1be71938e8f09efde030c0b` and
consumes immutable Reconstruction closeout
`00015df0ef2c3a249e0d707b1f8218e3c43f95aa`.

R22 establishes the missing trigger-agnostic Application live-service seam on
top of accepted R21/P7/P3 authority. The run owner now records exact
`STARTED_WAIT_FIRST_FRAME` versus `MPEG_OWNED` state, delegates all frame
mechanics to the existing exact-generation P7 consumer, requires synchronized
first-frame/P3 proof before promotion, preserves benign post-first
IDLE/WAIT/PRESENTED/DROPPED outcomes, and fails closed on unexpected worker or
lower-owner contradictions without performing cleanup it does not own.

The next dependency is the normal retirement safe-stop/drain path. It is kept
separate from RFB restoration/reveal: first close the exact Pi generation,
continue accepted P7 service while already-admitted data drains, establish the
finite producer fence, prove natural worker completion, reclaim R4/R5 and the
R18 Transport run, and stop with the retained MPEG presentation still visible
and P2 still frozen. A later packet will own thaw/FULL-refresh/restoration and
synchronized reveal.

R22 changes PS2 loadable bytes. Its exact reproducible identity is recorded
below and is not hardware-qualified.

## Temporal architecture reconciliation

Wire Runtime Decisions revision 0011 and Architecture Overlay revision 0007
remain governing for their ownership/lifecycle rules. Their earlier statements
that then-accepted packets preserved existing Wire/product-version bytes are
true as-of those revisions and do not prohibit a later explicitly authorized,
compatibility-gated protocol semantic extension.

The accepted R16A contract
`docs/ledge/LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT.md`, together with this
later Foreman state, is the narrow later authority for provider-terminal Wire
representation and Q4 product compatibility. It does not alter the fixed PSTV
header/framing version, physical ownership, R13 finite-quiesce meanings, R14
runtime profile authority, or Application recovery ownership.

Current accepted representation:

- fixed PSTV framing/header version remains `1`;
- Q4 product-establishment compatibility is `2` on both Pi and PS2;
- RFB provider-terminal report is frame kind `ERROR=7`, channel `RFB=1`, flags
  `0`, payload length `4`;
- payload is one big-endian `uint32`: `CONNECT=1`, `READ=2`, `WRITE=3`;
- wrong channel, flags, payload length, or unknown reason is invalid;
- zero-length channel-1 DATA remains exclusively the accepted R13
  REQUEST/BOUNDARY/COMMIT/COMPLETE lifecycle representation;
- a valid provider-terminal fact is RFB-local and does not by itself set generic
  physical Wire/Transport failure;
- incompatible product-v1/product-v2 peers reject during Q4 before ACTIVE.

## Current Foreman phase

`A003_R22_APPLICATION_MPEG_LIVE_SERVICE_FOREMAN_ACCEPTED__A003_R23_APPLICATION_MPEG_RETIREMENT_DRAIN_ACTIVE__RFB_RESTORE_AND_ORDINARY_ACTIVATION_DEFERRED`

ARCHITECTURE_BLOCKER=NONE
WORK_LOG_CONTRACT_REVISION_0007_ACTIVE=YES
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
A003_MPEG_GENERATION_CONTROL_RELAY_FOREMAN_ACCEPTED=YES
A003_MPEG_RUNTIME_PROFILE_AUTHORITY_FOREMAN_ACCEPTED=YES
PI_WIRE_ESTABLISHMENT_FOUNDATION_FOREMAN_ACCEPTED=YES
PS2_WIRE_SESSION_ESTABLISHMENT_FOREMAN_ACCEPTED=YES
PI_RFB_WIRE_RELAY_CORE_FOREMAN_ACCEPTED=YES
PI_NATIVE_RFB_PROVIDER_AUTHORITY_FOREMAN_ACCEPTED=YES
PI_RFB_INTERNAL_PROVIDER_ENDPOINT_FOREMAN_ACCEPTED=YES
PI_RFB_ATTACHMENT_QUIESCE_FOREMAN_ACCEPTED=YES
RFB_SHARED_RUNTIME_PROFILE=FOREMAN_ACCEPTED
APPLICATION_RFB_ACTIVATION=FOREMAN_ACCEPTED
RFB_PROVIDER_FAILURE_REPRESENTATION=FOREMAN_ACCEPTED
RFB_FAILURE_RESTART_POLICY=FOREMAN_ACCEPTED
PI_MPEG_CONTROL_PRODUCER_OWNER=FOREMAN_ACCEPTED
MPEG_TRANSPORT_RUN_BOUNDARY=FOREMAN_ACCEPTED
RFB_FLOW_APPLICATION_COMPOSITION=FOREMAN_ACCEPTED
MPEG_PRIVATE_SESSION_BINDING=FOREMAN_ACCEPTED
TRANSPORT_RECEIVER_COMPLETION_FENCE=FOREMAN_ACCEPTED_AT_R20E_AUTHORITY
TRANSPORT_OUTBOUND_SUBMITTER_DRAIN=FOREMAN_ACCEPTED_AT_R20E_AUTHORITY
TRANSPORT_DRAIN_FAILURE_FENCE=FOREMAN_ACCEPTED
APPLICATION_MPEG_RUN_START=FOREMAN_ACCEPTED
APPLICATION_MPEG_LIVE_SERVICE=FOREMAN_ACCEPTED
APPLICATION_MPEG_RETIREMENT=RECONSTRUCTION_ACTIVE
ORDINARY_MPEG_PRODUCT_ACTIVATION=DEFERRED
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
## Accepted R16A authority

PACKET_ID=A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A
PACKET_STATUS=COMPLETED_FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0046
ASSIGNING_FOREMAN_COMMIT=ea9b07f21bab5e1686f024cae154697f0a85b1c0
RECONSTRUCTION_STARTING_COMMIT=594f2fdcebbf0f23e724b203532aeb2a14882f36
FINAL_R16A_SOURCE_COMMIT=961ad59d82b1c865b9d330a7dd9cdc1ed1e32528
R16A_RECONSTRUCTION_LOG_COMMIT=a307de050c015a46548647b45ac9f145b8ba8e71
R16A_PRE_LOG_COMMIT_COUNT=34
R16A_COMPARE_BEHIND_BY=0

The required Reconstruction closeout is:

`docs/ledge/work-log/20260922T094431-0400__reconstruction__a003-mpeg-generation__interactive.md`

### R16A criterion disposition

A003-R16A-C1=MET
A003-R16A-C2=MET
A003-R16A-C3=MET
A003-R16A-C4=MET
A003-R16A-C5=MET
A003-R16A-C6=MET
A003-R16A-C7=MET
A003-R16A-C8=MET
A003-R16A-C9=MET
A003-R16A-C10=MET
A003-R16A-C11=MET
A003-R16A-C12=MET

Independent Foreman findings:

1. `pi/rfb_attachment.py` latches the first specific CONNECT/READ/WRITE provider
   cause, makes the attachment terminal, clears provider-bound capacity, and
   retires provider and private wake resources. It contains no reconnect/rebind
   loop.
2. `pi/wire_server.py` remains the sole Pi physical recv/send/global-sequence
   owner. It serializes the attachment's terminal fact through the existing owner
   send path; successful report transmission does not erase the local failure.
3. `pi/wire_protocol.py` and PS2 `src/transport/protocol.*` define the same exact
   ERROR/channel/reason contract while retaining fixed framing version 1.
4. PS2 `src/transport/runtime.c` accepts the typed RFB terminal fact into
   RFB-local state, removes provider-bound credit, wakes blocked RFB activity,
   and does not set generic `runtime->failed` merely because the provider died.
5. Already ordered old RFB DATA may drain in its existing sequence; no later
   provider credit/ticket is minted after terminal reporting and no old authority
   is rebound to a replacement instance.
6. `src/rfb/bridge.*` and `src/rfb/rfb_session.*` expose/refine the typed terminal
   cause as RFB CONNECT/READ/WRITE failure without acquiring Application retry
   policy.
7. PS2 establishment emits Q4 product compatibility 2 on the actual physical
   establishment path. Pi and PS2 golden/protocol tests preserve header version
   1 and prove product-version mismatch rejection before ACTIVE.
8. R13 zero-length channel-1 quiesce bytes retain their sole existing meaning;
   ERROR reporting is structurally distinct from DATA/CREDIT/quiesce.
9. The dedicated PS2 host fixture proves a reader genuinely blocked for
   provider-supplied RFB bytes and a writer genuinely blocked for provider
   capacity both terminate on the typed provider fact while the physical Wire
   runtime remains live.
10. The complete R16A diff stays inside the packet-authorized Pi RFB/Wire and PS2
    Transport/RFB mechanism surfaces plus tests, workflow, dictionaries and
    directly affected documentation. `src/app.*` recovery behavior, AUDIO and
    MPEG product source are untouched.

### Exact current machine evidence

At exact pre-log source authority
`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`, GitHub Actions run
`35772175812` completed SUCCESS. Observed successful jobs:

- `host-unit`, including `Prove R16A PS2 RFB wait termination`;
- `project-check`;
- `dictionary-long`;
- `ps2-compile`;
- `ps2-link` with current-source reproducibility.

At exact immutable closeout head
`a307de050c015a46548647b45ac9f145b8ba8e71`, GitHub Actions run
`35773248375` also completed SUCCESS with the same canonical gates and dedicated
R16A wait-termination step. `dictionary-reconcile` was correctly skipped because
committed dictionaries were already reconciled.

Current linked-build evidence at the closeout head:

ISSUE7_LINKED_BUILD=PASS
LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS
ELF_PRISTINE_SHA256=06a1d2858dd195458bab81d8e88842d4fca4d3e3aa38d412c8b303fc9f8b4de2
PT_LOAD_SEGMENTS=1
PT_LOAD_SHA256=db94f95160ec28491546d53231ecf9235fcacbae1680875ac7f16bcd1b77bff8
PT_LOAD_BYTES=486932
PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

R16A_SOURCE_COMPLETE=YES
R16A_HOST_TESTED=YES
R16A_PROJECT_CHECK=PASS
R16A_STRICT_DICTIONARIES=PASS
R16A_PS2_COMPILE=PASS
R16A_PS2_LINK=PASS
R16A_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R16A_MACHINE_EVIDENCE=GITHUB_ACTIONS_ONLY
R16A_INDEPENDENT_VALIDATION=NOT_RUN
R16A_OPERATOR_OBSERVED=NO
R16A_HARDWARE_QUALIFIED=NO

The new linked identity is reproducible evidence only. It does not inherit
hardware qualification from earlier H1/checkpoint or RFB-only ELFs.

## Accepted R16B authority

PACKET_ID=A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0047
ASSIGNING_FOREMAN_COMMIT=ee0fd512175036a6e931c05701867a3b2069b9ec
ASSIGNING_FOREMAN_LOG_COMMIT=353f51c248235a9d6c1c1e5d7078de21dc6d1a74
RECONSTRUCTION_STARTING_COMMIT=353f51c248235a9d6c1c1e5d7078de21dc6d1a74
R16B_FINAL_SOURCE_COMMIT=3310568e6c35ef59d77ee6075f1f9ea5a561476e
R16B_RECONSTRUCTION_LOG_COMMIT=856a1065f5941c5149518ffe2a322bc81fe91642
R16B_PRE_LOG_COMMIT_COUNT=8

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260922T204600-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R16B criteria:

1. CONNECT, READ/EOF and WRITE provider-terminal causes reach an explicit
   Application-owned recovery branch without becoming physical-Wire failure.
2. Once that typed failure is observed, the failed attempt admits no further
   controller/keyboard/mouse publication, RFB update request or parser service.
3. Input shutdown must return its owner-proven completion result and Transport
   abort must retire the sole receiver/session before replacement admission.
4. Failure of either retirement proof blocks replacement and converges to the
   ordinary fatal path rather than force-deleting or relabeling live ownership.
5. Each replacement uses a newly connected descriptor and ordinary Q4/Transport
   startup; canonical Transport fencing keeps stale access from a dead Wire
   Session out of the successor.
6. RFB session/parser state, framebuffer validity, input runtime and published
   neutral pointer authority are freshly initialized on each attempt.
7. Pi tests prove successive Wire Sessions allocate distinct session identities
   and distinct R13 attachment objects rather than rebinding the failed one.
8. Generic RFB/physical I/O failure remains separate from the three typed
   provider-local causes and does not enter provider recovery.
9. No retry delay, backoff, timeout-as-success or in-place component restart was
   added. Successful retirement only permits another ordinary startup attempt.
10. R16A provider-terminal bytes/Q4 compatibility, R13 finite quiesce and R14
    selected RFB values are unchanged. The two R13 fixture edits only wait for
    explicit owner-state publication after already-observed serialized markers.
11. No AUDIO, MPEG, CONFIG-on-Wire, heartbeat, direct-RFB fallback or final
    all-guns orchestration product source entered R16B.
12. Exact final source and immutable-log heads are green under canonical host,
    project, strict-dictionary, pinned PS2 compile/link and reproducibility
    evidence.

R16B_SOURCE_COMPLETE=YES
R16B_HOST_TESTED=YES
R16B_PROJECT_CHECK=PASS
R16B_STRICT_DICTIONARIES=PASS
R16B_PS2_COMPILE=PASS
R16B_PS2_LINK=PASS
R16B_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R16B_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35804891365
R16B_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35805083590
R16B_INDEPENDENT_VALIDATION=NOT_RUN
R16B_OPERATOR_OBSERVED=NO
R16B_HARDWARE_QUALIFIED=NO

R16B changes linked current-source identity to:

`ELF_PRISTINE_SHA256=9b98aaa5b5239eec42545d111e7d32cedb418273fe4092f3deb5d8bd6690c40c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`

These are build/reproducibility facts only. Earlier physical qualification does
not transfer to these changed bytes.

## Accepted R17 authority

PACKET_ID=A003-PI-MPEG-CONTROL-PRODUCER-R17
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0048
ASSIGNING_FOREMAN_STATE_COMMIT=dade25f6c971584dacc7a039bc10961156654958
ASSIGNING_FOREMAN_LOG_COMMIT=826d19dd08efe48db9731ed4713f30b15be0e4b8
RECONSTRUCTION_STARTING_COMMIT=826d19dd08efe48db9731ed4713f30b15be0e4b8
R17_FINAL_SOURCE_COMMIT=cc7dc1237957bfd288addc8379caae47e83bc5a6
R17_RECONSTRUCTION_LOG_COMMIT=8698890b4f839d5708e6cefc129c37608643e155
R17_PRE_LOG_COMMIT_COUNT=15

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260922T213555-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R17 criteria:

1. Pi START/RETIRE codecs exactly mirror the accepted PS2 kind/channel/flags,
   generation-control version and 44/12-byte big-endian payload contracts.
2. START validates the active Q4 session identity, nonzero monotonically fresh
   run fence, current injected desktop bounds, 16-pixel base dimensions and
   suppression containment.
3. One explicit Pi generation owner permits only one PREPARING/LIVE/RETIRING/
   completion-pending run and rejects stale or mismatched authority.
4. Exact base capture and distinct outer suppression preparation are established
   before producer emission; inner matte remains PS2-local.
5. One FFmpeg process/reader thread uses a Configuration-bounded condition
   buffer; launch, premature EOF, reader and retirement failures fail closed.
6. Channel-4 CREDIT is session-scoped capacity while generation-local emission
   leases are exact-run fenced; all physical MPEG DATA remains serialized only
   by `WireConnectionOwner` under the global Wire sequence.
7. RETIRE moves LIVE to RETIRING before waiting for admitted send leases, so no
   new emission can enter while retirement waits.
8. RETIRE completion becomes serializable only after real producer/thread and
   exact suppression preparation retirement; deadline expiration cannot create
   success.
9. N+1 receives fresh producer, capture plan, suppression and local buffer/lease
   authority. The surviving credit balance is correctly Wire-session capacity,
   not dead-generation identity.
10. Existing Q4/R16A/R13/R14 RFB contracts and accepted PS2 MPEG Transport
    source are unchanged; the R13 fixture/source-expression correction preserves
    its existing sole-owner guard.
11. R17 does not activate PS2 Application MPEG, decoder/presentation policy,
    AUDIO, heartbeat, CONFIG delivery or final all-guns composition. The default
    Pi runtime supplies no MPEG generation factory.
12. Final source and immutable-log heads pass canonical host, project, complete
    strict dictionary, pinned PS2 compile/link and reproducibility evidence.

R17_SOURCE_COMPLETE=YES
R17_HOST_TESTED=YES
R17_FOCUSED_TESTS=12_PASS
R17_PROJECT_CHECK=PASS
R17_STRICT_DICTIONARIES=PASS
R17_PS2_COMPILE=PASS
R17_PS2_LINK=PASS
R17_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R17_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35808953320_ATTEMPT_2
R17_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35809286786_ATTEMPT_1
R17_INDEPENDENT_VALIDATION=NOT_RUN
R17_OPERATOR_OBSERVED=NO
R17_HARDWARE_QUALIFIED=NO

R17 current linked identity is:

`ELF_PRISTINE_SHA256=615407bd6ce43722f6ead9ced75985d85bec18fa24e3e0f3a841c4af21894dfc`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`

The whole ELF hash changed because the Configuration source/projection inputs
changed, but the loadable PT_LOAD fingerprint and byte count are exactly
unchanged from accepted R16B. This is build identity only; R16B was already
hardware-unqualified and no qualification is inferred for R17.

## Accepted R18 authority

PACKET_ID=A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0049
ASSIGNING_FOREMAN_STATE_COMMIT=3718615a9cf9d07c3598ecab6fbaeba09e5751a3
ASSIGNING_FOREMAN_LOG_COMMIT=95d7b77e431f0f0765491da79e75591aae6e0300
RECONSTRUCTION_STARTING_COMMIT=95d7b77e431f0f0765491da79e75591aae6e0300
R18_FINAL_SOURCE_COMMIT=2e0589fd9fe531c5dfb7130df301b6f0eadd09fd
R18_RECONSTRUCTION_LOG_COMMIT=ce1d405bde73ee764408c2dddcc40d2f8cc6eccc
R18_PRE_LOG_COMMIT_COUNT=17

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260922T234027-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R18 criteria:

1. MPEG run admission opens only from a fully clean idle Transport state and
   does not allocate product generation identity.
2. Pre-START abort is permitted only while the opened boundary is still proven
   pristine; dirty data/state cannot be laundered back into idle.
3. Channel-4 DATA requires an explicit open, non-retiring, non-finalizing run
   boundary; out-of-phase DATA fails the Wire runtime.
4. Exact RETIRE completion is correlated to the submitted RETIRE payload and
   atomically closes DATA admission under the MPEG queue lock before the
   completion becomes observable to Application.
5. Taking the completion clears only the bounded completion slot; the retirement
   latch remains closed until explicit finalization.
6. Residual queue discard returns an exact byte count and resets ring offsets
   without claiming decoder consumption.
7. Finalization combines already-withheld consumed-byte credit with residual
   discarded bytes exactly once and sends that value only through Transport's
   existing sole outbound path.
8. Successful finalization clears old queue, producer-done, control-correlation,
   activity, credit and retirement facts while reusing the same session-owned
   allocation.
9. A live MPEG activity waiter or dequeue-to-credit-return consumer transaction
   blocks finalization. `mpeg_finalization_in_progress` blocks new MPEG
   consumer/waiter/producer-done activity while final credit is in flight.
10. N finalization followed by N+1 open admits only fresh successor bytes/state;
    old residual, producer-done, completion, pending credit and activity facts
    cannot be observed as N+1 authority.
11. RFB, AUDIO, Q4, R16A/R16B, R17 Pi, exact START/RETIRE framing and MPEG
    decoder/worker/presentation contracts remain unchanged.
12. Final source and immutable-log heads pass canonical host, project, complete
    strict dictionary, pinned PS2 compile/link and reproducibility evidence.

The late R18 concurrency correction is part of accepted authority, not an
optional follow-up. Earlier green checkpoints before `1406100933666b8a19f800ec7f0d90548b03d06b`
are superseded by the final source authority above.

R18_SOURCE_COMPLETE=YES
R18_HOST_TESTED=YES
R18_PROJECT_CHECK=PASS
R18_STRICT_DICTIONARIES=PASS
R18_PS2_COMPILE=PASS
R18_PS2_LINK=PASS
R18_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R18_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35816881762_ATTEMPT_1
R18_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35817077825_ATTEMPT_1
R18_INDEPENDENT_VALIDATION=NOT_RUN
R18_OPERATOR_OBSERVED=NO
R18_HARDWARE_QUALIFIED=NO

R18 current linked identity is:

`ELF_PRISTINE_SHA256=4d0bc02f6bc89f138acb53ddb7751fd9e91a300b9e7cbfd9bc7301920f08d1c4`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=e244769ef21dadd8a09e6fe65ec4b2201e0acd9beb52ae06180ab3d4f3a3a232`
`PT_LOAD_BYTES=490516`

That PT_LOAD differs from accepted R17 and therefore creates new exact-identity
hardware debt. Build reproducibility does not qualify the new hardware-facing
Transport source.

## Accepted R19 authority

PACKET_ID=A004-RFB-FLOW-APPLICATION-COMPOSITION-R19
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0050
ASSIGNING_FOREMAN_STATE_COMMIT=1dd32c35e9ef528fe2d4b6561d18c6bc4b328e6d
ASSIGNING_FOREMAN_LOG_COMMIT=3ab9d8bbd57bac9cf078c92dae772474f358d198
RECONSTRUCTION_STARTING_COMMIT=3ab9d8bbd57bac9cf078c92dae772474f358d198
R19_FINAL_SOURCE_COMMIT=cd09bc263e5835c75d5a8fea256cd71127e7821b
R19_RECONSTRUCTION_LOG_COMMIT=1ded79fdfb5f15eb4266a3e997cd445a26c126a2
R19_PRE_LOG_COMMIT_COUNT=6

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260923T042621-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R19 criteria:

1. A fresh thawed `pstvnc_rfb_flow_policy_t` is initialized only after each
   attempt's authoritative startup full frame and is rebuilt on provider/Wire
   replacement.
2. Every ordinary post-startup framebuffer request is selected through
   `pstvnc_rfb_flow_policy_next_request()`; Application no longer owns an
   independent incremental/FULL scheduling rule.
3. A request is recorded as outstanding only after the existing RFB session
   reports successful serialization. Failed serialization records no P2 send.
4. Every completed live UPDATE retires exactly one outstanding P2 request
   before publication or successor scheduling.
5. Dirty remote framebuffer truth is presented only when P2 permits remote
   publication; parsing and framebuffer mutation remain independent.
6. With no production freeze caller, the ordinary product remains thawed and
   continues one incremental request at a time with normal dirty presentation.
7. IDLE and HOLD neither fabricate update completion nor send/record duplicate
   requests.
8. R16B provider replacement cannot inherit outstanding/frozen/FULL-debt P2
   state from the failed attempt.
9. FULL-refresh debt, coalescing and frozen in-flight semantics remain solely in
   the unchanged accepted `src/rfb/flow_policy.*` implementation.
10. RFB session/parser, Transport, framebuffer truth, graphics/local UI and
    Application composition ownership remain separated.
11. No MPEG, calibration, AUDIO, Pi, Transport-product, protocol-byte or final
    all-guns activation entered R19.
12. Final source and immutable-log heads pass canonical host, project, strict
    dictionary, pinned PS2 compile/link and current-source reproducibility.

R19_SOURCE_COMPLETE=YES
R19_HOST_TESTED=YES
R19_PROJECT_CHECK=PASS
R19_STRICT_DICTIONARIES=PASS
R19_PS2_COMPILE=PASS
R19_PS2_LINK=PASS
R19_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R19_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35852093499_ATTEMPT_1
R19_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35852383542_ATTEMPT_2
R19_INDEPENDENT_VALIDATION=NOT_RUN
R19_OPERATOR_OBSERVED=NO
R19_HARDWARE_QUALIFIED=NO

R19 current linked identity is:

`ELF_PRISTINE_SHA256=308688f933c72de19934eae3d8048e9476de91e688d2098db0e98d283ca0f4b0`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`
`PT_LOAD_BYTES=491156`

This PT_LOAD differs from accepted R18 and therefore creates a newer exact
hardware-debt identity. Repository reproducibility does not physically qualify
that image.

## Accepted R20 authority

PACKET_ID=A003-MPEG-PRIVATE-SESSION-BINDING-R20
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0051
ASSIGNING_FOREMAN_STATE_COMMIT=84e0434f1e20f18a38d74d20b4d20116124eaedd
ASSIGNING_FOREMAN_LOG_COMMIT=102010653642af43e7b1e32d585c4d16b280b12f
RECONSTRUCTION_STARTING_COMMIT=102010653642af43e7b1e32d585c4d16b280b12f
R20_FINAL_SOURCE_COMMIT=3e39753b1b3bce9fe187748deb7eeb4d6201151c
R20_RECONSTRUCTION_LOG_COMMIT=0a81114fb1c8691fb677fed343cd45bfbae24487
R20_PRE_LOG_COMMIT_COUNT=8

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260923T075029-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R20 criteria:

1. Public START request contains generation plus base/suppression geometry only;
   no Wire session identity or generation-control version leaks upward.
2. Transport stamps the current private Q4 session identity and accepted control
   version into the unchanged full START payload.
3. Public RETIRE request contains only exact caller-owned generation.
4. Runtime retains full submitted RETIRE payload correlation before publishing
   completion.
5. Bridge completion revalidates private version/session authority and projects
   only completed generation above Transport.
6. No public Q4 session-ID accessor or duplicate owner was introduced.
7. START remains 44-byte v1 and RETIRE remains 12-byte v1 with unchanged frame
   kinds/channels/flags/endianness/product compatibility.
8. Stale access tickets cannot submit START/RETIRE or consume completion after
   replacement; current-session requests stamp only the replacement identity.
9. R18 run-open, DATA admission, retirement latch, residual/credit finalization
   and successor-run semantics remain unchanged.
10. Pi R17 exact-session/exact-generation source is unchanged and its tests
    remain green.
11. No Application, calibration, worker/backend, Display, RFB, AUDIO, Pi or
    Configuration product activation entered R20.
12. Final source and immutable-log heads pass host, project, strict dictionary,
    pinned PS2 compile/link and current-source reproducibility evidence.

R20_SOURCE_COMPLETE=YES
R20_HOST_TESTED=YES
R20_PROJECT_CHECK=PASS
R20_STRICT_DICTIONARIES=PASS
R20_PS2_COMPILE=PASS
R20_PS2_LINK=PASS
R20_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R20_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35857721371_ATTEMPT_1
R20_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35858084665_ATTEMPT_1
R20_INDEPENDENT_VALIDATION=NOT_RUN
R20_OPERATOR_OBSERVED=NO
R20_HARDWARE_QUALIFIED=NO

R20 current linked identity is:

`ELF_PRISTINE_SHA256=43f2c43f537a32f7205ab4b7711f0c53a6ac8bdb049818f5416d24a14fdb63d0`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
`PT_LOAD_BYTES=491540`

This PT_LOAD differs from accepted R19 and therefore creates the current exact
hardware-debt identity. Repository reproducibility does not physically qualify
that image.

## Accepted cumulative R20C/R20D/R20E Transport reclaim authority

ACCEPTED_CORRECTION_AUTHORITY=316ad217bef229c9ca0134b9b922a213cb5af247
R20C_SOURCE_COMMIT=4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3
R20C_LOG_COMMIT=3fac8afb2d403f2b96171dfadc3386c8c57da347
R20D_SOURCE_COMMIT=c719a46003e04ac625cb92f4b455c94fe7281020
R20D_LOG_COMMIT=f15ff5bc4969cb867088b9567667865ef6849685
R20E_SOURCE_COMMIT=316ad217bef229c9ca0134b9b922a213cb5af247
R20E_LOG_COMMIT=5357e7ed422bb402f187a2887ad0e67bcc710e4a

R20C and R20D remain historically recorded as intermediate candidates that
were not accepted at their own heads because later review found additional
ownership gaps. State 0056 accepts the **cumulative corrected mechanism** only
at the R20E authority above.

Independent Foreman review accepts all twelve R20E criteria:

1. Reclaim success now requires explicit `PROVEN` completion outcome after
   successful R20D submitter drain.
2. A private drain `WaitSema()` failure latches `FAILED` and cannot publish
   reclaim success.
3. A drain wake with protected nonzero submitter count latches `FAILED` and
   cannot publish reclaim success.
4. `wait_receiver_done()` returns success only for `PROVEN`; repeated FAILED
   observation cannot upgrade the runtime.
5. `release()` exits before kernel-thread inspection or any semaphore/queue/
   stack/physical-stream reclaim when outcome is FAILED.
6. Normal R20D active+queued submitter drain remains unchanged and proven.
7. Successful completion remains latched/reobservable through the binary
   terminal-outcome rendezvous.
8. A failed completion cannot become fresh-session authority even if submitter
   count later reaches zero.
9. R20C failed-shutdown convergence and post-proof retryable kernel-status
   behavior remain intact.
10. No timeout, sleep, priority assumption or diagnostic fact substitutes for
    reclaim proof.
11. No Application, Pi, RFB policy, AUDIO policy, MPEG media owner,
    Presentation/calibration, Configuration or protocol scope entered R20E.
12. Exact final source and immutable-log heads pass canonical host, project,
    strict dictionary, pinned PS2 compile/link and reproducibility evidence.

R20E_SOURCE_COMPLETE=YES
R20E_FOREMAN_ACCEPTED=YES
R20C_R20D_R20E_CUMULATIVE_RECLAIM_FENCE=FOREMAN_ACCEPTED
R20E_HOST_TESTED=YES
R20E_PROJECT_CHECK=PASS
R20E_STRICT_DICTIONARIES=PASS
R20E_PS2_COMPILE=PASS
R20E_PS2_LINK=PASS
R20E_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R20E_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36030820124_ATTEMPT_2
R20E_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36031179032_ATTEMPT_1
R20E_INDEPENDENT_VALIDATION=NOT_RUN
R20E_OPERATOR_OBSERVED=NO
R20E_HARDWARE_QUALIFIED=NO

Host regression evidence at the immutable-log head includes:

- `transport protocol tests passed`;
- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi R17 MPEG generation fixture — 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`;
- `transport_rfb_provider_failure_test: PASS`.

Accepted cumulative correction linked identity:

`ELF_PRISTINE_SHA256=c838f0afda456c8026f8ea30afed1aad68eb7b0480142d0a3d956ed0e636274c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c280849d1310b0a530daa77da736f22b0eae175a0adbdc43a0cb856c3c560adc`
`PT_LOAD_BYTES=492308`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

This exact loadable image differs from accepted R20 and therefore becomes the
new current exact hardware-debt identity. Repository evidence does not
physically qualify it.

## Accepted R21 authority

PACKET_ID=A003-APPLICATION-MPEG-RUN-START-R21
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0056
ASSIGNING_FOREMAN_STATE_COMMIT=ff0bdf1adcf28aa810a03ff1bfb9e4177d86d7dd
ASSIGNING_FOREMAN_LOG_COMMIT=05f8dd5cbcacbc46510f2d04f229e593def6601b
RECONSTRUCTION_STARTING_COMMIT=05f8dd5cbcacbc46510f2d04f229e593def6601b
R21_FINAL_SOURCE_COMMIT=fc8bb2652d94d8163e1e5b375e33c9e37b73017b
R21_RECONSTRUCTION_LOG_COMMIT=964d1cfa8db8c834d9ee32a1541c42ffb1aace70
R21_PRE_LOG_COMMIT_COUNT=7

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260924T131021-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R21 criteria:

1. One coordinator instance owns a monotonically increasing nonzero generation
   sequence; allocation is never rolled back after an admitted attempt and
   UINT32 exhaustion faults without wrap.
2. One already-resolved base/inner/suppression geometry value is validated
   before activation, including nonempty rectangles, macroblock-sized base,
   symmetric inner inset, suppression containment and selected decoder-profile
   maximum dimensions.
3. P3 is armed with the full geometry; R20 START is built only from the retained
   exact P3 snapshot and carries base/suppression but not the inner matte.
4. R21 requires P2 already to deny remote publication and never freezes or
   thaws P2 itself.
5. R18 run-open occurs before worker consumer activity and is pre-START-aborted
   only when the START invocation boundary has not been crossed.
6. Fresh R5 runtime, R3 backend operations and R4 worker are composed for the
   exact allocated generation using the selected R7 profile.
7. P3 WAIT_FIRST_FRAME and P7 consumer initialization both complete before
   START may be invoked.
8. START is the final startup action. Successful START leaves exact
   `STARTED_WAIT_FIRST_FRAME` run authority.
9. Every pre-START failure either proves reverse-order release through P3/R4/R5/
   R18 or faults the coordinator when cleanup cannot be proven.
10. Once START is invoked, even a non-OK result faults and requires outer
    session teardown; pre-START abort/local cleanup is never used after that
    irreversible boundary.
11. R21 does not service frames, retire the run, thaw/reveal RFB, activate Pi
    MPEG product behavior, select a UI/controller trigger, modify ordinary
    `src/app.c`, or alter AUDIO/lower-owner product semantics.
12. Final source and immutable-log heads pass canonical host/project/strict
    dictionary/pinned PS2 compile/link/current-source reproducibility evidence.

R21_SOURCE_COMPLETE=YES
R21_FOREMAN_ACCEPTED=YES
R21_HOST_TESTED=YES
R21_PROJECT_CHECK=PASS
R21_STRICT_DICTIONARIES=PASS
R21_PS2_COMPILE=PASS
R21_PS2_LINK=PASS
R21_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R21_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36035059916_ATTEMPT_1
R21_LOG_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36035354181_ATTEMPT_1
R21_INDEPENDENT_VALIDATION=NOT_RUN
R21_OPERATOR_OBSERVED=NO
R21_HARDWARE_QUALIFIED=NO

Host evidence at both exact final authorities includes
`app_mpeg_run_test: PASS` together with the existing Transport runtime/MPEG,
P2, Pi R17 and ordinary Application R15/R16B/R19 regressions.

Accepted R21 linked identity:

`ELF_PRISTINE_SHA256=82ea220b084d50a857957f68950c10e8e7eab6a23e4857ab923e61f713c03ac8`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=910597af49fe2121ab730a2db82aa4259177256156062853bd2a008f0d4d6045`
`PT_LOAD_BYTES=494996`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

This exact loadable image differs from accepted R20E and therefore becomes the
new current hardware-debt identity. Repository reproducibility does not
physically qualify it.

## Next dependency decision

R21 intentionally leaves the successful run in
`PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME` with one initialized P7 frame
consumer and a live exact-generation worker.

The accepted P7 mechanism already owns the actual main-thread frame service:

- it claims exact-generation worker frames;
- first frame performs synchronized physical MPEG presentation, arms the
  common media epoch at that exact boundary, and promotes P3 to MPEG_OWNED;
- later frames use the accepted absolute scheduler/drop policy;
- WAIT retains the exact claim and deadline without blocking the main loop;
- mapping/scheduler/compositor/release failures contain the borrow and request
  exact worker stop where possible.

However, R21's Application run owner currently exposes no service operation and
its run state cannot record the first-frame promotion. Jumping directly to
retirement would force a later caller either to reach into the coordinator's
`frame_consumer` field directly or to infer live ownership only from lower
component state. That would weaken the coherent Application transaction R21
just established.

The smallest next dependency is therefore to compose P7 service through the
R21 run owner and make the first synchronized promotion a truthful Application
run-state transition. Retirement remains a separate following packet.

## Governing invariants for R22

1. R22 uses the already-accepted P7 consumer directly; it does not fork frame
   claim/mapping/scheduler/compositor mechanics into `app_mpeg_run.*`.
2. Live service is valid only for the exact current nonzero run generation in
   STARTED_WAIT_FIRST_FRAME or MPEG_OWNED Application state.
3. Before first-frame promotion, P3 must remain exact-generation
   WAIT_FIRST_FRAME. A benign P7 IDLE with an unfinished worker leaves the
   Application state unchanged.
4. The first successful P7 PRESENTED result may transition Application state to
   MPEG_OWNED only when P3 now confirms exact-generation MPEG_OWNED and the
   P7 result proves the synchronized first-frame promotion.
5. Application does not arm the media clock. P7/compositor remain sole owners
   of first physical synchronization, clock arm and P3 promotion.
6. In MPEG_OWNED state, benign P7 IDLE/WAIT/PRESENTED/DROPPED results preserve
   exact run identity. WAIT retains the lower P7 claim/deadline exactly.
7. An observed worker-finished fact while no Application stop/retirement has
   been requested is unexpected termination under A003 and must fail the run
   closed; it is not normal completion or retirement proof.
8. Any negative/faulted P7 service result or impossible P3/generation/state
   contradiction faults the Application run and requires outer teardown. R22
   must preserve lower-owner claim/worker evidence rather than memset or
   manufacture cleanup.
9. R22 performs no run retirement: no P3 begin/seal/reveal, no R20 RETIRE, no
   producer-done mark, no R4 stop/join/release, no R5 release, no R18 finalize,
   and no P2 thaw/FULL-refresh scheduling.
10. R22 does not choose a calibration/input trigger or modify ordinary
    `pstvnc_app_run*()` to start/service MPEG.
11. Pi product runtime remains dormant; RFB/AUDIO/Transport/MPEG/Display private
    mechanisms and fixed Wire bytes remain unchanged.
12. Existing R21 start evidence and P7 focused tests remain authoritative and
    green; R22 adds only Application-state/service composition evidence.

## Accepted R22 authority

PACKET_ID=A003-APPLICATION-MPEG-LIVE-SERVICE-R22
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0057
ASSIGNING_FOREMAN_STATE_COMMIT=6b0e06c99f950a3e1f145c38321b5fd0828b3802
ASSIGNING_FOREMAN_LOG_COMMIT=07b9dc4ec91c2947b7da62164bda6f214ab5b78e
RECONSTRUCTION_STARTING_COMMIT=07b9dc4ec91c2947b7da62164bda6f214ab5b78e
R22_FINAL_SOURCE_COMMIT=345e726effee8f01d1be71938e8f09efde030c0b
R22_RECONSTRUCTION_LOG_COMMIT=00015df0ef2c3a249e0d707b1f8218e3c43f95aa
R22_PRE_LOG_COMMIT_COUNT=4

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260924T164702-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts all twelve R22 criteria:

A003-R22-C1=MET
A003-R22-C2=MET
A003-R22-C3=MET
A003-R22-C4=MET
A003-R22-C5=MET
A003-R22-C6=MET
A003-R22-C7=MET
A003-R22-C8=MET
A003-R22-C9=MET
A003-R22-C10=MET
A003-R22-C11=MET
A003-R22-C12=MET

Independent findings:

1. The run owner exposes exactly one new `MPEG_OWNED` state and one
   `pstvnc_app_mpeg_run_service()` operation; generation allocation and
   lower-owner mechanics are unchanged.
2. The service invokes only the existing P7 consumer for
   `run->current_generation`; first-frame promotion requires exact P7
   synchronized/first-frame effects plus an independent exact-generation P3
   MPEG_OWNED snapshot.
3. Before first frame, unfinished P7 IDLE preserves WAIT_FIRST_FRAME. After
   promotion, IDLE/WAIT/PRESENTED/DROPPED preserve Application live authority
   and the detailed P7 result, including WAIT claim/deadline evidence.
4. Unexpected worker completion, negative P7 results, faulted P7 status,
   generation mismatch, impossible P3 state, or result/status contradictions
   fault the run and require outer session teardown.
5. The live failure path performs no P3 abort, worker stop/join/release, R5
   release, RETIRE, Transport finalize, P2 thaw, or reveal. Lower-owner evidence
   remains intact.
6. The behavioral diff is confined to `src/app_mpeg_run.*`, its focused unit
   fixture, and directly affected documentation/dictionaries. Ordinary
   `src/app.c`, P7, Display, MPEG worker/runtime/backend, Transport, Pi, AUDIO,
   Input/UI and protocol bytes are unchanged.

Exact machine evidence at final source authority
`345e726effee8f01d1be71938e8f09efde030c0b` is GitHub Actions run
`36058496825`, attempt 1:

- host-unit — PASS, including `app_mpeg_run_test: PASS`,
  `APP_MPEG_FRAME_TEST=PASS`, Presentation/scheduler/compositor/worker,
  Transport runtime/MPEG and RFB-flow regressions;
- project-check — PASS, including source topology and work-log checks;
- dictionary-long — PASS with `SOURCE_DICTIONARIES=PASS`;
- ps2-compile — PASS with `CLEAN_PS2_COMPILE_CHECK=PASS`;
- ps2-link/current-source reproducibility — PASS.

R22_SOURCE_COMPLETE=YES
R22_FOREMAN_ACCEPTED=YES
R22_HOST_TESTED=YES
R22_PROJECT_CHECK=PASS
R22_STRICT_DICTIONARIES=PASS
R22_PS2_COMPILE=PASS
R22_PS2_LINK=PASS
R22_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R22_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36058496825_ATTEMPT_1
R22_INDEPENDENT_VALIDATION=NOT_RUN
R22_OPERATOR_OBSERVED=NO
R22_HARDWARE_QUALIFIED=NO

Accepted R22 linked identity:

`ELF_PRISTINE_SHA256=71412c76f5f748913b1f8247e6e1c18c5e4a4ffe7e8a89da98f65c6f343ab5d1`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`
`PT_LOAD_BYTES=496404`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R21 loadable bytes and is current exact
hardware-debt identity only.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0058
BASED_ON_ACCEPTED_R22_SOURCE=345e726effee8f01d1be71938e8f09efde030c0b
BASED_ON_R22_LOG=00015df0ef2c3a249e0d707b1f8218e3c43f95aa
BASED_ON_ACCEPTED_TRANSPORT_RECLAIM_SOURCE=316ad217bef229c9ca0134b9b922a213cb5af247

### Objective

Add one trigger-agnostic Application-owned retirement safe-stop/drain
transaction for an exact R22 MPEG_OWNED run.

R23 must close the exact Pi producer generation through the accepted R20 RETIRE
seam, continue the accepted P7 consumer while already-admitted MPEG data drains,
publish producer completion only after exact RETIRE completion, prove natural
R4 worker completion, reclaim the R4/R5 execution owners and finalize the R18
Transport run. It must then stop at a truthful restoration-pending state while
P3 still retains the MPEG composite and P2 remains frozen.

Do not thaw/refresh/reveal RFB and do not activate ordinary product flow in R23.

### Required behavior

1. **Exact retirement admission.** Retirement begins only from one healthy
   `MPEG_OWNED` Application run whose exact generation is simultaneously
   owned by P3/P7 and whose R18/R4/R5 owners remain live. IDLE,
   WAIT_FIRST_FRAME, FAULTED, duplicate-retirement and fabricated owner states
   reject or fail closed without manufacturing cleanup.
2. **Visible retirement precedes producer close.** Move P3 from MPEG_OWNED to
   RETIRING for the exact generation before invoking RETIRE. This keeps the
   retained MPEG/mattes visible and permits accepted P7 drain service.
3. **Exactly one R20 RETIRE.** Serialize one RETIRE for the current generation
   through the accepted private-session Transport seam. Once that invocation
   boundary is crossed, failure is teardown-required and may not be converted
   back to live MPEG_OWNED or IDLE.
4. **Nonblocking drain service.** While retirement is active, continue using
   only the existing P7 consumer in P3 RETIRING. IDLE/WAIT/PRESENTED/DROPPED
   remain ordinary drain outcomes while their exact claim/deadline/accounting
   facts stay coherent.
5. **Exact completion fence.** `WOULD_BLOCK` from RETIRE-completion take is
   benign pending state. Only an exact current-generation completion is
   accepted. Wrong generation, duplicate/impossible completion or Transport
   failure faults the run.
6. **Producer-done ordering.** Call
   `pstvnc_transport_mpeg_mark_producer_done()` exactly once and only after
   the exact RETIRE completion has been taken. Worker completion observed before
   that producer fence is failure, not clean retirement.
7. **Natural decoder/worker completion.** After producer-done, continue P7 drain
   until the worker reports finished with no P7 claim outstanding. Clean
   retirement does not request Application-owned worker stop. Then require exact
   worker join and a `COMPLETED` outcome for the current generation; STOPPED,
   FAILED, wrong-generation or unprovable outcome faults.
8. **Execution-owner reclaim.** Only after exact clean join/outcome, retire the
   now-empty P7 value, release the R4 worker, then release the R5 PS2 worker
   runtime. Failed reclaim leaves truthful retained ownership/fault evidence and
   requires outer teardown.
9. **R18 finalization last.** Invoke
   `pstvnc_transport_mpeg_run_finalize()` only after RETIRE completion was
   taken, producer-done was published, P7 has no claim, and R4/R5 consumer
   ownership is retired. Successful finalization clears only Transport's
   accepted old-run state/credit debt; it does not allocate or start a successor.
10. **Truthful restoration-pending endpoint.** On success, record an explicit
    Application state meaning the MPEG execution/Transport run is retired but
    visual/RFB restoration is still pending. Preserve the exact generation and
    require P3 to remain RETIRING with its snapshot intact and P2 to remain
    frozen.
11. **No restoration or product activation.** R23 performs no P3 seal/reveal,
    no synchronized no-MPEG reveal, no P2 thaw, no FULL-refresh request/debt
    completion, no ordinary `src/app.c` trigger/service wiring, no Pi product
    factory activation, and no AUDIO/Input/UI/calibration/protocol changes.
12. **Evidence/dictionaries.** Add deterministic retirement-order/state/failure
    tests while retaining R21/R22/P7/P3/R18/R20/R4/R5 regressions and canonical
    project/dictionary/PS2 build evidence green.

### Acceptance criteria

- A003-R23-C1 RETIREMENT_ADMISSION_REQUIRES_EXACT_HEALTHY_MPEG_OWNED_RUN
- A003-R23-C2 P3_RETIRING_PRECEDES_RETIRE_AND_PRESERVES_VISIBLE_DRAIN
- A003-R23-C3 EXACTLY_ONE_CURRENT_GENERATION_RETIRE_CROSSES_IRREVERSIBLE_BOUNDARY
- A003-R23-C4 RETIRING_DRAIN_USES_ONLY_EXISTING_P7_AND_PRESERVES_WAIT_CLAIM
- A003-R23-C5 RETIRE_COMPLETION_WOULD_BLOCK_IS_PENDING_AND_EXACT_COMPLETION_IS_FENCED
- A003-R23-C6 PRODUCER_DONE_OCCURS_ONLY_AFTER_EXACT_RETIRE_COMPLETION
- A003-R23-C7 CLEAN_WORKER_COMPLETION_IS_POST_FENCE_NATURAL_JOINED_COMPLETED_OUTCOME
- A003-R23-C8 P7_R4_R5_RECLAIM_REQUIRES_NO_OUTSTANDING_BORROW
- A003-R23-C9 R18_FINALIZE_IS_LAST_EXECUTION_RECLAIM_STEP
- A003-R23-C10 SUCCESS_ENDS_RESTORE_PENDING_WITH_P3_RETAINED_AND_P2_FROZEN
- A003-R23-C11 NO_THAW_FULL_REFRESH_REVEAL_PRODUCT_OR_LOWER_OWNER_SCOPE_CREEP
- A003-R23-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

R23 must prove at least:

1. begin-retirement rejects IDLE/WAIT_FIRST_FRAME/FAULTED and fabricated owner
   states before sending RETIRE;
2. exact MPEG_OWNED begin changes P3 to RETIRING before one current-generation
   RETIRE and records the irreversible boundary;
3. a RETIRE send failure faults/teardown-requires without reverting P3 or
   pretending the run is reusable;
4. pending RETIRE completion plus P7 IDLE/WAIT/PRESENTED/DROPPED preserves exact
   retiring state and P7 result/claim/deadline;
5. completion WOULD_BLOCK is benign, while wrong-generation/duplicate/failure is
   fail-closed;
6. producer-done is impossible before exact completion and is emitted exactly
   once after it;
7. worker_finished before producer-done faults; after producer-done it is
   accepted only with exact P7 no-claim/status proof;
8. clean join requires exact-generation COMPLETED outcome; STOPPED/FAILED/join
   failure is not clean retirement;
9. worker/runtime release ordering precedes Transport finalize, and any reclaim
   failure preserves truthful fault/ownership evidence;
10. successful endpoint has Transport run closed, worker/runtime no longer
    owned, current generation retained, P3 still RETIRING, and P2 still frozen;
11. no P3 seal/reveal or P2 thaw/FULL-refresh action occurs;
12. existing R21/R22 start/live-service tests and lower-owner regressions remain
    green.

### Authorized source surface

R23 may modify only the smallest justified subset of:

- `src/app_mpeg_run.c` / `.h`;
- `tests/unit/app_mpeg_run_test.c` or one focused companion R23 Application
  fixture;
- directly affected Application retirement documentation/dictionaries;
- build/check manifests only if genuinely required by the source/test change.

Display/Presentation/P7/compositor/scheduler, MPEG worker/backend/runtime,
Transport, RFB flow policy, Configuration, Pi product source, AUDIO,
Input/UI/calibration, protocol bytes and ordinary `src/app.c` are not
authorized. If composition exposes a concrete accepted lower-owner defect,
return BLOCKED rather than broadening R23.

### Required checks before handoff

Run focused R21/R22/R23 Application tests plus existing P7, P3, R18/R20,
worker/runtime/backend, RFB-flow and ordinary Application regressions; canonical
host tests; project check; complete strict dictionary audit; pinned PS2
compile/link and current-source reproducibility. Preserve exact new ELF/PT_LOAD
identity if loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

Accepted current PS2 loadable authority is R22:

`PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`
`PT_LOAD_BYTES=496404`

This exact identity is repository-reproducible but not physically qualified.
R21 and earlier hardware-facing identities remain historical evidence at their
respective source authorities.

Any loadable-byte change from R23 creates a newer exact hardware-debt identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
