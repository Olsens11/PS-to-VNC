# Clean Raspberry Pi Companion

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    BASELINE_CAPTURE=COMPLETE
    PRODUCT_MUTATIONS=TRACKED_AND_QUALIFIED
    DEPENDENCY_LEDGER=ACTIVE
    TIGERVNC_DECISION=ADOPTED_RUNTIME
    TIGERVNC_ROLE=PROVIDER_FAMILY_REPLACEABLE
    TRACKED_FOUNDATION_PROVISIONING=READY
    SELECTED_RFB_PROVIDER=X0TIGERVNC_NATIVE_DISPLAY_0
    HISTORICAL_QUALIFIED_PROVIDER=XTIGERVNC_DEDICATED_DISPLAY_1
    TIGERVNC_SESSION_LIVE_VALIDATION=PASS
    RFB_SOCKET_ACTIVATION=ADOPTED_RUNTIME
    NETWORKMANAGER_NO_CARRIER_OVERRIDE=REJECTED_FOR_CURRENT_REQUIREMENT
    QUALIFICATION_EVIDENCE_HEAD=b40f422a760a0b7b6f2ab41699c5e72fda83bb83

This directory is the current authority for building the clean Raspberry Pi
companion from a normal supported Raspberry Pi OS installation.

The governing rule is:

> Every product-significant deviation from the virgin supported OS must be
> declared, reproducible, and justified.

For mechanism selection, prefer conventional Linux ownership/lifecycle first,
then adapt policy where PS-to-VNC gains a demonstrated product benefit. A useful
project-specific behavior is not rejected merely because it is not a distro
default; it should be implemented through standard mechanisms where practical.

Apply the repository's clean-reconstruction principle at each decision: preserve
the demonstrated product requirement and lesson, not accidental historical
structure. Current dependencies/providers are allowed to be concrete without
being mistaken for permanent architecture.

The old PS2VNC Pi remains forensic/reference authority. It is not an installation
template for this machine.

## Virgin baseline

`BASELINE.md` records the clean 1 TB Pi before any PS-to-VNC runtime mutation.
The captured machine is Debian 13 (trixie), arm64, on a Raspberry Pi 4 Model B
Rev 1.5. At capture, `eth0` was unconfigured for PS-to-VNC and TigerVNC was not
installed.

The baseline intentionally omits household WLAN details, machine/boot IDs, MAC
addresses, credentials, and other host-specific private values.

## Dependency states

`DEPENDENCY_LEDGER.md` uses the project states established in
`docs/PROJECT_INTENT.md`:

- `EVALUATING`
- `ADOPTED_RUNTIME`
- `ADOPTED_BUILD`
- `ADOPTED_OPTIONAL`
- `OS_BASE`
- `REJECTED`
- `REPLACED`

The reason for a state is part of the record. Rejected and replaced experiments
remain documented.

TigerVNC remains the adopted provider family, but A003 R11 distinguishes two
authorities: the hardware-qualified historical dedicated `Xtigervnc :1`
provider and the selected current reconstruction route,
`X0tigervnc -> existing LightDM/Xorg :0`. That selection does not make
TigerVNC the permanent architectural boundary. The durable requirement is a
predictable PS2-facing RFB service contract; provider-specific assumptions should
remain localized so a future gateway or different provider can replace the
implementation when real requirements justify it.

WayVNC stays an OS-base capability and possible future upstream desktop source
rather than the default PS2 endpoint. LXPanel is rejected for the clean baseline;
Openbox remains a separate session-content decision.

Future ideas such as MPEG-2 high-motion presentation, audio, and remote-computer
sources are intentionally deferred. They are relevant today only insofar as they
reinforce the need for clean replaceable service/module seams; they do not justify
building speculative gateway or multiplexing infrastructure now.

## Configuration rule

Package installation is only one kind of dependency. Record meaningful
configuration changes too, including:

- interface/address/routing changes;
- NetworkManager/systemd units;
- VNC desktop/session configuration;
- traffic-control settings;
- management-service state paths and permissions;
- desktop/window-manager/panel setup;
- Samba/file-service configuration when adopted;
- boot/sysctl/firewall changes;
- project users/groups or privilege changes.

At a reproducibility milestone, a fresh supported OS must be transformable into
the required companion state using tracked project definitions rather than
operator memory.

## Tracked foundation provisioning

