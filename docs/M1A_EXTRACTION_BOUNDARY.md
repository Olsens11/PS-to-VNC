# M1A First Extraction Boundary

## Status

    SELECTED AND COMMITTED

M1A selects the first implementation boundary to be mechanically extracted in
M1B.

M1A itself does not modify implementation source.

## Entry authority

Completed M0 working source:

    working/b4a/ps2ip.c

M0 source-authority commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

M0 source SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

M0 ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

## Selected responsibility

Configuration text whitespace trimming.

Selected historical functions:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Historical source ranges:

    ps2vnc_config_trim_left   10335-10341
    ps2vnc_config_trim_right  10344-10355

Combined function lines:

    19

Historical source span:

    10335-10355

No function definition intervenes between the selected functions.

## Why this is the first extraction

The dependency analysis and direct source inspection show that this is a
deliberately small and unusually clean boundary.

Properties:

- both functions are classified as config responsibility;
- no mutable or immutable project global is referenced;
- neither function calls another project function;
- the implementation is physically contiguous;
- signatures require no project-private type;
- callers are limited to configuration parsing;
- the two functions express one coherent responsibility;
- the boundary can be moved without algorithm redesign.

External callers:

    ps2vnc_config_parse_startup_mode
    ps2vnc_config_parse_display_section
    ps2vnc_config_parse_display_policy

External project callees:

    NONE

Project global references:

    NONE

## Rejected higher-ranked / nearby alternatives

### Display transaction take helpers

The numerical family rank was slightly lower-risk, but the proposed pair is
not a contiguous source block.

`ps2vnc_display_tx_parse_uint` sits between:

    ps2vnc_display_tx_take_line
    ps2vnc_display_tx_take_profile_value

Using that pair first would either create a non-contiguous extraction or tempt
M1 to enlarge the change.

### Remote-control getters

This pair is clean, but it is 82 lines rather than 19 and participates in live
remote display-control parsing.

There is no benefit to exercising that wider live-control surface before the
simpler config-text boundary.

### Backend-name conversion

This pair is small but exposes the monolith-private `ps2vnc_backend_t` type.

Choosing it first would require deciding type ownership at the same time as
our first translation-unit extraction.

M1 explicitly avoids mixing those concerns.

### HTTP parsing

The HTTP pair is also a strong future extraction candidate.

It has no project globals or project callees, but it currently serves six
callers spanning configuration and management responsibilities.

That is a wider first public API surface than the selected config-text pair.

## M1B proposed files

Transitional source path:

    working/b4a/ps2vnc_config_text.c

Transitional public header:

    working/b4a/ps2vnc_config_text.h

These files intentionally remain beside the M0 monolith during the first
mechanical extraction.

M1B will not simultaneously introduce a new top-level `src/` hierarchy or
rename the entire working tree.

Repository-layout cleanup remains a later maturation operation.

Long-term responsibility:

    src/config/

## Proposed public API

The historical names remain unchanged:

    char *ps2vnc_config_trim_left(char *text);
    void ps2vnc_config_trim_right(char *text);

No new wrapper names or compatibility aliases are planned.

The header should expose only these declarations unless the build proves a
strictly necessary additional declaration is required.

## Required implementation dependencies

The moved implementation requires only ordinary C string facilities:

    strlen

and the standard `size_t` type used by the existing body.

It does not require a PS-to-VNC project structure, enumeration, global, or
callback.

## Mechanical extraction rule

M1B must preserve the two function bodies textually except for changes that
are strictly required by translation-unit separation.

Expected required changes are limited to:

1. create `ps2vnc_config_text.h`;
2. create `ps2vnc_config_text.c`;
3. move the two existing function implementations;
4. remove `static` because the functions must cross translation units;
5. include the new header where required;
6. add the new object to the build;
7. remove the historical definitions from `ps2ip.c`.

Do not combine this with:

- whitespace normalization;
- function renaming;
- parser redesign;
- config behavior changes;
- error-policy changes;
- warning cleanup;
- unrelated includes cleanup;
- other function movement.

## Optimization / binary identity consequence

At M0:

    ps2vnc_config_trim_left  compiled_symbol=0
    ps2vnc_config_trim_right compiled_symbol=1

The first function was therefore not present as a standalone compiled text
symbol in the M0 dependency analysis.

Moving the functions into another translation unit and changing linkage from
`static` to external can change inlining, optimization, section layout,
symbols, debug information, and final ELF bytes.

That is expected.

M1 must treat the first modularized ELF as a NEW DUT IDENTITY even though the
source transformation is intended to preserve behavior.

Byte identity with B4A is not an M1 requirement.

## Planned validation

M1B/M1C must verify:

- exact removal of only the selected implementations from the monolith;
- exact creation of the selected module;
- no unexplained caller changes;
- successful clean controlled build;
- new ELF SHA256 recorded;
- symbol/linkage consequences characterized;
- dependency map or equivalent interface inspection updated;
- documentation/recovery state coherent.

Because M1 creates a new executable DUT identity, hardware regression is
required before M1 can be declared COMPLETE.

## Optional host-level functional test

These functions have no PS2 hardware dependency and are suitable for a tiny
deterministic host test.

A later M1 validation step may exercise:

- no leading whitespace;
- spaces;
- tabs;
- mixed leading spaces/tabs;
- no trailing whitespace;
- trailing spaces;
- trailing tabs;
- mixed trailing spaces/tabs;
- empty string;
- whitespace-only string.

Adding such a test must not alter the production function bodies.

## Publication gate

Before M1B is allowed to mutate implementation source, the independent
PS-to-VNC repository will be published to its own new GitHub repository.

Publication must verify:

- the repository belongs to PS-to-VNC, not legacy PS2VNC;
- the new remote is not the old PS2VNC remote;
- `main` is pushed successfully;
- the visible remote HEAD contains this committed M1A boundary;
- no legacy repository is modified or pushed.

M1B is blocked until that publication checkpoint passes.

## Next action

    PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository
