# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0063
RECORDED_AT=2026-09-25T09:09:26-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0062
SUPERSEDES_FOREMAN_STATE_REVISION=0062
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0063 independently accepts
`A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9` at final source authority
`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5` and consumes immutable
Reconstruction closeout `af2c897a083442443345358577ddd63df7aee9ac`.

P9 establishes the trigger-agnostic Application foreground safety transaction
around manual MPEG CALIBRATION. It freezes P2 before local ownership, suspends
mouse interpretation, neutralizes/rebases exact published pointer state,
snapshots the last presented ordinary desktop into caller-owned storage,
presents P8 calibration from a separate work surface, waits for release
quarantine, physically restores the frozen desktop before mouse resume, and
then diverges cleanly: cancel thaws with normal FULL-refresh debt while accept
retains exact resolved MPEG presentation geometry in ACCEPTED_PROTECTED with P2
still frozen and P3 still RFB_ONLY.

The next dependency is the exact ownership-transfer transaction from that
protected accepted endpoint into the already-accepted R21 Application MPEG run
start. This handoff must preserve one continuous P2 protection interval, make
pre-START rollback distinguishable from irreversible/uncertain START failure,
and retire P9's protected-calibration authority only after a successful exact
WAIT_FIRST_FRAME run is proven. It still does not choose a product trigger or
wire the ordinary main loop.

P9 is now enrolled in the linked PS2 image and changes exact loadable identity;
that identity is reproducible but remains hardware-pending.

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

`A004_P9_APPLICATION_CALIBRATION_FOREGROUND_FOREMAN_ACCEPTED__A004_P10_PROTECTED_START_HANDOFF_ACTIVE__ORDINARY_MPEG_PRODUCT_ACTIVATION_DEFERRED`

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
APPLICATION_MPEG_RETIREMENT=FOREMAN_ACCEPTED
APPLICATION_MPEG_Q7_RESTORE_OVERLAP=FOREMAN_ACCEPTED
APPLICATION_MPEG_FINAL_REVEAL=FOREMAN_ACCEPTED
MPEG_CALIBRATION_REGION_SOURCE=FOREMAN_ACCEPTED
APPLICATION_MPEG_CALIBRATION_FOREGROUND=FOREMAN_ACCEPTED
APPLICATION_MPEG_PROTECTED_START_HANDOFF=RECONSTRUCTION_ACTIVE
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

## Accepted corrected R23/R23C retirement authority

R23_PACKET_ID=A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23
R23C_PACKET_ID=A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C
PACKET_STATUS=FOREMAN_ACCEPTED_AS_CORRECTED_COMBINED_TRANSACTION
R23_ASSIGNING_FOREMAN_STATE_REVISION=0058
R23C_ASSIGNING_FOREMAN_STATE_REVISION=0059
R23_FINAL_SOURCE_COMMIT=3b4087d7b65992f0c3edda5fcf34a627ad6f1d28
R23_RECONSTRUCTION_LOG_COMMIT=f8d36483a5b8c3c231b79417bc9cf86a6faa3890
R23C_FINAL_SOURCE_COMMIT=da091d1e1fcb2398bd695cae1516c4730213cf55
R23C_RECONSTRUCTION_LOG_COMMIT=842625885cfda96c5e068cba9dff90415190c7ad
R23C_PRE_LOG_COMMIT_COUNT=5

The required R23C immutable Reconstruction record is:

`docs/ledge/work-log/20260924T202745-0400__reconstruction__a003-mpeg-generation__interactive.md`

### Corrected criterion disposition

A003-R23C-C1=MET
A003-R23C-C2=MET
A003-R23C-C3=MET
A003-R23C-C4=MET
A003-R23C-C5=MET
A003-R23C-C6=MET
A003-R23C-C7=MET
A003-R23C-C8=MET
A003-R23C-C9=MET
A003-R23C-C10=MET
A003-R23C-C11=MET
A003-R23C-C12=MET

The prior R23 C1-C9/C12 safe-stop evidence remains accepted as corrected by
R23C. State 0058's invalid P2-frozen C10/C11 wording is superseded by the R23C
Q7 criteria above and is not current architecture.

Independent Foreman findings:

1. normal retirement admission still requires exact healthy MPEG_OWNED
   Application/P3/P7/R4/R5/R18 ownership and pre-existing P2 freeze;
