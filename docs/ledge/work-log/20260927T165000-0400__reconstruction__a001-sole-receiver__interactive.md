# Ledge Reconstruction work log — R44 HW1 incremental RFB live-progress correction

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T16:50:00-04:00
COMPLETED_AT=2026-09-27T17:05:21-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=deeec5a24e09f73eda19bb7703d5a8c280781f44
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and scope

Recovered Foreman State revision 0087 and executed only:

`A001-HW1-RFB-INCREMENTAL-LIVE-PROGRESS-R44`.

Assigning branch authority:

`deeec5a24e09f73eda19bb7703d5a8c280781f44`
— `docs(work-log): route HW1 RFB stall to Reconstruction`.

The packet was grounded in:

- independently validated product source
  `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`;
- HW1 evidence commit
  `bb646ed6c3733488876719cd4a4917831669da8d`;
- Validation State revision 0007;
- immutable work-log contract document revision 0007.

The canonical log-format value remains `LOG_FORMAT_REVISION=0001` per that
revision-0007 contract.

Final pre-log source/test authority:

`e005bf001a72302ba81e688e8616910077160764`
— `test(rfb): prove repeated isolated update boundaries`.

R44 did not modify Pi Wire/provider product source, selected RFB profile values,
Wire protocol/version, AUDIO/MPEG/Presentation/media-clock product source,
Input/UI semantics, Configuration/Management, H1 forensic source, or automatic
recovery policy. No hardware run was performed.

## Sealed HW1 boundary preserved

R44 treated the exact HW1 terminal valid update as authoritative:

- RFB payload length: 328 bytes;
- Raw rectangle: x=681, y=11, width=12, height=13;
- Raw pixel payload: 312 bytes;
- rectangle remained inside the negotiated 704x462 desktop;
- PS2 TCP had acknowledged the final Pi bytes;
- after that update the hardware evidence showed no next PS2 PSTV CREDIT and no
  successor FramebufferUpdateRequest;
- the Wire/provider/TCP session otherwise remained live and no product FATAL was
  established.

The reconstruction did not reinterpret that boundary as malformed RFB,
rectangle-bounds failure, provider failure, TCP loss, or operator error.

## First missing PS2 progress fact

Source and focused deterministic reconstruction distinguish the relevant local
boundaries.

The complete 328-byte Raw update is accepted by the clean RFB parser and reaches
a complete server-message boundary without any later server byte. Therefore the
sealed rectangle itself does not require a future RFB message to finish parser
framing.

Transport already commits a complete inbound RFB DATA frame into the logical RFB
channel before Application/RFB consumes it. Parser consumption then returns
flow-control authority through the existing
`pstvnc_transport_runtime_rfb_read_exact()` consumption path; flush-on-empty
can synchronously submit the exact consumed byte count as CREDIT.

The missing progress fact was the sole physical-I/O owner's **outbound
rendezvous after parser consumption**.

Before R44 the owner performed this sequence:

1. nonblocking `PollSema(outbound_ready)`;
2. if empty, enter
   `pstvnc_transport_physical_stream_wait_readable(..., 1000us)`;
3. on PS2 that readability operation is an EE-to-IOP synchronous select RPC
   observing only the physical Wire socket;
4. a parser/domain thread can publish CREDIT immediately after step 1 and then
   block waiting for outbound completion;
5. signaling the EE outbound semaphore does not interrupt an IOP select RPC that
   is already in flight;
6. with no later inbound Wire frame, progress therefore depended on the
   unrelated peer/socket readiness path returning control to the owner.

That ordering exactly explains an HW1 shape where the inbound DATA was already
accepted by TCP/Wire but the consumption CREDIT and successor request never
appeared.

The current pinned PS2SDK image is not being blamed for the older select RPC ABI
layout defect: canonical builds use
`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`,
whose build date is 2026-08-28, after the upstream May 2026 select RPC ABI
correction. R44 addresses the ownership/scheduling dependency itself.

## Product correction

Product behavior changed only in:

`src/transport/runtime.c`.

Commit:

`0c914d2b7136b18958a96f137ff3ec4722ca3723`
— `fix(transport): make HW1 outbound progress inbound-independent`.

