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

M1 — IN PROGRESS

M1A, the publication gate, M1B, and M1C are complete.

The first modularized executable has been built, independently reproduced,
and characterized.

It is a new DUT identity and has not yet been hardware validated.

## Current working source authority

working/b4a/ps2ip.c

This remains the transitional monolith path, but its contents are now M1B
source rather than byte-identical B4A source.

Current M1B source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Current monolith SHA256:

    7bafa7b58daa086cf10ce2e7ae467889873f9b282adc8101da966dc8d65ff653

First extracted module:

    working/b4a/ps2vnc_config_text.c
    working/b4a/ps2vnc_config_text.h

Frozen B4A remains unchanged under:

    baseline/frozen-b4a/

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

M1C build result:

    PASS — NEW MODULAR DUT

Current M1 ELF SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Current M1 ELF size:

    2927748 bytes

Second clean build reproduction:

    BYTE EXACT

Host config-text parity:

    PASS

Hardware validation:

    NOT RUN

Durable characterization:

    docs/M1C_DUT_CHARACTERIZATION.md

## Current documentation state

The successor documentation and recovery system are committed and active.

M0 source, build, dependency, reproduction, evidence, and hardware-resolution
authority are recorded inside PS-to-VNC.

Migration recoverability remains a definition-of-done requirement for every
subsequent modularization stage.

## Next action

Perform the required M1D hardware regression on the characterized modular
DUT.

The exact executable under test must be:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Machine observations and physical observations must remain separately
recorded.

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

    LAST_COMPLETE_STAGE=M2
    CURRENT_STAGE=M3
    CURRENT_STAGE_STATUS=IN_PROGRESS
    CURRENT_WORKING_SOURCE=working/b4a/ps2ip.c
    CURRENT_SOURCE_HEAD=58d22cba30174f92ebc50418da30080cef68d7c1
    LAST_HARDWARE_RESULT=M1D_PASS_MACHINE_AND_PHYSICAL
    NEXT_ACTION=M3A_plan_accelerated_extraction_waves

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
## M1A checkpoint

M1A is COMPLETE.

Selected functions:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Selected responsibility:

    configuration text whitespace trimming

Selection authority:

    docs/M1A_EXTRACTION_BOUNDARY.md

Implementation source mutation:

    NO

Next operation:

    PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository

M1B remains blocked until publication passes.
## GitHub publication checkpoint

Publication is COMPLETE.

Repository:

    Olsens11/PS-to-VNC

Visibility:

    PRIVATE

Default branch:

    main

Publication occurred after M1A and before any M1 implementation source
mutation.

Authority:

    docs/GITHUB_PUBLICATION.md

Next operation:

    M1B_mechanically_extract_config_text_module
## M1B checkpoint

M1B is COMPLETE.

Mechanical source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Extracted module:

    working/b4a/ps2vnc_config_text.c
    working/b4a/ps2vnc_config_text.h

M1 implementation source mutation:

    YES — CONFIG_TEXT_EXTRACTION_ONLY

M1 build performed:

    NO

M1 DUT binary:

    NONE

Next operation:

    M1C_build_and_characterize_first_modularized_DUT
## M1C checkpoint

M1C is COMPLETE.

M1 executable SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Second clean reproduction:

    PASS — BYTE EXACT

Extracted-function host parity:

    PASS

Hardware validation:

    NOT RUN

Authority:

    runtime/M1_DUT_AUTHORITY.env
    docs/M1C_DUT_CHARACTERIZATION.md

Next operation:

    M1D_run_required_hardware_regression

## M1D deployment checkpoint

The first modular M1 DUT is now present on the PS2.

DUT SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Deployment status:

    DEPLOYED_VERIFIED

Hardware status:

    NOT_RUN

Unique forensic PS2 target:

    /mass/0/PS-to-VNC-M1D1-20260830-020419-26ae06ff.ELF

Rolling launch target:

    /mass/0/PS2VNC.ELF

Deployment evidence:

    evidence/m1/m1d1-deployment/

The rolling ELF must not be treated as hardware validated until M1D records
both machine and physical results.

## M1D hardware-regression completion

M1D completed with independent machine and physical PASS authorities.

Exact DUT SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Startup gate:

    PASS

Machine result:

    PASS

Physical result:

    PASS

Evidence:

    evidence/m1/m1d2-hardware-20260830-022035/

The next lifecycle operation is M1E closure.

## M1E closure and M2A activation

M1 is complete.

Validated M1 DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

M1 hardware result:

    M1D_PASS_MACHINE_AND_PHYSICAL

M2 is now the active migration stage.

M2A selected:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

Target module:

    working/b4a/ps2vnc_config_text.c

Next:

    M2B_mechanically_extract_config_scalar_parsers

## M2B mechanical extraction checkpoint

M2B mechanically moved:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

from the monolith into:

    working/b4a/ps2vnc_config_text.c

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

Caller mutation:

    NO

Behavior mutation:

    NO

Makefile mutation:

    NO

Build performed:

    NO

Hardware run performed:

    NO

Next:

    M2C_build_and_characterize_scalar_parser_DUT

## M2 closure / accelerated migration transition

M2 is complete.

M2 DUT:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

M2 validation:

    PASS

Physical PS2 hardware requirement:

    NOT_REQUIRED

The active stage is now M3.

M3 begins the accelerated migration strategy by planning the remaining
monolith as coherent extraction waves rather than repeating one-helper
migration cycles.

Next:

    M3A_plan_accelerated_extraction_waves
