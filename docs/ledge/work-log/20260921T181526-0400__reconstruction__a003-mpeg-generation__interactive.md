# Reconstruction shift — A003 Pi internal RFB provider endpoint R12

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T18:15:26-04:00
COMPLETED_AT=2026-09-21T18:31:30-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=956ffd1f521f6e6654d6161d2cd34f01225b5b9e
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-PI-RFB-INTERNAL-PROVIDER-ENDPOINT-R12`

R12 reconstructs the mature Pi-local provider endpoint that a later bounded
R10 Relay connector can consume without leaving the RFB provider as a competing
PS2-facing product service.

Selected R12 provider-side route:

    future R10 Relay connector
        -> 127.0.0.1:5900
        -> ps-to-vnc-rfb-internal.socket
        -> ps-to-vnc-rfb-internal-x0tigervnc.service
        -> /usr/bin/X0tigervnc -display :0 -rfbport -1
        -> existing LightDM/Xorg :0

R12 does not attach the Relay, implement quiesce/provider-failure orchestration,
change PS2 source, activate live Pi units, or make a fresh hardware claim.

## Authority consumed

Truthful shift start:

`2026-09-21T18:15:26-04:00`

Live pickup authority:

`956ffd1f521f6e6654d6161d2cd34f01225b5b9e`

Foreman authority had independently accepted R11 and assigned:

- `PACKET_ID=A003-PI-RFB-INTERNAL-PROVIDER-ENDPOINT-R12`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

Current authority read at wake included:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/PROJECT_INTENT.md;
- docs/CLEAN_ARCHITECTURE.md;
- current source naming/topology/module-lifecycle guidance;
- current Reconstruction Contract and immutable work-log contract;
- current Wire Runtime Decisions;
- current Architecture Overlay;
- current A001 Transport/RFB audit;
- newest Foreman immutable log;
- accepted R10 Pi Wire server/Relay/protocol/service;
- accepted R11 native X0 provider drop-in, direct socket/base/persistent units,
  exact package authority, inactive stager and current Pi docs;
- current Pi provisioning, dependency ledger and file/service map.

Reference-only H1 CP2H bridge/session-adapter material was also inspected for
the demonstrated provider-local loopback lesson.

## Reference lesson carried forward without structural merge

The H1 CP2H evidence used one ordinary Pi-local VNC upstream connection and
defaulted that local provider endpoint to:

`127.0.0.1:5900`

The experiment preserved one physical PSTV connection and kept the VNC-provider
connection separate from the physical PS2 transport. It did not make the exact
loopback port an architectural identity.

R12 adopts the useful deployment lesson only:

- provider endpoint is Pi-local;
- Relay later connects to the local provider;
- provider remains ordinary RFB;
- physical PS2 product connection remains Wire-only;
- no second PS2-facing provider socket is introduced.

The explicitly excluded historical/operator
`127.0.0.1:5903` route is not reused.

## Internal socket authority

Added:

`systemd/pi/ps-to-vnc-rfb-internal.socket`

Git blob at final pre-log authority:

`ff7255440a9f3b8bd15bdfbac0cf895d8e601464`

Selected directives:

```ini
[Unit]
Conflicts=ps-to-vnc-rfb.socket ps-to-vnc-rfb-tigervnc.service ps-to-vnc-rfb-tigervnc-persistent.service
After=ps-to-vnc-rfb.socket ps-to-vnc-rfb-tigervnc.service ps-to-vnc-rfb-tigervnc-persistent.service

[Socket]
ListenStream=127.0.0.1:5900
Accept=no
Service=ps-to-vnc-rfb-internal-x0tigervnc.service

