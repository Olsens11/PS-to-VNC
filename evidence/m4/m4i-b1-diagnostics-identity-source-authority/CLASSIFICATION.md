# M4I-B1 identity source authority

M4I-B1 moves the active build/runtime identity transport into the permanent
diagnostics subsystem.

The new whole ELF differs from M4H because source/debug/build metadata changed,
but the complete runtime-loaded PT_LOAD image is byte-for-byte identical to the
hardware-qualified M4H image.

Therefore:

- M4I-B1 is a new source/build identity;
- it is not a new runtime load image;
- no new hardware qualification is required solely for this extraction;
- direct hardware authority remains M4H;
- M4I-B1 inherits runtime qualification through exact PT_LOAD equivalence.

The historical working/b4a identity source remains preserved for the M0
byte-exact reproduction contract and is not the active current-build source.