2. exact P3 RETIRING precedes the one irreversible current-generation RETIRE;
3. RETIRE failure leaves P2 frozen and does not manufacture Q7 restoration;
4. only successful RETIRE serialization invokes P2's real
   `pstvnc_rfb_flow_policy_set_frozen(..., 0)` transition;
5. real P2 semantics create FULL debt when no request is outstanding and HOLD
   when a genuine prior request remains in flight;
6. R23C sends no RFB request and records no fabricated update completion;
7. P3 stays exact-generation RETIRING and P7 continues IDLE/WAIT/PRESENTED/
   DROPPED drain after P2 is thawed;
8. unexpected re-freeze fails retirement service closed before lower retirement
   progress is consumed;
9. exact RETIRE completion still gates producer-done, which gates natural
   worker completion/no-borrow proof, exact join/COMPLETED outcome, P7/R4/R5
   reclaim and finally R18 finalization;
10. post-thaw failures never re-freeze P2 or manufacture restoration rollback;
11. success is RESTORE_PENDING with exact generation/P3 snapshot retained and
    P2 thawed/publication-enabled;
12. ordinary `src/app.c`, RFB/P3/P7/MPEG/Transport lower-owner implementation,
    Pi product source, protocol bytes, AUDIO, Input/UI and calibration source
    were not modified.

The focused Application fixture now links the real accepted
`src/rfb/flow_policy.c`, making the Q7 FULL/HOLD proof exercise actual P2
accounting rather than a publication-only stub.

### Exact R23C machine evidence

At exact final source authority
`da091d1e1fcb2398bd695cae1516c4730213cf55`, GitHub Actions run
`36078318661`, attempt 1, completed SUCCESS.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed host evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Observed project/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=202 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=c5fc16d2b958dd9689697ea6c15556da59d2c440539cedbcc3bfab33be4e3704`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c7443e28a6eb4a6580a153768e816d4011b2b2784dabb8ef274954cb36c081fa`
`PT_LOAD_BYTES=498964`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R23C_SOURCE_COMPLETE=YES
R23C_HOST_TESTED=YES
R23C_PROJECT_CHECK=PASS
R23C_STRICT_DICTIONARIES=PASS
R23C_PS2_COMPILE=PASS
R23C_PS2_LINK=PASS
R23C_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R23C_MACHINE_EVIDENCE=GITHUB_ACTIONS
R23C_INDEPENDENT_VALIDATION=NOT_RUN
R23C_OPERATOR_OBSERVED=NO
R23C_HARDWARE_QUALIFIED=NO
R23C_HARDWARE_PENDING=YES

No workflow/status record was exposed for the later immutable Reconstruction-log
head at Foreman review time, so no log-head CI result is manufactured.

## Accepted R24 final restored-RFB reveal authority

