# Current Project Status

This is the concise human-readable current-state authority.

Chronological migration history remains in `docs/MIGRATION_STATE.md` and Git.

## Product

    version=0.1.0-alpha.1

## Last completed normalization boundary

M4H-A safe-area / presentation geometry is hardware qualified.

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

    NEXT_ACTION=M4H-B_video_geometry_residual_boundary_census
    BLOCKED_BY=NONE

Next product work is the residual video-geometry boundary census/audit.

## Development continuity

Development-continuity baseline version 1 is active.

    SESSION_BOOTSTRAP=AGENTS.md
    DOCS_ROUTER=docs/README.md
    DEVELOPMENT_POLICY=docs/development/README.md

Routine procedures must reuse canonical saved tooling when available.

## Planning estimate

    OVERALL_ARCHITECTURAL_NORMALIZATION=approximately_24_percent
    VIDEO_GEOMETRY=approximately_80_percent

These percentages are planning estimates, not machine authority.

## Machine-state mirror

    LAST_HARDWARE_RESULT=M4H_PASS_MACHINE_AND_PHYSICAL
    LAST_VALIDATED_WORKING_ELF_SHA256=42163c30b68ebf84b51b1c8c81d541017e64ffaa52e27bb0408f91edb25e2992
    LAST_VALIDATED_PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9
    NEXT_ACTION=M4H-B_video_geometry_residual_boundary_census
    BLOCKED_BY=NONE

Machine-readable authority remains under `runtime/`.
