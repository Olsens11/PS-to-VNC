# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0075
RECORDED_AT=2026-09-25T21:33:35-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0074
SUPERSEDES_FOREMAN_STATE_REVISION=0074
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0075 independently accepts
`A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P` at final pre-log source
authority `c6e1dcf514adfc29296633374e75020b3480e5e0` and consumes immutable
Reconstruction closeout `c5fdd7f1acdaa2dcb526a5113f2e538614f2d976`.

The returned prerequisite range is exactly thirteen commits ahead of assigning
Foreman log `82cd9affd068ec835c1d2c1a930cedc4c1b95bdd`, zero behind, and is confined
to the authorized Application-MPEG-run / MPEG-worker / focused-test /
dictionary / lifecycle surface. The blocked R34 Application files
`src/app.c` and `src/app_mpeg_product.*` were not behaviorally modified.

Independent source review accepts all twelve R34P requirements. The MPEG worker
now owns one narrow retryable reclamation seam for the exact
create-success/start-failure/destroy-failure partial thread/stack owner; failed
reclamation preserves ownership, while success destroys the never-started thread
before releasing its stack and never fabricates join, worker-finished, decoder
outcome or stop facts.

The Application MPEG run owner now admits only exact reachable R21 pre-START
FAULTED/teardown-required prefixes after the old Transport ticket proves
terminal retained storage. Truly started workers still use proof-driven
stop/status/join/outcome/release; an already-joined pre-START worker is not
stopped/joined twice; the never-started partial worker delegates only to the
worker-owned reclaim seam; PS2 worker-runtime release remains after worker
no-touch proof. Success reaches terminal SESSION_ABORT_READY without START,
RETIRE, producer-done, Transport MPEG finalization, P2 thaw, P3 seal/reveal or
run reuse. Post-START R33 meaning remains unchanged.

Exact final-source GitHub Actions run `36208396947` (attempt 1) checked out
`c6e1dcf514adfc29296633374e75020b3480e5e0` on
`ledge/h1-all-guns` and passed host-unit, project-check, strict dictionaries,
pinned PS2 compile, linked build and current-source reproducibility. The linked
branch identity is
`ELF_PRISTINE_SHA256=a7127b71ede10efa4a91e1914dc55a64bf75d6dffe7169df825d444c77615ccd`,
one PT_LOAD
`76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`,
525716 bytes.

R34P source authority is Foreman-accepted, but that linked image also contains
the still-unaccepted partial R34 Application composition already present on the
branch. Therefore the image is reproducible machine evidence, not a newly
fully-accepted product image and not hardware-qualified. The newest fully
Foreman-accepted loadable product authority remains R33.

With the lower-owner prerequisite closed, the exact next dependency is to
resume R34 only far enough to make ordinary Application teardown selection
distinguish:

- a healthy/current live MPEG run that needs post-START R33;
- a pre-START R21 fault that now needs accepted R34P;
- a clean/no-MPEG attempt that must retain ordinary R16B one-shot Transport
  abort behavior.

That continuation must not broaden the existing live-service helper into a
pre-START service predicate, and it must not enter normal same-session
retirement/recalibration.

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

`A003_R34P_PRESTART_PARTIAL_SESSION_DORMANCY_FOREMAN_ACCEPTED__A006_R34C_ORDINARY_MPEG_ACTION_ACTIVATION_CONTINUATION_ACTIVE__NORMAL_RECALIBRATION_RETIREMENT_DEFERRED`

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
APPLICATION_MPEG_PROTECTED_START_HANDOFF=FOREMAN_ACCEPTED
PI_MPEG_ORDINARY_PRODUCT_COMPOSITION=FOREMAN_ACCEPTED
PS2_MEDIA_CLOCK_PRODUCT_BINDING=FOREMAN_ACCEPTED
APPLICATION_MPEG_SESSION_FOUNDATION=FOREMAN_ACCEPTED
SEMANTIC_PRODUCT_ACTION_BINDING_CORE=FOREMAN_ACCEPTED
INPUT_RUNTIME_PRODUCT_ACTION_PUBLICATION=FOREMAN_ACCEPTED
PRODUCT_ACTION_BINDING_CONFIG_MODEL=FOREMAN_ACCEPTED
MANAGEMENT_CONFIG_READ_CLIENT=FOREMAN_ACCEPTED
APPLICATION_PRODUCT_BINDING_SNAPSHOT=FOREMAN_ACCEPTED
TRANSPORT_MPEG_SESSION_ABORT_FENCE=FOREMAN_ACCEPTED
MPEG_PRESTART_PARTIAL_SESSION_DORMANCY=FOREMAN_ACCEPTED_R34P
MPEG_CALIBRATION_ACTION_ROUTING=RECONSTRUCTION_ACTIVE_R34C
MPEG_CALIBRATION_PRODUCT_BINDING=CONFIG_SELECTED_LIVE_INSTALL_RECONSTRUCTION_ACTIVE_R34C__NO_DEFAULT
ORDINARY_MPEG_PRODUCT_ACTIVATION=RECONSTRUCTION_ACTIVE_R34C_THROUGH_LIVE_FRAME_SERVICE_AND_SAFE_PRESTART_POSTSTART_SESSION_TEARDOWN
R34_CONTINUATION=RECONSTRUCTION_ACTIVE_R34C
NORMAL_MPEG_RECALIBRATION_RETIREMENT=DEPENDENCY_QUEUED_AFTER_R34
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

## Accepted P10 protected MPEG start-handoff authority

PACKET_ID=A004-APPLICATION-MPEG-PROTECTED-START-HANDOFF-P10
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0063
ASSIGNING_FOREMAN_STATE_COMMIT=1f1c0495d59ee6f58f45bd929dc88e5101160d79
ASSIGNING_FOREMAN_LOG_COMMIT=8bc28bd4cb023cbb0cf29625c5da165a37392fc2
RECONSTRUCTION_STARTING_COMMIT=8bc28bd4cb023cbb0cf29625c5da165a37392fc2
P10_FINAL_SOURCE_COMMIT=2daf7194c0c92412464d10cee67990c3bb270f63
P10_RECONSTRUCTION_LOG_COMMIT=e7baae8177882edbba8c19fba3069886e972b7bb
P10_PRE_LOG_COMMIT_COUNT=6

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T091219-0400__reconstruction__a004-presentation__interactive.md`

### P10 criterion disposition

A004-P10-C1=MET
A004-P10-C2=MET
A004-P10-C3=MET
A004-P10-C4=MET
A004-P10-C5=MET
A004-P10-C6=MET
A004-P10-C7=MET
A004-P10-C8=MET
A004-P10-C9=MET
A004-P10-C10=MET
A004-P10-C11=MET
A004-P10-C12=MET

Independent Foreman findings:

1. P10 admission requires P9 ACCEPTED_PROTECTED, exact copyable geometry, P2
   frozen/publication-denied, P3 exact RFB_ONLY/no snapshot, and a healthy IDLE
   run with zero current generation before start is invoked.
2. Accepted geometry is copied before mutation and P9's read seam does not thaw
   P2, consume protected authority or mutate P3.
3. The composition invokes only the existing
   `pstvnc_app_mpeg_run_start()` once, passing the copied geometry and exact
   borrowed P2/P3/Transport/media-clock owners. It owns no generation counter or
   lower run-start mechanism.
4. Success is independently re-proven through run status plus exact P3 snapshot:
   STARTED_WAIT_FIRST_FRAME, nonzero exact current generation, no teardown
   requirement, P2 still frozen, P3 exact WAIT_FIRST_FRAME and byte/value-equal
   geometry.
5. P9's new protected-handoff commit seam is legal only against that exact
   downstream WAIT_FIRST_FRAME authority. It clears calibration geometry and
   borrowed foreground references, returns P9 IDLE and never thaws P2.
6. A failed start may use P9's existing abort only after independent proof that
   R21 fully unwound to IDLE/current_generation zero and P3 exact
   RFB_ONLY/no-snapshot while P2 remains frozen.
7. START uncertainty, FAULTED/non-idle run ownership, nonzero generation,
   WAIT_FIRST_FRAME or other proof contradiction never invokes P9 abort and
   never thaws P2.
8. P9's fault-contained protected-handoff seam preserves frozen protection and
   retained geometry/evidence rather than advertising a normally reusable
   ACCEPTED_PROTECTED endpoint after downstream ownership becomes uncertain.
9. Successful transfer removes duplicate P9 geometry/protection authority; the
   existing run coordinator becomes the sole Application MPEG lifecycle owner.
10. P10 allocates, increments, resets and caches no generation identity.
    Repeated-cycle tests prove generation history remains in app_mpeg_run.
11. No ordinary `src/app.c`, permanent trigger, live service, retirement,
    reveal, Pi factory activation, active-MPEG recalibration, lower-owner
    mechanism or Wire/protocol behavior entered P10.
12. The final changed-path range is confined to the narrow activation
    composition, P9 owner seams, focused tests, build/topology enrollment,
    dictionaries and directly affected lifecycle documentation.

### Exact P10 machine evidence

At exact final source authority
`2daf7194c0c92412464d10cee67990c3bb270f63`, GitHub Actions run
`36140699072`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `2daf7194c0c92412464d10cee67990c3bb270f63`,
branch `ledge/h1-all-guns`, push event and title
`test: verify final P10 protected start handoff authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed host evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=210 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

