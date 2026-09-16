# Foreman shift — A004 calibration separation invariant

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T19:24:17-04:00
COMPLETED_AT=2026-09-16T19:25:38-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation-calibration
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=64f05d06b92ff9b060e29389cb52417ffe39db16
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Preserve an explicit future-A004 packetization invariant separating the older DESKTOP CALIBRATION lineage from the later H1 MPEG CALIBRATION lineage before any A004 Reconstruction packet is issued.

Authority inspected:

- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001;
- frozen H1 source commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- `experiments/media-harness-h1/mpeg_presentation_calibration/` at that frozen authority;
- current branch authority at shift start and immediately before each write.

The A004 audit independently confirms MPEG-specific base/inner/outer geometry authority, accept-to-first-frame protection, exact-generation preparation, suppression ownership, and first-valid-physically-presented-frame promotion. The frozen H1 tree contains a dedicated `mpeg_presentation_calibration/` module. These facts support preserving the MPEG calibration lineage separately rather than deriving it from older desktop screen-fit/display-profile code.

## Work performed

Created:

`docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md`

Commit:

`b06e34ee8389e7427fd4d6fa7ba3a6cc80223f8a` — `docs(a004): preserve desktop versus MPEG calibration separation`

The invariant defines:

- **DESKTOP CALIBRATION** as the older safe-desktop/display-profile/screen-fit system for ordinary desktop/display geometry;
- **MPEG CALIBRATION** as the later H1 MPEG-presentation calibration lineage owning generation-specific MPEG base rectangle, inner matte, outer/suppression footprint, accepted generation geometry, accept/prepare protection, and the first-physically-presented-frame transition where A004/A003 authority requires it;
- a prohibition on sourcing MPEG-calibration behavior from the desktop-calibration implementation merely because both use rectangles or similar UI mechanics;
- `experiments/media-harness-h1/mpeg_presentation_calibration/` plus A004 audit revision 0001 as mandatory MPEG-calibration tracing authority;
- fail-closed ambiguity handling: if repository evidence does not prove a relationship, preserve separation and flag the ambiguity rather than merge the systems;
- mandatory **DESKTOP CALIBRATION** / **MPEG CALIBRATION** terminology in future A004 worker packets wherever lineage or geometry ownership could be ambiguous;
- explicit future A004 worker acceptance criteria preserving separate state, geometry authority, lineage, and lifecycle meaning.

Updated the exhaustive compatibility documentation index so the new invariant is mechanically discoverable:

`6568bc3d2aada2121488b5cb158053eef75dd168` — `docs(a004): index calibration separation invariant`

No A004 product behavior was implemented, no current A003 packet was altered, and no existing audit authority was rewritten.

## Required future A004 packet invariant

Every future A004 Reconstruction packet touching calibration must explicitly carry:

**CALIBRATION SEPARATION INVARIANT:** DESKTOP CALIBRATION and MPEG CALIBRATION are separate historical systems with separate state, geometry authority, ownership, and evidence. Do not derive MPEG-calibration behavior from the older desktop-calibration implementation merely because both manipulate screen rectangles or use similar UI mechanics. For MPEG calibration, trace `experiments/media-harness-h1/mpeg_presentation_calibration/` and A004 audit authority. If any relationship is unclear, preserve the separation and flag the ambiguity rather than merging the systems.

Future acceptance criteria must separately prove, as applicable, DESKTOP CALIBRATION authority preservation and MPEG CALIBRATION authority reconstruction, including distinct MPEG base/inner/outer meanings and acceptance-versus-first-physical-presentation separation.

## Evidence / status

This was a documentation/packetization-authority preservation shift only. No hardware or product Validation claim is made.

PENDING_LOCAL=future A004 packetization must consume `LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` explicitly; any subsequent documentation/CI check result for these two documentation commits
HARDWARE_PENDING=unchanged; no A004 physical qualification performed

## Exact next pickup

Continue the currently active A003 work from current repository authority. Before the Foreman issues any A004 Reconstruction packet, read and carry `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` into the worker instructions and acceptance criteria. Explicitly label **DESKTOP CALIBRATION** and **MPEG CALIBRATION** wherever ambiguity is possible. For MPEG behavior, trace the frozen H1 `mpeg_presentation_calibration/` lineage and A004 audit; preserve and report ambiguity rather than merging calibration systems.
