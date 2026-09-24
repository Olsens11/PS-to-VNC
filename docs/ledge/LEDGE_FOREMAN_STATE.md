# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0052
RECORDED_AT=2026-09-24T00:13:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0051
SUPERSEDES_FOREMAN_STATE_REVISION=0051
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0052 independently accepts `A003-MPEG-PRIVATE-SESSION-BINDING-R20`
at source authority `3e39753b1b3bce9fe187748deb7eeb4d6201151c` and consumes its
immutable Reconstruction closeout `0a81114fb1c8691fb677fed343cd45bfbae24487`.

Transport now keeps Q4 Wire identity and MPEG control-version representation
below its public bridge. Higher owners express only exact generation/geometry
meaning, while Transport stamps the current private session identity and keeps
full RETIRE correlation private before projecting only completed generation.

Independent dependency review found that ordinary `app.c` still cannot safely
activate MPEG yet: the maintained Pi runtime composes RFB only, the historical
physical calibration trigger is explicitly discarded, and no Application owner
yet sequences an exact local run from accepted geometry through worker,
Presentation, frame-consumer and START readiness.

Revision 0052 therefore activates a trigger-agnostic Application run-start
transaction packet. It creates no product trigger and is not wired into the
ordinary Application loop until retirement/restoration is separately accepted.

R20 changes loadable PS2 bytes and is not hardware-qualified. R21 likewise
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

`A003_R20_MPEG_PRIVATE_SESSION_BINDING_FOREMAN_ACCEPTED__A003_APPLICATION_MPEG_RUN_START_R21_ACTIVE__RETIREMENT_AND_PRODUCT_ACTIVATION_DEFERRED`

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
APPLICATION_MPEG_RUN_START=RECONSTRUCTION_ACTIVE
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

## Next dependency decision

R20 removes the last Transport-identity leak, but ordinary MPEG product
activation is still not the next bounded step.

Current dependency facts:

- `app.c` opens an RFB-only Transport runtime; channel-4 product activation is
  not yet composed there;
- `pi/wire_runtime.py` intentionally composes only the RFB attachment; R17's
  MPEG generation controller remains dormant unless a factory is injected;
- the Pi MPEG controller requires product composition values such as active
  desktop/display and retirement policy that are not yet selected by current
  ordinary Pi runtime authority;
- A005 explicitly discards the historical START+SELECT calibration chord as a
  product binding, so Foreman will not invent a replacement physical trigger;
- accepted A003/A004 mechanisms already provide worker/backend/runtime,
  Presentation, frame consumer, RFB flow policy, selected MPEG profile and
  Transport run/control seams.

The smallest coherent next dependency is therefore a trigger-agnostic
Application-owned run-start process. It accepts already-resolved calibration
geometry and already-existing session/component authorities, allocates one
session-local exact generation, proves every local start prerequisite, and
submits START only after all reversible local setup is complete.

R21 is deliberately not invoked by ordinary `app.c`. A successful R21 start
would establish a live run that still needs separately reconstructed
retirement/restoration. Product activation must wait until that complement and
Pi runtime composition are both accepted.

## Governing invariants for R21

1. Application owns exact run generation identity; Transport, MPEG worker,
   Presentation and Pi consume but do not allocate it.
2. One R21 coordinator instance belongs to one current MPEG-capable Transport
   session/attempt. Generation values are monotonically increasing nonzero
   uint32 values within that owner; allocated attempt identities are never
   reused, and exhaustion fails closed instead of wrapping.
3. R21 receives one already-resolved `pstvnc_mpeg_calibration_geometry_t` or
   equivalently narrow accepted geometry value. It does not edit calibration
   and must not create a second geometry authority.
4. The exact same base rectangle reaches Presentation and Transport START; the
   exact same suppression rectangle reaches both. Inner matte remains
   Presentation-only and never enters START.
5. Geometry must fit selected decoder/profile bounds before worker/START
   activation. R21 must not move Pi desktop bounds or calibration policy into
   Transport.
6. The caller's RFB P2 state must already deny remote publication before the
   start transaction can proceed. R21 does not invent a physical calibration
   trigger or silently steal/release another owner's freeze obligation.
7. Successful startup orders reversible local prerequisites before START:
   Transport clean run-open; fresh PS2 worker runtime/backend operations;
   exact-generation worker start; Presentation WAIT_FIRST_FRAME arm; P7 frame
   consumer initialization; then exact START as the final irreversible action.