P10_SOURCE_COMPLETE=YES
P10_HOST_TESTED=PASS
P10_PROJECT_CHECK=PASS
P10_STRICT_DICTIONARIES=PASS
P10_PS2_COMPILE=PASS
P10_PS2_LINK=PASS
P10_CURRENT_SOURCE_REPRODUCIBILITY=PASS
P10_LOADABLE_BYTES_CHANGED=YES
P10_MACHINE_EVIDENCE=GITHUB_ACTIONS
P10_INDEPENDENT_VALIDATION=NOT_RUN
P10_OPERATOR_OBSERVED=NO
P10_HARDWARE_QUALIFIED=NO
P10_HARDWARE_PENDING=YES
P10_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R25 ordinary Pi MPEG product-composition authority

PACKET_ID=A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0064
ASSIGNING_FOREMAN_STATE_COMMIT=cf0fa37ff0d7ef5b0c74fc6ee79cf207ac14919d
ASSIGNING_FOREMAN_LOG_COMMIT=501d6ade655e0ef68692ac42b9b9959c340f3e17
RECONSTRUCTION_STARTING_COMMIT=501d6ade655e0ef68692ac42b9b9959c340f3e17
R25_FINAL_SOURCE_COMMIT=60b7759fb78d5f555a589b9ce8cb58ce96096945
R25_RECONSTRUCTION_LOG_COMMIT=cb78de304554f1ee61be39a24fd04d256a8bdea9
R25_PRE_LOG_COMMIT_COUNT=10

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T101750-0400__reconstruction__a003-mpeg-generation__interactive.md`

### R25 criterion disposition

A003-R25-C1=MET
A003-R25-C2=MET
A003-R25-C3=MET
A003-R25-C4=MET
A003-R25-C5=MET
A003-R25-C6=MET
A003-R25-C7=MET
A003-R25-C8=MET
A003-R25-C9=MET
A003-R25-C10=MET
A003-R25-C11=MET
A003-R25-C12=MET

Independent Foreman findings:

1. `pi/mpeg_product_profile.py` is a narrow immutable composition owner for
   exactly display `:0`, desktop 704x462 and retirement deadline 8.0 seconds.
   It records the frozen H1 deadline provenance and does not duplicate R17
   channel/buffer/frame-rate tuning.
2. `pi/wire_runtime.py` constructs one fresh
   `MpegGenerationController` from exact Wire Session ID plus the selected
   composition and existing R17 producer profiles; no generation identity is
   minted there.
3. Ordinary `build_product_wire_server()` supplies both the accepted RFB
   attachment factory and the new MPEG generation factory. No change was made to
   `pi/wire_server.py`.
4. Real WireServer-focused tests prove rejected/malformed Q4 invokes the MPEG
   factory zero times and accepted Q4 invokes it exactly once with the allocated
   session ID.
5. Controller construction and MPEG CREDIT are producer-inert. Exact START is
   the first injected producer-factory invocation.
6. RFB and MPEG remain independent riders: the MPEG lifecycle is exercised with
   zero RFB provider connection attempts while both factories coexist.
7. MPEG DATA and RETIRE completion are observed only through the real accepted
   WireServer/WireConnectionOwner serialization path; R25 adds no send/recv or
   sequence owner.
8. One controller retains same-session generation high-water across N->RETIRE->
   N+1 and rejects stale N. A later physical Wire Session receives a distinct
   fresh controller with zero generation history and no old producer/plan.
9. Exact session finish still calls the attached controller `close()`. An
   injected false close result marks the otherwise accepted session
   protocol-failed.
10. Factory construction failure after Q4 terminates only that physical session;
    the persistent server can accept a later fresh session with a new controller.
11. The installer stages/verifies/removes the new maintained profile file while
    preserving its existing no-live-system-mutation guard.
12. The final changed-path range is Pi composition/profile, deterministic tests,
    safe staging, dictionaries and directly affected documentation only. No PS2
    product source, AUDIO, systemd state, R17 mechanism or Wire protocol bytes
    changed.

### Exact R25 machine evidence

At exact final source authority
`60b7759fb78d5f555a589b9ce8cb58ce96096945`, GitHub Actions run
`36148209311`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `60b7759fb78d5f555a589b9ce8cb58ce96096945`,
branch `ledge/h1-all-guns`, push event and title
`test(pi): prove fresh R25 controller per session`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed Pi evidence includes:

- `pi_wire_server_test.py`: 20 tests, OK;
- `pi_internal_rfb_provider_test.py`: 9 tests, OK;
- `pi_mpeg_generation_test.py`: 12 tests, OK;
- `pi_mpeg_product_composition_test.py`: 9 tests, OK;
- accepted Pi RFB/native-provider/runtime-profile suites remained green.

Observed cross-domain evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=212 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked PS2 identity remains:

`ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R25_SOURCE_COMPLETE=YES
R25_HOST_TESTED=PASS
R25_PROJECT_CHECK=PASS
R25_STRICT_DICTIONARIES=PASS
R25_PS2_COMPILE=PASS
R25_PS2_LINK=PASS
R25_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R25_PS2_PT_LOAD_CHANGED=NO
R25_PI_PRODUCT_BYTES_CHANGED=YES
R25_MACHINE_EVIDENCE=GITHUB_ACTIONS
R25_INDEPENDENT_VALIDATION=NOT_RUN
R25_OPERATOR_OBSERVED=NO
R25_HARDWARE_QUALIFIED=NO_NEW_CLAIM
R25_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R26 PS2 media-clock product-binding authority

PACKET_ID=A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0065
ASSIGNING_FOREMAN_STATE_COMMIT=ee41686f274ac814e58d2e5bc0271e275eade8a3
ASSIGNING_FOREMAN_LOG_COMMIT=e3b55a804a3133c7bd1c495772039d122a403aff
RECONSTRUCTION_STARTING_COMMIT=e3b55a804a3133c7bd1c495772039d122a403aff
R26_FINAL_SOURCE_COMMIT=82a21a3a68a4288362003df2346783cb0c12ed0b
R26_RECONSTRUCTION_LOG_COMMIT=dd8a116ff1473aaedd5483e671efb8bc8b767718
R26_PRE_LOG_COMMIT_COUNT=7

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T105026-0400__reconstruction__a002-config-audio-clock__interactive.md`

### R26 criterion disposition

A002-R26-C1=MET
A002-R26-C2=MET
A002-R26-C3=MET
A002-R26-C4=MET
A002-R26-C5=MET
A002-R26-C6=MET
A002-R26-C7=MET
A002-R26-C8=MET
A002-R26-C9=MET
A002-R26-C10=MET
A002-R26-C11=MET
A002-R26-C12=MET

Independent Foreman findings:

1. `src/config/media_clock_profile.*` publishes exactly
   `epoch_lead_us=0`, `audio_presentation_offset_us=0` and
   `video_presentation_offset_us=0`. Selection returns a value copy, so caller
   mutation cannot alter Configuration-owned authority.
2. The Configuration source records A002 and frozen H1 provenance and adds no
   PSTV CONFIG field or dynamic user/network tuning mechanism.
3. `src/platform/ps2_media_clock.*` owns only one session-scoped semaphore
   handle/state plus injected synchronization/time callbacks. It contains no
   epoch, armed, offset, deadline or media-generation state.
4. Binding lifetime is NEW -> ACTIVE -> RETIRED. Initialization pessimistically
   marks the object RETIRED before CreateSema, so a create failure exposes no
   partially usable authority and cannot be retried as if nothing happened.
5. A successful binding creates one EE semaphore with initial count 1, maximum
   count 1 and option 0. Lock/unlock call WaitSema/SignalSema only on that exact
   active binding's semaphore ID and propagate kernel failure.
6. Release revokes local authority before DeleteSema. Even if kernel deletion
   fails, all previously copied sync/time observers resolve through the now-
   RETIRED binding and fail closed; the binding cannot be reinitialized.
7. Current tick and injected read_ticks directly return
   `GetTimerSystemTime()`; tick rate is exactly `kBUSCLK`; delay delegates
   the exact bounded request to `DelayThread()` and propagates failure.
8. Session-A release followed by Session-B initialization uses distinct binding
   objects and fresh semaphore authority. Stale Session-A observer copies fail
   while Session B remains usable.
9. No `src/media/clock.*` mechanism changed. Existing A002 one-shot arm,
   synchronized publication, signed offsets, saturation and wait semantics
   remain the sole media-clock implementation.
10. R26 source contains no `pstvnc_media_clock_arm()` call. Ordinary
    `src/app.c` does not consume the new binding/profile symbols yet.
11. The final changed-path range is confined to Configuration/Platform binding,
    tests/stubs, build/topology enrollment, dictionaries and directly affected
    documentation. No Application MPEG, AUDIO, Transport/Wire or Pi behavior
    changed.
12. R26's new Config/Platform objects are explicitly enrolled in pinned PS2
    compile and the product link rather than being unlinked host-only source.

### Exact R26 machine evidence

At exact final source authority
`82a21a3a68a4288362003df2346783cb0c12ed0b`, GitHub Actions run
`36155777592`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `82a21a3a68a4288362003df2346783cb0c12ed0b`,
branch `ledge/h1-all-guns`, push event and title
`test: prove R26 media clock ownership boundaries`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed focused/cross-domain evidence includes:

- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`;
- R25 `pi_mpeg_product_composition_test.py`: 9 tests, OK;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=214 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled
  `src/config/media_clock_profile.c` and `src/platform/ps2_media_clock.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=2dd5cd5a5083c659efcdc3f3cdbcafc9c3147f8ba964088b2d99d79b3e963613`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=e3b84ce42a3527c9518643edd808228637c980f5ea901fd0cf5256e28b6858aa`
`PT_LOAD_BYTES=513812`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from the previously accepted P10/R25 PT_LOAD and is current new
PS2 hardware debt.

R26_SOURCE_COMPLETE=YES
R26_HOST_TESTED=PASS
R26_PROJECT_CHECK=PASS
R26_STRICT_DICTIONARIES=PASS
R26_PS2_COMPILE=PASS
R26_PS2_LINK=PASS
R26_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R26_PS2_PT_LOAD_CHANGED=YES
R26_MACHINE_EVIDENCE=GITHUB_ACTIONS
R26_INDEPENDENT_VALIDATION=NOT_RUN
R26_OPERATOR_OBSERVED=NO
R26_HARDWARE_QUALIFIED=NO
R26_HARDWARE_PENDING=YES
R26_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R27 ordinary Application MPEG-session foundation authority

PACKET_ID=A003-APPLICATION-MPEG-SESSION-FOUNDATION-R27
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0066
ASSIGNING_FOREMAN_STATE_COMMIT=92f745bb0fa9bb0b04091a843793db77673f0a6b
ASSIGNING_FOREMAN_LOG_COMMIT=bc03d921ed08b0156db3de9ae4a4b60f7b751604
RECONSTRUCTION_STARTING_COMMIT=bc03d921ed08b0156db3de9ae4a4b60f7b751604
R27_FINAL_SOURCE_COMMIT=b126b749c2d3b3182c9b562973614106e98f0edb
R27_RECONSTRUCTION_LOG_COMMIT=41688117e7dd1f5283a687d08c26acf605afa3b1
R27_PRE_LOG_COMMIT_COUNT=7

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T121143-0400__reconstruction__a003-mpeg-generation__interactive.md`

