# PS-to-VNC File and Service Map

This file is the canonical living location map for the successor project.

Update it whenever canonical code ownership or file location changes.

## Repositories

| Role | Location | Mutable by PS-to-VNC work? |
|---|---|---|
| Historical PS2VNC authority | `/home/ps2/ps2vnc` | No |
| Current PS-to-VNC development | `/home/ps2/PS-to-VNC` | Yes |

## Historical backup

Legacy Git bundle:

`/home/ps2/ps2vnc-legacy-backups/ps2vnc-legacy-allrefs-20260830-002434.bundle`

SHA256:

`7f018a3623c5cdd7e7ed30bfbc3f00040b632735fc83f92d49c5a0c672bea85a`

## Baseline

| Responsibility | Current location | Owner |
|---|---|---|
| Frozen B4A implementation | `baseline/frozen-b4a/` | immutable historical reference |
| Frozen B4A source | `baseline/frozen-b4a/ps2ip.c` | immutable historical reference |
| Dependency analysis | `baseline/analysis/B4A-dependency-map/` | immutable analysis reference |
| Baseline identity | `baseline/IDENTITY.env` | project provenance |

## Working source

Current working source:

**NONE — M0 has not started.**

No file under `baseline/frozen-b4a/` is a writable modular-development
location.

## Documentation

| Responsibility | Location |
|---|---|
| Documentation entry point | `docs/INDEX.md` |
| Current project state | `docs/PROJECT_STATE.md` |
| Modularization ledger | `docs/MODULARIZATION.md` |
| Architecture | `docs/architecture/ARCHITECTURE.md` |
| Current file/service map | `docs/reference/FILE_AND_SERVICE_MAP.md` |
| Roadmap | `docs/ROADMAP.md` |
| Legacy relationship | `docs/LEGACY_HANDOFF.md` |
| Project identity | `docs/PROJECT_IDENTITY.md` |

## Runtime services

Historical currently deployed PS2VNC runtime remains external to this
successor repository during bootstrap.

PS-to-VNC does not yet own or deploy a runtime service.

That authority changes only through a later documented stage.

## Migration recovery authority

| Responsibility | Canonical location |
|---|---|
| Recovery entry point | `START_HERE.md` |
| Machine-readable migration state | `runtime/MIGRATION_STATE.env` |
| Human-readable migration state | `docs/MIGRATION_STATE.md` |
| Recovery summary command | `scripts/resume-state.sh` |
| Migration coherence gate | `scripts/migration-check.sh` |
| Documentation coherence gate | `scripts/docs-check.sh` |

These locations move in lockstep with implementation authority.

No modularization stage is complete until this map and the migration state
agree with the actual source tree.

## M0 build provenance authority

| Responsibility | Canonical location |
|---|---|
| M0 reproduction contract | `docs/M0_BASELINE_CONTRACT.md` |
| Historical build provenance | `docs/M0_BUILD_PROVENANCE.md` |
| Machine-readable M0 build authority | `runtime/M0_BUILD_AUTHORITY.env` |
| Frozen B4A implementation | `baseline/frozen-b4a/` |

The old `/home/ps2/ps2vnc/build/` directory is historical evidence only and is
not PS-to-VNC build authority.

## M0D writable authority

| Responsibility | Canonical location |
|---|---|
| Writable B4A application source | `working/b4a/` |
| Writable primary source | `working/b4a/ps2ip.c` |
| Controlled M0 build wrapper | `scripts/build-m0-b4a.sh` |
| M0 build provenance | `docs/M0_BUILD_PROVENANCE.md` |
| M0 build fingerprints | `runtime/M0_BUILD_AUTHORITY.env` |

`working/b4a/` begins byte-identical to the frozen B4A application inputs.

Generated objects, generated IRX C files, the copied build dependency, and the
ELF are build products rather than source authority.

## M0D2 reproduction evidence

| Responsibility | Canonical location |
|---|---|
| M0 reproduction result | `docs/M0_BUILD_RESULT.md` |
| M0D2 captured evidence | `evidence/m0/m0d2/` |
| Machine build-result authority | `runtime/M0_BUILD_AUTHORITY.env` |

The generated `working/b4a/PS2VNC.ELF` remains a build product.

The committed frozen B4A ELF remains the durable executable byte authority
because the M0D2 result proved the generated ELF byte-identical to it.

## M0 hardware-validation resolution

| Responsibility | Canonical location |
|---|---|
| M0 hardware-validation decision | `docs/M0_HARDWARE_RESOLUTION.md` |
| M0 reproduction result | `docs/M0_BUILD_RESULT.md` |
| Machine M0 reproduction authority | `runtime/M0_BUILD_AUTHORITY.env` |

The M0 hardware decision inherits historical B4A evidence only because the
reproduced final executable is byte-identical. It is not a new hardware PASS.