PACKET_ID=A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0060
ASSIGNING_FOREMAN_STATE_COMMIT=3ceb16b59d73535244d6afb2b42a9485ba04580b
ASSIGNING_FOREMAN_LOG_COMMIT=5b329dd46066602cd95eb1813be226395d3d07a0
RECONSTRUCTION_STARTING_COMMIT=5b329dd46066602cd95eb1813be226395d3d07a0
R24_FINAL_SOURCE_COMMIT=d777835c53a0e829e423f76bd0f78097c88e0ffb
R24_RECONSTRUCTION_LOG_COMMIT=2f4a03ff38f851b6d9f73ab5f5305e6751e532d5
R24_PRE_LOG_COMMIT_COUNT=6

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260924T205847-0400__reconstruction__a003-mpeg-generation__interactive.md`

### R24 criterion disposition

A003-R24-C1=MET
A003-R24-C2=MET
A003-R24-C3=MET
A003-R24-C4=MET
A003-R24-C5=MET
A003-R24-C6=MET
A003-R24-C7=MET
A003-R24-C8=MET
A003-R24-C9=MET
A003-R24-C10=MET
A003-R24-C11=MET
A003-R24-C12=MET

Independent Foreman findings:

1. final restoration admission requires exact RESTORE_PENDING/REVEAL_PENDING
   state plus the complete R23C execution-retired fact set;
2. `rfb_restoration_presented` is an explicit run-scoped Application fact and
   is not inferred from P2 protocol state;
3. the marker can be recorded only after real P2 post-thaw FULL completion:
   thawed/publication-enabled, no outstanding request and next request
   INCREMENTAL;
4. the API contract explicitly requires the corresponding authoritative
   FULL-refreshed desktop to have crossed the existing successful graphics
   presentation/upload boundary before the caller records the marker;
5. FULL owed, request outstanding, frozen P2, absent marker, wrong generation or
   fabricated retired-execution state prevents seal/reveal;
6. exact P3 RETIRING -> REVEAL_PENDING seal occurs only after protocol freshness
   plus the graphics-presentation proof;
7. Application records explicit REVEAL_PENDING after independently verifying P3;
8. physical handoff calls only
   `pstvnc_mpeg_compositor_reveal_retired()`; Application does not call
   Platform reveal or P3 commit directly;
9. compositor PLATFORM_FAILED and SYNC_INVALID are retryable pre-sync outcomes:
   generation, proof and exact P3 REVEAL_PENDING snapshot remain, no reseal is
   performed, and session teardown is not demanded merely for those outcomes;
10. nonretryable compositor/state/effects contradictions fail closed without
    pretending IDLE;
11. successful completion requires synchronized + retirement_revealed effects,
    exact P3 RFB_ONLY, no retained snapshot and no MPEG visual ownership;
12. exact success clears only current-run/transient coordinator state, preserves
    `last_allocated_generation`, leaves external P2 thawed and does not mutate
    the external session media-clock object;
13. focused evidence proves the same coordinator allocates generation N+1 after
    the external caller re-establishes the existing frozen-P2 start
    precondition;
14. ordinary `src/app.c`, lower-owner RFB/P3/compositor/Platform/P7/MPEG/
    Transport implementation, Pi product source, AUDIO, Input/UI/calibration
    behavior and Wire protocol were untouched.

### Exact R24 machine evidence

At exact final source authority
`d777835c53a0e829e423f76bd0f78097c88e0ffb`, GitHub Actions run
`36080636735`, attempt 1, completed SUCCESS.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed host evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Observed project/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=204 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=d9fa5af6749f21f4a7e5ab6dc727bebb66c584c37bc487a4c909b96abcdfb549`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R24_SOURCE_COMPLETE=YES
R24_HOST_TESTED=PASS
R24_PROJECT_CHECK=PASS
R24_STRICT_DICTIONARIES=PASS
R24_PS2_COMPILE=PASS
R24_PS2_LINK=PASS
R24_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R24_MACHINE_EVIDENCE=GITHUB_ACTIONS
R24_INDEPENDENT_VALIDATION=NOT_RUN
R24_OPERATOR_OBSERVED=NO
R24_HARDWARE_QUALIFIED=NO
R24_HARDWARE_PENDING=YES

No workflow/status record was exposed for the later immutable Reconstruction-log
head at Foreman review time, so no log-head CI result is manufactured.

## Accepted P8 manual MPEG CALIBRATION region-source authority

