# Clean Raspberry Pi Companion

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    BASELINE_CAPTURE=COMPLETE
    PRODUCT_MUTATIONS=TRACKED_AND_QUALIFIED
    DEPENDENCY_LEDGER=ACTIVE
    TIGERVNC_DECISION=ADOPTED_RUNTIME
    TIGERVNC_ROLE=CURRENT_REPLACEABLE_RFB_PROVIDER
    TRACKED_FOUNDATION_PROVISIONING=READY
    TIGERVNC_SESSION_CANDIDATE=QUALIFIED_CURRENT_PROVIDER
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

TigerVNC/Xtigervnc is adopted as the **current runtime provider** of the stable
PS2-facing RFB endpoint for the present milestone. That adoption does not make
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

## Dedicated TigerVNC provider

`TIGERVNC_SESSION.md` records the qualified smallest current provider for the
clean Issue #7 milestone.

The provider deliberately separates the PS2-facing RFB behavior from desktop
contents. It targets one fixed 704x462, depth-16 Xtigervnc virtual desktop on
`192.168.50.1:5900` with SecurityType None restricted to the private PS2 link.
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
- `ps-to-vnc-rfb-tigervnc.service` as the **current replaceable provider**;
- TigerVNC's documented `-inetd` wait mode as the provider-specific adapter to
  the inherited listening socket.

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
  generic socket, current provider, and persistent control, with no
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
systemd-owned RFB endpoint with packaged Xtigervnc as its current replaceable
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
