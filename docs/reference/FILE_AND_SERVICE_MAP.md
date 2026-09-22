# PS-to-VNC File and Service Map

This file is the canonical living location map for the successor project.

Update it whenever canonical code ownership or file location changes.

## Repositories

| Role | Location | Mutable by PS-to-VNC work? |
|---|---|---|
| Historical PS2VNC authority | `/home/ps2/ps2vnc` when present | No |
| Current PS-to-VNC development | Git repository `Olsens11/PS-to-VNC`; selected worktree path is not authority | Yes |

## Historical backup

Legacy Git bundle:

`/home/ps2/ps2vnc-legacy-backups/ps2vnc-legacy-allrefs-20260830-002434.bundle`

SHA256:

`7f018a3623c5cdd7e7ed30bfbc3f00040b632735fc83f92d49c5a0c672bea85a`

## Baseline

| Responsibility | Current location | Owner |
|---|---|---|
| Frozen B4A implementation | `baseline/frozen-b4a/` | immutable historical reference |
| Frozen B4A source | `baseline/frozen-b4a/ps2ip.c` | immutable historical reference |
| Dependency analysis | `baseline/analysis/B4A-dependency-map/` | immutable analysis reference |
| Baseline identity | `baseline/IDENTITY.env` | project provenance |

## Current clean reconstruction source — adopted 2026-09-05

Current maintained clean product source is rooted at:

    src/
    pi/

`src/` is the PS2 executable tree. `pi/` is the custom Raspberry Pi companion
runtime tree adopted by A003 R8; provisioning remains under `scripts/pi/`.

Current responsibility locations are:

| Responsibility | Current location |
|---|---|
| Raspberry Pi product Wire protocol/server and provider-neutral RFB Relay runtime | `pi/wire_protocol.py`, `pi/wire_server.py`, `pi/rfb_relay.py` |
| Executable entry / application coordination | `src/main.c`, `src/app.c`, `src/app.h` |
| Session CONFIG/profile decoding, validation, owner-specific immutable values, and config-text helpers | `src/config/` |
| PCM playback, resident AUDSRV adapter, and session-scoped audio worker/resource/reservoir lifecycle | `src/audio/` |
| Session common-media epoch, signed/saturating deadlines, synchronization contract, and host-testable wait boundary | `src/media/` |
| Controller/input/keyboard/mouse | `src/input/` |
| Local foreground / OSK / local presentation | `src/ui/` |
| RFB wire/session parsing and logical-stream adaptation | `src/rfb/` |
| PSTV Q4 establishment/availability, physical stream, sole I/O owner, logical channels, flow control, and Transport lifecycle | `src/transport/` |
| Authoritative desktop framebuffer | `src/framebuffer/` |
| Platform-neutral display conversion | `src/display/` |
| Diagnostics/runtime identity | `src/diagnostics/` |
| PS2-specific system/network/graphics mechanisms | `src/platform/` |
| Local product symbol authorities | each active directory's `SYMBOLS.md` |
| Generated dictionary portal | `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` |
| Source-placement policy | `docs/development/source-topology.md` |
| Topology adoption history | `docs/reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md` plus later tranche-owned living-map/topology updates |

Only `main.c`, `app.c`, and `app.h` are current clean C/H source directly in
`src/`. Feature families belong in their owning responsibility directories.

`src/config/` was deliberately adopted as clean product source during the A002
configuration/profile tranche on 2026-09-16. `profile.{c,h}` owns the bounded,
versioned, side-effect-free production CONFIG value boundary; `text.{c,h}` is
retained behavior-identically as clean local configuration utility code. This
ownership does not include live CONFIG negotiation, PCM/AUDSRV runtime,
media-clock wait/arming runtime, or MPEG/video behavior.

`src/audio/` was deliberately adopted during the A002 PCM playback-core tranche
on 2026-09-16. `playback.{c,h}` owns the synchronous, host-testable PCM consumer
of the public Transport AUDIO seam; `audsrv_service.{c,h}` owns the concrete
resident AUDSRV adapter; and `session.{c,h}` owns the session-scoped worker,
explicit stack/buffer authority, non-consuming startup-reservoir observation,
common-clock audio-deadline gate, stop/join fence, and post-join reclamation.
This domain still does not own top-level application Transport abort/close
orchestration, epoch arming, MPEG/video presentation, or hardware qualification.

