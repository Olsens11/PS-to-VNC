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
| labwc `0.9.7-1+rpt1` | `OS_BASE` | Installed and active as the virgin graphical compositor. It remains the ordinary local Pi frontend, not the PS2-facing VNC session authority. |
| WayVNC `0.9.1-1+rpt5` | `OS_BASE` | Installed in virgin image. Not adopted as the default PS2-facing server. Retain as a possible future upstream desktop source or revisit only for a concrete TigerVNC incompatibility or demonstrated product benefit. |
| Openbox `3.6.1-12+rpt1` | `OS_BASE` | Installed in virgin image; historical companion used it, but clean adoption is not automatic. |
| `wayvnc-control.service` | `OS_BASE` | Enabled in virgin image. Presence does not make WayVNC part of the PS2-facing runtime contract. |
| LightDM | `OS_BASE` | Enabled in virgin image; desktop-login mechanism, not a PS-to-VNC dependency. |
| `samba` package | `EVALUATING` | Targeted virgin `dpkg-query` reports `not-installed`; useful broader companion capability but absent from the clean base image and not required for the first VNC milestone. |
| `lxpanel` package | `REJECTED` | Not installed in the virgin image. The clean OS already supplies a native graphical frontend, so the historical LXPanel role is not carried forward by default. If a future dedicated PS2 VNC session needs its own panel/taskbar, select that deliberately rather than resurrecting LXPanel for historical similarity. |
| TigerVNC/Xtigervnc | `ADOPTED_RUNTIME` | Chosen as the stable PS2-facing RFB server. The exploratory hardware campaign already proved PS2VNC against a dedicated Xtigervnc session, and Xtigervnc naturally supplies an independent virtual framebuffer, deterministic geometry, restartable service boundary, RGB565-capable RFB endpoint, and separation from the Pi's ordinary local desktop. Change away from TigerVNC only for a specific demonstrated incompatibility or material product benefit. |
| Dedicated PS2-facing VNC desktop/session | `ADOPTED_RUNTIME` | Implement as an independently managed TigerVNC/Xtigervnc virtual desktop. Exact window-manager/session contents remain a separate decision; the PS2-facing RFB endpoint itself is now fixed to TigerVNC for the clean foundation. |
| Private PS2 Ethernet `192.168.50.1/24` on `eth0` | `EVALUATING` | Required historical/product topology, but virgin `eth0` is still unconfigured/down. First clean network mutation must be tracked and reproducible. |
| Pi management service / TCP 5959 | `EVALUATING` | Product behavior is rebuild-ready but not required for the first minimal Ethernet+RFB proof. |
| Historical VNC-only 60 Mbit/s pacing | `EVALUATING` | Historical integrated setting; not a universal requirement. Re-evaluate only if clean hardware evidence needs it. |
| Historical Openbox/LXPanel dedicated desktop stack | `REPLACED` | The old stack as a unit is not adopted. TigerVNC is retained deliberately; LXPanel is rejected; Openbox remains independently evaluable if the clean dedicated session needs a lightweight window manager. |
| WayVNC as optional upstream desktop source | `EVALUATING` | Future possibility: expose the Pi's real labwc/Wayland desktop through WayVNC and present it inside or through the stable TigerVNC-facing PS2 environment. This is not part of the first clean milestone. |
| Remote-computer VNC source through Pi | `EVALUATING` | Future architecture direction: the Pi may present another machine's VNC desktop to the PS2 while keeping TigerVNC as the stable PS2-facing compatibility/presentation endpoint. No implementation work belongs in the first clean milestone. |
| Samba/file services | `EVALUATING` | Useful broader companion feature but not a dependency of the first clean VNC milestone. |
| Docker on Pi | `EVALUATING` | Not shown as installed by the virgin package query. Build strategy should not assume it until needed and justified. |

## Decision rules

1. `OS_BASE` means only that the virgin supported image supplied the component; it does not mean PS-to-VNC depends on it.
2. Promote an `EVALUATING` item to `ADOPTED_*` only when a product need and reproducible configuration are documented.
3. A historical component may be `REPLACED` by a modern OS-native mechanism while preserving the same product contract.
4. Rejected and replaced candidates remain in this ledger with reasons.
5. Package installation, configuration changes, systemd units, persistent state, permissions, network changes, and traffic-control rules are all dependency decisions.
6. TigerVNC is the stable PS2-facing RFB contract. WayVNC or remote-machine VNC may later become upstream desktop sources without requiring the PS2-facing endpoint to change.

## Immediate implementation order

For the first clean PS2↔Pi milestone:

1. inspect the Debian 13/arm64 TigerVNC package/version/dependency surface before installation;
2. define and apply the private `eth0` link reproducibly;
3. install/configure the smallest dedicated TigerVNC/Xtigervnc session that satisfies the PS2 RFB contract;
4. choose only the minimum session/window-manager components actually required inside that virtual desktop;
5. verify RFB 3.8, SecurityType None on the private link, fixed 480p geometry, RGB565 behavior, Raw first-frame behavior, resize capability, and endpoint health/restartability;
6. leave WayVNC integration, remote-VNC routing, management, Samba, pacing, and extra desktop conveniences out until the core requires them.
