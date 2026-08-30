# M1C First Modular DUT Characterization

## Status

    COMPLETE

M1C built and characterized the first executable produced from modularized
PS-to-VNC source.

## Source authority

Source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Source generation:

    M1B config-text extraction

No implementation source was changed by M1C.

## First modular DUT

SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Size:

    2927748 bytes

M0/B4A reference SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Relationship:

    NEW NON-IDENTICAL DUT

This difference is expected because M1 introduced a new translation-unit and
linkage boundary.

## Reproducibility

A second clean build was created independently from the same committed M1B
source, using the same pinned Docker image and B4A libps2ip authority.

The second build produced exactly the same final ELF bytes:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Result:

    BYTE-EXACT SECOND-BUILD REPRODUCTION

Ordinary intermediate objects are compared in:

    evidence/m1/m1c2/OBJECT_REPRODUCTION.tsv

The gsHires slim-LTO object remains diagnostic at the intermediate-container
level, following the M0 policy established for generated LTO metadata.

Final executable identity is authoritative.

## Translation-unit linkage

The new config-text object defines:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

The remaining ps2ip object contains undefined external references to those two
symbols.

The final ELF defines both functions globally.

Result:

    PASS

## Host behavior parity

The exact historical B4A trim implementations and the M1 extracted
implementations were compiled into one deterministic host test.

The test covers empty strings, ordinary text, leading spaces, leading tabs,
mixed whitespace, trailing whitespace, whitespace-only strings, embedded
spaces, and representative key/value text.

For every case:

- left-trim pointer offset matched;
- left-trim resulting text matched;
- right-trim resulting text matched;
- combined left/right output matched.

Result:

    PASS

This is not a PS2 hardware test.

It specifically validates the behavior of the responsibility moved by M1B.

## ELF characterization

Both M0 and M1:

- remain ELF32 little-endian MIPS executables;
- retain entry point `0x103210`;
- retain the same ELF machine and architectural flags;
- retain 28 section headers.

M0 `.text`:

    375144 bytes

M1 `.text`:

    373720 bytes

Delta:

    -1424 bytes

Whole ELF size delta:

    -5804 bytes

The `.data`, `.rodata`, and `.bss` section sizes are unchanged.

Detailed section payload/address comparison:

    evidence/m1/m1c2/ELF_SECTION_COMPARISON.tsv

Runtime summary:

    evidence/m1/m1c2/ELF_RUNTIME_SUMMARY.txt

Symbol comparison:

    evidence/m1/m1c2/SYMBOL_COMPARISON.txt

## Current operational build product

The characterized M1 ELF is installed at:

    working/b4a/PS2VNC.ELF

That path is an ignored generated build product.

The durable committed binary evidence is:

    evidence/m1/m1c1/PS2VNC-M1C1.ELF

## Hardware status

Hardware validation:

    NOT RUN

Deployment to PS2:

    NOT PERFORMED

M1C does not claim physical equivalence to B4A merely because the extraction
was mechanical.

This is a new executable DUT identity and must receive the required M1
hardware regression before M1 can close.

## Next

    M1D_run_required_hardware_regression
