# Reconstruction shift — A003 Pi native RFB provider authority R11

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T17:37:35-04:00
COMPLETED_AT=2026-09-21T17:53:54-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f516f401d8f1404893ac01194088e28ba9ba2483
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-PI-NATIVE-RFB-PROVIDER-AUTHORITY-R11`

R11 closes the selected Pi direct-RFB provider authority/reproducibility debt
without attaching that provider to the accepted R10 Wire Relay.

The selected reconstruction route is:

    existing LightDM/Xorg X11 desktop :0
        -> /usr/bin/X0tigervnc
        -> existing systemd-owned ps-to-vnc-rfb.socket
        -> 192.168.50.1:5900

The packet preserves, rather than rewrites, the historical Issue #5
`Xtigervnc :1` hardware qualification and its tracked base/fallback units.

R11 is source/static/repository qualification only. It performs no live Pi
systemd mutation, no display mutation, no provider activation, no Relay
attachment, no physical PS2/Pi qualification, and no PS2 source change.

## Authority consumed

Truthful shift start:

`2026-09-21T17:37:35-04:00`

Live pickup authority:

`f516f401d8f1404893ac01194088e28ba9ba2483`

Foreman authority had consumed and accepted A003 R10, then assigned:

- `PACKET_ID=A003-PI-NATIVE-RFB-PROVIDER-AUTHORITY-R11`;
- `PACKET_STATUS=ACTIVE`;
- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

Current authority read included:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/PROJECT_INTENT.md;
- docs/CLEAN_ARCHITECTURE.md;
- current source naming/topology/module-lifecycle guidance;
- current Reconstruction Contract;
- current immutable work-log contract;
- current Wire Runtime Decisions;
- current Architecture Overlay;
- newest Foreman immutable log;
- current Pi RFB endpoint/provider/desktop/package/provisioning authority;
- current direct-RFB socket/base-provider/persistent-provider units;
- current R10 Wire server/Relay/protocol/systemd/stager authority;
- current RFB/TigerVNC provisioning/staging tools;
- current tests and canonical Actions workflow.

Reference-only forensic authority included the Q1-Q12
`NATIVE_PI_RFB_PATH_CLASSIFICATION_2026-09-19.md` record.

Preserved operator/machine archives from the earlier native-provider campaign
were also inspected to recover the exact installed native-provider drop-in
rather than approximating it from prose.

## Historical machine provenance recovered

The preserved native-provider campaign recorded this exact installed path:

`/etc/systemd/system/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`

Installed metadata:

- mode: `0644`;
- owner: `root:root`;
- bytes: `919`;
- SHA-256:
  `cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`.

The archived machine evidence also established:

- an existing LightDM-owned graphical session;
- Xorg display `:0`;
- session user `ps2`;
- `DISPLAY=:0`;
- `XAUTHORITY=/home/ps2/.Xauthority`;
- `/usr/bin/X0tigervnc`;
- package ownership by
  `tigervnc-scraping-server=1.15.0+dfsg-2.1~deb13u1`;
- systemd-owned PS2-facing listener on `192.168.50.1:5900`;
- X0tigervnc consuming that inherited listener;
- no second product listener from the selected X0 process.

A separate observed X0tigervnc process bound to
`127.0.0.1:5903` belonged to Windows/operator development tooling. It is
explicitly excluded from product routing.

R11 does not convert these historical observations into a claim of fresh live
qualification. It reconstructs their selected source authority exactly.

## Exact selected provider drop-in

Added maintained tracked file:

`systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`

The tracked bytes are the exact recovered 919-byte historical file.

Effective directives:

```ini
[Unit]
Requires=lightdm.service
After=lightdm.service

[Service]
Environment=DISPLAY=:0
Environment=XAUTHORITY=/home/ps2/.Xauthority
StandardInput=null
StandardOutput=journal
StandardError=journal