The correction keeps exactly one physical Wire recv/send owner.

The owner now:

1. proves outbound work only through the real nonblocking
   `PollSema(outbound_ready)` primitive;
2. probes physical socket readability with timeout `0`, so no remote
   EE-to-IOP select wait can become the blocking wake mechanism;
3. after an idle socket probe, immediately polls outbound-ready a second time,
   closing the exact check-then-wait race where a submitter published during the
   physical probe;
4. only when both the socket and outbound queue are idle uses the existing
   1000-us EE cooperative delay;
5. loops and proves readiness again from the actual semaphore/socket facts.

The 1000-us local delay remains cadence/yield only. It is not completion,
readiness, timeout-success, reconnect, or watchdog authority.

Work published after the second poll is serviced on the next local owner pass
after that cooperative yield. It does not require any future inbound Wire frame.

R44 does **not**:

- add a second socket writer/receiver;
- let RFB or Application write the socket directly;
- return CREDIT before parser consumption;
- add auto reconnect/restart;
- fabricate success from elapsed time or retry count;
- change queue/credit values;
- change Wire bytes/version.

Selected A001 values remain:

- RFB queue/window = 32768 bytes;
- credit batch = 8192 bytes;
- max DATA payload = 8192 bytes;
- flush-on-empty = enabled.

## Critical EE-order concurrency proof

The host Transport fixture previously used
`outbound_pending + WaitSema` as a shortcut for the owner readiness check.
That was not sufficient for R44 because it bypassed the actual EE
`PollSema` ordering.

R44 changed the focused host execution shim so the product source uses a real
fake `PollSema` with EE-like nonblocking consume semantics in every Transport
runtime fixture.

The new deterministic
`test_hw1_consumed_rfb_credit_progress_is_inbound_independent` performs the
critical race:

1. initializes a healthy Transport with the selected 32768/8192/8192
   queue/credit/max-payload geometry and flush-on-empty;
2. queues exactly one 328-byte logical RFB DATA frame;
3. waits until the sole physical owner has received and committed it;
4. places the owner at a real `PollSema(outbound_ready)` miss after the empty
   state has been observed;
5. while the owner is held in that exact window, a parser-side reader consumes
   all 328 bytes;
6. consumption empties the logical queue, earns exactly 328 bytes of CREDIT,
   publishes outbound-ready, and the submitter blocks on outbound completion;
7. no second inbound frame is queued;
8. releasing the owner proves the physical readiness probe is nonblocking and
   the second `PollSema` observes the published CREDIT;
9. the sole owner emits exactly one channel-1 CREDIT for 328 bytes and releases
   the submitter;
10. receive-call/thread evidence remains one physical receive owner.

The fixture explicitly requires the product source to call physical readiness
with timeout zero. The pre-correction 1000-us remote select mechanism therefore
does not satisfy the regression.

A second isolated 328-byte DATA/consume interval then proves:

- no credit remains pending from the first cycle;
- a second exact 328-byte CREDIT is returned;
- exactly two inbound frames were received total;
- no third inbound frame is needed to complete the second cycle.

## Exact HW1 Raw parser / request proof

`tests/unit/rfb_async_framing_test.c` now constructs the exact sealed HW1 RFB
message:

- 4-byte FramebufferUpdate header;
- one 12-byte Raw rectangle header;
- x=681;
- y=11;
- width=12;
- height=13;
- 156 16-bit pixels / 312 pixel bytes;
- total = 328 bytes.

Using the real clean RFB session parser and the 704x462 framebuffer fixture, it
proves:

- the 328-byte payload is accepted;
- every byte is consumed;
- session remains READY with no error;
- dirty rectangle is exactly 681,11 12x13;
- first and final pixel positions land at the expected in-bounds framebuffer
  addresses;
- with no subsequent server message,
  `pstvnc_rfb_session_try_receive_update()` returns the benign IDLE result at
  a true complete-message boundary;
- an incremental FramebufferUpdateRequest is immediately serializable with the
  established 704x462 geometry.

