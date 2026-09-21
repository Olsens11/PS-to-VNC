# Reconstruction shift — A003 MPEG generation-control relay R6

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T07:12:23-04:00
COMPLETED_AT=2026-09-21T07:21:28-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a646b49ed8200ee127dd96f37d4e3359145c6607
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-MPEG-GENERATION-CONTROL-RELAY-R6`

The bounded objective was to reconstruct the PS2-side Transport-owned MPEG
generation-control relay so a later Application owner can send exact START and
RETIRE requests and consume the exact Pi RETIRE completion without bypassing the
sole physical-I/O owner.

R6 owns only Wire envelope/relay mechanics and bounded session-local control
storage.

It does not decide which MPEG generation is active, start or stop a decoder,
start or retire the Pi producer, publish MPEG producer completion, discard
retirement residual bytes, return retirement credit, arm Presentation, invoke
the P7 frame consumer, restore RFB, execute P5/Q7 retirement, or implement the
Pi production Wire/control side.

## Authority consumed

Consumed current authority at worker wake:

- Foreman State revision 0034;
- Reconstruction Contract revision 0006;
- work-log contract revision 0007;
- Wire Runtime Decisions revision 0011;
- Architecture Overlay revision 0004;
- A003 MPEG/generation audit revision 0001;
- current Transport protocol/runtime/bridge/physical-stream source and tests;
- frozen `h1_cp2p_retirement_control.py` evidence only for exact RETIRE
  request/completion wire semantics;
- current Q1-Q12 ownership, which supersedes the historical direct physical
  send structure.

Starting branch authority:

`a646b49ed8200ee127dd96f37d4e3359145c6607`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## Worker-role recovery note

The conversation is permanently the interactive Reconstruction worker.

Before source execution, repository authority was refreshed and showed that the
prior R5 worker result had already been accepted by a separate Foreman round and
R6 was the active packet. No Foreman-owned repository write was made by this
worker conversation before R6 execution. The exact worker execution start above
was captured after that role correction.

## Public Transport control-relay surface

Extended the existing Transport bridge with:

- `pstvnc_transport_mpeg_send_start()`;
- `pstvnc_transport_mpeg_send_retire()`;
- `pstvnc_transport_mpeg_take_retire_completion()`.

All three require the same current opaque `pstvnc_transport_access_t` already
used by other cross-Wire Transport operations.

The bridge validates the access ticket before invoking any runtime operation.
Therefore stale Session-A authority cannot:

- submit START into Session B;
- submit RETIRE into Session B;
- consume a RETIRE completion stored by Session B.

No Wire Session ID is exposed to the caller and no new module-facing session
identity was created.

## Sole physical-I/O outbound control path

Added runtime operations:

- `pstvnc_transport_runtime_mpeg_send_start()`;
- `pstvnc_transport_runtime_mpeg_send_retire()`.

Each operation:

1. requires an initialized MPEG-enabled Transport runtime;
2. encodes the caller-supplied shared protocol payload with the existing exact
   codec;
3. submits the complete control frame through
   `pstvnc_transport_runtime_submit_frame()`;
4. therefore waits on the existing bounded outbound rendezvous;
5. leaves the sole Transport I/O thread as the only caller of the physical
   framed-send primitive.

No Application or MPEG-domain code gained access to
`pstvnc_transport_physical_stream_send_frame()`.

The old inline convenience path:

`pstvnc_transport_physical_stream_send_mpeg_start()`

was removed from `src/transport/physical_stream.h`.

Its direct-helper protocol test was removed as well. Product callers therefore
have no accepted physical-stream START bypass around the runtime owner.

## Exact outbound START envelope

START uses the existing shared
`pstvnc_mpeg_start_payload_t` and
`pstvnc_mpeg_start_payload_encode()`.

The emitted frame is exactly:

- kind `PSTVNC_TRANSPORT_FRAME_MPEG_START` = 11;
- channel `PSTVNC_TRANSPORT_CHANNEL_CONTROL` = 0;
- flags = 0;
- payload size = `PSTVNC_MPEG_START_PAYLOAD_SIZE` = 44 bytes.

The relay validates only codec/envelope representation.

It does not validate whether calibration was accepted, whether the generation is
currently active, or whether producer startup is product-policy legal.

Focused host evidence decodes the exact bytes submitted through the runtime
outbound owner and proves equality with the caller payload.

## Exact outbound RETIRE envelope

RETIRE uses the existing shared
`pstvnc_mpeg_retire_payload_t` and
`pstvnc_mpeg_retire_payload_encode()`.

The emitted frame is exactly:

- kind `PSTVNC_TRANSPORT_FRAME_MPEG_RETIRE` = 10;
- channel control = 0;
- flags = 0;
- payload size = `PSTVNC_MPEG_RETIRE_PAYLOAD_SIZE` = 12 bytes.

Focused host evidence decodes the outbound runtime-owned frame and proves exact
payload equality.

## Exact inbound RETIRE-completion acceptance

The sole Transport receiver now recognizes one additional inbound frame class:

`PSTVNC_TRANSPORT_FRAME_MPEG_RETIRE`

Acceptance requires all of:

- MPEG enabled for the current Transport session;
- exact RETIRE kind;
- exact control channel;
- flags = 0;
- exact 12-byte payload length;
- successful v1 RETIRE payload decode.

Only after all checks succeed is the decoded payload eligible for the
session-local completion slot.

Malformed RETIRE is a Transport-session failure rather than media.

Focused tests independently reject:

- RETIRE on the MPEG DATA channel;
- nonzero RETIRE flags;
- wrong RETIRE payload length;
- unsupported RETIRE payload version.

## Inbound START remains invalid

Inbound kind-11 START receives no PS2-side acceptance path.

Even an otherwise exact START envelope remains unaccepted by the sole receiver
and therefore fails the Transport session.

The relay does not reinterpret inbound START as media or as a valid control
request.

## One bounded synchronized completion slot

The MPEG-enabled runtime now owns:

- one dedicated `mpeg_control_semaphore_id`;
- one `pstvnc_mpeg_retire_payload_t mpeg_retire_completion`;
- one `mpeg_retire_completion_pending` fact.

The dedicated count-1 semaphore separates receiver-thread control publication
from main/Application-thread control consumption without aliasing MPEG DATA
queue synchronization.

The receiver accepts a RETIRE completion only when the slot is empty.

If another valid RETIRE completion arrives before the first is consumed:

- the first value is not overwritten;
- the second is unaccepted;
- the Wire session fails closed.

There is no unbounded control queue.

## Nonblocking one-shot take

`pstvnc_transport_runtime_mpeg_take_retire_completion()` and its public bridge
wrapper are nonblocking.

For a healthy current session:

- no pending completion -> `PSTVNC_TRANSPORT_WOULD_BLOCK`;
- one pending completion -> exact value copied and slot cleared;
- second take -> `PSTVNC_TRANSPORT_WOULD_BLOCK`.

The operation does not wait for remote cleanup and does not infer whether the
payload matches an Application-owned pending run.

## Session-local lifetime

Runtime initialization begins from zeroed completion state.

The control semaphore and completion slot are owned by one Transport runtime
interval and disappear with runtime release.

Focused tests prove reinitializing the same runtime object begins with:

`mpeg_retire_completion_pending == 0`

The existing opaque access-ticket bridge proof additionally creates Session A,
closes it, creates Session B, and proves stale access A cannot call any R6
runtime control operation while fresh B remains usable.

## No generation business semantics

Neither runtime nor bridge compares:

- RETIRE session_id;
- RETIRE generation;
- START session_id;
- START generation

against a Transport-owned active MPEG generation.

Static final-source review finds no generation equality/inequality decision in
the R6 relay source.

Transport stores and relays the exact payload identity only.

A later Application owner must decide whether a received completion is the
completion expected for its pending MPEG retirement transaction.

## No synthetic producer completion

Receiving an exact RETIRE completion does not call:

`pstvnc_transport_mpeg_mark_producer_done()`

or the lower MPEG-channel producer-done operation.

Focused host evidence receives a valid RETIRE completion and then proves MPEG
status still reports:

`producer_done == 0`

Producer completion remains an explicit separate Transport fact that a later
higher owner may publish only after the ordered retirement fence is proven.

No decoder EOF is synthesized by R6.

## No residual or credit finalization

R6 adds no operation that:

- snapshots or discards MPEG retirement residual;
- returns residual credit;
- reopens MPEG storage for a successor run;
- clears a generation-specific media queue;
- finalizes Q7 retirement.

Existing ordinary DATA consumption continues to earn ordinary credit according
to the previously accepted channel policy.

Retirement residual semantics remain dependency-queued to the later Application
retirement transaction.

## Pure MPEG DATA remains opaque media

The receiver still classifies MPEG media by frame identity:

- kind DATA;
- channel MPEG2;
- flags 0;
- nonzero bounded payload.

R6 does not inspect DATA bytes for START- or RETIRE-shaped content.

Focused host evidence constructs the exact 44-byte START wire payload, sends
those bytes as DATA/channel4, receives them as ordinary MPEG media, and proves:

- runtime remains healthy;
- no RETIRE completion appears;
- all 44 bytes are returned unchanged by MPEG DATA consumption.

Therefore payload shape does not create a control side channel.

## Existing riders preserved

R6 changes no RFB/AUDIO/MPEG DATA queue format, credit policy, RFB quiesce
transaction, physical framing sequence, decoder feed API, MPEG worker API,
Presentation API, or Application runtime.

The complete host suite remains green, including:

- Transport protocol;
- Transport bridge;
- Transport runtime;
- Transport audio;
- Transport MPEG;
- MPEG decoder;
- MPEG worker;
- Application MPEG frame consumer.

## Focused test additions

`tests/unit/transport_mpeg_test.c` now proves:

- exact START submission through runtime;
- exact RETIRE submission through runtime;
- exact inbound RETIRE acceptance;
- WOULD_BLOCK before completion;
- one-shot take;
- no producer-done side effect;
- exact 44-byte START-shaped DATA remains media;
- fresh runtime completion slot is empty;
- wrong-channel RETIRE rejection;
- nonzero-flags RETIRE rejection;
- wrong-length RETIRE rejection;
- bad-version RETIRE rejection;
- inbound START rejection;
- duplicate pending completion fails closed without overwrite.

`tests/unit/transport_bridge_test.c` now proves:

- public START relay result mapping;
- public RETIRE relay result mapping;
- public completion take result mapping;
- stale Session-A access cannot invoke any R6 runtime operation after Session B
  becomes current;
- fresh Session-B access remains usable.

`tests/unit/transport_protocol_test.c` retains exact codec and frame-identity
proofs while deleting the now-invalid direct physical START-helper test.

## Build and source integration

No new product source file or directory was required.

R6 extends the already-earned Transport owners:

- `src/transport/runtime.{c,h}`;
- `src/transport/bridge.{c,h}`;
- `src/transport/physical_stream.h`.

The existing strict PS2 compile manifest already includes these files.

The existing current-source linked build already includes:

- `transport_runtime.o`;
- `transport_bridge.o`;
- `transport_physical_stream.o`.

No historical Issue #7 object authority was broadened.

## Dictionary integration

The behavior head correctly produced strict dictionary drift for:

- new runtime/bridge control-relay definitions;
- the removed direct physical START helper.

Canonical deterministic reconciliation changed only:

- `src/transport/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Transport dictionary count advanced:

`1014 -> 1062`

The reconciliation added the current control-relay definitions and removed the
four stale direct-helper definitions.

Maintained Transport responsibility prose was then updated to name:

- the sole physical-I/O owner;
- exact START/RETIRE envelope relay;
- one session-local pending RETIRE-completion slot;
- explicit separation between payload/session validity and Application-owned
  generation/producer lifecycle meaning.

## Commit chronology

R6 worker range after Foreman base
`a646b49ed8200ee127dd96f37d4e3359145c6607`:

1. `34f7c0b65378f58cf68cc22497a9884bbd1f57d3`
   — declare MPEG control-relay runtime state/API;
2. `f993ef300332cb2c83871d297841237cb157a477`
   — implement runtime control relay, exact receive validation and one-slot take;
3. `0dc2f5f6e75fa3b5e85315edc0154bca41429f1e`
   — expose public bridge control-relay declarations;
4. `cb9724481295e9d6d3ff3d8140ff7ada1a487e59`
   — implement public bridge control-relay operations;
5. `26423aaf7add846e93c96b7b76375301e33c6d06`
   — remove direct physical MPEG START bypass;
6. `f76fa0d930fd7b1784465a1ee93ba910f717be66`
   — remove obsolete direct-helper protocol fixture;
7. `2f89faed56a308ef06083bae976445a972311e71`
   — focused runtime generation-control relay tests;
