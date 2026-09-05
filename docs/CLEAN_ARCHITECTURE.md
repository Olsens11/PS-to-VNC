# PS-to-VNC Clean Reconstruction Architecture

## Status

    ARCHITECTURE_VERSION=1
    STATUS=REBUILD_READY
    DERIVED_FROM=B01_B14_SEMANTIC_AUDIT
    PRIMARY_ARCHITECTURE_AUTHORITY=YES

This is the current architecture authority for the clean PS-to-VNC
reconstruction.

It is derived from the evidence-supported B01-B14 behavioral inventory and the
cross-domain ownership/interface synthesis.

For the clean reconstruction, this document supersedes the earlier
normalization-era target contained in:

- `ARCHITECTURE.md`;
- `TARGET_ARCHITECTURE.md`;
- `STATE_OWNERSHIP.md`;
- `DEPENDENCY_RULES.md`;
- `NORMALIZATION_SEQUENCE.md`;
- `ARCHITECTURAL_NORMALIZATION.md`.

Those documents remain valuable historical architecture/reference material.
Their completed migration evidence and useful principles remain valid; their
preselected M4 decomposition is no longer the current product blueprint.

Supporting semantic authority:

- `audit/BEHAVIORAL_INVENTORY.md`;
- `audit/CROSS_DOMAIN_SYNTHESIS.md`;
- `audit/CROSS_DOMAIN_STATE_INTERFACES.md`;
- `audit/SOURCE_RESPONSIBILITY_MAP.md`;
- `audit/HISTORICAL_LESSONS.md`.

---

## 1. Architecture rule

> Use the smallest set of explicit owners and interfaces that makes the product
> easier to understand than the exploratory implementation.

PS-to-VNC is an embedded C application, not a framework.

Prefer:

- direct calls;
- ordinary structs and enums;
- one clear owner for important mutable state;
- explicit lifecycle/state machines where behavior actually has states;
- narrow domain APIs;
- platform seams only where hardware ownership or testing earns them;
- host-testable pure logic;
- real PS2 qualification for hardware-facing claims.

Do not trade a readable monolith for a large number of tiny forwarding modules.
File boundaries remain malleable; responsibility and ownership boundaries are the
architecture.

---

## 2. PS2-side runtime responsibilities

### Application coordinator

Owns product-level orchestration:

- startup and shutdown ordering;
- the live main loop;
- routing semantic input actions;
- urgent cross-thread control intent;
- manual Refresh policy/cooldown;
- risky display transition, confirmation, rollback, and startup reconciliation;
- failure classification and recovery policy.

It coordinates other domains but does not implement their private mechanisms.

### RFB client/session

Owns one synchronized VNC/RFB protocol session:

- socket/session state;
- RFB 3.8 handshake/security/ServerInit;
- exact receive buffering/framing;
- pixel format, encodings, capabilities;
- framebuffer requests;
- server-message/rectangle parsing;
- Raw/Hextile decoding mechanics;
- ExtendedDesktopSize protocol behavior;
- pointer/key wire serialization;
- complete session replacement.

RFB reports mechanism results; it does not decide Refresh, rollback, or product
recovery policy.

### Desktop framebuffer

Owns the authoritative CPU-side remote desktop image:

- storage/capacity;
- logical dimensions;
- validity/staleness;
- dirty/update regions.

RFB writes validated pixels into it. Display reads coherent pixels from it.

The first clean reconstruction uses one conventional authoritative EE
framebuffer. Historical direct-to-presentation paths are deferred performance
work.

### Display model and presentation

The display domain owns:

- mode catalog and stable mode identity;
- backend compatibility;
- physical raster/timing properties;
- safe-area/geometry rules;
- complete `display_profile` values;
- the active display profile;
- Standard/HIRES presentation resources and validity;
- prepare/draw/finish/flip/sync mechanisms;
- qualified PS2 HIRES interrupt behavior.

It implements display mechanisms. The application decides whether a candidate
profile should be entered, kept, or rolled back.

### Input/controller

This is a responsibility family rather than a requirement for one controller
translation layer.

