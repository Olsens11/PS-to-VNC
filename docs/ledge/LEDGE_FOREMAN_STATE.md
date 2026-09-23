# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0048
RECORDED_AT=2026-09-22T21:22:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0047
SUPERSEDES_FOREMAN_STATE_REVISION=0047
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0048 independently accepts `A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`
at source authority `3310568e6c35ef59d77ee6075f1f9ea5a561476e` and consumes its
immutable Reconstruction closeout `856a1065f5941c5149518ffe2a322bc81fe91642`.

Application now treats the accepted R16A CONNECT, READ/EOF and WRITE provider
terminal causes as explicit recovery-policy inputs. A failed ordinary-RFB
attempt admits no further provider-bound input/RFB work; replacement is allowed
only after input shutdown proves worker dormancy and Transport abort proves the
sole receiver/session retired. The replacement uses ordinary startup and fresh
network/Q4/Transport/RFB/framebuffer/input authority. Generic physical/RFB I/O
failure remains distinct and fatal under this bounded policy.

The next dependency is not another RFB repair. Maintained PS2 source already
contains the accepted MPEG START/RETIRE Transport relay and MPEG logical channel,
while maintained Pi product source still explicitly contains no MPEG rider or
producer. Revision 0048 therefore activates the bounded Pi-side MPEG
generation-control/producer ownership packet before final Application MPEG
orchestration.

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

