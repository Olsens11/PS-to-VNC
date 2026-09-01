# PS-to-VNC Cross-Domain State Ownership and Interfaces

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_6
    INPUT_BEHAVIOR_FAMILIES=B01_B14
    INPUT_MATURITY=EVIDENCE_SUPPORTED
    SYNTHESIS_COMPONENT=STATE_OWNERSHIP_AND_INTERFACES
    SYNTHESIS_STATE=ACTIVE
    REBUILD_READY=NO

This document makes the state-ownership and interface consequences of the
B01-B14 behavioral audit explicit. It is a companion to
`docs/audit/CROSS_DOMAIN_SYNTHESIS.md`.

The goal is not to maximize module count. The goal is to make it obvious who may
change each important piece of state, which values cross boundaries, and where
cross-domain policy is allowed to execute.

---

## 1. Ownership rule

Every important mutable state group has one semantic owner.

Other domains may:

- receive an immutable/copy value;
- hold an opaque handle owned by the provider;
- call a narrow owner API;
- consume an event explicitly published for them.

They may not acquire ownership merely because historical source exposed a global.

A top-level application object may contain subsystem objects for lifetime and
allocation convenience. That does not make the object a public bag of fields.

---

## 2. PS2-side mutable-state ownership table

| State group | Owner | Writer(s) | Legitimate readers/consumers | Thread/lifetime rule |
|---|---|---|---|---|
| application phase and shutdown outcome | application coordinator | main/application | diagnostics, UI through snapshots | main-owned |
| urgent control intent | application control mailbox | controller producer; main clears/consumes | RFB wait sees only generic `urgent pending`; app reads semantic reason | cross-thread, minimal SPSC control seam |
| ordinary semantic input events | input event queue | controller producer | application coordinator | cross-thread SPSC queue; no encoded RFB bytes |
| manual Refresh lifecycle/cooldown deadline | application coordinator | main/application | UI presentation, diagnostics | main-owned policy state |
| risky display transition/confirmation/rollback lifecycle | application coordinator | main/application; controller may publish timeout/urgent intent only | UI receives presentation snapshot; diagnostics | main-owned policy state |
| startup transaction reconciliation result | application coordinator | main/application | display, RFB, management as passed `display_profile` values | resolved before ordinary live operation |
| RFB socket and connection generation | RFB session | main through RFB API | app sees status/result only | one socket owner; controller never calls send/recv |
| RFB receive buffer/cursors/exact framing state | RFB session | RFB receive path | diagnostics snapshot only | private to session; reset on replacement |
| negotiated RFB pixel format/encodings/capabilities | RFB session | RFB negotiation | app/display transition via queries/results | private protocol state |
| outstanding framebuffer request / resize protocol state | RFB session | RFB API | app receives completion/failure | private protocol state |
| authoritative remote desktop pixels | desktop framebuffer | RFB decode through framebuffer API | display presentation | one CPU-image owner |
| desktop framebuffer dimensions/validity/dirty state | desktop framebuffer | framebuffer API during RFB decode/reset | RFB validation, display presentation, diagnostics snapshots | validity changes with pixels atomically at domain level |
| active display profile | display | display transaction mechanism called by app | app/config/UI/management receive copies | display owns live authority, app owns transition policy |
| GS/Standard/HIRES presentation resources | display/presentation | display implementation | app only through operations/status | hardware-facing owner |
| presentation-buffer/mapping validity | display/presentation | display implementation | diagnostics snapshot only | must not be inferred from RFB framebuffer validity |
| mode catalog and pure geometry rules | display model | effectively immutable after initialization | app, config, UI, display, management serialization | stable value/model data |
| transient calibration edit state | local UI | main/UI | app receives proposed values | UI owns interaction; display model validates/derives profile |
| persisted desired per-mode calibration/startup/lock/hide/bindings | configuration model | config parse/update operations | app, input, UI, management persistence adapters | desired state, not live hardware authority |
| controller physical/derived/pointer/gesture state | input/controller | controller thread | app only via events/snapshots | controller-owned |
| live binding recognizer state | input/controller | controller thread | diagnostics snapshot | human-readable binding definition remains config-owned |
| libpad pause/acquire/ack/release state | input/controller | app requests/release; controller publishes ack | app and controller only | explicit ownership handshake |
| physical-release quarantine | input/controller | app/UI requests quarantine; controller clears on physical release | input recognizer | physical-state owner remains input |
| local UI foreground/menu/OSK/modifier state | local UI | main/UI | app and diagnostics through snapshots | main-owned; controller does not mutate UI globals |
| local UI generation/dirty state | local UI | main/UI | coordinator/display presentation | first-class local wake/repaint condition |
| parsed human configuration candidate | configuration | main/config parser | app after complete validation | side-effect free until accepted |
| management connection-local HTTP state | management client | main/management | caller receives typed result | short-lived/private |
| cached management display policy where needed | management client or app-owned accepted policy snapshot | main | app/UI/config by passed value | cache does not become display authority |
| durable display transaction authority on Pi | Pi management service | Pi service through semantic transaction endpoints | PS2 app obtains typed snapshot/ack | cross-machine safety authority, not PS2 global state |
| diagnostics transport/stage state | diagnostics | main/diagnostic API | diagnostics only | optional runtime facility |
| profiling counters/timestamps | diagnostics | instrumented owners through diagnostic API or owned scoped counters | evidence/reporting | must not become product control state |
| runtime ELF identity blob/emission state | diagnostics identity | linker/TestKit stamp + identity implementation | qualification apparatus | specialized immutable/stampable identity |
| PS2 network hardware/link state | platform network mechanism | PS2SDK/NetMan | app/RFB recovery via query | mechanism only; policy remains above |
| PS2 pad hardware state | platform pad mechanism | input owner while it owns libpad | input or temporary app calibration owner during explicit handoff | never concurrently owned |