### R27 criterion disposition

A003-R27-C1=MET
A003-R27-C2=MET
A003-R27-C3=MET
A003-R27-C4=MET
A003-R27-C5=MET
A003-R27-C6=MET
A003-R27-C7=MET
A003-R27-C8=MET
A003-R27-C9=MET
A003-R27-C10=MET
A003-R27-C11=MET
A003-R27-C12=MET

Independent Foreman findings:

1. `pstvnc_app_run()` resolves the selected RFB Transport projection, selected
   R7 MPEG runtime profile and selected R26 media-clock profile before entering
   IOP/network/platform startup. Missing RFB or MPEG profile authority fails
   before resource ownership.
2. The configured Application seam now accepts exact RFB Transport, MPEG
   Transport and media-clock profile values and rejects null required authority
   before startup.
3. Every fresh physical attempt uses exactly
   `pstvnc_transport_session_open_with_mpeg()` with the existing RFB
   Transport values plus selected R7 `profile->transport`. Ordinary product
   source no longer calls the plain RFB-only Transport constructor.
4. R27 creates no second Transport/channel-4 owner. Accepted Transport remains
   the sole physical/runtime owner and R25's Pi controller/CREDIT boundary
   remains producer-inert until future exact START.
5. After Transport admission, each attempt creates one fresh automatic R26
   binding, obtains exact synchronization and tick-rate authority, initializes
   one clean A002 media clock from the selected profile and proves it unarmed.
6. R27 contains no media-clock arm call. Ordinary RFB startup, presentation,
   P2 service, input/UI routing and idle polling cannot arm the clock.
7. Provider-local recovery preserves the required exact order:
   input shutdown/dormancy proof -> R26 binding release -> Transport abort ->
   successor connect. No replacement is admitted while any proof remains
   unproven.
8. R26 revokes local binding authority before DeleteSema result. R27 therefore
   records a release failure, does not retry/reuse the binding, continues
   monotonic Transport teardown, and blocks successor admission.
9. Fatal cleanup remains reverse-owned: input first, then active clock binding,
   then Transport/caller socket, followed by resident graphics/diagnostics.
   A pre-binding failure fabricates no binding release.
10. Successive provider-recovery attempts create distinct binding/clock objects;
    no armed/epoch/synchronization identity is copied forward.
11. R27 does not call P9/P10/R21-R24, Transport MPEG run-open, START/RETIRE,
    MPEG worker/decoder or P3 mutation; no controller mapping, AUDIO, Pi or Wire
    protocol source changed.
12. Existing R15/R16B/R19 Application behavior and focused R21-R26/R25
    regressions remain green.

### Exact R27 machine evidence

At exact final source authority
`b126b749c2d3b3182c9b562973614106e98f0edb`, GitHub Actions run
`36160798566`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `b126b749c2d3b3182c9b562973614106e98f0edb`,
branch `ledge/h1-all-guns`, push event and title
`test: verify final R27 dormant MPEG session authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

Observed focused/cross-domain evidence includes:

- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- R25 `pi_mpeg_product_composition_test.py`: 9 tests, OK;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=216 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled current `src/app.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=7f5decc9f09b32d1060f18eca1462bcb189cc3ce96297fd5b3435ba448288d2c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=30c8d9d1db9106f51c8de56c88e68cf3d6181d98cfd8ff4d402ce24a6c94c13d`
`PT_LOAD_BYTES=513940`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R26 and is current new PS2 hardware debt.

R27_SOURCE_COMPLETE=YES
R27_HOST_TESTED=PASS
R27_PROJECT_CHECK=PASS
R27_STRICT_DICTIONARIES=PASS
R27_PS2_COMPILE=PASS
R27_PS2_LINK=PASS
R27_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R27_PS2_PT_LOAD_CHANGED=YES
R27_MACHINE_EVIDENCE=GITHUB_ACTIONS
R27_INDEPENDENT_VALIDATION=NOT_RUN
R27_OPERATOR_OBSERVED=NO
R27_HARDWARE_QUALIFIED=NO
R27_HARDWARE_PENDING=YES
R27_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R28 semantic product-action binding-core authority

PACKET_ID=A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0067
ASSIGNING_FOREMAN_STATE_COMMIT=bf26fc0b10435b355a6e97445d04a8a8368028ed
ASSIGNING_FOREMAN_LOG_COMMIT=dd771a86c0922f7c4f4b5a4ebc333e16d496f293
RECONSTRUCTION_STARTING_COMMIT=dd771a86c0922f7c4f4b5a4ebc333e16d496f293
R28_FINAL_SOURCE_COMMIT=c9df08288689d47eb85c889b8c95c7a4741a48a9
R28_RECONSTRUCTION_LOG_COMMIT=22aea289ba06d744387913bc1809c60ba32800a8
R28_PRE_LOG_COMMIT_COUNT=7

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T144549-0400__reconstruction__a005-interaction-input__interactive.md`

### R28 criterion disposition

A005-R28-C1=MET
A005-R28-C2=MET
A005-R28-C3=MET
A005-R28-C4=MET
A005-R28-C5=MET
A005-R28-C6=MET
A005-R28-C7=MET
A005-R28-C8=MET
A005-R28-C9=MET
A005-R28-C10=MET
A005-R28-C11=MET
A005-R28-C12=MET

Independent Foreman findings:

1. `pstvnc_product_action_t` is Input-owned semantic meaning only and currently
   admits exactly `MPEG_CALIBRATION` beyond NONE.
2. Each caller-supplied binding explicitly carries project button mask, semantic
   action, SETTLE/RELEASE/HOLD trigger and DESKTOP/GLOBAL context.
3. Resolver initialization accepts an explicit zero-binding configuration and
   rejects null non-empty sets, invalid fields, out-of-range button bits and
   duplicate exact masks, including cross-context ambiguity.
4. No compiled/default binding table exists. The maintained Input action source
   contains no START/SELECT product bit and no 750000-us H1 adapter.