ExecStart=
ExecStart=/usr/bin/X0tigervnc -display :0 -rfbport -1 -SecurityTypes None -AlwaysShared=1 -AcceptPointerEvents=1 -AcceptKeyEvents=1 -AcceptSetDesktopSize=0 -UseIPv6=0
```

The complete tracked file includes explanatory comments identifying:

- the native Raspberry Pi desktop;
- existing Xorg display `:0`;
- the generic systemd RFB socket as endpoint owner;
- native systemd socket activation;
- the historical Xtigervnc `:1` inetd-style handoff distinction.

Deterministic host evidence pins:

- byte count = 919;
- SHA-256 =
  `cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`.

## Existing X11 session reuse

R11 does not create another X desktop.

The selected provider definition:

- requires `lightdm.service`;
- starts after `lightdm.service`;
- targets `DISPLAY=:0`;
- uses `XAUTHORITY=/home/ps2/.Xauthority`;
- invokes X0tigervnc against that existing display.

This is the selected current reconstruction authority.

The old dedicated `:1` Openbox/lxpanel work remains preserved as
historical/evaluating material in `docs/pi/MINIMUM_DESKTOP.md`; it is not the
selected current desktop route and was not deleted or relabeled as a failed
experiment.

## One generic PS2-facing endpoint

The generic socket remains the exact R11-base blob:

`systemd/pi/ps-to-vnc-rfb.socket`

Git blob:

`e24e4e4f389c155c7e6eff0fbd93c4c01602a361`

Required directives remain:

- `ListenStream=192.168.50.1:5900`;
- `Accept=no`;
- `BindToDevice=eth0`;
- `FreeBind=yes`;
- `Service=ps-to-vnc-rfb-tigervnc.service`.

No socket-unit byte changed in R11.

The selected X0 provider uses:

`-rfbport -1`

so it consumes the inherited listener rather than binding a second product RFB
TCP endpoint.

The selected provider drop-in contains no:

- `-rfbport 5900`;
- `-interface 192.168.50.1`;
- loopback provider bind;
- `5903` route.

## Historical dedicated provider authority preserved

The tracked base socket-activated provider remains byte-identical to R11 base:

`systemd/pi/ps-to-vnc-rfb-tigervnc.service`

Git blob:

`bb2e02bda68d40baa4b8c5152c19dbe825978038`

It retains the historically qualified dedicated model including:

- `StandardInput=socket`;
- `/usr/bin/Xtigervnc :1 -inetd ...`;
- the historical `-Log *:syslog:30` argument.

The tracked persistent fallback/control remains byte-identical:

`systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service`

Git blob:

`64874d9c5018d75e330140d3d833f5bb9ccd28d4`

It retains:

- `/usr/bin/Xtigervnc :1`;
- direct `-rfbport 5900`;
- `-interface 192.168.50.1`;
- historical logger syntax.

R11 changes selected effective provider authority only through the additive
drop-in.

The failed X0 logger variant is not promoted. The selected X0 drop-in contains
no `-Log` argument.

## Windows 5903 development route excluded

R11 current docs explicitly classify:

`127.0.0.1:5903`

as Windows/operator development tooling.

It is not:

- an internal product provider endpoint;
- an R10 Relay upstream;
- a Wire hop;
- a selected direct-RFB listener.

Static host evidence confirms no `5903` or X0 provider endpoint selection was
added to:

- `pi/wire_server.py`;
- `pi/rfb_relay.py`;
- `pi/wire_protocol.py`;
- `systemd/pi/ps-to-vnc-wire.service`.

## Exact inactive-only RFB staging

Reconstructed:

`scripts/pi/install-rfb-activation-units.sh`

The former tool performed live `systemctl daemon-reload` operations. R11
replaces that behavior with explicit:

- `stage`;
- `verify`;
- `remove`.

Tracked staged files:

1. `/etc/systemd/system/ps-to-vnc-rfb.socket`;
2. `/etc/systemd/system/ps-to-vnc-rfb-tigervnc.service`;
3. `/etc/systemd/system/ps-to-vnc-rfb-tigervnc-persistent.service`;
4. `/etc/systemd/system/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`.

All are staged/verified as mode `0644`.

The tool:

- requires tracked source files;
- requires user `ps2`;
- requires `/home/ps2`;
- requires selected `/usr/bin/X0tigervnc`;
- refuses a stale/unknown generic
  `/etc/systemd/system/ps-to-vnc-rfb.service`;
- refuses non-identical existing targets before stage;
- verifies exact source/installed bytes with `cmp`;
- verifies modes;
- refuses removal of non-identical targets;
- verifies the composed base service + drop-in using
  `systemd-analyze verify` under a temporary unit path.

Before stage/verify/remove it requires the generic socket, activated provider and
persistent control all to be inactive and disabled.

It never performs:

- `systemctl daemon-reload`;
- enable;
- disable;
- start;
- stop;
- restart;
- LightDM mutation;
- Xorg mutation.

It emits explicit nonmutation evidence including:

- `SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED`;
- `RFB_UNIT_ENABLEMENT=NOT_CHANGED`;
- `RFB_UNIT_RUNNING_STATE=NOT_CHANGED`;
- `LIGHTDM_XORG_STATE=NOT_CHANGED`.

No live staging was performed in this Reconstruction shift.

## Native provider package reproducibility

Updated:

- `scripts/pi/install-tigervnc.sh`;
- `scripts/pi/verify-foundation.sh`.

The selected native provider executable belongs to:

`tigervnc-scraping-server=1.15.0+dfsg-2.1~deb13u1`

R11 provisioning now exact-version checks both:

- `tigervnc-standalone-server=1.15.0+dfsg-2.1~deb13u1`;
- `tigervnc-scraping-server=1.15.0+dfsg-2.1~deb13u1`.

The standalone package remains because historical dedicated `:1` authority is
preserved.

The scraping package is required because the selected native provider is:

`/usr/bin/X0tigervnc`.

Provisioning fails closed on candidate or installed-version drift and performs
no silent fallback from X0tigervnc to Xtigervnc `:1`.

The foundation verifier now requires the same exact versions and both expected
provider executables.

R11 did not run APT or mutate the live Pi package state.

## R10 Wire Relay remains unattached

R11 changes none of:

- `pi/wire_server.py`;
- `pi/rfb_relay.py`;
- `pi/wire_protocol.py`;
- `systemd/pi/ps-to-vnc-wire.service`.

Their Git blobs remain byte-identical to Foreman base.

The ordinary Wire server still calls:

`self.serve_connection(connection)`

with no RFB attachment.

R11 adds no:

- provider connector;
- `--rfb-provider` option;
- X0tigervnc endpoint to Wire;
- loopback 5903 path;
- quiesce lifecycle;
- auto-attach behavior.

Provider selection authority and R10 Relay attachment remain distinct
engineering boundaries.

## Deterministic host/static proof

Added:

`tests/unit/pi_native_rfb_provider_test.py`

and wired it into canonical:

`make -C tests unit`

through `tests/Makefile`.

The eight focused tests prove:

1. exact recovered native drop-in identity:
   - 919 bytes;
   - SHA-256 `cf09...`;
2. LightDM / `:0` / XAUTHORITY / X0tigervnc selected semantics;
3. no selected competing listener:
   - `-rfbport -1`;
   - generic socket owns 5900;
   - no 5903;
4. historical base and persistent Xtigervnc `:1` definitions remain present
   and distinct from X0 selection;
5. exact inactive/manager-inert stager contract;
6. exact selected X0 package dependency and version;
7. changed shell tools pass `bash -n`;
8. R10 default Wire service remains unattached.

The stager test additionally pins the exact drop-in target composition:

- `DEST_DIR='/etc/systemd/system'`;
- `ACTIVATED_SERVICE_NAME='ps-to-vnc-rfb-tigervnc.service'`;
- `DROPIN_NAME='90-native-x0vnc.conf'`;
- `DROPIN_RELATIVE="$ACTIVATED_SERVICE_NAME.d/$DROPIN_NAME"`;
- `DROPIN_DEST="$DEST_DIR/$DROPIN_RELATIVE"`;
- exact mode 0644 stage/verify;
- fail-closed removal evidence.

## Documentation reconciliation

Updated current authority in:

- `docs/pi/README.md`;
- `docs/pi/RFB_SOCKET_ACTIVATION.md`;
- `docs/pi/TIGERVNC_SESSION.md`;
- `docs/pi/TIGERVNC_PACKAGE_AUTHORITY.md`;
- `docs/pi/MINIMUM_DESKTOP.md`;
- `docs/pi/DEPENDENCY_LEDGER.md`;
- `docs/pi/PROVISIONING.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`.

The reconciled distinction is:

- selected current reconstruction provider:
  X0tigervnc over existing LightDM/Xorg `:0`;
- historical physically qualified provider:
  dedicated Xtigervnc `:1`;
- old dedicated Openbox/lxpanel `:1` desktop candidate:
  historical/evaluating;
- loopback `5903`:
  development-only;
- R10 Wire Relay:
  still unattached;
- fresh R11 native-provider hardware qualification:
  not performed.

Historical Issue #5 qualification prose is retained and scoped historically
rather than rewritten as if R11 had performed that hardware campaign.

## PS2 / Wire product source containment

Compared with R11 Foreman base, there are no changed paths under:

- `src/`;
- `mk/`.

Exact unchanged examples:

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

Therefore R11 creates no changed PS2 PT_LOAD input and makes no PT_LOAD/hardware
claim.

## Exact changed paths

Compared with Foreman base
`f516f401d8f1404893ac01194088e28ba9ba2483`, final pre-log authority changes
only:

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/pi/DEPENDENCY_LEDGER.md`;
- `docs/pi/MINIMUM_DESKTOP.md`;
- `docs/pi/PROVISIONING.md`;
- `docs/pi/README.md`;
- `docs/pi/RFB_SOCKET_ACTIVATION.md`;
- `docs/pi/TIGERVNC_PACKAGE_AUTHORITY.md`;
- `docs/pi/TIGERVNC_SESSION.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `scripts/pi/install-rfb-activation-units.sh`;
- `scripts/pi/install-tigervnc.sh`;
- `scripts/pi/verify-foundation.sh`;
- `systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`;
- `tests/Makefile`;
- `tests/unit/pi_native_rfb_provider_test.py`.

No other product/source/build-input file changed.

## Commit chronology

Starting Foreman authority:

`f516f401d8f1404893ac01194088e28ba9ba2483`

R11 commits:

1. `4722344af90f7044e8ea7871eaf0c42cfade5822`
   — track exact native X0tigervnc provider override;
2. `95fdc2413bf475f91ce241b24c539f99aad7c700`
   — make native RFB staging inactive-only;
3. `99f09780aecf4c54de473ac2528bde0f51cb9dcd`
   — provision native TigerVNC provider package;
4. `b196f0de47e07a81a57571b9f4000f9ada8257d9`
   — verify native TigerVNC dependency;
5. `66ee1ec6214c7fc18adc250f439c921895ae1199`
   — prove native RFB provider authority;
6. `e76a0f94c2213f395eef98e7e83a1c45cc23f199`
   — run native-provider static proof in canonical host tests;
7. `ace3d9f0d1a97519d14394e7a311d6b35a552f70`
   — reconcile TigerVNC provider packages;
8. `bc5af99df4448f98e0fa87c5c81cb165e1c2f667`
   — select native desktop RFB provider in Pi authority;
9. `91b23ab0603e2396d649165a1cdca625ccb924cb`
   — reconcile native provider with RFB lifecycle;
10. `6eeb2427e9962af0fdde1433241ab0f0475060ab`
    — separate selected and historical TigerVNC provider claims;
11. `9350b9f822bc34b8cd7a222d035805dfd7eaf309`
    — mark dedicated desktop candidate historical/evaluating;
12. `3184274c66842723229f97b9ade1985c4f1f999b`
    — reconcile native provider dependencies;
13. `7ea525a0da316406fb288b741041e484b351c98b`
    — provision selected native RFB provider;
14. `202c812dc5fa2f4d3aec83542fee9f775479397d`
    — route selected native provider in file/service map;
15. `eddb1ccfcd89a8ebb20d9178ed422af94f0b2ad5`
    — record selected native provider architecture;
16. `b56c8c09b011b73a7d0b037829c21468b0118702`
    — remove stale dedicated-provider current labels;
17. `1b8c21e7ee7cef805ead9a4ee1423a97f083d42f`
    — scope Issue #5 provider language historically;
18. `73841b64012829095d4da1b8759a4833c425eccb`
    — scope Xtigervnc parameter authority historically;
19. `a3e72d27384b613573d9cfc9e3c02949e23d829e`
    — align dependency ledger with R11 selection;
20. `adcf0f1447c3a0a102a4a032b4df5b00ae3aaca6`
    — clarify historical provider qualification;
21. `a8adc4c33efca1dd1cdd31fcb882a4ad5f38d75a`
    — finish historical provider wording;
22. `0699c25f14f1946af914bacfb4bf719a86134e92`
    — format selected native provider command;
23. `c490afc926e8d33636afd0cdb8c8cc206a306762`
    — pin exact native-provider install target/removal contract.

## Intermediate failures / corrections

### Historical exact-byte recovery prevented an approximation

Initial repository/reference evidence supplied the selected drop-in path,
semantics and historical SHA, but not the complete tracked bytes on the current
branch.

Preserved machine/operator archives were recovered and contained the exact
installed file. R11 therefore promoted the exact 919-byte historical definition
rather than inventing an equivalent-looking new drop-in.

This was an authority-recovery correction, not a runtime behavior change.

### Old RFB stager performed forbidden manager mutation

The pre-R11
`scripts/pi/install-rfb-activation-units.sh` performed
`systemctl daemon-reload` during install/remove.

R11 replaced it with inactive-only exact stage/verify/remove behavior and
static composed-unit validation.

No live manager mutation was performed.

### Native provider package dependency was missing from foundation authority

Pre-R11 package provisioning pinned only
`tigervnc-standalone-server`.

Historical machine evidence identified
`/usr/bin/X0tigervnc` as owned by
`tigervnc-scraping-server=1.15.0+dfsg-2.1~deb13u1`.

Provisioning and foundation verification were corrected to require that exact
package/version.

### Documentation still called the old :1 provider "current"

After initial R11 reconciliation, a stale wording sweep found several temporal
Issue #5 uses of "current provider".

Those were scoped explicitly as historical Issue #5 qualification language so a
fresh reader cannot confuse the dedicated `:1` evidence with R11 selected
native `:0` authority.

### Pre-existing host timing flakes

Several intermediate Actions runs during R11 hit existing timing-sensitive host
fixtures in unchanged Transport source.

Observed RFB fixture failure:

- `transport_runtime_test.c:862`;
- `transport_runtime_test.c:864`;
- fake receive progress became visible before queue/activity publication.

Observed AUDIO fixture failure:

- `transport_audio_test.c:716`;
- status observation raced producer/data publication.

R11 changed no `src/` or `mk/` file.

Later identical-source runs passed these fixtures.

Final pre-log workflow `35659565917` initially hit the known RFB fixture race.
Only the failed host job was rerun on the exact same commit. Attempt 2 passed the
complete host suite, including all R10 and R11 Pi tests.

No out-of-scope Transport source was changed to mask these races.

## Final machine evidence

Final pre-log authority:

`c490afc926e8d33636afd0cdb8c8cc206a306762`

Canonical workflow:

`35659565917`

Final result:

`SUCCESS` on run attempt 2 after exact-head host-job rerun.

Jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

R11 focused host markers include:

- `test_changed_shell_tools_parse_cleanly ... ok`;
- `test_dropin_selects_existing_lightdm_xorg_display ... ok`;
- `test_exact_recovered_native_dropin_identity ... ok`;
- `test_historical_dedicated_provider_definitions_are_preserved ... ok`;
- `test_r10_default_wire_service_remains_unattached ... ok`;
- `test_selected_native_provider_creates_no_competing_listener ... ok`;
- `test_selected_x0_package_dependency_is_exact ... ok`;
- `test_stager_is_exact_inactive_and_manager_inert ... ok`;
- `Ran 8 tests`;
- `OK`.

R10 regression evidence includes all 19 Pi Wire/Relay tests PASS.

Transport regression marker:

- `transport_runtime_test: PASS`.

Project/dictionary markers:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS records=164 grandfathered=9 format_compat=2 stamp_compat=1`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

