# M0 Working-Baseline Reproduction Contract

## Purpose

M0 establishes the first writable PS-to-VNC implementation authority without
changing the behavior of the frozen D17AL-F8J2-B4A reference.

M0 is a reproduction stage, not a refactoring or cleanup stage.

No modular extraction begins until M0 is closed.

## Immutable historical authority

Frozen source:

    baseline/frozen-b4a/ps2ip.c

Source SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

Source lines:

    18938

Frozen Makefile SHA256:

    4fa4efa12140a18bda808cf7a1d183a70d29e41bd0484dc5e12f8188abe429c7

Frozen local gsHires source SHA256:

    70d46c55314de0e11098cca184ac1c5838a1a29c437a58c1a1131bfe028cf8fc

Frozen identity source SHA256:

    1b7bfd4ce194241435528816955a61f4283a40f0ccca00a5568c3fe5c805d5f6

Frozen patched libps2ip SHA256:

    b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

Frozen libps2ip patch SHA256:

    c5ae37ac8228bc7cd73a5ea275b5c7f3d19d68119bd99d740d4326759074e098

Known tested B4A ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Known ELF size:

    2933552 bytes

## Critical build characteristics

The B4A Makefile links:

- ps2ip.o
- ps2vnc_identity.o
- ps2vnc_gsHires.o
- DEV9_irx.o
- NETMAN_irx.o
- SMAP_irx.o
- gsKit
- dmaKit
- libpad
- netman
- the patched libps2ip archive
- debug
- patches

The link also uses:

    -Wl,--wrap=sendto

The local gsHires implementation records source lineage:

    gsKit 43122eb96289167975b56caa45beb71eb8684fa2

Its explicitly preserved compilation characteristics include:

    -D_EE
    -DF_gsKit_hires_init_screen=1
    -G0
    -march=r5900
    -mhard-float
    -msingle-float
    -mno-llsc
    -mno-shared
    -mplt
    -O2
    -g
    -flto=auto
    -fno-fat-lto-objects

## Patched network dependency

The frozen B4A build does not use an interchangeable generic libps2ip.

Its preserved dependency includes the historical networking changes for:

- Ethernet MTU 1458;
- PBUF pool size 256;
- receive-window scaling enabled;
- TCP receive scale 1;
- TCP receive window 131070.

The frozen archive and patch are authoritative evidence.

Do not silently substitute an ordinary SDK libps2ip.

## Known build-provenance gap

The frozen Makefile expects:

    /work/build/deps/libps2ip_mtu1458_wscale128.a

and directs historical builds through:

    ./scripts/build.sh

That wrapper is not contained in the frozen B4A package itself.

Therefore the frozen package does not, by itself, prove the complete external
toolchain/container/library environment needed for byte-identical ELF
reproduction.

M0 must reconstruct and record that environment before interpreting an ELF
hash mismatch.

## Planned writable authority

The writable monolithic reproduction will be created under:

    working/b4a/

That location is deliberately separate from:

    baseline/frozen-b4a/

The frozen tree remains immutable.

The writable tree initially retains historical symbol names and source
structure.

Do not perform a mass ps2vnc-to-pstvnc rename during M0.

## Build-layout principle

The successor build tooling should be capable of presenting the writable B4A
tree to the historical toolchain with source/build paths equivalent to the
original build where practical.

This is important because debug information, LTO, archive ordering, generated
IRX objects, compiler versions, library versions, and paths may affect the
resulting ELF even when C source is identical.

## Reproduction outcomes

### Outcome A — byte-exact reproduction

Preferred result:

    successor ELF SHA256 =
    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

If achieved, record the complete successor build environment and preserve the
reproduction procedure.

Byte identity establishes that the rebuilt executable is the same B4A binary
already represented by the frozen behavioral evidence.

### Outcome B — characterized non-identical reproduction

If the source and intended build inputs are exact but the ELF differs:

1. do not edit source merely to chase the old hash;
2. record the new ELF SHA256 and size;
3. fingerprint the compiler/toolchain/container;
4. fingerprint linked SDK and gsKit inputs where practical;
5. compare ELF sections, symbols, build/debug metadata, and code/data identity;
6. identify known causes of difference where evidence permits;
7. preserve both the historical and successor identities.

A non-identical rebuild is not automatically a failed implementation, but it
is a new DUT identity.

### Outcome C — unexplained build divergence

If the successor cannot reproduce the intended B4A source/build semantics, M0
is BLOCKED.

Do not proceed to modularization.

## Hardware-validation rule

If Outcome A is achieved, the new ELF is byte-identical to the frozen tested
B4A ELF. Additional hardware smoke testing may still be performed, but binary
behavior has not changed.

If Outcome B is accepted, hardware regression is mandatory before M0 closes.

The rebuilt successor must establish B4A-equivalent operating behavior before
M1 begins.

## M0 prohibition list

During M0 do not:

- split the monolith;
- rename historical APIs;
- normalize source formatting;
- remove warnings;
- change algorithms;
- change controller behavior;
- change RFB behavior;
- change display behavior;
- change recovery behavior;
- change configuration semantics;
- optimize the code;
- alter the frozen baseline.

Those belong to later modularization or maturation stages.

## M0 phases

M0A — frozen build-authority inventory.

Status:

    COMPLETE

M0B — define reproduction contract and formally start M0.

Status after this document is committed:

    COMPLETE

M0C — reconstruct and fingerprint historical build environment.

Status:

    COMPLETE

Authority:

    docs/M0_BUILD_PROVENANCE.md
    runtime/M0_BUILD_AUTHORITY.env

M0D — create writable `working/b4a/` authority and perform controlled rebuild.

Status:

    COMPLETE

Result:

    Final ELF byte-exact to historical B4A.

M0E — classify build result as Outcome A, B, or C.

Status:

    COMPLETE

Classification:

    OUTCOME_A_BYTE_EXACT_ELF

Evidence:

    docs/M0_BUILD_RESULT.md
    evidence/m0/m0d2/

M0F — perform any hardware validation required by the classified outcome.

Status:

    COMPLETE

Resolution:

    Mandatory new hardware regression is NOT REQUIRED because the M0
    executable is byte-identical to the historically validated B4A DUT.

No new hardware run was performed by M0F.

Authority:

    docs/M0_HARDWARE_RESOLUTION.md

M0G — close M0, record the resulting source/build authority, and define M1.

## Completion requirement

M0 is COMPLETE only when a fresh conversation can determine from the
repository:

- exact writable source authority;
- exact build procedure;
- exact ELF identity;
- toolchain/build-environment identity;
- reproduction classification;
- hardware-validation requirement and result;
- next modularization operation.
