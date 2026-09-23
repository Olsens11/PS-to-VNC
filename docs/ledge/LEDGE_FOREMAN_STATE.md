# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0051
RECORDED_AT=2026-09-23T07:21:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0050
SUPERSEDES_FOREMAN_STATE_REVISION=0050
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0051 independently accepts `A004-RFB-FLOW-APPLICATION-COMPOSITION-R19`
at source authority `cd09bc263e5835c75d5a8fea256cd71127e7821b` and consumes its
immutable Reconstruction closeout `1ded79fdfb5f15eb4266a3e997cd445a26c126a2`.

The ordinary Application loop now composes the accepted P2 RFB flow policy as
the single authority for post-startup live request cadence, successful-send
accounting, update-completion accounting and remote-publication permission.
R19 preserves the thawed RFB-only product behavior and R16B provider recovery.

Independent dependency review found one owner-boundary defect before MPEG
Application orchestration: Transport privately owns the Pi-assigned Q4 Wire
`session_id`, but its public MPEG START/RETIRE bridge currently accepts full
wire payload structs that require callers to supply that private identity and
the protocol control version. Application therefore has no owner-correct way
to create exact-session generation control without duplicating or exposing
Transport authority.

Revision 0051 activates a narrow Transport-only packet that moves exact Wire
session/version stamping and completion correlation behind the existing
Transport bridge while preserving the accepted START/RETIRE bytes exactly.

R19 changes the loadable PS2 image and is not hardware-qualified. R20 likewise
carries no hardware-qualification claim.
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

`A004_R19_RFB_FLOW_APPLICATION_COMPOSITION_FOREMAN_ACCEPTED__A003_MPEG_PRIVATE_SESSION_BINDING_R20_ACTIVE__MPEG_APPLICATION_TRANSACTION_DEPENDENCY_QUEUED`

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
MPEG_PRIVATE_SESSION_BINDING=RECONSTRUCTION_ACTIVE
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
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

## Next dependency decision

R19 makes the accepted RFB P2 state machine real in Application, but the next
MPEG transaction still cannot be expressed without violating current
Transport ownership.

Q4 establishment gives the Pi authority to allocate one nonzero Wire
`session_id`. PS2 Transport receives and stores that value in
`pstvnc_transport_bridge_private_session_id`, and the bridge documentation
explicitly states that the Pi session identity is Transport-private and dies
with the physical Wire Session.

However, the current public MPEG bridge functions accept
`pstvnc_mpeg_start_payload_t` and `pstvnc_mpeg_retire_payload_t` directly.
Those are wire-representation structs containing both
`PSTVNC_MPEG_GENERATION_CONTROL_VERSION` and `session_id`. A future Application
caller would therefore have to know or manufacture Transport-private protocol
identity merely to request a generation transition.

That is the wrong dependency direction. It would also make stale-session
correlation an Application convention instead of a Transport invariant.

The smallest prerequisite before an Application MPEG generation transaction is
therefore to keep session identity and wire-control version inside Transport:
Application supplies only run-owned meaning (generation and accepted geometry);
Transport stamps the currently active private Wire identity/version, preserves
the exact already-accepted bytes, and exposes only the exact completion meaning
needed above the bridge.

## Governing invariants for R20

1. Pi remains the sole allocator of Q4 Wire `session_id`; PS2 Transport remains
   the sole owner of the accepted private value.
2. Application, MPEG, Display, calibration and other components must not gain a
   public accessor for the private Wire session ID.
3. `PSTVNC_MPEG_GENERATION_CONTROL_VERSION` is wire representation owned below
   the Transport bridge; callers request MPEG generation behavior, not protocol
   version bytes.
4. The public START request carries only caller-owned exact generation and
   accepted base/suppression geometry. Transport constructs the existing
   44-byte START v1 payload with its current private session ID.
5. The public RETIRE request carries only caller-owned exact generation.
   Transport constructs the existing 12-byte RETIRE v1 payload with the same
   private session ID.
6. Runtime/protocol internals may retain the accepted full wire payload structs;
   this packet changes the cross-component seam, not fixed Wire bytes.