At the hardware-facing edge, PS-to-VNC uses PS2SDK `libpad` directly and keeps
its established PS2 vocabulary. Basic pad acquisition owns factual physical
observation, minimal immediate transition history, polling/lifecycle state, and
stale-state invalidation across libpad ownership boundaries.

Higher-level input responsibilities compose around those facts:

- pointer/scroll response;
- optional chord/hold/release recognition;
- live binding recognition;
- libpad acquire/ack/release handoff;
- physical-release quarantine;
- semantic input-event production.

Pointer interpretation and chord recognition are not properties of basic pad
acquisition. Chord semantics are opt-in per consuming input path.

Input never writes the RFB socket or manipulates GS/display state.

See `adr/0002-use-libpad-directly-reference-opl.md`.

### Local UI

Owns:

- foreground/modal state;
- System and Display Settings interaction;
- OSK interaction and one-shot modifier state;
- confirmation/restored-ack presentation;
- underlay/return relationships;
- local UI surface generation and dirty state.

UI produces semantic actions. It does not perform management, RFB, or display
transactions.

### Configuration

Owns the typed human-editable desired configuration:

- parsing/grammar/text helpers;
- strict recognized-value validation;
- defaults;
- desired startup mode;
- per-mode safe area;
- lock/hide policy;
- human-readable controller/hotkey bindings;
- serialization of desired configuration where needed.

Parsing is side-effect free. Desired configuration is not live display authority
and is not crash-safe display transaction authority.

### Management client

Owns the small PS2-side Pi management protocol:

- bounded HTTP request/response mechanics;
- semantic configuration/persistence operations;
- display policy operations;
- durable display transaction wire operations;
- explicitly best-effort development/test-control operations where retained.

Sharing TCP 5959 does not make these operations one reliability class.

### Diagnostics and runtime identity

Owns optional observability:

- debug stage/UDP transport;
- DBG/PRF/GEOM reporting;
- diagnostic profiling;
- exact runtime ELF identity emission.

Diagnostics receives snapshots and never becomes the owner of product state.
Runtime identity remains specialized because its linker-stamped format and
qualification contract are genuinely distinct.

### PS2 platform mechanisms

Recognizable PS2-specific mechanisms exist where hardware/lifecycle invariants
justify a seam:

- IOP/module/system bootstrap and OSDSYS exit;
- network-interface initialization/link state;
- libpad access;
- GS/dmaKit/Standard/HIRES graphics operations;
- Test14 interrupt-delivery/`ExitHandler()` discipline;
- timing/cache primitives where needed.

Do not wrap every PS2SDK/libc function merely for architectural appearance.

---

## 3. Stable cross-domain values

### `display_profile`

A complete display profile is passed as one value. It contains enough validated
state to identify or restore a display configuration, including:

- mode identity;
- backend identity;
- logical desktop geometry;
- presented output rectangle;
- safe-area/calibration identity or values required for complete restoration.

No consumer reconstructs a profile by independently reading unrelated globals.

### `desktop_framebuffer`

An owned object containing the authoritative CPU image plus dimensions,
validity, and dirty state.

### semantic input/action values

Typed values represent pointer, buttons, scroll, keys, UI navigation, and
configured product actions. Controller gestures map to semantic identifiers;
input does not own what executing the action means.

### typed management values

Management serializes stable config/profile/transaction values. It does not read
live display globals to construct authority implicitly.

---

## 4. Concurrency model

The clean runtime preserves one essential historical rule:

> the main/application thread is the sole owner of VNC socket operations and
> cross-domain product side effects.

### Main/application thread

Owns:

- all RFB writes/requests and receive orchestration;
- display transactions/reconstruction;
- management transaction sequencing;
- UI routing/state mutation;
- shutdown/recovery execution.

### Controller thread

Owns controller hardware/derived state and publishes only semantic events or
urgent intent.

It does not call VNC `send()`/`recv()` and does not manipulate GS.

### Three explicit cross-thread seams

1. **ordinary semantic input queue** — controller producer, application consumer;
2. **urgent control mailbox** — Refresh/exit/mandatory rollback/controller error
   can regain main-thread control while RFB is idle;
