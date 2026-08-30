# M4I-C2 diagnostics/debug boundary

This extraction assigns the first permanent runtime debug ownership to:

    src/diagnostics/debug.c
    src/diagnostics/debug.h

The permanent module owns:

- debug-stage enumeration, naming, and current stage state;
- UDP diagnostic socket and destination state;
- initialization and raw diagnostic datagram transport.

The migration-era runtime, calibration, and profiling formatters continue to own
the application-domain observations that they format. They call the diagnostics
transport API rather than importing or exposing the raw diagnostic socket.

This change intentionally does not move `profile_ticks_us()`. That helper is
also used for product timing and is not diagnostics-owned merely because of its
historical name.

The M4I-C2 build is reproducible, but its PT_LOAD image is not byte-identical to
M4I-B1. Therefore M4I-C2 is a new DUT generation and requires direct PS2
hardware qualification before it can be promoted as current validated runtime
authority.
