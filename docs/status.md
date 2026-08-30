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
    PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9

## Current development focus

Tranche 3 diagnostics is active.

M4I-B1 identity normalization is complete. The active build/runtime identity
transport now lives under:

    src/diagnostics/identity.c
    src/diagnostics/identity.h

Its reproducible PT_LOAD image is byte-for-byte identical to M4H, so the
existing M4H hardware qualification remains applicable to the runtime image.

    NEXT_ACTION=M4I-C_debug_reporting_boundary_census
    BLOCKED_BY=NONE

Next product work is the permanent debug-reporting ownership boundary.

## Development continuity

Development-continuity baseline version 1 is active.

    SESSION_BOOTSTRAP=AGENTS.md
    DOCS_ROUTER=docs/README.md
    DEVELOPMENT_POLICY=docs/development/README.md

Routine procedures must reuse canonical saved tooling when available.

## Planning estimate

    OVERALL_ARCHITECTURAL_NORMALIZATION=approximately_31_percent
    DIAGNOSTICS_TRANCHE=approximately_40_percent
    VIDEO_GEOMETRY=complete
    DIAGNOSTICS_IDENTITY=complete

These percentages are planning estimates, not machine authority.

## Machine-state mirror

    LAST_HARDWARE_RESULT=M4H_PASS_MACHINE_AND_PHYSICAL
    LAST_VALIDATED_WORKING_ELF_SHA256=6491424f81fe46c630863dcf13e2d4575c5a3925f5d5e704d6fdb8e47a8db3ac
    LAST_VALIDATED_PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9
    CURRENT_WORKING_ELF_SHA256=6491424f81fe46c630863dcf13e2d4575c5a3925f5d5e704d6fdb8e47a8db3ac
    CURRENT_WORKING_VALIDATION_BASIS=INHERITED_M4H_PT_LOAD_BYTE_EXACT
    NEXT_ACTION=M4I-C_debug_reporting_boundary_census
    BLOCKED_BY=NONE

Machine-readable authority remains under `runtime/`.
