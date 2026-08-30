# M0 Historical Build Provenance

## Status

M0C historical build-environment reconstruction is complete.

No writable PS-to-VNC implementation exists yet.

The next stage is M0D: create the writable B4A authority and perform a
controlled reproduction build.

## Historical build authority

B4A:

    D17AL-F8J2-B4A

Historical base commit:

    b6d76f330439b7047fdb17422aa41cfbe2324a08

Frozen B4A source:

    baseline/frozen-b4a/ps2ip.c

Frozen B4A ELF:

    baseline/frozen-b4a/PS2VNC.ELF

The surviving historical B4A worktree was independently compared against the
frozen package.

The following were byte-identical:

- `ps2ip.c`
- `Makefile`
- `ps2vnc_gsHires.c`
- `ps2vnc_identity.c`
- `PS2VNC.ELF`

Therefore the frozen package is directly tied to the historical B4A build
worktree rather than merely being a later reconstruction.

## Historical build scripts

Historical `scripts/build.sh` SHA256:

    6c3cba1553ce14b5211f4abd24fc8888defb4b0e145f6b71abda73e025ff63ab

Historical `scripts/build-ps2ip.sh` SHA256:

    59eb34050885ae6e087b35f5c13a0a38e25ea9245e5330d0fa79c40b5b23253d

The base-head and current tracked versions of both scripts were found to be
byte-identical.

## Pinned dependency source revisions

PS2SDK:

    f08e889fef8ab361f863c44ebe78212ced2839ca

lwIP:

    77dcd25a72509eb83f72b033d219b1d40cd8eb95

The historical builder deliberately mounts the PS2SDK source checkout at:

    /work/forensics/test15B2K-wscale128-src

This path is part of the reproduction environment because GCC/LTO may encode
compiler-visible paths in intermediate objects.

## Docker and compiler authority

Pinned historical image:

    ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11

That exact digest remains available locally.

Observed compiler:

    GCC 15.2.0

Observed binutils:

    2.45.1

The surviving historical application objects also identify themselves as
GCC 15.2.0.

Detailed machine-readable hashes are recorded in:

    runtime/M0_BUILD_AUTHORITY.env

## B4A libps2ip authority

B4A authoritative patched archive SHA256:

    b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

The archive found inside the surviving B4A worktree has this exact hash and is
byte-identical to:

    baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a

Therefore the frozen archive is the actual B4A link-input authority.

A different archive currently exists at the old legacy root:

    /home/ps2/ps2vnc/build/deps/libps2ip_mtu1458_wscale128.a

Its SHA256 is:

    f16bd80321657a854f09d96622a11308232c9053486d6158b78825e452b35860

That archive predates the B4A build and is not the B4A link input.

It must not be substituted into the PS-to-VNC M0 reproduction.

## Stale archive comparison

The B4A and stale archives contain the same 36 archive members.

Thirty-two members are byte-identical.

Four differ:

- `ps2ip.o`
- `sys_arch.o`
- `ps2ip_ps2sdk.o`
- `erl-support.o`

The corresponding inspected source trees use the same pinned PS2SDK and lwIP
commits and show the same expected patched source-file hashes.

Both sets of differing objects identify GCC 15.2.0.

This historical difference is recorded as provenance but does not override the
direct B4A archive authority.

## Reference top-level build objects

Reference B4A `ps2ip.o`:

    a406425e6af6cf2ae9bc8822f3daf655115d7a14913cd961aff9ada83308ec2e

Reference B4A `ps2vnc_identity.o`:

    f3876aa8dedb6eccd95632ae074d8af80d7b99f20635d57f250dbf961a4a7b81

Reference B4A `ps2vnc_gsHires.o`:

    c0be403fa626c6e8f463a27a27c6d89671eee4906ab419dffb6432ec418b9a6b

Generated DEV9 C:

    cdf6c86894b2e49aadd943c6afc59558a8ae7151dc86be5d7ab895b56af40c9f

Generated DEV9 object:

    d266c9176195021d6f5bffc7123c17974706e946e221c61f7db9ff139fe2ea4b

Generated NETMAN C:

    5c39327e2bf3c4e439cfc6d511a21db0251598af2e4b3694d733bb40dee2a162

Generated NETMAN object:

    bc99cc61be33f00603d43b3a354771f13a0f97ddb2148cedea40100390c568f2

Generated SMAP C:

    bfaada6dd54ecdf816066d307cc79c3e332c48877dd75c4b08570dee4698708c

Generated SMAP object:

    5149431f2b5550ed526e209daa7b62ade808773461f3e7edfafc979d88b36550

## Final reference ELF

SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Size:

    2933552 bytes

Entry point:

    0x103210

## M0D comparison ladder

The controlled M0D build should compare in this order:

1. writable source bytes;
2. generated IRX C files;
3. generated IRX objects;
4. `ps2ip.o`;
5. `ps2vnc_identity.o`;
6. `ps2vnc_gsHires.o`;
7. exact patched libps2ip dependency;
8. final ELF.

This permits identification of the first divergence rather than treating the
ELF as a single opaque result.

## M0D dependency policy

The first controlled application build must use the exact preserved B4A
libps2ip archive.

This isolates reproduction of the application build from reproduction of the
network-library archive.

A separate controlled rebuild of libps2ip may then test whether the historical
archive itself is reproducible.

Do not allow a freshly generated archive to silently replace the preserved
B4A dependency before its identity has been checked.

## Authority hierarchy

For M0 reproduction, use this order:

1. committed frozen B4A artifacts and recorded hashes;
2. recorded historical build scripts and pinned revisions;
3. pinned Docker digest and recorded toolchain hashes;
4. surviving historical worktree only as corroborating evidence;
5. current legacy working/build directories only as historical evidence.

The old legacy build directory is not successor build authority.
