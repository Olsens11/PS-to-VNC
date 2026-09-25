DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T17:52:27-04:00
COMPLETED_AT=2026-09-25T18:07:30-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=bca9fe2153f97c2d0784a788f61c323197b3da39
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — B11 R31 read-only human configuration GET

## Recovery provenance

This Reconstruction shift independently recovered the live
`ledge/h1-all-guns` branch before making behavior-bearing writes.

Pickup authority was exactly:

`bca9fe2153f97c2d0784a788f61c323197b3da39`

message:

`docs(work-log): accept R30 and hand off config GET R31`.

Current Foreman State revision `0070` activated exactly:

`B11-READ-ONLY-HUMAN-CONFIG-GET-R31`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a005-interaction-input`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- management endpoint `192.168.50.1:5959`;
- config path `/ps2vnc.conf`;
- R30 parsing/publication deferred;
- R29 binding installation deferred;
- Application product-action routing deferred;
- management POST/persistence deferred.

The accepted lower authority consumed by this packet was R30 source
`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8` with immutable Reconstruction
record `377416cd03a9f24bf1d6b0cd3145b3729b600fd1`.

The newest Foreman handoff consumed was:

`docs/ledge/work-log/20260925T174540-0400__foreman__a005-interaction-input__interactive.md`.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area, or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Governing authority consumed

This shift read and followed, at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/README.md`;
- `docs/development/source-naming-and-symbols.md`;
- `docs/development/source-topology.md`;
- `docs/development/module-lifecycle.md`;
- current Reconstruction Contract;
- Work Log Contract revision `0007`;
- Foreman State revision `0070`;
- current Architecture Overlay;
- current Wire Runtime Decisions;
- Q1-Q12 reconciliation;
- `docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md`;
- current Platform network source/dictionary authority;
- frozen B4A read-only management GET evidence.

Clean Architecture explicitly places bounded HTTP mechanics in a Management
owner and permits Management to depend on Platform network mechanisms. Platform
remains lower-level and does not gain management protocol semantics.

## Objective executed

R31 reconstructs the smallest read-only PS2 management client required to
retrieve the human-readable configuration document from the Pi while preserving
strict separation from the sole PSTV Transport session.

The operation:

1. creates one fresh caller-owned management TCP descriptor through Platform;
2. connects only to `192.168.50.1:5959`;
3. sends exactly the read-only HTTP/1.0 config request;
4. accepts only bounded HTTP/1.0 or HTTP/1.1 status 200;
5. frames the body by clean peer EOF after `Connection: close`;
6. publishes exact raw body bytes/count only after complete success;
7. closes the owned management descriptor exactly once on every terminal path.

The returned bytes remain untrusted. R31 does not parse R30, install R29
bindings, select product policy, or execute a product effect.

## Exact R31 pre-log source authority

Final pre-log source authority:

`fb6eb12868a3045c098651bb97c2577a696bc686`.

From assigning authority
`bca9fe2153f97c2d0784a788f61c323197b3da39`
to final pre-log source authority, the branch is eighteen commits ahead / zero
behind:

1. `1d2d048b09aedba04f62355f22024a20c4e63306` —
   `management: define read-only config GET boundary`;
2. `0ea741d5d1f65b1ac017c14c55c909b124912b12` —
   `management: retrieve bounded human config document`;
3. `918e80f9ca6498d7e2638c4f5baa071cd18d13fa` —
   `docs(management): adopt clean management owner`;
4. `9877f9d603f0e918f75e16c35d0e55c38d0802fd` —
   `platform(network): add fixed management connection seam`;
5. `5e073e4d24605fd00fed75563cd93f55edf34944` —
   `platform(network): share fixed endpoint connect mechanism`;
6. `b420d002e4ac6e542c68cbb0737e8ed7f3127eb2` —
   `test(management): prove bounded config GET semantics`;
7. `3d6b1d26a98b4f820ae182347ea8382696d86d5f` —
   `test(management): enforce R31 ownership boundary`;
8. `2e6fdd65c12c70ec4cd824c52539d23cb09135a8` —
   `test(management): enroll R31 host evidence`;
9. `416abd4c11193b844f0e6d0a143a84d92c3136cd` —
   `build: link R31 management client`;
10. `5b717d4c0c72b65b4c1f63a25501e73281ed6145` —
    `build: PS2-compile R31 management source`;
