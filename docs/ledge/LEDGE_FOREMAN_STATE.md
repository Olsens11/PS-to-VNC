# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0049
RECORDED_AT=2026-09-22T23:04:48-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0048
SUPERSEDES_FOREMAN_STATE_REVISION=0048
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0049 independently accepts `A003-PI-MPEG-CONTROL-PRODUCER-R17`
at source authority `cc7dc1237957bfd288addc8379caae47e83bc5a6` and consumes its
immutable Reconstruction closeout `8698890b4f839d5708e6cefc129c37608643e155`.

The maintained Pi now has the exact dormant MPEG generation-control/producer
mechanism required by the accepted PS2 START/RETIRE representation while
`WireConnectionOwner` remains the only PS2-facing receive/send/sequence owner.
R17 also projects the already-selected A003-R7 MPEG values from one canonical
machine-readable Configuration record without retuning them or broadening
CONFIG-on-Wire.

Independent review found the next dependency before Application MPEG activation:
the PS2 Transport MPEG queue still lacks the accepted A003 old-run/new-run
boundary. It can relay START/RETIRE and receive exact RETIRE completion, but it
does not yet close channel-4 DATA admission on completion, discard residual
old-run bytes, return withheld credit, clear finite-producer state, or reopen
the same session-scoped queue only after finalization.

Revision 0049 therefore activates a bounded Transport-owned MPEG run-boundary
packet before final Application orchestration.

No reconstructed R17 Pi mechanism or current R16A/R16B PS2 recovery path is
hardware-qualified by this state.
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

