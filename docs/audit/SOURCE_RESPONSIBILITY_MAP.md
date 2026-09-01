# PS-to-VNC Historical Source-to-Responsibility Map

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_3
    INPUT_BEHAVIOR_FAMILIES=B01_B14
    MATURITY=EVIDENCE_SUPPORTED
    ROLE=SOURCE_STATE_COUPLING_MAP

This document maps the frozen B4A/current exploratory implementation to the
product responsibilities established by the semantic audit.

It is **not** a recommendation to preserve the historical file layout.

Primary current audit authority remains:

- `docs/audit/BEHAVIORAL_INVENTORY.md`;
- `docs/audit/CROSS_DOMAIN_SYNTHESIS.md`;
- `docs/audit/CROSS_DOMAIN_STATE_INTERFACES.md`.

---

## 1. Responsibility map

| Behavior | Historical implementation roots | Important historical state/coupling | Clean ownership consequence |
|---|---|---|---|
| B01 startup/lifecycle | `working/b4a/ps2vnc_display_tail.inc` (`main()` orchestration); `ps2vnc_runtime_tail.inc` exit publication; `ps2vnc_calibration_core.inc` startup display reconciliation | startup ordering is interleaved with display/RFB globals; `exit_requested`; force-calibration startup sample; display transaction state | one application coordinator owns lifecycle/order; platform/display/RFB/input expose mechanisms |
| B02 PS2/Pi Ethernet/networking | B4A runtime startup/network helpers; frozen `baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a`; preserved patch/provenance | static private-link addresses; NetMan/link status; same PS2IP stack serves RFB, management, diagnostics | small platform-network mechanism; higher protocols remain separate consumers |
| B03 RFB session/transport | `ps2vnc_runtime_tail.inc` handshake; `ps2vnc_runtime_core.inc` `recv_exact`, RX buffer, queue/socket state; management/display paths for replacement | socket, `rfb_rx_*`, `rfb_connected`, negotiated state, ExtendedDesktopSize state, historical encoded outbound queue | one RFB session owns exact stream state and replacement; product policy remains application-owned |
| B04 framebuffer/update decode | `ps2vnc_framebuffer_core.inc`; runtime dirty-region arrays and logical geometry | parser/decode operates against framebuffer globals; dirty row/bounds; `live_linear_framebuffer_stale`; RFB/GS storage may alias | explicit desktop-framebuffer owner between protocol and presentation; decoder never owns GS |
| B05 GS/presentation | `working/b4a/ps2vnc_gsHires.c`; `ps2vnc_calibration_tail.inc` presentation helpers; `ps2vnc_display_core.inc` reconstruction | active backend/mode globals; HIRES buffers/mapping; GS interrupt state; presentation validity | display/presentation owner plus recognizable PS2 graphics mechanism; preserve Test14 interrupt contract |
| B06 display modes/transactions/calibration | `src/video/mode.c`; B4A display core/tail; calibration core/tail; UI confirmation/menu paths; Pi management transaction server | mode/backend, logical geometry, output rectangle, safe area, confirmation deadline, provisional/restore state distributed across globals and machines | first-class `display_profile`; application owns risky transaction policy; display owns live profile/mechanism; Pi owns durable transaction record |
| B07 controller/pointer/actions | controller thread and hotkey/input logic primarily in B4A UI/runtime regions; F8J2 pad handoff/transition paths | physical pad state, pointer acceleration, hotkey state, pause/ack globals, historical encoded RFB event production | input owns physical/derived state and semantic events; app routes effects; libpad handoff remains explicit |
| B08 keyboard/OSK | B4A UI/runtime keyboard and OSK logic; RFB KeyEvent serializers | OSK visibility/page/row/column and modifiers historically shared with controller/render code | UI owns OSK interaction state; RFB owns wire key encoding; configured actions remain typed |
| B09 local UI | `ps2vnc_ui_core.inc`, `ps2vnc_ui_tail.inc`, display/system menu and confirmation paths | menu/overlay globals shared across controller and render; local redraw historically coupled to RFB wait/pointer-jiggle mechanisms | main-owned UI foreground model; input quarantine API; explicit local dirty/wake path |
| B10 configuration/persistence | `src/config/text.c/.h`; B4A config/calibration/management parsing and save helpers; Pi `management-server.py` | desired config, live display globals, policy cache and durable transaction state historically adjacent because all use TCP 5959 | one typed side-effect-free configuration model; persistence adapters separate; durable transaction authority separate |
| B11 recovery/management | `ps2vnc_runtime_core.inc` wait/interruption; RFB replacement paths; management core/tail; display rollback paths | Refresh/exit/rollback globals observed inside receive loop; same HTTP transport carries config, durable safety, and best-effort test control | application owns recovery policy/control intent; RFB owns replacement mechanism; management owns wire operations only |
| B12 diagnostics/identity | `src/diagnostics/identity.c`, `debug.c`, `report.c`; M4I hardware/test apparatus | diagnostic stage/counters mirror many product globals; linker-stamped identity blob and sendto wrapper | diagnostics receives snapshots; identity remains specialized; diagnostics never becomes product-state owner |
| B13 Pi companion | legacy `pi/runtime/install-runtime.sh`, `management-server.py`, `vnc-pacing.sh`, `ps2vnc-displayctl`; systemd runtime | dedicated VNC desktop, management/persistent state, health supervisor, private link, traffic pacing; development checkout historically nearby | ordinary supported OS + narrowly custom installed companion runtime; package choices tracked separately |
| B14 build/test/qualification | `scripts/build.sh`; `scripts/testkit/`; `tests/m4/*.env`; `evidence/`; legacy TestKit bridge | exact toolchain/dependency/ELF/PT_LOAD/identity/apparatus state defines empirical meaning | successor-owned reproducible build/identity/evidence contract; migration mechanics historical unless recurring |

---

## 2. Hidden cross-couplings revealed by the source

### 2.1 `runtime` was not one semantic responsibility

The B4A runtime regions contain or expose:

- RFB transport and exact receive buffering;
- active display mode/backend/geometry;
- framebuffer dirty state;
- controller thread and outbound queue state;
- UI/OSK/menu globals;
- display confirmation and remote-test state;
- management paths/constants;
- diagnostics/profiling mirrors.

The historical grouping is therefore a shared-scope artifact, not a clean
runtime domain.

### 2.2 `display` orchestration contained application lifecycle policy

`ps2vnc_display_tail.inc` contains the top-level `main()`/live-loop ownership and
handles controller failure, runtime actions, display requests, recovery, and
exit. The file name describes migration history more than semantic ownership.

The clean application coordinator inherits this orchestration responsibility;
`display` itself should not become the owner of `main()` merely because the
historical tail contains it.

### 2.3 RFB input production and socket ownership were deliberately separated

Historical Test26+ already established that after the controller thread starts,
only the main/render thread may call `send()`/`recv()` on the VNC socket.

The controller thread historically queued **complete encoded RFB messages**.
`recv_exact()` used nonblocking refill after that queue existed and flushed the
queue while waiting on `EAGAIN`.

The clean adaptation retains the one-thread socket rule but moves the queue one
semantic level upward: controller/input publishes typed events, the application
routes them, and RFB owns wire encoding.

### 2.4 RFB wait and local responsiveness share a scheduling seam, not ownership

The final receive loop already distinguishes:

- benign local yield only at a complete message boundary and before the current
  exact read has consumed bytes;
- hard Refresh/rollback/exit interruption that may abandon a partial read and
  therefore makes the stream suspect.

This is a scheduling/coordination seam. It does not justify UI/input owning RFB
transport.

### 2.5 Framebuffer and presentation could alias but are not the same concept

The historical Raw path may receive directly into storage that is also GS-ready,
and later fast paths can make the conventional linear representation stale.