5. Exact relevant-mask changes reset settle/hold progress. RELEASE arms only on
   legitimate growth into its exact mask, growth cancels an existing arm, and
   later shrink cannot manufacture a subset arm.
6. SETTLE uses the qualified 8-poll threshold, extends to 18 when an eligible
   HOLD strict superset exists, and HOLD uses the qualified 120-poll completion
   count after settle admission.
7. Any fired SETTLE/HOLD or partially released RELEASE gesture latches until
   complete relevant-button release, preventing same-gesture subset leakage.
8. DESKTOP provenance is monotonic within one gesture: it must begin in desktop
   and remain eligible through resolution. Foreground change cannot acquire or
   reacquire DESKTOP authority mid-gesture.
9. `connection_epoch_started` resets pending resolver history before the new
   physical epoch is interpreted.
10. `PSTVNC_INPUT_EVENT_PRODUCT_ACTION` validates semantic action payloads and
    preserves the existing bounded FIFO copy/order/wrap/discard contract.
11. R28 does not touch live `input_runtime`, Application, UI, RFB, MPEG,
    media clock, Transport, AUDIO, Pi, binding-config parsing or urgent-control
    behavior.
12. The R28 source boundary and changed-path set remain inside the assigned
    Input/test/docs/build surface.

### Exact R28 machine evidence

At exact final source authority
`c9df08288689d47eb85c889b8c95c7a4741a48a9`, GitHub Actions run
`36177342038`, attempt 1, completed SUCCESS. The run object independently
identifies exact head SHA `c9df08288689d47eb85c889b8c95c7a4741a48a9`,
branch `ledge/h1-all-guns`, push event and title
`test: verify final R28 semantic product action authority`.

Observed focused/cross-domain evidence includes:

- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- R25 Pi MPEG product composition: 9 tests, OK.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=218 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled current
  `src/input/input.c` and `src/input/product_action.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=954e0cf8ff435e9e948afc5ace3251108d5736957bf45273f86bbdd274e5698f`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=2281d0d06b62635dfb829512c7c51c10cf1bfd3c6f96169db677bb034b952876`
`PT_LOAD_BYTES=515732`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R27 and is current new PS2 hardware debt.

R28_SOURCE_COMPLETE=YES
R28_HOST_TESTED=PASS
R28_PROJECT_CHECK=PASS
R28_STRICT_DICTIONARIES=PASS
R28_PS2_COMPILE=PASS
R28_PS2_LINK=PASS
R28_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R28_PS2_PT_LOAD_CHANGED=YES
R28_MACHINE_EVIDENCE=GITHUB_ACTIONS
R28_INDEPENDENT_VALIDATION=NOT_RUN
R28_OPERATOR_OBSERVED=NO
R28_HARDWARE_QUALIFIED=NO
R28_HARDWARE_PENDING=YES
R28_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R29 live Input-runtime product-action publication authority

PACKET_ID=A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0068
ASSIGNING_FOREMAN_STATE_COMMIT=395127dd548ab2fe0eecfebfffd4f1969e8f32b3
ASSIGNING_FOREMAN_LOG_COMMIT=b51ea127e69717e4ea9ad3d6237377a8b9baa47f
R29_FINAL_SOURCE_COMMIT=56092a3a6d02df4a9feb89aec5c095e2542b178f
R29_RECONSTRUCTION_LOG_COMMIT=6f6cffe32874b615bfdb71c03eef328d7c2a7188
R29_PRE_LOG_COMMIT_COUNT=7

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T160751-0400__reconstruction__a005-interaction-input__interactive.md`

The R29 closeout worker truthfully began at final source authority
`56092a3a6d02df4a9feb89aec5c095e2542b178f` because concurrent repository
authority had already landed the complete source/test/docs range before that
interactive closeout shift recovered. That does not obscure source provenance:
independent compare from the assigning Foreman log
`b51ea127e69717e4ea9ad3d6237377a8b9baa47f` to final source proves exactly
seven pre-log commits ahead / zero behind, and final source to the immutable
closeout is exactly one log-only commit.

### R29 criterion disposition

A005-R29-C1=MET
A005-R29-C2=MET
A005-R29-C3=MET
A005-R29-C4=MET
A005-R29-C5=MET
A005-R29-C6=MET
A005-R29-C7=MET
A005-R29-C8=MET
A005-R29-C9=MET
A005-R29-C10=MET
A005-R29-C11=MET
A005-R29-C12=MET

Independent Foreman findings:

1. Every `pstvnc_input_runtime_t` initializes with a valid zero-binding R28
   resolver. Zero bindings carry no caller pointer and preserve ordinary
   pre-R29 product behavior.
2. `pstvnc_input_runtime_set_product_action_bindings()` is pre-start only and
   validates the caller-owned immutable binding set through the accepted R28
   initializer. Worker start revalidates the retained values before CreateThread
   and clears all gesture history.
3. Resolver state lives inside the runtime instance. No gesture/settle/hold/
   release/latch state is process-global or shared between sessions.
4. Every trustworthy pad sample is translated to the project controller fact
   and offered exactly once to R28 before sparse CONTROLLER_STATE publication
   and before mouse interpretation. Stable no-edge physical polls therefore
   advance the accepted 8/18/120 timing directly.
5. DESKTOP eligibility is one volatile 0/1 caller fact supplied through the
   narrow Input API. Input includes no UI/Application state and R28 remains sole
   owner of begin/loss/no-reacquisition provenance.
6. If R28 resolves an action, Input enqueues exactly one ordinary
   `PSTVNC_INPUT_EVENT_PRODUCT_ACTION` before same-sample controller-state and
   mouse work.
7. Product-action publication reuses the existing queue semaphore/FIFO. Resolver
   failure and queue wait/signal/full failure record Input worker error and
   abort same-sample continuation.
8. Existing controller-state sparsity, mouse behavior, physical continuity and
   activity notification remain intact when zero bindings are installed or a
   configured gesture does not fire.
9. Physical continuity loss resets resolver history before existing mouse hard-
   boundary handling. Explicit libpad handoff resets product-action history
   before acknowledgement and returns through a fresh pad connection epoch.
10. Ordinary mouse-interpretation suspension retains live libpad polling and
    deliberately does not reset or replace the R28 gesture owner.
11. Activity notification occurs after successful sample processing and sees
    already-published ordinary queue work. No second queue or urgent mailbox was
    introduced.
12. The final changed-path range is confined to Input runtime, focused tests/
    stubs, build/test enrollment, Input dictionaries and directly affected
    documentation. No Application, UI, Configuration parser, RFB, Transport,
    MPEG, media-clock, AUDIO, Pi or H1 forensic product source changed.

### Exact R29 machine evidence

At exact final source authority
`56092a3a6d02df4a9feb89aec5c095e2542b178f`, GitHub Actions run
`36182879910`, attempt 1, completed SUCCESS. The exact run object identifies
branch `ledge/h1-all-guns`, head SHA
`56092a3a6d02df4a9feb89aec5c095e2542b178f`, push event and title
`test: verify final R29 Input runtime action authority`.

Observed focused/cross-domain evidence includes:

- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- R25 Pi product composition: 9 tests, OK.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=220 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled
  `src/input/product_action.c` and `src/input/input_runtime.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=8b0c020f4e410d7a7cc5f7015cc34845f02d382b16055dc607d635a8bbdc7c51`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=f72a1a65b6a16abf792c803a8cfa3322ded51ccf6a038b6a27e94e3ee6eb43f7`
`PT_LOAD_BYTES=516116`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R28 and is current new PS2 hardware debt.

R29_SOURCE_COMPLETE=YES
R29_HOST_TESTED=PASS
R29_PROJECT_CHECK=PASS
R29_STRICT_DICTIONARIES=PASS
R29_PS2_COMPILE=PASS
R29_PS2_LINK=PASS
R29_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R29_PS2_PT_LOAD_CHANGED=YES
R29_MACHINE_EVIDENCE=GITHUB_ACTIONS
R29_INDEPENDENT_VALIDATION=NOT_RUN
R29_OPERATOR_OBSERVED=NO
R29_HARDWARE_QUALIFIED=NO
R29_HARDWARE_PENDING=YES
R29_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R30 human-readable product-action binding authority

PACKET_ID=B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0069
ASSIGNING_FOREMAN_STATE_COMMIT=71c254b752dc6fac187622b3e4dd1c9dbef94642
ASSIGNING_FOREMAN_LOG_COMMIT=25dc15455be2ae68b535d74d5b1dc298ccd7fcac
RECONSTRUCTION_STARTING_COMMIT=25dc15455be2ae68b535d74d5b1dc298ccd7fcac
R30_FINAL_SOURCE_COMMIT=6f42cf5ada266c46e2c6a6b84a91b823d939d1c8
R30_RECONSTRUCTION_LOG_COMMIT=377416cd03a9f24bf1d6b0cd3145b3729b600fd1
R30_PRE_LOG_COMMIT_COUNT=10

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T163720-0400__reconstruction__a005-interaction-input__interactive.md`

