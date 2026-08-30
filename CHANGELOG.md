# Changelog

All notable PS-to-VNC product changes are recorded here.

The project uses Semantic Versioning for product releases. Engineering
milestones such as M3Z and M4A remain separate evidence identifiers and do
not replace product versions.

## Unreleased

Development after the first versioned baseline is focused on architectural
normalization before substantial new feature work.

### Architecture normalization

- Normalized configuration text helpers into the first permanent
  `src/config/` module.
- Introduced the permanent `pstvnc_` module API naming convention in
  compiled application code.
- Added host-side C unit-test infrastructure.
- Added the current repository-level reproducible PS2 build script.
- Confirmed the normalized build retains the exact M4A-qualified PS2
  PT_LOAD image.

## 0.1.0-alpha.1 - 2026-08-30

### Baseline

- Established the first formally versioned PS-to-VNC development baseline.
- Completed the coarse structural migration from the inherited monolithic
  implementation.
- Established twelve real translation units.
- Confirmed zero remaining coarse-migration blockers under the M3W
  completion rule.
- Hardware-qualified the exact baseline ELF on a physical PlayStation 2.
- Passed the five-mode display sequence:
  480i, 480p-hires, 576i, 720p, and 1080i.
- Confirmed successful restoration to the 480p baseline after every
  temporary mode.
- Preserved the exact qualified ELF while beginning architectural
  normalization.

### Architecture

- Architectural normalization becomes the next development phase.
- Future decomposition is based on responsibility, ownership, interfaces,
  and dependency direction rather than source-file line count.
- Migration `.inc` partitions remain temporary compatibility scaffolding,
  not the desired permanent architecture.