3. **libpad ownership handshake** — explicit request/ack/release for hazardous
   application operations.

These remain separate instead of becoming a giant thread-safe application-state
object.

---

## 5. RFB wait and application responsiveness

The clean implementation may retain a straightforward exact-length RFB parser.
It does not need a generalized incremental parser framework merely to service
input.

Once controller/event production is active:

- RFB socket refill is nonblocking on the main thread;
- idle `EAGAIN` polling can detect queued application work;
- **benign work** yields only at a complete server-message boundary before the
  current exact read consumes bytes;
- application drains/routes events and performs any remote-input RFB writes;
- receive resumes on the same synchronized stream;
- **hard intent** such as Refresh/rollback/exit may interrupt even after partial
  consumption, making the old stream suspect and requiring replacement.

This generalizes the proven historical safe-boundary local-redraw mechanism while
removing encoded RFB construction from the controller thread.

---

## 6. Dependency direction

Conceptually:

    application
        -> input
        -> ui
        -> config
        -> management
        -> rfb
        -> desktop framebuffer
        -> display
        -> diagnostics
        -> platform mechanisms

    rfb
        -> desktop framebuffer
        -> diagnostics
        -> socket/PS2IP facilities

    display
        -> display model/value types
        -> desktop framebuffer
        -> platform graphics
        -> diagnostics

    input
        -> stable action/event types
        -> platform pad
        -> diagnostics

    ui
        -> stable action/display value types

    config
        -> stable display/input value types

    management
        -> stable config/display/transaction values
        -> socket facilities

    diagnostics
        -> transport/timing facilities only

Prohibited shortcuts:

- subsystem -> application internals;
- input/UI -> RFB socket or management transport;
- RFB -> GS/display policy;
- display -> RFB parsing;
- management -> direct display mutation;
- config parser -> product effects;
- diagnostics -> product decisions;
- broad `globals.h`/`common.h` used to disguise cycles.

---

## 7. Startup lifecycle

The clean startup is readable top-level orchestration:

1. allocate the authoritative desktop framebuffer capacity;
2. initialize PS2 system/IOP/module foundation;
3. initialize PS2 network hardware;
4. initialize early visible/debug facilities as appropriate;
5. initialize pad hardware and sample the startup force-calibration chord;
6. configure PS2 IP and wait for the private Ethernet link;
7. retrieve human configuration and durable display transaction state;
8. parse/validate config and reconcile one authoritative startup
   `display_profile`;
9. construct local display/presentation for that authority;
10. initialize optional diagnostics/runtime identity;
11. establish a fresh reconciled RFB session;
12. obtain one complete authoritative framebuffer;
13. present the coherent desktop;
14. enable ordinary controller-event production;
15. enter the live coordinator loop.

Explicit fatal startup and user exit paths converge on the known OSDSYS/system
menu outcome.

---

## 8. Live loop

Conceptually the coordinator repeatedly:

1. process urgent control intent;
2. drain semantic input events;
3. route them to UI, application policy, or RFB input serialization;
4. present local UI changes when locally dirty;
5. request/receive RFB damage;
6. decode into the authoritative desktop framebuffer;
7. present coherent changed desktop regions;
8. advance wall-clock confirmation/cooldown state;
9. poll explicitly best-effort development control when enabled;
10. classify failures and enter the owning policy.

Exact scheduling can evolve, but ownership and safe RFB-boundary rules remain.

---

## 9. Risky display transition

Application policy coordinates the transaction:

1. resolve/validate complete previous and candidate profiles;
2. begin durable Pi transaction state before destroying known-good authority;
3. acquire libpad ownership and discard/quarantine pre-boundary remote input as
   required;
4. expose a local transition curtain;
5. reconstruct candidate presentation;
6. converge RFB/TigerVNC logical geometry;
7. obtain a complete authoritative candidate framebuffer;
8. present one coherent destination desktop;
9. release controller ownership;
10. run human confirmation or explicit remote-test lease policy;
11. KEEP commits durable candidate authority;
12. rollback reacquires ownership, reconstructs/verifies the complete saved
    profile, and reconciles RESTORED/ACK state explicitly.

No participant other than application policy owns the cross-domain sequence.

