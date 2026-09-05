# PS-to-VNC Roadmap

## Current clean-reconstruction roadmap — 2026-09-05

The migration/extraction roadmap below remains historical. The active
clean-reconstruction ladder is now:

1. Issue #38 — controller/input semantic foundation — **COMPLETE**.
2. Issue #39 — keyboard/OSK/local foreground foundation — **COMPLETE**.
3. Pre-#40 Pi desktop prerequisite — establish a reproducible dedicated VNC
   session with known session/window-manager ownership, a visible launchable
   terminal, and a deterministic PS2-only route to that terminal.
4. Issue #40 — minimal Pi management/config transport seam.
5. Issue #41 — configuration, persistence, bindings, and settings.
6. Issue #27 — display modes and safe-area/calibration work.
7. Issue #25 — recovery/Refresh/provider behavior.
8. Issue #33 — profiling.
9. Issue #28 — compression/performance.

Later planned families include Issue #30 companion desktop maturation, Issue #31
routing, Issue #24 storage/import, Issue #32 bootstrap/reproducibility, Issue #26
Wayland/provider work, Issues #34/#35 media work, and Issue #36 1.0 integration.

The pre-#40 desktop prerequisite is intentionally smaller than Issue #30. It
exists because management/config work needs a defined reproducible companion
desktop target rather than the temporary TigerVNC/Openbox qualification fixture.

## Historical migration roadmap

The sections below preserve the roadmap that governed the earlier
behavior-preserving migration. They are historical project records, not the
current next-action authority.

## Immediate

### M0 — establish successor working baseline

Reproduce the frozen B4A implementation as current PS-to-VNC working build
authority without modifying the immutable baseline.

### Define exact extraction stages

Convert the frozen dependency graph into M1 through Mn operations with exact
function/state ownership and documentation deliverables.

### Mechanical modularization

Move code in small behavior-preserving stages.

Do not combine refactoring, optimization or new feature work with extraction.

## After modular baseline

### Internal RFB-to-GS profiler

Add low-overhead subsystem timing/counter instrumentation as the first major
new feature in the modular tree.

Use it to localize the high-resolution RFB backpressure previously measured in
the historical project.

### Dedicated media path

Develop a PS-friendly media transport/render path, including MPEG-2 where
appropriate, rather than forcing moving video through ordinary RFB updates.

### Pi Cast Gateway

Expose the Pi as a discoverable local-network casting/media target and convert
received sessions into the dedicated PS media path.

## Continuing requirement

Documentation moves in lockstep with implementation.

No completed source stage may leave the living source map or project state
behind.

## Post-migration project maturation

After the behavior-preserving migration and modular regression are complete,
PS-to-VNC will undergo a dedicated project-maturation pass.

That pass may include:

- source formatting and whitespace normalization;
- consistent naming and file conventions;
- cleaner public and private APIs;
- removal of temporary migration scaffolding;
- consolidation of generated analysis and development artifacts;
- compiler-warning cleanup;
- build-system cleanup;
- coding and formatting policy;
- automated validation and CI where practical;
- test organization;
- `.gitignore` and artifact-retention policy;
- licensing and project metadata;
- contributor/developer documentation;
- release/versioning conventions;
- packaging and reproducible release builds;
- general repository organization suitable for a maintained project.

Historical provenance must remain recoverable.

Byte-exact frozen evidence should not be silently rewritten merely to make it
look cleaner. It may later be moved, packaged, or archived separately once its
identity and provenance remain durably preserved.

The working implementation and derived project artifacts are free to be
cleaned and normalized once migration no longer depends on byte-for-byte
comparison with the historical baseline.
