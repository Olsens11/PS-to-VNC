# Current Project Status

This is the concise human-readable current-state authority.

Chronological migration and exploratory-normalization history remains in
`docs/MIGRATION_STATE.md`, preserved evidence, and Git.

## Product

    version=0.1.0-alpha.1

## Strategic transition

The exploratory implementation and successor normalization work have been
preserved as reference authority.

The project is now entering a clean reconstruction phase governed by:

    docs/PROJECT_INTENT.md

The reconstruction principle is:

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

Existing B4A/current code remains behavioral reference, evidence, and a source
of proven mechanisms. It is not the structural blueprint for the new product.

The previous extraction-oriented normalization trajectory is historical from
this point forward unless a specific completed mechanism or lesson is
explicitly adopted into the clean design.

## Preserved exploratory authority

The final completed normalization boundary before the strategic restart was
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

These remain valid historical/empirical authorities. The former planned next
action `M4J_management_normalization` is no longer the active product roadmap.

## Current development focus

The active phase is:

    PHASE=PRESERVE_CLEAN_UNDERSTAND_RECONSTRUCT

Reference preservation is complete. The preserved exploratory environment is
now forensic/reference authority rather than unfinished reconstruction work.

Immediate work:

1. complete the semantic behavioral inventory and cross-domain synthesis;
2. reconcile B4A/current mutable state to explicit clean ownership;
3. derive subsystem interfaces and dependency direction from audited behavior;
4. establish the clean Raspberry Pi baseline and dependency/reproducibility
   ledger in parallel;
5. promote sufficiently reconciled behavior to `REBUILD_READY`;
6. begin bottom-up reconstruction from the minimal PS2/Pi Ethernet and RFB
   path.

The semantic audit must describe purpose, inputs/triggers, owned state, normal
behavior, outputs/effects, dependencies, failure behavior, invariants,
implementation references, historical lessons, and rebuild implications for
each meaningful component.

### Behavioral-audit checkpoint

B01 through B14 have completed their source-plus-historical-evidence audit
tranches and are recorded as `EVIDENCE_SUPPORTED` in:

    docs/audit/BEHAVIORAL_INVENTORY.md

Detailed grouped tranches are retained at:

    docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md
    docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md
    docs/audit/B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md

Covered behavior families:

- B01 startup/application lifecycle;
- B02 PS2-to-Pi Ethernet/networking;
- B03 RFB session/transport state;
- B04 framebuffer/update processing;
- B05 GS/video presentation and PS2 interrupt discipline;
- B06 display modes, transactions, geometry, calibration, rollback, and
  persistence;
- B07 controller acquisition, pointer/click/scroll semantics, hotkeys, and
  libpad ownership;
- B08 keyboard/RFB key events, OSK navigation, and one-shot modifiers;
- B09 local menu/overlay/curtain ownership, input quarantine, and foreground
  flow;
- B10 human-readable configuration, strict typed validation, persistence
  boundaries, display policy, and the controller-binding gap;
- B11 manual Refresh, explicit-failure recovery, silent-stall policy, RFB
  replacement, and semantic Pi management transactions;
- B12 diagnostics transport/stages, structured reports, deterministic runtime
  identity, profiling, and qualification-facing telemetry;
- B13 reproducible Pi companion networking, dedicated VNC desktop, management,
  service health, persistence, and runtime/operator controls;
- B14 reproducible build/dependency/DUT identity, deployment/TestKit apparatus,
  evidence integrity, PT_LOAD hardware gating, and qualification authority.

Important display conclusions include:

- RFB decoding and GS presentation are separate responsibilities;
- PS2SDK `ExitHandler()` is an interrupt-return mechanism, not application exit;
- physical raster, GS drawing geometry, RFB logical geometry, presented desktop,
  safe area, and startup-mode persistence are distinct state;
- selecting the active mode enters safe-area calibration rather than
  reinitializing that mode;
- risky mode changes are complete transactions with wall-clock confirmation,
  durable provisional/restore state, and complete-profile rollback;
- the principal five-mode matrix (480i, 480p-hires, 576i, 720p, 1080i) has
  direct machine and physical qualification.

Important input/UI conclusions include:

- controller/libpad handoff is explicit ownership, not merely a mutex around a
  read call;
- the controller acknowledgement is published immediately before the next
  libpad access, after which that owner performs no pad reads until release;
- the durable handoff rule is no pre-handoff physical/derived state leakage;
  calibration may enforce that with physical release, while the B4A
  hostile-stick remote transition enforces it by state invalidation;
- physical input ownership and already-queued logical RFB input are separate
  concerns;
- B4A drops pre-boundary queued controller actions before hazardous remote
  reconstruction and exposes one coherent destination frame before returning
  input;
- recovered Test11 is a proven historical foundation for pointer/keyboard/OSK
  behavior but is not authority to restore obsolete mappings;
- OSK Shift/Ctrl/Alt are deliberate one-shot modifiers;
- buttons consumed by a local UI remain quarantined until physical release;
- local UI foreground/underlay ownership must be explicit and local repaint must
  not depend on unrelated remote framebuffer damage.

Important configuration/recovery conclusions include:

- human-editable desired configuration, validated live state, and crash-safe
  transactional authority are different concepts;
- configuration parsing is side-effect free and recognized state publishes only
  after complete validation;
- startup mode, per-mode calibration, and display lock/hide policy are distinct
  persisted state;
- B4A has a typed semantic hotkey model but does not establish completed
  human-readable configurable bindings; that remains an explicit clean-product
  requirement;
- explicit carrier/socket/RFB failure recovery is supported automatically, but
  unexplained silent waits do not become a generic automatic reconnect timer;
- manual Refresh deliberately can break a suspect blocking/partial receive and
  then replaces that stream rather than guessing protocol alignment;
- successful manual Refresh has visible completion state plus a three-second
  anti-spam cooldown;
- an unconfirmed display candidate owns transport failure and rolls back rather
  than being legitimized by generic reconnect;
- sharing TCP 5959/HTTP does not make configuration, durable transactions, and
  development control one reliability class;
- local recovery success and durable cross-machine reconciliation are not
  silently treated as equivalent.

Important diagnostics/Pi/development conclusions now include:

- diagnostics are optional to ordinary product startup but may be mandatory
  evidence for a particular qualification manifest;
- exact runtime identity closes the DUT-to-telemetry loop, and ambiguous runtime
  identity fails qualification even when the desktop visibly runs;
- the M4I identity truncation was a real source/runtime defect, not a reason to
  weaken TestKit validation; deterministic bounded identity serialization fixed
  it and was requalified on hardware;
- the Pi companion should be a normal supported Raspberry Pi OS plus narrowly
  product-specific services, not a clone of the exploratory development
  machine;
- VNC desktop availability is independent of management-service availability;
- service health means actual listener/X usability rather than merely an active
  process;
- historical VNC-only traffic pacing is explicit runtime state to preserve and
  re-evaluate, not an invisible machine tweak or universally proven constant;
- build environment, significant dependency identity, ELF/PT_LOAD/runtime DUT
  identity, apparatus, and operator procedure are part of empirical meaning;
- a changed PT_LOAD crosses the hardware gate unless an explicit authority
  records a valid qualification transfer;
- machine evidence and physical/operator evidence remain independent;
- raw evidence is byte-preserved and invalid/ambiguous evidence fails closed;
- migration-specific machinery survives only when it expresses a recurring
  development invariant.

All seeded behavior families are therefore evidence-supported. The next phase is
not implementation yet: cross-domain synthesis must turn these audited facts into
explicit state ownership, interfaces, dependency direction, PS2/Pi boundaries,
and a deliberately small target architecture before `REBUILD_READY` promotion.

## Development continuity

Development-continuity baseline version 1 remains active.

    SESSION_BOOTSTRAP=AGENTS.md
    DOCS_ROUTER=docs/README.md
    DEVELOPMENT_POLICY=docs/development/README.md
    PROJECT_INTENT=docs/PROJECT_INTENT.md
    MACHINE_CURRENT_STATE=runtime/PROJECT_STATE.env
    MIGRATION_STATE_ROLE=HISTORICAL_REFERENCE

Repository authority remains more important than conversational memory.
Historical evidence must be preserved rather than rewritten to fit the new
architecture.

## Reconstruction progress

These are planning indicators, not machine authority:

    REFERENCE_PRESERVATION=100_PERCENT
    SEMANTIC_AUDIT=50_PERCENT
    CLEAN_PS2_RECONSTRUCTION=0_PERCENT
    PI_REPRODUCIBILITY_PACKAGE=0_PERCENT
    GITHUB_RECONCILIATION=100_PERCENT

The 50% semantic-audit checkpoint means all seeded behavior families have
source-plus-evidence support. It deliberately reserves the remaining audit work
for cross-domain ownership/interface synthesis and `REBUILD_READY` promotion.

The previous `OVERALL_ARCHITECTURAL_NORMALIZATION` percentage is historical and
is not carried forward as reconstruction progress.

## Repository / GitHub authority

The development Pi and GitHub are reconciled on the formal
clean-reconstruction baseline:

    RECONCILED_MAIN=434384e56c0acfc978922f62572da8df9b0372ec

PR #10 promoted the qualified clean-reconstruction intent, project-state
authority, continuity model, and semantic-audit direction to `main`.

The final exploratory successor head remains preserved separately as:

    EXPLORATORY_FINAL_SUCCESSOR_HEAD=d8ae5b13947f7ef3d250c0b18be967a10f8c76a8

## Frozen safety boundary

Historical repository:

    /home/ps2/ps2vnc

This remains immutable reference authority.

Frozen B4A and successor-era evidence remain reference material. No clean
reconstruction step should destroy or silently rewrite them.

## Next action

    NEXT_ACTION=SYNTHESIZE_CROSS_DOMAIN_OWNERSHIP_INTERFACES_AND_REBUILD_READY_ARCHITECTURE
    BLOCKED_BY=NONE

Current machine-readable project authority is `runtime/PROJECT_STATE.env`.

Migration-era authority remains byte-preserved under
`runtime/MIGRATION_STATE.env` and `runtime/M4_SOURCE_AUTHORITY.env` as
historical/reference authority for the exploratory generation.
