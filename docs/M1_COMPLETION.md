# M1 Completion

## Result

M1 is complete.

M1 proved the migration procedure by extracting the first low-coupling
implementation from the historical monolith into a real translation unit,
building a new DUT, and validating that DUT on real hardware.

## What moved

The following implementations moved out of `working/b4a/ps2ip.c`:

- `ps2vnc_config_trim_left`
- `ps2vnc_config_trim_right`

They now live in:

    working/b4a/ps2vnc_config_text.c

with declarations in:

    working/b4a/ps2vnc_config_text.h

## Module responsibility

The module owns small configuration-text utility operations.

Public module API at M1 completion:

- `ps2vnc_config_trim_left`
- `ps2vnc_config_trim_right`

Private module state:

    NONE

Temporary coupling debt:

    CALLERS_REMAIN_IN_MONOLITH

That coupling is deliberate during incremental migration.

## Source authority

M1 implementation source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Exact validated M1 DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Build result:

    M1C_BUILD_PASS_NEW_DUT

Second clean build reproduction:

    BYTE_EXACT

Hardware result:

    M1D_PASS_MACHINE_AND_PHYSICAL

Machine:

    PASS

Physical:

    PASS

Startup gate:

    PASS

Hardware evidence:

    evidence/m1/m1d2-hardware-20260830-022035/

## Closure

M1A through M1E are complete.

The next migration stage is M2.

M2A selects two scalar configuration parsers for the existing
`ps2vnc_config_text` module:

- `ps2vnc_config_parse_int`
- `ps2vnc_config_parse_bool`

Next implementation action:

    M2B_mechanically_extract_config_scalar_parsers
