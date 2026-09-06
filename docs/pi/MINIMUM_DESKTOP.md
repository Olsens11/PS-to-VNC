# Minimum reproducible PS2-facing Pi desktop

## Status

    WORKSTREAM=PRE_ISSUE40_PI_DESKTOP_PREREQUISITE
    CLASSIFICATION=EVALUATING
    BASE_AUTHORITY=fc5fad58404c5d9ffdaaedae3b850aed8fcbfa51
    ISSUE5_FOUNDATION=PRESERVED
    DESKTOP_WINDOW_MANAGER=OPENBOX
    DESKTOP_PANEL=LXPANEL_PI
    PHYSICAL_PI_DESKTOP=WAYLAND_LABWC_WF_PANEL_PI
    DEDICATED_UNIX_USER_REQUIRED=NO_EVIDENCE
    PANEL_PROFILE_BISECTION_REQUIRED=NO
    REAL_DISPLAY1_APPLICATION=NOT_YET_PERFORMED

## Purpose

Establish the smallest reproducible Pi desktop that is genuinely useful from
the PS2 before Issue #40 begins.

The target now includes the Raspberry Pi X11 panel rather than stopping at a
terminal-only Openbox desktop.

## Proven coexistence boundary

On 2026-09-05 the stock Raspberry Pi `lxpanel-pi` profile was tested on a
temporary `704x462`, depth-16 RGB565 Xtigervnc display while the physical
Raspberry Pi Wayland desktop remained logged in and active.

The following failed:

    same ps2 Unix user
    separate X display
    private DBus
    shared /run/user/1000

The panel exited with SIGSEGV and GVFS attempted to use the physical session's
runtime resources.

The following passed:

    same ps2 Unix user
    separate X display
    private XDG_RUNTIME_DIR
    private DBus session
    private XDG config/cache/data/state homes

`lxpanel-pi` remained alive for the full observation window and created the
expected full-width panel while physical `labwc`/`wf-panel-pi` remained alive.

Therefore the minimum architectural boundary is session isolation, not a
separate Unix account.

## Runtime model

The completed Issue #5 provider remains unchanged:

    ps-to-vnc-rfb.socket
      -> ps-to-vnc-rfb-tigervnc.service
          -> Xtigervnc :1

The new candidate layer is:

    ps-to-vnc-rfb-tigervnc.service
      -> ps-to-vnc-desktop.service
          -> private runtime directory
          -> dbus-run-session
              -> run-desktop-session.sh
                  -> Openbox
                  -> lxpanel-pi
                      -> terminal
                      -> application menu
                      -> taskbar
                      -> tray
                      -> Raspberry Pi panel plugins

The physical Raspberry Pi desktop remains separate:

    lightdm
      -> labwc / Wayland
          -> wf-panel-pi

## Panel authority

The candidate panel profile is not reconstructed by hand.

The tracked files under:

    config/pi/desktop/xdg/lxpanel-pi/

are byte-for-byte copies of the package-managed Raspberry Pi X11 panel
configuration supplied by `lxpanel-pi` and `rpd-x-core`.

`PANEL_PROVENANCE.env` records the package versions, source paths and SHA256
identities captured when the candidate was created.

## Openbox authority

The PS2-facing session uses a small project-owned Openbox configuration.

`lxpanel-pi` is the primary launcher/taskbar interface.

Desktop right-click remains a deliberately small fallback path to LXTerminal if
the panel ever becomes unavailable.

## Session isolation

The service must not share the physical desktop's:

    /run/user/1000
    DBUS_SESSION_BUS_ADDRESS
    WAYLAND_DISPLAY

Instead it receives a systemd-owned private runtime directory and runs its
entire graphical desktop beneath one private `dbus-run-session`.

Applications launched from the panel therefore inherit the PS2-facing session's
own DBus and runtime identity.

## Known non-blocking warnings

The passing isolated tests still emitted repeated GLib/GIO
`GFileInfo ... standard::is-symlink` critical messages and an AT-SPI warning.

Those messages are not classified as the cause of the earlier SIGSEGV because
they remained present during the passing panel run.

They should remain visible as follow-up diagnostics rather than being silently
suppressed.

## Promotion gate

Do not classify this candidate as adopted runtime until real `:1` qualification
proves:

1. the qualified Issue #5 RFB endpoint is unchanged;
2. systemd owns the desktop lifecycle;
3. Openbox and lxpanel-pi use the private runtime and private DBus;
4. the physical Wayland desktop remains functional concurrently;
5. the PS2 sees the panel;
6. the PS2 can launch LXTerminal from the panel;
7. PS2 keyboard input works inside that terminal;
8. application/taskbar behavior is coherent;
9. provider stop removes the dedicated desktop cleanly;
10. provider reactivation recreates the desktop without SSH intervention.

The real `:1` provider is not changed merely by preparing this candidate.

## Issue #5 installed-byte authority nuance

The live Issue #5 socket-activated provider may legitimately differ
byte-for-byte from the finalized tracked unit while remaining the exact
hardware-qualified runtime authority.

The qualification chronology is:

    b40f422a760a0b7b6f2ab41699c5e72fda83bb83
        exact provider bytes used during hardware qualification

    e909ffc97563d678e87a7d8cf9febfdd452fe7b9
        finalized Issue #5 source authority

The finalized source changed only comments describing the result of the
qualification. Its effective systemd directives and provider `ExecStart` are
unchanged.

The live provider observed during the pre-Issue40 desktop prerequisite retained
the exact `b40f422a...` hardware-qualified bytes:

    SHA256=990261225ee6b57519bd708ea70c5b771fe592e76bd77dbb064c1ec1b2fea26b

Therefore the desktop installer does not weaken the Issue #5 boundary to
arbitrary live differences.

It accepts:

1. exact current tracked bytes; or
2. for the active provider only, the exact known hardware-qualified SHA above
   when its non-comment effective systemd directives are identical to the
   finalized tracked source.

The inactive persistent comparison/control unit may also retain earlier
comment-only wording, but its effective directives must remain identical and it
must not be active or enabled.

Any effective directive difference remains a hard failure.

## Live staging checkpoint

The runtime-proven candidate was staged into its intended production filesystem
locations from repository authority:

    70e51f0f1e233fd7c5ac3b50a37e0cd4c1e68542

The staged state is:

    STAGE_STATUS=STAGED_NOT_RELOADED
    BYTE_IDENTITY=PASS
    MODE_IDENTITY=PASS
    INSTALLER_VERIFY=PASS
    SYSTEMD_DAEMON_RELOAD=NO
    DESKTOP_SERVICE_ENABLED=NO
    DESKTOP_SERVICE_STARTED=NO
    REAL_DISPLAY1_MUTATION=NO

The files now exist under `/etc/ps-to-vnc/desktop/`,
`/usr/lib/ps-to-vnc/`, and `/etc/systemd/system/`, but systemd has not been
asked to reload the new unit.

The existing qualified Xtigervnc `:1` provider and the pre-existing manually
owned Openbox process therefore remain the active real-display fixture.

The initial post-stage verifier failure was not a permission failure. The
installer requested mode strings such as `0644`, while `stat -c %a` reports
the equivalent representation `644`. Independent inspection proved all staged
modes correct. The verifier now normalizes that representation before
comparison.

Do not restage merely because of that historical false negative.

The next consequential boundary is a separately designed controlled systemd
activation transaction.
