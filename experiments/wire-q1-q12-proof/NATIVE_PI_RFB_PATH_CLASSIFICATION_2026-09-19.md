# Native Pi RFB path classification — 2026-09-19

## Scope

This record closes the bounded read-only question needed by the
`wire/q1-q12-hardware-proof` phase:

> What provider/path is currently configured to expose the normal Raspberry Pi
> desktop to the PS2-facing RFB endpoint, and how does it attach to the existing
> graphical session?

It does **not** qualify the complete Pi reproducibility package, does not replace
Issue #5 qualification evidence, and does not authorize the historical
Openbox/lxpanel `:1` candidate.

## Observation identity

Interactive return:

- returned at: `2026-09-19T23:13:38.0350945-04:00`
- SSH exit: `0`
- input SHA-256:
  `023cc3f26caecae41663463f9a7f688bbe907d30bed7b08e2d49ebaa07c83171`
- mutation performed: **NO**
- daemon reload: **NO**
- unit-state change: **NO**
- display-state change: **NO**

The observation was taken on host `ps-to-vnc-dev`.

## Machine-observed facts

### Existing graphical session

`loginctl` reported session `1` as:

- user `ps2`;
- `seat0`;
- `Display=:0`;
- `Type=x11`;
- `Class=user`;
- `State=active`;
- `Remote=no`.

The live display server was:

`/usr/lib/xorg/Xorg :0 -seat seat0 ...`

LightDM and its session child were live. No live `Xwayland`,
`Xtigervnc`, `x0vncserver`, or `w0vncserver` process was observed.

### PS2-facing endpoint ownership

`ps-to-vnc-rfb.socket` was loaded, enabled, active and listening at:

`192.168.50.1:5900`

The socket has:

- `Accept=no`;
- `BindToDevice=eth0`;
- `Service=ps-to-vnc-rfb-tigervnc.service`.

The listener itself was owned by systemd PID 1.

### Effective provider definition

The effective provider service was static and inactive/dead at the observation
instant, with `MainPID=0`, while the activation socket remained listening.

Its live drop-in was:

`/etc/systemd/system/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf`

Observed SHA-256:

`cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d`

The drop-in overrides the historical dedicated-`:1` provider and establishes:

- `Requires=lightdm.service`;
- `After=lightdm.service`;
- `DISPLAY=:0`;
- `XAUTHORITY=/home/ps2/.Xauthority`;
- `StandardInput=null`;
- effective `ExecStart=/usr/bin/X0tigervnc -display :0 -rfbport -1 ...`.

The drop-in explicitly documents that `X0tigervnc` consumes the inherited
systemd listener through native socket activation rather than creating another
desktop.

Therefore the effective configured product route is:

```text
existing LightDM/Xorg X11 desktop :0
    -> socket-activated X0tigervnc provider
    -> systemd-owned 192.168.50.1:5900 PS2-facing endpoint
```

This run classified the effective configuration. Because no PS2 connection was
opened during this read-only observation, it does not by itself claim a fresh
end-to-end activation/recovery qualification.

## Operator-observed classification

A separate live process was observed as:

`X0tigervnc -display :0 -rfbport 5903 -interface 127.0.0.1 ...`

The operator identified `127.0.0.1:5903` as the VNC endpoint used through an
SSH tunnel by the Windows machine for development viewing/access.

Accordingly:

`WINDOWS_5903_PATH=DEVELOPMENT_TOOLING_NOT_PRODUCT`

That endpoint must not be modeled as an internal PS-to-VNC product hop.

## Architecture consequence

The normal Raspberry Pi desktop remains the desktop authority. The product does
not need a second Openbox/lxpanel `:1` session merely to provide the RFB source.

This closes the provider-classification side question for the current hardware
proof phase. The broader pre-Issue40 Pi package remains separately blocked on
reproducible installation/lifecycle qualification.

The interactive hardware-proof effort now returns to the unfinished MPEG
generation and MPEG/RFB lifecycle questions.