### R30 criterion disposition

B10-R30-C1=MET
B10-R30-C2=MET
B10-R30-C3=MET
B10-R30-C4=MET
B10-R30-C5=MET
B10-R30-C6=MET
B10-R30-C7=MET
B10-R30-C8=MET
B10-R30-C9=MET
B10-R30-C10=MET
B10-R30-C11=MET
B10-R30-C12=MET

Independent Foreman findings:

1. `src/config/product_action_bindings.*` owns only a bounded Configuration
   model containing accepted R28 typed values plus count. It does not duplicate
   Input gesture state or define a second binding type.
2. The recognized document surface is exactly `[bindings]` with current key
   `mpeg_calibration`, whose value is chord + trigger + context.
3. Empty/unrelated documents and missing recognized key publish a valid
   zero-binding model. There is no fallback physical mask or default action.
4. All sixteen symbolic button names map one-to-one to the project-owned
   controller bits. Chords reject empty, repeated, unknown and malformed tokens.
5. `settle/release/hold` and `desktop/global` map only to accepted R28 enum
   values. The recognized key maps only to semantic MPEG_CALIBRATION.
6. A completed typed candidate is passed through
   `pstvnc_product_action_resolver_init()` as the accepted R28 validation
   firewall; Config never calls live observe/recognition.
7. Duplicate recognized section/key and malformed recognized value authority
   fail closed. Unknown well-formed future sections and unknown keys remain
   opaque/ignored.
8. Parsing occurs into temporary state and only publishes after complete document
   success. Focused tests prove seeded caller output remains byte-identical on
   recognized failure.
9. Canonical formatting uses stable lowercase tokens and deterministic project
   button ordering. It formats into private bounded storage before touching the
   caller and round-trips exactly through the parser.
10. Document and line sizes are explicitly bounded, embedded NUL rejects, and
    the production implementation uses no heap, executable config or callback
    framework.
11. R30 performs no file/network management operation, no Input-runtime install,
    no Application/UI/RFB/MPEG/media-clock/Transport/AUDIO/Pi effect and no
    urgent mailbox.
12. The final changed-path range is confined to Configuration binding source,
    focused tests/build enrollment, Configuration dictionary and directly
    affected topology documentation.

### Exact R30 machine evidence

At exact final source authority
`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8`, GitHub Actions run
`36187973098`, attempt 1, completed SUCCESS. The exact run object identifies
branch `ledge/h1-all-guns`, head SHA
`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8`, push event and title
`docs(config): explain R30 dictionary ownership`.

Observed focused/cross-domain evidence includes:

- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `config_text_test: PASS`;
- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `WORK_LOG_CHECK=PASS records=222 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=5c78663f84cf6372e9887337095d5b5da2424c2f7e26b68017387ac188d5cca8`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=f6b0a92cb1667a6a5d41ec27d3836d95ddc56450e55562354894637076ceba12`
`PT_LOAD_BYTES=519700`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R29 and is current new PS2 hardware debt.

R30_SOURCE_COMPLETE=YES
R30_HOST_TESTED=PASS
R30_PROJECT_CHECK=PASS
R30_STRICT_DICTIONARIES=PASS
R30_PS2_COMPILE=PASS
R30_PS2_LINK=PASS
R30_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R30_PS2_PT_LOAD_CHANGED=YES
R30_MACHINE_EVIDENCE=GITHUB_ACTIONS
R30_INDEPENDENT_VALIDATION=NOT_RUN
R30_OPERATOR_OBSERVED=NO
R30_HARDWARE_QUALIFIED=NO
R30_HARDWARE_PENDING=YES
R30_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R31 read-only human configuration GET authority

PACKET_ID=B11-READ-ONLY-HUMAN-CONFIG-GET-R31
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0070
ASSIGNING_FOREMAN_STATE_COMMIT=79ec761badfd055d92c795987e03c5c5296d883e
ASSIGNING_FOREMAN_LOG_COMMIT=bca9fe2153f97c2d0784a788f61c323197b3da39
RECONSTRUCTION_STARTING_COMMIT=bca9fe2153f97c2d0784a788f61c323197b3da39
R31_FINAL_SOURCE_COMMIT=fb6eb12868a3045c098651bb97c2577a696bc686
R31_RECONSTRUCTION_LOG_COMMIT=d80ab11662ef5b53b8ac792331ee69015deb37eb
R31_PRE_LOG_COMMIT_COUNT=18

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T175227-0400__reconstruction__a005-interaction-input__interactive.md`

### R31 criterion disposition

B11-R31-C1=MET
B11-R31-C2=MET
B11-R31-C3=MET
B11-R31-C4=MET
B11-R31-C5=MET
B11-R31-C6=MET
B11-R31-C7=MET
B11-R31-C8=MET
B11-R31-C9=MET
B11-R31-C10=MET
B11-R31-C11=MET
B11-R31-C12=MET

Independent Foreman findings:

1. `src/management/config_get.*` is one clean Management owner exposing only
   `pstvnc_management_config_get()`; it exposes no generic arbitrary-path
   product API or POST surface.
2. The exact request is
   `GET /ps2vnc.conf HTTP/1.0\r\nHost: 192.168.50.1\r\nConnection: close\r\n\r\n`
   with no body.
3. Platform owns fixed endpoint socket creation through distinct public PSTV
   and Management constructors backed by one private connect helper. PSTV
   adoption semantics are unchanged.
4. Management receives only on the management descriptor returned by
   `pstvnc_ps2_network_connect_management()`; no Transport API or descriptor
   is referenced.
5. Request, HTTP header, receive chunk and candidate body are all explicitly
   bounded. No heap is used by the R31 client.
6. Only HTTP/1.0 and HTTP/1.1 status 200 forms are accepted. Malformed/short
   versions, non-200 status and missing/oversized header reject.
7. Header termination is exact `\r\n\r\n`; bytes already coalesced after
   it are preserved as the first body bytes and subsequent receives append in
   exact order until clean EOF.
8. The body is connection-close framed only. No Content-Length, chunked,
   compression, redirect or connection-reuse mechanism was invented.
9. Candidate bytes remain private until complete success. Failures preserve
   seeded caller bytes and length. Exact-capacity bodies require a subsequent
   EOF; one extra byte rejects rather than truncating successfully.
10. Partial request sends continue until complete. Zero/error fails. Every
    successfully acquired management descriptor reaches exactly one close.
11. Retrieval returns mechanism success/failure only; it owns no startup
    fallback/recovery/default-binding policy.
12. The new `src/management/` domain is fully enrolled in source topology,
    local dictionary ownership, deterministic reconciliation, host tests and
    PS2 build inputs.

The worker also recorded one intermediate unrelated
`transport_mpeg_test` assertion failure at commit `5b717d4c...`. R31 changes
no Transport/MPEG source, and the exact final canonical run reran the same test
successfully. The final acceptance evidence is therefore the exact final-source
run below, not the superseded intermediate run.

### Exact R31 machine evidence

At exact final source authority
`fb6eb12868a3045c098651bb97c2577a696bc686`, GitHub Actions run
`36195001094`, attempt 1, completed SUCCESS. The run object identifies branch
`ledge/h1-all-guns`, exact head SHA
`fb6eb12868a3045c098651bb97c2577a696bc686`, push event and title
`tooling(topology): enroll clean management domain`.

Observed focused/cross-domain evidence includes:

- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `config_text_test: PASS`;
- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_DIRECTORIES=pi,src,src/audio,src/config,src/diagnostics,src/display,src/framebuffer,src/input,src/management,src/media,src/mpeg,src/platform,src/rfb,src/transport,src/ui`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=224 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled
  `src/management/config_get.c` and `src/platform/ps2_network.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=c834f488d568b4a67e9b4b0eb7622ba2ddf5d820dd95203b907350f3576fae20`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=03e6511045ed8f46ee82e91f8de7d264275b54bf840371a2dfd190367352db3e`
`PT_LOAD_BYTES=520724`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R31_SOURCE_COMPLETE=YES
R31_HOST_TESTED=PASS
R31_PROJECT_CHECK=PASS
R31_STRICT_DICTIONARIES=PASS
R31_PS2_COMPILE=PASS
R31_PS2_LINK=PASS
R31_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R31_PS2_PT_LOAD_CHANGED=YES
R31_MACHINE_EVIDENCE=GITHUB_ACTIONS
R31_INDEPENDENT_VALIDATION=NOT_RUN
R31_OPERATOR_OBSERVED=NO
R31_HARDWARE_QUALIFIED=NO
R31_HARDWARE_PENDING=YES
R31_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R32 resident desired product-binding snapshot authority

