# Reconstruction shift — A003 Pi RFB Wire Relay R10

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T16:44:35-04:00
COMPLETED_AT=2026-09-21T17:01:57-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f6081f723057d301068b06dfb32844a2d3f407be
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-PI-RFB-WIRE-RELAY-R10`

R10 reconstructs the smallest mature bidirectional raw-RFB Wire Channel Relay
core between the accepted Pi Wire server and one explicitly supplied local RFB
provider socket while preserving:

- the Pi Wire connection owner as the sole physical PS2-facing Wire recv/send
  context;
- exact existing DATA kind 3 / CREDIT kind 4 / RFB channel 1 framing;
- finite credit in both directions;
- bounded provider-write storage;
- opaque raw RFB payload bytes;
- provider-local stall/failure containment;
- fresh relay state for each replacement Wire Session;
- the zero-length channel-1 DATA reservation for the existing finite RFB
  quiesce lifecycle;
- current direct-RFB deployment units byte-for-byte;
- the ordinary installed Wire service as establishment-only with no implicit
  provider selection.

R10 does not migrate the qualified provider, select a live provider endpoint,
complete RFB quiesce lifecycle orchestration, implement Application RFB startup,
or begin AUDIO/MPEG/CONFIG/provider-migration work.

## Authority consumed

Truthful shift start:

`2026-09-21T16:44:35-04:00`

Live pickup authority:

`f6081f723057d301068b06dfb32844a2d3f407be`

Current Foreman State had accepted R9 and assigned:

- `PACKET_ID=A003-PI-RFB-WIRE-RELAY-R10`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

Read current authority included:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/PROJECT_INTENT.md;
- docs/CLEAN_ARCHITECTURE.md;
- current source naming/topology/module-lifecycle guidance;
- Reconstruction Contract rev 0006;
- work-log contract rev 0007;
- current Wire Runtime Decisions;
- current Architecture Overlay;
- A001 Transport/RFB audit;
- A003 MPEG-generation audit;
- current Pi Wire protocol/server/runtime staging;
- current PS2 Transport protocol/runtime/bridge/logical RFB channel;
- current RFB bridge/session source;
- current direct-RFB systemd units;
- current Wire systemd unit/stager;
- current host tests;
- current R10 Foreman immutable log.

Forensic/reference source inspected included:

- H1 `h1_rfb_pi_bridge.py` and tests;
- H1 `h1_rfb_session_adapter.py` and tests;
- CP2H/CP2I RFB relay/quiesce evidence;
- Q1-Q12 Proof 3 RFB Relay source/result;
- Q1-Q12 Proof 4C rider-isolation source/result.

Historical source was mechanism evidence only. The old synchronous H1
provider-write shape was explicitly not promoted because a blocking provider
send in the Wire receive path would violate R10's sole-owner liveness
requirement.

## Product Pi Wire vocabulary

Extended maintained `pi/wire_protocol.py` with the existing current product
identities:

- DATA kind = 3;
- CREDIT kind = 4;
- RFB channel = 1;
- flags = 0;
- credit payload = exact four-byte big-endian nonzero uint32.

Added exact helpers for:

- generic zero-flags logical-channel frame encoding;
- RFB CREDIT payload encode/decode;
- RFB CREDIT frame encode;
- RFB DATA frame encode;
- exact RFB CREDIT header classification;
- exact RFB DATA header classification.

Zero-length RFB DATA remains valid channel-1 framing but is explicitly
documented as lifecycle-reserved rather than ordinary provider payload.

Host golden fixtures prove Python and C produce the same exact bytes.

RFB CREDIT, sequence 2, amount 8:

`50 53 54 56 01 04 01 00 00 00 00 02 00 00 00 04 00 00 00 08`

RFB DATA, sequence 3, payload `abc`:

`50 53 54 56 01 03 01 00 00 00 00 03 00 00 00 03 61 62 63`

C host coverage constructs these same frames using current
`PSTVNC_TRANSPORT_FRAME_DATA`,
`PSTVNC_TRANSPORT_FRAME_CREDIT`,
`PSTVNC_TRANSPORT_CHANNEL_RFB`,
header encoding, and big-endian uint32 helpers.

## Pi provider-neutral RFB Relay owner

Created maintained product source:

`pi/rfb_relay.py`

`RfbRelay` owns only:

- one explicitly supplied local provider socket;
- PS2-granted provider-read credit;
- Pi-granted PS2->provider credit;
- one bounded PS2->provider byte queue;
- bytes actually drained but not yet confirmed as returned Wire CREDIT;
- relay-local terminal state/statistics.

It does not own:

- the PS2-facing Wire socket;
- Wire sequence authority;
- Wire recv/send/sendall;
- Wire Session identity;
- a concrete provider endpoint;
- provider service selection;
- TigerVNC/X0tigervnc policy;
- RFB parsing;
- RFB framebuffer/presentation/input policy;
- Application lifecycle;
- AUDIO/MPEG/CONFIG riders.

The provider socket is made nonblocking at relay construction.

No product hardcoding exists for:

- `192.168.50.1:5900`;
- `127.0.0.1:5903`;
- TigerVNC;
- X0tigervnc;
- a command-line `--rfb-provider` path.

## Provider -> PS2 credit

Provider bytes are readable only while
`provider_read_credit > 0`.

The relay's read limit is caller supplied and finite.

One ready provider read is bounded by:

`min(provider_read_credit, max_data_payload)`

No credit means:

- no provider recv;
- no staging of additional provider bytes;
- provider TCP/kernel backpressure remains the capacity boundary.

Only actual bytes returned from provider recv decrement PS2-granted credit.

A short provider read therefore leaves unused credit intact.

Provider EOF/OSError marks only the RFB relay terminal.

No Wire-session failure is synthesized from that provider-local condition.

## PS2 -> provider bounded queue / Pi CREDIT

Relay activation publishes exactly one finite amount:

`provider_write_capacity`

That is the only initial Pi-granted PS2->provider authority.

Each incoming nonempty RFB DATA fragment must satisfy:

- payload length <= configured max DATA payload;
- payload length <= remaining Pi-granted credit;
- resulting local queue length <= configured provider-write capacity.

Credit is consumed when the Wire DATA is admitted.

If no capacity/credit remains, additional PS2 DATA is a protocol violation
rather than unbounded buffering.

The queue itself is finite and cannot grow above the exact advertised
capacity.

## Returned Pi credit follows actual provider drain

Free local memory alone is not peer-visible capacity.

When an already-ready nonblocking provider `send()` actually drains N bytes:

1. those N bytes leave the queue;
2. N enters `pending_credit_return`;
3. the sole Wire owner serializes an exact RFB CREDIT(N);
4. only after that send succeeds does `confirm_credit_sent(N)` restore N to
   local peer-visible `ps2_write_credit`.

Therefore R10 does not return credit merely because bytes were received from
Wire or because queue storage was locally freed.

Partial provider sends return only their actual sent count.

A provider write OSError marks the RFB relay terminal, discards queued
undeliverable bytes locally, and intentionally returns no capacity for those
discarded bytes.

Previously granted but unused PS2 credit may still arrive once after provider
failure. Such late bytes consume that old grant and are dropped locally without
replacement credit. This contains provider failure inside RFB without
manufacturing Wire failure.

## Pi sole physical Wire I/O owner

`pi/wire_server.py` remains the only owner of physical PS2-facing Wire I/O.

R10 adds explicit optional composition:

`WireServer.serve_connection(connection, rfb_attachment=...)`

The relay receives only its provider socket.

It has no Wire socket argument.

The `WireConnectionOwner` performs:

- all complete Wire frame reads;
- active sequence validation;
- all Wire frame writes;
- global send/receive sequence advancement;
- RFB DATA/CREDIT dispatch into/from the relay.

Post-Q4 physical sequence remains one continuous authority.

No second Wire reader/writer thread/process was introduced.

The relay module contains no `sendall()` and no Wire connection field.

## Readiness-driven provider I/O

When an RFB relay is explicitly attached, the existing Wire connection owner
multiplexes:

- Wire readability;
- provider readability only when PS2 read credit exists;
- provider writability only when queued PS2 bytes exist.

Provider writes are nonblocking.

A non-reading/stalled provider therefore cannot suspend the sole Wire owner.

The integrated host fixture intentionally fills the provider socket's send
buffer until nonblocking send would block, then:

1. establishes product Q4;
2. receives finite Pi RFB CREDIT;
3. sends four PS2 RFB bytes into the bounded queue;
4. proves no replacement Pi CREDIT appears while provider write is stalled;
5. sends reverse provider bytes;
6. sends PS2 RFB CREDIT for those provider bytes;
7. proves reverse RFB DATA crosses Wire while provider-write direction is still
   full;
8. drains the artificial provider backlog;
9. proves queued PS2 bytes actually reach provider;
10. proves replacement Pi CREDIT appears only after that drain;
11. injects provider EOF;
12. proves relay terminality while the containing Wire owner remains alive;
13. sends another valid RFB CREDIT after relay terminality;
14. proves Wire remains alive;
15. closes Wire normally;
16. proves the Wire Session retires without protocol failure.

This is deterministic socketpair host evidence, not a live Pi/provider claim.

## Zero-length RFB DATA reservation

Zero-length channel-1 DATA is not treated as raw provider payload.

When observed by the Pi R10 composition:

- no provider queue write occurs;
- no provider send occurs;
- the relay records only a reserved lifecycle marker.

R10 intentionally does not invent or complete the Pi-side
REQUEST/BOUNDARY/COMMIT/COMPLETE finite-quiesce state machine.

That integration remains later work.

## Session-scoped relay / nonresume

Relay state belongs to one composition instance and one Wire Session.

Closing the relay terminally retires its provider attachment and queue state.

A fresh Session-B relay starts with:

- `activated = false`;
- `terminal = false`;
- provider-read credit = 0;
- PS2-write credit = 0 before activation;
- pending returned credit = 0;
- provider-write queue empty.

Host coverage constructs a used Session-A relay, retires it, constructs a
fresh B relay, and proves no A queue/credit/terminal state appears in B.

No replacement session reuses old relay state.

## Default installed service remains establishment-only

The tracked `ps-to-vnc-wire.service` command remains unchanged:

`/usr/bin/python3 /usr/lib/ps-to-vnc/wire_server.py --listen 192.168.50.1 --port 5902`

`WireServer.serve_forever()` invokes:

`self.serve_connection(connection)`

with no RFB attachment.

R10 adds no provider-selection CLI option.

Therefore the ordinary installed Wire service still performs Q4 and permits an
idle ACTIVE Wire Session, but does not automatically connect/migrate any RFB
provider.

R10 source makes the provider-neutral relay composable for a later authorized
owner without silently changing deployed provider policy.

## Wire runtime staging

Updated:

`scripts/pi/install-wire-runtime.sh`

Tracked installed Pi runtime now includes:

- executable `wire_protocol.py`;
- imported module `rfb_relay.py` with mode 0644;
- executable `wire_server.py`;
- existing Wire systemd unit.

Stage/verify/remove remains exact-byte and inactive-only.

No daemon reload, enable, disable, start, stop or restart behavior was added.

The stager still does not mutate direct-RFB units.

Host repository tests verify the new relay path and mode.

## PS2 outbound RFB credit owner

PS2 Transport now accepts exact Pi channel-1 CREDIT and gates outbound RFB
DATA with it.

Added to `pstvnc_transport_runtime_t`:

- one RFB-specific outbound-credit semaphore;
- `rfb_outbound_credit_bytes`;
- one three-state RFB outbound-credit waiter fence.

The existing `rfb_queue_semaphore_id` protects this counter/wait state.

No generic rider scheduler or cross-module credit framework was added.

The only new inbound CREDIT accepted by this work is exact:

- kind CREDIT;
- channel RFB;
- flags 0;
- payload length exactly four bytes;
- nonzero uint32 amount.

Zero credit fails closed.

Credit counter overflow fails closed.

Other malformed/unrecognized control traffic retains the existing terminal
Transport behavior.

## PS2 partial-credit behavior

`pstvnc_transport_runtime_rfb_write_exact()` no longer emits RFB DATA merely
because the rider runtime is live.

For each outgoing fragment it first reserves exact Pi-granted credit.

Fragment size is bounded by:

- bytes still requested;
- currently available Pi credit;
- configured max DATA payload.

If credit is zero, the one RFB writer waits.

A focused concurrent host fixture proves:

- with zero Pi credit, no outbound RFB DATA frame exists;
- CREDIT(3) releases exactly the first three bytes;
- the same writer blocks again;
- CREDIT(2) releases exactly the next two bytes;
- the same writer blocks again;
- CREDIT(1) releases exactly the final byte;
- final credit is zero;
- writer completes only after all six bytes obtained credit.

The pre-existing 10-byte fragmentation regression now first grants 10 bytes of
Pi credit and still proves 4+4+2 DATA fragmentation.

A physical-send failure after credit reservation does not restore the reserved
credit because a frame prefix may already have escaped and the Wire Session is
terminal.

## PS2 terminal/wait lifecycle

The RFB outbound-credit waiter uses three states:

- 0 = no waiter;
- 1 = armed/not signaled;
- 2 = signaled but not yet returned through protected state.

Incoming Pi CREDIT wakes the waiter when required.

Wire terminality also wakes an RFB writer waiting only for credit.

Runtime release observes the protected wait state and refuses resource
reclamation while that rendezvous is still live.

The dedicated semaphore is initialized, partial-init cleaned, and final-release
deleted with the existing Transport runtime resources.

This preserves the sole Transport I/O thread as the only physical sender:
domain/RFB writer code requests complete frames through the existing one-item
outbound rendezvous after obtaining credit.

## Existing PS2 inbound RFB credit semantics preserved

Existing PS2-advertised credit for Pi->PS2 DATA is unchanged:

- runtime startup advertises configured finite RFB receive capacity;
- incoming RFB DATA is committed to the existing bounded logical RFB channel;
- capacity is returned only after RFB-domain consumption according to the
  current batch/flush policy;
- finite quiesce residual discard still earns no credit.

R10 therefore adds the missing opposite-direction credit without replacing the
already reconstructed inbound credit/accounting path.

## Direct-RFB deployment identity

Foreman-base and final pre-log authority have identical blobs for:

- `systemd/pi/ps-to-vnc-rfb.socket`;
- `systemd/pi/ps-to-vnc-rfb-tigervnc.service`;
- `systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service`.

Final static evidence:

- `RFB_SOCKET_UNCHANGED=true`;
- `RFB_SERVICE_UNCHANGED=true`;
- `RFB_PERSISTENT_UNCHANGED=true`.

No direct-RFB endpoint was edited, disabled, redirected, removed or replaced.

## Dictionary / topology authority

The deterministic reconciliation path was invoked with exact tree-identical
trigger commit:

`f44fc6c1fdaadf238f618f31798a03f42463e0a2`

Automation produced:

`27f0794169c67c47e92bba3ff55de00707093ce9`
`docs(symbols): reconcile current clean definitions`

Final dictionary portal reports:

- `pi` — COMPLETE — 294 definitions;
- `src/transport` — COMPLETE — 1247 definitions.

Final source-topology local-file coverage includes the new
`pi/rfb_relay.py`.

Current Pi README, Pi authority router, file/service map, and Architecture
Overlay now describe the provider-neutral R10 ownership and explicitly preserve
the no-auto-attach/deployment boundary.

## Intermediate failures / corrections

### New product-source dictionary/topology debt

Before deterministic reconciliation, strict checks correctly reported:

- missing local-file coverage for `pi/rfb_relay.py`;
- new Pi RFB relay/protocol/server symbols;
- new Transport outbound-credit symbols;
- stale generated dictionary counts.

This was expected new-source debt and was resolved only through the canonical
deterministic reconciliation workflow.

### Relay install-mode correction

The new `rfb_relay.py` is an imported product module, not an executable entry
point.

The first staging edit treated it like the executable server/protocol files.

Repository mode inspection showed:

- `pi/rfb_relay.py` = 100644;
- `pi/wire_protocol.py` = 100755;
- `pi/wire_server.py` = 100755.

The stager was corrected to preserve/verify relay mode 0644 rather than forcing
an executable-source requirement.

This is packaging correctness only; no runtime semantics changed.

### Known Transport host-fixture scheduling race

The dictionary-trigger workflow used an identical behavior tree but one
host-unit attempt hit the pre-existing timing-sensitive
`transport_runtime_test` assertion in
`test_sole_receiver_dispatch_and_activity`.

The fake physical receive counter becomes visible before the Transport runtime
necessarily completes its queue commit/activity publication. That run therefore
observed:

- `receive_calls == 1`;
- queue/activity assertions momentarily still pre-commit.

The identical R10 behavior head immediately before the trigger had passed all
new R10 host tests, and the final coherent R10 head
`6256ce33...` passed `transport_runtime_test` and the complete host suite on
attempt 1.

No product behavior was changed to mask this unrelated historical fixture race.

The intermediate failure is preserved here rather than hidden.

## Commit chronology

Starting Foreman authority:

`f6081f723057d301068b06dfb32844a2d3f407be`

R10 commits:

1. `d863b7d0137bc3ba936652c06ee3839daeced5f8`
   — add exact Pi RFB DATA/CREDIT framing;
2. `eb35be287c56edbf24c3d7ed68366d009412b95a`
   — add bounded Pi RFB relay owner;
3. `0e47de5757aabc2fa2ea0ed519e14dbe836b0c0c`
   — compose RFB relay behind sole Wire owner;
4. `db8af5d868381f09baec45567069c5dbcde3dfee`
   — declare PS2 RFB outbound-credit state;
5. `bff1d50cc7b0e065b1abc5cf3cc4c050ac64a625`
   — gate outbound PS2 RFB by Pi credit;
6. `4b401b543d78c8ae0f14f7fa94f4f77791c5c245`
   — prove outbound PS2 RFB credit gating;
7. `0d0c02ae725ea1d6c8ec650a737f5f1ac823c5e2`
   — stage RFB relay with Wire runtime;
8. `11e8056c25d41acbf54c7fa8980b763c305bd70f`
   — preserve relay imported-module mode;
9. `e4cfa76989cc647e4ec8f7622b0ff89b5d17ed85`
   — prove bounded bidirectional Pi RFB relay;
10. `b0c1a961126e0d0d92955380c37bb654ad8ef261`
    — pin C RFB DATA/CREDIT wire bytes;
11. `25927573074e0c37abe2c91f461699a9d6d88a35`
    — record R10 Pi product ownership;
12. `28cf896a9bbf14bdde252b6686daa585de8bd589`
    — explain provider-neutral Pi RFB relay;
13. `7e9d10913d135f37b93e488b8c2ddb1b27547b60`
    — route Pi RFB relay ownership;
14. `48a62ef2fc036e948b1240ef2d219a3f75433559`
    — reject zero PS2 RFB credit;
15. `f44fc6c1fdaadf238f618f31798a03f42463e0a2`
    — deterministic dictionary reconciliation trigger;
16. `27f0794169c67c47e92bba3ff55de00707093ce9`
    — automated current clean dictionary reconciliation;
17. `6256ce33a6ba9dcbf95f836e8e51f061628d6c5b`
    — record current product RFB Relay architecture boundary.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/pi/README.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `pi/rfb_relay.py`;
- `pi/wire_protocol.py`;
- `pi/wire_server.py`;
- `scripts/pi/install-wire-runtime.sh`;
- `src/transport/SYMBOLS.md`;
- `src/transport/runtime.c`;
- `src/transport/runtime.h`;
- `tests/unit/pi_wire_server_test.py`;
- `tests/unit/transport_protocol_test.c`;
- `tests/unit/transport_runtime_test.c`.

No Application, RFB domain, AUDIO, MPEG, Presentation, Platform-network, CONFIG,
or direct-RFB systemd product source changed.

## Final machine evidence

Final pre-log authority:

`6256ce33a6ba9dcbf95f836e8e51f061628d6c5b`

Canonical workflow:

`35654550153`

Result:

`SUCCESS` on attempt 1.

Jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Host markers include:

- `transport protocol tests passed`;
- `transport_runtime_test: PASS`;
- `test_exact_rfb_data_and_credit_bytes ... ok`;
- `test_provider_reads_require_ps2_credit_and_fragment_to_budget ... ok`;
- `test_provider_write_capacity_returns_credit_only_after_drain ... ok`;
- `test_session_b_starts_with_fresh_queue_and_credit_state ... ok`;
- `test_stalled_provider_write_does_not_block_opposite_wire_progress ... ok`;
- `test_rfb_relay_has_no_wire_socket_or_provider_endpoint_policy ... ok`;
- all 19 Pi Wire/RFB/service/staging tests PASS.

Dictionary/project markers:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS records=162 grandfathered=9 format_compat=2 stamp_compat=1`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Direct PS2 evidence:

