# Reconstruction shift — A003 PS2 Wire Session establishment R9

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T10:55:24-04:00
COMPLETED_AT=2026-09-21T12:32:47-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=16c1e24fd56df10a035dbd96287cc9ca33d1d1b5
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only active Foreman packet:

`A003-PS2-WIRE-SESSION-ESTABLISHMENT-R9`

R9 reconstructs the PS2 Transport-owned product Q4 Wire Session client so a
fresh Platform-created TCP descriptor can become one ACTIVE product Wire Session
before any rider is activated.

Required packet properties were:

- Transport, not Application or Platform, owns Q4;
- exact sequence-1 HELLO and exact sequence-1 ACCEPT/NOT_ACCEPTED;
- typed ESTABLISHED / NOT_ACCEPTED / mechanism-failure outcomes;
- explicit descriptor ownership transfer with no double-close ambiguity;
- same physical lineage continues at send sequence 2 / receive sequence 2;
- Pi-assigned nonzero session identity remains Transport-private;
- public Wire availability is only INACTIVE/ACTIVE;
- an ACTIVE Wire Session may remain idle with no rider startup or credit;
- ordinary rider activation cannot bypass Q4;
- Session B starts a fresh provisional sequence-1 exchange and cannot inherit
  Session A Transport access;
- existing RFB/AUDIO/MPEG rider mechanics remain preserved;
- no Pi rider, MPEG producer, CONFIG, heartbeat, Application MPEG activation,
  Presentation arm, Q7 finalization, reconnect policy, or physical
  qualification is added.

## Authority consumed

Truthful shift start was captured as:

`2026-09-21T10:55:24-04:00`

Live pickup authority:

`16c1e24fd56df10a035dbd96287cc9ca33d1d1b5`

Foreman had consumed and accepted R8, then published R9 in Foreman State rev
0037 with:

- `PACKET_ID=A003-PS2-WIRE-SESSION-ESTABLISHMENT-R9`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

Read current authority included AGENTS.md, CONTRIBUTING.md, Project Intent,
Clean Architecture, source naming/topology/module lifecycle guidance,
Reconstruction Contract rev 0006, work-log contract rev 0007, Wire Runtime
Decisions rev 0011, Architecture Overlay rev 0004, A001/A003 audit authority,
current Platform network source, current Transport protocol/physical
stream/runtime/bridge source and host fixtures, current Application call path,
R8 Pi product peer source, and reference-only Q1-Q12 proof clients/results.

Reference proof source was used only to recover accepted behavior. No proof
client was mechanically merged into product source.

## Product Transport Q4 client

The Q4 client is implemented inside Transport's physical-stream ownership
boundary.

`pstvnc_transport_physical_stream_establish_client()`:

1. receives one fresh caller-owned TCP descriptor;
2. attempts physical-stream adoption;
3. transfers descriptor ownership only after adoption succeeds;
4. writes `*socket_fd = -1` at that ownership fence;
5. encodes the current product HELLO payload with exact 1/1 compatibility;
6. sends HELLO as the first framed application traffic at outbound sequence 1;
7. receives exactly one complete inbound sequence-1 framed result;
8. accepts only exact ACCEPT or NOT_ACCEPTED control envelopes;
9. rejects wrong kind/channel/flags/sequence/length, zero ACCEPT ID, unknown
   rejection reason, EOF and framing/I/O failure;
10. returns a typed protocol rejection only for exact NOT_ACCEPTED;
11. closes Transport-owned physical state on every post-adoption failure;
12. on ACCEPT, proves send/receive continuation state is exactly 2/2.

A failed adoption leaves the descriptor caller-owned and unchanged.

A failure after adoption leaves the caller descriptor set to -1 and Transport
performs the one close.

This is the explicit no-double-close fence required by R9.

## Exact Q4 bytes and result semantics

R9 consumes the R8 product protocol authority rather than inventing a second
handshake representation.

HELLO:

- kind 1;
- CONTROL channel 0;
- flags 0;
- sequence 1;
- payload length 8;
- Wire version 1;
- product establishment version 1.

ACCEPT:

- kind 12;
- CONTROL channel 0;
- flags 0;
- sequence 1;
- payload length 4;
- decoded session ID must be nonzero.

NOT_ACCEPTED:

- kind 13;
- CONTROL channel 0;
- flags 0;
- sequence 1;
- payload length 4;
- reason must be one of the existing bounded product reasons.

The public bridge establishment result distinguishes:

- `PSTVNC_TRANSPORT_WIRE_ESTABLISHED`;
- `PSTVNC_TRANSPORT_WIRE_NOT_ACCEPTED`;
- `PSTVNC_TRANSPORT_WIRE_ESTABLISHMENT_FAILED`.

Exact NOT_ACCEPTED carries typed
`pstvnc_wire_not_accepted_reason_t` evidence.

Mechanism/framing failure does not manufacture a rejection reason.

## Private Pi session identity

The Pi-assigned ACCEPT session ID is stored only in Transport bridge-private
state:

`pstvnc_transport_bridge_private_session_id`

It is never returned by the public establishment result.

No generic module-facing session-ID getter was added.

The private ID is cleared when the Wire Session is retired and is never copied
into replacement-session module access.

The existing opaque Transport access ticket remains the module-facing
cross-session validity fence.

## Public Wire availability

R9 adds the minimal public fact:

`PSTVNC_TRANSPORT_WIRE_INACTIVE`
`PSTVNC_TRANSPORT_WIRE_ACTIVE`

`pstvnc_transport_wire_availability()` reports ACTIVE only while current
Transport Wire authority is live.

It reports INACTIVE when:

- no Q4 ACCEPT has established a current Wire Session;
- idle Wire authority is explicitly retired;
- the rider runtime has known terminal failure;
- the sole receiver has completed;
- stop has been requested;
- final runtime release retires the current session.

Wire availability does not expose the Pi session ID and does not imply rider
readiness.

## Idle ACTIVE path

`pstvnc_transport_wire_establish()` performs product Q4 but deliberately does
not:

- initialize an RFB queue;
- initialize AUDIO;
- initialize MPEG;
- create/start the rider I/O thread;
- send RFB/AUDIO/MPEG CREDIT;
- send CONFIG;
- send heartbeat;
- send rider DATA.

Focused bridge host coverage proves:

- establishment returns ESTABLISHED;
- public availability becomes ACTIVE;
- all rider-runtime initialize counters remain zero;
- rider start count remains zero;
- Transport access acquisition remains CLOSED because no rider runtime exists;
- the idle Wire Session can then be retired back to INACTIVE.

This is the R9 product-source realization of Q10's distinction between Wire
availability and module readiness.

## Sequence-2 lineage transfer

R9 does not expose arbitrary public sequence seeding.

`pstvnc_transport_physical_stream_transfer_established()` moves the exact
already-established physical stream object, including its descriptor, send lock
and sequence state.

It accepts only an established source with:

- send sequence 2;
- expected receive sequence 2.

On success:

- destination receives the exact physical lineage;
- source descriptor/lock ownership becomes inert;
- source local sequence fields reset only because it no longer owns a session.

The rider runtime gained Transport-internal
`*_initialize_established*` constructors. These consume only an established
physical stream and preserve its 2/2 state.

The old raw-descriptor runtime constructors remain Transport-internal regression
seams in runtime.h. They are not cross-component bridge APIs.

The cross-component bridge cannot activate a rider runtime from a fresh raw
descriptor without performing Q4 first.

Focused physical-stream coverage goes further than state inspection: after
ACCEPT and established-stream transfer, it sends a real first post-Q4 DATA frame
and decodes the emitted header as sequence 2.

## Rider activation boundary

Current `pstvnc_transport_session_open*` bridge operations are reshaped as a
migration-compatible Q4 gate.

If Wire is INACTIVE:

- a supplied fresh descriptor must first pass product Q4;
- no runtime initialization occurs before Q4 ACCEPT.

If Wire is already ACTIVE:

- no second raw descriptor may be supplied;
- rider activation consumes only the existing established physical lineage.

Only after the established lineage has moved into the rider runtime does the
existing `pstvnc_transport_runtime_start_receiver()` execute.

Therefore startup credits remain part of explicit rider activation, not Q4.

No existing RFB/AUDIO/MPEG queue/credit semantics were moved into
establishment.

## Rider-open failure cleanup correction

While exercising the new ownership contract, host regression exposed a
reconstruction-owned cleanup edge.

If Q4 succeeded, `socket_fd` was already -1, but a later rider-runtime
initialization failure could initially leave an idle Transport-owned Wire
session alive while the legacy Application caller had not marked its rider
session active.

That would violate the pre-existing failed-open ownership guarantee.

Corrected behavior:

- any rider-open failure after Q4 retires the still-idle established physical
  stream;
- Wire authority is cleared;
- caller descriptor remains -1 because ownership had already transferred;
- no hidden Transport-owned descriptor survives the failed open.

