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
