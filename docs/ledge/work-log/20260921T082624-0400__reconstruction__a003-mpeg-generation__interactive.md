# Reconstruction shift — A003 Pi Wire server establishment R8

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T08:26:24-04:00
COMPLETED_AT=2026-09-21T09:26:17-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=7bac022fb36db15127a01257067d43d1b9d4e71d
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Timestamp note

The interactive baton pickup preceded the first repository write.

The closing context no longer retained the exact earlier chat-clock result from
the initial pickup call. To avoid inventing a timestamp, STARTED_AT above is the
exact Git-recorded timestamp of the first R8 repository-changing commit:

`c0e02ce69627d98dd7ee8a10e65c8fbb3cf5a7f9`
(`2026-09-21T08:26:24-04:00`).

This is therefore a conservative exact execution timestamp, not a claim that no
authority reading occurred before it.

## Objective

Execute only Foreman packet:

`A003-PI-WIRE-SERVER-ESTABLISHMENT-R8`

The packet establishes the smallest maintained Raspberry Pi product Wire
foundation:

- adopt top-level `pi/` as maintained Pi product runtime source;
- promote final Q4 provisional establishment bytes into product protocol
  authority;
- implement one persistent Pi Wire server on the selected
  `192.168.50.1:5902` endpoint;
- make TCP acceptance provisional until exact HELLO/ACCEPT completion;
- allocate nonzero Pi-owned process-local monotonically increasing Wire Session
  IDs without reuse in one process lifetime;
- preserve one physical Pi I/O owner for the accepted socket;
- allow an ACTIVE Wire Session to remain entirely idle;
- retire finite sessions completely and accept later fresh sessions;
- add tracked ordinary-systemd supervision and inactive-only exact-byte staging;
- preserve the qualified direct-RFB runtime unchanged;
- add no RFB/AUDIO/MPEG/CONFIG/heartbeat rider implementation;
- integrate `pi/` into strict source dictionary/topology authority.

R8 does not activate the Pi service or physically qualify the reconstructed
server.

## Authority consumed

At pickup, live branch authority was:

`7bac022fb36db15127a01257067d43d1b9d4e71d`

Current Foreman State assigned:

- `PACKET_ID=A003-PI-WIRE-SERVER-ESTABLISHMENT-R8`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`.

Read current authority included:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/PROJECT_INTENT.md;
- docs/CLEAN_ARCHITECTURE.md;
- source topology/naming/tooling guidance;
- Reconstruction Contract;
- work-log contract;
- Wire Runtime Decisions;
- Architecture Overlay;
- current Pi companion/provisioning/service maps;
- current Transport product protocol;
- current selected PS2 PSTV endpoint;
- current direct-RFB systemd/service staging patterns;
- reference-only Q1-Q12 Wire proof material.

The proof branch was used only to recover adopted representation/behavior.
No proof coordinator/server was mechanically copied into product source.

## Product Q4 protocol authority

Extended current product Transport protocol authority:

- `src/transport/protocol.h`;
- `src/transport/protocol.c`;
- `tests/unit/transport_protocol_test.c`.

The existing fixed PSTV header remains exactly 16 bytes.

Current establishment identity is:

- HELLO kind = 1;
- ACCEPT kind = 12;
- NOT_ACCEPTED kind = 13;
- control channel = 0;
- flags = 0;
- Wire header version = 1;
- product Wire compatibility version = 1;
- product establishment version = 1.

HELLO payload is exactly 8 big-endian bytes:

- uint32 wire_version;
- uint32 product_establishment_version.

ACCEPT payload is exactly 4 big-endian bytes:

- nonzero uint32 session_id.

NOT_ACCEPTED payload is exactly 4 big-endian bytes with one reason:

- 1 = WIRE_VERSION;
- 2 = PRODUCT_VERSION;
- 3 = MALFORMED.

Sequence ownership begins independently at one in each direction:

- provisional PS2 HELLO = sequence 1;
- Pi ACCEPT/NOT_ACCEPTED = sequence 1;
- successful establishment leaves the next sequence in each direction at 2.

C product codecs reject:

- zero ACCEPT session ID;
- unknown rejection reason;
- wrong payload lengths;
- non-control/flagged establishment envelope identities.

Golden C fixture bytes match the Python product representation.

## Maintained Pi product source root

Created the new maintained clean product source root:

`pi/`

It is explicitly distinct from:

- `src/` — PS2 executable/application source;
- `scripts/pi/` — provisioning/staging/development tooling;
- `experiments/` — evidence/apparatus.

Current product files are:

- `pi/wire_protocol.py`;
- `pi/wire_server.py`;
- `pi/README.md`;
- `pi/SYMBOLS.md`.

Current Pi product source participates in the same strict local-file/dictionary
completeness model as `src/`.

The final dictionary portal reports:

- `pi` — COMPLETE — 173 definitions;
- `src/transport` — COMPLETE — 1139 definitions.

Source topology now reports the maintained directory set beginning with:

`pi,src,src/audio,...,src/transport,src/ui`

and passes local file coverage.

## Pi product protocol module

`pi/wire_protocol.py` owns bytes only:

- fixed PSTV header representation;
- Q4 frame identities;
- big-endian payload codecs;
- bounded uint validation;
- exact header/envelope classifiers.

It owns no:

- listener lifecycle;
- accepted socket;
- session allocator;
- RFB provider;
- AUDIO/MPEG/CONFIG rider;
- MPEG producer;
- reconnect policy;
- systemd behavior.

The implementation includes explicit code comments explaining:

- fixed-header layout;
- the distinction between header version and negotiated Wire/product versions;
- Q4 control-frame identity;
- why ACCEPT session ID zero is prohibited;
- why provisional sequence 1 is reserved for establishment.

## Persistent Pi Wire server

`pi/wire_server.py` implements one long-lived product Wire owner.

Selected default endpoint:

`192.168.50.1:5902`

The server process itself owns the listener.

It is not systemd socket activated.

The runtime is deliberately sequential in R8:

1. listener accepts one TCP connection;
2. one `WireConnectionOwner` exclusively owns that accepted socket;
3. that owner performs physical recv;
4. that owner parses establishment bytes;
5. that owner performs physical send;
6. the connection is fully retired and closed;
7. only then does the persistent server accept the next peer.

The accepted socket is never handed to rider code, another process, or another
server execution context.

No production thread, subprocess, fork or generic rider-dispatch mechanism was
added.

## Provisional-to-ACTIVE establishment

A new accepted TCP connection begins:

`PROVISIONAL`

It is not an ACTIVE Wire Session merely because TCP accept succeeded.

The first application frame must be:

- valid PSTV header;
- HELLO kind 1;
- control channel 0;
- flags 0;
- inbound sequence 1;
- exact 8-byte HELLO payload.

Unsupported but safely parsed Wire/product version receives exact
NOT_ACCEPTED reason at outbound sequence 1 and never becomes ACTIVE.

A structurally valid Wire frame that is not the exact allowed first HELLO may
receive MALFORMED and remains non-ACTIVE.

Invalid framing that is not safe enough to answer with trusted Wire framing
closes the provisional connection without manufacturing a response or session.

The code comments explicitly document this distinction.

## Authoritative session identity

`SessionIdAllocator` owns a process-local monotonically increasing nonzero
uint32 sequence.

Default first ID is 1.

Within one service process lifetime:

- zero is never allocated;
- an ID is never reused;
- UINT32_MAX may be allocated once;
- a subsequent allocation fails closed;
- allocator never wraps.

The Pi allocates the candidate session ID before ACCEPT.

ACTIVE is published only after `sendall(ACCEPT)` succeeds.

If ACCEPT send fails:

- no ACTIVE session is published;
- the allocated ID remains consumed.

This is intentional because a failed `sendall` can expose a prefix of the
ACCEPT to the peer; reusing the same ID on a later connection would create
ambiguous identity.

Host coverage explicitly proves the next allocation advances after an injected
ACCEPT send failure.

## Idle ACTIVE session

After ACCEPT succeeds:

- local state becomes ACTIVE;
- send sequence becomes 2;
- receive sequence is already 2.

R8 has no heartbeat and no rider traffic requirement.

The ACTIVE owner performs a blocking receive waiting for physical session end.

Silence:

- does not time out;
- sends no telemetry;
- sends no heartbeat;
- requires no RFB;
- requires no MPEG;
- requires no PCM;
- requires no CONFIG.

A strengthened host fixture establishes a session in a worker thread, receives
ACCEPT, proves the server remains blocked/alive while the peer sends no rider
traffic, then closes the peer write side and proves the owner completes.

## Repeated finite sessions

EOF or connection loss retires the current connection to INACTIVE.

A later TCP connection begins a completely fresh PROVISIONAL transaction.

It receives a new session ID.

Host coverage proves consecutive accepted sessions use distinct monotonic IDs.

No dead session is resumed.

The previous session ID returned in test/evidence outcomes is historical
evidence only, not reusable authority.

## Session-local failure containment

R8 deliberately supports no post-establishment rider frame.

If an ACTIVE peer sends an application byte:

- that current session fails closed;
- the accepted socket is retired;
- the persistent listener/server owner survives;
- a subsequent fresh HELLO can establish a new session with a distinct ID.

Host coverage proves a failed session followed by a successful replacement
session on the same persistent `WireServer` object.

Malformed provisional framing likewise does not kill the persistent server
owner.

## Code-comment clarification requested during shift

During R8 execution, the operator explicitly requested:

`Insert code comments to explain the behavior`

Behavior-identical comments were added in:

- `pi/wire_protocol.py`;
- `pi/wire_server.py`.

They explain:

- Q4 frame/version separation;
- provisional/ACTIVE/INACTIVE state meaning;
- TCP exact-read behavior;
- safe-versus-unsafe malformed framing response policy;
- direction-local establishment sequence ownership;
- Pi session-ID authority and nonreuse;
- why failed ACCEPT burns an ID;
- why ACTIVE is published only after successful ACCEPT send;
- why an idle ACTIVE session is valid without a heartbeat;
- why any R8 post-establishment byte fails closed;
- why the accepted socket remains single-owner;
- why sequential full retirement happens before the next accept.

No behavior changed in these comment-only commits.

## No premature riders

Product Pi Wire runtime contains no ordinary DATA or MPEG frame dispatcher.

Static source review finds no product implementation for:

- RFB relay;
- AUDIO relay;
- MPEG DATA;
- MPEG START;
- MPEG RETIRE;
- MPEG producer;
- CONFIG delivery;
- heartbeat;
- telemetry;
- generic rider registration.

References to names such as RFB/MPEG/CONFIG in source are explanatory
non-ownership comments/docstrings, not implementations.

The server recognizes only Q4 establishment and connection lifetime in R8.

## Systemd service definition

Added:

`systemd/pi/ps-to-vnc-wire.service`

Required shape:

- ordinary `Type=simple` service;
- `User=ps2`;
- `Group=ps2`;
- `WorkingDirectory=/usr/lib/ps-to-vnc`;
- `ExecStart=/usr/bin/python3 /usr/lib/ps-to-vnc/wire_server.py --listen 192.168.50.1 --port 5902`;
- `Restart=on-failure`;
- no `StandardInput=socket`;
- no dependency on the current RFB provider.

systemd supervises the process.

The Python server contains no custom self-restart loop.

## Inactive-only staging / verification

Added:

`scripts/pi/install-wire-runtime.sh`

Supported actions:

- `stage`;
- `verify`;
- `remove`.

Tracked installed locations:

- `/usr/lib/ps-to-vnc/wire_protocol.py`;
- `/usr/lib/ps-to-vnc/wire_server.py`;
- `/etc/systemd/system/ps-to-vnc-wire.service`.

The stager:

- requires root;
- requires the existing `ps2` service user;
- verifies `/usr/bin/python3`;
- syntax-checks tracked Python;
- refuses non-identical pre-existing targets;
- installs exact tracked bytes;
- verifies byte identity with `cmp`;
- verifies expected modes;
- runs `systemd-analyze verify` against the staged unit;
- refuses stage/verify/remove while the Wire candidate is active or enabled;
- removes only byte-identical tracked files.

It contains no:

- `systemctl daemon-reload`;
- `systemctl enable`;
- `systemctl disable`;
- `systemctl start`;
- `systemctl stop`;
- `systemctl restart`.

It does not reference or mutate:

- `ps-to-vnc-rfb.socket`;
- `ps-to-vnc-rfb-tigervnc.service`.

A final host fixture explicitly asserts the exact-byte `cmp`, install-mode,
verify-mode and systemd static-verification mechanisms.

No live Pi staging was performed by this Reconstruction seat.

## Current direct-RFB runtime unchanged

Foreman base and final R8 pre-log authority have identical blobs for:

- `systemd/pi/ps-to-vnc-rfb.socket`;
- `systemd/pi/ps-to-vnc-rfb-tigervnc.service`.

Final evidence:

- `RFB_SOCKET_UNCHANGED=true`;
- `RFB_SERVICE_UNCHANGED=true`.

R8 does not migrate RFB onto Wire.

It does not create another rider-owned PS2-facing socket.

The selected Wire listener is the separate product Wire endpoint 5902.

## Dependency / routing documentation

Current Pi authority now records:

- `pi/` as maintained product runtime;
- Wire protocol/server paths;
- Wire systemd unit;
- inactive stager;
- selected 5902 endpoint;
- current no-activation claim.

The dependency ledger promotes the already-present Python 3
`3.13.5-1` from OS_BASE-only presence to ADOPTED_RUNTIME for the maintained
Pi Wire service.

No additional Pi package was installed by R8.

## Source dictionary / topology tooling

R8 extends product-source discovery from:

`src/`

to:

`src/` and `pi/`.

Updated current tooling includes:

- `scripts/source-dictionary.py`;
- `scripts/continuity-check.sh`;
- `scripts/reconcile-source-dictionaries.py`;
- the dictionary reconciliation workflow.

The new Pi dictionary was mechanically reconciled and promoted to:

`COVERAGE=COMPLETE`

The existing `src/` completeness guarantees remain intact.

No exemption was created merely because the Pi implementation is Python.

## Intermediate failures / corrections

### Combined topology tool-call limit

An attempted combined topology/documentation write exceeded the connected
repository tool's per-call orchestration limit before branch update.

Live branch authority was checked immediately afterward and remained unchanged.

The same bounded changes were then split into smaller commits.

No partial branch state was left by the failed orchestration call.

### Unsafe malformed framing response

Initial server logic would attempt MALFORMED after any
`protocol.decode_header()` failure.

That was corrected in:

`74c5d4801abc360c75f68b43aa74b220d283f9ed`

Invalid fixed framing now raises `UnsafeProvisionalFraming` and closes without
manufacturing a response.

Well-framed but semantically malformed establishment remains eligible for exact
MALFORMED.

### Malformed-close host fixture

The first host run correctly closed an invalid provisional header but Linux
reported the close as `ECONNRESET` because unread peer bytes remained.

The fixture incorrectly assumed only clean EOF.

Current authority corrected the harness to accept connection reset as the same
fail-closed no-response outcome.

The server behavior was not weakened.

### Dictionary bootstrap / concurrent authority movement

The first full R8 host/project run correctly reported new `pi/` local-file and
dictionary coverage debt.

While the worker was reviewing that run, live branch authority advanced through
compatible same-packet maintenance/reconciliation commits, including:

- malformed-close fixture correction;
- Python-root dictionary reconciliation support;
- canonical `pi/SYMBOLS.md` generation;
- generated portal reconciliation;
- Transport dictionary reconciliation for new Q4 protocol definitions;
- stronger idle-session and failed-ACCEPT ID-nonreuse tests.

The worker refreshed live authority before further writes, inspected the diff,
confirmed Foreman State still had R8 ACTIVE for
`reconstruction/interactive`, and did not overwrite or duplicate those newer
commits.

The final current authority is linear and all final evidence below is from that
authority plus the later behavior-identical code comments and staging-identity
test.

## Commit chronology

R8 authority after Foreman base
`7bac022fb36db15127a01257067d43d1b9d4e71d`:

1. `c0e02ce69627d98dd7ee8a10e65c8fbb3cf5a7f9`
   — promote Q4 establishment protocol;
2. `5e5ea039718ff3774dbe0ee6708b74652c601c98`
   — add persistent Pi Wire establishment server;
3. `60e5f7f3ee99171fd46c3abf1b667ff21cc01c99`
   — stage supervised Wire service;
4. `74c5d4801abc360c75f68b43aa74b220d283f9ed`
   — keep invalid framing response-safe;
5. `a434050e7ea8287da0c90f193104c63669f156ed`
   — include Pi product source in dictionaries;
6. `6a74e009143494c53693c12bcc6cbc13ae951f7f`
   — include Pi product source in topology;
7. `d6e9b65272c06b34bf44075db892c346c9ff60f3`
   — run Wire server host coverage;
8. `1691d48d29f38aee919325a275adc55983534afe`
   — adopt Pi product source root;
9. `662d970490d6ae79b327fbc29133bdc09481382a`
   — record Pi product topology;
10. `a0c4ca79aea6caf05f4742996b0f565473405933`
    — route Pi Wire product runtime;
11. `d7e6f37c49509739b968228732c25a140e655f3e`
    — record Wire server foundation;
12. `18d61586552f31d94949f3c9f64855dc4a4cc309`
    — document inactive Wire staging;
13. `3fd49ae544a7b87b96d51a362e4c230f99252339`
    — adopt Python Wire runtime dependency;
14. `f0c5c36d0c83d560af0fec5a5ec5585ae84f1686`
    — accept reset on malformed provisional close;
15. `ee56931aeb60a2f3e99d185ba33730f6628797a0`
    — deterministic dictionary reconciliation trigger;
16. `3cc4998bb41a4d8af1919485689b306caf9a8d74`
    — reconcile Python product root;
17. `aaaf984e4fc38a178312e3e2859cc6cdd39cd4d2`
    — deterministic dictionary reconciliation trigger;
18. `f83574df7bb74109e8e570f799cfaad7385a2191`
    — automated current clean dictionary reconciliation;
19. `04b4df15fd02369b64688236e0773db236a746d6`
    — commit Pi product dictionary workflow support;
20. `7feb07e6af62dfd289080b19f18a9b54a994efde`
    — deterministic dictionary reconciliation trigger;
21. `e9ea1fcb680ca09a2a130f3ab42d70a907d4c6fa`
    — automated current clean dictionary reconciliation;
22. `09bfe121f53982351ce11199dceededb87d09152`
    — describe Q4 framing authority;
23. `9519da60254116360c496c51492907a2e52a99a4`
    — strengthen idle-session and ID-nonreuse proof;
24. `4e4da2208f8f4f9112a5d408bee2679757953dec`
    — explain Wire protocol behavior in code comments;
25. `e282e3612eb93519e5bdbf0a636548b186536fd4`
    — explain Wire server lifecycle behavior in code comments;
26. `6d9faaf4ed9d512d1697c0b7b4d56886bde210b6`
    — prove Wire staging identity contract.

## Exact final changed paths

Compared with Foreman base, final pre-log authority changes:

- `.github/workflows/ledge-reconstruction.yml`;
- `docs/development/source-naming-and-symbols.md`;
- `docs/development/source-topology.md`;
- `docs/pi/DEPENDENCY_LEDGER.md`;
- `docs/pi/PROVISIONING.md`;
- `docs/pi/README.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `pi/wire_protocol.py`;
- `pi/wire_server.py`;
- `scripts/continuity-check.sh`;
- `scripts/pi/install-wire-runtime.sh`;
- `scripts/reconcile-source-dictionaries.py`;
- `scripts/source-dictionary.py`;
- `src/transport/SYMBOLS.md`;
- `src/transport/protocol.c`;
- `src/transport/protocol.h`;
- `systemd/pi/ps-to-vnc-wire.service`;
- `tests/Makefile`;
- `tests/unit/pi_wire_server_test.py`;
- `tests/unit/transport_protocol_test.c`.