Startup-thread failure similarly releases the runtime and clears Wire
authority.

This correction is packet-owned and required for descriptor-fence correctness.

## Repeated Session A / Session B

R9 adds deterministic repeated-session evidence at two levels.

Physical-stream fixture:

1. establish Session A with exact sequence-1 HELLO and ACCEPT;
2. verify local continuation reaches 2/2;
3. retire A completely;
4. establish Session B in the same test process;
5. verify B's HELLO is again sequence 1;
6. accept a distinct nonzero Pi identity;
7. prove A and B accepted IDs differ.

Bridge fixture:

1. establish/open A and acquire opaque access A;
2. retire A;
3. establish/open B and acquire opaque access B;
4. prove access tickets differ;
5. prove stale access A returns CLOSED before touching B;
6. prove fresh B access remains usable.

No Session A sequence, private session identity or module access is resumed into
B.

## Existing rider mechanics preserved

The canonical host suite remained green for:

- Transport protocol;
- Transport bridge;
- physical stream;
- Transport runtime;
- AUDIO;
- MPEG;
- MPEG worker;
- Application MPEG frame;
- existing Pi R8 protocol/server/service/staging tests.

No product source outside `src/transport/` changed.

In particular, R9 changes no:

- `src/app.c`;
- Platform network implementation;
- RFB product source;
- AUDIO product source;
- MPEG product source;
- Presentation product source;
- Pi product source.

Current configured Application still reaches Q4 through the reshaped Transport
bridge call; Application itself does not hand-build establishment.

Current fail-closed `pstvnc_app_run()` remains fail-closed and no production
configuration or MPEG activation was invented.

## Comments / readability

R9 adds ownership/rationale comments at the required boundaries:

- descriptor adoption/transfer;
- sequence-1 Q4 transaction;
- 2/2 continuation;
- ACTIVE publication;
- private session-ID retirement;
- establishment-only no-rider state;
- established-lineage runtime move;
- rider-open failure cleanup;
- idle Wire close/abort.

Comments explain invariants and failure consequences rather than narrating
syntax.

## Dictionary and authority documentation

The repository's deterministic dictionary reconciliation path was invoked
through exact trigger commit:

`314ffc5ac14639033e3b4e1e926b28098cbe2758`

Automation produced:

`da88aeca9bf6ed34c5a29a731fb09e180fd8ace9`
`docs(symbols): reconcile current clean definitions`

Final portal authority reports:

- `pi` COMPLETE — 173 definitions;
- `src/transport` COMPLETE — 1231 definitions.

Architecture Overlay now records that R8/R9 provide maintained product Q4 source
on both peers while final physical qualification remains pending.

File/service map now routes Q4 establishment/availability ownership to
`src/transport/`.

## Intermediate failures and corrections

### Bridge fixture link debt

The first R9 source head compiled and linked on PS2, but host
`transport_bridge_test` still stubbed only the pre-R9 internal constructors.

CI correctly failed with unresolved references to the new Q4 physical and
established-runtime seams.

The fixture was updated to model:

- physical Q4 establishment;
- exact ownership movement;
- established-runtime initialization;
- idle physical shutdown/release.

### Legacy bridge expectation mismatch

After linking was restored, older host expectations still assumed the original
descriptor flowed directly into runtime adoption and that failed initialization
could leave the raw descriptor caller-owned.

R9 intentionally changes that boundary: Q4 owns the descriptor first.

The fixture was updated to verify the new ownership contract, and the
packet-owned hidden-idle-session cleanup defect described above was corrected.

### Runtime-family fixture link debt

`runtime.c` now references established-stream transfer even when legacy raw
constructors are the specific test path.

Runtime/AUDIO/MPEG host fixtures therefore needed a deterministic
`pstvnc_transport_physical_stream_transfer_established` stub.

Those fixtures were updated without changing their rider behavior.

### Dictionary drift

Strict dictionaries correctly reported new/stale Transport symbols after R9
source changes.

No manual generated-dictionary edit was used. The canonical deterministic
reconciliation workflow produced the current COMPLETE dictionary and portal.

No unresolved product defect remains from these intermediate red gates.

## Commit chronology

Starting Foreman authority:

`16c1e24fd56df10a035dbd96287cc9ca33d1d1b5`

R9 commits:

1. `bcdb28e00a799d7c3d6ea2a6bf314a98aa4146de`
   — declare Q4 physical establishment;
2. `edbeeaafc079a606f1996417fd184191b8fe8a70`
   — implement Q4 physical establishment;