8. `7f7f9c1d32cea6394431f49579feff6f08790945`
   — public access-fencing/control-relay bridge tests;
9. `29c1a97acefb37868b383d35d2e95306d8439f42`
   — tree-identical canonical dictionary-reconciliation trigger;
10. `8d9efa5bcc598191b17c92a4e8e6733574121d0b`
    — automated current-clean dictionary reconciliation;
11. `31db24ad56515d09a0218b1d9e20718c7bcf9662`
    — maintained Transport control-relay responsibility prose.

The multiple small source/test commits were produced because the connected
repository write surface imposes a bounded per-call orchestration limit. The
final authority is one coherent linear branch state and all canonical evidence
below is from that final state.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `src/transport/SYMBOLS.md`;
- `src/transport/bridge.c`;
- `src/transport/bridge.h`;
- `src/transport/physical_stream.h`;
- `src/transport/runtime.c`;
- `src/transport/runtime.h`;
- `tests/unit/transport_bridge_test.c`;
- `tests/unit/transport_mpeg_test.c`;
- `tests/unit/transport_protocol_test.c`.

No Application, MPEG decoder/worker/backend, Display, media-clock, RFB,
AUDIO-channel, MPEG-channel, Platform graphics, config, UI, input or Pi product
source changed.

## Behavior-head evidence

