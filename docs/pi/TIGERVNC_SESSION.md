# Dedicated PS2-Facing TigerVNC Session

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=PRE_LIVE_SESSION_DESIGN
    TIGERVNC=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    LIVE_APPLICATION=NOT_YET_PERFORMED
    HARDWARE_QUALIFICATION=NOT_YET_PERFORMED

This document defines the smallest clean candidate for the PS2-facing TigerVNC
runtime. It is a design and validation contract only. Nothing here claims that
the service has been installed, started, or qualified on the clean Pi yet.

The reconstruction rule remains:

> preserve the proven product contract, not the historical companion desktop.

The first purpose of this session is to provide a deterministic RFB endpoint for
the clean Raw-only 704x462 PS2 milestone. It is not yet the full user desktop.

## Authority and reviewed parameter surface

The adopted Debian 13 package is:

    tigervnc-standalone-server
    1.15.0+dfsg-2.1~deb13u1

The Debian trixie `Xtigervnc(1)` man page for that exact package confirms the
parameter surface used by this candidate:

- `-geometry widthxheight` selects virtual desktop geometry;
- `-depth 16` is supported;
- the default pixel format at depth 16 is RGB565, and `-pixelformat` may make it
  explicit;
- `-interface IP-address` restricts the listening interface;
- `-rfbport` selects the RFB TCP port independently of X display number;
- `-SecurityTypes None` is supported;
- `-localhost` can restrict connections to loopback, so the project candidate
  must explicitly permit the private Ethernet client;
- `-AcceptSetDesktopSize=0` can prevent a viewer from mutating the fixed
  server-side desktop geometry.

Reference:
`https://manpages.debian.org/trixie/tigervnc-standalone-server/Xtigervnc.1.en.html`

These package facts are source support for a candidate. They do not replace live
verification on the clean Pi.

## First-session product contract

The initial PS2-facing endpoint should satisfy exactly this narrow contract:

    implementation = Xtigervnc
    X display = :1
    RFB TCP port = 5900
    bind/interface = 192.168.50.1
    logical desktop = 704x462
    X depth = 16
    server pixel format candidate = RGB565
    RFB security = None
    desktop resize by clients = disabled
    lifetime = independently supervised/restartable

Why 704x462 rather than the historical 1280x720 session:

Issue #7 deliberately requires one exact fixed desktop first. Its clean PS2 RFB
session rejects any `ServerInit` geometry other than 704x462. The companion must
therefore meet that exact geometry during the first qualification instead of
relying on resize or mode-selection machinery that belongs to later milestones.

Why SecurityType None:

The clean Issue #7 RFB client deliberately supports only the proven
SecurityType None startup path. The server therefore has to offer None for this
milestone. The resulting unauthenticated RFB endpoint must be restricted to the
private PS2-facing address rather than exposed on household/Wi-Fi interfaces.
Authentication/encryption changes are a later explicit product decision, not an
implicit foundation change.

## Candidate Xtigervnc invocation

The first candidate should be equivalent to:

```sh
Xtigervnc :1 \
    -geometry 704x462 \
    -depth 16 \
    -pixelformat RGB565 \
    -rfbport 5900 \
    -interface 192.168.50.1 \
    -localhost=0 \
    -SecurityTypes None \
    -AcceptSetDesktopSize=0
```

This is intentionally documented as an invocation contract rather than already
committed as a systemd unit. The exact executable path, foreground behavior,
PID/lifetime behavior, logs, X authority state, and service-user environment
must be observed from the installed Debian package before a unit is promoted.

Do not add historical Openbox/LXPanel startup to this command. Xtigervnc itself
is sufficient to establish the RFB/X framebuffer endpoint. Session contents are
a separate layer.

## Window-manager/session contents

For the first clean hardware proof, the endpoint and the desktop contents are
separate concerns.

Current state:

    LXPanel = REJECTED
    Openbox = OS_BASE / independently EVALUATING
    labwc = OS_BASE local Pi frontend
    WayVNC = OS_BASE, not PS2-facing endpoint

The first TigerVNC service should not require a window manager merely to make a
VNC listener exist. If later validation proves that a window manager is needed
to exercise ordinary desktop interaction, choose the smallest session content
that satisfies that need and record it separately in the dependency ledger.

This preserves the future architecture in which the stable PS2-facing TigerVNC
framebuffer can display different upstream desktop sources without changing the
PS2 endpoint contract.

## Network/service ordering issue to prove live

The current tracked `ps2-link` NetworkManager profile defines
`192.168.50.1/24`, but the clean foundation deliberately does not force
activation without Ethernet carrier.

A service bound specifically to `192.168.50.1` cannot be assumed to start
successfully before that address exists. Therefore the first live session test
must explicitly classify startup behavior in both conditions:

1. private Ethernet carrier/address already present;
2. service/boot occurs while private Ethernet carrier is absent.

Do not resurrect the historical immediate network unit automatically. If clean
hardware shows that early/no-carrier address ownership is required for reliable
boot behavior, add the smallest declared mechanism that satisfies it and record
that as a new companion dependency decision.

## Live validation gate

After the foundation provisioning succeeds, the session is not promoted until a
read-only validation records all of the following:

- exact installed TigerVNC package version;
- exact `Xtigervnc` executable path/version;
- exact command line/service unit used;
- process ownership and service user;
- bound listener is exactly `192.168.50.1:5900`, not `0.0.0.0:5900` or a Wi-Fi
  address;
- no unintended IPv6/public listener;
- RFB server banner is compatible with the clean RFB 3.8 client;
- SecurityType None is actually offered;
- `ServerInit` geometry is exactly 704x462;
- endpoint survives client disconnect/reconnect as intended;
- systemd stop/start/restart behavior is deterministic;
- service failure is visible to systemd and does not leave a misleading stale
  listener;
- no window manager/panel dependency is silently introduced by the service;
- logs identify startup failures clearly enough to diagnose address/bind/session
  problems.

The first PS2 hardware qualification should consume this verified endpoint. It
must not be the first time the endpoint contract itself is inspected.

## Incremental-update stimulus

A static root framebuffer is enough to prove initial RFB handshake and the first
complete Raw frame, but Issue #7 also contains an ordinary incremental
request/receive/present loop.

Before hardware qualification, define one small deterministic way to produce a
known desktop change inside the TigerVNC X display. Do not install a historical
panel or full desktop merely to create motion. Prefer an already-present X tool
if the live package census proves one exists; otherwise record and adopt the
smallest justified stimulus dependency or tracked helper separately.

Until that live census is available, the stimulus mechanism remains an explicit
open validation item rather than a guessed package dependency.

## Explicit exclusions

This session tranche does not add:

- Openbox adoption;
- a panel/taskbar;
- WayVNC bridging;
- remote-computer VNC routing;
- Samba;
- management TCP 5959;
- traffic pacing;
- dynamic resize/multimode behavior;
- authentication redesign;
- a custom RFB proxy/gateway.

Those remain later product layers.

## Promotion rule

This candidate becomes clean companion authority only after the tracked
foundation has been applied, the exact runtime invocation/service has been
validated on the clean Pi, and the required endpoint properties above are
recorded as evidence.

Until then:

    TIGERVNC_DECISION=ADOPTED_RUNTIME
    TIGERVNC_SESSION_CANDIDATE=DEFINED
    TIGERVNC_SESSION_LIVE_VALIDATION=PENDING