PACKET_ID=B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0071
ASSIGNING_FOREMAN_STATE_COMMIT=5971ce0611f7e1c9cdd513d1009153eafa58e3a1
ASSIGNING_FOREMAN_LOG_COMMIT=410846c0826b988567c1f98987e980bd097a2e2b
RECONSTRUCTION_STARTING_COMMIT=410846c0826b988567c1f98987e980bd097a2e2b
R32_FINAL_SOURCE_COMMIT=e18e0170d009093514768d9ea5e4c58b344582e5
R32_RECONSTRUCTION_LOG_COMMIT=6011811a9f9778eca4a603b5679e2929e4649ebc
R32_PRE_LOG_COMMIT_COUNT=16

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T181735-0400__reconstruction__a005-interaction-input__interactive.md`

### R32 criterion disposition

B10-B11-R32-C1=MET
B10-B11-R32-C2=MET
B10-B11-R32-C3=MET
B10-B11-R32-C4=MET
B10-B11-R32-C5=MET
B10-B11-R32-C6=MET
B10-B11-R32-C7=MET
B10-B11-R32-C8=MET
B10-B11-R32-C9=MET
B10-B11-R32-C10=MET
B10-B11-R32-C11=MET
B10-B11-R32-C12=MET

Independent Foreman findings:

1. `pstvnc_app_product_bindings_acquire()` is an Application composition seam
   only; Management still owns raw retrieval and Configuration still owns
   parsing/validation.
2. The complete candidate snapshot is zeroed before R31 is called. Retrieval
   failure publishes FETCH_FAILED_ZERO with an exact zero R30 model.
3. Successful retrieval passes the exact returned raw pointer/byte count into
   R30. No NUL terminator, strlen inference, whitespace rewrite or
   Application-side syntax handling is inserted.
4. Parse rejection publishes INVALID_CONFIG_ZERO and cannot leak partial R30
   state because the output candidate began zero and R30 is atomic.
5. Empty successful body is a VALID zero-binding document.
6. Valid explicit non-default chord/trigger/context values are retained exactly
   as R30 typed authority. Application substitutes no product default.
7. Raw document storage is an automatic fixed 4096-byte buffer whose bound is
   compile-time checked against the R30 document bound. No heap is used.
8. Ordinary `app.c` invokes acquisition exactly once after network/link
   success, before the first PSTV connect, and outside the provider-replacement
   loop.
9. The resident snapshot therefore survives R16B replacement attempts unchanged
   and is not refetched or rewritten by provider failure.
10. Ordinary Input runtime still receives no nonzero binding installation.
11. Ordinary Application still has no PRODUCT_ACTION switch case and invokes no
    P9/P10/R21-R24 activation because of the retained desired snapshot.
12. Changed paths are confined to the Application binding helper, one ordinary
    startup composition call, focused tests/build enrollment, Application
    dictionary and topology documentation.

### Exact R32 machine evidence

At exact final source authority
`e18e0170d009093514768d9ea5e4c58b344582e5`, GitHub Actions run
`36196725652`, attempt 1, completed SUCCESS. The exact run object identifies
branch `ledge/h1-all-guns`, exact head SHA
`e18e0170d009093514768d9ea5e4c58b344582e5`, push event and title
`test(app): verify reconciled R32 source authority`.

Observed focused/cross-domain evidence includes:

- `APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS`;
- `APP_PRODUCT_BINDINGS_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27/R32 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=226 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled `src/app.c` and
  `src/app_product_bindings.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=cf7db4dfd05c8aa0ae8a2b2df94bd279ec36addfbc8e9569e486532a06ca94ff`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c9565c3c55cd758624967ee9cb4b5824ad73bc6f69543d5adb0f1c0b5be2429c`
`PT_LOAD_BYTES=520852`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

The worker recorded one intermediate test-fixture state leak and one later
intermittent `transport_mpeg_test` failure. Neither touched product
Transport/MPEG source. The exact reconciled final-source workflow reran the
complete suite and is the only run used for acceptance.

R32_SOURCE_COMPLETE=YES
R32_HOST_TESTED=PASS
R32_PROJECT_CHECK=PASS
R32_STRICT_DICTIONARIES=PASS
R32_PS2_COMPILE=PASS
R32_PS2_LINK=PASS
R32_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R32_PS2_PT_LOAD_CHANGED=YES
R32_MACHINE_EVIDENCE=GITHUB_ACTIONS
R32_INDEPENDENT_VALIDATION=NOT_RUN
R32_OPERATOR_OBSERVED=NO
R32_HARDWARE_QUALIFIED=NO
R32_HARDWARE_PENDING=YES
R32_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Accepted R33 Transport/MPEG enclosing-session abort authority

PACKET_ID=A003-TRANSPORT-MPEG-SESSION-ABORT-FENCE-R33
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0072
ASSIGNING_FOREMAN_STATE_COMMIT=75109b71226afd7e3798e6d2f70f8b846228d61c
ASSIGNING_FOREMAN_LOG_COMMIT=6410a5de2a6a17c94cd5d5c8011b7509dcdebdd7
RECONSTRUCTION_STARTING_COMMIT=6410a5de2a6a17c94cd5d5c8011b7509dcdebdd7
R33_FINAL_SOURCE_COMMIT=c48d460aa3d2e095d079289d07321f96151113a9
R33_RECONSTRUCTION_LOG_COMMIT=91f10ef8263beab9e40f89beb9d2ed5482888e80
R33_PRE_LOG_COMMIT_COUNT=17

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T194406-0400__reconstruction__a003-mpeg-generation__interactive.md`

### R33 criterion disposition

A003-R33-C1=MET
A003-R33-C2=MET
A003-R33-C3=MET
A003-R33-C4=MET
A003-R33-C5=MET
A003-R33-C6=MET
A003-R33-C7=MET
A003-R33-C8=MET
A003-R33-C9=MET
A003-R33-C10=MET
A003-R33-C11=MET
A003-R33-C12=MET

Independent Foreman findings:

1. `pstvnc_transport_session_begin_abort()` terminalizes the active runtime,
   requests receiver stop, wakes rider waiters through accepted Transport
   runtime behavior and proves receiver completion without reclaiming runtime
   storage.
2. `pstvnc_transport_session_abort_storage_retained()` proves an exact old
   access ticket still names that retained terminal runtime; it rejects stale,
   replaced, uncompleted or unproven authority.
3. Legacy `pstvnc_transport_session_abort()` preserves its complete one-shot
   contract by composing begin-abort with final close, including the
   establishment-only pre-runtime path.
4. Final Transport runtime release still refuses live receiver/waiter ownership.
   R33 therefore does not weaken the accepted lower-owner reclamation fence.
5. `pstvnc_app_mpeg_run_session_abort_service()` is an abnormal enclosing-
   session path owned by the exact old Application MPEG run. It is distinct from
   normal R23/R24 retirement/reveal.
6. Any exact P7 consumer claim is abandoned through the worker release seam
   before join; failure preserves the represented claim for a retry rather than
   fabricating release.
7. Safe worker stop is requested once for the exact generation after Transport
   terminality is proven. A still-running worker is pending progress, not
   timeout-success.
8. Worker completion, join, exact terminal outcome, worker release and
   `pstvnc_mpeg_ps2_worker_runtime_release()` are ordered and individually
   proven before SESSION_ABORT_READY.
9. The abnormal path emits no RETIRE/producer-done/finalize, does not thaw P2,
   and does not seal/reveal P3. STOPPED versus genuine FAILED worker outcome is
   retained as evidence.
10. SESSION_ABORT_READY is terminal for that run object; stale old-session
    authority cannot become a successor generation.
11. The complete Worker range stayed inside the R33 authorized
    Transport/Application-MPEG/tests/dictionary/lifecycle surface. Ordinary
    `app.c`, live binding install/action routing, UI/Config/Management/RFB,
    Pi and AUDIO product source were untouched.
12. Final focused and canonical machine evidence is green and bound to the exact
    final source authority.

### Exact R33 machine evidence

GitHub Actions run `36203058779`, attempt 1, checked out exact head SHA
`c48d460aa3d2e095d079289d07321f96151113a9` on
`ledge/h1-all-guns` and completed successfully.

Observed focused evidence includes:

- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- Transport bridge/session-abort host coverage PASS;
- `transport_mpeg_test: PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- P9/P10 and R21-R32 cross-domain regressions PASS.

Observed canonical evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=95a3828c42b7ec6c77aab87f6048500d9e77896874fea351714b053fd9b523ea`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c07ef5062c8d467d2075dfaa3162681924bfd1a344bd2a4d56c45516f3493812`
`PT_LOAD_BYTES=522388`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R33_SOURCE_COMPLETE=YES
R33_HOST_TESTED=PASS
R33_PROJECT_CHECK=PASS
R33_STRICT_DICTIONARIES=PASS
R33_PS2_COMPILE=PASS
R33_PS2_LINK=PASS
R33_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R33_MACHINE_EVIDENCE=GITHUB_ACTIONS
R33_INDEPENDENT_VALIDATION=NOT_RUN
R33_OPERATOR_OBSERVED=NO
R33_HARDWARE_QUALIFIED=NO
R33_HARDWARE_PENDING=YES
R33_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Blocked R34 partial Application composition authority

PACKET_ID=A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34
PACKET_STATUS=BLOCKED_NOT_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0073
ASSIGNING_FOREMAN_STATE_COMMIT=3076ac73fe4b9b72d1ece33f612faf359d900559
ASSIGNING_FOREMAN_LOG_COMMIT=39eef5de7061c43e76ad1a19d1d12f944b949106
RECONSTRUCTION_STARTING_COMMIT=39eef5de7061c43e76ad1a19d1d12f944b949106
R34_PARTIAL_SOURCE_COMMIT=6f1acf56d0007a05c081c0217f099a5fcbc4a754
R34_RECONSTRUCTION_LOG_COMMIT=488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4
R34_PRE_LOG_COMMIT_COUNT=21

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T203707-0400__reconstruction__a006-orchestration-shutdown__interactive.md`