[Install]
WantedBy=sockets.target
```

The socket owns exactly one selected provider endpoint:

`127.0.0.1:5900`

It contains no:

- wildcard listener;
- IPv6 listener;
- `192.168.50.1` bind;
- `BindToDevice=eth0`;
- `5903`;
- Wire endpoint;
- PS2-facing product-listener claim.

The unit comments explicitly classify the socket as Pi-local provider
infrastructure for a later Relay attachment.

## Internal X0tigervnc provider authority

Added:

`systemd/pi/ps-to-vnc-rfb-internal-x0tigervnc.service`

Git blob at final pre-log authority:

`41305e57aa8bb684b0231f58f873afe9c346b42a`

Selected unit dependencies:

```ini
Requires=ps-to-vnc-rfb-internal.socket lightdm.service
After=ps-to-vnc-rfb-internal.socket lightdm.service
Conflicts=ps-to-vnc-rfb.socket ps-to-vnc-rfb-tigervnc.service ps-to-vnc-rfb-tigervnc-persistent.service
After=ps-to-vnc-rfb.socket ps-to-vnc-rfb-tigervnc.service ps-to-vnc-rfb-tigervnc-persistent.service
```

Selected service state:

```ini
Type=simple
User=ps2
Group=ps2
WorkingDirectory=/home/ps2
UMask=0077
Environment=DISPLAY=:0
Environment=XAUTHORITY=/home/ps2/.Xauthority
StandardInput=null
StandardOutput=journal
StandardError=journal
ExecStart=/usr/bin/X0tigervnc -display :0 -rfbport -1 -SecurityTypes None -AlwaysShared=1 -AcceptPointerEvents=1 -AcceptKeyEvents=1 -AcceptSetDesktopSize=0 -UseIPv6=0
```

The non-empty ExecStart is exactly the selected R11 X0tigervnc provider command.

Therefore R12:

- reuses existing LightDM/Xorg display `:0`;
- reuses the ps2 Xauthority;
- reuses R11 SecurityTypes/shared/input/resize/IPv6 policy;
- consumes the systemd-inherited descriptor with `-rfbport -1`;
- does not create a second RFB listener;
- does not create another X desktop;
- does not adopt the failed historical X0 logger spelling.

## Direct/internal mutual exclusion

R12 deliberately leaves all R11/direct historical files byte-identical and
places the new mutual-exclusion authority only in the two new internal units.

Both new units declare:

`Conflicts=ps-to-vnc-rfb.socket ps-to-vnc-rfb-tigervnc.service ps-to-vnc-rfb-tigervnc-persistent.service`

and order themselves after the same three direct alternatives.

This uses ordinary systemd negative dependency plus ordering semantics so the
internal and direct alternatives cannot be selected concurrently and transition
ordering is explicit.

No R11 file was rewritten merely to teach historical evidence about the new
internal alternative.

## R11 direct and historical authority preserved exactly

Compared with R12 Foreman base
`956ffd1f521f6e6654d6161d2cd34f01225b5b9e`, these files are byte-identical:

### Direct PS2-facing socket

`systemd/pi/ps-to-vnc-rfb.socket`

Git blob:

`e24e4e4f389c155c7e6eff0fbd93c4c01602a361`

It remains the preserved direct
`192.168.50.1:5900` / `eth0` / `FreeBind=yes` route.

### Historical socket-activated provider base

`systemd/pi/ps-to-vnc-rfb-tigervnc.service`

Git blob:

`bb2e02bda68d40baa4b8c5152c19dbe825978038`

It retains the qualified historical Xtigervnc `:1 -inetd` definition.

### Historical persistent provider control

`systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service`

Git blob:

`64874d9c5018d75e330140d3d833f5bb9ccd28d4`

It remains the direct persistent/fallback control.

### R11 native direct-provider override

`systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`

Git blob:

`4dfbdbe7a3fc8ef4964c5704a337631482296cbb`

Its historical exact-byte identity remains:

- 919 bytes;
- SHA-256
  `cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`.

R12 therefore preserves both the physically qualified direct history and R11's
native direct-route provenance while selecting the new internal provider
endpoint as current mature provider-side authority.

## Windows/operator 5903 remains excluded

R12 selected product/provider authority contains no
`127.0.0.1:5903`.

Current docs continue to classify that endpoint as Windows/operator development
tooling.

The new internal socket, internal service and internal stager contain no 5903
selected path.

R10 Wire/Relay/product service likewise remain free of 5903.

## Separate fail-closed internal staging

Added:

`scripts/pi/install-rfb-internal-provider-units.sh`

Git blob at final pre-log authority:

`c05ed09120cf3401aa0e4352a7634b3d949cebdd`

The tool deliberately has its own:

- `stage`;
- `verify`;
- `remove`.

It installs only:

1. `/etc/systemd/system/ps-to-vnc-rfb-internal.socket`;
2. `/etc/systemd/system/ps-to-vnc-rfb-internal-x0tigervnc.service`.

Both are mode `0644`.

The R11/direct definitions are **not** installation targets of this tool.

The tool requires:

- tracked internal sources;
- tracked direct/R11 sources for static composition only;
- Unix user `ps2`;
- `/home/ps2`;
- executable `/usr/bin/X0tigervnc`;
- internal socket inactive;
- internal socket disabled;
- internal provider inactive;
- internal provider disabled.

Staging:

- refuses non-identical existing internal targets;
- installs exact tracked bytes;
- verifies installed identity with `cmp`;
- emits installed SHA-256 identities.

Verification:

- requires exact bytes and mode 0644.

Removal:

- removes only byte-identical internal targets;
- refuses unknown/non-identical targets;
- never removes or modifies direct/R11 authority.

## Static composed-unit validation

The internal stager builds a temporary systemd unit tree containing:

- R12 internal socket;
- R12 internal provider service;
- R11 direct socket;
- historical/R11 direct provider base;
- historical persistent provider;
- R11 native direct-provider drop-in.

It then invokes:

`SYSTEMD_UNIT_PATH="$TMP_CANDIDATE:" systemd-analyze verify ...`

over both alternatives.

This temporary composition proves the intended validation surface without
staging the direct alternative.

The host fixture additionally parses the tracked directives and deterministically
proves:

- exact loopback-only listener;
- exact internal service selection;
- exact R11 X0 command equality;
- exact conflict set against all three direct alternatives;
- ordering against all three direct alternatives;
- exact R11/direct Git blob identities;
- no 5903 selected path.

## R12 staging remains manager/display inert

The internal stager contains no:

- `systemctl daemon-reload`;
- `systemctl enable`;
- `systemctl disable`;
- `systemctl start`;
- `systemctl stop`;
- `systemctl restart`;
- LightDM mutation;
- Xorg mutation;
- endpoint activation.

It emits explicit nonmutation evidence including:

- `DIRECT_RFB_AUTHORITY_STAGED_BY_THIS_TOOL=NO`;
- `SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED`;
- `RFB_INTERNAL_ENABLEMENT=NOT_CHANGED`;
- `RFB_INTERNAL_RUNNING_STATE=NOT_CHANGED`;
- `LIGHTDM_XORG_STATE=NOT_CHANGED`.

No live Pi staging or activation was performed in this Reconstruction shift.

## R10 Wire Relay remains byte-identical and unattached

Compared with R12 Foreman base, these files remain exact:

- `pi/wire_server.py`
  blob `d1accd8b3b786bea4969ca8a74ecdb03c548eddc`;
- `pi/rfb_relay.py`
  blob `55e946f2cc8e7d449b83616a6fe8653695c06702`;
- `pi/wire_protocol.py`
  blob `a136640272ab942ce3e15096ff9da2089491745d`;
- `systemd/pi/ps-to-vnc-wire.service`
  blob `316c6d2eafdfdf5ecd8c89678e2dfe227e709556`.

The ordinary Wire server still has no internal-provider connector/auto-attach.

R12 adds no:

- provider socket connection from R10;
- provider retry/backoff;
- provider-failure orchestration;
- REQUEST/BOUNDARY/COMMIT/COMPLETE integration;
- Wire lifetime ownership in the provider unit;
- Application startup policy.

## Deterministic R12 host/static proof

Added:

`tests/unit/pi_internal_rfb_provider_test.py`

Git blob at final pre-log authority:

`a95fa289f7bb018ae2d3a68629db316638113ecf`

and wired it into canonical:

`make -C tests unit`

through `tests/Makefile`.

Nine focused tests prove:

1. `test_internal_socket_is_loopback_only_and_provider_local`;
2. `test_internal_service_reuses_exact_r11_x0_provider_policy`;
3. `test_internal_units_make_direct_and_internal_paths_mutually_exclusive`;
4. `test_r11_direct_and_historical_units_are_byte_identical`;
5. `test_internal_stager_is_separate_exact_and_manager_inert`;
6. `test_internal_stager_static_validation_includes_both_alternatives`;
7. `test_internal_stager_shell_syntax_is_clean`;
8. `test_r10_wire_and_relay_remain_byte_identical_and_unattached`;
9. `test_selected_internal_authority_never_uses_development_5903`.

The first implementation CI at
`6c96a1a5ec004db8bd094c6f8dca3cd20c25143f`
passed all nine R12 tests together with all eight R11 and all nineteen R10 Pi
tests.

## Documentation reconciliation

Current authority was reconciled in:

- `docs/pi/README.md`;
- `docs/pi/RFB_SOCKET_ACTIVATION.md`;
- `docs/pi/TIGERVNC_SESSION.md`;
- `docs/pi/DEPENDENCY_LEDGER.md`;
- `docs/pi/PROVISIONING.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`.

The resulting authority hierarchy is explicit:

1. selected mature provider endpoint:
   `127.0.0.1:5900` internal systemd socket;
2. selected provider:
   X0tigervnc exposing existing LightDM/Xorg `:0`;
3. preserved R11 direct native route:
   `192.168.50.1:5900` as direct-RFB fallback/provenance;
4. preserved historical physically qualified Xtigervnc `:1` authority;
5. rejected product use of development-only
   `127.0.0.1:5903`;
6. R10 provider-neutral Relay remains unattached.

Chronology was preserved rather than rewritten:

- Issue #5 direct qualified provider;
- R11 native `:0` provider selection;
- R12 internal provider endpoint selection.

A stale wording sweep corrected the remaining R11 labels that still sounded
like the direct route was the presently selected mature endpoint.

## PS2 and Wire source containment

Compared with R12 Foreman base, no path under:

- `src/`;
- `mk/`

changed.

Exact unchanged examples include:

- `src/transport/runtime.c`
  blob `6b12bed6d2983cf99f1aa26687d0d74f94880ecd`;
- `src/transport/runtime.h`
  blob `279efc5898b1abac8ff56bcc33a20b794120c051`;
- `src/app.c`
  blob `0b2738c594ad1039bff3c475d0482b48bda323b7`;
- `src/rfb/rfb_session.c`
  blob `a30cf8191b26891ad5b16571c03060c6014d46c3`;
- `mk/issue7-clean.mk`
  blob `f0be26c4a518bf871ca48cccb6db00023176f21b`.

R12 therefore creates no changed PS2 PT_LOAD input and makes no new PS2
hardware/ELF claim.

## Exact changed paths

Compared with Foreman base
`956ffd1f521f6e6654d6161d2cd34f01225b5b9e`, final pre-log authority changes
only:

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/pi/DEPENDENCY_LEDGER.md`;
- `docs/pi/PROVISIONING.md`;
- `docs/pi/README.md`;
- `docs/pi/RFB_SOCKET_ACTIVATION.md`;
- `docs/pi/TIGERVNC_SESSION.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `scripts/pi/install-rfb-internal-provider-units.sh`;
- `systemd/pi/ps-to-vnc-rfb-internal-x0tigervnc.service`;
- `systemd/pi/ps-to-vnc-rfb-internal.socket`;
- `tests/Makefile`;
- `tests/unit/pi_internal_rfb_provider_test.py`.

No other product/source/build-input file changed.

## Commit chronology

Starting Foreman authority:

`956ffd1f521f6e6654d6161d2cd34f01225b5b9e`

R12 commits:

1. `32f668a795541f69f8daa924051f97ba4a6c3132`
   — add internal RFB provider socket;
2. `abffdedf1e9f5a7a41da78f00c7561af97c75387`
   — add internal X0tigervnc provider;
3. `eb83fe3a1b2db1cd44a2e15f08bac79fa94a718d`
   — add separate inactive internal-provider stager;
4. `adb7b20c88230a2c327a07f7f903a006c1271846`
   — add focused internal-provider authority tests;
5. `6c96a1a5ec004db8bd094c6f8dca3cd20c25143f`
   — run R12 static proof in canonical host tests;
6. `240c99f09bef053a7e465bb02fbfc9f739981423`
   — select internal provider endpoint in Pi overview;
7. `cf203761d13f258f4a11b60ed29083ad87f010ca`
   — record internal endpoint lifecycle;
8. `cda4c19cc8caa392bc979f457c5d674131d77861`
   — route X0 provider through internal endpoint;
9. `3c3159032d969b0eacd6c48d1af021c2da281fdd`
   — reconcile internal-provider dependency authority;
10. `6ca7e4b52f7a5a37851c91504f5eee6f2a1e3b76`
    — provision internal provider endpoint;
11. `b636b1b5de3d2aae99d53e6bb3d4ca2d34d50c75`
    — add internal provider authority to file/service map;
12. `5f6dbd2fadbff108e3bbbf8fdd48cce01b6cc91f`
    — select internal provider endpoint in architecture overlay;
13. `8dfb3c7d353e8f9090f770c88578529a321eabe3`
    — scope R11 direct selection historically;
14. `4bab683674f29a790cc1d8a6d80f788aab2db744`
    — label R11 direct staging as preserved.

## Intermediate defects and corrections

### Preserving R11 bytes while adding live mutual exclusion

R12 requires deterministic mutual exclusion between the new internal route and
all preserved direct alternatives.

The chosen implementation places `Conflicts=` plus ordering dependencies only
in the new R12 internal units. This preserves every R11/direct unit byte-for-byte
while still making the alternatives mutually exclusive under systemd
transactions.

No historical unit was rewritten merely to make it aware of R12.

### R11/direct stager was not broadened

The R12 packet explicitly forbids turning the R11 direct-provider stager into
one inseparable installer for both alternatives.

A new separate tool was therefore created. It stages/removes only R12 internal
units and uses the direct units only as temporary static-validation inputs.

### Documentation initially retained temporal R11 "selected direct" wording

After the first documentation reconciliation, a stale-claim sweep found:

- R11 overview text saying it "reconstructs the selected direct-RFB provider";
- R11 provisioning heading saying "Selected direct-RFB provider staging".

Those labels were corrected to historical/preserved R11 wording so the living
R12 endpoint selection is unambiguous without erasing chronology.

### Exact-head project-check transient

On final pre-log commit
`4bab683674f29a790cc1d8a6d80f788aab2db744`,
the initial project-check job concluded failure, but its GitHub log artifact was
not retrievable when inspected.

The project-check job was rerun on the exact same source commit without any
repository mutation and passed all project/topology/dictionary/work-log gates.

No reproducible R12 project-check defect was identified.

### Existing scheduler-sensitive Transport host fixtures

R12 changes no `src/` or `mk/` input, but exact-head canonical host runs
repeatedly exposed known scheduler-sensitive Transport fixture timing.

Observed exact-head attempts included:

1. RFB queue publication race:
   `transport_runtime_test.c:862`;
2. a later attempt with RFB/quiesce publication assertions including
   lines 862/864 and 1155 through 1210;
3. a later attempt in which RFB runtime and AUDIO passed, but
   `transport_mpeg_test.c:997` observed the retire-completion pending flag
   before its expected publication.

The first R12 implementation run had already passed unchanged
`transport_runtime_test` and all R10/R11/R12 Python tests.

No Transport source was changed to mask these unrelated timing races.

A final exact-head host rerun on the same
`4bab683674f29a790cc1d8a6d80f788aab2db744` source passed:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- all R10 Pi tests;
- all R11 Pi tests;
- all nine R12 Pi tests.

The full retry history is retained as evidence rather than hidden.

## Final machine evidence

Final pre-log authority:

`4bab683674f29a790cc1d8a6d80f788aab2db744`

Canonical workflow:

`35662431648`

Final workflow state:

`SUCCESS` on run attempt 5.

Final jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host markers include:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- R10 `Ran 19 tests` / `OK`;
- R11 `Ran 8 tests` / `OK`;
- R12 `Ran 9 tests` / `OK`.

R12 focused markers include all nine named
`InternalProviderAuthorityTests` cases PASS.

Project/dictionary markers include:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS records=166 grandfathered=9 format_compat=2 stamp_compat=1`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