Final pre-dictionary behavior authority:

`7f7f9c1d32cea6394431f49579feff6f08790945`

Workflow:

`35593359640`

Observed results:

- host-unit PASS;
- project-check PASS;
- direct PS2 compile PASS;
- linked current-source reproducibility PASS;
- dictionary-long expected FAIL only for the newly added control-relay symbols
  and removed direct-helper rows.

No behavioral compile/test failure was hidden by reconciliation.

## Final coherent machine evidence

Final pre-log authority:

`31db24ad56515d09a0218b1d9e20718c7bcf9662`

GitHub Actions workflow:

`35593508080`

completed SUCCESS on the first attempt.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host logs explicitly report:

- `transport protocol tests passed`;
- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`.

Final direct PS2 compile explicitly reports:

- `PS2_COMPILE=src/transport/runtime.c`;
- `PS2_COMPILE=src/transport/bridge.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked evidence compiles and links the changed Transport objects in both
clean reproducibility builds and reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Final documentation/dictionary evidence reports:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

This remains repository/source/build evidence rather than physical Wire/MPEG
hardware qualification.

## A003 R6 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R6-C1 SOLE_IO_CONTROL_SEND — MET

START and RETIRE encode exact shared protocol payloads and enter only the
existing runtime outbound rendezvous. The sole Transport I/O thread remains the
physical send owner.

### A003-R6-C2 EXACT_START_ENVELOPE — MET

Outbound START is kind 11/channel 0/flags 0/exact 44-byte v1. Focused runtime
evidence decodes the submitted frame and proves exact caller-payload identity.

### A003-R6-C3 EXACT_RETIRE_ENVELOPE — MET

Outbound RETIRE is kind 10/channel 0/flags 0/exact 12-byte v1. Focused runtime
evidence decodes and compares the exact submitted value.

### A003-R6-C4 INBOUND_RETIRE_COMPLETION — MET