PACKET_ID=A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0061
ASSIGNING_FOREMAN_STATE_COMMIT=4a62a8fd0d33ad4530247b05e7e2a8d2a862c9b1
ASSIGNING_FOREMAN_LOG_COMMIT=7e7253a12626deb6f845a03cbb860318b3d39c73
RECONSTRUCTION_STARTING_COMMIT=7e7253a12626deb6f845a03cbb860318b3d39c73
P8_FINAL_SOURCE_COMMIT=40841daec820c8be01ba2fbaa93d954dea43b506
P8_RECONSTRUCTION_LOG_COMMIT=5945eb7972ca1dd6c7b913275306ecc7b19f36ad
P8_PRE_LOG_COMMIT_COUNT=8

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T035017-0400__reconstruction__a004-presentation__interactive.md`

### P8 criterion disposition

A004-P8-C1=MET
A004-P8-C2=MET
A004-P8-C3=MET
A004-P8-C4=MET
A004-P8-C5=MET
A004-P8-C6=MET
A004-P8-C7=MET
A004-P8-C8=MET
A004-P8-C9=MET
A004-P8-C10=MET
A004-P8-C11=MET
A004-P8-C12=MET

Independent Foreman findings:

1. P8 exposes only explicit begin; no START+SELECT entry detector, hold timer or
   libpad polling entered clean product source;
2. normalized controller mapping is limited to the accepted P1 actions:
   D-pad resize, R1 move, R2 inner matte, L2 outer matte, START reset, CROSS
   review/accept and CIRCLE cancel;
3. calibration-owned EDIT/REVIEW/release-quarantine samples publish an explicit
   consume-controller fact and inactive samples are not claimed;
4. the accepted result is propagated only from P1's exact release/re-press edge
   and carries a value copy independent of later committed-state mutation;
5. cancel emits no accepted region and release quarantine remains owned until
   the all-released proof;
6. visual-plan geometry is copied from the P1 candidate and resolved only through
   `pstvnc_mpeg_calibration_resolve_geometry()`;
7. base, inner-content and suppression meanings remain distinct and no DESKTOP
   CALIBRATION geometry authority is introduced;
8. the caller-owned raster validates plan/capacity/non-overlap before writing,
   leaves the frozen input desktop byte-identical, and derives drawing only from
   the resolved plan geometry;
9. clean P8 intentionally omits the H1-only CONTROLS state and adapter-stack
   proliferation while preserving the accepted P1 edit/review semantics;
10. P8 has no RFB, input-runtime, ordinary Application, Transport, MPEG
    execution, Display/Platform, Pi, AUDIO, DESKTOP CALIBRATION or Wire behavior;
11. the accepted output is the neutral P1 region value, so future non-manual
    region sources remain architecturally possible;
12. the complete final source range stays within the authorized UI/test/docs/
    dictionary surface.

### Exact P8 machine evidence

At exact final source authority
`40841daec820c8be01ba2fbaa93d954dea43b506`, GitHub Actions run
`36110790554`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `40841daec820c8be01ba2fbaa93d954dea43b506`
and title `test: verify final P8 manual MPEG calibration authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed focused host evidence:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=206 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=d9fa5af6749f21f4a7e5ab6dc727bebb66c584c37bc487a4c909b96abcdfb549`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This is exactly the already-accepted R24 loadable identity.

P8_SOURCE_COMPLETE=YES
P8_HOST_TESTED=PASS
P8_PROJECT_CHECK=PASS
P8_STRICT_DICTIONARIES=PASS
P8_PS2_COMPILE=PASS
P8_PS2_LINK=PASS
P8_CURRENT_SOURCE_REPRODUCIBILITY=PASS
P8_LOADABLE_BYTES_CHANGED=NO
P8_MACHINE_EVIDENCE=GITHUB_ACTIONS
P8_INDEPENDENT_VALIDATION=NOT_RUN
P8_OPERATOR_OBSERVED=NO
P8_HARDWARE_QUALIFIED=NO_NEW_CLAIM
P8_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted P9 Application MPEG CALIBRATION foreground authority

PACKET_ID=A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0062
ASSIGNING_FOREMAN_STATE_COMMIT=197183e1ef078be69cd3e20ec9fd63e9b66e45f7
ASSIGNING_FOREMAN_LOG_COMMIT=b1a6ed31207d65ce5cd2a717a5bef47129f778ad
RECONSTRUCTION_STARTING_COMMIT=b1a6ed31207d65ce5cd2a717a5bef47129f778ad
P9_FINAL_SOURCE_COMMIT=b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5
P9_RECONSTRUCTION_LOG_COMMIT=af2c897a083442443345358577ddd63df7aee9ac
P9_PRE_LOG_COMMIT_COUNT=9

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T065836-0400__reconstruction__a004-presentation__interactive.md`

### P9 criterion disposition

A004-P9-C1=MET
A004-P9-C2=MET
A004-P9-C3=MET
A004-P9-C4=MET
A004-P9-C5=MET
A004-P9-C6=MET
A004-P9-C7=MET
A004-P9-C8=MET
A004-P9-C9=MET
A004-P9-C10=MET
A004-P9-C11=MET
A004-P9-C12=MET

Independent Foreman findings:

1. admission is exact IDLE/manual-inactive/DESKTOP/no-local-quarantine/P3
   RFB_ONLY/no-snapshot/P2-thawed authority and rejects existing protected
   acceptance without rewriting it;
2. begin orders P2 freeze before mouse suspend, optional exact pointer release,
   suspended-state rebase, desktop snapshot, P8 begin, raster and local
   presentation;
3. published click authority becomes neutral only after successful RFB pointer
   release serialization; failure leaves the old click fact intact and contains
   the transaction frozen/suspended;
4. the caller's last-presented desktop is copied into dedicated frozen storage;
   calibration rendering writes only a distinct work surface and never owns RFB
   framebuffer storage;
5. visible EDIT/REVIEW service uses the accepted Platform desktop presentation
   seam and presentation failure fails closed with P2/input protection retained;
