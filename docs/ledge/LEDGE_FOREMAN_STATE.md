# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0046
RECORDED_AT=2026-09-22T09:36:47-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0045
SUPERSEDES_FOREMAN_STATE_REVISION=0045
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0046 independently confirms the truthful `BLOCKED` result returned by
`A003-RFB-FAILURE-STOP-RESTART-POLICY-R16`. The worker made no product-source
change and obeyed R16's explicit stop rule: accepted R13 provider connect,
provider EOF/read, and provider write failures are local terminal attachment
facts, but the currently accepted cross-Wire contract contains no unambiguous
provider-failure representation that can deliver that fact to the PS2 RFB /
Application side while keeping the physical Wire Session healthy.

The blocker is architectural/mechanical, not a failed implementation attempt.
Closing the physical Wire socket would falsely convert rider-local provider
failure into Wire failure. Existing zero-length channel-1 DATA is already exact
R13 REQUEST/BOUNDARY/COMMIT/COMPLETE authority. The maintained Pi protocol has
no provider-failure codec or dispatch path. The PS2 runtime has no RFB-local
terminal-frame receive path. The numeric `ERROR=7` frame kind exists as a
historical/dormant framing reservation only; repository evidence does not assign
it a payload, channel rule, reason vocabulary, dispatch behavior, or compatibility
meaning. It is therefore evidence/candidate surface for Reconstruction to assess,
not Foreman authority to declare its semantics.

Because Reconstruction Contract revision 0006 keeps protocol mechanics,
lifecycle interfaces, and product behavior with Reconstruction, Foreman does
not choose the missing bytes or compatibility/version design here. Instead this
revision activates one bounded prerequisite packet that owns that engineering
decision under the already-governing failure-domain, sole-owner, and
complete-stop-before-restart constraints.

No current-source hardware qualification is claimed by this state.

## Current Foreman phase