7. Exact RETIRE completion must still match the full submitted wire transaction
   inside Transport. Above the bridge, expose only the minimum exact-generation
   completion fact needed by Application; do not leak session/version merely
   because the wire codec contains them.
8. Inactive Wire, stale/invalid access tickets, zero generation, invalid
   geometry or impossible bridge state fail closed before a control frame is
   admitted.
9. A replacement Q4 Wire Session receives a new private identity. No stale
   access ticket or prior-run request may serialize control with the old
   identity on the replacement session.
10. R18 run-open/retirement/finalization ordering and exact START-before-RETIRE
    constraints remain unchanged.
11. Pi R17 exact-session/exact-generation validation, fixed framing/product
    compatibility, RFB/AUDIO behavior and all accepted neighboring contracts
    remain unchanged.
12. R20 does not allocate Application generation numbers, choose calibration
    geometry, freeze/thaw RFB, start MPEG workers, arm Presentation, change Pi
    product source, activate AUDIO, or perform final MPEG Application
    orchestration.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-MPEG-PRIVATE-SESSION-BINDING-R20
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0051
BASED_ON_ACCEPTED_R19_SOURCE=cd09bc263e5835c75d5a8fea256cd71127e7821b
BASED_ON_R19_LOG=1ded79fdfb5f15eb4266a3e997cd445a26c126a2

### Objective

Refactor the public Transport MPEG generation-control seam so exact Q4 Wire
session identity and generation-control protocol version remain Transport-owned.
Callers provide only exact run meaning; Transport constructs/validates the
already-accepted START/RETIRE wire representation internally.

This packet is a prerequisite for later Application generation orchestration.
It does not activate MPEG in `app.c`.

### Required behavior

1. **Owner-correct START request.** Replace the public requirement for a full
   `pstvnc_mpeg_start_payload_t` with the smallest clear request representation
   containing only nonzero generation plus base/suppression geometry owned by
   the higher-level transaction.
2. **Transport stamps START identity.** At the bridge boundary, construct the
   exact accepted START v1 wire payload from the caller request plus
   `pstvnc_transport_bridge_private_session_id`. Reject if no matching active
   Wire/runtime/access authority exists.
3. **Owner-correct RETIRE request.** Public RETIRE requires only the exact
   nonzero generation; Transport stamps version/private session ID internally.
4. **Minimal completion exposure.** Runtime continues exact full-payload
   correlation. Bridge completion consumption validates internal private
   session/version authority and exposes only the exact generation completion
   meaning required above Transport.
5. **No private identity accessor.** Do not solve composition by adding a public
   `session_id` getter or by copying Q4 identity into Application/Configuration
   mutable state.
6. **No wire change.** START remains exactly 44 bytes/version 1 and RETIRE
   remains exactly 12 bytes/version 1 with the accepted field order and
   endianness. Frame kinds/channels/flags and Q4 product compatibility do not
   change.
7. **Stale-session fencing.** Old access tickets and prior-session requests
   cannot serialize on a replacement session or cause completion from an old
   session to be accepted as current.
8. **Preserve R18 lifecycle.** Run open, pre-START abort, DATA admission,
   retirement latch, residual/credit finalization and fresh successor behavior
   remain exactly governed by accepted R18.
9. **Preserve Pi exactness.** Existing R17 Pi generation controller continues
   to receive the exact session ID it was constructed for and reject mismatches
   without any Pi product-source change.
10. **Clear public naming/types.** Any new cross-component request/completion
    type must describe application meaning rather than wire representation and
    follow current source dictionary/topology rules.
11. **No scope creep.** Do not add generation allocation, calibration/product
    triggers, RFB freeze/thaw, worker/decoder/presentation activation, AUDIO,
    heartbeat, CONFIG-on-Wire or final all-guns orchestration.
12. **Evidence and dictionaries.** Extend focused Transport bridge/runtime
    tests for exact stamping/correlation and replacement-session fencing; keep
    canonical project/dictionary/PS2 build evidence green and preserve exact
    identity if loadable bytes change.

