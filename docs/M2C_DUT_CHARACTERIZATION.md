# M2C DUT Characterization

## Result

M2C passed.

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

Build commit:

    59f5dbd345e1edea5e2b623485c8adcb9281c482

Exact M2 DUT SHA256:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

Bytes:

    2927920

## Behavioral and linkage validation

Host scalar-parser parity:

    PASS — 24 / 24

Translation-unit linkage:

    PASS

Two independent clean builds:

    BYTE_EXACT

## Build dependency

The exact frozen successor PS2IP authority was restored into the generated
root build path.

SHA256:

    b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

No legacy artifact was required.

## Relation to M1

Validated M1 DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

M2 DUT:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

Identity relation:

    NEW_NONIDENTICAL_DUT

Byte delta:

    +172

The new identity is expected because two real functions now cross a
translation-unit boundary.

## Validation classification

    LOW_RISK_MECHANICAL_CONFIG_SCALAR_PARSER_EXTRACTION

The extraction changed no caller logic, parser behavior, Makefile linkage,
or hardware-facing implementation.

Hardware requirement:

    NOT_REQUIRED_LOW_RISK_MECHANICAL_EXTRACTION

Hardware run:

    NO

## Evidence

    evidence/m2/m2c

Manifest SHA256:

    92d1956fa523035e4e293798b92f1bdb1754b8334ab05dcc5a1618b69bf83d45