8. Before START is attempted, any failure must unwind only owners actually
   acquired, prove worker/thread/resource retirement where applicable, abort
   Presentation WAIT_FIRST_FRAME if armed, and call Transport pre-START abort.
   A failed unwind is fatal/faulted authority, never a successful rollback.
9. Once START submission is attempted, a non-success result may represent
   uncertain physical exposure. R21 must not call pre-START abort and claim a
   clean reusable run; it records fault/session-teardown-required state.
10. First physical frame promotion, P2 thaw, live frame service, RETIRE, worker
    stop/join/release, Transport finalization, FULL restoration and reveal are
    outside R21 and remain later Application lifecycle work.
11. R21 does not change Pi product source/runtime composition, ordinary `app.c`
    behavior, AUDIO, protocol bytes, RFB parser/session, calibration input
    bindings, Presentation internals, worker internals or Transport internals.
12. The new process must be directly host-testable with deterministic owner
    failure injection and preserve canonical project/dictionary/PS2 evidence.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-APPLICATION-MPEG-RUN-START-R21
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0052
BASED_ON_ACCEPTED_R20_SOURCE=3e39753b1b3bce9fe187748deb7eeb4d6201151c
BASED_ON_R20_LOG=0a81114fb1c8691fb677fed343cd45bfbae24487

### Objective

Implement one trigger-agnostic Application-owned MPEG run-start transaction that
binds accepted calibration geometry and selected MPEG mechanism policy to the
accepted Transport/worker/Presentation/frame-consumer seams, allocates exact
session-local generation identity, and makes START the final irreversible
startup action.

Do not wire this transaction into the ordinary Application loop in R21.

### Required behavior

1. **Application run owner.** Add the smallest coherent Application-owned run
   state/process representation. A new root `src/app_mpeg_run.*` pair is
   permitted because `src/` is the Application coordinator responsibility and
   this is cross-domain Application orchestration, not a new feature domain.
2. **Fresh session-local identity.** Initialization establishes idle run state
   and monotonic nonzero generation allocation. No generation may be reused
   after allocation, including a locally failed pre-START attempt; wrap/exhaustion
   fails closed.
3. **Accepted geometry mapping.** Consume one resolved calibration geometry
   value. Map base/inner/suppression exactly into P3 Presentation geometry and
   map only base/suppression into the R20 START request. Prove signed/nonempty/
   containment/alignment/profile-bound validity before activation.
4. **Existing protection required.** Refuse startup unless the supplied P2 flow
   policy currently denies remote publication. Do not set or clear the freeze
   in R21.
5. **Transport access/run-open.** Require current caller-supplied Transport
   access and open one clean R18 run before the MPEG worker can consume channel
   state.
6. **Fresh MPEG execution owners.** Initialize one fresh R5 PS2 worker runtime,
   obtain its decoder/worker operation tables, initialize one fresh R3 PS2
   decoder backend, obtain platform operations, and start one R4 worker with
   the allocated generation and selected R7 decoder/worker/runtime policy.
7. **Presentation/frame consumer.** Arm P3 WAIT_FIRST_FRAME with the exact same
   generation/geometry, then initialize P7 frame consumer against that worker,
   Presentation, caller-owned media clock and selected scheduler profile.
8. **START last.** Only after all prior steps succeed may R21 submit the R20
   semantic START request. Successful submission establishes one
   `STARTED_WAIT_FIRST_FRAME` transaction state.
9. **Pre-START unwind.** For every injected failure before START invocation,
   unwind in reverse ownership order. Release any P7 claim/state that needs no
   separate ownership; abort pending Presentation if armed; request/verify
   worker stop/join/outcome/release as required; release R5 runtime resources;
   abort R18 run-open. Report cleanup failure distinctly and leave the owner
   faulted rather than claiming idle.
10. **START-attempt failure fence.** If the START call itself returns non-OK,
    mark the transaction/session as faulted/teardown-required. Do not use
    `pstvnc_transport_mpeg_run_abort_pre_start()` after START invocation merely
    because the call returned failure.
11. **No downstream lifecycle scope.** Do not service frames, thaw P2, begin
    Presentation retirement, send RETIRE, mark producer done, stop/join a live
    successful worker, finalize Transport, schedule FULL refresh, reveal RFB,
    or activate the Pi product runtime.
