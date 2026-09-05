# Current Project Status

This is the concise human-readable current-state authority.

Chronological migration/exploratory history remains in `docs/MIGRATION_STATE.md`,
preserved evidence, and Git.

## Product

    version=0.1.0-alpha.1

## Current phase

    PHASE=PRESERVE_CLEAN_UNDERSTAND_RECONSTRUCT
    REFERENCE_PRESERVATION=COMPLETE
    SEMANTIC_AUDIT=COMPLETE
    CLEAN_ARCHITECTURE=REBUILD_READY
    CLEAN_PS2_RECONSTRUCTION=ISSUE39_STAGE2_COMPLETE
    PI_REPRODUCIBILITY_PACKAGE=PRE_ISSUE40_MINIMUM_DESKTOP_PENDING

The reconstruction principle remains:

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

Frozen B4A/current code and migration-era slices remain behavioral/reference
authority and sources of proven mechanisms. They are not the structural
blueprint for the new product.

## Current architecture authority

The clean reconstruction architecture is now:

    docs/CLEAN_ARCHITECTURE.md

It was derived from the completed B01-B14 semantic audit rather than from the
M4 file hierarchy.

For new clean-reconstruction work, `CLEAN_ARCHITECTURE.md` supersedes the
normalization-era target documents as current design authority. The older
`ARCHITECTURE.md`, `TARGET_ARCHITECTURE.md`, `STATE_OWNERSHIP.md`,
`DEPENDENCY_RULES.md`, `NORMALIZATION_SEQUENCE.md`, and
`ARCHITECTURAL_NORMALIZATION.md` remain historical/reference material.

The architecture is responsibility-oriented and deliberately small. Its stable
contract is ownership/dependency direction, not an exact permanent count of `.c`
files.

## Semantic audit completion

B01 through B14 have completed:

- behavioral/source/evidence audit;
- historical source/state/coupling mapping;
- historical experiment/lesson mining;
- cross-domain state ownership;
- interface/dependency synthesis;
- startup/live/display-transition/recovery flow verification;
- current clean-architecture derivation.

Current maturity authority:

    docs/audit/REBUILD_READY_PROMOTION.md

All B01-B14 behavior families are now:

    REBUILD_READY

`docs/audit/BEHAVIORAL_INVENTORY.md` remains the detailed evidence-harvest ledger
and intentionally preserves the earlier `EVIDENCE_SUPPORTED` checkpoint inside
its long-form entries. The promotion record is additive rather than a cosmetic
rewrite of that historical audit record.

Durable audit outputs:

    docs/audit/BEHAVIORAL_INVENTORY.md
    docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md
    docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md
    docs/audit/B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md
    docs/audit/SOURCE_RESPONSIBILITY_MAP.md
    docs/audit/HISTORICAL_LESSONS.md
    docs/audit/CROSS_DOMAIN_SYNTHESIS.md
    docs/audit/CROSS_DOMAIN_STATE_INTERFACES.md
    docs/audit/REBUILD_READY_PROMOTION.md

GitHub Issues #1, #2, #3, #4, and #6 closed as completed when PR #12 promoted the
semantic audit and clean architecture to `main`.

## Core clean-architecture conclusions

The initial PS2 design has explicit owners for:

- application orchestration/policy;
- one RFB session/stream;
- one authoritative CPU-side desktop framebuffer;
- display model/profile and PS2 presentation;
- controller/input semantics;
- local UI/OSK;
- typed human configuration;
- the small Pi management client;
- diagnostics/runtime identity;
- genuinely PS2-specific platform mechanisms.

Important cross-domain rules include:

- main/application remains the sole VNC socket owner;
- controller publishes typed semantic input rather than encoded RFB packets;
- ordinary semantic input, urgent control intent, and libpad ownership handoff
  are three separate cross-thread seams;
- RFB may keep straightforward exact-length parsing;
- after input becomes active, nonblocking RFB refill permits benign application
  yield only at a complete server-message boundary before partial consumption;
