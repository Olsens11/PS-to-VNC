# Clean Raspberry Pi Companion

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    BASELINE_CAPTURE=COMPLETE
    PRODUCT_MUTATIONS=NONE
    DEPENDENCY_LEDGER=ACTIVE
    TIGERVNC_DECISION=ADOPTED_RUNTIME
    TRACKED_FOUNDATION_PROVISIONING=READY
    TIGERVNC_SESSION_CANDIDATE=DEFINED
    TIGERVNC_SESSION_LIVE_VALIDATION=PENDING
    RFB_SOCKET_ACTIVATION=EVALUATING

This directory is the current authority for building the clean Raspberry Pi
companion from a normal supported Raspberry Pi OS installation.

The governing rule is:

> Every product-significant deviation from the virgin supported OS must be
> declared, reproducible, and justified.

For mechanism selection, prefer conventional Linux ownership/lifecycle first,
then adapt policy where PS-to-VNC gains a demonstrated product benefit. A useful
project-specific behavior is not rejected merely because it is not a distro
default; it should be implemented through standard mechanisms where practical.

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

TigerVNC/Xtigervnc is adopted as the stable PS2-facing RFB runtime. WayVNC stays
an OS-base capability and possible future upstream desktop source rather than the
default PS2 endpoint. LXPanel is rejected for the clean baseline; Openbox remains
a separate session-content decision.

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
foundation must be applied and verified before a systemd TigerVNC service is
promoted.

## Dedicated TigerVNC session candidate

`TIGERVNC_SESSION.md` defines the pre-live candidate for the smallest
PS2-facing RFB endpoint required by the clean Issue #7 milestone.

The candidate deliberately separates the stable RFB endpoint from desktop
contents. It targets one fixed 704x462, depth-16 Xtigervnc virtual desktop on
`192.168.50.1:5900` with SecurityType None restricted to the private PS2 link.
It does not adopt Openbox, LXPanel, management, Samba, traffic pacing, WayVNC
bridging, or remote-VNC routing.

The document also records the live-validation questions that cannot be answered
from source/package documentation alone, especially exact installed executable
behavior, systemd lifecycle, no-carrier network ordering, listener ownership,
and a deterministic incremental-update stimulus.

## RFB connection-establishment evaluation

`RFB_ACTIVATION_RESEARCH.md` records the standards research prompted by the
observed cold-boot/first-connect race. `RFB_SOCKET_ACTIVATION.md` turns that
research into one deliberately narrow live candidate.

The preferred experiment uses:

- NetworkManager for the static private `eth0` identity;
- systemd `Accept=no` socket activation for an always-ready kernel listener;
- `BindToDevice=eth0` and `FreeBind=yes` for private-interface and boot-order
  semantics;
- TigerVNC's documented `-inetd` **wait** mode so one demand-started Xtigervnc
  inherits the listening socket and remains available for reconnects.

This is still `EVALUATING`. The ordinary always-running direct Xtigervnc service
remains the control/fallback. A scoped NetworkManager `ignore-carrier` snippet is
also tracked only as a conditional candidate and must not be installed unless a
read-only baseline proves the existing static profile does not own
`192.168.50.1/24` before carrier.

Prepared apparatus and mutation staging are intentionally separate:

- `scripts/pi/inspect-rfb-activation.sh` — read-only preflight;
- `scripts/pi/capture-rfb-activation-timeline.sh` — bounded read-only timing and
  packet evidence;
- `scripts/pi/install-rfb-activation-units.sh` — fail-closed unit-file staging,
  with no enable/start/stop side effects;
- `scripts/pi/install-ps2-link-no-carrier-candidate.sh` — fail-closed conditional
  NetworkManager snippet staging, with no reload or connection-state side effect.

## Current next step

On the next hardware session, capture the untouched pre-change no-carrier state
and one current first-connect attempt before installing the socket-activation
candidate. Use that evidence to decide whether the NetworkManager no-carrier
snippet is necessary. Then validate socket ownership, one-launch demand
activation, RFB first bytes, persistent reconnect behavior, controlled service
exit/retrigger behavior, and finally cold reboot behavior.

No socket-activation or no-carrier candidate has been promoted by this branch.
