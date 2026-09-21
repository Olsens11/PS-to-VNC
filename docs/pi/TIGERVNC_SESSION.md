# TigerVNC RFB Provider Authority

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=HISTORICAL_QUALIFICATION_AND_SELECTED_PROVIDER_AUTHORITY
    HISTORICAL_PROVIDER=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    HISTORICAL_EXECUTABLE=/usr/bin/Xtigervnc
    SELECTED_PROVIDER=X0tigervnc_1.15.0+dfsg-2.1~deb13u1
    SELECTED_EXECUTABLE=/usr/bin/X0tigervnc
    PROVIDER_ROLE=REPLACEABLE_IMPLEMENTATION
    HISTORICAL_LIVE_APPLICATION=PERFORMED
    HISTORICAL_HARDWARE_QUALIFICATION=PASS
    R11_NATIVE_FRESH_QUALIFICATION=NOT_RUN
    QUALIFICATION_EVIDENCE_HEAD=b40f422a760a0b7b6f2ab41699c5e72fda83bb83

This document preserves the physically qualified dedicated `Xtigervnc :1`
provider contract and records the selected A003 R11 native-desktop
`X0tigervnc :0` authority. The two claims are deliberately distinct.

The reconstruction rule remains:

> preserve the proven product contract, not the historical companion desktop or
> an accidental provider identity.

The first purpose of this provider is to supply a deterministic RFB endpoint for
the clean Raw-only 704x462 PS2 milestone. It is not yet the full user desktop,
and TigerVNC itself is not the permanent architectural boundary.

`RFB_SOCKET_ACTIVATION.md` owns the adopted lifecycle decision and the
provider-boundary interpretation.

## Selected A003 R11 native provider

Current reconstruction selection:

    existing LightDM/Xorg X11 display :0
        -> /usr/bin/X0tigervnc
        -> inherited systemd ps-to-vnc-rfb.socket
        -> 192.168.50.1:5900

The selected provider comes from:

    tigervnc-scraping-server
    1.15.0+dfsg-2.1~deb13u1

and is applied through:

    systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf

The exact recovered/tracked drop-in requires and orders after LightDM, sets
`DISPLAY=:0` and `XAUTHORITY=/home/ps2/.Xauthority`, changes stdin from the
historical inetd-style socket handoff to `null`, clears the base ExecStart and
runs:

    /usr/bin/X0tigervnc -display :0 -rfbport -1         -SecurityTypes None -AlwaysShared=1         -AcceptPointerEvents=1 -AcceptKeyEvents=1         -AcceptSetDesktopSize=0 -UseIPv6=0

`-rfbport -1` is important: X0tigervnc consumes the already inherited systemd
listener and does not create a competing product TCP endpoint.

The recovered installed drop-in was 919 bytes, mode 0644, SHA-256
`cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`.
The tracked R11 definition matches those exact bytes.

This provider exposes the existing desktop; it does not create a new fixed
704x462 X server. The 704x462/depth-16 dedicated virtual framebuffer contract
below belongs to the historical Issue #5 qualification.

The separate loopback `127.0.0.1:5903` X0tigervnc process belongs to
development viewing from Windows and is not product routing.

R11 does not activate or physically requalify this native route.

## Historical qualified dedicated provider

Everything describing `Xtigervnc :1`, 704x462/depth-16 server creation, or
`-inetd` below is retained as the Issue #5 qualified provider/control
authority. It is not the selected current reconstruction desktop source.

## Authority and reviewed parameter surface

The adopted Debian 13 package for the current provider is:

    tigervnc-standalone-server
    1.15.0+dfsg-2.1~deb13u1

`TIGERVNC_PACKAGE_AUTHORITY.md` records the reviewed trixie/arm64 package file
list. It establishes that this package supplies:

    /usr/bin/Xtigervnc

The Debian trixie `Xtigervnc(1)` man page for that exact package confirms the
parameter surface used by this provider:

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

These package facts support the provider contract; the clean-Pi evidence records
its live qualification.

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

The tracked optional control unit is:

    ps-to-vnc-rfb-tigervnc-persistent.service

This is the simple conventional lifecycle control. It does not run concurrently
with the adopted socket lifecycle.

## Socket-activated provider invocation

The adopted lifecycle separates the endpoint listener from this provider.
systemd owns `192.168.50.1:5900` through:

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

## Network/service ordering result

The `ps2-link` NetworkManager profile defines `192.168.50.1/24`. With no carrier,
the live interface had no private address. After the PS2 launch established
carrier, NetworkManager installed the address about 1.18 seconds before the sole
PS2 SYN.

The generic systemd socket used `FreeBind=yes` and owned the exact scoped listener
while the address and provider were absent. The first SYN then activated the
provider successfully. The scoped NetworkManager no-carrier candidate was
therefore rejected for the current requirement and was not installed.

NetworkManager remains the private Layer-3 identity owner; systemd owns endpoint
readiness; Xtigervnc remains the replaceable provider.

## Live validation gate

The completed read-only/live qualification records:

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

All listed gates passed on the clean Pi. The persistent-provider comparison was
conditional and was not needed because socket inheritance and lifecycle behavior
were unambiguous.

## Incremental-update stimulus

A static black root framebuffer established the initial complete Raw display.
While the same provider and PS2 session remained connected, `xsetroot` changed
display `:1` to solid red and the change appeared physically on the PS2. This
qualified the bare virtual framebuffer and ordinary update path without adding
a window manager, panel, or historical desktop stack.

This does not qualify finished desktop contents; those remain a separate layer.

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

## Qualified state

The tracked foundation, exact provider invocation, inherited-socket lifecycle,
private listener, RFB contract, reconnect behavior, controlled stop/failure
retrigger, and cold-reboot/one-launch behavior have been validated on the clean
Pi.

    TIGERVNC_DECISION=ADOPTED_RUNTIME_CURRENT_PROVIDER
    RFB_PROVIDER=TIGERVNC
    RFB_PROVIDER_REPLACEABLE=YES
    TIGERVNC_EXECUTABLE_PATH=PACKAGE_AUTHORITY
    RFB_SERVICE_MODEL=SYSTEMD_SOCKET_ACTIVATED
    TIGERVNC_PROVIDER_LIVE_VALIDATION=PASS
    DESKTOP_SESSION_CONTENTS=NOT_YET_SELECTED