- Refresh/rollback/exit may hard-interrupt a partial receive only by treating the
  old stream as suspect and replacing it;
- RFB decoding, CPU framebuffer validity, and GS/presentation validity are
  separate concepts;
- complete `display_profile` values cross config/application/display/RFB/
  management boundaries rather than component globals;
- display transitions remain application-owned cross-machine transactions;
- local UI has a first-class local dirty/wake path and does not depend on remote
  pointer-jiggle damage;
- human desired configuration, accepted live state, and crash-safe Pi
  transaction authority remain separate;
- explicit link/socket/RFB failures may recover automatically, while unexplained
  silent stalls remain manual under the current debugging policy;
- diagnostics observes product state without owning it;
- changed PT_LOAD remains hardware-gated unless explicit qualification-transfer
  authority exists.

## Historical mechanisms adopted/deferred

Adopted or adapted into clean ownership include the qualified PS2IP/private-link
foundation, exact RFB framing, one-thread socket ownership, authoritative full
frame before normal incremental operation, Test14 HIRES interrupt discipline,
complete display-profile rollback, libpad handoff, input quarantine, one-shot
OSK modifiers, strict config validation, manual Refresh cooldown, durable Pi
transaction reconciliation, deterministic runtime identity, endpoint-based Pi
health, and exact DUT/PT_LOAD/apparatus/evidence qualification.

Deferred from the first clean milestone include direct-to-presentation paths that
can stale the conventional framebuffer, advanced dirty/Hextile/presentation
optimization, MPEG/hybrid video, nonessential exploratory display modes, and
replacement of mature TestKit merely for aesthetic consistency.

Issue #7 hardware qualification now uses successor-owned deployment, UDP-stage
observation, PS2-facing packet capture, run ownership, evidence sealing, and
result tooling. The former successor-to-legacy TestKit bridge is retired; the
historical TestKit remains evidence/reference authority rather than a live
dependency.

## Clean reconstruction checkpoint — 2026-09-05

Issue #39 is closed as completed. The clean PS2 reconstruction now includes the
qualified minimal core, Stage-1 controller/pointer semantics, and Stage-2
keyboard/OSK/local-foreground foundation.

    CURRENT_RECONSTRUCTION_STAGE=POST_ISSUE39_PRE_ISSUE40_PI_DESKTOP_PREREQUISITE
    ISSUE39_STATUS=CLOSED_COMPLETED
    ISSUE39_CLOSEOUT_HEAD=5d67c60fc273500f7f3b7082e2c2f7b9b7ce389c
    ISSUE39_TOPOLOGY_COMMIT=0b3e20c7c4840073618a1bb487d554bc3309d057
    SOURCE_TOPOLOGY_ADOPTED_DATE=2026-09-05
    SOURCE_TOPOLOGY_POLICY=docs/development/source-topology.md
    SOURCE_TOPOLOGY_RECORD=docs/reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md

The domain-directory reorganization occurred during Issue #39 closeout on
2026-09-05. It is not retroactively attributed to Issue #7, Stage #38, the
semantic audit, or the earlier migration work.

The topology qualification build reproduced twice at one fixed build path and
retained the exact HW2-qualified loadable payload:

    ISSUE39_HW2_PT_LOAD_SHA256=512f5220c2be2c001c8641975c2045b3eb2319b4d384a0a311906e012944b85b
    ISSUE39_HW2_PT_LOAD_BYTES=403080

The live dictionary counts are generated from directory-owned `SYMBOLS.md`
files and are not duplicated as permanent machine-state invariants. The
2026-09-05 adoption snapshot contained 1,595 product symbol entries.

Before Issue #40 begins, the project has a deliberate Pi-side prerequisite:
establish the minimum reproducible terminal-capable VNC desktop target. The
previous ad-hoc `:1` desktop remains a qualification fixture rather than clean
companion authority.

## Pi companion boundary

The Pi behavior is `REBUILD_READY`, but the reproducible companion environment
needed by later management/config work is not yet established.

The clean companion remains a normal supported Raspberry Pi OS plus narrowly
product-specific state/services:

- private PS2 Ethernet;
- predictable dedicated VNC desktop;
- small management/persistence service;
- endpoint-based health/recovery;
- operator controls;
- versioned installer/package definition and dependency ledger.

Historical TigerVNC/Openbox/lxpanel and VNC-only pacing are reference candidates
to classify as `EVALUATING`/`ADOPTED_*` through the clean ledger rather than
being copied as unexplained machine folklore.

## Preserved exploratory hardware authority

The final completed normalization boundary before the strategic restart remains
M4I diagnostics.

Final qualified source generation:

    SOURCE_COMMIT=d205e32637a4cb0e7295a267cc471b076e58791d
    SOURCE_AUTHORITY_COMMIT=c62c7f7ee9a57bac03dcc9d20285400c2b1e2bc8
    ELF_SHA256=62e9b180ba73fc1377f853cfe21c17449be958723be6e7a2142de02dd1cee75e
    PT_LOAD_SHA256=fbb9a68913a10468827e72ef1db5c912a475fd357179f5f2cb993f3491d145db
    PT_LOAD_BYTES=450696

Final direct hardware authority:

    TEST_ID=M4I-FINAL-HW1
    HARDWARE_AUTHORITY_COMMIT=9ec0ddc6505f422dffd966b8be9c34aff97d6b38
    MACHINE_RESULT=PASS_5_OF_5
    PHYSICAL_RESULT=FULL_PASS
    FINAL_STARTUP_MODE=480p

These remain valid historical/empirical authorities. Former planned
`M4J_management_normalization` is not the current roadmap.

## Reconstruction progress

Current indicators:

    REFERENCE_PRESERVATION=100_PERCENT
    SEMANTIC_AUDIT=100_PERCENT
    CLEAN_PS2_RECONSTRUCTION=ISSUE39_STAGE2_COMPLETE
    PI_REPRODUCIBILITY_PACKAGE=PRE_ISSUE40_MINIMUM_DESKTOP_PENDING
    GITHUB_RECONCILIATION=100_PERCENT

The previous `OVERALL_ARCHITECTURAL_NORMALIZATION` percentage remains historical
and is not reconstruction progress.

PR #12 promoted the completed semantic audit and clean architecture to `main` at:

    SEMANTIC_AUDIT_PROMOTION_MAIN=a990bdc447f7f2073952b166b8cbd66e331f4e98

## Development continuity

    SESSION_BOOTSTRAP=AGENTS.md
    DOCS_ROUTER=docs/README.md
    DEVELOPMENT_POLICY=docs/development/README.md
    PROJECT_INTENT=docs/PROJECT_INTENT.md
    CLEAN_ARCHITECTURE=docs/CLEAN_ARCHITECTURE.md
    MACHINE_CURRENT_STATE=runtime/PROJECT_STATE.env
    MIGRATION_STATE_ROLE=HISTORICAL_REFERENCE

Repository authority remains more important than conversation memory.

## Frozen safety boundary

The historical PS2VNC repository and preserved legacy material remain
immutable reference authority. Frozen B4A and successor-era evidence must not
be destroyed or silently rewritten by clean reconstruction work.

Current tooling must not depend on a historical checkout being present at any
particular filesystem location.

## Next action

    NEXT_ACTION=ESTABLISH_MINIMUM_REPRODUCIBLE_TERMINAL_CAPABLE_PI_DESKTOP_BEFORE_ISSUE40
    BLOCKED_BY=NONE
    ISSUE40_BLOCKED_BY=MINIMUM_REPRODUCIBLE_PI_DESKTOP_TARGET_NOT_YET_ESTABLISHED

This prerequisite is intentionally smaller than the later companion-desktop
maturation work. Minimum success is a known reproducible VNC session with known
session/window-manager ownership, a visible launchable terminal, and a
deterministic PS2-only route to that terminal.

Issue #40 remains deliberately unopened as the active implementation step until
that target exists.

Current machine-readable authority is `runtime/PROJECT_STATE.env`.
