# M4I-C2 HW2 identity-serialization source authority

This authority records the source correction after the invalid M4I-C2-HW1
hardware attempt.

HW1 proved that the stamped ELF contained the full 64-character identity
digest, while the runtime identity datagram was truncated. The TestKit
correctly rejected that packet and granted no hardware qualification.

The successor source now constructs the runtime identity packet by exact
bounded byte serialization rather than printf-family formatting.

The production formatter is host-tested for:

- the current 107-byte identity message;
- the historical 108-byte identity message;
- a maximum legal 160-byte identity message;
- exactly 64 hexadecimal digest characters;
- fail-closed insufficient capacity;
- fail-closed malformed digest input.

The repaired ELF was reproduced byte-for-byte twice.

This source-authority record does not activate HW2 and does not replace any
current machine, validated-runtime, or direct-hardware authority. HW2 remains
subject to a fresh identity stamp, PT_LOAD characterization, activation, and
direct hardware qualification.