No current direct-RFB unit changed.

No Application, MPEG producer, audio runtime, RFB runtime, Transport runtime,
Transport bridge or Presentation product source changed.

## Final machine evidence

Final pre-log authority:

`6d9faaf4ed9d512d1697c0b7b4d56886bde210b6`

Canonical GitHub Actions workflow:

`35605504470`

completed SUCCESS on attempt 1.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host evidence includes:

- `transport protocol tests passed`;
- `transport_runtime_test: PASS`;
- `config_mpeg_runtime_profile_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- all Pi Wire protocol/server/service/staging tests PASS.

The final Pi host suite covers:

- exact golden HELLO/ACCEPT/NOT_ACCEPTED bytes;
- zero session rejection;
- unknown NOT_ACCEPTED reason rejection;
- unsupported Wire/product version rejection;
- malformed establishment never ACTIVE;
- unsafe header close without response;
- ACCEPT send failure never ACTIVE and consumes the candidate ID;
- ACTIVE idle without rider traffic;
- consecutive distinct IDs;
- allocator exhaustion fail-closed;
- session-local failure followed by successful fresh session;
- ordinary supervised service shape;
- no live systemctl mutation;
- exact staging identity/mode/static-unit verification mechanisms.

Final strict dictionary evidence:

`SOURCE_DICTIONARIES=PASS`

Final topology/project evidence:

- `SOURCE_TOPOLOGY_DIRECTORIES=pi,src,src/audio,src/config,src/diagnostics,src/display,src/framebuffer,src/input,src/media,src/mpeg,src/platform,src/rfb,src/transport,src/ui`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Final direct PS2 evidence includes:

- `PS2_COMPILE=src/transport/protocol.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked evidence includes:

