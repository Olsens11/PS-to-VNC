# Clean Raspberry Pi Dependency Ledger

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    LEDGER_VERSION=1
    VIRGIN_BASELINE_CAPTURED=YES
    PRODUCT_MUTATIONS=NONE

This ledger records product-significant clean-Pi dependency decisions. Historical presence is not adoption.

Allowed states:

- `EVALUATING`
- `ADOPTED_RUNTIME`
- `ADOPTED_BUILD`
- `ADOPTED_OPTIONAL`
- `OS_BASE`
- `REJECTED`
- `REPLACED`

## Initial classifications

| Dependency / capability | State | Evidence / reason |
|---|---|---|
| Debian GNU/Linux 13 (trixie), arm64 | `OS_BASE` | Virgin clean-card OS identity. |
| Linux `6.18.34+rpt-rpi-v8` | `OS_BASE` | Virgin kernel identity. |
| NetworkManager `1.52.1-1+rpt4` | `OS_BASE` | Installed in virgin image; suitable candidate for private `eth0` configuration. |
| SSH service | `OS_BASE` | Enabled in virgin image and used only as ordinary administration/development access. |
| Avahi/mDNS | `OS_BASE` | Enabled in virgin image; useful for `ps-to-vnc-dev.local`, not a PS2 runtime dependency. |
| Python 3 `3.13.5-1` | `OS_BASE` | Installed in virgin image; may support Pi-side management tooling if adopted. |
| curl `8.14.1-2+deb13u3` | `OS_BASE` | Installed in virgin image. |
| git `1:2.47.3-0+deb13u1` | `OS_BASE` | Installed in virgin image; development/install-time capability, not runtime requirement by itself. |
| labwc `0.9.7-1+rpt1` | `OS_BASE` | Installed and active as the virgin graphical compositor. Whether the dedicated PS2 desktop should depend on it remains undecided. |
| WayVNC `0.9.1-1+rpt5` | `OS_BASE` | Installed in virgin image; candidate VNC implementation, not yet adopted for PS2 runtime. |
| Openbox `3.6.1-12+rpt1` | `OS_BASE` | Installed in virgin image; historical companion used it, but clean adoption is not automatic. |
| `wayvnc-control.service` | `OS_BASE` | Enabled in virgin image. Presence does not prove a dedicated usable PS2-facing VNC endpoint. |
| LightDM | `OS_BASE` | Enabled in virgin image; desktop-login mechanism, not yet a PS-to-VNC dependency. |
| `samba` package | `EVALUATING` | Targeted virgin `dpkg-query` reports `not-installed`; useful broader companion capability but absent from the clean base image and not required for the first VNC milestone. |
| `lxpanel` package | `EVALUATING` | Targeted virgin `dpkg-query` reports `not-installed`; historical panel behavior is not automatically adopted. |
| TigerVNC/Xtigervnc | `EVALUATING` | `tigervnc-standalone-server` is not installed in the virgin clean image. Historical proven companion used a dedicated Xtigervnc `:1`; compare against OS-base WayVNC before adoption. |
| Dedicated PS2-facing VNC desktop/session | `EVALUATING` | Required product behavior; implementation choice still open. Must be predictable, independently health-checkable, and reachable from the private PS2 link. |
| Private PS2 Ethernet `192.168.50.1/24` on `eth0` | `EVALUATING` | Required historical/product topology, but virgin `eth0` is still unconfigured/down. First clean network mutation must be tracked and reproducible. |
| Pi management service / TCP 5959 | `EVALUATING` | Product behavior is rebuild-ready but not required for the first minimal Ethernet+RFB proof. |
| Historical VNC-only 60 Mbit/s pacing | `EVALUATING` | Historical integrated setting; not a universal requirement. Re-evaluate only if clean hardware evidence needs it. |
| Historical Openbox/LXPanel dedicated desktop stack | `EVALUATING` | Proven historical implementation; compare against the clean OS's native labwc/WayVNC possibilities. |
| Samba/file services | `EVALUATING` | Useful broader companion feature but not a dependency of the first clean VNC milestone. |
| Docker on Pi | `EVALUATING` | Not shown as installed by the virgin package query. Build strategy should not assume it until needed and justified. |

## Decision rules

1. `OS_BASE` means only that the virgin supported image supplied the component; it does not mean PS-to-VNC depends on it.
2. Promote an `EVALUATING` item to `ADOPTED_*` only when a product need and reproducible configuration are documented.
3. A historical component may be `REPLACED` by a modern OS-native mechanism while preserving the same product contract.
4. Rejected and replaced candidates remain in this ledger with reasons.
5. Package installation, configuration changes, systemd units, persistent state, permissions, network changes, and traffic-control rules are all dependency decisions.

## Immediate evaluation order

For the first clean PS2↔Pi milestone:

1. evaluate/configure the private `eth0` link reproducibly;
2. compare virgin WayVNC/labwc capability with the historical dedicated TigerVNC contract;
3. adopt the smallest VNC runtime that can supply the required RFB behavior to the PS2;
4. leave management, Samba, pacing, and desktop conveniences out until the core requires them.