`src/media/` was deliberately created during the A002 common-media-clock
tranche on 2026-09-16. `clock.{c,h}` owns one reusable session timing boundary:
one common epoch, signed/saturating audio and neutral-video deadlines, required
synchronization semantics, and injected timer/delay/stop observers. It does not
own PCM/AUDSRV runtime, MPEG/video presentation policy, or concrete PS2
lock/timer bindings.

`src/video/` still contains retained pre-refresh source. Its presence remains
historical/reference state and does not make it a current clean domain until a
later clean reconstruction deliberately adopts or replaces it.

## Historical bootstrap working-source snapshot

> **Historical snapshot:** the wording below records repository bootstrap state
> before M0. It is intentionally retained and is not current source authority.

Current working source:

**NONE — M0 has not started.**

No file under `baseline/frozen-b4a/` is a writable modular-development
location.

## Documentation

| Responsibility | Location |
|---|---|
| Documentation entry point | `docs/README.md` |
| Exhaustive compatibility document inventory | `docs/INDEX.md` |
| Current project state | `docs/PROJECT_STATE.md` |
| Modularization ledger | `docs/MODULARIZATION.md` |
| Architecture | `docs/architecture/ARCHITECTURE.md` |
| Current file/service map | `docs/reference/FILE_AND_SERVICE_MAP.md` |
| Roadmap | `docs/ROADMAP.md` |
| Legacy relationship | `docs/LEGACY_HANDOFF.md` |
| Project identity | `docs/PROJECT_IDENTITY.md` |

## Raspberry Pi companion and runtime services — Issue #5 authority carried forward 2026-09-05

The current reconstruction now carries the finalized clean Issue #5 Pi
foundation from source authority
`e909ffc97563d678e87a7d8cf9febfdd452fe7b9`.

Hardware qualification for the adopted RFB lifecycle is preserved at evidence
head `b40f422a760a0b7b6f2ab41699c5e72fda83bb83`.

| Responsibility | Current tracked location |
|---|---|
| Pi companion authority/router | `docs/pi/README.md` |
| Maintained Pi product runtime source | `pi/` |
| Product Wire server | `pi/wire_server.py`, `pi/wire_protocol.py` |
| Product RFB Relay / session attachment | `pi/rfb_relay.py`, `pi/rfb_attachment.py` |
| Canonical selected RFB runtime profile | `src/config/rfb_runtime_profile.json` |
| Deterministic RFB profile generator / drift check | `scripts/generate-rfb-runtime-profile.py` |
| PS2 RFB profile projection | `src/config/rfb_runtime_profile.{c,h}`, generated `src/config/rfb_runtime_profile_generated.h` |
| Pi RFB profile projection | `pi/rfb_runtime_profile.py`, generated `pi/rfb_runtime_profile_generated.py` |
| Product Wire systemd unit candidate | `systemd/pi/ps-to-vnc-wire.service` |
| Product Wire inactive stager/verifier | `scripts/pi/install-wire-runtime.sh` |
| Clean Pi provisioning | `docs/pi/PROVISIONING.md`, `scripts/pi/` |
| PS2 private-link provisioning | `scripts/pi/configure-ps2-link.sh` |
| Preserved direct PS2-facing RFB listener | `systemd/pi/ps-to-vnc-rfb.socket` |
| Historical qualified RFB provider base | `systemd/pi/ps-to-vnc-rfb-tigervnc.service` |
| Preserved R11 direct native-desktop provider override | `systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf` |
| Optional mutually exclusive direct provider control | `systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service` |
| Direct RFB authority stager | `scripts/pi/install-rfb-activation-units.sh` |
| Selected Pi-local RFB provider socket | `systemd/pi/ps-to-vnc-rfb-internal.socket` |
| Selected Pi-local X0tigervnc provider service | `systemd/pi/ps-to-vnc-rfb-internal-x0tigervnc.service` |
| Internal RFB provider stager | `scripts/pi/install-rfb-internal-provider-units.sh` |
| Qualified lifecycle record | `docs/pi/RFB_SOCKET_ACTIVATION.md` |
| Qualified provider/session record | `docs/pi/TIGERVNC_SESSION.md` |