12. **No product trigger/ordinary activation.** Do not modify ordinary
    `pstvnc_app_run*()` behavior to invoke R21 and do not choose a controller/
    keyboard/UI gesture for MPEG calibration/start.

### Acceptance criteria

- A003-R21-C1 APPLICATION_OWNS_MONOTONIC_SESSION_LOCAL_RUN_GENERATION
- A003-R21-C2 ACCEPTED_GEOMETRY_HAS_ONE_BASE_INNER_SUPPRESSION_AUTHORITY
- A003-R21-C3 START_BASE_AND_SUPPRESSION_MATCH_PRESENTATION_SNAPSHOT_EXACTLY
- A003-R21-C4 EXISTING_RFB_PROTECTION_IS_REQUIRED_NOT_STOLEN_OR_RELEASED
- A003-R21-C5 R18_RUN_OPEN_PRECEDES_MPEG_WORKER_CONSUMER_ACTIVITY
- A003-R21-C6 FRESH_R5_R3_R4_EXECUTION_OWNERS_BIND_EXACT_GENERATION
- A003-R21-C7 P3_WAIT_FIRST_FRAME_AND_P7_CONSUMER_READY_BEFORE_START
- A003-R21-C8 START_IS_FINAL_IRREVERSIBLE_STARTUP_ACTION
- A003-R21-C9 EVERY_PRE_START_FAILURE_PROVES_REVERSE_ORDER_UNWIND_OR_FAULTS
- A003-R21-C10 START_ATTEMPT_FAILURE_NEVER_FALSELY_USES_PRE_START_ABORT
- A003-R21-C11 NO_RETIREMENT_PI_TRIGGER_AUDIO_OR_ORDINARY_APP_SCOPE_CREEP
- A003-R21-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

R21 must prove at least:

1. fresh coordinator starts idle with generation 0/current-none and allocates
   generations 1,2,... without reuse across pre-START failed attempts;
2. UINT32 exhaustion fails closed and never wraps to zero;
3. invalid/unprotected geometry or thawed P2 fails before Transport run-open;
4. exact base/suppression values observed by R20 START equal the P3 snapshot;
5. inner matte reaches Presentation but not START;
6. run-open precedes worker start; worker start precedes Presentation/P7 ready;
   START is after all of them;
7. failures at run-open, runtime init/ops, backend ops, worker start,
   Presentation arm and P7 init produce the required reverse-order cleanup and
   no START invocation;
8. cleanup failure leaves explicit faulted state and does not permit another
   start on the same coordinator;
9. START success leaves exact generation in WAIT_FIRST_FRAME-ready state with
   no retirement/thaw/reveal side effect;
10. START invocation failure performs no pre-START abort and requires outer
    session teardown;
11. existing P2, P3, P7, R18, R20 and worker/runtime/backend focused tests
    remain green;
12. ordinary Application R15/R16B/R19 tests remain unchanged/green, proving
    R21 is not yet product-invoked.

### Authorized source surface

R21 may modify only the smallest justified subset of:

- new `src/app_mpeg_run.c` / `.h` Application-coordinator support files;
- directly required Application test/build enrollment, preferably a focused
  `tests/unit/app_mpeg_run_test.c` rather than expanding legacy `app_test`;
- `src/SYMBOLS.md`, generated dictionaries and directly affected development
  documentation;
- compile/link/check manifests required to compile/link the new Application
  coordinator source.

Existing `src/app_mpeg_frame.*`, RFB P2, Presentation/Display, calibration,
MPEG worker/backend/runtime, Transport, Configuration and Pi product source may
not be modified unless a concrete integration-contract defect is demonstrated;
if such a defect is found, stop and return BLOCKED rather than broadening R21.

`src/app.c` ordinary lifecycle behavior is not authorized for R21.

### Required checks before handoff

Run focused R21 host tests plus existing P2/P3/P7/R18/R20/worker/runtime/backend
regressions, canonical host tests, project check, complete strict dictionary
audit, pinned PS2 compile/link and current-source reproducibility. Preserve exact
new ELF/PT_LOAD identity if loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

R20 changed the current linked PS2 loadable image to
`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
with `PT_LOAD_BYTES=491540`. This exact identity is repository-reproducible but
not physically qualified. R19/R18 and earlier reconstructed hardware-facing
work remain unqualified at their respective identities.

Any loadable-byte change from R21 creates a newer exact hardware-debt identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
