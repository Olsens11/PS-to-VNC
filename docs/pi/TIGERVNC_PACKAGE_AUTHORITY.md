# TigerVNC Package Authority

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=PACKAGE_SURFACE_AUTHORITY
    TARGET_OS=Debian_13_trixie_arm64
    HISTORICAL_DEDICATED_PACKAGE=tigervnc-standalone-server
    SELECTED_NATIVE_PACKAGE=tigervnc-scraping-server
    REVIEWED_VERSION=1.15.0+dfsg-2.1~deb13u1
    HISTORICAL_DEDICATED_LIVE_QUALIFICATION=PASS
    SELECTED_NATIVE_MACHINE_EVIDENCE=PRESERVED_NOT_FRESHLY_REQUALIFIED
    QUALIFICATION_EVIDENCE_HEAD=b40f422a760a0b7b6f2ab41699c5e72fda83bb83

This note records both TigerVNC package surfaces now required by Pi provider
authority. The dedicated `Xtigervnc :1` route was hardware-qualified during
Issue #5. A later preserved machine campaign demonstrated the selected native
desktop route through `X0tigervnc :0`; A003 R11 makes that package/executable
reproducible without claiming a fresh live qualification.

Package research and tracked provisioning are source authority. Historical
machine evidence records what the installed implementations actually did.

## Debian package paths

The Debian trixie arm64 package file list for
`tigervnc-standalone-server` records these product-significant files:

    /usr/bin/Xtigervnc
    /usr/bin/tigervncserver
    /usr/lib/systemd/system/tigervncserver@.service
    /usr/libexec/tigervncsession-start
    /usr/sbin/tigervncsession
    /etc/X11/Xtigervnc-session
    /etc/tigervnc/vncserver.users

Source authority:

    https://packages.debian.org/trixie/arm64/tigervnc-standalone-server/filelist

Therefore the clean direct-server candidate in `TIGERVNC_SESSION.md` can use the
package-owned executable path:

    /usr/bin/Xtigervnc

rather than leaving executable-path discovery as an unresolved design question.
Live qualification confirmed that `/usr/bin/Xtigervnc` belongs to the installed
reviewed package and version.

## Native desktop provider package

The selected A003 R11 provider is supplied by the exact companion package:

    tigervnc-scraping-server
    1.15.0+dfsg-2.1~deb13u1

Preserved machine evidence from the same Debian 13 / arm64 Pi recorded:

    /usr/bin/X0tigervnc
    owner package = tigervnc-scraping-server
    package version = 1.15.0+dfsg-2.1~deb13u1

The exact selected native-provider systemd drop-in is tracked at:

    systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf

Its recovered historical installed identity is:

    installed path = /etc/systemd/system/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf
    mode = 0644
    bytes = 919
    SHA256 = cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d

That definition selects the already-running LightDM/Xorg display `:0` and
invokes `/usr/bin/X0tigervnc` with native systemd socket activation
(`-rfbport -1`). It does not create another X desktop or another product TCP
listener.

The separate `127.0.0.1:5903` X0tigervnc listener observed in development
history is Windows/operator tooling and is explicitly not a PS-to-VNC product
dependency or internal hop.

R11 provisioning therefore requires both exact-version packages:

- `tigervnc-standalone-server`, retained for the historical qualified
  dedicated-`:1` definition/control;
- `tigervnc-scraping-server`, required by the selected native-`:0`
  reconstruction route.

## Packaged service does not imply adoption

The same package ships `tigervncserver@.service` plus the `tigervncsession`
wrapper/session machinery. Their presence is evidence about the package surface,
not authority to adopt the packaged multi-user VNC-session model for PS-to-VNC.

The current clean product contract is narrower: an independently restartable
PS2-facing Xtigervnc endpoint at a fixed private address and geometry. Hardware
qualification selected small project-owned systemd socket/provider units that
invoke `/usr/bin/Xtigervnc` directly, without importing the packaged multi-user
session policy.

Do not copy the historical Pi's systemd units to answer this question.

## Reviewed command-line surface

The Debian trixie `Xtigervnc(1)` manual for the adopted package supports the
parameters used by the current provider:

- `-geometry` for the virtual desktop dimensions;
- `-depth 16`;
- RGB565 as the normal 16-bit pixel format, with `-pixelformat` available to make
  the requested format explicit;
- `-interface` for listener-address restriction;
- `-rfbport` for the RFB TCP port;
- `-SecurityTypes None`;
- `-AcceptSetDesktopSize=0` to reject viewer-requested desktop resizing.

Source authority:

    https://manpages.debian.org/trixie/tigervnc-standalone-server/Xtigervnc.1.en.html

These facts support the provider contract. The live qualification described
below verifies the actual RFB behavior, bind scope, lifecycle, and failure
visibility.

## Live qualification result

The 2026-09-02 clean-Pi campaign confirmed:

- `tigervnc-standalone-server=1.15.0+dfsg-2.1~deb13u1`;
- package ownership of `/usr/bin/Xtigervnc`;
- execution as user/group `ps2` through the tracked provider unit;
- inherited-listener operation through documented `-inetd` mode;
- exact private IPv4 listener ownership on `192.168.50.1:5900`/`eth0`;
- RFB 3.8 and SecurityType None negotiation with the clean PS2 client;
- server RGB565/depth-16 and client BGR555/16-bit negotiation;
- fixed 704x462 virtual-framebuffer operation;
- same-provider reconnect and fresh-provider retrigger behavior;
- visible systemd failure reporting after `SIGKILL`;
- automatic replacement of stale X display artifacts on the next demand;
- a deterministic `xsetroot` color change physically displayed through the PS2.

The standalone package supplied the historical qualified replaceable provider.
The selected reconstruction provider now uses the scraping package against the
existing native desktop. Neither package defines the permanent PS-to-VNC
architecture.

    HISTORICAL_XTIGERVNC_PACKAGE_SURFACE=REVIEWED_AND_LIVE_VERIFIED
    HISTORICAL_XTIGERVNC_EXECUTABLE=/usr/bin/Xtigervnc
    SELECTED_X0TIGERVNC_EXECUTABLE=/usr/bin/X0tigervnc
    SELECTED_X0TIGERVNC_PACKAGE=tigervnc-scraping-server
    TIGERVNC_SERVICE_MODEL=SYSTEMD_SOCKET_ACTIVATED
    HISTORICAL_XTIGERVNC_LIVE_VALIDATION=PASS
    R11_NATIVE_PROVIDER_FRESH_LIVE_VALIDATION=NOT_RUN
