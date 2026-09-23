# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0050
RECORDED_AT=2026-09-23T02:43:07-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0049
SUPERSEDES_FOREMAN_STATE_REVISION=0049
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0050 independently accepts `A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`
at source authority `2e0589fd9fe531c5dfb7130df301b6f0eadd09fd` and consumes its
immutable Reconstruction closeout `ce1d405bde73ee764408c2dddcc40d2f8cc6eccc`.

PS2 Transport now owns the previously missing session-local MPEG run boundary:
explicit clean open/abort, channel-4 DATA admission fencing, exact RETIRE
completion correlation, residual discard, pending-plus-residual credit
finalization, and clean same-session reuse. The final R18 correction also keeps
one MPEG consumer transaction live across dequeue-to-credit-return and blocks
new MPEG consumer/waiter activity while final credit is serialized.

Independent dependency review found that final MPEG orchestration should still
not land directly in the current ordinary Application loop. The accepted A004
RFB freeze/FULL-refresh policy exists as a tested mechanism, but `app.c` still
bypasses it and directly issues one incremental request after every update.
Revision 0050 therefore activates a narrow behavior-preserving Application
composition packet that makes this existing RFB flow policy authoritative in
the real ordinary loop before MPEG start/stop/calibration wiring is added.

R18 changes loadable PS2 bytes and is not hardware-qualified. R19 likewise
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

`A003_R18_MPEG_TRANSPORT_RUN_BOUNDARY_FOREMAN_ACCEPTED__A004_RFB_FLOW_APPLICATION_COMPOSITION_R19_ACTIVE__MPEG_APPLICATION_TRANSACTION_DEPENDENCY_QUEUED`

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
RFB_FLOW_APPLICATION_COMPOSITION=RECONSTRUCTION_ACTIVE
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

## Next dependency decision

The next missing mechanism is not another RFB protocol or Pi suppression path.
A004 already established that the authoritative remote framebuffer may continue
to update beneath an MPEG-owned suppression footprint. Visible suppression is
Presentation ownership. The generic A004 P2 RFB flow policy separately owns
global freeze/thaw, one outstanding request and one-shot post-thaw FULL debt for
calibration and the accept-to-first-frame ownership gap.

That accepted policy is implemented and host-tested in `src/rfb/flow_policy.*`,
but the current ordinary Application loop still bypasses it:

- after initial RFB setup it directly sends an incremental update request;
- every completed update directly triggers another incremental request;
- completed-update accounting is not connected to the P2 policy;
- remote dirty-frame presentation is not gated by the P2 publication decision;
- provider replacement therefore reconstructs no explicit Application-owned P2
  flow state because none is yet composed into the ordinary loop.

Adding MPEG start/stop/calibration on top of that bypass would force one packet
to change RFB request cadence/publication ownership and MPEG lifecycle at the
same time. The smaller dependency is to make the already-accepted P2 policy the
real ordinary-loop authority first, while remaining semantically thawed and
preserving current behavior.

## Governing invariants for R19

1. R19 composes the existing accepted `pstvnc_rfb_flow_policy_t`; it does not
   redesign RFB parsing, Transport, Pi RFB attachment, Presentation or MPEG.
2. One fresh flow-policy value belongs to one fresh ordinary RFB attempt. No
   request-outstanding, freeze or FULL-refresh debt may cross provider/Wire
   replacement.
3. The initial authoritative full desktop acquired by RFB session startup remains
   the existing handshake/startup mechanism. P2 governs only subsequent live
   request cadence/publication.
4. Every subsequent framebuffer update request is selected through
   `pstvnc_rfb_flow_policy_next_request()`. Application must not independently
   choose incremental/full behavior beside that authority.
5. `HOLD` means no framebuffer request is sent. Inspection of next action never
   consumes FULL debt.
6. A successful RFB request send is recorded exactly once through
   `pstvnc_rfb_flow_policy_record_request_sent()`. A failed send cannot be
   converted into successful policy accounting.