The sole receiver accepts only an exact v1 RETIRE completion envelope. Wrong
channel, flags, length and version fail closed. Inbound START remains invalid.

### A003-R6-C5 ONE_SLOT_NONBLOCKING_TAKE — MET

One dedicated synchronized pending slot exists per Transport runtime. Take is
WOULD_BLOCK when empty, returns one exact value once, and duplicate arrival
cannot overwrite a pending value.

### A003-R6-C6 SESSION_BOUND_ACCESS — MET

Every public R6 bridge operation is fenced by the current opaque Transport
access ticket. Stale Session-A access cannot send or consume Session-B control
state; fresh B remains usable.

### A003-R6-C7 NO_GENERATION_BUSINESS_STATE — MET

Transport stores/relays payload identity only. It performs no active-generation
comparison or Application run-policy decision.

### A003-R6-C8 NO_SYNTHETIC_PRODUCER_DONE — MET

RETIRE completion reception does not call producer-done publication and focused
status evidence proves producer_done remains false.

### A003-R6-C9 PURE_MPEG_DATA — MET

Nonempty DATA/channel4 remains opaque MPEG media. Exact 44-byte START-shaped
bytes pass through MPEG DATA unchanged and never create control state.

### A003-R6-C10 NO_DIRECT_PHYSICAL_BYPASS — MET

The public inline `pstvnc_transport_physical_stream_send_mpeg_start()` helper
and its direct-call fixture are removed. Product control sends use the runtime
owner.

### A003-R6-C11 EXISTING_RIDERS_PRESERVED — MET

Existing RFB/AUDIO/MPEG DATA, credit, quiesce, decoder/worker and P7 host
coverage remains green.

### A003-R6-C12 CLEAN_EVIDENCE — MET

Focused/full host tests, strict dictionaries, project checks, direct R5900
compile and current-source linked reproducibility all pass on exact final
pre-log authority.

Physical qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A003_R6_REQUIRED_REPOSITORY_MACHINE_GATES
A003_R6_PS2_CONTROL_RELAY_COMPILED_AND_LINKED=YES
PI_PRODUCT_WIRE_CONTROL_OWNER=NOT_IMPLEMENTED
PI_PRODUCT_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
PI_START_VALIDATION_PREPARED_GENERATION_OWNER=NOT_IMPLEMENTED
PI_RETIRE_CLEANUP_ACK_PRODUCT_PATH=NOT_IMPLEMENTED
MPEG_ACTIVATION_REQUIRED_VALUE_AUTHORITY=INCOMPLETE
APPLICATION_RUN_GENERATION_ALLOCATION=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM_FROM_CALIBRATION=NOT_IMPLEMENTED
R5_R4_LIVE_WORKER_START=NOT_IMPLEMENTED
P7_WIRED_INTO_LIVE_APP_LOOP=NO
PRODUCER_DONE_PUBLICATION_FROM_RETIRE_TRANSACTION=NOT_IMPLEMENTED
MPEG_RETIRE_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
MPEG_RETIRE_CREDIT_FINALIZATION=NOT_IMPLEMENTED
P5_Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
RFB_SUPPRESSION_RESTORATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_PS2_MPEG_CONTROL_RELAY=NOT_RUN_NOT_CLAIMED
PHYSICAL_PI_RETIRE_ACK=NOT_RUN_NOT_CLAIMED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`31db24ad56515d09a0218b1d9e20718c7bcf9662`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- sole-I/O START/RETIRE submission;
- exact START envelope;
- exact RETIRE envelope;
- exact inbound RETIRE completion validation;
- inbound START rejection;
- synchronized one-slot/no-overwrite completion storage;
- nonblocking one-shot take;
- stale Session-A send/take fencing against Session B;
- no generation-business comparison;
- no producer-done side effect;
- no residual/credit finalization;
- exact 44-byte MPEG DATA preservation;
- removal of direct physical START bypass;
- final host/direct-PS2/link/dictionary evidence.

The Foreman, not this worker, chooses the next Pi/control, validated-value,
Application activation or retirement packet.

This Reconstruction worker must stop after this R6 handoff.