3. `4a0f4fc9dabe20234e6929d6b356d1a7c44e9edc`
   — declare established runtime adoption;
4. `75bc0c2c1016e3052f168f2782caed66553d6b43`
   — preserve established sequence lineage;
5. `75d53a1ee703a9427d833a005c25112cf2280a89`
   — expose Q4 Wire availability;
6. `3fe47fbd808bc47f00bb787171687da95f7d2a1a`
   — own Q4 Wire session lifecycle;
7. `7fb4e45bd95b9f1b3b432fa077cb8f48b60e7197`
   — prove PS2 Q4 physical establishment;
8. `0fcd2daff16fdf8f62103db20294f18bf55d9c22`
   — cover Q4 bridge lifecycle;
9. `66152c1a0fb5c1af9d637757917e21bddb2ff1d9`
   — retire Wire on rider-open failure;
10. `1fbae919f29046ac183b92aaad83253980046a50`
    — align Q4 ownership failure proof;
11. `66f9de735cbd87684ba82c17d06f831f07eca8a5`
    — prove post-Q4 sequence two on wire;
12. `c381faeed226c92710bbac1bcb2c8e1f3c9553f2`
    — runtime fixture transfer stub;
13. `935ac0943f062a87ad5af6a0469e56c3f90ca70c`
    — AUDIO fixture transfer stub;
14. `2bb568acba1e7faf2fdca80348fad3055dbb376c`
    — MPEG fixture transfer stub;
15. `a4f5de4c5845da7f31eec7b1a48d745ef0f87ffb`
    — prove established runtime lineage;
16. `314ffc5ac14639033e3b4e1e926b28098cbe2758`
    — deterministic dictionary reconciliation trigger;
17. `da88aeca9bf6ed34c5a29a731fb09e180fd8ace9`
    — automated current clean dictionary reconciliation;
18. `8c75df05558b9db6bee82752f7461bdef35fb9f3`
    — record product Q4 source ownership;
19. `0716d615603221a20baab5e30828c73e9adb4ab4`
    — route Transport Q4 ownership;
20. `5f39bd64af666ceb0c1e45e5e3b9229ecbfc0a46`
    — prove Wire terminal availability;
21. `d9868a69249b5afde73fc4717cf20a3a41382c05`
    — prove fresh Q4 sequence on Session B.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `src/transport/SYMBOLS.md`;
- `src/transport/bridge.c`;
- `src/transport/bridge.h`;
- `src/transport/physical_stream.c`;
- `src/transport/physical_stream.h`;
- `src/transport/runtime.c`;
- `src/transport/runtime.h`;
- `tests/unit/transport_audio_test.c`;
- `tests/unit/transport_bridge_test.c`;
- `tests/unit/transport_mpeg_test.c`;
- `tests/unit/transport_physical_stream_test.c`;
- `tests/unit/transport_runtime_test.c`.

No `pi/` product source changed.

No non-Transport `src/` product source changed.

## Final machine evidence

Final pre-log authority:

`d9868a69249b5afde73fc4717cf20a3a41382c05`

Canonical GitHub Actions run:

`35626184669`

Result:

`SUCCESS` on attempt 1.

Final jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host logs include:

- `transport protocol tests passed`;
- `transport bridge tests passed`;
- `transport_physical_stream_test: PASS`;
- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- all 13 Pi R8 Wire/service/staging tests PASS.

Final project/dictionary evidence includes:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Final direct PS2 compile explicitly includes:

- `PS2_COMPILE=src/transport/physical_stream.c`;
- `PS2_COMPILE=src/transport/runtime.c`;
- `PS2_COMPILE=src/transport/bridge.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked build explicitly contains current:

- `transport_physical_stream.o`;
- `transport_runtime.o`;
- `transport_bridge.o`.

Linked evidence:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## A003 R9 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R9-C1 TRANSPORT_OWNS_Q4_CLIENT — MET

Platform still creates TCP only. Product HELLO/result handling, descriptor
ownership and session availability now belong to Transport.

### A003-R9-C2 EXACT_HELLO_ACCEPT_REJECT — MET

Exact R8 product codecs/classifiers are consumed. Focused host tests prove
sequence-1 HELLO, exact ACCEPT/NOT_ACCEPTED envelopes, typed rejection and
fail-closed malformed results.

### A003-R9-C3 DESCRIPTOR_OWNERSHIP_FENCED — MET

Adoption failure leaves the caller descriptor intact. Every post-adoption
outcome sets caller descriptor to -1 and closes only through Transport.
Rider-open failure cleanup cannot strand hidden ownership.

### A003-R9-C4 POST_ESTABLISH_SEQUENCE_TWO — MET

Successful Q4 leaves 2/2. Established-stream transfer preserves that lineage.
Focused host coverage decodes the first real post-Q4 frame as sequence 2.

### A003-R9-C5 TYPED_ESTABLISHMENT_RESULT — MET

Public result separates ESTABLISHED, exact NOT_ACCEPTED with typed reason and
mechanism failure.

### A003-R9-C6 WIRE_AVAILABILITY_INDEPENDENT — MET

Minimal public INACTIVE/ACTIVE fact exists independently of rider readiness and
becomes INACTIVE on known runtime terminality/final retirement.

### A003-R9-C7 IDLE_ACTIVE_NO_RIDER_TRAFFIC — MET

Establishment-only ACTIVE starts no rider runtime and sends no rider credit,
CONFIG, heartbeat or DATA.

### A003-R9-C8 PI_SESSION_ID_PRIVATE — MET

The nonzero Pi session ID is retained privately in bridge state and cleared on
retirement. No public/module getter exists.

### A003-R9-C9 NO_RAW_SESSION_BYPASS — MET

Cross-component rider open must pass Q4 or consume an already-established Wire
lineage. Arbitrary raw runtime constructors are Transport-internal only and no
public sequence seed exists.

### A003-R9-C10 REPEATED_SESSION_NONRESUME — MET

Host evidence proves Session B emits a fresh sequence-1 HELLO after A retirement,
uses a distinct accepted Pi ID, and stale A Transport access cannot touch B.

### A003-R9-C11 EXISTING_RIDER_MECHANICS_PRESERVED — MET

Canonical RFB/AUDIO/MPEG runtime and stale-access regressions remain green.
No rider-domain product source changed.

### A003-R9-C12 CLEAN_EVIDENCE_AND_CLAIM_BOUNDARY — MET

Strict dictionaries/topology/project checks, direct PS2 compile and current
linked reproducibility are green. Physical qualification remains explicitly
pending.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_A003_R9_REQUIRED_REPOSITORY_MACHINE_GATES
PS2_PRODUCT_Q4_SOURCE_IMPLEMENTED=YES
PS2_TRANSPORT_WIRE_AVAILABILITY_IMPLEMENTED=YES
PS2_PI_SESSION_ID_PRIVATE_AUTHORITY_IMPLEMENTED=YES
ESTABLISHMENT_ONLY_IDLE_ACTIVE_IMPLEMENTED=YES
POST_ESTABLISH_SEQUENCE_TWO_IMPLEMENTED=YES
REPEATED_SESSION_HOST_PROOF=YES
PI_R8_PRODUCT_SOURCE_MUTATED=NO
APPLICATION_PRODUCT_SOURCE_MUTATED=NO
PI_RFB_WIRE_RIDER=NOT_IMPLEMENTED
PI_AUDIO_WIRE_RIDER=NOT_IMPLEMENTED
PI_MPEG_WIRE_RIDER=NOT_IMPLEMENTED
PI_MPEG_START_RETIRE_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER=NOT_IMPLEMENTED
CONFIG_WIRE_DELIVERY=NOT_IMPLEMENTED
HEARTBEAT_POLICY=NOT_IMPLEMENTED
AUTOMATIC_RECONNECT_POLICY=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM=NOT_IMPLEMENTED
Q7_RETIREMENT_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
PI_WIRE_SERVICE_LIVE_ACTIVATION=NOT_RUN_NOT_CLAIMED
PHYSICAL_PS2_PI_PRODUCT_Q4=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R9=PENDING
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`d9868a69249b5afde73fc4717cf20a3a41382c05`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exact Q4 bytes and result classification;
- descriptor ownership transfer/cleanup;
- no cross-component raw-session bypass;
- private Pi session identity;
- public INACTIVE/ACTIVE Wire fact;
- zero-rider idle ACTIVE path;
- exact 2/2 continuation and real sequence-2 first post-Q4 frame;
- repeated Session A/B sequence-1/nonresume proof;
- existing rider regressions;
- strict dictionary/topology/project evidence;
- direct PS2 compile and current-source linked reproducibility;
- hardware-pending claim boundary.

The Foreman, not this Reconstruction worker, chooses the next packet.

This worker stops after R9 and does not begin Pi rider, MPEG producer,
Application MPEG activation, Q7 retirement, reconnect policy or physical
qualification work.
