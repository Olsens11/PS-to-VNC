# Issue #7 experimental RFB provider-handoff apparatus

This directory preserves the exact 480p provider-handoff arrangement that was
proved on hardware with the experimental explicit-RFB-reconnect build.

Classification:

    EXPERIMENTAL=YES
    FORMAL_ISSUE7_QUALIFICATION=NO
    PS2_RECONNECT_SOURCE_COMMIT=56e1d00bea70cfa8424a5c4104bebbd27990f709
    WAYLAND_GEOMETRY=704x462
    STOCK_WAYLAND_GEOMETRY=1920x1080
    PS2_FACING_ENDPOINT=192.168.50.1:5900
    W0_BACKEND_ENDPOINT=127.0.0.1:5901
    W0_VERSION=1.16.2
    W0_SHA256=76dd46e40a3b1d82da8a86b3bff622b2f6c9c703c2d1ed38dee81c660d0ef804

## What this reproduces

The Raspberry Pi's normal labwc/Wayland desktop already exists independently of
this apparatus. Setup does not install or configure a desktop session, LightDM,
or boot-time display policy. Raspberry Pi OS stock labwc launches kanshi; this
experiment does not add or modify any kanshi configuration.

The experiment adds only temporary runtime state:

1. `wlr-randr` forces labwc's `NOOP-1` output to the currently supported
   PS-to-VNC geometry, `704x462`.
2. The preserved TigerVNC 1.16.2 `w0vncserver` runs as a transient user service
   on `127.0.0.1:5901`.
3. A temporary system service runs `systemd-socket-proxyd 127.0.0.1:5901`.
4. A temporary `/run` drop-in can retarget the durable
   `ps-to-vnc-rfb.socket` from canonical Xtigervnc to that proxy.
5. Restarting the durable socket deliberately breaks the current RFB session;
   the experimental PS2 ELF reconnects to the same `192.168.50.1:5900`
   endpoint and negotiates a fresh RFB session with the newly selected provider.

The proved hardware sequence was:

    Wayland/w0 -> Xtigervnc -> Wayland/w0

with a new PS2 TCP connection on every transition and no ELF relaunch.

## Operator sequence

From this directory:

    ./setup-wayland-w0-apparatus.sh
    ./switch-rfb-to-wayland.sh
    ./switch-rfb-to-xtiger.sh
    ./switch-rfb-to-wayland.sh
    ./teardown-wayland-w0-apparatus.sh

`setup-wayland-w0-apparatus.sh` prepares Wayland/w0 but deliberately leaves the
active PS2-facing provider on canonical Xtigervnc.

The two switch scripts require a live PS2 RFB session because their purpose is
to prove a live provider handoff and a fresh reconnect.

`switch-rfb-to-xtiger.sh` leaves the Wayland/w0 backend prepared so repeated
bidirectional switching is cheap.

`teardown-wayland-w0-apparatus.sh` restores the canonical Xtigervnc socket
target, removes the temporary proxy unit and socket drop-in, stops the transient
w0 backend, and restores the stock headless Wayland output observed after a
clean LightDM autologin: `NOOP-1` enabled at `1920x1080`, position `0,0`, scale
`1`. It intentionally leaves only the installed known-good
`~/.local/lib/ps-to-vnc/w0vncserver` binary behind.

## Code orientation

`SYMBOLS.md` is the plain-language lookup for the maintained shell code in this
directory. When a function or variable such as `front_connection`,
`SOURCE_BEFORE`, `DROPIN`, or `BACKEND_LOAD` is unclear while reading a script,
look it up there for what it represents, why the script needs it, and what part
of the provider-handoff experiment it controls.

The dictionary describes the code; it does not replace this README's operator
sequence or turn this experimental apparatus into formal Issue #7 qualification.

## Preserved w0 binary

The known-good binary remains installed at:

    ~/.local/lib/ps-to-vnc/w0vncserver

Setup requires its exact SHA256 before doing anything. Teardown deliberately
leaves it there, so later experiments do not have to rebuild or reinstall it.

The exact binary and its source-build provenance are already preserved on:

    archive/w0vncserver-1.16.2-experiment

If the installed file is ever lost, restore the hash-verified binary from that
archive before running setup. These scripts do not reintroduce the older direct
`:5900` bind, the older experiment's kanshi configuration/helper, or any
persistent helper.

## Known systemd handoff artifact

During the original live provider switches, `systemctl restart
ps-to-vnc-rfb.socket` could briefly attempt to start the provider that had just
been deselected without an inherited socket. The old w0 proxy reported
`Didn't get any sockets passed in` in one direction, and Xtigervnc reported
`Got no socket` in the other. The newly selected provider then accepted the new
PS2 connection correctly. The switch scripts clear stale failed state after the
successful handoff; this artifact is apparatus behavior, not a PS2 reconnect
failure.

## Scope limits

This apparatus proves explicit TCP/RFB I/O-loss reconnect and provider
replacement. It does not implement or test silent-stall recovery, malformed-RFB
recovery, or an automatic stall watchdog.