`A003_R15_INTEGRATED__R16_BLOCKER_CONFIRMED__RFB_PROVIDER_FAILURE_REPRESENTATION_RECONSTRUCTION_ACTIVE__RFB_FAILURE_STOP_RESTART_POLICY_BLOCKED_PENDING_R16A__PI_MPEG_CONTROL_PRODUCER_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

ARCHITECTURE_BLOCKER=RFB_PROVIDER_FAILURE_REPRESENTATION_MISSING
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
RFB_FAILURE_RESTART_POLICY=BLOCKED_PENDING_R16A
RFB_PROVIDER_FAILURE_REPRESENTATION=RECONSTRUCTION_ACTIVE
PI_MPEG_CONTROL_PRODUCER_OWNER=DEPENDENCY_QUEUED
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## R16 blocker disposition

PACKET_ID=A003-RFB-FAILURE-STOP-RESTART-POLICY-R16
PACKET_STATUS=BLOCKED_FOREMAN_CONFIRMED
ASSIGNING_FOREMAN_STATE_REVISION=0045
ASSIGNING_FOREMAN_COMMIT=903c80694af0f13bec0134594a1824664e002df5
RECONSTRUCTION_STARTING_COMMIT=84e88377b55228dcf6fa0af8601550c05639ea6f
RECONSTRUCTION_BLOCKER_LOG_COMMIT=a957b9069be93398e1bd0e761bbe92da0e22a088
R16_PRODUCT_SOURCE_CHANGED=NO

The required Reconstruction blocker record is:

`docs/ledge/work-log/20260922T085807-0400__reconstruction__a003-mpeg-generation__interactive.md`

Independent Foreman review confirms:

1. R13 already makes provider connect failure, provider EOF/read failure and
   provider write failure terminal to the session-scoped Pi attachment without
   giving that attachment physical-Wire ownership or restart policy.
2. `pi/wire_server.py` remains the sole Pi physical send/receive/sequence owner.
   Its RFB path currently understands ordinary channel-1 DATA/CREDIT plus the
   exact R13 zero-length quiesce exchange, not a provider-failure fact.
3. `pi/wire_protocol.py` has no provider-failure representation or reason codec.
4. PS2 `src/transport/runtime.c` has no RFB-provider-terminal dispatch. Treating
   an unknown frame as generic Transport failure would collapse the required
   distinction between rider failure and physical Wire/session failure.
5. `src/transport/protocol.h` contains the numeric frame-kind reservation
   `PSTVNC_TRANSPORT_FRAME_ERROR = 7`, but historical/current source assigns it
   no product semantics. Its existence alone is not authority to use it.
6. The Pi source already distinguishes fixed framing/header version from the
   negotiated Wire/product compatibility values, while current PS2 establishment
   source uses the existing Transport version in Q4. Any new incompatible
   semantics therefore require explicit, symmetric compatibility reasoning rather
   than an accidental one-sided interpretation.
7. Existing Q4 identity, channel numbers and R13 quiesce meanings remain intact.
8. The worker correctly made no speculative protocol edit and returned `BLOCKED`
   under R16 requirement 10.

At exact blocker authority `a957b9069be93398e1bd0e761bbe92da0e22a088`,
GitHub Actions run `35732117886` completed SUCCESS. The blocked result is thus a
canonically green evidence baton, not a CI/build failure.

R16_SOURCE_COMPLETE=NO
R16_BLOCKER_VALID=YES
R16_PRODUCT_CHANGE=NONE
R16_HARDWARE_QUALIFIED=NO

The original R16 acceptance criteria remain unresolved where they require the
missing cross-Wire fact. R16 is not accepted as completed behavior and must not
resume until R16A establishes an accepted representation/interface authority.

## Governing invariants for R16A

1. One physical PS2↔Pi Wire connection and one physical receive/send owner per
   Wire Session remain authoritative. No RFB provider/attachment may acquire or
   bypass that ownership.
2. RFB provider failure remains an RFB/channel-local mechanism fact. Correctly
   reporting it must not, by itself, declare the physical Wire Session failed.
3. Q4 remains the sole establishment/compatibility gate for an active Wire
   Session. If the representation requires incompatible peer semantics, the
   compatibility boundary must be deliberate, symmetric and fail before peers
   disagree about an ACTIVE session.
4. Existing R13 zero-length channel-1 DATA REQUEST/BOUNDARY/COMMIT/COMPLETE
   representation is frozen and must not be overloaded with provider failure.
5. Application owns product recovery policy. R16A supplies only the missing
   protocol/interface mechanism needed to report the fact; it does not implement
   Application retry/restart/reconnect policy.
6. A dead R13 attachment remains terminal and is never rebound. R16A may expose
   its failure, but may not restart it in place.
7. Complete-stop-before-restart, stale-access rejection and fresh-session
   authority remain governing downstream recovery obligations.
8. Transport may perform channel-local mechanism reporting/bookkeeping but may
   not parse or acquire RFB domain policy.
9. A delivered failure fact never erases the original local failure merely
   because its report was transmitted successfully.
10. No generic timeout, sleep, poll count, systemd restart or peer disconnect may
    stand in for typed provider-failure semantics.
11. The accepted R14 runtime values remain singular Configuration authority and
    may not be copied/retuned by this packet.
12. Current-source host/CI/build evidence does not constitute PS2 hardware
    qualification.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0046
BASED_ON_BLOCKED_R16_AUTHORITY=a957b9069be93398e1bd0e761bbe92da0e22a088

### Objective

Reconstruct the smallest explicit cross-Wire mechanism that reports terminal
RFB-provider mechanism failure from the accepted Pi R13 attachment to the PS2
RFB-facing boundary while preserving a healthy physical Wire Session as a
separate fact. The packet must define the missing representation/interface and
its compatibility rules rigorously enough that downstream Application recovery
can later consume it without guessing.

R16A owns the bounded protocol/interface engineering decision that R16 was
forbidden to invent. Foreman intentionally does not prescribe the exact bytes,
reason encoding, use/non-use of the dormant `ERROR=7` reservation, or version
number. Reconstruction must derive and document the smallest coherent design
from current repository authority and prove it symmetrically on Pi and PS2.

### Required behavior

1. **Typed provider-terminal visibility.** Provider-connect failure,
   provider EOF/read failure and provider-write failure must cross the Wire
   boundary as an unambiguous RFB-local terminal mechanism fact and become
   observable at the PS2 RFB-facing seam.
2. **Preserve specific cause.** Where current mechanism evidence can distinguish
   the first terminal cause, preserve that first/specific cause through the
   reporting seam rather than collapsing every provider failure into generic
   physical-I/O failure.
3. **Sole-owner transmission.** Any Pi report must be serialized by the existing
   sole Wire physical send/sequence owner. The attachment/relay may publish a
   local fact to that owner but may not send on the physical socket itself.
4. **Wire remains distinct.** Correct delivery/receipt of an RFB provider-terminal
   fact must not itself close, rebind, or mark the physical Wire Session failed.
   Genuine Wire/Transport failure must remain separately observable.
5. **Do not overload R13 quiesce.** Existing zero-length channel-1 DATA
   REQUEST/BOUNDARY/COMMIT/COMPLETE bytes and meanings remain exact and must not
   acquire a second failure meaning.
6. **No ambiguous DATA/CREDIT reuse.** Do not encode terminal provider failure by
   a payload/value that an accepted peer can lawfully interpret as existing
   RFB DATA, CREDIT, quiesce or other established traffic.
7. **Resolve dormant framing vocabulary deliberately.** Inspect the historical
   `ERROR=7` reservation and either define a complete, symmetric, bounded
   channel-local contract for it or reject it with repository evidence and use
   another bounded representation. The numeric reservation alone is not semantic
   authority.
8. **Explicit compatibility/version contract.** Reconcile the current Pi/PS2
   versioning model. If the chosen provider-failure semantics are incompatible
   with an accepted Wire-v1 peer, introduce the smallest deliberate compatibility
   boundary and prove incompatible peers reject before ordinary ACTIVE-session
   traffic can be interpreted differently. Do not casually change fixed header
   framing merely because product semantics evolve.
9. **PS2 wait termination as RFB-local fact.** A PS2 RFB read/poll/session path
   blocked or waiting for provider-supplied RFB bytes must be able to observe the
   provider-terminal condition and converge as an RFB-local failure, not wait
   indefinitely and not misclassify it as physical Wire failure.
10. **Contain late/stale RFB payload.** Already credited/in-flight channel-1
    payload from the failed attachment must remain bounded to the old session
    scope. R16A must not create any mechanism by which stale old provider bytes
    or status can be rebound into a future attachment/session.
11. **No recovery-policy expansion.** Do not implement Application retry,
    reconnect, backoff, fresh-Q4 session creation, attachment restart or complete
    R16 recovery. Those resume only after this representation is Foreman accepted.
12. **No media or unrelated protocol expansion.** Do not activate AUDIO, MPEG,
    CONFIG delivery, heartbeat/liveness, direct-RFB fallback, or unrelated Wire
    control facilities. Do not retune R14 values.
13. **Document ownership and compatibility.** Update the smallest directly
    affected architecture/component/development records and dictionaries so the
    new fact's producer, Wire owner, consumer, failure-domain meaning and
    compatibility boundary are recoverable without conversational memory.
14. **Keep implementation bounded.** Avoid wholesale H1/checkpoint imports or
    opportunistic refactors. Prefer the smallest clean source surface that makes
    the new representation complete and testable.

### Acceptance criteria

- A003-R16A-C1 CONNECT_FAILURE_CROSSES_AS_TYPED_RFB_FACT
- A003-R16A-C2 EOF_READ_FAILURE_CROSSES_AS_TYPED_RFB_FACT
- A003-R16A-C3 WRITE_FAILURE_CROSSES_AS_TYPED_RFB_FACT
- A003-R16A-C4 FIRST_SPECIFIC_PROVIDER_FAILURE_PRESERVED
- A003-R16A-C5 WIRE_SESSION_REMAINS_DISTINCT_AND_NOT_FALSELY_FAILED
- A003-R16A-C6 SOLE_WIRE_SEND_RECEIVE_AND_SEQUENCE_OWNERSHIP_PRESERVED
- A003-R16A-C7 NORMAL_R13_QUIESCE_BYTES_AND_MEANING_UNCHANGED
- A003-R16A-C8 COMPATIBILITY_VERSION_CONTRACT_EXPLICIT_AND_SYMMETRIC
- A003-R16A-C9 PS2_RFB_WAIT_TERMINATES_WITH_RFB_LOCAL_FAILURE
- A003-R16A-C10 LATE_CREDITED_RFB_BYTES_CONTAINED_NO_REBIND
- A003-R16A-C11 NO_APPLICATION_RESTART_PROTOCOL_MEDIA_TIMEOUT_OR_RETUNE_SCOPE_CREEP
- A003-R16A-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance. A valid BLOCKED result is
still permitted if repository evidence proves no bounded representation can
satisfy the governing failure-domain/compatibility invariants; such a result
must identify the exact conflicting authority rather than weakening it.

### Required evidence

R16A must preserve executable/source evidence that distinguishes at least:

1. provider connect failure from provider EOF/read failure and provider write
   failure where those causes are mechanically distinguishable;
2. the exact Pi producer-side terminal fact and the exact PS2 consumer-side
   decoded/status result;
3. normal RFB DATA from provider-terminal reporting;
4. normal R13 finite quiesce from provider-terminal reporting;
5. RFB-provider failure from genuine physical Wire failure;
6. ordered publication through the sole Pi Wire send/sequence owner;
7. unchanged single physical receive ownership on both sides;
8. compatibility/version acceptance and rejection across the chosen semantic
   boundary, including old/new peer mismatch if the representation is not
   backward compatible;
9. a waiting PS2 RFB path terminating on the typed provider-local fact rather
   than hanging or receiving a generic Transport failure;
10. containment/rejection of stale old channel/session state after terminal
    reporting;
11. exact singular R14 selected values and absence of media/direct-RFB scope;
12. canonical host tests, project check, strict dictionaries and pinned PS2
    compile/link/current-source reproducibility when linked bytes change.

New linked bytes are identity evidence only and remain hardware-unqualified.

### Authorized source surface

R16A may modify only the smallest justified subset of:

- `pi/rfb_relay.py`;
- `pi/rfb_attachment.py`;
- `pi/wire_protocol.py`;
- `pi/wire_server.py`;
- `src/transport/protocol.h` / `src/transport/protocol.c`;
- `src/transport/runtime.h` / `src/transport/runtime.c`;
- `src/transport/bridge.h` / `src/transport/bridge.c` only for channel-local
  mechanism/status exposure;
- `src/rfb/bridge.h` / `src/rfb/bridge.c`;
- `src/rfb/rfb_session.h` / `src/rfb/rfb_session.c` only to expose the terminal
  RFB-local result to its owning boundary;
- directly affected unit/integration fixtures, build manifests, source-symbol
  dictionaries and component/architecture documentation.

`src/app.*` recovery/restart behavior is not authorized by R16A. If a tiny
Application compile/test adaptation becomes mechanically unavoidable solely
because an existing public return type changes, keep it compatibility-only and
record why; do not implement recovery policy.

### Non-goals / forbidden expansion

Do not:

- implement the downstream R16 Application restart/reconnect loop;
- rebind or retry a failed R13 attachment in place;
- close the physical Wire solely to communicate provider failure;
- create a second physical sender, receiver or sequence owner;
- overload R13 zero-length quiesce markers;
- silently reinterpret existing peers under the same compatibility identity;
- add heartbeat/liveness, generic timeout authority or systemd-restart policy;
- activate MPEG/PCM/audio/CONFIG delivery;
- reopen a direct-RFB PS2-facing product socket;
- retune/copy the R14 profile;
- claim hardware qualification from CI/build evidence.

### R16A stop rule

Stop after R16A. Emit exactly one immutable Reconstruction work-log record under
work-log contract revision 0007 and return the baton to the Foreman. Do not
resume R16 Application failure/stop/restart policy in the same shift even if
R16A is successful; Foreman must independently accept the representation first.

## Deferred dependency graph after R16 blocker confirmation

1. complete and independently accept `A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A`;
2. resume/repacketize the bounded Application RFB failure/stop/restart policy
   using the accepted R16A representation;
3. add Pi MPEG START/RETIRE control ownership behind the established Wire/rider
   architecture;
4. reconstruct Pi MPEG producer exact-run admission and one-way retirement;
5. compose accepted MPEG runtime profile/calibration into Application activation;
6. complete later all-guns integration, machine/hardware qualification and
   endurance work under fresh authority.

## Hardware qualification debt

HARDWARE_PENDING=R15 ordinary default RFB product activation and linked PS2 bytes; R14 shared profile linked bytes; corrected R13 attachment/quiesce/wake; R12 internal provider endpoint; R11 provider authority; R10 RFB Relay; R9 Q4 client; R8 Pi Wire service; any future R16A representation and linked PS2 bytes; downstream RFB recovery; reconstructed A003 MPEG runtime; MPEG repeated-run stale fencing; Wire-loss during MPEG; current-Q7 RFB restoration; A004 visible handoff; all-guns endurance; exact final product ELF

## Foreman next pickup

Consume the immutable Reconstruction return for
`A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A`. Independently verify the chosen
representation and compatibility boundary against both Pi and PS2 source, prove
provider-local failure remains distinct from physical Wire failure and R13
quiesce, inspect exact CI/build evidence, and either accept the prerequisite and
issue the downstream recovery packet or publish a bounded corrective/blocker
packet. Do not infer success merely from the presence of a numeric `ERROR=7`
constant or a passing build.