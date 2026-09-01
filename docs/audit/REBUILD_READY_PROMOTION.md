# PS-to-VNC Semantic Audit REBUILD_READY Promotion

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_1
    PROMOTION_VERSION=1
    BEHAVIOR_FAMILIES=B01_B14
    PREVIOUS_MATURITY=EVIDENCE_SUPPORTED
    CURRENT_MATURITY=REBUILD_READY
    SEMANTIC_AUDIT=COMPLETE

This document is the current maturity-promotion authority for the B01-B14
behavioral inventory.

`BEHAVIORAL_INVENTORY.md` remains the detailed evidence-harvest ledger and
therefore preserves the `EVIDENCE_SUPPORTED` checkpoint at which its detailed
entries were written. This promotion is additive rather than a cosmetic rewrite
of that historical audit record.

The promotion is justified by the completed cross-domain synthesis:

- `SOURCE_RESPONSIBILITY_MAP.md` maps historical source/state/coupling to product
  responsibilities;
- `HISTORICAL_LESSONS.md` records durable experimental lessons and preserved
  uncertainty;
- `CROSS_DOMAIN_SYNTHESIS.md` derives the smallest coherent responsibility map;
- `CROSS_DOMAIN_STATE_INTERFACES.md` assigns mutable-state ownership, interfaces,
  concurrency seams, and verifies startup/live/display-transition/recovery
  dependency flows;
- `../CLEAN_ARCHITECTURE.md` promotes the resulting architecture as current
  clean-reconstruction authority.

## Promotion table

| ID | Behavior family | Current maturity |
|---|---|---|
| B01 | Startup and application lifecycle | REBUILD_READY |
| B02 | PS2-to-Pi Ethernet and networking | REBUILD_READY |
| B03 | RFB connection, handshake, negotiation, and session state | REBUILD_READY |
| B04 | Framebuffer updates, rectangles, Raw/Hextile decode, and framebuffer validity | REBUILD_READY |
| B05 | GS/video presentation and framebuffer-to-display transfer | REBUILD_READY |
| B06 | Display modes, transactions, geometry, and safe-area calibration | REBUILD_READY |
| B07 | Controller acquisition, pointer semantics, clicks, and logical actions | REBUILD_READY |
| B08 | Keyboard, on-screen keyboard, modifiers, and text interaction | REBUILD_READY |
| B09 | Menus, overlays, curtains, status presentation, and local UI flow | REBUILD_READY |
| B10 | Human-readable configuration, validation, persistence, and bindings | REBUILD_READY |
| B11 | Manual Refresh, recovery policy, reconnect mechanisms, and Pi management transactions | REBUILD_READY |
| B12 | Diagnostics, runtime identity, telemetry, profiling, and reporting | REBUILD_READY |
| B13 | Pi-side VNC desktop, management/service, networking, and companion responsibilities | REBUILD_READY |
| B14 | Build, deployment, test, evidence, and qualification mechanisms that constrain product development | REBUILD_READY |

## Meaning of this promotion

`REBUILD_READY` means the project can design and implement the clean replacement
without rediscovering the purpose, ownership, failure behavior, invariants, or
historical reason for these behaviors.

It does **not** mean:

- the old B4A/M4 file structure is accepted;
- every exact future C file boundary is frozen;
- every Pi package choice is already adopted;
- every deferred optimization is required in the first milestone;
- new hardware evidence may never refine the architecture.

The architecture deliberately remains malleable while preserving the audited
behavior and qualification rules.

## Resolved synthesis questions

The prior open architecture questions are resolved sufficiently for
reconstruction:

1. **Cross-thread control:** use explicit semantic input queue, urgent control
   mailbox, and separate libpad ownership handshake; no giant shared app state.
2. **RFB responsiveness:** retain exact-length parsing with main-thread
   nonblocking refill after input becomes active; benign work yields only at a
   complete message boundary before partial consumption, while hard intent makes
   the stream suspect and triggers owning policy.
3. **Display/file granularity:** ownership is stable; initial implementation may
   keep pure display model/geometry cohesive and split hardware presentation only
   where clarity/hardware invariants earn it.
4. **UI granularity:** begin with one explicit main-owned UI/foreground model and
   first-class local dirty/wake state; split OSK/menus only when implementation
   size/testing earns it.
5. **Initial framebuffer path:** use one conventional authoritative EE desktop
   framebuffer; direct-write/high-resolution fast paths are deferred performance
   work.
6. **Pi packages:** exact clean-OS package adoption remains Issue #5
   implementation/dependency-ledger work, not missing behavior knowledge.
7. **TestKit:** first clean hardware milestones continue through the proven
   immutable successor-to-legacy TestKit bridge; a replacement is not a rebuild
   prerequisite.

## Audit exit condition

The semantic-audit exit condition from GitHub Issue #1 is satisfied:

> a clean target architecture can now be derived from documented product
> behavior without rediscovering why the old implementation worked.

The next phase is implementation and companion reproducibility, principally
GitHub Issues #5 and #7, with qualification continuing under B14 rules.
