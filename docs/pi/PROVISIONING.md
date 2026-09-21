# Clean Pi Foundation Provisioning

## Scope

This is the first tracked mutation contract for GitHub Issue #5. It deliberately
covers only the foundation required before the dedicated PS2-facing VNC session
is defined:

1. install the adopted TigerVNC runtime package;
2. define the private PS2 Ethernet profile;
3. verify both states.

It does **not** yet create or start the PS2-facing VNC service, choose the final
window-manager/session contents, configure Samba, install the management service,
apply traffic pacing, or import old Pi configuration.

## Adopted TigerVNC package

The clean Debian 13 / arm64 pre-mutation census found:

    tigervnc-standalone-server=1.15.0+dfsg-2.1~deb13u1

`scripts/pi/install-tigervnc.sh` fails closed if the current APT candidate differs
from that reviewed version. A changed repository candidate is a dependency-ledger
review event, not permission to silently install a newer package.

The script intentionally uses the normal Debian dependency/recommendation graph.
The pre-mutation simulation predicted five new packages and no unrelated upgrade:

- `tigervnc-standalone-server`;
- `tigervnc-common`;
- `tigervnc-tools`;
- `libfile-readbackwards-perl`;
- `xfonts-base`.

Exact installed versions are emitted after application and should be recorded in
the dependency ledger/evidence before this foundation is promoted.

## Private PS2 Ethernet contract

`scripts/pi/configure-ps2-link.sh` owns only the project profile:

    profile=ps2-link
    device=eth0
    ipv4=192.168.50.1/24
    ipv4.method=manual
    ipv4.never-default=yes
    ipv6.method=disabled
    autoconnect=yes
    autoconnect-priority=100

The script does not delete or rewrite the virgin image's `netplan-eth0` profile.
It defines the project profile but does not force activation when there is no
Ethernet carrier. The private link must never steal the Pi's default route.

## Application order

From an exact clean checkout of `pi/issue5-clean-baseline`:

```sh
sudo ./scripts/pi/install-tigervnc.sh
sudo ./scripts/pi/configure-ps2-link.sh
./scripts/pi/verify-foundation.sh
```

The two mutation scripts require root explicitly rather than embedding `sudo`.
This keeps privilege acquisition outside project logic and makes the scripts
usable from an interactive shell, installer, or future bootstrap.

## Idempotence and drift

- rerunning the TigerVNC installer is a no-op only when the exact adopted version
  is already installed;
- a different installed or candidate TigerVNC version fails closed;
- rerunning the network script accepts an already-correct `ps2-link` profile;
- an existing `ps2-link` with different product-significant values fails closed
  rather than being silently rewritten;
- the verifier is read-only.

## Next layer

After this foundation passes on the clean Pi, define the dedicated TigerVNC
session/service separately. That layer will prove fixed 480p geometry, the
PS2-facing RFB/security contract, RGB565 behavior, endpoint health/restartability,
and the minimum session/window-manager contents actually required.


## Product Wire runtime candidate — A003 R8

The maintained Pi product Wire runtime is sourced from `pi/` and staged with:

    sudo ./scripts/pi/install-wire-runtime.sh stage
    sudo ./scripts/pi/install-wire-runtime.sh verify

Tracked installation targets are:

    /usr/lib/ps-to-vnc/wire_protocol.py
    /usr/lib/ps-to-vnc/wire_server.py
    /etc/systemd/system/ps-to-vnc-wire.service

The current selected listener value is `192.168.50.1:5902`. The service runs
as the existing `ps2` user using the OS-base `/usr/bin/python3`.

The stager is intentionally inactive-only. It copies exact tracked bytes,
refuses non-identical pre-existing targets, verifies installed identity/modes,
and statically validates the installed unit. It does **not** reload systemd,
change enablement, start/stop/restart the Wire service, or touch the qualified
direct-RFB socket/provider definitions.

Removal likewise requires the candidate service to be inactive and disabled and
removes only byte-identical tracked targets.

R8 therefore establishes reproducible installation definitions without claiming
live listener, no-carrier, repeated-session, or physical PS2/Pi qualification.