- `PS2_COMPILE=src/transport/runtime.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Current link evidence:

- current `transport_runtime.o` rebuilt into the linked ELF;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## A003 R10 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R10-C1 RFB_RELAY_PRODUCT_OWNER — MET

`pi/rfb_relay.py` is maintained product source with explicit provider-socket,
credit, bounded-queue and relay-terminal ownership.

### A003-R10-C2 EXISTING_RFB_WIRE_VOCABULARY — MET

R10 uses current DATA=3, CREDIT=4, RFB channel=1, flags=0 and exact four-byte
big-endian nonzero credit. Python and C golden frames match exactly.

### A003-R10-C3 PROVIDER_TO_PS2_CREDIT_BOUNDED — MET

Provider reads do not occur before PS2 credit. Reads are bounded by remaining
credit and configured max DATA size; actual bytes alone consume credit.

### A003-R10-C4 PS2_TO_PROVIDER_CREDIT_BOUNDED — MET

Pi activation grants exactly finite queue capacity. PS2 outbound RFB DATA waits
for Pi credit. Partial credit releases only matching prefixes. Pi queue is
bounded and replacement credit follows actual provider drain.

### A003-R10-C5 SOLE_PI_WIRE_IO_OWNER — MET

Only `WireConnectionOwner` receives/sends on the PS2-facing Wire connection.
`RfbRelay` has no Wire socket and only provider-local recv/send.

### A003-R10-C6 RAW_RFB_PAYLOAD_OPACITY — MET

Nonempty channel-1 DATA is moved byte-for-byte without parsing provider/RFB
semantics. Golden and socketpair tests prove byte preservation.

### A003-R10-C7 PROVIDER_STALL_DOES_NOT_BLOCK_WIRE — MET

A deliberately full provider-write socket leaves PS2 bytes queued with no early
credit while reverse provider DATA still crosses the same Wire owner. Provider
EOF then makes only the relay terminal; Wire remains alive.

### A003-R10-C8 SESSION_SCOPED_NONRESUME — MET

Fresh relay B starts with empty queue, zero credits/pending credit, nonterminal
state, and independent activation. No A state is reused.

### A003-R10-C9 ZERO_LENGTH_QUIESCE_RESERVATION_PRESERVED — MET

Zero-length channel-1 DATA is recorded only as lifecycle-reserved and never
written to the provider byte stream. Full quiesce integration is explicitly
deferred.

### A003-R10-C10 CURRENT_DIRECT_RFB_RUNTIME_UNCHANGED — MET

All three current direct-RFB service blobs are exactly byte-identical to Foreman
base. No provider migration/redirect occurred.

### A003-R10-C11 EXISTING_PS2_RIDER_REGRESSIONS_PRESERVED — MET

Canonical Transport/RFB/AUDIO/MPEG/Application regressions remain green.
R10 modifies only Transport runtime behavior required for channel-1 outbound
credit and no rider-domain source.

### A003-R10-C12 CLEAN_EVIDENCE_AND_CLAIM_BOUNDARY — MET

Strict dictionaries/topology/project checks, direct PS2 compile and current
linked reproducibility are green. The default installed service remains
establishment-only. No live provider or hardware qualification is claimed.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_A003_R10_REQUIRED_REPOSITORY_MACHINE_GATES
PI_RFB_RELAY_CORE_IMPLEMENTED=YES
PI_RFB_DATA_CREDIT_PRODUCT_VOCABULARY_IMPLEMENTED=YES
PS2_RFB_OUTBOUND_PI_CREDIT_GATE_IMPLEMENTED=YES
BIDIRECTIONAL_RFB_CREDIT_HOST_PROOF=YES
PROVIDER_STALL_WIRE_LIVENESS_HOST_PROOF=YES
PROVIDER_LOCAL_EOF_CONTAINMENT_HOST_PROOF=YES
ZERO_LENGTH_RFB_PROVIDER_FORWARDING=NO
DEFAULT_WIRE_SERVICE_PROVIDER_AUTO_ATTACH=NO
CURRENT_DIRECT_RFB_RUNTIME_MUTATED=NO
CONCRETE_RFB_PROVIDER_ENDPOINT_SELECTED=NO
LIVE_PI_PROVIDER_CONNECTION=NOT_RUN_NOT_CLAIMED
FULL_RFB_QUIESCE_PI_LIFECYCLE=NOT_IMPLEMENTED
COMPLETE_PROVIDER_FAILURE_TO_PS2_RFB_RETIREMENT=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_RESTART_POLICY=NOT_IMPLEMENTED
RFB_PRESENTATION_INPUT_CHANGES=NOT_IMPLEMENTED
PI_AUDIO_WIRE_RELAY=NOT_IMPLEMENTED
PI_MPEG_DATA_RELAY=NOT_IMPLEMENTED
PI_MPEG_START_RETIRE_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER=NOT_IMPLEMENTED
CONFIG_WIRE_DELIVERY=NOT_IMPLEMENTED
HEARTBEAT_POLICY=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
PHYSICAL_PS2_PI_RFB_RELAY=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R10=PENDING
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`6256ce33a6ba9dcbf95f836e8e51f061628d6c5b`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exact DATA/CREDIT C/Python bytes;
- provider-neutral relay ownership;
- no Wire socket access from the relay;
- provider->PS2 read credit gating;
- finite PS2->provider queue/credit;
- replacement Pi credit only after actual provider drain;
- PS2 partial-credit blocking/release behavior;
- stalled-provider opposite-direction Wire progress;
- provider-local terminal state without Wire death;
- zero-length quiesce reservation;
- fresh Session-B relay state;
- installed-service no-provider-auto-attach boundary;
- exact direct-RFB blob identity;
- COMPLETE Pi/Transport dictionaries;
- strict topology/project checks;
- direct PS2 compile/current linked reproducibility;
- physical/hardware non-claim boundary.

The Foreman, not this worker, chooses the next packet.

This Reconstruction worker stops after R10 and does not begin provider migration,
full Pi RFB quiesce lifecycle, Application RFB startup, Pi AUDIO/MPEG,
Application MPEG activation, Q7 retirement/restoration or physical
qualification.
