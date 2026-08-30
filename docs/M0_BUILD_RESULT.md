# M0 Reproduction Build Result

## Classification

M0 reproduction classification:

    OUTCOME_A_BYTE_EXACT_ELF

The PS-to-VNC writable B4A authority independently regenerated the exact
historical B4A executable.

## Final executable

Historical/frozen SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Reproduced SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Size:

    2933552 bytes

The reproduced and frozen ELF files compare byte-for-byte equal.

Therefore M0 achieved the preferred Outcome A defined by the baseline
reproduction contract.

## Exact ordinary/generated intermediates

The following reproduction intermediates matched their historical B4A hashes:

- generated DEV9 IRX C;
- DEV9 IRX object;
- generated NETMAN IRX C;
- NETMAN IRX object;
- generated SMAP IRX C;
- SMAP IRX object;
- `ps2ip.o`;
- `ps2vnc_identity.o`.

The exact preserved B4A patched libps2ip archive was deliberately supplied as
the network link dependency.

## gsHires slim-LTO intermediate

`ps2vnc_gsHires.o` is compiled with GCC slim LTO.

Historical object SHA256:

    c0be403fa626c6e8f463a27a27c6d89671eee4906ab419dffb6432ec418b9a6b

Reproduced object SHA256:

    6d63b362fab80e541fb0f7fe4c77496defdb8f122607648f5102c97e7c07829f

Both objects are:

    35308 bytes

There are:

    504 differing bytes

The variance was characterized before accepting the build result.

Eighteen `.gnu.lto_*` sections have different generated section-name suffixes:

Historical suffix:

    6bd070f4f168563e

Reproduced suffix:

    a1c8abfaf31cb356

For all eighteen affected LTO sections:

- section sizes are identical;
- section payload SHA256 values are identical.

The only section payload differences resulting from those generated names are:

- `.strtab`
- `.shstrtab`

The symbol-table textual difference is likewise the section-name references.

Both objects identify:

    GCC 15.2.0

This is classified as slim-LTO intermediate naming metadata variance.

It has no effect on the final linked executable, which is byte-identical.

## M0D2 harness exit 60

The controlled build itself returned success and classified the executable as:

    OUTCOME_A_BYTE_EXACT

The outer M0D2 experiment then returned exit 60 because its initial diagnostic
ladder incorrectly required the slim-LTO object file itself to have a
byte-identical container hash.

M0D2A characterized that mismatch.

Therefore exit 60 is not a compiler failure, linker failure, source divergence,
or executable divergence.

It is superseded by the characterized LTO-intermediate policy recorded here.

## Reproducibility policy

For PS-to-VNC M0:

- source bytes are authoritative;
- ordinary/generated build intermediates are useful exact references;
- slim-LTO intermediate object container hashes are diagnostic;
- LTO payload equivalence may be characterized when container metadata differs;
- final ELF byte identity is authoritative for Outcome A.

A differing final ELF remains a new DUT identity regardless of intermediate
explanations.

## Hardware consequence

The reproduced ELF is byte-for-byte the same DUT as the frozen B4A ELF.

M0 therefore does not require a new hardware regression merely to establish
reproduction equivalence.

Historical B4A hardware validation remains applicable to these exact bytes.

Whether to perform an additional confidence launch is a separate optional
decision and is not required to classify M0 as Outcome A.

## Evidence

Committed evidence:

    evidence/m0/m0d2/

Machine authority:

    runtime/M0_BUILD_AUTHORITY.env