6. a P8 sample not consumed while P9 claims ACTIVE foreground is treated as a
   contradiction rather than being leaked into ordinary local-controller/OSK
   routing;
7. P8's accepted region is resolved only through P1
   `pstvnc_mpeg_calibration_resolve_geometry()`, then base/inner/suppression
   are copied field-for-field into P3 geometry form;
8. accepted/cancel edges latch outcome while release quarantine keeps P2 frozen
   and mouse interpretation suspended;
9. exact frozen ordinary desktop restoration precedes mouse resume;
10. cancel then thaws once and preserves P2 FULL/HOLD accounting; accept instead
    enters ACCEPTED_PROTECTED with exact geometry, P2 frozen and P3 exact
    RFB_ONLY;
11. accepted geometry read is side-effect-free; explicit abort alone releases
    ACCEPTED_PROTECTED without MPEG start and creates normal P2 FULL debt;
12. no ordinary `src/app.c`, app_mpeg_run call, permanent trigger, active-MPEG
    recalibration, lower-owner mechanism, Pi product, AUDIO, DESKTOP CALIBRATION
    or Wire/protocol behavior entered P9.

### Exact P9 machine evidence

At exact final source authority
`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5`, GitHub Actions run
`36128369370`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5`,
branch `ledge/h1-all-guns`, push event and title
`test: verify final P9 calibration foreground authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed focused/regression host evidence includes:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19 tests: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=208 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=aad95787527a9668a9cceb29affb25dbf1eb546f14c739fafae1d1d07830ecf5`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=b57964cb50464bcbe8239865f7d54d890e0fffe9d5bf4479b79d178c0293270a`
`PT_LOAD_BYTES=510996`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

P9_SOURCE_COMPLETE=YES
P9_HOST_TESTED=PASS
P9_PROJECT_CHECK=PASS
P9_STRICT_DICTIONARIES=PASS
P9_PS2_COMPILE=PASS
P9_PS2_LINK=PASS
P9_CURRENT_SOURCE_REPRODUCIBILITY=PASS
P9_LOADABLE_BYTES_CHANGED=YES
P9_MACHINE_EVIDENCE=GITHUB_ACTIONS
P9_INDEPENDENT_VALIDATION=NOT_RUN
P9_OPERATOR_OBSERVED=NO
P9_HARDWARE_QUALIFIED=NO
P9_HARDWARE_PENDING=YES
P9_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A004-APPLICATION-MPEG-PROTECTED-START-HANDOFF-P10
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0063
BASED_ON_ACCEPTED_P9_SOURCE=b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5
BASED_ON_P9_LOG=af2c897a083442443345358577ddd63df7aee9ac
BASED_ON_ACCEPTED_R24_SOURCE=d777835c53a0e829e423f76bd0f78097c88e0ffb
BASED_ON_ACCEPTED_R21_RUN_START=FOREMAN_ACCEPTED
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007

### Objective

Compose one exact trigger-agnostic Application ownership transfer from P9
`ACCEPTED_PROTECTED` into the already-accepted R21
`pstvnc_app_mpeg_run_start()` transaction.

P10 must preserve one continuous P2 freeze from calibration acceptance through
MPEG WAIT_FIRST_FRAME, retire the P9 protected-calibration authority only after a
successful exact run start is independently proven, and distinguish clean
pre-START rollback from any uncertain/irreversible start failure.

P10 still does not choose START+SELECT or any other product trigger, does not
wire ordinary `src/app.c`, and does not activate the currently dormant Pi MPEG
factory. Pi ordinary MPEG composition remains a separate downstream dependency;
its retirement deadline must be grounded in Configuration/evidence rather than
invented in this packet.

### Required behavior

1. **Exact dual-owner admission.** Start handoff is legal only from P9
   ACCEPTED_PROTECTED with copyable exact geometry, P2 still frozen, P3 exact
   RFB_ONLY/no snapshot, and one session-scoped R21/R24 run coordinator in IDLE
   with current_generation zero. Any mismatch rejects before invoking run start.
2. **Copy before mutation.** Copy P9's accepted geometry into an
   Application-local value before invoking R21. The read must not thaw P2,
   consume P9 protection or mutate P3.
3. **One existing run-start invocation.** Invoke
   `pstvnc_app_mpeg_run_start()` exactly once using that exact geometry and the
   same P2/P3 owners plus the caller-supplied current Transport access and
   session media clock. Do not duplicate generation allocation, profile lookup,
   worker/runtime startup, P3 arm, P7 init or START serialization.