PS2 unchanged-build regression markers include:

- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These PS2 build gates are regression evidence only. R12 changed no PS2 source
or build input.

## A003 R12 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R12-C1 INTERNAL_PROVIDER_ENDPOINT_TRACKED — MET

The new internal socket/service pair is tracked under `systemd/pi/` and
documented as selected provider-side authority.

### A003-R12-C2 LOOPBACK_ONLY_127_0_0_1_5900 — MET

The internal socket has exactly
`ListenStream=127.0.0.1:5900`, with no wildcard, eth0, household/Wi-Fi,
external private-address or IPv6 selected listener.

### A003-R12-C3 NATIVE_DISPLAY_0_X0_PROVIDER_REUSED — MET

The internal service reuses the exact R11 X0 provider command/policy and the
existing LightDM/Xorg `:0` session.

### A003-R12-C4 SYSTEMD_DEMAND_ACTIVATION — MET

The internal endpoint is an ordinary `.socket` with `Accept=no` and an
explicit provider service. X0tigervnc consumes the inherited descriptor with
`-rfbport -1`.

### A003-R12-C5 NO_COMPETING_PROVIDER_LISTENER — MET

The selected service has no provider-owned TCP bind. The systemd socket is the
single selected internal listener.

### A003-R12-C6 DIRECT_AND_INTERNAL_PATHS_MUTUALLY_EXCLUSIVE — MET

