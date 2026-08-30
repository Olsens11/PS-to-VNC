# M3 Coarse-First Macro Migration

M3 now uses hierarchical structural migration.

The complete mapped implementation envelope was moved from:

    working/b4a/ps2ip.c

into:

    working/b4a/ps2vnc_macro_body.inc

while remaining in the exact same C translation unit.

Extracted lines:

    18313

Remaining ps2ip.c lines:

    467

The resulting PS2 load image is byte-for-byte identical to the validated M2
load image:

    c8040ee6350e658d80f840d902dc9ab44e831f5907f484fa644e84fa1aaad05e

Therefore this operation requires no physical PS2 regression.

This large fragment is intentionally temporary. It will now be recursively
divided into a small number of major regions before those regions are
subdivided again.

Next:

    M3B_recursive_macro_split

## Historical M2 DUT reference repair

The M3 build correctly replaces the mutable working ELF.

The M2 DUT authority originally referenced that mutable working path, which
made the historical M2 checker fail after the first M3 build even though the
M2 DUT itself remained safely archived.

The M2 authority now references the immutable archived M2 DUT:

    evidence/m2/m2c/PS2VNC-M2C-build2.ELF

SHA256:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

This changes no M2 result or artifact. It corrects only the historical
authority pointer.

## M3B top-level recursive split

The 18,313-line implementation body is now divided into three large
source-ordered regions:

    region1 = 6510 lines
    region2 = 5625 lines
    region3 = 6178 lines

The children concatenate byte-for-byte to the original M3A parent body.

The PS2 load image remains byte-for-byte identical to M2:

    c8040ee6350e658d80f840d902dc9ab44e831f5907f484fa644e84fa1aaad05e

Next:

    M3C_recursive_split_macro_regions

## M3C six-leaf recursive hierarchy

All three M3B macro regions have been subdivided simultaneously.

The implementation now has six source-ordered leaves:

    region1a = 3414 lines
    region1b = 3096 lines
    region2a = 2500 lines
    region2b = 3125 lines
    region3a = 3254 lines
    region3b = 2924 lines

Each child pair reconstructs its M3B parent byte-for-byte.

The resulting PS2 load image remains byte-for-byte identical to M2:

    c8040ee6350e658d80f840d902dc9ab44e831f5907f484fa644e84fa1aaad05e

A per-leaf module/function inventory is stored at:

    evidence/m3/m3c-six-leaf-split/LEAF-INVENTORY.tsv

Next:

    M3D_classify_six_macro_leaves_and_promote_subsystems