### Acceptance criteria

- A003-R20-C1 PUBLIC_START_REQUEST_CONTAINS_NO_PRIVATE_WIRE_IDENTITY
- A003-R20-C2 TRANSPORT_STAMPS_CURRENT_PRIVATE_SESSION_AND_VERSION_ON_START
- A003-R20-C3 PUBLIC_RETIRE_REQUEST_CONTAINS_ONLY_EXACT_RUN_MEANING
- A003-R20-C4 COMPLETION_REMAINS_FULLY_CORRELATED_INSIDE_TRANSPORT
- A003-R20-C5 PUBLIC_COMPLETION_DOES_NOT_LEAK_PRIVATE_SESSION_OR_VERSION
- A003-R20-C6 NO_PUBLIC_Q4_SESSION_ID_ACCESSOR_OR_DUPLICATE_OWNER
- A003-R20-C7 START_RETIRE_WIRE_BYTES_AND_PRODUCT_COMPATIBILITY_UNCHANGED
- A003-R20-C8 STALE_ACCESS_OR_REPLACEMENT_SESSION_CANNOT_REUSE_OLD_IDENTITY
- A003-R20-C9 R18_RUN_BOUNDARY_AND_FINALIZATION_CONTRACTS_UNCHANGED
- A003-R20-C10 PI_R17_EXACT_SESSION_GENERATION_BEHAVIOR_UNCHANGED
- A003-R20-C11 NO_APPLICATION_CALIBRATION_MEDIA_OR_PI_SCOPE_CREEP
- A003-R20-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

R20 must prove at least:

1. a public START request has no session-id/version field;
2. one established session with a known Q4 acceptance identity serializes an
   exact START payload containing that identity, version 1, caller generation
   and caller geometry;
3. a public RETIRE request serializes exact version/private-session/generation
   bytes through the existing sole Transport sender;
4. zero generation or invalid geometry fails before outbound admission;
5. stale/invalid Transport access fails before START or RETIRE submission;
6. exact matching RETIRE completion is accepted internally and returned above
   the bridge only as the corresponding generation fact;
7. wrong-session/wrong-generation/wrong-version completion remains a Transport
   protocol failure or exact-correlation failure and is never surfaced as a
   valid completion;
8. after Wire Session A is retired and Session B is established, B control
   carries B's private identity and no A ticket/request can serialize;
9. protocol codec tests prove START/RETIRE sizes/bytes are unchanged;
10. R18 MPEG run-boundary/finalization tests and R17 Pi exact-generation tests
    remain green.

### Authorized source surface

R20 may modify only the smallest justified subset of:

- `src/transport/transport.h` for owner-correct cross-component value types if
  needed;
- `src/transport/bridge.c` / `.h`;
- `src/transport/runtime.c` / `.h` only if needed to keep full wire
  representation private below the bridge;
- directly affected Transport protocol/bridge/runtime unit fixtures;
- `src/transport/SYMBOLS.md`, generated dictionaries and directly affected
  development/architecture notes;
- compile/link/check manifests only if required by the source change.

`src/app.c`, `src/app_mpeg_frame.*`, Pi product source, RFB product source,
MPEG decoder/worker/backend, Display/Presentation, calibration/Input/UI, AUDIO
product source, Configuration product source and fixed protocol codec bytes are
not authorized by R20.

### Required checks before handoff

Run focused Transport bridge/runtime MPEG control tests, protocol codec tests,
accepted R18 MPEG run-boundary tests, R17 Pi generation tests, canonical host
tests, project check, complete strict source-dictionary audit, pinned PS2
compile/link and current-source reproducibility. Preserve exact new ELF/PT_LOAD
identity if loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

R19 changed the current linked PS2 loadable image to
`PT_LOAD_SHA256=47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`
with `PT_LOAD_BYTES=491156`. This exact identity is repository-reproducible but
not physically qualified. R18 Transport, R17 Pi MPEG, and R16A/R16B recovery
remain hardware-unqualified at their respective reconstructed identities.

Any loadable-byte change from R20 creates a newer exact hardware-debt identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
