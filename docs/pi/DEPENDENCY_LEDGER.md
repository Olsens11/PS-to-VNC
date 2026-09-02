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
| labwc `0.9.7-1+rpt1` | `OS_BASE` | Installed and active as the virgin graphical compositor. It remains the ordinary local Pi frontend, not automatically the PS2-facing desktop source. |
| WayVNC `0.9.1-1+rpt5` | `OS_BASE` | Installed in virgin image. Not adopted as the default PS2-facing server. Retain as a possible future desktop source/provider input or revisit for a concrete demonstrated product benefit. |
| Openbox `3.6.1-12+rpt1` | `OS_BASE` | Installed in virgin image; historical companion used it, but clean adoption is not automatic. |
| `wayvnc-control.service` | `OS_BASE` | Enabled in virgin image. Presence does not make WayVNC part of the PS2-facing runtime contract. |
| LightDM | `OS_BASE` | Enabled in virgin image; desktop-login mechanism, not a PS-to-VNC dependency. |
| `samba` package | `EVALUATING` | Targeted virgin `dpkg-query` reports `not-installed`; useful broader companion capability but absent from the clean base image and not required for the first VNC milestone. |
| `lxpanel` package | `REJECTED` | Not installed in the virgin image. The clean OS already supplies a native graphical frontend, so the historical LXPanel role is not carried forward by default. If a future dedicated PS2 VNC session needs its own panel/taskbar, select that deliberately rather than resurrecting LXPanel for historical similarity. |
| TigerVNC/Xtigervnc | `ADOPTED_RUNTIME` | Adopted as the **current RFB provider** for the clean foundation because the exploratory hardware campaign proved the PS2 client against it and it supplies a deterministic independent virtual framebuffer, fixed geometry, RGB565-capable RFB behavior, and ordinary Linux service integration. This is a concrete runtime dependency for the current milestone, not a permanent architecture requirement. Provider-specific assumptions must remain localized so a future PS-to-VNC gateway or another provider can replace it for a demonstrated product benefit. |
| Predictable PS2-facing RFB endpoint/session | `ADOPTED_RUNTIME` | Durable product requirement. The PS2 needs a deterministic private RFB service with known geometry/protocol behavior and restartable endpoint health. The current implementation is an independently managed Xtigervnc virtual desktop, but the service contract rather than the provider identity is the architectural boundary. |
| Private PS2 Ethernet `192.168.50.1/24` on `eth0` | `EVALUATING` | Required historical/product topology, but virgin `eth0` is still unconfigured/down. First clean network mutation must be tracked and reproducible. |
| systemd-owned PS2-facing RFB listening socket | `EVALUATING` | Standards-first demand-activation candidate. Could keep the RFB endpoint ready independently of the current provider process and directly address the legacy one-shot first-connect race. Live comparison against a persistent provider is required before promotion. |
| Scoped NetworkManager no-carrier behavior for `eth0` | `EVALUATING` | Conditional candidate only. Current NetworkManager may already activate a static Layer-3 profile without carrier; measure first and add a per-device override only if the real Pi proves it necessary. |
| Pi management service / TCP 5959 | `EVALUATING` | Product behavior is rebuild-ready but not required for the first minimal Ethernet+RFB proof. |
| Historical VNC-only 60 Mbit/s pacing | `EVALUATING` | Historical integrated setting; not a universal requirement. Re-evaluate only if clean hardware evidence needs it. |
| Historical Openbox/LXPanel dedicated desktop stack | `REPLACED` | The old stack as a unit is not adopted. TigerVNC is retained deliberately for the current provider; LXPanel is rejected; Openbox remains independently evaluable if the clean dedicated session needs a lightweight window manager. |
| WayVNC as optional desktop source/provider component | `EVALUATING` | Future possibility informed by the preserved experiment. It may expose the Pi's real labwc/Wayland desktop directly or feed a future gateway. It is not part of the first clean milestone and does not redefine the PS2-side RFB contract today. |
| Remote-computer desktop source through Pi | `EVALUATING` | Future architecture direction: the Pi may normalize another machine's desktop for the PS2. Do not pre-commit to TigerVNC, WayVNC, a proxy, or a custom gateway as that future implementation until the requirements become active. |
| Hybrid MPEG-2 high-motion video path | `EVALUATING` | Deferred future performance direction already acknowledged by the clean architecture. Its relevance today is only to preserve replaceable transport/provider seams; no media protocol or gateway implementation belongs in the first clean milestone. |
| Audio transport | `EVALUATING` | Deferred product direction. Do not reserve protocol structure or ports yet; preserve modular network/session ownership so it can be designed from real requirements later. |
| Samba/file services | `EVALUATING` | Useful broader companion feature but not a dependency of the first clean VNC milestone. |
| Docker on Pi | `EVALUATING` | Not shown as installed by the virgin package query. Build strategy should not assume it until needed and justified. |

## Decision rules

1. `OS_BASE` means only that the virgin supported image supplied the component; it does not mean PS-to-VNC depends on it.
2. Promote an `EVALUATING` item to `ADOPTED_*` only when a product need and reproducible configuration are documented.
3. A historical component may be `REPLACED` by a modern OS-native mechanism while preserving the same product contract.
4. Rejected and replaced candidates remain in this ledger with reasons.
5. Package installation, configuration changes, systemd units, persistent state, permissions, network changes, and traffic-control rules are all dependency decisions.
6. The durable PS2-facing contract is predictable RFB service behavior, **not TigerVNC identity**. TigerVNC is the current adopted provider and may be replaced later only through a deliberate, evidence-backed decision.
7. Future media/audio/remote-source ideas justify keeping seams clean but do not justify speculative gateway, multiplexing, or provider-selection infrastructure in the current milestone.
8. Prefer conventional platform machinery first; adapt policy only when PS-to-VNC gains a demonstrated benefit, and qualify the adaptation against the simpler conventional control.

## Immediate implementation order

For the first clean PS2↔Pi milestone:

1. inspect the Debian 13/arm64 TigerVNC package/version/dependency surface before installation;
2. define and apply the private `eth0` link reproducibly;
3. establish a predictable PS2-facing RFB endpoint using the current TigerVNC provider while keeping provider-specific assumptions localized;
4. compare the conventional persistent-provider lifecycle with the systemd socket-activation candidate using real first-connect evidence;
5. choose only the minimum session/window-manager components actually required inside the current virtual desktop;
6. verify RFB 3.8, SecurityType None on the private link, fixed 480p geometry, RGB565 behavior, Raw first-frame behavior, resize capability, and endpoint health/restartability;
7. leave WayVNC integration, remote-source routing, custom gateway work, MPEG-2, audio, management, Samba, pacing, and extra desktop conveniences out until their own milestones require them.