7. Every completed live framebuffer-update response is recorded exactly once
   through `pstvnc_rfb_flow_policy_record_update_complete()` before a successor
   request can be admitted.
8. Remote framebuffer parsing/truth remains authoritative even when future
   callers freeze publication. Application presents a completed dirty remote
   frame only when `pstvnc_rfb_flow_policy_allows_remote_publication()` permits
   it.
9. R19 introduces no production freeze trigger. In the ordinary thawed path its
   externally visible behavior remains one incremental request at a time and
   normal dirty-frame presentation.
10. Existing P2 tests remain authoritative for frozen->thawed FULL debt,
    coalescing and in-flight completion while frozen. R19 must not duplicate or
    reinterpret those semantics in Application.
11. Existing R16B typed provider recovery remains intact: a replacement attempt
    gets a fresh thawed flow policy only after old input/Transport owners prove
    stop; generic RFB/Transport failure remains fatal under current policy.
12. R19 does not open MPEG Transport, allocate run identity, enter MPEG
    calibration, arm Presentation, start a decoder/worker, send MPEG START/
    RETIRE, change Pi product source, activate AUDIO, or claim hardware
    qualification.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A004-RFB-FLOW-APPLICATION-COMPOSITION-R19
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0050
BASED_ON_ACCEPTED_R18_SOURCE=2e0589fd9fe531c5dfb7130df301b6f0eadd09fd
BASED_ON_R18_LOG=ce1d405bde73ee764408c2dddcc40d2f8cc6eccc

### Objective

Compose the already-accepted A004 P2 generic RFB flow policy into the real
ordinary Application live loop so it becomes the single authority for
post-startup request cadence, request completion accounting and remote visual
publication permission. Preserve the current thawed RFB-only behavior.

This packet deliberately stops before any MPEG/calibration freeze caller exists.

### Required behavior

1. **Fresh attempt-local flow state.** Initialize one thawed
   `pstvnc_rfb_flow_policy_t` for every newly established RFB attempt after the
   authoritative initial frame is obtained and before ordinary live requests.
2. **Policy-selected request service.** Replace direct unconditional incremental
   request calls with one small Application-local scheduling path that asks P2
   for HOLD/INCREMENTAL/FULL and maps those decisions to the existing
   `pstvnc_rfb_session_request_update()` API.
3. **Record only successful sends.** Advance P2 request accounting only after the
   matching RFB request serialization succeeds. Invalid policy/send sequencing
   fails closed rather than silently repairing counters.
4. **One outstanding response.** After a live UPDATE result, record completion
   exactly once before any next request is considered.
5. **Publication gate.** Keep parsing/framebuffer mutation independent from
   visual publication. A dirty completed remote frame is presented only when P2
   currently allows remote publication.
6. **Ordinary semantics unchanged.** With no R19 freeze caller, the live product
   still issues exactly one incremental request at a time and presents valid
   dirty updates as before.
7. **Idle/HOLD behavior.** IDLE receive and HOLD decisions must not consume
   request debt, fabricate update completion, spin-send duplicate requests, or
   alter provider-recovery semantics.
8. **Fresh provider replacement.** R16B replacement attempts reconstruct fresh
   thawed P2 state and cannot inherit outstanding/debt/freeze authority from the
   failed attempt.
9. **Preserve P2 mechanism authority.** Do not fork freeze/FULL/debt logic into
   `app.c`; use the accepted RFB policy directly. Modify `flow_policy.*` only if
   composition exposes a real defect in its existing public contract.
10. **Preserve neighboring ownership.** RFB session remains parser/protocol
    owner, Transport remains Wire owner, framebuffer remains remote truth,
    graphics/local UI remain current presentation owners, and Application owns
    only cross-domain composition.
11. **No MPEG scope creep.** No MPEG run open/start/retire/finalize, worker/
    decoder/backend, MPEG Presentation, calibration trigger/UI or Pi MPEG/RFB
    product behavior is activated in R19.
12. **Evidence and dictionaries.** Extend Application host regression coverage
    for policy-driven request accounting/publication and provider replacement;
    keep existing P2 tests green; run canonical project/dictionary/PS2 build
    evidence and preserve exact identity if loadable bytes change.

