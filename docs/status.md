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

A new M4 source generation is active as a hardware-pending checkpoint.

Its source/build authority is established, but it does not replace the
previously validated runtime until independent machine and physical
hardware qualification passes.

Permanent ownership:

    src/diagnostics/debug.c
    src/diagnostics/debug.h

Pristine reproducible candidate:

    ELF_SHA256=4d85cfe6fca18ff0d6e23b681354df8121e9ccc5cc104e48ca51ed110751f5e5
    PT_LOAD_SHA256=1c330db6c1a16974e7d7907fd57fbfa55f48d5131634be37f4efcaf310a88491
    PT_LOAD_BYTES=449928

Exact identity-stamped hardware DUT:

    TEST_ID=M4I-C2-HW1
    ELF_IDENTITY_SHA256=28dbe5d985cfcbdcc2721e10e8b816fcfe37eb1e20e0b4d0e97c217299c515b8
    STAMPED_ELF_SHA256=035bd9a3adb631984b4bbabe8f058d2ae3b4f6a078cbf64003a7add0d5099067
    STAMPED_PT_LOAD_SHA256=ba4e7a7b6139cca2e7ba81772ad731b18b3ff5dea4ec6e9058d83641928b561b

Previous validated ELF remains:

    LAST_VALIDATED_WORKING_ELF_SHA256=6491424f81fe46c630863dcf13e2d4575c5a3925f5d5e704d6fdb8e47a8db3ac

    NEXT_ACTION=M4I-C2_HW1_direct_hardware_qualification
    BLOCKED_BY=M4I_C2_NONIDENTICAL_PT_LOAD_REQUIRES_HARDWARE
    CHECKPOINT_ID=M4I-C2-HW1

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
    CURRENT_WORKING_ELF_SHA256=4d85cfe6fca18ff0d6e23b681354df8121e9ccc5cc104e48ca51ed110751f5e5
    CURRENT_WORKING_VALIDATION_BASIS=PENDING_DIRECT_HARDWARE_QUALIFICATION
    NEXT_ACTION=M4I-C2_HW1_direct_hardware_qualification
    BLOCKED_BY=M4I_C2_NONIDENTICAL_PT_LOAD_REQUIRES_HARDWARE

Machine-readable authority remains under `runtime/`.
