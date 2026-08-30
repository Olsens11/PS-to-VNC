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

## M3D semantic subsystem promotion

The six generic recursive leaves now have semantic subsystem identities:

- runtime support
- UI/controller
- framebuffer engine
- management/config
- calibration/state
- display orchestration

No implementation bytes were reordered within any parent region.

The resulting PS2 load image remains byte-for-byte identical to M2.

Exact cross-boundary promotion evidence for the first two real translation
unit candidates is stored under:

    evidence/m3/m3d-semantic-leaves

Next:

    M3E_promote_framebuffer_engine_and_management_config_to_real_translation_units

## M3E first real translation-unit boundary

M3E crossed the first genuinely independent multi-thousand-line C
translation-unit boundary.

Detached service-side region:

    management/config + calibration/state
    6379 migrated source lines

Independent services object:

    PASS
    60 defined text symbols

Two clean builds:

    BYTE_EXACT

ELF:

    ef0124605ebb2729ad0cf26eb9ba0e3d2ac0656eb72f4d8d82a2203a37088d4b
    2941156 bytes

Previous PS2 load image:

    c8040ee6350e658d80f840d902dc9ab44e831f5907f484fa644e84fa1aaad05e
    446472 bytes

M3E PS2 load image:

    e2389ad9ca0fa28d6f294bb75b209467c6d907ec18e01b16cbd011908d0026db
    448264 bytes

Difference:

    +1792 bytes

This is therefore a new deterministic DUT rather than a load-image-identical
structural rewrite.

Hardware checkpoint:

    REQUIRED

Next:

    M3F_hardware_validate_first_real_translation_unit_split

## M3F — first real translation-unit hardware qualification

The M3E `management_config + calibration_state` translation-unit split has
now passed its required first-real-TU hardware checkpoint.

The exact uninstrumented M3E DUT passed the five-mode physical matrix:

- 480i
- 480p-hires
- 576i
- 720p
- 1080i

Machine result: 5/5 PASS.
Physical operator result: PASS.
Startup mode remained 480p.
The PS2 VNC session remained live after the sweep.

An earlier startup attempt exposed a separate stalled Pi management-service
condition. The PS2-side diagnostic reached the management HTTP receive
boundary after a successful TCP connect and request send, while a Pi-local
management self-probe also timed out. Restarting only the management service
restored health. The unchanged uninstrumented M3E DUT then booted and passed
the hardware matrix.

M3 therefore continues with coarse real translation-unit decomposition.

## M3G — second coarse real translation unit

The complete remaining runtime envelope now compiles as a second real
translation unit while preserving the existing recursive semantic hierarchy.

- state shell: `ps2ip.c`
- state-shell text symbols: 0
- runtime TU: `ps2vnc_runtime.c`
- runtime semantic lines: 11933
- runtime text symbols: 80
- services TU: `ps2vnc_services.c`
- services semantic lines: 6379
- services text symbols: 60
- deterministic clean builds: byte-exact
- M3G ELF: `c65368d280f3104fd4a7ff37ef2df9615842fb0578b444f500a5425ad65ba967`
- M3G load image: `cfed368ec910dc2e43f0a40b47e39f3a75750fb1139d71c3a93dbb91c1c20ada`
- load relation to hardware-qualified M3E: `NEW_NONIDENTICAL_THREE_TU_LOAD_IMAGE`
- hardware requirement: `REQUIRED_NEW_LOAD_IMAGE`

The runtime leaf hierarchy remains a migration scaffold for the next recursive
compiler-boundary split.
