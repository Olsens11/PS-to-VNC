# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0047
RECORDED_AT=2026-09-22T15:43:20-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0046
SUPERSEDES_FOREMAN_STATE_REVISION=0046
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0047 independently accepts the completed
`A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A` source baton and activates the
bounded downstream recovery packet `A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`.
R16A supplies the missing typed RFB-provider terminal fact that caused original
R16 to stop truthfully `BLOCKED`: Pi provider CONNECT, READ/EOF, and WRITE
failure can now cross the existing Wire owner seam to PS2 without falsely
turning the still-healthy physical Wire Session into a Transport failure.

The original R16 remains historically `BLOCKED_FOREMAN_CONFIRMED`; revision 0047
does not rewrite that outcome into completion. R16A is the accepted prerequisite
that resolves its representation blocker. R16B owns the remaining Application
failure-convergence, complete-stop-before-restart, and fresh-authority recovery
work.

No current-source hardware qualification is claimed by this state.

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

`A003_R15_INTEGRATED__R16_BLOCKER_CONFIRMED__R16A_PROVIDER_FAILURE_REPRESENTATION_FOREMAN_ACCEPTED__R16B_RFB_FAILURE_STOP_RESTART_POLICY_RECONSTRUCTION_ACTIVE__PI_MPEG_CONTROL_PRODUCER_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

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
RFB_QUIESCE_WAKE_DEFECT=CLOSED
RFB_SHARED_RUNTIME_PROFILE=FOREMAN_ACCEPTED
RFB_SESSION_COMPOSITION_CONFIG=SHARED_PROFILE_FOREMAN_ACCEPTED
APPLICATION_RFB_ACTIVATION=FOREMAN_ACCEPTED
APPLICATION_ACTIVATION=RFB_ONLY_FOREMAN_ACCEPTED
RFB_PROVIDER_FAILURE_REPRESENTATION=FOREMAN_ACCEPTED
RFB_FAILURE_RESTART_POLICY=RECONSTRUCTION_ACTIVE
PI_MPEG_CONTROL_PRODUCER_OWNER=DEPENDENCY_QUEUED
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

## Governing invariants for R16B

1. Application owns product recovery policy. RFB, Transport and the Pi attachment
   expose mechanism/domain facts and retirement operations; none may silently
   acquire retry/reconnect/restart policy.
2. The accepted R16A provider-terminal representation is frozen for R16B.
   Header/framing version 1, Q4 product compatibility 2, ERROR=7/channel-1/4-byte
   reason semantics and malformed-frame rules must not be redesigned here.
3. Provider failure remains distinct from physical Wire failure. Application may
   deliberately choose teardown as policy, but mechanism must not falsify which
   domain actually failed.
4. Once a typed provider-terminal fact reaches Application, the failed ordinary
   RFB attempt admits no new RFB/input work and converges monotonically toward
   stop.
5. Complete stop is an observed owner-state fact, not elapsed time. Input,
   current RFB use, Transport receive/runtime ownership, the current physical
   descriptor/session, and all attempt-scoped authority must reach their proper
   retirement boundaries before replacement authority is admitted.
6. The dead R13 Pi attachment is never rebound. Recovery that needs ordinary RFB
   again must acquire a fresh attachment through fresh accepted authority.
7. If the old Wire/Application session is retired to recover RFB, restoration
   uses a fresh network connection and fresh Q4 Wire Session identity. The old
   session identity is never relabeled as the new attempt.
8. No stale Transport access ticket, channel credit, sequence state, provider
   descriptor, private wake descriptor, RFB parser/session state, framebuffer
   validity, published input state or input-worker state may become authority for
   the replacement attempt.
9. A successfully completed stop does not imply successful restart. A fresh
   connect/Q4/attachment/RFB startup must independently succeed before ordinary
   RFB is healthy again.
10. Delays/backoff may only pace explicit recovery operations; no timeout, sleep,
    poll count or retry count is proof that an old owner retired or a new owner
    became healthy.
11. Existing R13 finite quiesce semantics and the singular selected R14
    32768/8192/16384/63 profile remain unchanged. R16B must not duplicate or
    retune those values.
12. No AUDIO, MPEG, CONFIG-on-Wire, heartbeat/liveness, direct-RFB fallback or
    final all-guns orchestration is authorized by this packet.
13. Current-source host/build evidence is not PS2 hardware qualification.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0047
BASED_ON_ACCEPTED_R16A_SOURCE=961ad59d82b1c865b9d330a7dd9cdc1ed1e32528
BASED_ON_R16A_LOG=a307de050c015a46548647b45ac9f145b8ba8e71

### Objective

Complete the recovery-policy work that original R16 could not lawfully perform
before R16A existed. Make the accepted typed RFB-provider terminal cause reach an
explicit Application-owned recovery decision, stop the failed ordinary-RFB
attempt completely, and restore ordinary RFB only through fresh session-scoped
authority with no stale-state reuse.

R16B is an Application recovery/lifecycle packet. It is not permission to alter
the accepted R16A Wire contract, R13 quiesce mechanism, R14 tuning, media
activation, or final all-guns orchestration.

### Required behavior

1. **Application observes the typed terminal cause.** CONNECT, READ/EOF and WRITE
   provider terminal results exposed by accepted R16A must reach an explicit
   Application recovery-policy branch. Do not turn them back into an anonymous
   physical-I/O failure merely to reuse an old fatal path.
2. **Close admission immediately for the failed attempt.** After Application has
   the terminal RFB fact, no new controller/keyboard/mouse publication, RFB
   update request, or other provider-bound ordinary-RFB work may be admitted to
   that failed attempt.
3. **Converge monotonically to complete stop.** Drive the attempt's actual owners
   through their documented shutdown/release boundaries. Do not restart a failed
   subcomponent inside its still-live old attempt as a shortcut.
4. **Prove input retirement.** Any input worker/runtime belonging to the failed
   attempt must be shut down to its owned completion boundary before a later
   attempt can publish fresh input authority.
5. **Prove Transport/session retirement.** If recovery retires the containing
   Wire session, Application must use the existing Transport owner seam so the
   sole receiver is interrupted, proven complete, and released before a new
   physical/Q4 session is admitted. Application never directly races Transport
   for descriptor ownership.
6. **Fresh authority on restoration.** A recovery attempt must acquire a fresh
   network connection, fresh accepted Q4 session identity, fresh Transport
   runtime/access authority, fresh Pi R13 attachment/factory result, freshly
   initialized RFB parser/session state, fresh framebuffer validity, and fresh
   input-runtime/publication state as applicable to that attempt.
7. **No dead-authority rebind.** Never relabel/reuse the failed attachment, old
   Q4 identity, old Transport ticket/credit/sequence state, old provider/wake
   descriptors, old parser state, old framebuffer validity or old input-worker
   state as replacement authority.
8. **Keep failure domains truthful.** Provider-local terminal cause and genuine
   physical Wire/Transport failure remain distinguishable mechanism facts even
   if Application chooses a common outer teardown operation for both. R16B must
   not weaken R16A's `runtime->failed` distinction.
9. **Restart success requires fresh proof.** Do not report restored ordinary RFB
   until the new attempt has independently completed the required connect/Q4/RFB
   startup and reached the same ordinary healthy boundary used by normal startup.
10. **No success by delay.** A delay/backoff may pace retry attempts if the clean
    Application policy genuinely needs one, but it cannot prove retirement,
    provider health, Wire acceptance, or successful RFB startup. Do not use a
    timeout to force-delete a possibly-live owner.
11. **Preserve accepted lower-layer contracts.** R16A ERROR/Q4 compatibility,
    R13 finite quiesce, one physical Wire owner and the selected R14 profile stay
    byte-for-byte/semantically unchanged except for test fixtures that must
    observe them. Do not introduce another protocol marker/version or copy the
    selected tuning literals into recovery policy.
12. **No media scope.** Do not activate AUDIO, MPEG START/RETIRE, Pi MPEG
    production, decoder/presentation recovery, CONFIG delivery, heartbeat, or
    final orchestration.
13. **Stable repeated-attempt evidence.** Tests must prove at least one complete
    typed-provider-failure -> stop -> fresh-attempt -> ordinary-RFB-success cycle
    and a repeated cycle or equivalent deterministic fixture demonstrating stale
    attempt authority cannot contaminate the next attempt.
14. **Keep docs/dictionaries/build boundary current.** Update directly affected
    lifecycle/development documentation and symbol dictionaries, and preserve
    canonical host/project/dictionary/PS2 compile/link evidence for linked source
    changes.

### Acceptance criteria

- A003-R16B-C1 TYPED_PROVIDER_FAILURE_REACHES_APPLICATION_POLICY
- A003-R16B-C2 FAILED_ATTEMPT_ADMISSION_CLOSES_BEFORE_TEARDOWN
- A003-R16B-C3 INPUT_RFB_TRANSPORT_COMPLETE_STOP_PROVEN
- A003-R16B-C4 DEAD_ATTEMPT_AUTHORITY_NEVER_REBOUND
- A003-R16B-C5 RESTORATION_USES_FRESH_NETWORK_Q4_TRANSPORT_AND_ATTACHMENT_AUTHORITY
- A003-R16B-C6 RFB_FRAMEBUFFER_INPUT_STATE_FRESH_ON_RESTART
- A003-R16B-C7 STALE_TICKET_CREDIT_SEQUENCE_PROVIDER_WAKE_AND_INPUT_STATE_CONTAINED
- A003-R16B-C8 PHYSICAL_WIRE_FAILURE_REMAINS_DISTINCT_FROM_PROVIDER_FAILURE
- A003-R16B-C9 NO_SUCCESS_BY_DELAY_OR_IMPLICIT_COMPONENT_RESTART
- A003-R16B-C10 R16A_R13_R14_LOWER_LAYER_CONTRACTS_UNCHANGED
- A003-R16B-C11 NO_MEDIA_PROTOCOL_HEARTBEAT_OR_RETUNE_SCOPE_CREEP
- A003-R16B-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance. If current owner seams
cannot prove complete stop/fresh authority without adding a missing bounded
lifecycle interface, Reconstruction may add the smallest packet-local interface
within Application/RFB/Transport ownership or return `BLOCKED` with the exact
conflicting authority. It must not hide uncertainty behind sleeps, forced owner
deletion, or protocol redesign.

### Required evidence

R16B must preserve deterministic executable/source evidence that distinguishes
at least:

1. all three accepted typed provider causes reaching the Application policy seam;
2. no new ordinary-RFB/input admission after the terminal cause is observed;
3. input-worker shutdown completion before replacement input authority;
4. Transport sole-receiver/session shutdown completion before fresh Q4 authority;
5. old versus new Q4 session identity and old versus new attempt-scoped
   Transport/RFB/input state;
6. fresh Pi attachment/factory allocation for the replacement Wire Session,
   using existing accepted R15/R13 authority or a direct regression fixture;
7. stale old access/credit/sequence/provider/wake/parser/framebuffer/input state
   being rejected, unreachable, or otherwise incapable of contaminating the new
   attempt;
8. provider-local failure remaining distinct from genuine physical Wire failure;
9. fresh ordinary-RFB startup success after complete retirement;
10. at least one repeated recovery cycle or equivalent deterministic
    generation/attempt-fencing proof;
11. unchanged R16A exact representation/Q4-v2 compatibility, unchanged R13
    quiesce semantics and singular R14 selected profile;
12. canonical host tests, project check, strict dictionaries and pinned PS2
    compile/link/current-source reproducibility for the exact final source.

### Authorized source surface

R16B may modify only the smallest justified subset of:

- `src/app.c` / `src/app.h`;
- `src/rfb/rfb_session.c` / `src/rfb/rfb_session.h` only if Application needs a
  narrow public lifecycle/status seam not already exposed by R16A;
- `src/rfb/bridge.c` / `src/rfb/bridge.h` only for the same narrow public seam;
- `src/transport/bridge.c` / `src/transport/bridge.h` and
  `src/transport/runtime.c` / `src/transport/runtime.h` only if a missing
  explicit stop/status boundary is proven necessary for Application-owned
  complete-stop evidence; do not alter accepted R16A protocol mechanics;
- directly affected Application/RFB/Transport test fixtures;
- directly affected source-symbol dictionaries and lifecycle/development docs;
- Pi test fixtures only where needed to prove existing fresh-per-Wire-session
  attachment authority; Pi product RFB/Wire mechanism is not open for redesign.

A need to change `pi/wire_protocol.py`, `src/transport/protocol.*`, R13 quiesce
bytes, R14 selected values, AUDIO/MPEG product source, or final orchestration is
outside this packet and requires a new Foreman decision.

### Required checks before handoff

Run the repository's canonical host tests and project check, the complete strict
dictionary audit, and pinned PS2 compile/link/current-source reproducibility when
linked source changes. Preserve exact new linked identity/PT_LOAD evidence if
bytes change.

At shift end, emit exactly one immutable Reconstruction work log under
`docs/ledge/work-log/` following revision 0007 of its governing contract. Return
the baton only after that record exists.

## Current hardware debt

R16A source/build acceptance does not qualify its new linked PS2 bytes, Q4-v2
compatibility, provider-terminal behavior, or any future R16B recovery behavior
on physical hardware. Existing earlier hardware evidence remains historical
provenance for the exact binaries/workloads that produced it.

Current source may continue under this debt because
`HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO`; no later state may silently upgrade
CI, compile, link or reproducibility evidence into `HARDWARE_QUALIFIED`.