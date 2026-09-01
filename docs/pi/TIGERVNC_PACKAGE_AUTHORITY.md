# TigerVNC Package Authority

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    ROLE=PACKAGE_SURFACE_AUTHORITY
    TARGET_OS=Debian_13_trixie_arm64
    PACKAGE=tigervnc-standalone-server
    REVIEWED_VERSION=1.15.0+dfsg-2.1~deb13u1
    LIVE_INSTALLATION=NOT_YET_PERFORMED

This note records facts that can be established from the adopted Debian package
without mutating the clean Raspberry Pi. It narrows the unknowns for the first
live provisioning session; it does not claim that any package has been installed
or any service has been started on that machine.

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
The live verifier must still confirm that the installed filesystem matches the
reviewed package and version after provisioning.

## Packaged service does not imply adoption

The same package ships `tigervncserver@.service` plus the `tigervncsession`
wrapper/session machinery. Their presence is evidence about the package surface,
not authority to adopt the packaged multi-user VNC-session model for PS-to-VNC.

The current clean product contract is narrower: an independently restartable
PS2-facing Xtigervnc endpoint at a fixed private address and geometry. Whether a
small project-owned unit should invoke `/usr/bin/Xtigervnc` directly, or whether
the packaged session machinery can satisfy the same contract without importing
unwanted policy, remains a live-validation/design decision.

Do not copy the historical Pi's systemd units to answer this question.

## Reviewed command-line surface

The Debian trixie `Xtigervnc(1)` manual for the adopted package supports the
parameters used by the current candidate:

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

These facts support the candidate contract but still require live verification
of the actual RFB banner, security advertisement, ServerInit geometry, bind
scope, restart behavior, and failure visibility.

## What remains genuinely live

The following should not be guessed from package metadata:

- the exact post-install package/file identity on the clean Pi;
- direct Xtigervnc foreground/exit behavior under the intended service model;
- appropriate service user and X authority/runtime directory requirements;
- address-not-present/no-carrier startup behavior when binding 192.168.50.1;
- listener scope including any IPv6 side effects;
- actual RFB 3.8/SecurityType None/704x462 behavior observed on the wire;
- deterministic disconnect/reconnect and systemd restart behavior;
- the minimum desktop-change stimulus needed for Issue #7 incremental-update
  qualification.

Until those are observed:

    TIGERVNC_PACKAGE_SURFACE=REVIEWED
    TIGERVNC_DIRECT_EXECUTABLE_PATH=/usr/bin/Xtigervnc
    TIGERVNC_SERVICE_MODEL=EVALUATING
    TIGERVNC_LIVE_VALIDATION=PENDING
