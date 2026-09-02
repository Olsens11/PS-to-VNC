# TigerVNC Package Authority

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=PACKAGE_SURFACE_AUTHORITY
    TARGET_OS=Debian_13_trixie_arm64
    PACKAGE=tigervnc-standalone-server
    REVIEWED_VERSION=1.15.0+dfsg-2.1~deb13u1
    LIVE_INSTALLATION=QUALIFIED_ON_CLEAN_PI
    LIVE_PACKAGE_VERSION=1.15.0+dfsg-2.1~deb13u1
    QUALIFICATION_EVIDENCE_HEAD=b40f422a760a0b7b6f2ab41699c5e72fda83bb83

This note records the reviewed Debian package surface and the later clean-Pi
qualification of that exact packaged provider. Package research remains source
authority; the hardware evidence records what the installed implementation
actually did.

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

The package supplies the current replaceable provider. It does not define the
permanent PS-to-VNC architecture or silently adopt TigerVNC's packaged
multi-user-session policy.

    TIGERVNC_PACKAGE_SURFACE=REVIEWED_AND_LIVE_VERIFIED
    TIGERVNC_DIRECT_EXECUTABLE_PATH=/usr/bin/Xtigervnc
    TIGERVNC_SERVICE_MODEL=SYSTEMD_SOCKET_ACTIVATED
    TIGERVNC_LIVE_VALIDATION=PASS
