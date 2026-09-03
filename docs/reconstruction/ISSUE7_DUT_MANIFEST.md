# Issue #7 DUT Manifest Authority

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    ROLE=PRE_HARDWARE_DUT_PROVENANCE
    SOURCE_BRANCH=reconstruct/issue7-minimal-core
    HARDWARE_QUALIFICATION=NOT_RUN
    OPERATOR_RESULT=NOT_RUN

This document defines the successor-owned provenance record for the first clean
Issue #7 hardware DUT. It supplements `ISSUE7_HARDWARE_QUALIFICATION.md`; it does
not replace deployment evidence, runtime identity evidence, packet evidence, or
physical/operator observation.

The manifest exists to close one specific chain before hardware is touched:

    exact committed Issue #7 source
        -> pinned build inputs
        -> reproducible canonical clean linked ELF
        -> deterministic identity stamping
        -> exact stamped DUT ELF / PT_LOAD identity

It must never be interpreted as proof that the ELF was deployed or executed.

## Authority boundary

A real Issue #7 DUT manifest may be generated only when all of these are true:

1. the checkout is on `reconstruct/issue7-minimal-core`;
2. tracked source state is clean;
3. the canonical pristine ELF is exactly:

       build/reconstruction/issue7/PS-to-VNC-Issue7.ELF

4. that ELF has just passed the clean linked reproducibility gate;
5. `prepare-hardware-elf.sh` has produced one deterministic stamped DUT and a
   complete passing preparation log;
6. the pristine and stamped files still match the hashes/byte counts in that
   preparation evidence.

`scripts/testkit/issue7-dut-manifest.py` fails closed if those conditions are not
met. In particular, it does not accept an arbitrary ELF merely because the ELF
contains a valid historical identity blob.

This restriction is deliberate. The generic identity-stamping mechanism is
compatible with sealed historical artifacts, but an Issue #7 source-to-DUT
manifest must bind specifically to the current clean reconstruction build.

## Required sequence

From a clean checkout of the intended Issue #7 head:

```sh
./scripts/check-issue7-linked-reproducibility.sh

mkdir -p build/reconstruction/issue7/hardware

./scripts/testkit/prepare-hardware-elf.sh \
    build/reconstruction/issue7/PS-to-VNC-Issue7.ELF \
    ISSUE7-RAW480P-HW1 \
    build/reconstruction/issue7/hardware/PS-to-VNC-ISSUE7-RAW480P-HW1.ELF \
    > build/reconstruction/issue7/hardware/preparation.env

python3 scripts/testkit/issue7-dut-manifest.py \
    build/reconstruction/issue7/hardware/preparation.env \
    build/reconstruction/issue7/hardware/dut-manifest.env
```

The test ID shown above is the current recommended first-clean identifier from
`ISSUE7_HARDWARE_QUALIFICATION.md`. If the actual hardware experiment receives a
different unique ID, that exact ID must be used consistently in preparation,
manifesting, runtime identity observation, and the final evidence record.

Do not pre-commit a generated manifest with guessed DUT values.

## Manifest contents

The generated record includes:

- canonical repository and branch;
- exact 40-character Git source commit;
- canonical pristine build path;
- pinned PS2DEV container image digest;
- qualified PS2IP archive SHA256;
- SHA256 identities for the clean build script, linked reproducibility gate,
  clean Makefile, hardware-preparation tool, identity implementation, and
  PT_LOAD fingerprint tool;
- test ID;
- pristine whole-ELF SHA256 and byte count;
- pristine PT_LOAD SHA256 and byte count;
- stamped runtime identity digest;
- stamped whole-ELF SHA256 and byte count;
- stamped PT_LOAD SHA256 and byte count;
- measured PT_LOAD relationship across identity stamping;
- explicit deterministic-stamping result.

It also writes these non-qualification defaults deliberately:

    HARDWARE_QUALIFIED=NO
    DEPLOYED=NO
    RUNTIME_IDENTITY_OBSERVED=NO
    MACHINE_EVIDENCE_RESULT=NOT_RUN
    OPERATOR_RESULT=NOT_RUN

Those values prevent a preparation artifact from silently becoming a hardware
qualification record.

## CI proof

The Issue #7 `ps2-link` CI job exercises the same provenance boundary in this
order:

1. perform two independent clean linked builds and require byte-identical whole
   ELF and PT_LOAD results;
2. leave the second canonical clean build at the standard build path;
3. prepare a deterministic stamped self-test DUT from that exact ELF;
4. generate two manifests and require byte-identical manifest output;
5. verify source/build/preparation/non-qualification fields;
6. verify existing-output refusal;
7. alter the prepared stamped ELF after preparation and require manifest
   generation to fail;
8. only then preserve the unqualified clean ELF CI artifact.

This CI proof remains host/build evidence. It is not PS2 hardware evidence.

## Separation from the legacy hardware bridge

The successor repository now owns clean Issue #7 build identity, historical
identity compatibility, deterministic stamping/verification, PT_LOAD
fingerprinting, and DUT manifesting.

The frozen historical deployment/apparatus bridge remains a separate boundary.
Do not rewrite or generalize observer/deployment behavior from inference merely
to make the new manifest appear end-to-end. Deployment/readback and apparatus
provenance must continue through proven mechanics until a replacement is itself
supported by durable evidence and qualification.

Pre-GitHub development records may also exist only in the Windows archival
history. This manifest does not claim that GitHub was authoritative before the
project began recording work there; it binds the current clean reconstruction
only to its present Git authority.

## Promotion rule

A generated DUT manifest becomes one input to the hardware evidence package. It
is not edited in place to manufacture a PASS.

After the run, deployment/readback evidence, runtime identity, machine evidence,
and operator observations are recorded separately and compared against the
manifest. Issue #7 earns hardware authority only under the pass definition in
`ISSUE7_HARDWARE_QUALIFICATION.md`.