4. **Successful run proof before ownership transfer.** Treat R21 OK as
   insufficient by itself. Re-read run status and P3 snapshot and require:
   Application STARTED_WAIT_FIRST_FRAME, nonzero exact current generation,
   no teardown requirement, P2 still frozen, P3 exact WAIT_FIRST_FRAME with the
   same generation and geometry copied from P9.
5. **Commit P9 without thaw only after proof.** Add the narrow P9 owner operation
   needed to commit a successful protected handoff. It must verify the exact
   frozen P2 + WAIT_FIRST_FRAME P3 snapshot matches its retained accepted
   geometry, then clear only calibration-protected geometry/borrowed foreground
   authority and return P9 to reusable IDLE **without thawing P2**. It must not
   arm/promote/retire P3 or touch the run owner.
6. **Clean pre-START failure rolls back protection only when proven.** If
   `pstvnc_app_mpeg_run_start()` returns non-OK but the run independently
   proves clean IDLE/current_generation zero and P3 proves exact RFB_ONLY/no
   snapshot, use P9's existing explicit abort to thaw P2 once and return P9
   IDLE. Preserve P2's resulting FULL/HOLD debt. Do not manufacture rollback
   from a result code alone.
7. **Uncertain/irreversible failure never thaws.** If run start fails and clean
   pre-START rollback cannot be independently proven—including run FAULTED,
   nonzero/current generation ownership, P3 WAIT_FIRST_FRAME or any inconsistent
   state—leave P2 frozen, do not call P9 abort, and mark the handoff transaction
   faulted/teardown-required for outer recovery.
8. **Truthful P9 failure containment.** Provide the smallest P9-owned seam needed
   to mark its ACCEPTED_PROTECTED authority fault-contained without thawing when
   the run has acquired/possibly acquired downstream authority. Do not leave P9
   advertising a normally usable protected acceptance after P3 is no longer
   RFB_ONLY.
9. **No duplicate ownership after success.** On successful handoff, P9 no longer
   exposes accepted geometry or owns live protection; the run coordinator is
   the sole Application MPEG-run lifecycle owner while P2 remains frozen under
   that transaction's accepted contract.
10. **Generation history remains run-owned.** P10 never allocates, increments,
    resets or caches MPEG generation identity independently. Repeated successful
    cycles use R24's retained run-coordinator generation history.
11. **No trigger/service/retirement expansion.** P10 performs no ordinary
    controller trigger detection, no `app_mpeg_run_service()`, retirement,
    restored-RFB proof or reveal. It does not activate/reconfigure the Pi MPEG
    factory and does not implement active-MPEG recalibration.
12. **Scope/evidence.** Keep lower-owner RFB, input-runtime, P3/compositor,
    Platform, P7/MPEG/Transport mechanisms, Configuration, Pi product source,
    AUDIO, DESKTOP CALIBRATION and Wire bytes unchanged. Add focused deterministic
    composition evidence and keep canonical host/project/dictionary/PS2
    compile/link/reproducibility green.

### Acceptance criteria

- A004-P10-C1 HANDOFF_ADMISSION_REQUIRES_EXACT_P9_PROTECTION_AND_IDLE_RUN
- A004-P10-C2 ACCEPTED_GEOMETRY_COPY_IS_SIDE_EFFECT_FREE_BEFORE_RUN_START
- A004-P10-C3 EXACTLY_ONE_EXISTING_R21_START_TRANSACTION_IS_INVOKED
- A004-P10-C4 SUCCESS_REQUIRES_INDEPENDENT_WAIT_FIRST_FRAME_GENERATION_GEOMETRY_PROOF
- A004-P10-C5 P9_COMMIT_AFTER_SUCCESS_CLEARS_CALIBRATION_AUTHORITY_WITHOUT_P2_THAW
- A004-P10-C6 PROVEN_CLEAN_PRESTART_FAILURE_USES_EXISTING_P9_ABORT_AND_FULL_DEBT
- A004-P10-C7 UNCERTAIN_OR_IRREVERSIBLE_START_FAILURE_NEVER_THAWS_P2
- A004-P10-C8 P9_UNCERTAIN_HANDOFF_STATE_IS_FAULT_CONTAINED_NOT_NORMAL_ACCEPTED
- A004-P10-C9 SUCCESS_LEAVES_RUN_SOLE_LIFECYCLE_OWNER_WITH_P2_CONTINUOUSLY_FROZEN
- A004-P10-C10 GENERATION_ALLOCATION_AND_HISTORY_REMAIN_EXCLUSIVELY_APP_MPEG_RUN_OWNED
- A004-P10-C11 NO_TRIGGER_LIVE_SERVICE_RETIRE_REVEAL_PI_FACTORY_OR_RECALIBRATION_SCOPE
- A004-P10-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance.