11. `af6de0c561f98aab36ffe2231be085c2867435a6` —
    `docs(management): record R31 clean owner topology`;
12. `3b5180748f24dd382cc0f1d420174f422da19a82` —
    first deterministic dictionary-reconciliation trigger;
13. `2d6667b9e4833f33b557cc991a3b928d52aadae7` —
    `tooling(symbols): enroll management dictionary reconciliation`;
14. `c969e2a4e23399d5f6418d2e062fb3a38929edcc` —
    `ci(symbols): commit management dictionary reconciliation`;
15. `53417c5a5291ff13a6250c85c5fc406cc0eafaad` —
    final deterministic dictionary-reconciliation trigger;
16. `13f666a25f2ff566c1d254b3aae8c1971a683ebc` —
    automation-generated `docs(symbols): reconcile current clean definitions`;
17. `26c67516f8681662de6ae1994867544cd7066d52` —
    `docs(platform): explain separate R31 management endpoint seam`;
18. `fb6eb12868a3045c098651bb97c2577a696bc686` —
    `tooling(topology): enroll clean management domain`.

The first reconciliation trigger exposed that the existing deterministic helper
and CI add-list intentionally named only previously adopted clean domains.
Because R31 introduces the new authorized `src/management/` domain, the packet
also enrolled that directory into those existing reconciliation/check
mechanisms. The first trigger therefore produced no generated commit; after the
minimal enrollment repair, the second exact trigger generated the complete
dictionary authority at `13f666a25...`.

The final changed-path set is confined to:

- `src/management/config_get.c/.h`;
- `src/management/SYMBOLS.md`;
- `src/platform/ps2_network.c/.h`;
- `src/platform/SYMBOLS.md`;
- focused management tests;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `scripts/reconcile-source-dictionaries.py`;
- `scripts/continuity-check.sh`;
- `.github/workflows/ledge-reconstruction.yml`;
- `docs/development/source-topology.md`;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No Config product-action source, Input runtime, ordinary Application, UI,
RFB, Transport, MPEG, media-clock, AUDIO, Pi product/persistence or forensic
H1/B4A source changed.

## Clean Management owner

R31 establishes `src/management/` as a clean product domain.

Its public operation is:

`pstvnc_management_config_get(body, body_capacity, body_length)`.

Current explicit storage bounds are:

- maximum HTTP header: `1024` bytes;
- maximum configuration body: `4096` bytes;
- receive chunk: `256` bytes;
- fixed request storage maximum: `128` bytes.

No heap, executable configuration, arbitrary callback framework, retry loop,
watchdog, file persistence or generic HTTP framework was introduced.

The exact request bytes are:

```text
GET /ps2vnc.conf HTTP/1.0\r\n
Host: 192.168.50.1\r\n
Connection: close\r\n
\r\n
```

There is no request body and no POST surface.

## Separate management descriptor / Platform seam

Platform now exposes the narrow fixed-endpoint mechanism:

`pstvnc_ps2_network_connect_management()`

using only:

- `PSTVNC_PS2_MANAGEMENT_SERVER_IP="192.168.50.1"`;
- `PSTVNC_PS2_MANAGEMENT_SERVER_PORT=5959`.

The existing PSTV connect API remains separate and still uses only its PSTV
endpoint constants.

A small private Platform helper removes duplicate socket/connect lifecycle code
while preserving endpoint authority at the two public seams.

Management never calls `pstvnc_ps2_network_connect_pstv()`, never adopts a
descriptor into Transport, never references Transport APIs, and never closes a
PSTV descriptor.

Every management descriptor successfully returned by Platform reaches exactly
one `pstvnc_ps2_network_close()` at the sole terminal cleanup point.
Connection failure returns no owned descriptor and therefore performs no
caller-side close.

## HTTP status/header/body semantics

Header termination is recognized only at the exact byte sequence
`\r\n\r\n`, even when split across receive calls.

Only the historical status forms are accepted:

- HTTP/1.0 status 200;
- HTTP/1.1 status 200.

Malformed/short versions, non-200 statuses and missing/oversized headers reject.

Bytes coalesced in the same receive after the terminator become the first exact
body bytes. Subsequent body bytes append in receive order until clean EOF.

No Content-Length, chunked-transfer, compression, redirect or connection reuse
feature was invented. The proven HTTP/1.0 / `Connection: close` framing is the
entire body contract.

