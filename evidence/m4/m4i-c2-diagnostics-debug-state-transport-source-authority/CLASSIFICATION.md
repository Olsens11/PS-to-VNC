# M4I-C2 source authority classification

M4I-C2 moves permanent debug-stage state and UDP diagnostic transport ownership
into `src/diagnostics/debug.c` / `debug.h`.

The source/build transaction is reproducible and its TestKit identity contract
passes.

Unlike M4I-B1, the M4I-C2 PT_LOAD image is not byte-identical to the preceding
hardware-qualified generation. The load image grew by 128 bytes.

Therefore:

- M4I-C2 is a new DUT generation.
- Its source and exact build identity may be made authoritative before testing.
- It must not be promoted as the current validated runtime until direct PS2
  hardware qualification succeeds.
- M4I-B1 remains the current validated runtime in the meantime.
- No rebuild or ELF substitution is authorized between source authority and
  hardware qualification.