### Ownership consequences

Several historical global groupings deliberately disappear:

- display mode, backend, geometry, safe area, and startup persistence are not one
  writable global cluster;
- local UI state is not controller-thread state;
- RFB receive state is not framebuffer state;
- framebuffer validity is not presentation validity;
- desired configuration is not durable transaction authority;
- controller hardware ownership is not the same thing as already-published input
  intent;
- diagnostics mirrors state but does not own the state it reports.

---

## 3. Cross-thread communication model

The clean runtime needs only three explicit cross-thread mechanisms.

### 3.1 Ordinary semantic input queue

The controller thread publishes typed events such as:

- pointer motion/state;
- pointer button changes;
- scroll;
- key or configured logical action;
- local navigation/activation gestures.

The queue stores meaning, not serialized RFB packets.

The application coordinator consumes these events and decides whether the
current foreground routes them to local UI, a product command, or the RFB input
serializer.

### 3.2 Urgent control mailbox

Some controller-originated events must regain main-thread control even when the
main thread is waiting for RFB data. Examples include:

- manual Refresh;
- exit;
- confirmation timeout / mandatory rollback;
- controller fatal error.

The RFB receive layer is allowed to observe only a generic distinction:

    no urgent intent
    urgent intent pending

It does not inspect `Refresh`, `Exit`, or rollback policy itself. The application
coordinator consumes the semantic reason after control returns.

The exact EE synchronization primitive is an implementation choice. A small
single-producer/single-consumer scalar or queue protected by an existing simple
EE primitive is preferred over a generalized event framework.

### 3.3 libpad ownership handshake

This remains distinct from both event channels.

Application-owned hazardous operations request pad ownership. The controller
acknowledges immediately before its next libpad access and then performs no
further pad access until release. On resume, input invalidates stale derived
state according to the proven F8J2/B4A rules.

---

## 4. RFB wait responsiveness without a parser framework

The clean architecture does **not** require converting the RFB parser into a
fully incremental event-driven state machine merely to service controller/UI
work.

The final B4A path already demonstrates a simpler useful mechanism:

- before the controller queue exists, handshake/startup may use ordinary
  blocking receive;
- after controller/event production is active, the main-thread RFB receive path
  uses nonblocking `recv(..., MSG_DONTWAIT)` refill;
- the main thread therefore remains the sole PS2IP socket owner while it can
  periodically service work during `EAGAIN`;
- a benign local yield is allowed only when the caller marks a complete
  server-message boundary and the current `recv_exact()` has consumed zero bytes;
- hard actions such as Refresh/rollback/exit may abort even after partial
  consumption, in which case the stream becomes suspect and is replaced.

The clean adaptation generalizes the historical local-redraw yield:

1. `recv_exact()` continues to mean "obtain this exact protocol field or fail";
2. the RFB message decoder identifies complete server-message boundaries;
3. when the socket is idle at such a boundary and normal semantic input/UI work
   is pending, RFB returns a benign `APP_YIELD` result without invalidating the
   stream;
4. the coordinator drains/routes events and performs any RFB input writes;
5. receive resumes on the same synchronized stream;
6. if urgent hard intent is observed at any point, RFB returns a distinct
   interrupted/suspect result and the application applies the owning policy.

This preserves straightforward exact-length protocol code while replacing the
historical encoded-controller-RFB queue with typed input events.

### Why this matters

Without this boundary rule, one of two bad architectures would reappear:

- the controller thread would need to write the RFB socket again; or
- the main thread could remain blocked indefinitely and starve ordinary input.

The evidence-supported nonblocking refill plus safe-boundary yield avoids both.

---

## 5. Interface matrix

The following are semantic interfaces. Exact C names may change during
implementation; direction and ownership are the important contract.

| Provider | Consumer | Passed state/value | Operations needed | Forbidden shortcut |
|---|---|---|---|---|
| application coordinator | all runtime domains | explicit calls/values | orchestrate startup/live/transition/recovery | domains reading app internals |
| platform system | application | status/results | bootstrap IOP/modules; exit to OSDSYS | app duplicating low-level lifecycle sequences |
| platform network | application/RFB policy queries | link status/mechanism result | initialize private link; query carrier | platform deciding reconnect policy |
| input | application | `input_event`, urgent-pending, pad ownership result | start/stop; poll thread; dequeue; request/release pad; quarantine | input writing socket/GS or mutating UI state |
| local UI | application/display presentation | semantic UI actions + UI surface/dirty state | route event; render/update local surface; show policy snapshots | UI performing management/RFB/display transaction |
| config | application | validated typed config/profile/binding values | parse; validate; derive accepted config; serialize desired settings | parser mutating live display/input |
| management | application | typed config/policy/transaction values | GET/POST semantic operations | management mutating GS/display directly |
| RFB session | application | session status/update result | connect/handshake; request; receive; replace; resize; send remote input | RFB deciding Refresh/rollback policy |
| RFB session | desktop framebuffer | validated pixel updates | write Raw/Hextile result through framebuffer API | RFB touching GS resources |
| desktop framebuffer | display | coherent pixels, dimensions, dirty region | query/read coherent image; clear/advance dirty state by explicit contract | display inferring parser/session state |
| display model | application/config/UI/management/display | `display_profile`, mode/profile properties | lookup; validate; derive geometry/profile | callers rebuilding profile from unrelated globals |
| display presentation | application | accepted `display_profile` + framebuffer reference | create/reconstruct; prepare; present; flip/sync; destroy | display deciding candidate acceptance |
| diagnostics | all domains | snapshots/reports | stage/report/profile/identity APIs | diagnostics calling back to change product state |
| Pi management service | PS2 management client | HTTP wire representation of typed values | config/policy/transaction/control endpoints | PS2 assuming service process state from transport success alone |

### Remote input serialization

RFB pointer/key wire encoding remains RFB-owned. The application routes a typed
remote-input event to an RFB operation; it does not construct protocol bytes.

This retains one socket owner without giving RFB ownership of physical control
mapping.

---

## 6. Critical-flow dependency verification

### 6.1 Startup

Expected dependency sequence:

    application
      -> desktop framebuffer allocation
      -> platform system/bootstrap
      -> platform/input startup pad sample
      -> platform network
      -> management/config retrieval
      -> config validation
      -> application startup transaction reconciliation
      -> display model/profile selection
      -> display presentation creation
      -> diagnostics optional initialization
      -> RFB session establishment
      -> complete framebuffer acquisition
      -> display presentation
      -> input event production enabled
      -> live loop

Result: **PASS**.

No lower domain needs application internals. Startup pad sampling occurs before
ordinary controller event production, so the force-calibration chord does not
require prematurely starting a controller thread that can emit remote actions.

### 6.2 Ordinary live desktop

Expected loop dependency:

    controller thread -> semantic input queue
    main application -> dequeue/route
      -> UI for local foreground actions
      -> RFB serializer for remote pointer/key actions
      -> application policy for Refresh/display/exit commands
    main application -> RFB request/receive
    RFB decode -> desktop framebuffer
    display -> desktop framebuffer -> physical presentation

