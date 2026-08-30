# PS-to-VNC Migration State

This document is the human-readable companion to:

    runtime/MIGRATION_STATE.env

## Last completed stage

M0

M0 is the last completed migration stage.

It established the first writable PS-to-VNC source/build authority and
independently reproduced the exact historical B4A executable bytes.

M0 completion authority is recorded in:

    runtime/M0_BUILD_AUTHORITY.env

## Current stage

M1 — NOT STARTED

M1 is the first mechanical modular-extraction stage.

M1A is read-only with respect to implementation source and will select the
first exact low-coupling extraction boundary from the committed dependency
map before any M1 source mutation occurs.

Governing contract:

    docs/M1_EXTRACTION_CONTRACT.md

## Current working source authority

working/b4a/ps2ip.c

This is the completed M0 writable source authority.

M0 source-authority commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

At M1 entry no M1 implementation mutation has occurred.

The historical frozen reference remains:

    baseline/frozen-b4a/

The frozen reference must not be edited.

## Frozen behavioral authority

Baseline DUT:

    D17AL-F8J2-B4A

Frozen source:

    baseline/frozen-b4a/ps2ip.c

Frozen source SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

Known tested ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Frozen dependency analysis:

    baseline/analysis/B4A-dependency-map/

Dependency-analysis SHA256SUMS manifest SHA256:

    075d68d9992332a32bdca54a32964363fb893071fdcc5c9e2dfc7fb26a2e1749

## Legacy repository boundary

Historical PS2VNC repository:

    /home/ps2/ps2vnc

That repository is read-only historical authority for successor development.

All new work belongs in:

    /home/ps2/PS-to-VNC

## Bootstrap preservation

Before the recovery-state machinery was added, the exact staged successor tree
was preserved as Git tree:

    2f7f3dcae878251bda1ad8169912df035ca34020

It was also preserved as:

    /home/ps2/ps2vnc-legacy-backups/PS-to-VNC-pre-recovery-staged-20260830-003258.tar

Archive SHA256:

    a8426ff95e82b06fe1dc9056d7dabeeca91b3bccb5d2711ac3d2d8c59ecd5251

## Current build state

M0 build result:

    OUTCOME_A_BYTE_EXACT_ELF

Validated working ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

The M0-generated executable is byte-identical to the historically validated
B4A ELF.

No M1 build exists yet because M1 source mutation has not begun.

## Current documentation state

The successor documentation and recovery system are committed and active.

M0 source, build, dependency, reproduction, evidence, and hardware-resolution
authority are recorded inside PS-to-VNC.

Migration recoverability remains a definition-of-done requirement for every
subsequent modularization stage.

## Next action

M1A:

Select and document the first exact low-coupling mechanical extraction
boundary using the committed B4A dependency analysis.

M1A must not modify implementation source.

Do not begin M1B source extraction until the M1A boundary is documented,
validated, and committed.

## Conversation-limit recovery

A fresh conversation should begin with:

    START_HERE.md

Then run:

    scripts/resume-state.sh
    scripts/migration-check.sh
    scripts/docs-check.sh

The repository, not conversational memory, determines where development
resumes.

## Machine-state mirror

The following exact indented tokens mirror the stable migration fields in the
canonical machine-readable state.

They exist so consistency tooling can verify the human document without
parsing prose.

    LAST_COMPLETE_STAGE=M0
    CURRENT_STAGE=M1
    CURRENT_STAGE_STATUS=NOT_STARTED
    CURRENT_WORKING_SOURCE=working/b4a/ps2ip.c
    CURRENT_SOURCE_HEAD=d1c0d6a4829c03f3a062095afd00859188e13dfe
    NEXT_ACTION=M1A_select_first_low_coupling_leaf_extraction_boundary

The canonical machine-readable source remains:

    runtime/MIGRATION_STATE.env

## M0B checkpoint

The M0 reproduction contract is now defined at:

    docs/M0_BASELINE_CONTRACT.md

M0 has formally entered IN_PROGRESS state.

No writable source has been created yet.

Next operation:

    M0C_reconstruct_and_fingerprint_historical_build_environment

## M0C checkpoint

M0C historical build-environment reconstruction is COMPLETE.

Canonical human build provenance:

    docs/M0_BUILD_PROVENANCE.md

Canonical machine-readable build authority:

    runtime/M0_BUILD_AUTHORITY.env

The B4A libps2ip authority is the frozen archive with SHA256:

    b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

The different legacy-root archive with SHA256:

    f16bd80321657a854f09d96622a11308232c9053486d6158b78825e452b35860

is older historical residue and is not the B4A link input.

No writable working source exists yet.

Next operation:

    M0D_create_writable_B4A_authority_and_controlled_rebuild

## M0D1 checkpoint

The pristine writable B4A implementation has been established at:

    working/b4a/

The primary writable source is:

    working/b4a/ps2ip.c

At creation it is byte-identical to the frozen B4A source.

A controlled reproduction builder has been established at:

    scripts/build-m0-b4a.sh

M0D1 does not execute the compiler.

The next operation after this checkpoint is committed is the first controlled
M0 reproduction build.

## M0D1 committed authority

Writable-source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

Machine state now recognizes:

    CURRENT_WORKING_SOURCE=working/b4a/ps2ip.c
    CURRENT_SOURCE_HEAD=d1c0d6a4829c03f3a062095afd00859188e13dfe
    NEXT_ACTION=M0D2_run_controlled_B4A_reproduction_build

## M0E checkpoint

M0E reproduction classification is COMPLETE.

Classification:

    OUTCOME_A_BYTE_EXACT_ELF

The independently generated working ELF is byte-identical to the historical
and frozen B4A ELF:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

The earlier M0D2 outer-harness exit 60 was caused only by an over-strict
byte-hash requirement on the GCC slim-LTO `ps2vnc_gsHires.o` intermediate.

M0D2A established that its eighteen LTO payloads are byte-identical and only
the generated LTO section-name suffix, `.strtab`, and `.shstrtab` vary.

Canonical result:

    docs/M0_BUILD_RESULT.md

Committed evidence:

    evidence/m0/m0d2/

Next operation:

    M0F_resolve_hardware_validation_requirement_for_byte_exact_DUT

## M0F checkpoint

M0F hardware-validation requirement resolution is COMPLETE.

New hardware regression required:

    NO

New M0 hardware run performed:

    NO

Reason:

    the M0 reproduced ELF is byte-identical to the historically validated B4A
    ELF, so no new DUT identity exists.

Canonical resolution:

    docs/M0_HARDWARE_RESOLUTION.md

Next operation:

    M0G_close_M0_and_define_M1
## M0G checkpoint

M0 is COMPLETE.

Completed M0 source authority:

    working/b4a/ps2ip.c

Completed M0 source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

Completed M0 build result:

    OUTCOME_A_BYTE_EXACT_ELF

Completed M0 executable SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Completed M0 hardware resolution:

    no new M0 hardware run was required because the reproduced executable is
    byte-identical to the historically validated B4A DUT.

This is inherited validation for identical DUT bytes, not a new physical
hardware PASS.

Current stage:

    M1 — NOT STARTED

M1 governing contract:

    docs/M1_EXTRACTION_CONTRACT.md

Next operation:

    M1A_select_first_low_coupling_leaf_extraction_boundary