PS2 unchanged-build evidence:

- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These compile/link gates are regression evidence only. R11 changed no PS2
source/build input and therefore claims no new PS2 binary or PT_LOAD behavior.

## A003 R11 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R11-C1 NATIVE_DESKTOP_PROVIDER_SELECTED — MET

Current Pi authority selects the existing LightDM/Xorg `:0` desktop through
X0tigervnc as the direct-RFB provider architecture.

### A003-R11-C2 TRACKED_X0VNC_DROPIN — MET

The exact 919-byte historically observed
`90-native-x0vnc.conf` is tracked at the canonical provider-service drop-in
path with exact SHA pinned by host evidence.

### A003-R11-C3 EXISTING_X11_SESSION_REUSED — MET

The drop-in requires/orders after LightDM, publishes `DISPLAY=:0` and the
ps2 Xauthority, and invokes X0tigervnc against that existing display. No new
selected X desktop is created.

### A003-R11-C4 GENERIC_RFB_SOCKET_PRESERVED — MET

`ps-to-vnc-rfb.socket` is byte-identical to R11 Foreman base and remains the
sole PS2-facing `192.168.50.1:5900` endpoint owner.

### A003-R11-C5 NO_COMPETING_PROVIDER_LISTENER — MET

Selected X0 uses inherited systemd socket activation with `-rfbport -1`.
No selected provider bind to 5900/5903 or second listener is introduced.

