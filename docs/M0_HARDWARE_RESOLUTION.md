# M0 Hardware Validation Resolution

## Decision

Mandatory new hardware regression for M0:

    NOT REQUIRED

M0 did not perform a new hardware test.

This document must not be interpreted as a new physical hardware PASS.

## Basis

The controlled M0 reproduction generated an ELF with SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

That SHA256 is exactly the frozen historical B4A ELF SHA256.

The reproduced and frozen executables compare byte-for-byte equal.

Therefore the M0 reproduction did not create a new executable DUT identity.

## Historical hardware evidence

D17AL-F8J2-B4A already has historical hardware-validation evidence recorded by
the project.

Because M0 regenerated the same executable bytes, that historical evidence
continues to apply to the reproduced executable identity.

This is inherited evidence for an identical DUT, not a new M0 hardware
observation.

## Physical-observation rule

Machine equivalence and physical observations remain distinct.

M0F makes only this statement:

    no new hardware regression is required to establish reproduction
    equivalence because the executable bytes are identical.

M0F does not assert that a new launch, display transition, controller test, or
other physical event occurred during this stage.

## Optional confidence launch

A confidence launch may still be performed later if operational reassurance is
useful.

Such a launch would be a smoke test of deployment and current apparatus
conditions.

It is not required to establish M0 Outcome A and is not required to close M0.

## M0 consequence

The hardware-validation requirement for the reproduction stage is resolved.

M0 may proceed to M0G closure.

Before any later modularized build is accepted as equivalent, that new build
must be treated according to its own DUT identity and validation contract.