`A003_R16B_RFB_RECOVERY_ACCEPTED__R17_PI_MPEG_CONTROL_PRODUCER_FOREMAN_ACCEPTED__R18_MPEG_TRANSPORT_RUN_BOUNDARY_RECONSTRUCTION_ACTIVE__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

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
MPEG_TRANSPORT_RUN_BOUNDARY=RECONSTRUCTION_ACTIVE
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

## Next dependency decision

R17 closes the maintained Pi producer/control-owner gap, but current PS2
Transport still lacks one A003-required session-local MPEG run boundary.

Current accepted Transport can:

- open a session with logical MPEG storage and initial channel-4 credit;
- consume DATA/channel4 into one bounded queue;
- return credit for decoder-consumed bytes using `mpeg_credit_pending`;
- publish finite producer completion;
- relay exact START and RETIRE control;
- receive and expose one exact Pi RETIRE completion.

It cannot yet:

- explicitly open one MPEG DATA-admission interval before START;
- close channel-4 DATA admission atomically when RETIRE completion is accepted;
- reject post-completion DATA as a protocol violation;
- keep retirement latched after the completion value is taken;
- discard residual old-run queue bytes after the local consumer has retired;
- return the combined residual + already-withheld credit through the existing
  Transport send owner;
- clear old finite-producer/queue state and reopen admission for a later run.

A003 and the qualified historical generation-boundary record require exactly
those mechanics. The boundary is a session-scoped queue epoch, not a second
socket, per-packet generation tag, generic module-generation system, or
Transport-owned product activation policy.

Therefore the smallest prerequisite before Application MPEG composition is a
Transport-only run-boundary packet.

## Governing invariants for R18

1. Transport remains owner of physical Wire validity, channel-4 queue, credit
   accounting and DATA admission mechanics; it does not own user activation,
   calibration, MPEG region meaning, decoder/presentation policy, or Pi producer
   lifecycle.
2. One session-scoped MPEG queue is reused across runs. Do not add a second
   queue, socket, receive owner, per-DATA generation field or generic global
   module-generation manager.
3. A higher owner explicitly opens the MPEG run boundary before START. Initial
   open requires a clean idle MPEG channel: no residual bytes, no prior
   producer-done state, no retirement latch and no withheld old-run credit.
4. If START cannot be admitted/sent after opening, the higher owner can abort the
   unopened/failed run boundary back to a proven clean state without claiming
   producer retirement that did not occur.
5. While a run boundary is open, ordinary channel-4 DATA remains accepted under
   the existing bounded queue/credit rules.
6. Acceptance of the exact Pi RETIRE completion atomically closes channel-4
   DATA admission before the completion becomes visible to Application. Because
   TCP/Wire order is singular, all earlier accepted N DATA is already consumed
   or resident in N's queue at that point.
7. Any channel-4 DATA received after exact RETIRE completion and before a later
   explicit clean reopen is a protocol violation and fails the current Wire
   runtime; it is never treated as N+1 data.
8. Taking/observing the RETIRE completion does not reopen MPEG DATA admission.
   Retirement remains latched until the higher owner has retired the exact local
   MPEG consumer and explicitly finalizes Transport.
9. Finalization runs only with no MPEG activity waiter/consumer still owning the
   queue. Under the existing MPEG queue synchronization it discards all residual
   bytes, resets queue/finite-producer state, and records discarded bytes as
   discarded—not decoder-consumed.
10. Finalization returns all owed selected-path MPEG credit exactly once through
    Transport's existing sole outbound path: already-batched `mpeg_credit_pending`
    plus residual discarded queue bytes. It must detect overflow/failure and
    must not double-return bytes already credited during normal consumption.
11. Only after successful residual/credit finalization may the retirement latch
    clear and a fresh later MPEG run boundary open. Old queue, producer-done,
    activity-wait and retirement state cannot leak into the successor run.
12. R18 preserves RFB/AUDIO/Q4/R16A/R13/R14/R17 contracts, exact START/RETIRE
    bytes, decoder/worker/presentation mechanisms and Application policy. It
    adds no final composition and claims no hardware qualification.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
BASED_ON_FOREMAN_STATE_REVISION=0049
BASED_ON_ACCEPTED_R17_SOURCE=cc7dc1237957bfd288addc8379caae47e83bc5a6
BASED_ON_R17_LOG=8698890b4f839d5708e6cefc129c37608643e155

### Objective

Implement the missing PS2 Transport-owned MPEG run boundary around the already
accepted session-scoped channel-4 queue: explicit clean open/abort, immediate
DATA-admission closure on exact RETIRE completion, residual/credit finalization
after consumer retirement, and clean reopen for a successor run.

This packet supplies mechanism required by later Application orchestration. It
does not activate MPEG in `app.c` and does not own decoder, Presentation, Pi
producer or calibration policy.

### Required behavior

1. **Explicit clean open.** Expose the smallest public Transport bridge operation
   that opens one MPEG DATA-admission interval only from a proven clean idle
   channel state. Do not allocate product generation identity in Transport.
2. **Pre-START abort.** Expose a bounded abort/reset path for an opened boundary
   that has not reached a valid live transaction, proving no residual queue,
   pending retirement or producer-done state is silently carried forward.
3. **Admission-gated DATA.** MPEG DATA is accepted only while the run boundary is
   open and not retirement-latched; invalid timing is a protocol failure.
4. **Completion closes first.** Exact RETIRE completion acceptance closes DATA
   admission under Transport authority before publishing/storing the completion
   for the higher owner.
5. **Completion observation is not reopen.** Taking the exact completion may
   clear the one completion value slot but must leave the retirement boundary
   closed/latched.
6. **Residual discard primitive.** Add the owner-correct MPEG queue primitive
   needed to discard all residual bytes atomically while preserving bounded
   storage invariants. Residual discard is not decoder consumption.
7. **Exact credit finalization.** Finalization returns residual bytes plus any
   already-withheld MPEG credit exactly once through the existing Transport
   outbound/sequence owner, respecting overflow/failure semantics.
8. **Generation-local reset.** Successful finalization clears old queue offsets/
   byte count, producer-done state, completion/retirement latch and generation-
   local activity state required for safe reuse, without recreating the Wire
   Session or MPEG queue allocation.
9. **Safe consumer fence.** Finalization must fail closed if an MPEG activity
   waiter or other Transport-visible consumer state proves the channel is still
   in use; it may not delete/wake/reuse live synchronization ownership as a
   shortcut.
10. **Fresh successor.** After successful finalization, a later explicit open
    admits a fresh run; tests must prove old residual bytes, producer-done,
    completion, pending credit and activity state cannot appear in that run.
11. **Preserve neighboring contracts.** No change to fixed Wire framing, exact
    START/RETIRE payloads, RFB/AUDIO behavior, Pi R17 source, decoder safe-stop,
    worker/compositor/presentation policy or R16B recovery.
12. **Documentation/dictionaries/build evidence.** Update directly affected
    Transport lifecycle docs/dictionaries and preserve focused host, canonical
    project, strict dictionary, pinned PS2 compile/link and current-source
    reproducibility evidence.

### Acceptance criteria

- A003-R18-C1 MPEG_RUN_BOUNDARY_OPENS_ONLY_FROM_CLEAN_IDLE_TRANSPORT
- A003-R18-C2 PRE_START_ABORT_RESTORES_PROVEN_CLEAN_BOUNDARY
- A003-R18-C3 MPEG_DATA_ADMISSION_REQUIRES_OPEN_NONRETIRING_RUN
- A003-R18-C4 RETIRE_COMPLETION_ATOMICALLY_CLOSES_DATA_ADMISSION
- A003-R18-C5 COMPLETION_TAKE_DOES_NOT_REOPEN_CHANNEL
- A003-R18-C6 RESIDUAL_QUEUE_DISCARD_IS_ATOMIC_AND_NOT_CONSUMPTION
- A003-R18-C7 RESIDUAL_PLUS_PENDING_CREDIT_RETURNED_EXACTLY_ONCE
- A003-R18-C8 FINALIZATION_RESETS_OLD_RUN_TRANSPORT_STATE
- A003-R18-C9 LIVE_WAITER_OR_UNPROVEN_CONSUMER_BLOCKS_FINALIZATION
- A003-R18-C10 FRESH_SUCCESSOR_RUN_CANNOT_OBSERVE_OLD_RUN_STATE
- A003-R18-C11 RFB_AUDIO_WIRE_R17_AND_MEDIA_OWNER_CONTRACTS_UNCHANGED
- A003-R18-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN

All twelve criteria must be MET for source acceptance. If current Transport
synchronization cannot prove a safe finalization boundary without Application-
owned knowledge not yet exposed, return a truthful BLOCKED record naming the
missing seam instead of inventing timeout/sleep/diagnostic authority.

### Required deterministic evidence

R18 must prove at least:

1. DATA before explicit run-open is rejected/fails the runtime;
2. clean open from initial session state succeeds exactly once;
3. duplicate open and open with dirty/residual/retirement state fail closed;
4. pre-START abort returns to clean idle only when no run data/state exists;
5. valid DATA remains bounded/credit-accounted while admission is open;
6. exact RETIRE completion closes admission before higher-owner observation;
7. DATA after completion is a protocol failure even after completion is taken;
8. wrapped residual queue discard returns the exact discarded byte count;
9. finalization combines pending consumed-credit debt plus residual discarded
   bytes without duplicate return and emits that credit through the existing
   sole Transport sender;
10. producer-done/exhaustion and activity state from N do not survive finalize;
11. N finalize -> N+1 open/read cycle contains only N+1 bytes/state;
12. existing RFB/AUDIO, MPEG control relay, Q4 and R17-related contract tests
    remain green.

### Authorized source surface

R18 may modify only the smallest justified subset of:

- `src/transport/mpeg_channel.c` / `.h`;
- `src/transport/runtime.c` / `.h`;
- `src/transport/bridge.c` / `.h`;
- directly affected Transport MPEG unit/integration fixtures;
- `src/transport/SYMBOLS.md`, directly affected development/architecture notes,
  source dictionaries, compile/link manifests or check tooling as required.

`src/app.c`, Pi product source, MPEG decoder/worker/PS2 backend, Display/
Presentation, calibration/Input/UI, RFB product source and AUDIO product source
are not authorized by R18.

### Required checks before handoff

Run focused MPEG Transport run-boundary tests, canonical host tests, project
check, complete strict source-dictionary audit, pinned PS2 compile/link and
current-source reproducibility. Preserve exact new ELF/PT_LOAD identity if
loadable bytes change.

At shift end emit exactly one immutable Reconstruction record under
`docs/ledge/work-log/` following revision 0007, then stop and return the baton.

## Current hardware debt

R17's Pi generation/producer mechanism is source/host/machine evidence only and
has not been physically activated or qualified. R16A/R16B recovery remains
hardware-unqualified. Any PT_LOAD change introduced by R18 creates new exact-
identity hardware debt; unchanged PT_LOAD would not upgrade existing debt.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO
