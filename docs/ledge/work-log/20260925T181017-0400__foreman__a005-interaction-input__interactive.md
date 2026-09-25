DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T18:10:17-04:00
COMPLETED_AT=2026-09-25T18:16:23-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=d80ab11662ef5b53b8ac792331ee69015deb37eb
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R31 accepted; Application binding snapshot R32 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`B11-READ-ONLY-HUMAN-CONFIG-GET-R31`, independently recover live repository
authority, inspect the exact R31 source/test/build range, disposition R31, and
publish exactly one dependency-correct bounded Reconstruction packet without
performing product implementation from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T18:10:17-04:00`

Live pickup authority:

`d80ab11662ef5b53b8ac792331ee69015deb37eb`

The branch was nineteen commits ahead / zero behind prior Foreman closeout
`bca9fe2153f97c2d0784a788f61c323197b3da39`.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0070;
- B10/B11 configuration/recovery/management audit authority;
- A005 interaction/input audit revision 0001;
- A006 orchestration/shutdown audit revision 0001;
- accepted R28/R29 semantic-input authority;
- accepted R30 human-readable binding authority;
- current Platform network and clean topology authority;
- frozen B4A read-only config retrieval evidence.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T175227-0400__reconstruction__a005-interaction-input__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R31
during pickup.

## Exact R31 range

Assigning Foreman authority:

`bca9fe2153f97c2d0784a788f61c323197b3da39`

Final pre-log source authority:

`fb6eb12868a3045c098651bb97c2577a696bc686`

Immutable Reconstruction closeout:

`d80ab11662ef5b53b8ac792331ee69015deb37eb`

Independent GitHub compare proves exactly eighteen pre-log commits ahead / zero
behind, followed by exactly one log-only commit.

The R31 changed-path set is confined to:

- new `src/management/config_get.c/.h`;
- new `src/management/SYMBOLS.md`;
- narrow fixed-endpoint changes in `src/platform/ps2_network.c/.h`;
- Platform dictionary reconciliation;
- focused management tests;
- test/build enrollment;
- deterministic dictionary/topology enrollment for the new Management domain;
- source-topology and generated dictionary documentation.

No R30 Config binding source, R29 Input runtime, ordinary Application, UI, RFB,
Transport, MPEG, media-clock, AUDIO, Pi product/persistence or forensic H1/B4A
source changed.

## Independent R31 source review

### Separate management ownership

The clean Management public surface exposes only:

`pstvnc_management_config_get(body, body_capacity, body_length)`.

It acquires exactly one fresh descriptor through:

`pstvnc_ps2_network_connect_management()`.

The existing PSTV constructor remains:

`pstvnc_ps2_network_connect_pstv()`.

Both use one private Platform endpoint-connect helper, but endpoint identity and
higher-owner lifecycle remain distinct. Management never adopts into or calls
Transport.

### Exact request contract

The request bytes are exactly:

`GET /ps2vnc.conf HTTP/1.0\r\n`
`Host: 192.168.50.1\r\n`
`Connection: close\r\n`
`\r\n`

There is no body, POST or arbitrary public path.

Partial sends advance by the actual positive byte count. Zero, negative or
impossible over-reporting fails closed.

### Bounded HTTP framing

Current explicit bounds are:

- request maximum 128 bytes;
- HTTP header maximum 1024 bytes;
- receive chunk 256 bytes;
- configuration body maximum 4096 bytes.

Header completion requires exact `\r\n\r\n`, including split-boundary
cases.

Only HTTP/1.0 or HTTP/1.1 status 200 is accepted. Non-200, malformed/short
versions, missing terminator and oversized headers reject.

R31 deliberately implements no Content-Length, chunking, compression, redirect
or keep-alive framework. The body is all bytes after the header terminator
through clean peer EOF.

### Atomic body publication

All raw body accumulation occurs in private candidate storage.

A successful result copies exact candidate bytes and then publishes exact length.

Failure leaves caller body and caller length unchanged.

An exact-capacity response remains provisional until a later recv returns EOF.
Any additional byte beyond capacity rejects rather than producing a truncated
success.

A valid 200 response with empty body succeeds with length zero.

### Descriptor cleanup

A descriptor returned by the management Platform seam reaches one terminal
`pstvnc_ps2_network_close()`.

Connection failure returns no caller-owned descriptor and therefore causes no
duplicate close.

The Management owner does not close or reference any Transport-owned PSTV
descriptor.

### Scope review

R31 contains no:

- R30 parse or Configuration publication;
- R29 binding installation;
- Application product-action route;
- UI behavior;
- RFB/Transport product I/O;
- MPEG/media-clock/AUDIO effect;
- Pi persistence or POST;
- retry/reconnect/silent-stall policy;
- urgent mailbox.

No R31 product defect was found.

## R31 acceptance criteria

B11-R31-C1=MET
B11-R31-C2=MET
B11-R31-C3=MET
B11-R31-C4=MET
B11-R31-C5=MET
B11-R31-C6=MET
B11-R31-C7=MET
B11-R31-C8=MET
B11-R31-C9=MET
B11-R31-C10=MET
B11-R31-C11=MET
B11-R31-C12=MET

Overall disposition:

`R31_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R31 machine/build evidence

Final source authority:

`fb6eb12868a3045c098651bb97c2577a696bc686`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36195001094`
attempt `1`
conclusion `success`

The exact run object reports:

- branch `ledge/h1-all-guns`;
- head SHA `fb6eb12868a3045c098651bb97c2577a696bc686`;
- push event;
- title `tooling(topology): enroll clean management domain`.

Observed focused/cross-domain evidence:

- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `config_text_test: PASS`;
- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build evidence:

- Management appears in the exact clean source-topology directory set;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=224 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled
  `src/management/config_get.c` and `src/platform/ps2_network.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=c834f488d568b4a67e9b4b0eb7622ba2ddf5d820dd95203b907350f3576fae20`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=03e6511045ed8f46ee82e91f8de7d264275b54bf840371a2dfd190367352db3e`
`PT_LOAD_BYTES=520724`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

The worker transparently recorded an intermediate `transport_mpeg_test`
failure at commit `5b717d4c...`. R31 modifies no Transport/MPEG source. The
exact final-source canonical run reran that test and reports
`transport_mpeg_test: PASS`; only the exact final-source run is used for R31
acceptance.

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
PS2_PT_LOAD_CHANGED=YES
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency analysis

The clean stack now has:

- R30: typed human-readable binding parser/formatter;
- R31: raw read-only management document retrieval;
- R29: live Input resolver/publication, but zero-bound by ordinary Application;
- P9/P10/R21-R24: trigger-agnostic MPEG lifecycle mechanisms not yet composed
  into ordinary product.

B10 authority says ordinary human configuration retrieval failure and malformed
user configuration are non-fatal startup conditions. The clean binding fallback
must therefore be an explicit zero-binding value, not a guessed physical chord.

Ordinary `app.c` currently has no PRODUCT_ACTION switch case. Installing a
valid nonzero R30 binding into R29 before the Application consumer exists would
make a user-configured gesture produce an event that the current ordinary
Application treats as invalid/fatal.

The safe dependency order is consequently:

1. acquire/validate and retain desired binding authority;
2. reconstruct the Application action consumer/lifecycle route;
3. only then install nonzero desired bindings into live Input.

R32 implements only step 1.

## R32 architecture decision

The active packet is:

`B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32`.

One Application-owned resident-start process will:

- start from exact zero binding;
- call R31 once after network/link availability;
- parse successful raw bytes through R30 unchanged;
- retain an exact valid typed model when present;
- treat fetch or parse failure as nonfatal zero-binding fallback;
- retain the same snapshot across R16B provider replacement attempts.

The process may live directly in `app.c` or in one coherent Application-root
helper if independent host testing warrants the split.

It must not install the result into Input and must not make PRODUCT_ACTION
reachable.

## Foreman-owned state publication

Published Foreman State revision 0071:

`5971ce0611f7e1c9cdd513d1009153eafa58e3a1`

message:

`docs(foreman): accept R31 and activate binding snapshot R32`

State 0071:

- accepts R31 and all twelve criteria;
- records exact R31 source, CI and hardware-debt identity;
- activates exactly one next packet:
  `B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32`;
- keeps Reconstruction work key `a005-interaction-input`;
- requires once-per-resident post-link/pre-PSTV acquisition;
- makes fetch/parse failure nonfatal with exact zero-binding fallback;
- preserves a valid user-supplied typed binding without default substitution;
- explicitly forbids live R29 installation and PRODUCT_ACTION routing;
- preserves all media/UI/persistence/mailbox work as deferred;
- claims no hardware qualification.

Immediately before this immutable log write, live branch authority remained
exactly at State 0071.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R31:

`PT_LOAD_SHA256=03e6511045ed8f46ee82e91f8de7d264275b54bf840371a2dfd190367352db3e`
`PT_LOAD_BYTES=520724`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi operator/hardware qualification claim.

R32 may change linked Application code and must measure any new PT_LOAD identity
exactly.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R32_FROM_FOREMAN_STATE_0071

The Reconstruction worker must independently recover live branch authority,
consume State 0071 plus B10/B11/A005/A006 and R29-R31 authority, execute only
R32, create exactly one immutable Reconstruction record using WORK_ITEM_KEY
`a005-interaction-input`, stop, and return the baton.

Do not install bindings into Input, do not route PRODUCT_ACTION, do not invoke
P9/P10 or start MPEG, do not add persistence/editor/reload behavior, do not add
a recovery watchdog or urgent mailbox, and do not claim hardware qualification.