That optimization created real representation-validity state. The clean initial
architecture therefore separates the authoritative desktop framebuffer from
presentation validity even if a later optimized implementation aliases storage
again.

### 2.6 Display state crossed almost every subsystem

A complete display transition historically touched:

- mode/backend/geometry;
- RFB desktop size and stream state;
- framebuffer completeness;
- GS/presentation resources;
- controller/libpad ownership;
- UI curtain/confirmation;
- Pi durable transaction state;
- configuration persistence;
- recovery policy.

This is evidence that the transition is **application orchestration around a
first-class display profile**, not evidence that one giant display module should
own all of those mechanisms.

### 2.7 TCP 5959 grouped unrelated reliability classes

Configuration fetch/save, user display policy, crash-safe display transaction
state, and best-effort development control all used the same small management
transport.

The shared port is not a shared semantic owner. The clean management client
carries typed operations whose callers retain their own failure policy.

### 2.8 Diagnostics historically reached through global state

DBG/PRF/GEOM reporting needed visibility into many subsystems and therefore
historically read broad state directly.

The clean model reverses that dependency: owners construct/report snapshots;
diagnostics never gains authority over the state simply because it records it.

---

## 3. Mechanical slices that should not define the new architecture

The successor M2/M3 extraction work was useful because it exposed link and state
coupling without changing the qualified behavior. Several resulting boundaries
are archaeological rather than semantic:

- `runtime_core` / `runtime_tail`;
- `framebuffer_core` / `framebuffer_tail`;
- `display_core` / `display_tail`;
- `ui_core` / `ui_tail`;
- `calibration_core` / `calibration_tail`;
- migration-direction headers such as `*_core_to_tail.h`, `*_runtime_to_*.h`,
  imports/exports/compatibility headers;
- the broad detached `services` translation unit that accumulated management,
  calibration, reporting, and other shared-scope needs.

These artifacts remain valuable navigation/reference material. Their existence
is not architectural evidence that the clean product needs matching modules.

---

## 4. Proven mechanisms worth retaining or adapting

Retain or adapt based on evidence, not source location:

- qualified PS2IP/dependency discipline and private-link initialization;
- exact RFB framing and fail-closed stream handling;
- nonblocking main-thread receive polling after controller/event production;
- safe message-boundary benign yield versus hard suspect-stream interruption;
- complete authoritative framebuffer before normal incremental operation;
- explicit framebuffer/presentation validity;
- Test14 HIRES interrupt-delivery/`ExitHandler()` discipline;
- complete display-profile rollback and cross-machine durable transaction state;
- controller/libpad ownership handshake and stale-state invalidation;
- one-thread RFB socket ownership;
- one-shot OSK modifiers and local held-button quarantine;
- strict configuration validation with forward-compatible unknown keys/sections;
- manual Refresh cooldown and explicit/silent failure distinction;
- deterministic runtime identity and fail-closed TestKit identity checks;
- Pi endpoint-based service health;
- exact DUT/PT_LOAD/apparatus/evidence qualification contracts.

---

## 5. Historical structures not to copy forward

Do not preserve merely because they are present in qualified source:

- broad writable globals spanning unrelated domains;
- encoded controller-thread RFB packet construction;
- UI/controller code directly reading/writing each other's internal state;
- one-pixel remote-pointer wakeup as a local repaint mechanism;
- RFB and GS storage aliasing as an architectural requirement;
- management transport helpers copied into multiple semantic paths;
- B4A `.inc`/core/tail migration partitions;
- a one-file-per-M4-target-leaf decomposition;
- development checkout location as Pi runtime state;
- test-control helpers as hard product runtime dependencies.

---

## 6. Completion

The source/state responsibility mapping requested by GitHub Issue #3 is now
durable in this document and the B01-B14 detailed audits.

The clean replacement authority is derived in the cross-domain synthesis rather
than inferred from this historical source map.
