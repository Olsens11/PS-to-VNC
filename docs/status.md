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

A new M4 source generation is active as a hardware-pending checkpoint.

Its source/build authority is established, but it does not replace the
previously validated runtime until independent machine and physical
hardware qualification passes.

Permanent ownership:

    src/diagnostics/identity.c
    src/diagnostics/identity.h

Pristine reproducible candidate:

    ELF_SHA256=144f668da0a3756ba4bd8f2fbfc3826983a1df6f5ea4ed0c44d2a1b0f45b2d93
    PT_LOAD_SHA256=c0030d28dea593b056bc604f94a9bd68576ccfc0f79e688c694193045378e701
    PT_LOAD_BYTES=450440

Exact identity-stamped hardware DUT:

    TEST_ID=M4I-C2-HW2
    ELF_IDENTITY_SHA256=0366569620c55b9657523032173d64fdddb744547fdbbf96e3edc105bacdb033
    STAMPED_ELF_SHA256=d3b99c8c48d5b34057ee139650fd885cee998bd13f2b026279799512c865f912
    STAMPED_PT_LOAD_SHA256=bb9c6b3e2ae98dfae811c4ec42ca65d7e028716a27862a1d88ba24f357e1476f

Previous validated ELF remains:

    LAST_VALIDATED_WORKING_ELF_SHA256=144f668da0a3756ba4bd8f2fbfc3826983a1df6f5ea4ed0c44d2a1b0f45b2d93

    NEXT_ACTION=M4I-C_debug_reporting_boundary_census
    BLOCKED_BY=NONE
    CHECKPOINT_ID=M4I-C2-HW2

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

    LAST_HARDWARE_RESULT=M4I_C2_PASS_MACHINE_AND_PHYSICAL
    LAST_VALIDATED_WORKING_ELF_SHA256=144f668da0a3756ba4bd8f2fbfc3826983a1df6f5ea4ed0c44d2a1b0f45b2d93
    LAST_VALIDATED_PT_LOAD_SHA256=c0030d28dea593b056bc604f94a9bd68576ccfc0f79e688c694193045378e701
    CURRENT_WORKING_ELF_SHA256=144f668da0a3756ba4bd8f2fbfc3826983a1df6f5ea4ed0c44d2a1b0f45b2d93
    CURRENT_WORKING_VALIDATION_BASIS=PENDING_DIRECT_HARDWARE_QUALIFICATION
    NEXT_ACTION=M4I-C_debug_reporting_boundary_census
    BLOCKED_BY=NONE

Machine-readable authority remains under `runtime/`.