Both R12 units declare conflicts and explicit ordering against the direct
socket, direct provider service and persistent control. R11 files remain
unchanged.

### A003-R12-C7 R11_DIRECT_AND_HISTORICAL_AUTHORITY_PRESERVED — MET

All four direct/R11 unit/drop-in Git blobs are byte-identical to Foreman base.

### A003-R12-C8 WINDOWS_5903_EXCLUDED — MET

No selected R12 unit/stager or R10 Wire/Relay authority uses
`127.0.0.1:5903`; docs continue to classify it as development-only.

### A003-R12-C9 FAIL_CLOSED_INACTIVE_STAGING — MET

The separate internal stager exact-stages/verifies/removes only R12 files,
refuses active/enabled or unknown-byte targets, statically verifies both
alternatives and contains no live manager/display mutation.

### A003-R12-C10 R10_RELAY_REMAINS_UNATTACHED — MET

R10 Wire server/Relay/protocol/service are byte-identical to Foreman base and
contain no internal-provider connector or auto-attach.

### A003-R12-C11 STATIC_TEST_AND_DOC_RECONCILIATION — MET

All nine R12 tests pass in canonical host CI, and current Pi lifecycle,
provisioning, dependency, file-map and architecture authority consistently
distinguish internal, direct-fallback, historical and development-only routes.

