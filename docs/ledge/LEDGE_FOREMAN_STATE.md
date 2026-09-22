# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0045
RECORDED_AT=2026-09-22T08:22:04-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0044
SUPERSEDES_FOREMAN_STATE_REVISION=0044
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0045 independently accepts the completed
`A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15` source baton and activates the
next bounded dependency: explicit RFB failure / stop / restart policy through
Application and the existing RFB/Wire owner seams. R15 makes the ordinary
no-argument PS2 product path consume the accepted R14 profile before startup,
makes the default Pi Wire service compose a fresh R13 attachment from the same
selected profile, preserves first-CREDIT-lazy provider start, leaves Q4/Wire
bytes unchanged, keeps provider failure RFB-local, and stages the Pi runtime
without mutating a live service.

This revision deliberately does **not** claim hardware qualification. The exact
current linked PS2 ELF remains unqualified, as do the R15 ordinary Pi/PS2
activation path and all earlier hardware-pending ledge mechanisms.

## State-snapshot compaction note

Revision 0044 and earlier Foreman states accumulated long chronological
acceptance narratives. Revision 0045 returns this file to its declared
`STATE_SNAPSHOT` purpose: current authority, current debt, the active packet,
and the immediate dependency graph are kept here, while prior state text remains
preserved in Git history and immutable worker records under
`docs/ledge/work-log/`.

No prior source acceptance is revoked by this compaction. Historical packet
criteria/evidence remain recoverable from their original state revisions and
immutable work logs. Where an older as-of-R13/R14 activation-status sentence in
Architecture Overlay revision 0007 differs from this later state, revision 0045
is the later temporal authority; the overlay's ownership/invariant rules remain
governing.

## Current Foreman phase

`A003_R15_INTEGRATED__RFB_FAILURE_STOP_RESTART_POLICY_RECONSTRUCTION_ACTIVE__PI_MPEG_CONTROL_PRODUCER_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

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
RFB_FAILURE_RESTART_POLICY=RECONSTRUCTION_ACTIVE
PI_MPEG_CONTROL_PRODUCER_OWNER=DEPENDENCY_QUEUED
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Accepted R15 authority

PACKET_ID=A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15
PACKET_STATUS=COMPLETED_FOREMAN_ACCEPTED
ASSIGNING_FOREMAN_COMMIT=fa61f9db392d9984f520ec8559c3137a9b16e958
FINAL_R15_SOURCE_COMMIT=377ab24f066a71a813aecccd4c92ccb096d94c27
R15_RECOVERY_LOG_COMMIT=63adce230262343f1987255ad848fed779c9ccef
R15_COMMIT_COUNT=18
R15_COMPARE_BEHIND_BY=0

The required Reconstruction closeout is:

`docs/ledge/work-log/20260922T075533-0400__reconstruction__a003-mpeg-generation__interactive.md`

The earlier Foreman pickup correctly refused to accept provisional R15 work
without that immutable closeout and recorded the blocker at
`944cd9d453697d00288f653c854bdeacf69b016c`. The recovery shift then preserved
all existing R15 commits and emitted a truthful recovery record without
inventing timestamps for the timed-out coding shift.

### R15 criterion disposition

A003-R15-C1=MET
A003-R15-C2=MET
A003-R15-C3=MET
A003-R15-C4=MET
A003-R15-C5=MET
A003-R15-C6=MET
A003-R15-C7=MET
A003-R15-C8=MET
A003-R15-C9=MET
A003-R15-C10=MET
A003-R15-C11=MET
A003-R15-C12=MET

Evidence and independent Foreman findings:

1. `src/app.c` now makes the no-argument `pstvnc_app_run()` project the selected
   R14 profile before any platform/network/Transport startup. Projection failure
   returns `-1` with no startup side effect. The configured Application lifecycle
   remains the existing owner.
2. `tests/unit/app_test.c` independently proves the exact selected R14 Transport
   values reach the existing Application lifecycle: queue/initial credit 32768,
   credit batch 8192, flush-on-empty 1, credit return 1, receiver stack 16384,
   receiver priority 63, and max DATA payload 8192.
3. New `pi/wire_runtime.py` composes the ordinary Pi product process from the R14
   selector plus the accepted R13 `RfbAttachment` factory. OFF returns no
   attachment factory; invalid profile selection fails before listener
   construction.
4. The R15 runtime layer contains no copied 32768/8192/16384 tuning literals and
   no independent provider endpoint policy. The canonical Configuration-owned
   R14 source remains the authority.
5. Each `WireServer.serve_connection()` receives a fresh attachment from the
   factory. Accepted R13 state, wake descriptors, provider descriptors and
   quiesce state are therefore not reused across Wire Sessions.
6. The first valid nonzero channel-1 CREDIT remains the only provider-start
   edge. R15 does not eagerly connect/start the provider.
7. The selected internal provider remains R12's Pi-local 127.0.0.1:5900 route.
   R15 introduces no competing PS2-facing direct RFB socket and does not revive
   historical 5903 test topology.
8. R15 does not modify `pi/wire_protocol.py`, `pi/wire_server.py` product source,
   Transport protocol source, or Q4 establishment representation. Q4 remains
   the sole Wire Session identity.
9. Provider connect/read/write failure remains RFB-local under accepted R13.
   R15 adds no retry, reconnect, backoff, heartbeat, or recovery loop.
10. `scripts/pi/install-wire-runtime.sh` stages/verifies exact tracked runtime
    files and explicitly refuses live enabled/active Wire-service mutation. It
    does not daemon-reload, enable, start, stop, restart, or otherwise change
    service/provider state.
11. The default systemd unit now invokes `wire_runtime.py`. Its pre-existing
    `Restart=on-failure`/`RestartSec=1s` service supervision predates R15 and is
    not new RFB recovery policy.
12. The R13 host scheduling correction is test-only: the proof waits for the
    post-REQUEST `WAIT_BOUNDARY` local state instead of assuming scheduler order.
    Product R13 behavior is unchanged.
13. The accidental temporary test-file commit and its cleanup remain visible in
    chronology; no rewrite hid them.
14. No R15 source change imports AUDIO/MPEG generation, CONFIG-on-Wire, a second
    physical owner, or direct RFB bypass.

### Exact current machine evidence

At exact branch authority `63adce230262343f1987255ad848fed779c9ccef`, GitHub
Actions run `35724698349` completed SUCCESS.

Observed successful jobs:

- `host-unit`
- `project-check`
- `dictionary-long`
- `ps2-compile`
- `ps2-link`

`dictionary-reconcile` was correctly skipped because the committed dictionaries
were already reconciled.

Current linked-build evidence:

ISSUE7_LINKED_BUILD=PASS
LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS
ELF_PRISTINE_SHA256=f50b67db26cab9a3ad2a6877ab445a3583c418bb0b8cdd609d4af9245eb61578
PT_LOAD_SEGMENTS=1
PT_LOAD_SHA256=093b3398c5efd8af619b041384e574f2ffa2e94634f5c83d71e41043093eca14
PT_LOAD_BYTES=484884
PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

The prior accepted R14 linked PT_LOAD was 484756 bytes, so R15 adds 128 linked
PT_LOAD bytes. Those bytes are reproducible but not hardware-qualified.

R15_SOURCE_COMPLETE=YES
R15_HOST_TESTED=YES
R15_PROJECT_CHECK=PASS
R15_STRICT_DICTIONARIES=PASS
R15_PS2_COMPILE=PASS
R15_PS2_LINK=PASS
R15_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R15_OPERATOR_OBSERVED=NO
R15_HARDWARE_QUALIFIED=NO

## Governing invariants for the active RFB recovery dependency

The following already-governing authority is unchanged and must constrain R16:

1. There is one physical PS2↔Pi Wire connection owner and one physical receive
   owner per Wire Session. RFB never owns or directly reads/writes that socket.
2. Q4 is the only Wire Session establishment/identity transaction. R16 must not
   create a second session identity or skip Q4 on restoration.
3. RFB provider failure is RFB-local mechanism/domain failure. The provider or
   attachment may not directly terminate, rebind, or take ownership of Wire.
4. A dead R13 attachment is never rebound. Its provider socket, private wake
   descriptors, relay state and quiesce state must be completely retired before
   any replacement instance is admitted.
5. Application owns cross-domain product recovery policy. RFB owns RFB session
   semantics; Transport owns Wire/session and logical-channel mechanism; the Pi
   attachment owns Pi-local provider attachment mechanism.
6. Failure convergence is monotonic toward stop. No failed subcomponent is
   silently restarted inside its still-live old instance/generation as an
   implicit recovery trick.
7. No generic timeout converts unexplained lack of progress into successful
   retirement/recovery. Poll/yield delays are not lifecycle authority.
8. A successfully transmitted protocol/result fact can never erase an earlier
   local failure.
9. Historical direct-RFB provider service definitions remain evidence/fallback,
   not an ordinary product bypass.
10. Hardware evidence from older H1/checkpoint ELFs does not qualify the current
    reconstructed source.

## ACTIVE RECONSTRUCTION PACKET

PACKET_ID=A003-RFB-FAILURE-STOP-RESTART-POLICY-R16
PACKET_STATUS=ACTIVE
PACKET_OWNER=RECONSTRUCTION
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
BASED_ON_FOREMAN_STATE_REVISION=0045
BASED_ON_ACCEPTED_R15_SOURCE=377ab24f066a71a813aecccd4c92ccb096d94c27

### Objective

Reconstruct the smallest explicit ordinary-RFB failure/stop/restart policy that
makes accepted R13 provider failure visible to Product/Application, prevents a
silent RFB wait from masquerading as health, proves complete retirement before
any recovery attempt, and restores ordinary RFB only through fresh authority.

R16 is a recovery-policy packet, not a media packet and not permission to
redesign Wire Protocol opportunistically.

### Required behavior

1. **Expose provider failure through an explicit owner seam.** Provider connect
   failure, provider EOF/read failure, and provider write failure must become an
   observable RFB terminal/failure fact instead of leaving the PS2/Application
   in an indefinite ordinary-RFB wait. Preserve the concrete first/specific
   failure evidence where the owning layer can do so.
2. **Keep failure ownership local.** The provider and R13 attachment must not
   directly close/reconnect/rebind the physical Wire socket or acquire product
   recovery policy. Application remains the policy owner.
3. **Preserve normal R13 finite quiesce.** Do not ambiguously reinterpret the
   existing zero-length REQUEST/BOUNDARY/COMMIT/COMPLETE sequence. Normal
   parser-safe quiesce remains distinct from failure reporting.
4. **Converge the current ordinary RFB/Application session to stop.** Once
   Application receives a terminal RFB failure/stop fact, it admits no new RFB
   work for that failed instance and drives existing owners toward their
   documented teardown boundaries. The old provider attachment must reach
   terminal retirement and close its provider/wake resources.
5. **Prove complete stop before restart.** Input/RFB/Transport/session resources
   belonging to the failed attempt must be dormant/retired before a later
   attempt can acquire authority. Do not force-delete or reuse a possibly-live
   owner merely to make recovery proceed.
6. **Fresh authority on restoration.** A later recovery attempt must establish
   fresh session-scoped authority: fresh Q4 Wire Session identity when the old
   Wire/Application session was retired, fresh R13 attachment/factory result,
   fresh RFB parser/session state, fresh quiesce/wake state, and no stale
   Transport access ticket, credit, sequence, provider descriptor, framebuffer
   validity or input-worker state from the prior attempt.
7. **Do not rebind dead authority.** No failed attachment, stale access ticket,
   old Q4 session identity or old session-scoped queue/credit state may be
   relabeled as the replacement instance.
8. **No success-by-delay.** Recovery may yield while attempting explicit owned
   operations, but a sleep/timeout/counter is not proof that an old owner is
   stopped, that a provider is healthy, or that a new session is accepted.
9. **Keep Wire/session failure distinct.** A genuine physical Wire/Transport
   failure remains a Transport/session fact. RFB provider failure must not be
   mislabeled as a physical-I/O failure merely to obtain recovery behavior.
10. **Keep the protocol boundary honest.** Q4 bytes, existing Wire frame kinds,
    channel numbers and the accepted normal finite-RFB quiesce representation
    are frozen for this packet. If exact provider-failure visibility cannot be
    implemented without a new/ambiguous Wire representation, STOP `BLOCKED`,
    preserve that proof, and return the baton to the Foreman. Do not silently
    invent a new frame flag, frame kind, Wire version, control channel or marker
    meaning under R16.
11. **No media scope.** Do not activate AUDIO, MPEG START/RETIRE, the Pi MPEG
    producer, decoder/presentation recovery, or final all-guns orchestration.
12. **Preserve the selected R14 profile.** R16 may consume the accepted profile;
    it may not duplicate or retune the selected 32768/8192/16384/63 values.

### Acceptance criteria

- A003-R16-C1 PROVIDER_CONNECT_FAILURE_VISIBLE
- A003-R16-C2 PROVIDER_EOF_READ_FAILURE_VISIBLE
- A003-R16-C3 PROVIDER_WRITE_FAILURE_VISIBLE
- A003-R16-C4 PROVIDER_FAILURE_DOES_NOT_ACQUIRE_WIRE_OWNERSHIP
- A003-R16-C5 NORMAL_R13_QUIESCE_REPRESENTATION_UNCHANGED
- A003-R16-C6 APPLICATION_FAILURE_CONVERGENCE_EXPLICIT
- A003-R16-C7 COMPLETE_STOP_BEFORE_RESTART_PROVEN
- A003-R16-C8 RESTORATION_USES_FRESH_SESSION_SCOPED_AUTHORITY
- A003-R16-C9 NO_STALE_ATTACHMENT_ACCESS_TICKET_CREDIT_OR_WAKE_REUSE
- A003-R16-C10 PHYSICAL_WIRE_FAILURE_REMAINS_DISTINCT
- A003-R16-C11 NO_PROTOCOL_MEDIA_TIMEOUT_OR_RETUNE_SCOPE_CREEP
- A003-R16-C12 STABLE_TEST_DOC_DICTIONARY_BUILD_BOUNDARY

All twelve criteria must be MET for source acceptance. If C1-C3 cannot be met
without violating C5/C10 or the frozen protocol boundary, the correct packet
result is `BLOCKED`, not a speculative protocol extension.

### Required evidence

R16 must preserve tests/evidence that distinguish at least:

1. provider connect failure before ordinary RFB becomes ready;
2. provider EOF/read failure after attachment/start;
3. provider write failure after PS2->provider traffic exists;
4. normal server-driven finite quiesce from provider failure;
5. RFB-local failure from genuine physical Wire failure;
6. old-instance complete stop from later restoration admission;
7. fresh attachment/wake/quiesce/session state from all prior failed state;
8. stale old Transport/RFB access rejected after retirement;
9. R14 selected values remain exact and singular;
10. no new Wire/Q4 representation or media activation;
11. canonical host tests, project check and strict dictionaries;
12. clean pinned-toolchain PS2 compile/link plus reproducibility if PS2 linked
    bytes change.

Any source/test documentation changed by R16 must retain file synopses, clear
ownership comments, and source-symbol dictionary coverage under current project
rules.

### Authorized source surface

R16 may modify only the smallest surfaces required to express the above policy,
including as justified by source evidence:

- Application coordinator/public seam (`src/app.*`);
- RFB session/bridge lifecycle surfaces (`src/rfb/*`);
- existing Transport/RFB bridge status exposure only where required to surface
  an already-owned lifecycle fact (`src/transport/*`), without changing frozen
  Wire representation;
- Pi RFB attachment / Wire composition surfaces (`pi/rfb_attachment.py`,
  `pi/wire_server.py`, `pi/wire_runtime.py`) only as required for explicit
  failure/status handoff and fresh-instance lifecycle;
- bounded unit/integration fixtures for those surfaces;
- directly affected component documentation, dictionaries and build/test
  manifests.

This authorization is not permission for unrelated refactors.

### Non-goals / forbidden expansion

Do not:

- add or activate MPEG/PCM/audio behavior;
- add heartbeat/liveness protocol;
- add a new Wire frame kind/channel/version/marker/flag meaning;
- change Q4 HELLO/ACCEPT/NOT_ACCEPTED bytes;
- reopen a direct-RFB PS2-facing product socket;
- add a second physical receive/send owner;
- make systemd service restart policy stand in for Application recovery;
- retry or rebind a failed R13 attachment in place;
- use generic timeout/backoff as lifecycle proof;
- import H1 checkpoint coordinator code wholesale;
- claim hardware qualification from host/CI evidence.

### R16 stop rule

Stop after R16. Emit exactly one immutable Reconstruction work-log record under
work-log contract revision 0007 and return the baton to the Foreman.

If the frozen Wire representation makes provider-failure visibility impossible,
stop earlier with `STATUS=BLOCKED`, include the exact source/mechanism proof of
the missing seam, make no speculative protocol extension, and return the baton.

## Deferred dependency graph after accepted R16

1. add Pi MPEG START/RETIRE control ownership behind the established Wire/rider
   architecture;
2. reconstruct the Pi MPEG producer with exact-run admission, one-way retirement
   and no post-retire DATA;
3. compose the accepted MPEG runtime profile/calibration into bounded
   Application activation;
4. reconstruct current-Q7 ordered MPEG retirement / RFB restoration through
   Application;
5. complete remaining Application repeated-session/all-guns orchestration in
   dependency order;
6. perform fresh exact-source hardware qualification as separately authorized.

Foreman must re-evaluate the returned R16 authority before authorizing item 1.

## Hardware qualification debt

HARDWARE_PENDING=R16 explicit RFB failure/stop/restart policy; R15 ordinary default RFB product activation and its +128 linked PS2 PT_LOAD bytes; R14 shared-profile +128-byte PT_LOAD and Pi provider-write-capacity projection; corrected R13 provider attachment/quiesce and local request-wake; R12 internal X0tigervnc loopback endpoint staging/live demand activation; R11 selected native-provider source authority; R10 product bidirectional RFB Relay/credit mechanics; R9 product Q4 client; R8 Pi Wire service/no-carrier/listener lifecycle; reconstructed A003 R3-R7 MPEG runtime; MPEG repeated-run stale fencing; Wire-loss during MPEG; current-Q7 overlapped RFB restoration; A004 visible handoff; all-guns endurance; exact final product ELF

HARDWARE_QUALIFIED=NO

Historical H1/manual hardware evidence remains mechanism/provenance evidence only
and does not qualify reconstructed product bytes.

## Foreman next pickup

Consume `A003-RFB-FAILURE-STOP-RESTART-POLICY-R16` only after the Reconstruction
worker emits its immutable closeout. Independently refresh branch authority,
read the active packet and returned log, review exact changed source/tests,
classify any protocol-gap BLOCKED return without inventing a workaround, and
accept or correct the packet criterion-by-criterion.
