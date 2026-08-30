# M1B Mechanical Extraction Result

## Status

    COMPLETE

M1B performed the first implementation-source modularization in PS-to-VNC.

No build was performed by M1B.

No hardware run was performed by M1B.

## Source authority

Mechanical source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

M0 entry source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

M0 monolith SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

M1B monolith SHA256:

    7bafa7b58daa086cf10ce2e7ae467889873f9b282adc8101da966dc8d65ff653

M1B Makefile SHA256:

    f0ee6782ca805edd8b27092ec7a4fdc526750b42cae1e2e459285a20529ed6ee

## Extracted responsibility

Configuration text whitespace trimming.

Functions moved:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Historical location:

    working/b4a/ps2ip.c
    historical lines 10335-10355

New source:

    working/b4a/ps2vnc_config_text.c

New public header:

    working/b4a/ps2vnc_config_text.h

New source SHA256:

    b95819a508657ed50e553f9f9ae9a37854bb467fd5fb5c11e53f9ad0665a3f9c

New header SHA256:

    06a1f25b1cc0f00701885528177e81c53b128818f74ddf6bb1c090373c585884

## Mechanical transformation

The transformation was deliberately limited to:

1. remove the two selected definitions from `ps2ip.c`;
2. add `ps2vnc_config_text.h` to the monolith;
3. create `ps2vnc_config_text.c`;
4. remove `static` from the two moved definitions so callers in another
   translation unit can link to them;
5. expose only those two declarations in the new header;
6. add `ps2vnc_config_text.o` to `EE_OBJS`.

The existing parser call sites were not rewritten.

No parser algorithm, whitespace semantics, error policy, project-private type,
display behavior, RFB behavior, controller behavior, or recovery behavior was
changed.

## Dependency boundary

Project globals referenced by the moved functions:

    NONE

External project callees from the moved functions:

    NONE

Existing caller functions:

    ps2vnc_config_parse_startup_mode
    ps2vnc_config_parse_display_section
    ps2vnc_config_parse_display_policy

## Translation-unit consequence

This source layout is no longer byte-identical to M0 source.

That is expected.

The prior M0 ELF was removed from the working directory after the source
commit so it cannot be mistaken for an executable built from the new M1
source.

M1B did not build a replacement ELF.

Therefore there is currently:

    NO M1 DUT BINARY

M1C must perform a controlled clean build and establish the first modularized
ELF identity.

## Binary-equivalence rule

The M1 ELF is expected to differ from B4A because translation-unit boundaries,
linkage, optimization, symbol materialization, debug information, and section
layout may change.

A differing ELF is not by itself an M1 failure.

It is a new DUT identity and must be characterized and later hardware-tested.

## Next

    M1C_build_and_characterize_first_modularized_DUT