`PROVISIONING.md` defines the first mutation contract. The tracked scripts are:

- `scripts/pi/install-tigervnc.sh`;
- `scripts/pi/configure-ps2-link.sh`;
- `scripts/pi/verify-foundation.sh`.

These deliberately stop before VNC service/session creation. The first clean
foundation must be applied and verified before a systemd RFB-provider service is
promoted.

## TigerVNC provider authority

`TIGERVNC_SESSION.md` preserves the qualified Issue #5 dedicated provider and
records the selected A003 R11 native-desktop provider separately.

The historical qualification targeted one fixed 704x462, depth-16 Xtigervnc
virtual desktop on `192.168.50.1:5900`. R11 does not erase that result; it
selects the later machine-proven native architecture that exposes the existing
LightDM/Xorg `:0` desktop through X0tigervnc behind the same generic socket.
It does not adopt Openbox, LXPanel, management, Samba, traffic pacing, WayVNC
bridging, or remote-VNC routing.

The live hardware record now answers the package, lifecycle, no-carrier,
listener-ownership, first-connect, reconnect, controlled-failure, and
incremental-update questions that source documentation alone could not answer.

## RFB connection-establishment evaluation

`RFB_ACTIVATION_RESEARCH.md` preserves the pre-test standards research prompted
by the cold-boot/first-connect risk. `RFB_SOCKET_ACTIVATION.md` records the
completed live evaluation and adopted lifecycle decision.

The adopted lifecycle separates:

- NetworkManager as owner of the static private `eth0` identity;
- `ps-to-vnc-rfb.socket` as the generic PS2-facing RFB endpoint boundary;
- `ps-to-vnc-rfb-tigervnc.service` as the replaceable provider service;
- the tracked `90-native-x0vnc.conf` drop-in as the selected reconstruction
  adapter, clearing the historical Xtigervnc `:1` ExecStart and using
  X0tigervnc native socket activation for existing display `:0`.

A conventional always-running provider control remains tracked as
`ps-to-vnc-rfb-tigervnc-persistent.service`. It must not run concurrently with
the adopted socket lifecycle. The control remains an optional fallback; its
conditional comparison was unnecessary because inherited-socket and provider
lifecycle behavior were unambiguous in hardware qualification.

The scoped NetworkManager `ignore-carrier` snippet is rejected for the current
requirement. The address was absent without carrier, but NetworkManager installed
it about 1.18 seconds before the PS2's sole SYN after carrier appeared. The
candidate remains tracked as rejected evidence and is not installed.

Prepared apparatus and mutation staging are intentionally separate:

- `scripts/pi/inspect-rfb-activation.sh` — read-only preflight;
- `scripts/pi/capture-rfb-activation-timeline.sh` — bounded read-only timing and
  packet evidence;
- `scripts/pi/install-rfb-activation-units.sh` — fail-closed staging of the
  generic socket, selected provider definition, and persistent control, with no
  enable/start/stop side effects;
- `scripts/pi/install-ps2-link-no-carrier-candidate.sh` — fail-closed conditional
  NetworkManager snippet staging, with no reload or connection-state side effect.

## Qualified stopping point

Issue #5 hardware qualification passed the planned socket-lifecycle gates:

- systemd owned `192.168.50.1:5900` before carrier, address assignment, or
  provider startup;
- one PS2 launch activated packaged Xtigervnc and completed the intended RFB
  contract;
- disconnect/relaunch retained the same provider and framebuffer;
- orderly stop and `SIGKILL` failure both left the socket demand-ready and the
  next launch activated a fresh provider without a `Restart=` loop or manual
  stale-display cleanup;
- cold reboot with the PS2 off restored the providerless socket, and one launch
  produced the qualified 704x462 endpoint;
- a red `xsetroot` stimulus was physically displayed through the PS2, proving
  the bare virtual framebuffer and ordinary update path.

Evidence through
`b40f422a760a0b7b6f2ab41699c5e72fda83bb83` supports adopting the generic
systemd-owned RFB endpoint with packaged Xtigervnc as its then-current replaceable
provider. The persistent unit remains an optional mutually exclusive fallback.

The qualified provider supplies a bare virtual framebuffer, not the finished
user desktop. Selecting and qualifying any window manager, panel, or other
session contents is the next separate Pi layer.

## Post-Issue #5 desktop layer

Issue #5 deliberately ended at the qualified bare RFB framebuffer/provider.

The current pre-Issue40 desktop prerequisite is documented in:

    MINIMUM_DESKTOP.md

The evaluated architecture is:

    Xtigervnc :1
      -> private XDG runtime
      -> private DBus session
      -> Openbox
      -> lxpanel-pi

The exact tracked session supervisor reproduced this architecture on an isolated
704x462 RGB565 X display while the normal Raspberry Pi Wayland desktop remained
active.

The committed candidate is now staged byte-exact in its production filesystem
locations but remains `EVALUATING`: systemd has not been daemon-reloaded and the
desktop has not yet been qualified on the real `:1` provider.


## Selected native Raspberry Pi desktop provider — A003 R11

R11 reconstructs the selected direct-RFB provider authority without attaching
it to the R10 Wire Relay.

Selected effective route:

    existing LightDM/Xorg X11 desktop :0
        -> /usr/bin/X0tigervnc
        -> inherited ps-to-vnc-rfb.socket
        -> 192.168.50.1:5900

Tracked selection:

    systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf

The tracked drop-in is byte-identical to the preserved historical machine copy:
919 bytes, mode 0644, SHA-256
`cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`.

The base `Xtigervnc :1` service and persistent fallback remain tracked as the
historical qualified/control definitions. The old Openbox/lxpanel dedicated
`:1` candidate remains historical/evaluating material rather than the
selected current desktop route.

`127.0.0.1:5903` is Windows/operator development tooling, not an internal
product provider endpoint.

The R11 stager is exact-byte, mode-aware and inactive-only. It performs no
systemd manager reload, enable/disable, start/stop/restart, or LightDM/Xorg
mutation. R11 therefore records selected source authority and historical
machine provenance only; it does not claim a fresh live native-provider
qualification.

## Product Wire server foundation — A003 R8

A003 R8 establishes the first maintained custom Pi product runtime under
`pi/`. The tracked product files are:

- `pi/wire_protocol.py` — exact PSTV framing and provisional Q4
  HELLO/ACCEPT/NOT_ACCEPTED representation;
- `pi/wire_server.py` — the persistent listener/session owner for the selected
  `192.168.50.1:5902` product Wire endpoint;
- `systemd/pi/ps-to-vnc-wire.service` — ordinary systemd supervision for that
  process;
- `scripts/pi/install-wire-runtime.sh` — exact-byte stage/verify/remove tooling.

The Wire server owns its own listener; it is **not** systemd socket activated.
TCP accept remains provisional until exact Q4 establishment succeeds. An
accepted session may remain completely idle. EOF or session-local protocol
failure retires only that connection and the persistent server returns to
listening.

This R8 source is tracked/repository-tested but **not live-qualified or
activated on the Pi**. The staging tool deliberately performs no systemd
manager reload and makes no enable/start/stop/restart change.

The existing qualified direct-RFB `192.168.50.1:5900` socket/provider
definitions remain unchanged and independent. R8 does not migrate RFB onto Wire
and implements no AUDIO, MPEG, CONFIG, heartbeat, or generic rider framework.


## Product RFB Wire Relay core — A003 R10

A003 R10 adds a maintained provider-neutral raw-RFB relay at
`pi/rfb_relay.py` and composes it only through an explicit
`WireServer.serve_connection(..., rfb_attachment=...)` seam.

The ownership boundary is strict:

- `wire_server.py` remains the only reader/writer of the PS2-facing Wire
  connection and the only owner of its global send/receive sequence;
- `rfb_relay.py` owns only the injected local provider socket and bounded
  channel-1 credit/queue state;
- provider reads stop at zero PS2-granted credit and rely on provider TCP
  backpressure rather than an unbounded staging reservoir;
- PS2->provider bytes enter only Pi-granted finite capacity;
- replacement Pi CREDIT is earned only after queued bytes actually leave through
  a nonblocking provider send;
- zero-length RFB DATA remains reserved for the existing quiesce lifecycle and
  is never forwarded as provider bytes.

R10 deliberately does **not** select a provider endpoint or mutate the current
qualified direct-RFB deployment. The ordinary installed Wire service invokes
`serve_forever()` without an RFB attachment, so its runtime behavior remains
establishment-only until a later authorized integration supplies a provider
socket explicitly.

This tranche is source/host/build evidence only. It does not claim a live Pi
provider connection or physical PS2↔Pi RFB relay qualification.