### Required deterministic evidence

P10 must prove at least:

1. admission rejects P9 not ACCEPTED_PROTECTED, unfrozen P2, non-RFB_ONLY P3,
   non-idle/faulted run, or uncopyable geometry before any start call;
2. accepted geometry copy causes no P2/P3/P9 event;
3. exactly one start call receives byte/value-identical geometry and the exact
   borrowed P2/P3/Transport/media-clock owners;
4. a successful start that does not independently prove exact
   STARTED_WAIT_FIRST_FRAME + P3 snapshot is treated as contradiction, not
   success;
5. exact success proves P2 never thawed between P9 accept and run
   WAIT_FIRST_FRAME;
6. P9 successful handoff commit clears its geometry/protection authority without
   calling P2 set_frozen(0), while run/P3 retain exact generation geometry;
7. representative clean pre-START failures whose R21 unwind returns run IDLE/P3
   RFB_ONLY invoke P9 abort exactly once and leave ordinary P2 FULL/HOLD debt;
8. START send failure or cleanup/owner uncertainty leaves P2 frozen, does not
   invoke P9 abort and fault-contains P9/handoff authority for outer teardown;
9. no double-start is possible from one accepted P9 transaction;
10. after a later independently completed R24 cycle and a new P9 acceptance, the
    same run coordinator would allocate the successor generation rather than P10
    owning any counter;
11. direct source scan contains no permanent trigger, run live-service,
    retirement/reveal, Pi-factory activation or lower-owner mutation;
12. P9/P8/P1, R21-R24, P2/P3/P7/Transport/MPEG and ordinary Application
    regressions remain green.

### Authorized source surface

P10 may modify only the smallest justified subset of:

- `src/app_mpeg_calibration.c` / `.h` for narrow protected-handoff
  commit/fault-containment owner seams;
- new narrowly named `src/app_mpeg_activation.c` / `.h` (preferred) for the
  cross-owner P9 -> R21 composition;
- focused `tests/unit/app_mpeg_activation_test.c` and/or directly affected P9
  test coverage;
- `tests/Makefile`;
- root symbol dictionaries and source-topology/build manifests only if genuinely
  required;
- directly affected Application MPEG lifecycle documentation.

Do not modify ordinary `src/app.c`, `src/app_mpeg_run.*`, P8/P1 behavior,
RFB/input-runtime/local-UI lower owners, Display/P3/compositor/Platform
mechanisms, P7/MPEG worker/runtime/backend, Transport, Configuration, Pi product
source, AUDIO, DESKTOP CALIBRATION or Wire protocol. If the accepted R21/P9
public seams cannot express the transaction without a lower-owner change, stop
and return BLOCKED with the exact missing contract rather than widening scope.

### Required checks before handoff

Run focused P10 transfer tests plus P9/P8/P1, R21-R24, P2/P3/compositor/P7,
Transport/MPEG and ordinary Application regressions; canonical host tests;
project check; complete strict dictionary audit; pinned PS2 compile/link and
current-source reproducibility. Record the exact linked identity if P10
enrollment changes loadable bytes.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` revision 0007 using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a004-presentation`;
- WORKER_KEY=`interactive`.

Then stop and return the baton.

## Current hardware debt

Current fully Foreman-accepted behavior-bearing loadable authority is P9:

`PT_LOAD_SHA256=b57964cb50464bcbe8239865f7d54d890e0fffe9d5bf4479b79d178c0293270a`
`PT_LOAD_BYTES=510996`

This identity is repository-reproducible and not physically hardware-qualified.

P10 may add linked Application composition bytes. Any new exact PT_LOAD identity
remains hardware-pending until separately physically qualified.

The accepted R17 Pi MPEG mechanism is still deliberately dormant in ordinary
`pi/wire_runtime.py`. Its later product factory composition remains deferred;
no retirement-timeout value is invented by State 0063.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
