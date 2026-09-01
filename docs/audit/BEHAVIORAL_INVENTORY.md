# PS-to-VNC Behavioral Inventory

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    INVENTORY_STRUCTURE=SEEDED
    DETAILED_BEHAVIOR_AUDIT=IN_PROGRESS

This is the top-level inventory of product behaviors and responsibilities that
must be understood before the clean architecture is derived.

A row marked `SEEDED` means only that the behavior family is known to exist and
initial evidence roots have been identified.

It does **not** mean the historical implementation, ownership model, or prior
architecture has been accepted for the clean reconstruction.

## Inventory

| ID | Behavior family | Maturity | Initial evidence roots |
|---|---|---|---|
| B01 | Startup and application lifecycle | SEEDED | `working/b4a/`, migration/runtime authority, historical startup evidence |
| B02 | PS2-to-Pi Ethernet and networking | SEEDED | B4A/current source, PS2SDK-facing code, network test history |
| B03 | RFB connection, handshake, negotiation, and session state | SEEDED | B4A/current RFB code, protocol diagnostics, RFB test evidence |
| B04 | Framebuffer updates, rectangles, Raw/Hextile decode, and framebuffer validity | SEEDED | B4A/current decode path, profiling/evidence, framebuffer investigations |
| B05 | GS/video presentation and framebuffer-to-display transfer | SEEDED | B4A/current video code, `src/video/`, display evidence |
| B06 | Display modes, display transactions, geometry, and safe-area calibration | SEEDED | display-mode source, M4 display evidence, historical hardware qualification |
| B07 | Controller acquisition, pointer semantics, clicks, and logical actions | SEEDED | controller/input source, controller tests and diagnostics |
| B08 | Keyboard, on-screen keyboard, modifiers, and text interaction | SEEDED | OSK/input/UI source and interaction history |
| B09 | Menus, overlays, curtains, status presentation, and local UI flow | SEEDED | UI source, display-transition/UI evidence |
| B10 | Human-readable configuration, validation, persistence, and bindings | SEEDED | configuration source/modules, config tests and migration evidence |
| B11 | Manual refresh, recovery policy, reconnect mechanisms, and Pi management transactions | SEEDED | management/recovery source, rollback/recovery evidence |
| B12 | Diagnostics, runtime identity, telemetry, profiling, and reporting | SEEDED | `src/diagnostics/`, identity/profiling evidence |
| B13 | Pi-side VNC desktop, management/service, networking, and companion responsibilities | SEEDED | preserved Pi environment, service/config census, product intent |
| B14 | Build, deployment, test, evidence, and qualification mechanisms that constrain product development | SEEDED | `scripts/`, TestKit history, `evidence/`, development-system lessons |

## B01 — Startup and application lifecycle

**Maturity:** `SEEDED`

Audit pending.

Questions include:

- What is the exact startup order?
- Which initialization failures are fatal?
- When are display, network, RFB, input, configuration, and diagnostics brought
  online?
- What shutdown/exit behavior is required specifically because of PS2 hardware
  or PS2SDK behavior?
- Which startup state is product policy versus platform mechanism?

## B02 — PS2-to-Pi Ethernet and networking

**Maturity:** `SEEDED`

Audit pending.

Questions include:

- What network state must exist before RFB begins?
- Which behavior belongs to PS2 platform initialization versus RFB transport?
- What socket/link failures are detected explicitly?
- Which network recovery behavior is automatic and which is deliberately
  user-controlled?

## B03 — RFB session

**Maturity:** `SEEDED`

Audit pending.

The audit must distinguish protocol framing/session state from display,
recovery-policy, and Pi-management responsibilities.

## B04 — Framebuffer update processing

**Maturity:** `SEEDED`

Audit pending.

The audit must describe rectangle framing, encoding selection, Raw/Hextile
handling, CPU framebuffer writes, validity/staleness semantics, and malformed or
incomplete update behavior.

## B05 — GS/video presentation

**Maturity:** `SEEDED`

Audit pending.

The audit must separate the logical framebuffer produced by RFB decoding from
PS2-specific GS/DMA/presentation mechanisms.

## B06 — Display modes and calibration

**Maturity:** `SEEDED`

Audit pending.

This domain includes mode selection, mode persistence, transactions,
confirmation/rollback behavior, logical versus presented geometry, safe-area
calibration, and hardware-specific invariants.

## B07 — Controller and pointer behavior

**Maturity:** `SEEDED`

Audit pending.

This domain includes controller hardware acquisition, logical button actions,
pointer movement, click semantics, pause/ack behavior, chords, and configurable
bindings.

## B08 — Keyboard and on-screen keyboard

**Maturity:** `SEEDED`

Audit pending.

This domain includes keyboard event generation, OSK navigation, modifier state,
and interaction between local UI controls and remote keyboard input.

## B09 — Local UI

**Maturity:** `SEEDED`

Audit pending.

This domain includes menus, overlays, transition curtains, current-state
presentation, selection/focus behavior, and any UI state that exists
independently of the remote desktop.

## B10 — Configuration and persistence

**Maturity:** `SEEDED`

Audit pending.

The audit must identify the human-readable configuration contract, defaults,
validation, persistence boundaries, display-mode/calibration persistence,
controller bindings, and ownership of parsed versus live state.

## B11 — Recovery and management

**Maturity:** `SEEDED`

Audit pending.

The audit must distinguish:

- mechanism from policy;
- explicit socket/link error recovery from silent-stall policy;
- manual refresh from automatic recovery;
- PS2-local state from Pi-management transactions;
- display-mode rollback/recovery from RFB-session recovery.

## B12 — Diagnostics and identity

**Maturity:** `SEEDED`

Audit pending.

This domain includes debug stages, low-overhead telemetry, profiling,
deterministic identity/report serialization, build/runtime provenance, and
evidence-facing state.

## B13 — Raspberry Pi companion

**Maturity:** `SEEDED`

Audit pending.

This domain must identify which Pi responsibilities are ordinary operating
system responsibilities and which genuinely belong to PS-to-VNC.

Areas include VNC desktop/service lifecycle, networking, management endpoints,
recovery, Samba/file access where product-significant, and future installer
state.

## B14 — Product-significant development infrastructure

**Maturity:** `SEEDED`

Audit pending.

Build, deployment, TestKit, identity, evidence capture, and hardware
qualification are not runtime product features, but some of their constraints
are part of the product's engineering definition and must survive the clean
reconstruction.

## Cross-domain rules already established

These are inputs to the audit, not conclusions about final module layout:

- evidence outranks intuition;
- historical source remains reference authority;
- migration-era module boundaries are not automatically retained;
- RFB protocol and PS2 graphics presentation are different responsibilities;
- policy and mechanism should be distinguishable;
- important mutable state requires explicit ownership;
- silent-freeze recovery policy must not be invented merely to make testing
  convenient;
- hardware-facing conclusions ultimately require physical PS2 qualification.

## Next audit action

Populate B01 through B14 from preserved source and evidence.

Begin with B01–B04 because lifecycle, networking, RFB session state, and
framebuffer processing establish the minimum path on which later behavior
depends.