- current `transport_protocol.o`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## A003 R8 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R8-C1 PI_PRODUCT_SOURCE_ROOT — MET

`pi/` is an explicitly adopted maintained product-source root with local
README and COMPLETE symbol authority. It participates in strict topology and
dictionary checks.

### A003-R8-C2 EXACT_Q4_PROTOCOL — MET

C and Python product authority encode the exact 16-byte PSTV header plus
HELLO=1, ACCEPT=12, NOT_ACCEPTED=13, control/flags zero, exact 8/4/4-byte
payloads and bounded reason identities. Golden fixtures agree.

### A003-R8-C3 PROVISIONAL_ESTABLISHMENT — MET

TCP accept is PROVISIONAL. Only exact inbound sequence-1 HELLO with accepted
1/1 versions followed by successful outbound sequence-1 ACCEPT creates ACTIVE.
Safe incompatibility/malformed paths never create ACTIVE.

### A003-R8-C4 AUTHORITATIVE_SESSION_ID — MET

The Pi allocator supplies monotonically increasing nonzero uint32 IDs, never
reuses within one process, burns a candidate on failed ACCEPT send, and fails
closed at exhaustion rather than wrapping.

### A003-R8-C5 SOLE_PI_PHYSICAL_IO_OWNER — MET

One sequential `WireConnectionOwner` owns physical recv/parsing/send for the
accepted socket. The persistent server retains listener ownership. No rider,
thread, process or registration API receives the accepted socket.