When RFB is idle, safe-boundary `APP_YIELD` returns control to service queued
input/UI work. A partially consumed protocol field is not abandoned for benign
work.

Result: **PASS**, provided the clean RFB wait API preserves the safe-boundary
benign-yield versus hard-interrupt distinction.

### 6.3 Risky display transition

Expected ownership sequence:

    application
      -> resolve previous/candidate display_profile values
      -> management BEGIN durable transaction
      -> input acquire pad ownership
      -> input discard/quarantine pre-boundary remote intent as required
      -> UI transition curtain
      -> display reconstruct candidate
      -> RFB resize/reconcile
      -> desktop framebuffer complete authoritative frame
      -> display prepare/present coherent destination
      -> input release ownership
      -> application confirmation/lease policy
          KEEP -> management COMMIT
          ROLLBACK -> input acquire -> display/RFB/full-frame restore
                      -> management RESTORED/ACK reconciliation

Result: **PASS**.

The profile value prevents management, RFB, and display from independently
assembling different interpretations of the candidate/rollback authority.

### 6.4 Explicit RFB failure / manual Refresh

Expected dependency sequence:

    RFB reports explicit failure or hard interrupt
      -> application classifies owning policy
          explicit ordinary failure -> automatic replacement policy
          manual Refresh -> Refresh lifecycle/cooldown policy
          unconfirmed candidate -> display rollback policy
          unexplained EAGAIN silence -> no generic timeout policy
      -> RFB complete replacement mechanism where appropriate
      -> complete authoritative desktop framebuffer
      -> display coherent presentation
      -> UI status/result

Result: **PASS**.

RFB replacement is one mechanism reused by multiple policies; it does not become
the owner of those policies.

### 6.5 Local UI/OSK while remote desktop is idle

Expected sequence:

    controller -> semantic event
    safe RFB message-boundary APP_YIELD
    application -> UI consumes event
    UI marks local generation dirty
    application/display presents local UI change
    application resumes RFB wait

Result: **PASS**.

No pointer jiggle or synthetic remote framebuffer damage is required.

---

## 7. Initial clean source shape implied by the ownership model

This is a deliberately small **starting shape**, not a promise that every domain
must remain one file forever:

    src/
        main.c
        app.c / app.h
        rfb.c / rfb.h
        framebuffer.c / framebuffer.h
        display.c / display.h
        input.c / input.h
        ui.c / ui.h
        config.c / config.h
        management.c / management.h
        diagnostics.c / diagnostics.h
        diagnostics_identity.c / diagnostics_identity.h
        platform/
            ps2_system.c / ps2_system.h
            ps2_network.c / ps2_network.h
            ps2_pad.c / ps2_pad.h
            ps2_graphics.c / ps2_graphics.h
            ps2_graphics_hires.c / ps2_graphics_hires.h   # only if HIRES size/invariants earn a split

This is substantially smaller than the old M4 target hierarchy.

Likely later splits are permitted when implementation earns them—for example
Hextile from RFB, OSK from UI, pure display-model/geometry from hardware display,
or HIRES graphics from ordinary GS operations. The first reconstruction should
not create those files merely because the names are predictable.

The stable architecture is the **ownership and dependency contract**, not the
number of `.c` files.

---

## 8. Remaining synthesis decisions before REBUILD_READY

The behavioral knowledge is no longer missing. Remaining work is repository and
implementation-boundary closure:

1. reconcile/supersede the old M4 architecture documents that still describe a
   normalization-era target as normative;
2. decide how the first clean Pi OS/runtime dependency ledger records TigerVNC,
   Openbox/lxpanel or replacements, management service, private Ethernet, and
   historical VNC pacing;
3. decide which first hardware milestone continues through the proven legacy
   TestKit bridge and which minimal successor-owned tools are required before
   that milestone;
4. publish one canonical clean architecture document and update navigation/status
   so a new development session cannot confuse the M4 target with current
   authority;
5. promote B01-B14 to `REBUILD_READY` only after those authority surfaces agree.

The first three are adoption/packaging decisions; they do not require reopening
B01-B14 behavioral discovery unless new evidence contradicts the audit.