The fixture then performs a second isolated 328-byte Raw interval after the first
successor request and proves the same complete-message boundary plus another
successor incremental request. This prevents one successful interval from
masking stale parser/request state.

Existing flow-policy/Application tests remain the authority for one-outstanding
request accounting and presentation integration; those tests remain green on
the same exact head.

## Test-fixture reconciliation

Changing host runtime execution to the real `PollSema` seam required the
Transport AUDIO and MPEG runtime fixtures to implement the same host semaphore
primitive because both compile `src/transport/runtime.c`.

Test-only commits:

- `b2425ec5ac6abac22aa363010f27822f8da40575`
  — expose `PollSema` in the common Transport host kernel shim;
- `8f29226e65031a67abe1c0377697518c46a4dc2c`
  — add the exact HW1 Transport race witness;
- `58beadea1adbef75a8cfedd8a9dde847e5e2002f`
  — add the exact isolated Raw parser witness;
- `4e908e6b5e41f45d1db526f97e70839a82b92d77`
  — add `PollSema` semantics to AUDIO Transport fixture;
- `46fd4664f0f371a2940cbd42189f63b636974499`
  — add `PollSema` semantics to MPEG Transport fixture;
- `a0da2f833b7a1e32b8d3709a1e33fb04c931f0d7`
  — prove repeated isolated Transport CREDIT cycles;
- `e005bf001a72302ba81e688e8616910077160764`
  — prove repeated isolated RFB parser/request boundaries.

The first intermediate run after introducing real `PollSema` exposed missing
host implementations in neighboring AUDIO/MPEG Transport fixtures. The product
Transport runtime test itself already passed; PS2 compile/link and project check
also passed. The fixture-only omissions were repaired without another product
behavior change.

No product source changed after
`0c914d2b7136b18958a96f137ff3ec4722ca3723`.

## Changed paths

Relative to the assigning R44 head, changes are confined to:

- `src/transport/runtime.c`;
- `tests/unit/transport_runtime_test.c`;
- `tests/unit/transport_host_stubs/kernel.h`;
- `tests/unit/rfb_async_framing_test.c`;
- `tests/unit/transport_audio_test.c`;
- `tests/unit/transport_mpeg_test.c`.

No new product-source identifier required dictionary reconciliation. Strict
dictionary and portal/topology checks remain green with unchanged counts.

## Exact final deterministic evidence

Canonical exact-source GitHub Actions run:

`36350306508`, run 1049, attempt 1, exact head
`e005bf001a72302ba81e688e8616910077160764`, completed SUCCESS.

Canonical jobs:

- host-unit = SUCCESS;
- project-check = SUCCESS;
- dictionary-long = SUCCESS;
- ps2-compile = SUCCESS;
- ps2-link = SUCCESS;
- dictionary-reconcile = correctly SKIPPED.

Selected exact-head results include:

- `transport_runtime_test: PASS`;
- `transport_physical_stream_test: PASS`;
- `transport_rfb_channel_test: PASS`;
- `transport_rfb_provider_failure_test: PASS`;
- `rfb_async_framing_test: PASS`;
- `rfb_session_test: PASS`;
- `rfb_initial_frame_test: PASS`;
- `rfb_initial_coverage_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_AUDIO_PRODUCT_TEST=PASS`;
- `APP_AUDIO_COMPOSITION_SOURCE_TEST=PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`;
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`;
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`;
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`;
- `PI_AUDIO_PRODUCT_COMPOSITION_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The existing `transport_runtime_test` family containing the R20C/R20D/R20E
completion/drain/reclaim cases remains green. The complete Application/AUDIO/
MPEG suite that carries R42/R43 teardown/composition evidence also remains
green. No neighboring media source changed.

Final product symbol counts remain:

- `src:1132`;
- `pi:922`;
- total product symbols: `7349`.

## Linked identity / hardware boundary

The independently validated identity that failed HW1 was:

`ELF_PRISTINE_SHA256=993655ccffc430347281291c1fa917e3b22ce77f17407a99edbc920b71a65a2a`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`

`PT_LOAD_BYTES=556180`.

Final R44 exact-head identity is:

`ELF_PRISTINE_SHA256=d025df4cbe8c92e15931ab1ee30fe2f869fb2d61862441e5ad93cf4e0f20feb0`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=9c4a8d25ca9ec1798e84f716f16b711bcc86328449082e054ce1027a69016966`

