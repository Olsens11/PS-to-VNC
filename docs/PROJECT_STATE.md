# PS-to-VNC Project State

## Current state

Repository bootstrap is in progress.

No modular working source has yet been promoted.

Current immutable behavioral reference:

`D17AL-F8J2-B4A`

Frozen source:

`baseline/frozen-b4a/ps2ip.c`

SHA256:

`67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac`

Source size:

`18,938 lines`

Frozen dependency analysis:

`baseline/analysis/B4A-dependency-map/`

Analysis manifest SHA256:

`075d68d9992332a32bdca54a32964363fb893071fdcc5c9e2dfc7fb26a2e1749`

## Development authority

Current working source authority:

**NONE**

The first source-development stage is M0.

M0 will establish a reproducible B4A-equivalent working build inside
PS-to-VNC without modifying the immutable baseline.

## Documentation state

PS-to-VNC documentation is living documentation.

Code movement and documentation movement are one operation.

Current source location authority is:

`docs/reference/FILE_AND_SERVICE_MAP.md`

## Durable recovery authority

Conversation state is not project authority.

Canonical machine-readable migration state:

`runtime/MIGRATION_STATE.env`

Canonical human-readable migration state:

`docs/MIGRATION_STATE.md`

Mandatory recovery entry point:

`START_HERE.md`

Recovery command:

`scripts/resume-state.sh`

Consistency gate:

`scripts/migration-check.sh`

The next source-changing operation must establish the expected migration state
before mutation.

## M0 reproduction contract

M0 is now in progress.

The governing build/reproduction contract is:

`docs/M0_BASELINE_CONTRACT.md`

No writable source authority has been created yet.

The next operation is read-only reconstruction and fingerprinting of the
historical build environment before `working/b4a/` is populated.

## M0 Outcome A reproduction

PS-to-VNC has independently rebuilt the writable B4A authority to the exact
historical ELF bytes.

Result:

    OUTCOME_A_BYTE_EXACT_ELF

The sole non-identical intermediate was the slim-LTO gsHires object container.
Its LTO payloads are identical; only generated section-name metadata differs.

The final executable is byte-identical, so this does not constitute a new DUT
identity.

See:

    docs/M0_BUILD_RESULT.md

## M0F hardware-validation resolution

M0F is complete.

No new hardware run was performed.

A new M0 regression is not required because the independently generated M0 ELF
is byte-identical to the historically validated B4A executable.

Historical B4A validation therefore remains applicable to the identical DUT
bytes.

This is an inherited-evidence determination, not a new hardware PASS.

Next:

    M0G — close M0 and define the first modularization stage.


## M0 complete — M1 entry

M0 is COMPLETE.

Established authority:

    working/b4a/ps2ip.c

M0 source authority commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

M0 result:

    OUTCOME_A_BYTE_EXACT_ELF

M1 status:

    NOT STARTED

M1 is governed by:

    docs/M1_EXTRACTION_CONTRACT.md

Next operation:

    M1A_select_first_low_coupling_leaf_extraction_boundary

## M1A first extraction selected

M1A is COMPLETE.

The first mechanical extraction boundary is:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Authority:

    docs/M1A_EXTRACTION_BOUNDARY.md

No implementation source has changed yet.

Before M1B begins, PS-to-VNC must be published to its own new GitHub
repository and the remote identity must be verified against the legacy
repository.

## Private GitHub publication checkpoint

PS-to-VNC is now published to its own private GitHub repository:

    Olsens11/PS-to-VNC

The first remote publication occurred after M1A and before M1B.

No M1 implementation source mutation had occurred at the publication point.

The repository is deliberately PRIVATE while the project remains under active
development.

Next:

    M1B_mechanically_extract_config_text_module

## M1B first mechanical extraction

M1B is COMPLETE.

The first implementation boundary has moved out of the monolith:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

New module:

    working/b4a/ps2vnc_config_text.c

New header:

    working/b4a/ps2vnc_config_text.h

Mechanical source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

No M1 ELF has been built yet.

Next:

    M1C_build_and_characterize_first_modularized_DUT

## M1C first modular DUT

M1C is COMPLETE.

The first modular PS-to-VNC executable is:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

It is a new DUT identity.

A second clean build reproduced the ELF byte-for-byte.

The extracted config-text responsibility also passed deterministic host parity
against the historical B4A implementations.

Hardware validation has not yet been performed.

Next:

    M1D_run_required_hardware_regression

## M1D deployment checkpoint

The exact M1C DUT has been deployed to the PS2 and verified by FTP
download-back SHA256.

DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Unique target:

    /mass/0/PS-to-VNC-M1D1-20260830-020419-26ae06ff.ELF

Rolling target:

    /mass/0/PS2VNC.ELF

Deployment:

    PASS — VERIFIED

Hardware regression:

    NOT RUN

Next operation remains:

    M1D_run_required_hardware_regression

## M1D hardware regression passed

The first modular M1 DUT passed its required real-hardware migration gate.

Exact DUT SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Machine result:

    PASS

Physical result:

    PASS

Startup gate:

    PASS

Evidence:

    evidence/m1/m1d2-hardware-20260830-022035/

M1D hardware authority is complete. The next migration operation is M1E
closure and selection of the next mechanical extraction boundary.

## M1 complete — M2 active

The first modular extraction cycle is complete through real-hardware
validation.

M1 validated DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

M2 now continues migration of the configuration-text utility boundary.

M2A selected:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

Target:

    working/b4a/ps2vnc_config_text.c

Next:

    M2B_mechanically_extract_config_scalar_parsers

## M2B source extraction complete

The M2 scalar configuration parsers have been mechanically moved into the
existing configuration-text translation unit.

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

Moved:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

No callers, behavior, Makefile linkage, build result, or hardware state were
changed by M2B.

Next:

    M2C_build_and_characterize_scalar_parser_DUT

## M2 complete — accelerated migration begins

M2 is complete with exact DUT:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

The migration now changes strategy from proof-sized helper moves to larger
dependency-informed extraction waves.

Current stage:

    M3

Next:

    M3A_plan_accelerated_extraction_waves
