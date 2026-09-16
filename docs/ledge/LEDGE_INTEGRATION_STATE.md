# Ledge Reconstruction Integration / Evidence — Lane State

DOCUMENT=LEDGE_INTEGRATION_STATE
STATE_REVISION=0001
RECORDED_AT=2026-09-15T21:24:14-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0004
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0011
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This state owns non-behavioral reconstruction integration/evidence continuity only. It does not supersede reconstruction product-source authority, audit dispositions, validation findings, or global continuity authority.

## Current integration phase

`A001_BRIDGE_INTEGRATION_EVIDENCE_BACKLOG`

## Lane boundary

Integration/Evidence is not a third product-behavior writer and is not Validation. It owns repository plumbing and proof preparation around already-reconstructed behavior: host-test registration, build/linkage inclusion that does not define semantics, symbol dictionaries, source topology/checker/portal integration, canonical executable repository checks when available, reproducibility/identity evidence scaffolding, and next-tranche preflight analysis when the immediate integration queue is empty.

If integration work exposes a likely product-source defect, record exact evidence and hand it to Reconstruction A/B. Do not silently repair behavior in this lane.

## Initial A001 backlog

Highest-priority safe integration work visible at lane creation:

1. wire `tests/unit/transport_bridge_test.c` and `tests/unit/rfb_bridge_test.c` into canonical host-unit entry points without altering DUT behavior;
2. complete `src/rfb/SYMBOLS.md` coverage for `bridge.h/.c` and continue definition-level completion of `src/transport/SYMBOLS.md`;
3. deliberately adopt `src/transport` into `docs/development/source-topology.md`, applicable checker/domain allowlists, and the generated source-dictionary portal using canonical tooling rather than hand-manufacturing generated compliance;
4. run and preserve any repository-native host/static checks executable from the worker surface, clearly separating actual PASS from `PENDING_LOCAL` PS2DEV/toolchain work;
5. prepare clean build/linkage/reproducibility evidence plumbing for A001 as the live source path becomes coherent;
6. if the above queue becomes temporarily exhausted, preflight the next dependency-permitted reconstruction work from forensic H1/current call-chain/config-owner evidence without implementing product behavior.

## Current evidence boundary

A001 is not `VALIDATION_READY`. V003/V004 remain open validation authority. No host test, PS2DEV compile, canonical clean DUT build, ELF/PT_LOAD identity, or hardware PASS is created merely by establishing this lane.

Unknown external Pi-local dirty work remains outside the GitHub-native mutation surface and must neither be overwritten nor declared absent.

## Exact next pickup

Start with A001 bridge-test registration and current dictionary/topology adoption. Re-read current branch HEAD, reconstruction state, newest reconstruction/validation handoffs, and contract revision 0004 before each write. Work only on non-behavioral integration/evidence surfaces and pivot rather than collide if Reconstruction A/B is touching the same file or responsibility.