---

## 10. Recovery policy

The architecture preserves four separate cases:

- explicit carrier/socket/RFB failure -> automatic complete session replacement;
- manual Refresh -> controlled replacement with visible state and anti-spam
  cooldown;
- failure during an unconfirmed display candidate -> display rollback policy;
- unexplained silent wait -> no generic automatic timeout during the current
  debugging policy.

RFB exposes the replacement mechanism. Application decides when and why it is
used.

---

## 11. Pi companion boundary

The Pi is the other side of the product, not an extension of the PS2 source
hierarchy.

Required behavior is:

- supported ordinary Raspberry Pi OS baseline;
- standard package/network/service mechanisms wherever practical;
- private PS2 Ethernet;
- predictable dedicated VNC desktop/session;
- small management daemon and persistent state;
- endpoint-based health/recovery;
- operator start/stop/restart/status controls;
- versioned install/package definition and dependency ledger.

Exact package choices belong to GitHub Issue #5 and may remain `EVALUATING`
without reopening PS2 architecture.

Historical TigerVNC/Openbox/lxpanel and 60 Mbit/s VNC-only pacing are reference
implementations/settings to evaluate, not unexplained defaults that automatically
become clean-platform law.

VNC desktop availability remains independent of management-service availability.

---

## 12. Development and qualification boundary

The clean runtime architecture does not include TestKit as a runtime subsystem,
but product engineering must preserve:

- reproducible toolchain/dependency identity;
- exact whole-ELF/PT_LOAD/runtime identity;
- canonical deployment/readback where required;
- declarative tests and apparatus provenance;
- machine versus physical/operator evidence separation;
- PT_LOAD hardware gate and explicit qualification-transfer authority;
- byte-preserved/sealed raw evidence;
- safe dirty-state/development continuity.

The first clean PS2 hardware milestones continue through the proven immutable
successor→legacy TestKit bridge. Replacing mature TestKit is not a prerequisite
for reconstruction and must itself be justified/qualified when undertaken.

---

## 13. Source-shape evolution

### Initial planning shape — historical proposal

The original clean-reconstruction architecture proposed starting small with the
following mostly-flat candidate shape. This records the planning state that
existed before the clean implementation grew through Issues #7, #38, and #39.
It is not the current file-location map and is intentionally retained here so
the later topology is not projected backward into earlier history.

Start small:

    src/
        main.c
        app.c / app.h
        rfb.c / rfb.h
        framebuffer.c / framebuffer.h
        display.c / display.h
        pad.c / pad.h
        input.c / input.h
        ui.c / ui.h
        config.c / config.h
        management.c / management.h
        diagnostics.c / diagnostics.h
        diagnostics_identity.c / diagnostics_identity.h
        platform/
            ps2_system.c / ps2_system.h
            ps2_network.c / ps2_network.h
            ps2_graphics.c / ps2_graphics.h

A separate `ps2_graphics_hires.c`, Hextile implementation file, OSK file, pure
display-model/geometry file, or other split is introduced only when real
implementation size, independent testing, or hardware invariants make the split
clearer.

Stage #38 refined the earlier candidate `input.c` plus
`platform/ps2_pad.c` split. `pad.c` / `pad.h` owns the minimal project-specific
use of libpad; it is not a replacement controller API. A broader `input.c`
remains available when semantic routing or another earned input responsibility
requires it.

### Current clean source topology — adopted 2026-09-05

By Issue #39 closeout the implementation had earned several coherent
responsibility families. On 2026-09-05 those files were reorganized into the
following current topology:

    src/
        main.c
        app.c / app.h
        SYMBOLS.md
        diagnostics/
            diagnostics.c / diagnostics.h
            identity.c / identity.h
            SYMBOLS.md
        display/
            display.c / display.h
            SYMBOLS.md
        framebuffer/
            framebuffer.c / framebuffer.h
            SYMBOLS.md
        input/
            controller.h
            input.c / input.h
            input_runtime.c / input_runtime.h
            keyboard.c / keyboard.h
            mouse.c / mouse.h
            pad.c / pad.h
            SYMBOLS.md
        platform/
            ps2_system.c / ps2_system.h
            ps2_network.c / ps2_network.h
            ps2_graphics.c / ps2_graphics.h
            SYMBOLS.md
        rfb/
            rfb.c / rfb.h
            rfb_io.h
            rfb_session.c / rfb_session.h
            SYMBOLS.md
        ui/
            local_controller.c / local_controller.h
            local_ui.c / local_ui.h
            local_ui_presentation.c / local_ui_presentation.h
            osk.c / osk.h
            osk_render.c / osk_render.h
            SYMBOLS.md

