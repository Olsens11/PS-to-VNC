# Dedicated Current TigerVNC RFB Provider

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=PRE_LIVE_CURRENT_PROVIDER_DESIGN
    TIGERVNC=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    TIGERVNC_EXECUTABLE=/usr/bin/Xtigervnc
    PROVIDER_ROLE=REPLACEABLE_IMPLEMENTATION
    LIVE_APPLICATION=NOT_YET_PERFORMED
    HARDWARE_QUALIFICATION=NOT_YET_PERFORMED

This document defines the smallest clean **current TigerVNC provider** candidate
for the PS2-facing RFB service. It is a design and validation contract only.
Nothing here claims that the provider or lifecycle model has been promoted on the
clean Pi yet.

The reconstruction rule remains:

> preserve the proven product contract, not the historical companion desktop or
> an accidental provider identity.

The first purpose of this provider is to supply a deterministic RFB endpoint for
the clean Raw-only 704x462 PS2 milestone. It is not yet the full user desktop,
and TigerVNC itself is not the permanent architectural boundary.

`RFB_SOCKET_ACTIVATION.md` owns the current lifecycle evaluation and the
provider-boundary interpretation.

## Authority and reviewed parameter surface

The adopted Debian 13 package for the current provider is:

    tigervnc-standalone-server
    1.15.0+dfsg-2.1~deb13u1

`TIGERVNC_PACKAGE_AUTHORITY.md` records the reviewed trixie/arm64 package file
list. It establishes that this package supplies:

    /usr/bin/Xtigervnc

The Debian trixie `Xtigervnc(1)` man page for that exact package confirms the
parameter surface used by this candidate:

- `-geometry widthxheight` selects virtual desktop geometry;
- `-depth 16` is supported;
- the default pixel format at depth 16 is RGB565, and `-pixelformat` may make it
  explicit;
- `-interface IP-address` restricts the listening interface for direct mode;
- `-rfbport` selects the RFB TCP port independently of X display number;
- `-SecurityTypes None` is supported;
- `-localhost` can restrict connections to loopback, so a direct provider must
  explicitly permit the private Ethernet client;
- `-AcceptSetDesktopSize=0` can prevent a viewer from mutating the fixed
  server-side desktop geometry;
- `-inetd` allows the provider to consume an inherited socket for the current
  systemd socket-activation experiment.

References:

- `docs/pi/TIGERVNC_PACKAGE_AUTHORITY.md`;
- `https://packages.debian.org/trixie/arm64/tigervnc-standalone-server/filelist`;
- `https://manpages.debian.org/trixie/tigervnc-standalone-server/Xtigervnc.1.en.html`.

These package facts are source support for a candidate. They do not replace live
verification on the clean Pi.

## Current provider product contract

For the present milestone the provider must satisfy:

    implementation = /usr/bin/Xtigervnc
    X display = :1
    RFB TCP port = 5900
    private peer address = 192.168.50.1
    logical desktop = 704x462
    X depth = 16
    server pixel format candidate = RGB565
    RFB security = None
    desktop resize by clients = disabled
    lifetime = independently supervised/restartable

The architectural contract above this provider is narrower: the PS2 must see the
qualified RFB behavior. Future provider replacement should not require unrelated
PS2/network modules to know how TigerVNC implements it.

Why 704x462 rather than the historical 1280x720 session:

Issue #7 deliberately requires one exact fixed desktop first. Its clean PS2 RFB
session rejects any `ServerInit` geometry other than 704x462. The companion must
therefore meet that exact geometry during the first qualification instead of
relying on resize or mode-selection machinery that belongs to later milestones.

Why SecurityType None:

The clean Issue #7 RFB client deliberately supports only the proven
SecurityType None startup path. The current provider therefore has to offer None
for this milestone. The resulting unauthenticated RFB endpoint must be restricted
to the private PS2-facing address rather than exposed on household/Wi-Fi
interfaces. Authentication/encryption changes are a later explicit product
decision, not an implicit foundation change.

## Direct persistent-provider invocation

The conventional direct control should be equivalent to:

```sh
/usr/bin/Xtigervnc :1 \
    -geometry 704x462 \
    -depth 16 \
    -pixelformat RGB565 \
    -rfbport 5900 \
    -interface 192.168.50.1 \
    -localhost=0 \
    -SecurityTypes None \
    -AcceptSetDesktopSize=0
```

The tracked evaluation unit is:

    ps-to-vnc-rfb-tigervnc-persistent.service

This is the simple conventional lifecycle control. It does not run concurrently
with the socket-activation candidate.

## Socket-activated provider invocation

