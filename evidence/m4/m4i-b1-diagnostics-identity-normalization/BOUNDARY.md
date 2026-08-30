# M4I-B1 diagnostics identity normalization

The active build/runtime identity transport now belongs to the permanent
`diagnostics` subsystem.

Current build ownership:

    src/diagnostics/identity.c
    src/diagnostics/identity.h
    -> pstvnc_diagnostics_identity.o

The historical file:

    working/b4a/ps2vnc_identity.c

is intentionally retained unchanged because it remains an input to the
historical M0 byte-exact reproduction procedure. It is no longer an active
object in the current canonical build.

The established linker `--wrap=sendto` behavior and fixed
`PS2VNCIDv1!BLOB!` TestKit contract are preserved.

The M4I-B1 ELF is reproducible across two canonical builds. Its complete
PT_LOAD content is byte-for-byte identical to the hardware-qualified M4H
PT_LOAD image.

Therefore M4I-B1 does not create a new runtime DUT and does not require a new
hardware qualification solely for this extraction.
