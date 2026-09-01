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

1. build the complete semantic behavioral inventory;
2. map B4A/current implementation and mutable state to product responsibilities;
3. mine historical tests for durable lessons and proven mechanisms;
4. establish the clean Raspberry Pi baseline and dependency/reproducibility
   ledger in parallel;
5. derive the clean target architecture from audited behavior;
6. begin bottom-up reconstruction from the minimal PS2/Pi Ethernet and RFB
   path.

The semantic audit must describe purpose, inputs/triggers, owned state, normal
behavior, outputs/effects, dependencies, failure behavior, invariants,
implementation references, historical lessons, and rebuild implications for
each meaningful component.

### Behavioral-audit checkpoint

B01 through B06 have completed their source-plus-historical-evidence audit
tranches and are recorded as `EVIDENCE_SUPPORTED` in:

    docs/audit/BEHAVIORAL_INVENTORY.md

Covered so far:

- B01 startup/application lifecycle;
- B02 PS2-to-Pi Ethernet/networking;
- B03 RFB session/transport state;
- B04 framebuffer/update processing;
- B05 GS/video presentation and PS2 interrupt discipline;
- B06 display modes, transactions, geometry, calibration, rollback, and
  persistence.

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

The next grouped audit is B07/B08/B09: controller/pointer behavior,
keyboard/OSK behavior, and local menus/overlays. These share historical input
ownership/quarantine machinery while remaining separate product responsibilities.

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
    SEMANTIC_AUDIT=22_PERCENT
    CLEAN_PS2_RECONSTRUCTION=0_PERCENT
    PI_REPRODUCIBILITY_PACKAGE=0_PERCENT
    GITHUB_RECONCILIATION=100_PERCENT

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

    NEXT_ACTION=AUDIT_B07_B08_B09_INPUT_KEYBOARD_AND_LOCAL_UI
    BLOCKED_BY=NONE

Current machine-readable project authority is `runtime/PROJECT_STATE.env`.

Migration-era authority remains byte-preserved under
`runtime/MIGRATION_STATE.env` and `runtime/M4_SOURCE_AUTHORITY.env` as
historical/reference authority for the exploratory generation.