### A003-R12-C12 CLAIM_BOUNDARY_PRESERVED — MET

R12 claims repository/source/static authority only. It does not claim live Pi
activation, provider connection, RFB quiesce, physical qualification or any new
PS2 binary behavior.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_A003_R12_REQUIRED_REPOSITORY_MACHINE_GATES
SELECTED_INTERNAL_RFB_PROVIDER_ENDPOINT=127.0.0.1:5900
SELECTED_INTERNAL_RFB_PROVIDER=X0TIGERVNC_EXISTING_DISPLAY_0
INTERNAL_SOCKET_DEMAND_ACTIVATION_SOURCE_AUTHORITY=YES
DIRECT_AND_INTERNAL_MUTUAL_EXCLUSION_SOURCE_AUTHORITY=YES
R11_DIRECT_SOCKET_MUTATED=NO
R11_DIRECT_BASE_SERVICE_MUTATED=NO
R11_DIRECT_PERSISTENT_SERVICE_MUTATED=NO
R11_NATIVE_DIRECT_DROPIN_MUTATED=NO
WINDOWS_5903_PRODUCT_ROUTE=NO
R10_WIRE_SERVER_MUTATED=NO
R10_RFB_RELAY_MUTATED=NO
R10_WIRE_PROTOCOL_MUTATED=NO
R10_WIRE_SERVICE_MUTATED=NO
R10_WIRE_RELAY_PROVIDER_AUTO_ATTACH=NO
PS2_SOURCE_CHANGED=NO
PS2_BUILD_INPUT_CHANGED=NO
NEW_PS2_PT_LOAD_CLAIM=NO
LIVE_PI_R12_STAGING=NOT_RUN_NOT_CLAIMED
SYSTEMD_DAEMON_RELOAD_R12=NOT_RUN
SYSTEMD_ENABLE_DISABLE_R12=NOT_RUN
SYSTEMD_START_STOP_RESTART_R12=NOT_RUN
LIGHTDM_XORG_MUTATION_R12=NOT_RUN
INTERNAL_X0_PROVIDER_FRESH_PHYSICAL_QUALIFICATION=NOT_RUN_NOT_CLAIMED
PROVIDER_SOCKET_CONNECTION_FROM_R10=NOT_IMPLEMENTED
PROVIDER_RETRY_BACKOFF_RESTART_POLICY=NOT_IMPLEMENTED
RFB_REQUEST_BOUNDARY_COMMIT_COMPLETE_INTEGRATION=NOT_IMPLEMENTED
PROVIDER_FAILURE_TO_PS2_RFB_RETIREMENT_RESTART=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_READINESS=NOT_IMPLEMENTED
PI_AUDIO_WIRE_RELAY=NOT_IMPLEMENTED
PI_MPEG_DATA_CONTROL_PRODUCER=NOT_IMPLEMENTED
CONFIG_WIRE_DELIVERY=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_RESTORATION=NOT_IMPLEMENTED
PHYSICAL_PS2_PI_QUALIFICATION=NOT_RUN_NOT_CLAIMED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`4bab683674f29a790cc1d8a6d80f788aab2db744`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exact internal socket path/directives;
- exact loopback-only `127.0.0.1:5900`;
- internal X0tigervnc provider command and existing-display `:0` reuse;
- inherited-listener `-rfbport -1` semantics;
- no competing provider listener;
- mutual exclusion and ordering against all preserved direct alternatives;
- exact four R11/direct Git blob identities;
- no product 5903;
- separate exact internal-only stager;
- no manager/display mutation;
- static composed-unit authority;
- exact four R10 Wire/Relay Git blob identities and no auto-attach;
- zero `src/` / `mk/` change;
- final retry history and exact-head successful host/project/dictionary/build
  evidence;
- all live/hardware non-claims.

The Foreman, not this worker, chooses the next packet.

This Reconstruction worker stops after R12 and does not begin provider-to-Relay
attachment, quiesce, provider failure/restart orchestration, Application RFB
startup, AUDIO/MPEG/CONFIG work, Q7 retirement/restoration, or physical
qualification.