### A003-R11-C6 FAILED_X0_VARIANT_NOT_PROMOTED — MET

The selected exact X0 drop-in contains no failed `-Log` variant. Historical
Xtigervnc logger arguments remain only in preserved `:1` definitions.

### A003-R11-C7 WINDOWS_5903_EXCLUDED — MET

Current docs classify loopback 5903 as Windows/operator development tooling.
R10 Wire/Relay/product service remain free of that route.

### A003-R11-C8 FAIL_CLOSED_INACTIVE_STAGING — MET

The RFB stager now provides exact stage/verify/remove, refuses live/enabled
units and unknown bytes, requires X0tigervnc, performs static composed-unit
validation and makes no systemd/display state change.

### A003-R11-C9 HISTORICAL_DEDICATED_DESKTOP_AUTHORITY_PRESERVED — MET

Base Xtigervnc `:1` socket-activated provider and persistent fallback remain
byte-identical. The dedicated Openbox/lxpanel candidate is preserved explicitly
as historical/evaluating evidence.

### A003-R11-C10 WIRE_RELAY_NOT_AUTO_ATTACHED — MET

All R10 Wire server/Relay/protocol/unit blobs remain byte-identical. No provider
auto-attach, connector or endpoint was added.

### A003-R11-C11 STATIC_TEST_AND_DOC_RECONCILIATION — MET

