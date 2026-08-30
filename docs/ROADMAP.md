# PS-to-VNC Roadmap

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
