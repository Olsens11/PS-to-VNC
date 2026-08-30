# M2B Mechanical Extraction Result

## Result

M2B completed the exact mechanical extraction selected by M2A.

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

No build or hardware run was performed in M2B.

## Moved implementations

The following definitions were removed from:

    working/b4a/ps2ip.c

and moved into the existing configuration-text translation unit:

    working/b4a/ps2vnc_config_text.c

Functions:

- `ps2vnc_config_parse_int`
- `ps2vnc_config_parse_bool`

Their declarations now live in:

    working/b4a/ps2vnc_config_text.h

## Mechanical constraints

The move preserved the selected implementations exactly except for removing
their former `static` translation-unit linkage.

Caller changes:

    NONE

Behavior changes:

    NONE

Makefile changes:

    NONE

New object files:

    NONE

The already-linked object remains:

    ps2vnc_config_text.o

## Source identities

Pre-M2B monolith SHA256:

    7bafa7b58daa086cf10ce2e7ae467889873f9b282adc8101da966dc8d65ff653

Post-M2B monolith SHA256:

    60989653549cd3d2c0669d4c3e90f88073c59f607e338489a7c42f9d969e204f

Pre-M2B config-text source SHA256:

    b95819a508657ed50e553f9f9ae9a37854bb467fd5fb5c11e53f9ad0665a3f9c

Post-M2B config-text source SHA256:

    e2519fd0cb2033982d523898c48bb826c368d945ad3196732be0b80964fa61d2

Pre-M2B config-text header SHA256:

    06a1f25b1cc0f00701885528177e81c53b128818f74ddf6bb1c090373c585884

Post-M2B config-text header SHA256:

    a2067b8a9d67ca7b03731f4ccac856da51e8d5951ac5c49b33da8a5dfddbf7f7

Unchanged Makefile SHA256:

    f0ee6782ca805edd8b27092ec7a4fdc526750b42cae1e2e459285a20529ed6ee

Selected `ps2vnc_config_parse_int` pre-move SHA256:

    b3c0223ca1087662d1d55235b01ac924c13cbc8356205e5892b66e597702a735

Exported definition SHA256 after static-linkage removal:

    5ca03c2bc16bec3171d7a92cb5bfa7c44b2714adff64d80e059fe74f36a448b8

Selected `ps2vnc_config_parse_bool` pre-move SHA256:

    1ec9d694f09541d5faa5787f47bc8c06bd3ed3e6d3aa3fb7d40183cf13bff273

Exported definition SHA256 after static-linkage removal:

    6cb69079007ebc81cc3645662bc8422d150dc9617beada851485029148516d26

## Call-site preservation

`ps2vnc_config_parse_int` retains five monolith call sites.

`ps2vnc_config_parse_bool` retains two monolith call sites.

The existing cross-module management call to
`ps2vnc_config_parse_int` remains unchanged and now resolves through the
public configuration-text header.

## Next

M2B establishes source authority only.

The next migration operation is:

    M2C_build_and_characterize_scalar_parser_DUT