Eight focused host/static tests pass in canonical CI; current Pi package,
provider, lifecycle, desktop, provisioning, dependency, file-map and
architecture docs agree with selected R11 authority.

### A003-R11-C12 CLAIM_BOUNDARY_PRESERVED — MET

R11 source/static evidence is complete without claiming fresh Pi live state or
hardware qualification. No PS2 source/build-input changed.

## Explicit non-claims

PENDING_LOCAL=NONE_FOR_A003_R11_REQUIRED_REPOSITORY_MACHINE_GATES
SELECTED_NATIVE_RFB_PROVIDER_AUTHORITY=X0TIGERVNC_EXISTING_DISPLAY_0
TRACKED_NATIVE_DROPIN_EXACT_HISTORICAL_BYTES=YES
TRACKED_NATIVE_DROPIN_BYTES=919
TRACKED_NATIVE_DROPIN_SHA256=cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d
GENERIC_RFB_SOCKET_MUTATED=NO
HISTORICAL_XTIGERVNC_BASE_SERVICE_MUTATED=NO
HISTORICAL_XTIGERVNC_PERSISTENT_SERVICE_MUTATED=NO
WINDOWS_5903_PRODUCT_ROUTE=NO
R10_WIRE_RELAY_MUTATED=NO
R10_WIRE_RELAY_PROVIDER_AUTO_ATTACH=NO
PS2_SOURCE_CHANGED=NO
PS2_BUILD_INPUT_CHANGED=NO
NEW_PS2_PT_LOAD_CLAIM=NO
LIVE_PI_R11_STAGING=NOT_RUN_NOT_CLAIMED
SYSTEMD_DAEMON_RELOAD_R11=NOT_RUN
SYSTEMD_ENABLE_DISABLE_R11=NOT_RUN
SYSTEMD_START_STOP_RESTART_R11=NOT_RUN
LIGHTDM_XORG_MUTATION_R11=NOT_RUN
NATIVE_X0_PROVIDER_FRESH_PHYSICAL_QUALIFICATION=NOT_RUN_NOT_CLAIMED
FULL_PI_RFB_QUIESCE_LIFECYCLE=NOT_IMPLEMENTED
PROVIDER_TO_R10_RELAY_ATTACHMENT=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_RESTART_POLICY=NOT_IMPLEMENTED
PI_AUDIO_WIRE_RELAY=NOT_IMPLEMENTED
PI_MPEG_DATA_CONTROL_PRODUCER=NOT_IMPLEMENTED
CONFIG_WIRE_DELIVERY=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`c490afc926e8d33636afd0cdb8c8cc206a306762`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exact 919-byte / SHA-pinned native provider drop-in;
- selected LightDM/Xorg `:0` reuse;
- X0tigervnc native inherited-socket command;
- no competing listener;
- unchanged generic socket blob;
- unchanged historical Xtigervnc `:1` base/persistent blobs;
- failed X0 logger variant absent;
- 5903 development-only classification;
- exact inactive manager-inert stager;
- exact X0 package dependency;
- historical dedicated desktop evidence preserved;
- R10 Wire/Relay byte identity and no auto-attach;
- no `src/` or `mk/` change;
- strict project/dictionary/host/build regression evidence;
- static/source versus live/hardware claim boundary.

The Foreman, not this worker, chooses the next packet.

This Reconstruction worker stops after R11 and does not begin provider-to-Relay
attachment, full Pi RFB quiesce lifecycle, Application RFB startup, AUDIO/MPEG,
Application MPEG activation, Q7 retirement/restoration or physical
qualification.