The preferred lifecycle experiment separates the endpoint listener from this
provider. systemd owns `192.168.50.1:5900` through:

    ps-to-vnc-rfb.socket

and activates:

    ps-to-vnc-rfb-tigervnc.service

The provider then uses `-inetd` wait mode and inherits the listening socket. In
this mode, systemd owns address/port exposure and the provider-specific command
must not also create a competing RFB listener.

This is useful specifically because the generic socket boundary can remain
conceptually valid if TigerVNC is later replaced. The `-inetd` details stay in
this provider adapter.

## Window-manager/session contents

For the first clean hardware proof, the endpoint/provider and desktop contents
are separate concerns.

Current state:

    LXPanel = REJECTED
    Openbox = OS_BASE / independently EVALUATING
    labwc = OS_BASE local Pi frontend
    WayVNC = OS_BASE, not current PS2-facing provider

The first TigerVNC provider should not require a window manager merely to make a
VNC endpoint exist. If later validation proves that a window manager is needed
to exercise ordinary desktop interaction, choose the smallest session content
that satisfies that need and record it separately in the dependency ledger.

Future local-Wayland, remote-computer, gateway, MPEG-2, or audio work must not be
pre-solved by importing extra desktop machinery into this provider milestone.

## Network/service ordering issue to prove live

The tracked `ps2-link` NetworkManager profile defines `192.168.50.1/24`, but the
current live no-carrier behavior must be measured rather than assumed.

A provider bound directly to `192.168.50.1` cannot be assumed to start
successfully before that address exists. The socket-activation candidate can use
`FreeBind=yes` to remove the userspace-listen ordering race, but NetworkManager
still must establish the real Layer-3 identity for ARP/IP traffic.

Therefore the first live session test must explicitly classify:

1. whether the private address exists before Ethernet carrier;
2. whether a scoped NetworkManager no-carrier setting is necessary;
3. whether the generic systemd socket can be ready while this provider is absent;
4. whether the first PS2 connection activates this provider successfully.

Do not resurrect a historical immediate-network unit automatically. Add only the
smallest declared mechanism that the clean hardware proves necessary.

## Live validation gate

The current provider is not qualified until read-only/live validation records:

- exact installed TigerVNC package version;
- `/usr/bin/Xtigervnc` belongs to the reviewed package;
- exact `Xtigervnc` version reported live;
- exact provider command line and lifecycle unit used;
- process ownership and service user;
- RFB listener is exactly private IPv4 `192.168.50.1:5900` whether owned by the
  direct provider or generic socket boundary;
- no unintended IPv6/public/wildcard listener;
- RFB server banner is compatible with the clean RFB 3.8 client;
- SecurityType None is actually offered;
- `ServerInit` geometry is exactly 704x462;
- endpoint survives client disconnect/reconnect as intended;
- systemd stop/start/retrigger behavior is deterministic;
- provider failure is visible and does not leave misleading endpoint-health
  claims;
- no window manager/panel dependency is silently introduced;
- logs identify startup failures clearly enough to diagnose
  address/bind/provider problems.

The first PS2 hardware qualification should consume this verified provider path.
It must not be the first time the provider contract itself is inspected.

## Incremental-update stimulus

A static root framebuffer is enough to prove initial RFB handshake and the first
complete Raw frame, but Issue #7 also contains an ordinary incremental
request/receive/present loop.

Use a small deterministic desktop change to prove incremental behavior. Do not
install a historical panel or full desktop merely to create motion. The prior
hardware work has already shown `xsetroot` can provide a simple visible stimulus
when available; the live test should verify the exact tool/environment before
using it as evidence.

## Explicit exclusions

This provider tranche does not add:

- Openbox adoption;
- a panel/taskbar;
- WayVNC promotion;
- remote-computer routing;
- a custom RFB proxy/gateway;
- MPEG-2 hybrid video;
- audio transport;
- Samba;
- management TCP 5959;
- traffic pacing;
- dynamic resize/multimode behavior;
- authentication redesign.

Those remain later product layers.

## Promotion rule

The current TigerVNC provider becomes qualified clean-companion runtime only
after the tracked foundation has been applied, the exact provider invocation and
selected lifecycle have been validated on the clean Pi, and the required
endpoint properties above are recorded as evidence.

Until then:

    TIGERVNC_DECISION=ADOPTED_RUNTIME_CURRENT_PROVIDER
    RFB_PROVIDER=TIGERVNC
    RFB_PROVIDER_REPLACEABLE=YES
    TIGERVNC_EXECUTABLE_PATH=PACKAGE_AUTHORITY
    RFB_SERVICE_MODEL=EVALUATING
    TIGERVNC_PROVIDER_LIVE_VALIDATION=PENDING
