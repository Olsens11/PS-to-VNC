# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0054
RECORDED_AT=2026-09-24T07:58:40-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0053
SUPERSEDES_FOREMAN_STATE_REVISION=0053
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0054 independently reviews the returned
`A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C` candidate at final source
`4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3` and immutable Reconstruction
closeout `3fac8afb2d403f2b96171dfadc3386c8c57da347`.

R20C correctly separates early receiver terminality from the receiver thread's
final no-touch edge and prevents release from terminating/deleting that I/O
owner before its pending-outbound resolution and logical RFB/AUDIO/MPEG terminal
publication are complete.

Foreman review nevertheless finds one remaining reclaim race not covered by the
returned implementation: failing a pending outbound transaction signals its
submitting caller, but the caller still touches `outbound_work` and
`outbound_slot_semaphore_id` after that wake. The receiver may publish its final
completion semaphore before that submitter has returned from the outbound
rendezvous. `release()` may then delete the outbound semaphores while the
submitter is still using them. A second caller that passed admission before
terminality may also already be queued on the outbound slot.

The R20C host proof masks this product gap by explicitly joining the outbound
submitter before releasing the receiver-completion test barrier. Product source
contains no equivalent submitter-drain rendezvous.

Revision 0054 therefore does not Foreman-accept R20C. R21 remains queued and the
architecture blocker narrows to an outbound-submitter drain fence. One bounded
A001 corrective packet must make receiver completion reclaim-authoritative for
both the I/O owner and all pre-terminal outbound submitters.
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

`A003_R20_MPEG_PRIVATE_SESSION_BINDING_FOREMAN_ACCEPTED__A001_R20C_RECEIVER_FENCE_CORRECT_BUT_OUTBOUND_SUBMITTER_DRAIN_INCOMPLETE__A001_R20D_ACTIVE__A003_R21_QUEUED`

ARCHITECTURE_BLOCKER=TRANSPORT_OUTBOUND_SUBMITTER_DRAIN_FENCE
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
TRANSPORT_RECEIVER_COMPLETION_FENCE=CORRECTION_REQUIRED
TRANSPORT_OUTBOUND_SUBMITTER_DRAIN=RECONSTRUCTION_ACTIVE
APPLICATION_MPEG_RUN_START=DEPENDENCY_QUEUED
APPLICATION_MPEG_RETIREMENT=DEPENDENCY_QUEUED
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

## R20C Foreman review

PACKET_ID=A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C
PACKET_STATUS=FOREMAN_NOT_ACCEPTED_CORRECTION_REQUIRED
ASSIGNING_FOREMAN_STATE_REVISION=0053
ASSIGNING_FOREMAN_STATE_COMMIT=27be6d8f5cf753617912a8e4c4284d720534d919
ASSIGNING_FOREMAN_LOG_COMMIT=003306d399c22f2a02625a867a512c225931a87a
RECONSTRUCTION_STARTING_COMMIT=003306d399c22f2a02625a867a512c225931a87a
R20C_FINAL_SOURCE_COMMIT=4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3
R20C_RECONSTRUCTION_LOG_COMMIT=3fac8afb2d403f2b96171dfadc3386c8c57da347
R20C_PRE_LOG_COMMIT_COUNT=8

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260924T061735-0400__reconstruction__a003-mpeg-generation__interactive.md`

R20C correctly fixes the originally discovered receiver-thread race:

- `receiver_done` remains an early admission-closing fact;
- the receiver completion semaphore is signaled only after the sole physical-I/O
  owner resolves the currently pending outbound item and publishes terminal
  RFB/AUDIO/MPEG wake state;
- `wait_receiver_done()` consumes/restores that completion token instead of
  short-circuiting on early `receiver_done`;
- `release()` must pass the completion token before inspecting or forcing the
  receiver thread and before reclaiming queues, semaphores, stack or stream;
- deterministic barriers prove release cannot terminate/delete the receiver
  while that receiver is held before final completion publication;
- failed socket shutdown convergence, retryable `ReferThreadStatus` failure and
  fresh-runtime reset behavior remain covered.

However, receiver completion is still not a complete Transport-resource
no-touch fence.

### Remaining outbound-submitter race

`pstvnc_transport_runtime_submit_frame()` owns
`outbound_slot_semaphore_id` from entry at the serialized outbound slot until
after it wakes from `outbound_done_semaphore_id`, reads
`outbound_work.result`, and signals `outbound_slot_semaphore_id` on return.

On terminal receiver exit, `pstvnc_transport_runtime_fail_pending_outbound()`
sets the pending result and signals `outbound_done_semaphore_id`, but does not
wait for the submitting caller to finish that return path.

A valid product interleaving is therefore:

1. submitter A owns the outbound slot, publishes one pending item, and waits on
   outbound-done;
2. receiver publishes early terminality, fails the pending item, and signals
   outbound-done;
3. before submitter A runs again, receiver completes logical terminal wake work
   and signals the receiver-completion semaphore;
4. release passes that receiver fence and may delete outbound-done/ready/slot
   semaphores;
5. submitter A resumes, reads the result and attempts to signal the now-reclaimed
   outbound-slot semaphore.

A second submitter B can make the ownership gap stronger: B may have passed the
initial terminal admission check before step 2 and already be blocked in
`WaitSema(outbound_slot_semaphore_id)`. Receiver completion currently carries no
proof that such pre-terminal queued submitters have exited.

The returned host test does not prove this impossible. In
`test_receiver_completion_event_is_real_no_touch_fence()` it explicitly calls
`pthread_join(outbound_submit_thread, NULL)` while the receiver is held at the
test-only completion-signal barrier. Only after that join does the test release
the receiver completion barrier. That external test choreography manufactures
the desired submitter-before-completion ordering; product source has no matching
rendezvous.

### R20C criterion disposition

A001-R20C-C1 EARLY_TERMINAL_STATE_IS_NOT_RECLAIM_COMPLETION_AUTHORITY — MET
A001-R20C-C2 WAIT_RECEIVER_DONE_SYNCHRONIZES_AFTER_ALL_TERMINAL_OWNER_WORK — MET_FOR_RECEIVER_OWNER_ONLY
A001-R20C-C3 RELEASE_CANNOT_TERMINATE_OR_DELETE_BEFORE_COMPLETION_FENCE — MET_FOR_RECEIVER_OWNER_ONLY
A001-R20C-C4 PENDING_OUTBOUND_AND_LOGICAL_TERMINAL_WAKES_PRECEDE_RECLAIM — NOT_MET_AS_COMPLETE_TRANSACTION_DRAIN
A001-R20C-C5 FAILED_SOCKET_SHUTDOWN_STILL_CONVERGES_WITHOUT_TIMEOUT_SUCCESS — MET
A001-R20C-C6 PRE_RECLAIM_KERNEL_STATUS_FAILURE_PRESERVES_RETRYABLE_OWNERSHIP — MET
A001-R20C-C7 STACK_SEMAPHORES_QUEUES_AND_STREAM_RECLAIM_ONLY_AFTER_NO_TOUCH_PROOF — NOT_MET
A001-R20C-C8 FRESH_RUNTIME_REUSE_CANNOT_INHERIT_COMPLETION_AUTHORITY — NOT_PROVEN_WHILE_SUBMITTER_CAN_OUTLIVE_RELEASE
A001-R20C-C9 DETERMINISTIC_BARRIER_PROVES_PRE_COMPLETION_RACE_IS_CLOSED — NOT_MET_FOR_OUTBOUND_SUBMITTER
A001-R20C-C10 RFB_AUDIO_MPEG_AND_SINGLE_IO_OWNER_CONTRACTS_UNCHANGED — MET
A001-R20C-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP — MET
A001-R20C-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

Because all twelve criteria were required, R20C is not Foreman-accepted.

### R20C machine evidence retained without promotion

Final R20C source run `35987788623` attempt 1 — SUCCESS.
Immutable-log-head run `35988159385` attempt 1 — SUCCESS.

Both exact heads passed canonical host, project, complete strict dictionaries,
pinned PS2 compile/link and current-source reproducibility. Green machine
evidence does not override the uncovered ownership interleaving.

R20C candidate linked identity is:

`ELF_PRISTINE_SHA256=28965b2409074a034e491d316f1474c214d2fb2fe6167f2c366b7c25fa706f91`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
`PT_LOAD_BYTES=491540`

This is current branch loadable identity but is not accepted or physically
qualified authority.

## Corrective dependency decision

R21 remains unsafe to execute while Transport can reclaim an outbound
rendezvous from beneath a pre-terminal submitting caller. The existing R20C
receiver completion work should be preserved, not reverted; the next packet
extends that completion meaning to include a real outbound-submitter drain.

The correction must cover both:

- the currently active submitter that has been signaled through outbound-done
  but has not yet completed its slot/result/semaphore return path; and
- any caller that crossed admission before terminality and is already queued
  waiting to enter the outbound slot.

No test-only join, sleep, favorable priority assumption, or generic timeout may
stand in for the product drain fence.

## Governing invariants for R20D

1. Early `receiver_done` continues to close new outbound admission before final
   Transport reclaim completion.
2. Receiver completion is reclaim-authoritative only when the sole I/O owner
   has finished its R20C terminal work **and** every outbound submitter that
   crossed pre-terminal admission can no longer touch outbound work or any
   outbound rendezvous semaphore.
3. Signaling `outbound_done_semaphore_id` resolves an item but is not by itself
   proof that the submitting caller has completed its return path.
4. A submitter already blocked on `outbound_slot_semaphore_id` before terminality
   must be allowed/fenced to observe terminal state and leave safely; it may not
   remain asleep on a semaphore that release can delete.
5. No submitter first entering after early terminality may join the drain set or
   touch outbound semaphores.
6. `release()` may delete outbound-done/ready/slot resources only after the
   outbound drain is proven complete.
7. The drain proof must preserve the synchronous one-item outbound queue and
   sole physical-I/O send ownership; no caller may send directly.
8. R20C receiver pending-item failure and RFB/AUDIO/MPEG terminal publication
   ordering remain intact and precede final receiver completion.
9. Failed socket shutdown and pre-reclaim kernel-status failure remain
   fail-closed/retryable with ownership preserved.
10. Fresh runtime reuse has no stale receiver-completion or outbound-submitter
    drain authority.
11. No timeout, delay, scheduling priority assumption, or test-only join may be
    a correctness condition.
12. R18/R20 MPEG semantics, RFB/AUDIO behavior, fixed Wire bytes and all
    neighboring component ownership remain unchanged.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0054
BASED_ON_R20C_CANDIDATE_SOURCE=4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3
BASED_ON_R20C_LOG=3fac8afb2d403f2b96171dfadc3386c8c57da347
BASED_ON_ACCEPTED_R20_SOURCE=3e39753b1b3bce9fe187748deb7eeb4d6201151c

### Objective

Complete the A001 teardown ownership fence by proving that all outbound
submitters admitted before terminality have fully left the outbound rendezvous
before receiver completion becomes reclaim-authoritative.

Preserve the valid R20C receiver-completion correction. Do not execute R21.

### Required behavior

1. **Track the whole submit transaction.** Define one private lifecycle fact or
   equivalent synchronization mechanism that covers a caller from the point it
   can become dependent on outbound rendezvous resources until its final
   outbound-work/semaphore touch is complete.
2. **Admission closes before drain.** Once receiver terminality is published,
   no new caller may enter outbound rendezvous ownership. A caller that already
   crossed the pre-terminal boundary must remain part of the drain proof.
3. **Current pending caller drains.** Terminal pending-item failure must wake the
   current submitter and receiver completion must wait until that caller has
   consumed its result and completed the slot/rendezvous release path.
4. **Queued callers drain.** Deterministically handle callers already queued on
   the outbound slot when terminality closes. They must wake/enter far enough to
   observe terminality and leave all outbound-semaphore ownership, or an
   equivalent product mechanism must prove they can no longer touch reclaimed
   resources.
5. **Receiver completion includes drain.** The final receiver-completion token
   introduced by R20C may be published only after R20C terminal owner work and
   the complete outbound-submitter drain are both true.
6. **Release remains simple consumer of proof.** `release()` may rely on that
   final completion fence; it must not race-delete outbound semaphores or add a
   scheduling-luck workaround.
7. **Multiple observers remain safe.** Preserve the latched/reobservable R20C
   completion behavior for `wait_receiver_done()`, `release()` and retry paths.
8. **Preserve outbound semantics.** Keep one-item synchronous serialization,
   explicit backpressure, one physical sender, pending-result propagation and
   terminal failure semantics.
9. **Preserve retry/fresh reuse.** A failed pre-reclaim kernel operation keeps
   all drain/completion resources valid for retry; successful fresh runtime
   initialization carries no stale submitter count/token/waiter authority.
10. **No false drain by diagnostics.** Counters/events may witness behavior but
    correctness must derive from product synchronization, not logging, sleeps
    or host-only joins.
11. **No scope creep.** Do not modify Application/R21, Pi, protocol bytes, RFB
    parser/session/flow policy, AUDIO product policy, MPEG worker/backend/
    Presentation/calibration or Configuration product behavior.
12. **Evidence/dictionaries.** Add deterministic active-and-queued submitter
    race tests, retain R20C receiver tests, and pass canonical host/project/
    strict-dictionary/pinned-PS2 compile/link/reproducibility evidence.

### Acceptance criteria

- A001-R20D-C1 PRETERMINAL_SUBMITTER_LIFETIME_IS_EXPLICITLY_DRAINED
- A001-R20D-C2 TERMINALITY_PREVENTS_NEW_OUTBOUND_RENDEZVOUS_ADMISSION
- A001-R20D-C3 OUTBOUND_DONE_SIGNAL_IS_NOT_MISTAKEN_FOR_SUBMITTER_COMPLETION
- A001-R20D-C4 ACTIVE_SUBMITTER_EXITS_BEFORE_RECEIVER_COMPLETION
- A001-R20D-C5 QUEUED_OUTBOUND_SLOT_WAITERS_CANNOT_OUTLIVE_COMPLETION
- A001-R20D-C6 RELEASE_CANNOT_DELETE_OUTBOUND_SEMAPHORES_BEFORE_DRAIN
- A001-R20D-C7 R20C_RECEIVER_NO_TOUCH_AND_LOGICAL_TERMINAL_ORDERING_PRESERVED
- A001-R20D-C8 OUTBOUND_SERIALIZATION_BACKPRESSURE_AND_SOLE_SENDER_UNCHANGED
- A001-R20D-C9 FAILED_SHUTDOWN_RETRY_AND_FRESH_RUNTIME_REUSE_REMAIN_SAFE
- A001-R20D-C10 DETERMINISTIC_BARRIERS_PROVE_ACTIVE_AND_QUEUED_RACES_CLOSED
- A001-R20D-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP
- A001-R20D-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

R20D must prove at least:

1. hold one active submitter after outbound-done wake but before its final
   outbound-slot release; allow the receiver to finish every other R20C terminal
   operation and prove receiver completion is still unpublished;
2. release that active submitter and prove final completion may then publish;
3. place a second caller behind the occupied outbound slot **before** terminality
   closes, then prove neither that queued caller nor the active caller can
   outlive final completion or touch reclaimed semaphores;
4. prove a caller beginning after early terminality returns without entering
   outbound rendezvous ownership;
5. while active/queued submitter drain is intentionally held, prove
   `wait_receiver_done()`/release cannot authorize outbound semaphore deletion;
6. after drain and completion, prove any retained post-fence
   TerminateThread/DeleteThread and physical release remain ordered after the
   completion event;
7. retain the R20C pending-outbound, RFB credit waiter and AUDIO/MPEG terminal
   waiter proof;
8. retain failed-shutdown convergence and retryable first
   `ReferThreadStatus()` failure;
9. prove fresh runtime reuse resets both receiver completion and all new
   submitter-drain authority;
10. run the focused synchronization fixture repeatedly without source/delay/
    timeout changes and keep all neighboring Transport/R18/R20/Pi R17 tests
    green.

### Authorized source surface

R20D may modify only the smallest justified subset of:

- `src/transport/runtime.c` / `.h` for private lifecycle/drain state;
- `tests/unit/transport_runtime_test.c` and existing Transport host stubs needed
  for deterministic submitter barriers;
- directly affected Transport lifecycle documentation/dictionaries;
- compile/link/check manifests only if required by a real source dependency.

`src/app.c`, any R21 Application support, `src/app_mpeg_frame.*`, Pi product
source, RFB parser/session/flow-policy product source, MPEG decoder/worker/
backend, Display/Presentation, calibration/Input/UI, AUDIO product policy,
Configuration product source and protocol wire representation are not
authorized.

### Required checks before handoff

Run the focused R20C/R20D completion/outbound-drain fixture repeatedly enough
to prove synchronization rather than race luck, all canonical host tests,
project check, complete strict dictionary audit, pinned PS2 compile/link and
current-source reproducibility. Preserve exact new ELF/PT_LOAD identity if
loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Queued R21 authority

`A003-APPLICATION-MPEG-RUN-START-R21` remains the intended next MPEG dependency
from State 0052, but it is not authorized for execution. Its trigger-agnostic
Application semantics remain unchanged by this review. Foreman will decide
whether to reactivate it only after a complete Transport reclaim fence is
independently accepted.

## Current hardware debt

Last Foreman-accepted product identity remains accepted R20:

`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
`PT_LOAD_BYTES=491540`

Current branch R20C candidate identity is:

`PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
`PT_LOAD_BYTES=491540`

The R20C identity is repository-reproducible but not Foreman-accepted or
physically qualified. Any loadable-byte change from R20D creates a newer
provisional exact hardware-debt identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
