# Clean Raspberry Pi Dependency Ledger

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    LEDGER_VERSION=1
    VIRGIN_BASELINE_CAPTURED=YES
    PRODUCT_MUTATIONS=TRACKED_AND_QUALIFIED

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
| Python 3 `3.13.5-1` | `ADOPTED_RUNTIME` | Present in the virgin image and adopted by A003 R8 as the interpreter for the maintained `pi/` product Wire server. No additional package mutation is required; the tracked service uses `/usr/bin/python3`. |
| curl `8.14.1-2+deb13u3` | `OS_BASE` | Installed in virgin image. |
| git `1:2.47.3-0+deb13u1` | `OS_BASE` | Installed in virgin image; development/install-time capability, not runtime requirement by itself. |
| labwc `0.9.7-1+rpt1` | `OS_BASE` | Installed and active as the virgin graphical compositor. It remains the ordinary local Pi frontend, not automatically the PS2-facing desktop source. |
| WayVNC `0.9.1-1+rpt5` | `OS_BASE` | Installed in virgin image. Not adopted as the default PS2-facing server. Retain as a possible future desktop source/provider input or revisit for a concrete demonstrated product benefit. |
| Openbox `3.6.1-12+rpt1` | `OS_BASE` | Installed in virgin image; historical companion used it, but clean adoption is not automatic. |
| `wayvnc-control.service` | `OS_BASE` | Enabled in virgin image. Presence does not make WayVNC part of the PS2-facing runtime contract. |
| LightDM / native Xorg `:0` desktop | `ADOPTED_RUNTIME` | OS-base graphical-session authority selected by A003 R11 as the current PS2-facing desktop source. The native X0tigervnc provider explicitly Requires/After LightDM and reuses `DISPLAY=:0`; R11 adds no second X desktop. |
| `samba` package | `EVALUATING` | Targeted virgin `dpkg-query` reports `not-installed`; useful broader companion capability but absent from the clean base image and not required for the first VNC milestone. |
| `lxpanel` package | `REJECTED` | Not installed in the virgin image. The clean OS already supplies a native graphical frontend, so the historical LXPanel role is not carried forward by default. If a future dedicated PS2 VNC session needs its own panel/taskbar, select that deliberately rather than resurrecting LXPanel for historical similarity. |
| TigerVNC/Xtigervnc dedicated `:1` provider | `ADOPTED_RUNTIME` | Retained as the physically qualified Issue #5 provider/control authority. Its base socket-activated service and persistent fallback remain tracked byte-for-byte, but A003 R11 no longer selects this second virtual desktop as the current reconstruction source. |
| TigerVNC/X0tigervnc native `:0` provider | `ADOPTED_RUNTIME` | A003 R11 selected provider. `tigervnc-scraping-server=1.15.0+dfsg-2.1~deb13u1` supplies `/usr/bin/X0tigervnc`, which exposes the existing LightDM/Xorg `:0` desktop and consumes the inherited generic RFB socket with `-rfbport -1`. Selection is backed by preserved machine evidence and exact recovered drop-in bytes; fresh R11 physical qualification remains pending. |
| Predictable PS2-facing RFB endpoint/session | `ADOPTED_RUNTIME` | Durable product requirement. The PS2 needs a deterministic private RFB service with known geometry/protocol behavior and restartable endpoint health. The current implementation is an independently managed Xtigervnc virtual desktop, but the service contract rather than the provider identity is the architectural boundary. |
| Private PS2 Ethernet `192.168.50.1/24` on `eth0` | `ADOPTED_RUNTIME` | NetworkManager owns the tracked `ps2-link` profile with no gateway, `never-default`, and IPv6 disabled. Live qualification preserved the Wi-Fi management/default route and installed the private address before the PS2's first SYN after carrier. |
| systemd-owned PS2-facing RFB listening socket | `ADOPTED_RUNTIME` | Qualified standards-based lifecycle boundary. systemd owned the exact private listener before carrier/address/provider readiness, activated packaged Xtigervnc from the first PS2 SYN, preserved same-provider reconnects, retriggered after orderly stop and `SIGKILL`, and passed cold-reboot/one-launch qualification. |
| Persistent Xtigervnc provider control | `ADOPTED_OPTIONAL` | Retained as a mutually exclusive conventional fallback/control, not the selected runtime. Its conditional comparison was not required because the adopted socket path produced unambiguous first-connect and lifecycle evidence. |
| X0tigervnc loopback `127.0.0.1:5903` | `REJECTED` | Explicitly development tooling for the Windows/operator VNC path. It is not an internal PS-to-VNC provider endpoint or Relay hop and must not enter product routing. |
| Scoped NetworkManager no-carrier behavior for `eth0` | `REJECTED` | Not required for the present endpoint contract. The private address was absent without carrier but NetworkManager installed it about 1.18 seconds before the sole PS2 SYN; no override was installed. Retain the scoped candidate only as rejected evidence for reconsideration if future hardware changes the timing requirement. |
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

## Qualified foundation and next layer

The first clean PS2↔Pi foundation now has:

1. the reviewed packaged Xtigervnc provider;
2. NetworkManager ownership of the private `eth0` identity;
3. a systemd-owned PS2-facing RFB listener;
4. first-SYN demand activation with provider-specific `-inetd` details localized;
5. qualified reconnect, orderly-stop, failure-retrigger, and cold-boot behavior;
6. verified RFB 3.8, SecurityType None, fixed 704x462/depth-16 behavior, private
   listener scope, Raw presentation, and incremental framebuffer updates.

The next Pi layer is deliberate desktop/session content selection. WayVNC
integration, remote-source routing, custom gateway work, MPEG-2, audio,
management, Samba, traffic pacing, and extra desktop conveniences remain outside
this milestone until their own requirements justify them.
