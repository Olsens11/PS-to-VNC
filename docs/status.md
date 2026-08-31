# Current Project Status

This is the concise human-readable current-state authority.

Chronological migration history remains in `docs/MIGRATION_STATE.md` and Git.

## Product

    version=0.1.0-alpha.1

## Last completed normalization boundary

Normalization Tranche 2 — video mode and pure geometry — is complete.

M4H-A safe-area / presentation geometry is hardware qualified.

M4H-B residual-boundary review is complete. The valid self-tested third
census, followed by manual boundary confirmation, found no additional coherent
pure-geometry responsibility to extract.

    PURE_VIDEO_GEOMETRY_MODEL=COMPLETE
    ADDITIONAL_PURE_GEOMETRY_EXTRACTION=NO

Residual calibration state/policy, framebuffer/raster mapping, GS
presentation, UI/diagnostics, and display orchestration remain assigned to
their later canonical owners.

Permanent module:

    src/video/geometry.c
    src/video/geometry.h

Qualification included:

- host unit tests;
- two byte-reproducible PS2 builds;
- exact ELF/PT_LOAD identity;
- controlled nontrivial safe-area hardware validation;
- signed center-relative offset validation;
- byte-exact configuration restoration;
- normal 480p restoration;
- five-mode machine regression PASS 5/5;
- operator physical result FULL PASS;
- final display state IDLE / 480p.

## Validated executable

    ELF_SHA256=42163c30b68ebf84b51b1c8c81d541017e64ffaa52e27bb0408f91edb25e2992
    PT_LOAD_SHA256=c0030d28dea593b056bc604f94a9bd68576ccfc0f79e688c694193045378e701

## Current development focus

M4I diagnostics normalization is COMPLETE.

Permanent diagnostics ownership now includes identity, debug state /
transport, and DBG / PRF / GEOM reporting serialization.

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

    NEXT_ACTION=M4J_management_normalization
    BLOCKED_BY=NONE
    CHECKPOINT_ID=M4I-FINAL-HW1

## Development continuity

Development-continuity baseline version 1 is active.

    SESSION_BOOTSTRAP=AGENTS.md
    DOCS_ROUTER=docs/README.md
    DEVELOPMENT_POLICY=docs/development/README.md

Routine procedures must reuse canonical saved tooling when available.

## Planning estimate

    OVERALL_ARCHITECTURAL_NORMALIZATION=approximately_36_percent
    DIAGNOSTICS_TRANCHE=complete_100_percent
    VIDEO_GEOMETRY=complete
    DIAGNOSTICS_IDENTITY=complete
    DIAGNOSTICS_DEBUG_STATE_TRANSPORT=complete
    DIAGNOSTICS_REPORTING=complete

These percentages are planning estimates, not machine authority.

## Machine-state mirror

    LAST_HARDWARE_RESULT=M4I_FINAL_PASS_MACHINE_AND_PHYSICAL
    LAST_VALIDATED_WORKING_ELF_SHA256=62e9b180ba73fc1377f853cfe21c17449be958723be6e7a2142de02dd1cee75e
    LAST_VALIDATED_PT_LOAD_SHA256=fbb9a68913a10468827e72ef1db5c912a475fd357179f5f2cb993f3491d145db
    CURRENT_WORKING_ELF_SHA256=62e9b180ba73fc1377f853cfe21c17449be958723be6e7a2142de02dd1cee75e
    CURRENT_WORKING_VALIDATION_BASIS=M4I_FINAL_DIRECT_HARDWARE_QUALIFICATION
    NEXT_ACTION=M4J_management_normalization
    BLOCKED_BY=NONE

Machine-readable authority remains under `runtime/`.