### Independent R34 criterion disposition

A006-R34-C1=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C2=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C3=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C4=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C5=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C6=PARTIAL_BLOCKED
A006-R34-C7=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C8=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C9=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C10=BLOCKED
A006-R34-C11=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
A006-R34-C12=NOT_MET

No individual partial disposition above promotes R34 product behavior into an
accepted contract. R34 remains one incomplete packet and must be re-reviewed as
a whole after the prerequisite is accepted and the continuation closes all
criteria.

### Independent blocker findings

1. Accepted R21 orders run-open -> worker runtime -> worker start -> P3 arm ->
   P7 init -> START.
2. Its pre-START unwind is deliberately fail-closed. A worker stop/join/release,
   runtime release, presentation abort, Transport pre-start abort or partial
   worker-start cleanup failure may leave real owner state represented while the
   run becomes FAULTED with `session_teardown_required=1`.
3. The exact accepted fixture
   `test_cleanup_failure_faults_and_blocks_retry()` proves a started worker may
   remain unjoined with worker runtime and Transport run ownership while START
   was never invoked.
4. R33 currently requires the complete post-START owner shape:
   current generation, exact Transport ticket, Transport run open, worker
   runtime owned, started worker, initialized P7, armed P3 and
   `start_invoked=1`. It therefore correctly rejects the pre-START retained
   shape rather than guessing cleanup.
5. Independent review additionally confirms that worker start itself has one
   retained partial-owner case: thread create succeeds, thread start fails, the
   immediate destroy fails, and the worker remains initialized with a created
   but never-started thread/stack. R21 marks that state unsafe, but current R4
   public cleanup requires joined started-worker authority and cannot reclaim it.
6. Legacy one-shot Transport abort is not a substitute. Transport storage cannot
   be reclaimed first and then used as retroactive proof that the Application
   MPEG owner is dormant.
7. The correct owner split remains: MPEG worker owns safe partial-worker
   reclamation; Application MPEG run owns the exact retained run prefix and
   sequencing; Transport only supplies terminal old-session validity and final
   storage reclamation.

### Exact partial R34 machine evidence

At exact partial source authority
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`, GitHub Actions run
`36206592591` completed:

- host-unit: SUCCESS;
- ps2-compile: SUCCESS;
- ps2-link/current-source reproducibility: SUCCESS;
- project-check: FAILURE because the newly added R34 files lacked final topology
  local-file coverage;
- dictionary-long: FAILURE because the new R34 files/symbols were intentionally
  not reconciled after the blocker was discovered;
- dictionary-reconcile: SKIPPED.

Observed partial focused evidence includes
`APP_MPEG_PRODUCT_SOURCE_TEST=PASS`,
`APP_MPEG_PRODUCT_TEST=PASS`,
`app R15/R16B/R19/R27/R32/R34 tests: PASS`,
`app_mpeg_run_test: PASS`, P9/P10, R28-R33 and Transport/MPEG regressions.

Partial linked identity:

`ELF_PRISTINE_SHA256=8cc45464d0e8d71111677a865ddb5bca7f09c58013f1ad88e70f683b86533adc`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=185f1de7fd1bd658f350e3927bae3e7d69fb0733fb6194baeee37e7bd35ff8cd`
`PT_LOAD_BYTES=524820`

These bytes are **not accepted product authority** and are not hardware
qualified. The newest fully Foreman-accepted linked authority remains R33.

## Accepted R34P pre-START partial session dormancy authority

PACKET_ID=A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P
PACKET_STATUS=FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_STATE_REVISION=0074
ASSIGNING_FOREMAN_STATE_COMMIT=c12919cde5bef2c1a997810faa25181d7648da3f
ASSIGNING_FOREMAN_LOG_COMMIT=82cd9affd068ec835c1d2c1a930cedc4c1b95bdd
RECONSTRUCTION_STARTING_COMMIT=82cd9affd068ec835c1d2c1a930cedc4c1b95bdd
R34P_FINAL_SOURCE_COMMIT=c6e1dcf514adfc29296633374e75020b3480e5e0
R34P_RECONSTRUCTION_LOG_COMMIT=c5fdd7f1acdaa2dcb526a5113f2e538614f2d976
R34P_PRE_LOG_COMMIT_COUNT=13

The required immutable Reconstruction record is:

`docs/ledge/work-log/20260925T211157-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review accepts every R34P requirement:

A003-R34P-C1=MET
A003-R34P-C2=MET
A003-R34P-C3=MET
A003-R34P-C4=MET
A003-R34P-C5=MET
A003-R34P-C6=MET
A003-R34P-C7=MET
A003-R34P-C8=MET
A003-R34P-C9=MET
A003-R34P-C10=MET
A003-R34P-C11=MET
A003-R34P-C12=MET

Independent findings:

1. Exact reachable R21 pre-START cleanup prefixes are represented rather than
   collapsed into a generic teardown state.
2. `pstvnc_mpeg_worker_reclaim_unstarted()` accepts only the exact
   created-but-never-started partial worker and retries only thread destruction.
3. Failed partial destruction leaves initialized/thread-created/thread-id/stack
   ownership intact; successful destruction precedes stack release and clears
   initialized ownership only afterward.
4. Pre-START abnormal admission requires FAULTED + teardown-required + nonzero
   generation + exact old Transport ticket + START not invoked, and rejects
   live P7/retirement/restoration facts.
5. Exact retained Transport proof occurs before local worker/runtime reclaim.
6. Started workers retain proof-driven stop/status/join/outcome/release; a
   genuine still-running worker is pending progress rather than timeout success.
7. Reachable pre-START states cannot contain an Application-serviced P7 borrow;
   post-START R33 claim abandonment remains owner-driven.
8. PS2 worker runtime is released only after worker no-touch proof.
9. No abnormal pre-START path emits START/RETIRE/producer-done/finalize, thaws
   P2, seals/reveals P3, or fabricates normal MPEG completion.
10. SESSION_ABORT_READY remains terminal for the old run/session and preserves
    pre-START diagnostic identity.
11. Existing post-START R33 entry/ordering semantics remain intact.
12. Canonical branch-wide evidence is green at exact final source authority.

R34P_SOURCE_COMPLETE=YES
R34P_FOREMAN_ACCEPTED=YES
R34P_HOST_TESTED=YES
R34P_PROJECT_CHECK=PASS
R34P_STRICT_DICTIONARIES=PASS
R34P_PS2_COMPILE=PASS
R34P_PS2_LINK=PASS
R34P_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R34P_SOURCE_HEAD_MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_36208396947_ATTEMPT_1
R34P_INDEPENDENT_VALIDATION=NOT_RUN
R34P_OPERATOR_OBSERVED=NO
R34P_HARDWARE_QUALIFIED=NO

Exact linked branch identity at R34P source head:

`ELF_PRISTINE_SHA256=a7127b71ede10efa4a91e1914dc55a64bf75d6dffe7169df825d444c77615ccd`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`
`PT_LOAD_BYTES=525716`

This image includes still-unaccepted R34 partial Application source and is
therefore reproducible branch evidence only, not a fully Foreman-accepted
product image or hardware-qualified identity.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34C
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0075
BASED_ON_ACCEPTED_R34P_SOURCE=c6e1dcf514adfc29296633374e75020b3480e5e0
BASED_ON_R34P_LOG=c5fdd7f1acdaa2dcb526a5113f2e538614f2d976
BASED_ON_BLOCKED_R34_PARTIAL_SOURCE=6f1acf56d0007a05c081c0217f099a5fcbc4a754
BASED_ON_R34_BLOCKED_LOG=488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4
ORIGINAL_R34_CRITERIA=ALL_REOPENED_FOR_FINAL_ACCEPTANCE
NORMAL_MPEG_RETIREMENT_R23_R24=DEFERRED
ACTION_DRIVEN_RECALIBRATION=DEFERRED
AUDIO_ACTIVATION=DEFERRED
CONFIG_PERSISTENCE_EDITOR_RELOAD=DEFERRED

### Objective

Complete the originally assigned R34 ordinary MPEG product activation packet on
top of accepted R34P without expanding scope.

The already-landed partial R34 composition remains the implementation base.
R34C must add only the missing Application-owned distinction between:

- a live/post-START MPEG owner that requires accepted R33 before Transport
  release;
- a pre-START teardown-required R21 owner that requires accepted R34P before
  Transport release;
- no MPEG teardown owner, which retains accepted R16B one-shot Transport abort.

After this continuation, the Foreman must be able to re-evaluate all original
A006-R34-C1 through C12 as one complete packet.

### Required behavior

1. **Preserve exact R32 binding installation.** Every fresh R29 Input runtime
   receives only the immutable resident R32 desired binding snapshot before
   worker start. Zero binding remains zero and no default physical chord is
   introduced.
2. **Preserve truthful product-action context.** DESKTOP eligibility continues
   to reflect actual local UI/quarantine/calibration/run ownership, including
   held-gesture context changes. Do not weaken R28/R29 recognition semantics.
3. **Keep live-service and teardown predicates distinct.**
   `pstvnc_app_mpeg_product_has_started_run()` (or its exact replacement) must
   remain a predicate for a genuinely started/live R21 owner used by current-tick
   R22 service. Do not broaden it so a pre-START fault is accidentally passed to
   live frame service.
4. **Add one Application-owned abnormal-teardown predicate/seam.** Through
   public run status/owner seams, represent whether the attempt owns an MPEG run
   that must complete local abnormal dormancy before Transport release. It must
   include accepted post-START R33 owners and accepted pre-START R34P
   teardown-required owners, while excluding clean IDLE/zero-generation
   rollback and attempts that never acquired MPEG lifecycle ownership.
5. **P10 teardown-required results select R34P, not legacy abort.** When
   protected activation returns a teardown-required result and the run is an
   exact pre-START R34P owner, the Application failure path must preserve that
   fact through Input shutdown, Transport begin-abort/retained storage, repeated
   local session-abort service, and final Transport close. It must never relabel
   the attempt as no-MPEG merely because START was never invoked.
6. **Post-START failures retain R33 ordering.** A healthy started run or any
   post-START session failure still requires Input dormancy, Transport
   begin-abort, retained old-session proof, R33 local dormancy, media-clock
   binding release and final Transport close before replacement.
7. **No-MPEG R16B remains one-shot.** If no abnormal MPEG teardown owner exists,
   provider failure/fatal convergence retains the accepted one-shot
   `pstvnc_transport_session_abort()` path. Clean P10 rollback must not create
   false two-phase-abort debt.
8. **Input dormancy remains before dependent MPEG retirement.** If any pre-START
   R34P or post-START R33 owner exists, failed Input shutdown prevents
   Transport begin-abort/local MPEG retirement/replacement. Do not convert input
   failure into cleanup success.
9. **Abort service is proof-driven to terminal readiness.** Once two-phase abort
   begins, repeatedly service the exact old MPEG owner until
   SESSION_ABORT_READY without timeout-as-success; any owner error blocks
   replacement. Release the session media-clock binding only after local MPEG
   dormancy, then perform final Transport close/release.
10. **Preserve ordinary activation/live behavior.** Semantic
    `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION` remains the only routed action;
    P9 gets controller first refusal, cancel does not start MPEG, accepted
    protected calibration invokes P10 once, current-tick R22 service occurs only
    for a genuinely started run, and the first physical frame remains the sole
    P3/media-clock arm boundary.
11. **Preserve original R34 scope fences.** No normal R23/R24 same-session
    retirement/reveal, user-facing stop/recalibration, AUDIO, persistence/editor/
    reload, Pi/Wire production change, physical-mask logic in Application,
    default binding, generic timeout/watchdog or hardware-success claim.
12. **Close original R34 evidence as one packet.** Deterministic focused tests
    plus canonical host/project/strict-dictionary/pinned-PS2
    compile/link/current-source reproducibility must establish original
    A006-R34-C1 through C12 on the final combined R34C authority, including
    accepted R34P pre-START failure composition.

### Required deterministic evidence

Focused evidence must prove at minimum:

- zero desired binding still produces no routable configured gesture;
- one explicit nondefault configured binding installs byte-for-byte and
  publishes exactly one MPEG_CALIBRATION semantic action;
- DESKTOP-only admission is blocked by OSK/quarantine/calibration/live MPEG and
  by held context changes;
- GLOBAL recognition during owned calibration/live run is consumed without
  overlapping generation/start;
- idle semantic action begins P9 without Application physical-mask/timing
  knowledge;
- P9 controller first refusal and cancel-without-P10 remain intact;
- protected accept invokes P10 exactly once and successful start reaches exact
  WAIT_FIRST_FRAME;
- clean P10 pre-START rollback produces no abnormal MPEG teardown owner and
  preserves the ordinary no-MPEG/R16B path;
- a deterministic P10 pre-START CLEANUP_FAILED/teardown-required case selects
  Transport begin-abort -> accepted R34P local dormancy -> final Transport
  close, and never calls legacy one-shot abort first;
- the created-but-never-started R34P shape is routable through the same
  Application teardown selection without being passed to R22 live service;
- post-START WAIT_FIRST_FRAME and MPEG_OWNED failure/provider-loss cases select
  Transport begin-abort -> R33 -> final close;
- Input shutdown failure with either pre- or post-START MPEG owner prevents
  begin-abort/replacement;
- current-tick service is never called for a pre-START fault;
- clock remains unarmed until accepted P7/P4 first physical presentation;
- P2 remains frozen through protected/live MPEG;
- no ordinary RETIRE/R23/R24/reveal/recalibration appears in R34C product route;
- ordinary provider failure with no MPEG owner still follows accepted R16B
  replacement behavior.

### Authorized source surface

R34C may modify only the smallest justified subset of:

- `src/app.c/.h`;
- `src/app_mpeg_product.c/.h`;
- focused Application/R34 product-composition tests/stubs;
- directly affected Application dictionaries and topology/lifecycle docs;
- canonical test/build enrollment only if required by the continuation.

Consume but do not modify absent an independently demonstrated prerequisite
defect:

- `src/app_mpeg_run.c/.h` including accepted R34P;
- `src/mpeg/worker.c/.h`;
- `src/app_mpeg_activation.*`;
- `src/app_mpeg_calibration.*`;
- `src/app_mpeg_frame.*`;
- `src/app_product_bindings.*`;
- `src/input/*`;
- `src/transport/*`;
- `src/display/*`;
- `src/media/*`;
- `src/config/*`;
- `src/management/*`;
- RFB product source;
- Pi product source;
- AUDIO product source;
- H1/B4A forensic source.

If accepted public R34P/run-status seams are insufficient for exact Application
teardown selection, return BLOCKED naming the exact missing owner contract
rather than reaching into lower-owner private fields.

### Explicit non-goals

R34C does not:

- implement normal user-facing MPEG stop;
- implement action-driven recalibration of a live run;
- compose ordinary R23/R23C/R24 retirement/restoration/reveal;
- add AUDIO activation;
- add config persistence/editor/live reload;
- add a default physical binding;
- modify Pi MPEG production or Wire framing;
- add generic timeout/watchdog success;
- claim independent Validation, operator observation or hardware qualification.

### Required checks before handoff

Run focused final R34C Application composition evidence; all R28-R34 binding /
Input / config / management regressions; P9/P10; R21-R24/R33/R34P; R15/R16B/R19/
R27; canonical host tests; project check; complete strict dictionaries; pinned
PS2 compile/link and current-source reproducibility.

Record the exact final ELF/PT_LOAD identity if bytes change. Because R34 product
behavior is not accepted until the Foreman reviews the returned continuation,
classify those bytes hardware-pending.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` revision 0007 using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a006-orchestration-shutdown`;
- WORKER_KEY=`interactive`.

Then stop and return the baton.

FALLBACK=NONE__RETURN_BLOCKED_WITH_EXACT_MISSING_ACCEPTED_APPLICATION_OR_RUN_STATUS_CONTRACT
STRETCH=NONE__DO_NOT_ENTER_NORMAL_R23_R24_RETIREMENT_OR_RECALIBRATION

## Current hardware debt

Newest fully Foreman-accepted loadable product authority remains R33:

`ELF_PRISTINE_SHA256=95a3828c42b7ec6c77aab87f6048500d9e77896874fea351714b053fd9b523ea`
`PT_LOAD_SHA256=c07ef5062c8d467d2075dfaa3162681924bfd1a344bd2a4d56c45516f3493812`
`PT_LOAD_BYTES=522388`

Current branch source through accepted R34P plus unaccepted partial R34 is
reproducible as:

`ELF_PRISTINE_SHA256=a7127b71ede10efa4a91e1914dc55a64bf75d6dffe7169df825d444c77615ccd`
`PT_LOAD_SHA256=76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`
`PT_LOAD_BYTES=525716`

The latter image is not yet fully product-accepted and is not
hardware-qualified.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
