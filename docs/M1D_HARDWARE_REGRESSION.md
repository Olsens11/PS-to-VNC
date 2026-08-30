# M1D Hardware Regression

## Status

M1D is in progress.

Verified PS2 deployment:

    COMPLETE

Hardware regression:

    NOT RUN

Physical result:

    NOT RUN

Machine result:

    NOT RUN

## Exact DUT

SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Bytes:

    2927748

Source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

## PS2 deployment

Unique forensic target:

    /mass/0/PS-to-VNC-M1D1-20260830-020419-26ae06ff.ELF

Rolling launch target:

    /mass/0/PS2VNC.ELF

Both PS2 copies were downloaded back after upload.

Unique readback SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Rolling readback SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Both exactly equal the canonical M1 DUT SHA256.

Deployment evidence:

    evidence/m1/m1d1-deployment/

Evidence manifest SHA256:

    3e59cd9a09d12d1b64338b465766762a8829b60f083249b21870ef839e1a591f

## Previous rolling target

SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Classification:

    KNOWN_M0_B4A

If the prior rolling target was not a known preserved binary, its bytes were
preserved in the deployment evidence directory before replacement.

## Hardware-validation boundary

Deployment is not a hardware PASS.

Launching the ELF is not a hardware PASS.

The machine benchmark and operator physical observations remain separate
authorities.

The required physical observations remain:

- every requested temporary mode visibly appears;
- every rollback visibly returns to 480p;
- cursor disappearance/return during transitions is observed;
- green/corrupt output, freeze, no-signal, or other anomalies are recorded.

Do not launch the rolling ELF until the M1D benchmark apparatus has been
prepared for the new DUT.

## Next

Prepare the M1D regression apparatus, then launch:

    mass:/0/PS2VNC.ELF