`A003_R15_INTEGRATED__R16A_PROVIDER_FAILURE_REPRESENTATION_ACCEPTED__R16B_RFB_RECOVERY_FOREMAN_ACCEPTED__PI_MPEG_CONTROL_PRODUCER_RECONSTRUCTION_ACTIVE__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

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
PI_MPEG_CONTROL_PRODUCER_OWNER=RECONSTRUCTION_ACTIVE
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

## Next dependency decision

Current maintained PS2 authority already provides:

- exact MPEG START kind 11 on control channel 0, flags zero, 44-byte payload;
- exact MPEG RETIRE kind 10 on control channel 0, flags zero, 12-byte payload;
- MPEG generation-control version 1 and big-endian payload codecs;
- logical MPEG DATA on channel 4;
- MPEG queue/credit/activity state and initial channel-4 credit;
- exact RETIRE-completion reception through Transport;
- the accepted Configuration-owned MPEG runtime profile.

Maintained Pi product source still explicitly excludes MPEG control, MPEG DATA
scheduling and an MPEG producer. Historical H1 code proves useful mechanisms
(exact prepared generation, exact capture geometry, one producer, bounded
buffering, generation-specific emission leases and retirement ordering), but
that experimental class/process structure is reference evidence, not a merge
target.

The smallest dependency before final Application orchestration is therefore a
maintained Pi-side exact-generation owner that consumes the already-accepted PS2
control representation and relays producer bytes through the existing sole Wire
owner. It must preserve one physical connection and must not invent a second
session/generation protocol.

## Governing invariants for R17

1. `WireConnectionOwner` remains the sole Pi physical receive/send/sequence
   owner. MPEG producer/control code never reads or writes the PS2-facing socket.
2. Current PS2 MPEG START/RETIRE bytes are authority. R17 mirrors them exactly;
   it does not revive older experimental START-as-DATA representations or add a
   new generation tag to every MPEG DATA frame.
3. START is exact active-Wire-session and exact nonzero-generation state. One
   generation may be prepared/live/retiring at a time; stale or mismatched
   session/generation control fails at the smallest safe scope.
4. The accepted base rectangle is the Pi capture rectangle. Suppression is the
   exact generation-specific outer footprint. Inner matte remains PS2-local and
   is never sent to or reconstructed by the Pi.
5. Producer emission starts only after exact-generation capture/suppression
   preparation succeeds and only while channel-4 credit permits. Pi memory must
   remain bounded; lack of credit is backpressure, not permission to accumulate
   an unbounded encoded stream.
6. Every MPEG DATA frame is serialized by the existing sole Wire owner and
   consumes only channel-4 credit. RFB credit/state remains independent.
7. Retirement closes new generation-N emission admission first, waits for any
   already-admitted physical-send lease to finish, then retires capture/
   suppression/producer ownership. RETIRE completion is emitted only after that
   exact Pi retirement proof succeeds.
8. A stuck or failed producer may fail closed; a timeout may detect failure but
   must never be treated as proof that live ownership retired successfully.
9. The dead generation is never rebound. Generation N+1 receives fresh producer,
   capture/suppression and local queue/buffer authority after N is fully retired.
10. Existing Q4/R16A/R16B RFB recovery, R13 quiesce, R14 RFB values and accepted
    PS2 MPEG Transport/decoder/presentation mechanisms remain unchanged.
11. R17 does not perform final PS2 Application MPEG activation, calibration UI
    policy, first-frame ownership promotion, decoder/presentation orchestration,
    AUDIO activation, heartbeat or final all-guns composition.
12. A003/A004 exact-generation geometry/profile facts remain singular authority.
    If Pi encoding requires a value not presently available without duplicating
    or inventing product tuning, Reconstruction must introduce the smallest
    owner-correct projection from accepted Configuration authority or return
    BLOCKED; it may not silently copy laboratory knobs.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-PI-MPEG-CONTROL-PRODUCER-R17
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0048
BASED_ON_ACCEPTED_R16B_SOURCE=3310568e6c35ef59d77ee6075f1f9ea5a561476e
BASED_ON_R16B_LOG=856a1065f5941c5149518ffe2a322bc81fe91642

### Objective

Reconstruct the smallest maintained Pi-side MPEG generation-control and producer
boundary required by the already-accepted PS2 MPEG Transport relay. The Pi must
decode/validate exact START, prepare one exact generation, own its bounded
capture/encoder lifetime, emit MPEG DATA only through channel 4 under Transport
credit and sole-Wire serialization, and complete exact RETIRE only after the
generation's Pi-owned capture/suppression/producer work is proven retired.

This packet productizes the missing Pi mechanism only. It does not wire final
Application MPEG policy or claim presentation/decoder hardware qualification.

### Required behavior

1. **Exact symmetric control codec.** Add/complete maintained Pi protocol support
   for the current PS2 START/RETIRE representations exactly: kind 11/10, control
   channel 0, zero flags, generation-control version 1, 44/12-byte big-endian
   payloads. Malformed or wrong-envelope control is rejected.
2. **Session/generation validation.** START must match the active authoritative
   Q4 session identity, use a nonzero fresh generation, and pass exact geometry
   bounds/alignment/containment checks against injected/current desktop geometry.
3. **One exact prepared/live owner.** Only one generation may be prepared/live/
   retiring. Duplicate, older, mismatched-session or otherwise stale START/
   RETIRE cannot replace current authority.
4. **Exact capture/suppression preparation.** Derive capture from START base
   geometry and suppression from START outer geometry. Preparation must finish
   before emission admission opens; failed preparation leaves no falsely-live
   generation.
5. **Bounded producer ownership.** Launch/own one generation-scoped producer with
   bounded buffering/backpressure and explicit stop/retirement outcome. Preserve
   exact producer failure rather than converting it into successful EOF.
6. **Channel-4 credit and sole-owner send.** Accept MPEG credit only for channel
   4; producer code publishes bytes to the Wire owner rather than sending
   physically. The Wire owner allocates global sequence and sends DATA/channel4
   in bounded fragments no larger than accepted Wire payload limits.
7. **Exact emission lease/fence.** New generation-N emission closes atomically
   when retirement begins. RETIRE waits for every already-admitted N send lease
   to finish before producer/capture retirement and completion publication.
8. **Retirement completion means real Pi retirement.** RETIRE completion echoes
   the exact accepted version/session/generation only after producer, capture and
   generation-specific suppression are no longer live. Failure to prove that
   state fails closed and emits no false completion.
9. **Fresh repeated generation.** After successful retirement, a later fresh
   generation can start with new producer/capture/suppression/buffer authority;
   old local bytes, leases, credit bookkeeping or status cannot contaminate it.
10. **Preserve other riders/contracts.** Existing RFB attachment, R16A ERROR, R13
    quiesce, Q4 compatibility and singular R14 profile behavior remain intact;
    MPEG activity does not create a second physical owner or cross-rider credit.
11. **No final-orchestration expansion.** Do not activate PS2 MPEG worker/
    decoder/presentation policy, calibration acceptance, first-frame ownership,
    AUDIO, CONFIG delivery, heartbeat, direct-RFB fallback, or all-guns
    Application orchestration.
12. **Document maintained ownership.** Update Pi README/symbol dictionaries and
    the smallest directly affected architecture/development record so exact
    producer/control/retirement ownership is recoverable without H1 history.

### Acceptance criteria

- A003-R17-C1 PI_MPEG_CONTROL_CODEC_EXACTLY_MATCHES_ACCEPTED_PS2
- A003-R17-C2 START_VALIDATES_ACTIVE_SESSION_GENERATION_AND_GEOMETRY
- A003-R17-C3 ONE_EXACT_GENERATION_OWNER_STALE_CONTROL_REJECTED
- A003-R17-C4 CAPTURE_AND_SUPPRESSION_PREPARED_FOR_EXACT_GENERATION
- A003-R17-C5 PRODUCER_LIFETIME_AND_BUFFERING_BOUNDED_FAIL_CLOSED
- A003-R17-C6 CHANNEL4_CREDIT_AND_SOLE_WIRE_SERIALIZATION_PRESERVED
- A003-R17-C7 RETIRE_CLOSES_ADMISSION_AND_DRAINS_INFLIGHT_SEND_LEASES
- A003-R17-C8 RETIRE_COMPLETION_ONLY_AFTER_REAL_PI_RETIREMENT
- A003-R17-C9 REPEATED_GENERATION_USES_FRESH_AUTHORITY_NO_STALE_REBIND
- A003-R17-C10 RFB_Q4_R16A_R13_R14_AND_PS2_MPEG_CONTRACTS_UNCHANGED
- A003-R17-C11 NO_APPLICATION_PRESENTATION_AUDIO_HEARTBEAT_OR_FINAL_SCOPE_CREEP
- A003-R17-C12 HOST_PROJECT_DICTIONARY_AND_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance. A truthful BLOCKED result
is permitted if current Configuration/desktop/suppression owner seams cannot
supply a required fact without inventing duplicate authority; identify the exact
missing interface rather than importing H1 scaffolding or hard-coding an
unowned value.

### Required evidence

R17 must preserve deterministic evidence for at least:

1. byte-for-byte Pi/PS2 START and RETIRE codec symmetry plus malformed cases;
2. active-session mismatch, zero/stale/repeated generation, geometry alignment,
   bounds and suppression-containment rejection;
3. exactly one prepared/live/retiring generation and no replacement while live;
4. capture base geometry distinct from outer suppression geometry;
5. producer launch failure and producer retirement failure remaining fail-closed;
6. initial/returned channel-4 credit bounding DATA emission and independent RFB
   credit behavior;
7. sole Wire send/sequence ownership for MPEG DATA and RETIRE completion;
8. retirement racing an in-flight emission lease, proving completion orders
   strictly after that lease and real producer cleanup;
9. no RETIRE completion on unproven/stuck producer retirement;
10. generation N retirement followed by clean N+1 startup with no stale bytes,
    lease, credit, suppression or producer identity reused;
11. unchanged Q4/RFB/R13/R14 and current PS2 MPEG control tests;
12. canonical host tests, project check, complete strict dictionaries and
    pinned PS2 compile/link/current-source reproducibility if PS2/build inputs
    change.

### Authorized source surface

R17 may modify only the smallest justified subset of:

- `pi/wire_protocol.py`;
- `pi/wire_server.py` and/or `pi/wire_runtime.py` for sole-owner MPEG dispatch,
  credit and serialization mechanics;
- new maintained `pi/` MPEG generation/producer/capture-suppression source files
  when a separate responsibility is genuinely earned;
- `pi/README.md` and `pi/SYMBOLS.md`;
- directly affected Pi/protocol/integration fixtures;
- directly affected development/architecture documentation and dictionary/
  topology/build tooling;
- Configuration projection source/tooling only if required to expose an already-
  accepted MPEG value to the Pi without duplicating authority. Selected values
  may not be retuned and CONFIG-on-Wire may not be broadened.

PS2 Application, MPEG decoder/worker/backend, Display/Presentation, Input/UI,
RFB product source, AUDIO product source, R13/R14 semantics and final all-guns
orchestration are not authorized by R17.

### Required checks before handoff

Run the canonical host tests and project check, complete strict source-dictionary
audit, focused Pi MPEG protocol/generation tests, and pinned PS2 compile/link/
current-source reproducibility whenever linked/build-input bytes change. Preserve
exact linked identity/PT_LOAD evidence for any changed PS2 bytes.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

R16B's current linked bytes and R16A/R16B recovery behavior remain hardware
unqualified. R17 Pi source will likewise begin as source/host/machine evidence
only. Historical H1 producer/generation hardware results are mechanism
provenance, not qualification transfer to reconstructed Pi product source.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