### Acceptance criteria

- A004-R19-C1 FRESH_RFB_ATTEMPT_OWNS_FRESH_THAWED_FLOW_POLICY
- A004-R19-C2 LIVE_REQUESTS_ARE_SELECTED_ONLY_BY_P2_POLICY
- A004-R19-C3 REQUEST_ACCOUNTING_ADVANCES_ONLY_AFTER_SUCCESSFUL_SEND
- A004-R19-C4 COMPLETED_UPDATE_CLEARS_EXACT_OUTSTANDING_REQUEST_BEFORE_NEXT
- A004-R19-C5 REMOTE_VISUAL_PUBLICATION_IS_GATED_BY_P2_POLICY
- A004-R19-C6 ORDINARY_THAWED_INCREMENTAL_BEHAVIOR_IS_PRESERVED
- A004-R19-C7 IDLE_OR_HOLD_DOES_NOT_FABRICATE_REQUEST_OR_COMPLETION
- A004-R19-C8 PROVIDER_REPLACEMENT_CANNOT_INHERIT_FLOW_STATE
- A004-R19-C9 P2_FULL_REFRESH_DEBT_SEMANTICS_REMAIN_SINGLE_AUTHORITY
- A004-R19-C10 RFB_TRANSPORT_FRAMEBUFFER_PRESENTATION_OWNERSHIP_UNCHANGED
- A004-R19-C11 NO_MPEG_CALIBRATION_AUDIO_PI_OR_FINAL_COMPOSITION_SCOPE_CREEP
- A004-R19-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

R19 must prove at least:

1. the first ordinary post-startup request is policy-selected incremental;
2. a successful request creates exactly one outstanding policy obligation;
3. a failed request does not record a successful P2 send;
4. IDLE receive does not clear outstanding request accounting or send another
   request;
5. UPDATE completion clears exactly the outstanding obligation before the next
   request is selected;
6. ordinary thawed dirty UPDATE still presents once and then schedules the next
   incremental request;
7. ordinary clean UPDATE does not re-present but still completes/request-cycles
   correctly;
8. provider-local failure still closes the attempt before replacement and the
   replacement starts with fresh thawed flow state;
9. existing `rfb_flow_policy_test` FULL-debt/coalescing/frozen in-flight cases
   remain green;
10. existing Application R15/R16B lifecycle/input/recovery fixtures remain
    green.

### Authorized source surface

R19 may modify only the smallest justified subset of:

- `src/app.c` and, only if a real public declaration is needed, `src/app.h`;
- `src/rfb/flow_policy.c` / `.h` only for a demonstrated composition-contract
  defect, not redesign;
- `tests/unit/app_test.c`, its current legacy fixture include,
  `tests/unit/rfb_flow_policy_test.c`, and required test build enrollment;
- directly affected Application/RFB dictionaries and development documentation;
- compile/link/check manifests only as required by the source dependency.

Pi product source, Transport product source, RFB parser/session mechanics, MPEG
decoder/worker/backend, MPEG Presentation/compositor/frame consumer, calibration
and local-controller product semantics, AUDIO product source and protocol bytes
are not authorized by R19.

### Required checks before handoff

Run focused Application/RFB flow tests, existing P2 flow-policy tests, canonical
host tests, project check, complete strict source-dictionary audit, pinned PS2
compile/link and current-source reproducibility. Preserve exact new ELF/PT_LOAD
identity if loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

R18 changed the current linked PS2 loadable image to
`PT_LOAD_SHA256=e244769ef21dadd8a09e6fe65ec4b2201e0acd9beb52ae06180ab3d4f3a3a232`
with `PT_LOAD_BYTES=490516`. This exact identity is repository-reproducible but
not physically qualified. R17 Pi MPEG remains source/host/machine-only, and
R16A/R16B recovery remains hardware-unqualified.

Any loadable-byte change from R19 creates a newer exact hardware-debt identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