`PT_LOAD_BYTES=556180`.

Therefore exact loadable program identity changed even though PT_LOAD byte count
did not.

`HARDWARE_PENDING_AFTER_HW1_PRODUCT_CORRECTION=YES`

R44 does not claim physical success. Foreman/Validation review must precede a
new hardware run.

R44_SOURCE_COMPLETE=YES_WITHIN_PACKET
R44_FIRST_MISSING_PROGRESS_FACT=SOLE_OWNER_OUTBOUND_RENDEZVOUS
R44_POST_OUTBOUND_CHECK_RACE=PROVEN_AND_CORRECTED
R44_SINGLE_PHYSICAL_IO_OWNER=PRESERVED
R44_CREDIT_AFTER_CONSUMPTION=PRESERVED
R44_COMPLETE_MESSAGE_PARSER=PASS
R44_REPEATED_ISOLATED_PROGRESS=PASS
R44_HOST_TESTED=PASS
R44_PROJECT_CHECK=PASS
R44_STRICT_DICTIONARIES=PASS
R44_PS2_COMPILE=PASS
R44_PS2_LINK=PASS
R44_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R44_MACHINE_EVIDENCE=GITHUB_ACTIONS
R44_PS2_PT_LOAD_CHANGED=YES
R44_INDEPENDENT_VALIDATION=NOT_RUN_AFTER_CORRECTION
R44_OPERATOR_OBSERVED=NO_AFTER_CORRECTION
R44_HARDWARE_QUALIFIED=NO
R44_HARDWARE_PENDING=YES

## Worker criterion disposition

Worker findings only; Foreman/Validation acceptance remains separate.

1. Exact 328-byte HW1 Raw boundary preserved without reclassification — MET.
2. First missing PS2 fact isolated to post-consumption sole-owner outbound
   rendezvous — MET.
3. Post-outbound-check publication race modeled with actual `PollSema`
   ordering and corrected without future inbound dependency — MET.
4. One physical Wire I/O owner preserved — MET.
5. CREDIT remains earned only by actual RFB parser consumption — MET.
6. No timeout-success, watchdog, reconnect, automatic ELF restart or silent
   session replacement added — MET.
7. Complete isolated Raw message consumes to a true message boundary with no
   later RFB bytes — MET.
8. 32768/8192/8192 flow values, flush-on-empty and Wire protocol remain
   unchanged — MET.
9. AUDIO/MPEG/common-clock behavior and RFB-only media dormancy remain unchanged
   and neighboring regressions are green — MET.
10. Exact isolated HW1 update proves parser consumption, exact returned CREDIT,
    completed message boundary and successor incremental request with no second
    inbound frame needed for that cycle — MET.
11. Critical concurrency fixture models real EE `PollSema` ordering rather
    than the former host shortcut and proves bounded outbound progress — MET.
12. Canonical host/project/dictionaries/pinned PS2 compile-link/reproducibility
    are green; new identity is hardware-pending and no hardware run occurred —
    MET.

Required repeated-isolated-update evidence is also MET: two independent
328-byte Transport consumption cycles each return exactly 328 bytes of CREDIT
with zero accumulated pending credit, and two complete parser intervals each
return to IDLE boundary and serialize a successor incremental request.

## Blockers and deferred debt

BLOCKERS=NONE

No hardware witness was requested because deterministic reconstruction identified
and corrected the first missing local progress fact.

The separate provider-stager STATIC/enabled apparatus defect remains outside
R44 scope.

Independent Validation and physical PS2 qualification of the new R44 identity
remain pending.

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently review the R44 product correction, exact
PollSema/select scheduling proof, focused Raw/CREDIT regressions, complete
canonical evidence and changed PT_LOAD identity. If accepted, the corrected
source must return through independent Validation before any new HW1 hardware
qualification run.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R44_REVIEW_AND_VALIDATION_ROUTING