`src/config/` and `src/video/` contain retained pre-refresh source and are not
current clean-generation ownership merely because those historical paths still
exist. Future deliberate adoption or replacement must follow the same topology
and dictionary rules.

The architecture does **not** require this exact file count. It requires the
ownership contract above. New responsibility directories are allowed when
earned, but they are deliberate architecture changes rather than accidental
growth at the root.

The normative forward file-placement policy is
`development/source-topology.md`. The chronological adoption record is
`reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md`.

See `adr/0002-use-libpad-directly-reference-opl.md`.

---

## 14. Historical mechanism disposition

### Adopt

- qualified private Ethernet/PS2IP dependency discipline;
- exact RFB framing and fail-closed stream handling;
- one-thread PS2IP/VNC socket ownership;
- nonblocking idle receive servicing and safe-boundary benign yield;
- complete authoritative frame before normal incremental operation;
- explicit framebuffer/presentation validity;
- Test14 HIRES interrupt discipline;
- complete display-profile transaction/rollback;
- libpad handoff and stale-state invalidation;
- one-shot OSK modifiers;
- held-button quarantine;
- manual Refresh policy/cooldown;
- strict human config validation/forward compatibility;
- durable Pi display transaction reconciliation;
- deterministic runtime identity;
- endpoint-based Pi service health;
- exact DUT/PT_LOAD/apparatus/evidence qualification.

### Adapt

- encoded controller RFB queue -> typed semantic event queue;
- action/request globals -> explicit event queue + urgent control mailbox;
- display globals -> `display_profile` values + owner state;
- RFB/GS framebuffer globals -> owned desktop framebuffer;
- copied management helpers -> one small typed management client;
- menu/OSK globals -> one main-owned UI state;
- exploratory Pi checkout/runtime -> installed reproducible companion state.

### Defer

- direct-to-presentation paths that can stale the conventional framebuffer;
- advanced dirty/Hextile/presentation optimization beyond the first stable path;
- MPEG/hybrid-video work;
- nonessential exploratory display modes;
- aesthetic replacement of mature TestKit.

### Reject as clean architecture

- B4A `.inc`/core/tail boundaries;
- preselected M4 one-file-per-leaf hierarchy;
- broad unrestricted globals or giant public app state;
- controller thread socket writes;
- local UI pointer-jiggle wakeup as primary repaint;
- automatic generic silent-stall reconnect during current debugging;
- best-effort test control as hard runtime dependency;
- generalized component/callback/DI framework.

---

## 15. Reconstruction sequence

Design remains top-down; implementation proceeds bottom-up:

1. PS2/Pi Ethernet foundation;
2. minimal RFB handshake/session;
3. one authoritative desktop framebuffer;
4. fixed proven 480p presentation;
5. controller/pointer semantic event path;
6. keyboard/OSK;
7. local UI;
8. typed configuration/persistence;
9. display profiles/calibration and durable transitions;
10. recovery/management;
11. performance paths and additional modes;
12. future MPEG/hybrid video if still justified.

Every stage is built directly in the intended architecture rather than in a
second temporary monolith.

---

## 16. Acceptance and malleability

This architecture is `REBUILD_READY`, not immutable.

A boundary may change when implementation or new hardware evidence shows that a
simpler ownership arrangement is better. Such changes must preserve audited
behavior/invariants and update this authority rather than silently growing a
parallel architecture.

The reconstruction is succeeding when a fresh developer can follow startup,
RFB flow, input routing, display transitions, recovery, and Pi responsibilities
without knowing the M0-M4 extraction history or reading the frozen 18,938-line
implementation.