## Atomic caller publication and fail-closed I/O

The management client receives into private bounded candidate storage.

Caller-visible body bytes and caller length remain unchanged for:

- connect failure;
- partial-send zero/error before complete request;
- receive error;
- malformed/non-200 status;
- missing/oversized header;
- body overflow;
- invalid argument bounds.

Only a complete valid response terminated by clean EOF copies candidate body
bytes and publishes the exact body length.

An exact-capacity body succeeds only after a subsequent receive proves EOF.
One additional byte beyond caller capacity rejects rather than publishing a
truncated success.

An empty status-200 body is a valid success with length zero.

Partial request sends are retried only until the fixed request is complete.
There is no product retry/reconnect policy.

## Deterministic evidence disposition

R31's required deterministic evidence is satisfied as Reconstruction Worker
evidence:

1. exact GET, Host and Connection-close request bytes with no body — PASS;
2. separate management TCP5959 connection and management-only close ownership —
   PASS;
3. deterministic partial-send completion and zero/error rejection — PASS;
4. split header terminator and coalesced header/body receive handling — PASS;
5. HTTP/1.0 200 and HTTP/1.1 200 accepted; malformed/non-200/missing/oversized
   header cases rejected — PASS;
6. exact body bytes preserve first coalesced bytes plus later receive order
   through EOF — PASS;
7. exact-capacity body succeeds only after EOF; one extra byte fails atomically —
   PASS;
8. empty 200 body succeeds with exact length zero — PASS;
9. connect/send/recv failures prove management-only cleanup and unchanged seeded
   caller output/length — PASS;
10. source-boundary scan proves no PSTV Transport API, R30 parse, Application,
    Input runtime, UI, RFB, MPEG, media-clock, AUDIO, POST/persistence,
    retry/watchdog or mailbox scope — PASS;
11. R30 parser/formatter, R29 action/runtime, config-text and broader regressions
    remain green — PASS;
12. canonical host/project/strict-dictionary/PS2 compile/link/current-source
    reproducibility are green — PASS.

## Acceptance-criterion dispositions

These are Reconstruction Worker dispositions only; they are not Foreman
acceptance.

- `B11-R31-C1 CONFIG_GET_OWNS_SEPARATE_MANAGEMENT_DESCRIPTOR_NOT_PSTV_TRANSPORT`
  — MET.
- `B11-R31-C2 REQUEST_IS_EXACT_READ_ONLY_PS2VNC_CONF_HTTP10` — MET.
- `B11-R31-C3 CLEAN_MANAGEMENT_OWNER_CONTAINS_HTTP_MECHANISM` — MET.
- `B11-R31-C4 PLATFORM_PRIVATE_LINK_REUSE_DOES_NOT_CHANGE_PSTV_ADOPTION` — MET.
- `B11-R31-C5 REQUEST_HEADER_AND_BODY_MEMORY_ARE_EXPLICITLY_BOUNDED` — MET.
- `B11-R31-C6 ONLY_VALID_HTTP10_OR_HTTP11_200_IS_ACCEPTED` — MET.
- `B11-R31-C7 BODY_IS_EXACT_CONNECTION_CLOSE_PAYLOAD_WITHOUT_HTTP_FEATURE_CREEP`
  — MET.
- `B11-R31-C8 FAILURE_LEAVES_CALLER_BODY_AND_LENGTH_UNCHANGED` — MET.
- `B11-R31-C9 PARTIAL_IO_OVERFLOW_AND_ALL_DESCRIPTOR_CLEANUP_FAIL_CLOSED` — MET.
- `B11-R31-C10 RETRIEVAL_MECHANISM_OWNS_NO_STARTUP_RECOVERY_OR_DEFAULT_POLICY`
  — MET.
- `B11-R31-C11 NO_PARSE_RUNTIME_APPLICATION_MEDIA_PERSISTENCE_OR_MAILBOX_SCOPE`
  — MET.
- `B11-R31-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

All twelve R31 criteria are therefore `MET` as Worker dispositions.

## Final canonical machine evidence

Final pre-log source authority:

`fb6eb12868a3045c098651bb97c2577a696bc686`

Canonical workflow:

`Ledge reconstruction checks`
run `36195001094`
attempt `1`
conclusion `success`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected for a non-trigger final commit.

Focused/regression output includes:

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

Repository/build output includes:

- `SOURCE_TOPOLOGY_DIRECTORIES=pi,src,src/audio,src/config,src/diagnostics,src/display,src/framebuffer,src/input,src/management,src/media,src/mpeg,src/platform,src/rfb,src/transport,src/ui`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=224 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled
  `src/management/config_get.c` and `src/platform/ps2_network.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The deterministic dictionary authority now records:

- `src/management:39` complete symbols;
- `src/platform:221` complete symbols;
- synchronized aggregate portal;
- total clean product dictionary symbols `6639`.

## Exact linked PS2 identity / hardware classification

R31 changes linked PS2 bytes relative to accepted R30.

Exact R31 identity:

- `ELF_PRISTINE_SHA256=c834f488d568b4a67e9b4b0eb7622ba2ddf5d820dd95203b907350f3576fae20`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=03e6511045ed8f46ee82e91f8de7d264275b54bf840371a2dfd190367352db3e`;
- `PT_LOAD_BYTES=520724`.

Both canonical linked builds produced the same ELF and PT_LOAD fingerprints.

Accepted R30 authority was:

- `ELF_PRISTINE_SHA256=5c78663f84cf6372e9887337095d5b5da2424c2f7e26b68017387ac188d5cca8`;
- `PT_LOAD_SHA256=f6b0a92cb1667a6a5d41ec27d3836d95ddc56450e55562354894637076ceba12`;
- `PT_LOAD_BYTES=519700`.

Therefore:

`R31_SOURCE_COMPLETE=YES`
`R31_HOST_TESTED=PASS`
`R31_PROJECT_CHECK=PASS`
`R31_STRICT_DICTIONARIES=PASS`
`R31_PS2_COMPILE=PASS`
`R31_PS2_LINK=PASS`
`R31_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`R31_PS2_PT_LOAD_CHANGED=YES`
`R31_MACHINE_EVIDENCE=GITHUB_ACTIONS`
`R31_INDEPENDENT_VALIDATION=NOT_RUN`
`R31_OPERATOR_OBSERVED=NO`
`R31_HARDWARE_QUALIFIED=NO`
`R31_HARDWARE_PENDING=YES`
`R31_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

No hardware qualification was attempted or claimed.

## Findings / blockers / known-defect accounting

No R31 implementation blocker remains inside the authorized packet.

The existing Platform network seam was sufficient after adding one narrow fixed
management endpoint constructor. PSTV Transport ownership/adoption semantics
were not altered.

One intermediate host workflow at commit
`5b717d4c0c72b65b4c1f63a25501e73281ed6145` observed an existing
`transport_mpeg_test` assertion failure after the R31 management tests had
already passed. R31 changes no Transport/MPEG source. Subsequent canonical final
source workflow `36195001094` reran that test and produced
`transport_mpeg_test: PASS` together with the complete green suite. The
intermediate result is therefore recorded as superseded evidence, not hidden or
treated as R31 acceptance evidence.

The first dictionary reconciliation trigger correctly exposed that the
reconciliation target/add lists and topology checker predated the new clean
Management domain. Those packet-owned clean-domain enrollment gaps were
repaired through the existing mechanisms; final strict dictionaries and project
checks are green.

R30 parse/publication, R29 installation, Application semantic routing, POST/
persistence, binding-editor UI, recovery policy, silent-stall timeout and urgent
mailbox remain deferred exactly as assigned.

## State / contract accounting

Consumed:

- Foreman State revision `0070`;
- current Reconstruction Contract;
- Work Log Contract revision `0007`;
- current Architecture Overlay;
- current Wire Runtime Decisions;
- Q1-Q12 reconciliation;
- accepted R30 human-readable typed binding model;
- accepted R29 live Input action publication;
- B10/B11 management/config audit authority.

Produced:

- clean `src/management/` product owner;
- bounded read-only config GET mechanism;
- separate fixed management descriptor seam through Platform;
- deterministic management host/source-boundary tests;
- PS2 compile/link enrollment;
- complete Management/Platform dictionary and topology adoption;
- no Foreman State update;
- no Foreman acceptance;
- no independent Validation result;
- no R30 parse or R29 installation;
- no Application/UI product route;
- no POST/persistence;
- no recovery/watchdog/mailbox policy;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R31_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, review R31 source/evidence/this immutable record,
decide acceptance, and select any later bounded packet. Reconstruction must not
self-accept R31 or begin deferred parse/install/routing/persistence work.