### A003-R8-C6 IDLE_ACTIVE_SESSION — MET

After ACCEPT, the ACTIVE owner can block indefinitely with no rider/heartbeat
traffic. Host coverage proves it remains alive until peer EOF.

### A003-R8-C7 REPEATED_SESSION_CONTAINMENT — MET

EOF/failure retires one connection completely. A later peer begins a new
provisional transaction and receives a distinct ID. Session-local protocol
failure does not terminate the persistent server owner.

### A003-R8-C8 SYSTEMD_REPRODUCIBLE_STAGING — MET

Tracked ordinary service plus fail-closed stage/verify/remove tooling exists.
Repository tests prove service shape, no live lifecycle mutation and exact-byte
identity/mode/static-verification mechanisms.

This is repository/source evidence only; no live staging/daemon-reload/service
activation occurred.

### A003-R8-C9 CURRENT_RFB_RUNTIME_UNCHANGED — MET

Current qualified RFB socket and TigerVNC service blobs are byte-identical to
Foreman base. Wire staging has no references that mutate those units.

### A003-R8-C10 NO_RIDERS_OR_MPEG_POLICY — MET

No product RFB/AUDIO/MPEG DATA/START/RETIRE/CONFIG/heartbeat/telemetry/rider
framework was implemented. Active post-establishment bytes currently fail that
session closed.

