# M2A Extraction Boundary

## Status

M2A is complete.

This phase selects the next exact mechanical extraction boundary. It does not
change implementation source.

## Target module

Existing implementation module:

    working/b4a/ps2vnc_config_text.c

Existing public header:

    working/b4a/ps2vnc_config_text.h

M2 extends that module from whitespace handling into scalar configuration-text
parsing.

## Selected function 1

    ps2vnc_config_parse_int

Current M1-derived source range:

    10458-10578

Current line count:

    121

Exact current function-text SHA256:

    b3c0223ca1087662d1d55235b01ac924c13cbc8356205e5892b66e597702a735

Current call sites excluding its definition:

    5

The frozen B4A dependency map classifies this function in the config module.
It owns no global references and has no cross-module outgoing project call.

One current cross-module incoming call is intentionally accepted:

    ps2vnc_display_tx_parse_uint -> ps2vnc_config_parse_int

That caller uses the function as a generic scalar text parser, so this is a
natural utility-module API rather than hidden state coupling.

## Selected function 2

    ps2vnc_config_parse_bool

Current M1-derived source range:

    12217-12240

Current line count:

    24

Exact current function-text SHA256:

    1ec9d694f09541d5faa5787f47bc8c06bd3ed3e6d3aa3fb7d40183cf13bff273

Current call sites excluding its definition:

    2

The frozen dependency map records no global references and no cross-module
outgoing project call for this helper.

## Combined boundary

Selection source SHA256:

    7bafa7b58daa086cf10ce2e7ae467889873f9b282adc8101da966dc8d65ff653

Total current call sites:

    7

Selected global references:

    0

Selected cross-module outgoing project callees:

    0

Selected cross-module incoming edges:

    1

## M2B mechanical contract

M2B must:

1. remove only the two selected function definitions from
   `working/b4a/ps2ip.c`;
2. place those exact implementations in `ps2vnc_config_text.c`;
3. remove only their `static` linkage as required for translation-unit use;
4. add their declarations to `ps2vnc_config_text.h`;
5. add only the standard-library includes required by the moved bodies;
6. leave all call sites unchanged;
7. leave the existing `ps2vnc_config_text.o` Makefile linkage unchanged;
8. make no behavior, parsing-policy, naming, formatting, or cleanup changes.

M2B is a mechanical extraction, not a refactor.

Next:

    M2B_mechanically_extract_config_scalar_parsers