The selected provider-side reconstruction lifecycle is now the R12 internal
systemd socket on `127.0.0.1:5900`, which demand-activates X0tigervnc against
the existing LightDM/Xorg `:0` desktop. The R11 direct
`192.168.50.1:5900` socket/drop-in, base Xtigervnc `:1` service and
persistent provider remain preserved fallback / historical qualification
authority. R12 itself did not attach the R10 Relay or make a live/hardware
claim. A003 R13 now supplies the session-scoped, first-CREDIT-lazy attachment
mechanism to that internal endpoint while preserving the default installed Wire
service as establishment-only. R13 adds no live Pi activation or physical
qualification claim.

A003 R14 places the one current selected RFB value authority in
`src/config/rfb_runtime_profile.json`. Generated C/Python constants are strict
projections, while the hand-written PS2 and Pi seams project into the existing
Transport and R13 flow types. Semantic OFF is an absent composition, not a
zero-filled running configuration. Neither default Wire service nor
`pstvnc_app_run()` consumes this profile yet. The CP2N baseline provenance is
preserved; the new Pi provider-write-capacity projection remains hardware
pending.

The tracked NetworkManager no-carrier candidate under `config/pi/` is preserved
as rejected Issue #5 evidence and is not part of the selected installed
configuration.

Issue #5 deliberately qualified only the bare RFB framebuffer/provider layer.
The minimum Openbox/terminal session being established before Issue #40 is new
subsequent work and must remain distinguishable from the completed Issue #5
qualification.

## Migration recovery authority

| Responsibility | Canonical location |
|---|---|
| Recovery entry point | `START_HERE.md` |
| Machine-readable migration state | `runtime/MIGRATION_STATE.env` |
| Human-readable migration state | `docs/MIGRATION_STATE.md` |
| Recovery summary command | `scripts/resume-state.sh` |
| Current project coherence gate | `scripts/check.sh` |
| Documentation coherence gate | `scripts/docs-check.sh` |

These locations move in lockstep with implementation authority.

No modularization stage is complete until this map and the migration state
agree with the actual source tree.

## M0 build provenance authority

| Responsibility | Canonical location |
|---|---|
| M0 reproduction contract | `docs/M0_BASELINE_CONTRACT.md` |
| Historical build provenance | `docs/M0_BUILD_PROVENANCE.md` |
| Machine-readable M0 build authority | `runtime/M0_BUILD_AUTHORITY.env` |
| Frozen B4A implementation | `baseline/frozen-b4a/` |

The preserved legacy build material is historical evidence only and is
not PS-to-VNC build authority.

## M0D writable authority

| Responsibility | Canonical location |
|---|---|
| Writable B4A application source | `working/b4a/` |
| Writable primary source | `working/b4a/ps2ip.c` |
| Controlled M0 build wrapper | `scripts/build-m0-b4a.sh` |
| M0 build provenance | `docs/M0_BUILD_PROVENANCE.md` |
| M0 build fingerprints | `runtime/M0_BUILD_AUTHORITY.env` |

`working/b4a/` begins byte-identical to the frozen B4A application inputs.

Generated objects, generated IRX C files, the copied build dependency, and the
ELF are build products rather than source authority.

## M0D2 reproduction evidence

| Responsibility | Canonical location |
|---|---|
| M0 reproduction result | `docs/M0_BUILD_RESULT.md` |
| M0D2 captured evidence | `evidence/m0/m0d2/` |
| Machine build-result authority | `runtime/M0_BUILD_AUTHORITY.env` |

The generated `working/b4a/PS2VNC.ELF` remains a build product.

The committed frozen B4A ELF remains the durable binary authority
because the M0D2 result proved the generated ELF byte-identical to it.

## M0 hardware-validation resolution

| Responsibility | Canonical location |
|---|---|
| M0 hardware-validation decision | `docs/M0_HARDWARE_RESOLUTION.md` |
| M0 reproduction result | `docs/M0_BUILD_RESULT.md` |
| Machine M0 reproduction authority | `runtime/M0_BUILD_AUTHORITY.env` |

The M0 hardware decision inherits historical B4A evidence only because the
reproduced final executable is byte-identical. It is not a new hardware PASS.

## M1 extraction contract

| Responsibility | Canonical location |
|---|---|
| M1 governing contract | `docs/M1_EXTRACTION_CONTRACT.md` |
| M1 selection evidence | `baseline/analysis/B4A-dependency-map/` |
| M1 entry working source | `working/b4a/ps2ip.c` |
| M0 completed build authority | `runtime/M0_BUILD_AUTHORITY.env` |

The exact M1 module target is deliberately selected in M1A before source
mutation.

## M1A extraction boundary

| Responsibility | Canonical location |
|---|---|
| Selected first extraction | `docs/M1A_EXTRACTION_BOUNDARY.md` |
| M1 governing contract | `docs/M1_EXTRACTION_CONTRACT.md` |
| M1 entry monolith | `working/b4a/ps2ip.c` |
| Dependency evidence | `baseline/analysis/B4A-dependency-map/` |

The selected implementation has not yet moved.

M1B remains blocked by the GitHub publication checkpoint.

## GitHub publication authority

| Responsibility | Canonical location |
|---|---|
| GitHub publication record | `docs/GITHUB_PUBLICATION.md` |
| Git remote | `origin` |
| GitHub repository | `Olsens11/PS-to-VNC` |
| Visibility | `PRIVATE` |
| Default branch | `main` |

The successor remote is distinct from legacy `Olsens11/PS2VNC`.

## M1B config-text module

| Responsibility | Canonical location |
|---|---|
| Configuration text whitespace implementation | `working/b4a/ps2vnc_config_text.c` |
| Configuration text whitespace public API | `working/b4a/ps2vnc_config_text.h` |
| Remaining transitional monolith | `working/b4a/ps2ip.c` |
| M1B result | `docs/M1B_EXTRACTION_RESULT.md` |
| Machine M1 source authority | `runtime/M1_SOURCE_AUTHORITY.env` |

The `working/b4a/` directory name remains transitional lineage naming. It no
longer means every writable source file is byte-identical to B4A.

## M1C modular DUT authority

| Responsibility | Canonical location |
|---|---|
| M1 executable machine authority | `runtime/M1_DUT_AUTHORITY.env` |
| M1 source machine authority | `runtime/M1_SOURCE_AUTHORITY.env` |
| M1C characterization | `docs/M1C_DUT_CHARACTERIZATION.md` |
| Durable first modular ELF | `evidence/m1/m1c1/PS2VNC-M1C1.ELF` |
| M1C1 build evidence | `evidence/m1/m1c1/` |
| M1C2 characterization evidence | `evidence/m1/m1c2/` |
| Current operational build product | `working/b4a/PS2VNC.ELF` |

The operational ELF is generated/ignored. The committed evidence ELF is the
durable binary authority.

## Pre-Issue40 minimum desktop candidate

This layer is subsequent to and distinct from the completed Issue #5 RFB
qualification.

| Responsibility | Tracked location |
|---|---|
| Candidate contract / qualification gate | `docs/pi/MINIMUM_DESKTOP.md` |
| Panel package provenance | `config/pi/desktop/PANEL_PROVENANCE.env` |
| Project-owned Openbox configuration | `config/pi/desktop/openbox/rc.xml` |
| Project-owned Openbox recovery menu | `config/pi/desktop/openbox/menu.xml` |
| Stock Raspberry Pi lxpanel-pi profile | `config/pi/desktop/xdg/lxpanel-pi/` |
| Desktop contents supervisor | `scripts/pi/run-desktop-session.sh` |
| Fail-closed stage/verify/remove tool | `scripts/pi/install-minimum-desktop.sh` |
| Desktop systemd ownership candidate | `systemd/pi/ps-to-vnc-desktop.service` |

The runtime-proven coexistence boundary is:

    same ps2 Unix user
    + independent X display
    + private XDG_RUNTIME_DIR
    + private DBus
    = Openbox + stock lxpanel-pi coexist with physical Wayland desktop

The exact tracked supervisor reproduced this boundary on an isolated 704x462
RGB565 Xtigervnc display.

The candidate files are staged byte-exact in the live filesystem. The systemd
manager has not been daemon-reloaded; the desktop service is not enabled or
started and has not yet been hardware-qualified on real `:1`.