### A003-R8-C11 STRICT_TEST_DICTIONARY_TOPOLOGY_EVIDENCE — MET

Host suite, long complete strict dictionaries, local-file topology, project
check, direct PS2 compile and linked reproducibility all pass on exact final
pre-log authority.

### A003-R8-C12 CLAIM_BOUNDARY_PRESERVED — MET

No live Pi activation, no physical PS2/Pi Q4 run and no no-carrier service
qualification are claimed. Historical Q1-Q12 hardware evidence remains
provenance/reference rather than qualification transfer.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_A003_R8_REQUIRED_REPOSITORY_MACHINE_GATES
PI_PRODUCT_SOURCE_ROOT_IMPLEMENTED=YES
PRODUCT_Q4_PROTOCOL_AUTHORITY_IMPLEMENTED=YES
PI_PRODUCT_WIRE_SERVER_IMPLEMENTED=YES
PI_WIRE_SYSTEMD_UNIT_TRACKED=YES
PI_WIRE_STAGING_TOOL_TRACKED=YES
PI_WIRE_SERVICE_LIVE_STAGED=NO
PI_WIRE_SYSTEMD_DAEMON_RELOADED=NO
PI_WIRE_SERVICE_ENABLED=NO
PI_WIRE_SERVICE_STARTED=NO
PHYSICAL_PI_WIRE_LISTENER=NOT_RUN_NOT_CLAIMED
PHYSICAL_PS2_PRODUCT_Q4_ESTABLISHMENT=NOT_RUN_NOT_CLAIMED
NO_CARRIER_WIRE_SERVICE_BEHAVIOR=NOT_RUN_NOT_CLAIMED
RFB_PROVIDER_TO_WIRE_INGRESS=NOT_IMPLEMENTED
AUDIO_WIRE_RELAY=NOT_IMPLEMENTED
MPEG_DATA_WIRE_RELAY=NOT_IMPLEMENTED
PI_MPEG_START_RETIRE_HANDOFF=NOT_IMPLEMENTED
PI_MPEG_PRODUCER=NOT_IMPLEMENTED
CONFIG_WIRE_DELIVERY=NOT_IMPLEMENTED
HEARTBEAT_POLICY=NOT_IMPLEMENTED
PS2_PRODUCT_Q4_CALL_GRAPH_WIRING=NOT_IMPLEMENTED
APPLICATION_NOT_ACCEPTED_POLICY=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM=NOT_IMPLEMENTED
Q7_RETIREMENT_FINALIZATION=NOT_IMPLEMENTED
WIRE_LOSS_MODULE_RECOVERY=NOT_IMPLEMENTED
CURRENT_DIRECT_RFB_RUNTIME_REPLACED=NO
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`6d9faaf4ed9d512d1697c0b7b4d56886bde210b6`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- `pi/` product-source adoption and COMPLETE dictionary coverage;
- exact Q4 C/Python golden parity;
- provisional-to-ACTIVE transition;
- nonzero monotonic/nonreused session IDs;
- failed-ACCEPT ID burn;
- one Pi physical I/O owner;
- idle ACTIVE session;
- repeated finite-session containment;
- service/stager exact identity and no-live-mutation shape;
- byte-unchanged current RFB units;
- no rider/MPEG policy implementation;
- final host/dictionary/topology/PS2 compile/link evidence;
- machine-vs-hardware claim boundary.

The Foreman, not this worker, chooses the next packet.

This Reconstruction worker stops after R8 and does not begin Pi MPEG
control/producer work or PS2/Application Q4 wiring.
