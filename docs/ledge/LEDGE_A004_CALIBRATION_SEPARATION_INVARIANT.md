# A004 Calibration Separation Invariant

DOCUMENT=LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-16T19:24:17-04:00
TEMPORAL_CLASS=FOREMAN_PACKETIZATION_INVARIANT
TEMPORAL_SEMANTICS=GOVERNS_FUTURE_A004_PACKETIZATION
BASED_ON_A004_AUDIT_REVISION=0001
H1_FORENSIC_SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
STATUS=OPERATIONAL

## Purpose

Future A004 Reconstruction packetization must preserve a strict distinction between **DESKTOP CALIBRATION** and **MPEG CALIBRATION**. Similar visual/UI mechanics, rectangle editing, controller use, or screen-fitting concepts do not make these systems interchangeable and do not establish shared behavior authority.

If repository evidence leaves any relationship between the two lineages unclear, the ambiguity must remain explicit and the systems must remain separate. Do not merge them by inference.

## DESKTOP CALIBRATION

**DESKTOP CALIBRATION** is the older safe-desktop/display-profile/screen-fit calibration system for ordinary desktop/display geometry.

Its authority concerns ordinary desktop/display presentation geometry and the existing display-profile/screen-fit lineage. It does not become an authority for MPEG generation geometry, MPEG ownership, MPEG suppression, MPEG accept/preparation transitions, or first-physically-presented-frame behavior merely because it manipulates rectangles or uses similar UI controls.

Future A004 work must not source MPEG-calibration behavior from this older desktop-calibration implementation unless a specific repository authority explicitly proves a narrow relationship. Any such relationship must be named and bounded rather than assumed.

## MPEG CALIBRATION

**MPEG CALIBRATION** is the later H1 MPEG-presentation calibration lineage.

Its authority includes the generation-specific MPEG calibration state and geometry classified by A004, including:

- the MPEG **base rectangle** — the generation's exact capture/presentation region;
- the MPEG **inner matte** — PS2-local presentation-only inset state;
- the MPEG **outer/suppression footprint** — the generation-specific MPEG visual-ownership / RFB-suppression perimeter;
- the foreground calibration transaction and its accept/cancel semantics;
- accepted immutable generation geometry;
- accept-to-generation-preparation protection;
- the distinction between calibration acceptance/preparation and actual MPEG visual ownership;
- the first valid **physically presented** MPEG frame as the later ownership-promotion / first-real-presentation boundary where required by A004/A003 authority.

The primary MPEG-calibration forensic lineage for future reconstruction is:

`experiments/media-harness-h1/mpeg_presentation_calibration/`

at frozen H1 authority commit:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Future workers must trace that module and `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` before reconstructing MPEG-calibration behavior.

The A004 audit's base/inner/outer geometry authority, RFB-freeze/suppression ownership, accept-to-first-frame protection, exact-generation preparation, and first-physical-frame promotion are MPEG-calibration/presentation facts. They must not be silently reinterpreted through the older desktop screen-fit implementation.

## Required packet terminology

Any future A004 Reconstruction packet must explicitly label calibration requirements as **DESKTOP CALIBRATION** or **MPEG CALIBRATION** wherever ambiguity is possible.

A packet must not use an unqualified phrase such as "calibration geometry", "calibration rectangle", "screen calibration", "accepted calibration", or "calibration state" when the intended lineage/owner could reasonably be unclear.

Examples:

- write **MPEG CALIBRATION base rectangle**, not merely "calibration rectangle", when referring to the generation-specific MPEG capture/presentation region;
- write **MPEG CALIBRATION outer/suppression footprint**, not "screen-fit bounds";
- write **DESKTOP CALIBRATION display-profile/screen-fit geometry** when referring to ordinary desktop/display calibration;
- write **MPEG CALIBRATION accepted generation geometry** when referring to the accept/prepare/first-physical-frame lifecycle.

## Future A004 Reconstruction packet invariant

Every A004 packet that touches calibration must carry the following invariant explicitly in its worker instructions:

> **CALIBRATION SEPARATION INVARIANT:** DESKTOP CALIBRATION and MPEG CALIBRATION are separate historical systems with separate state, geometry authority, ownership, and evidence. Do not derive MPEG-calibration behavior from the older desktop-calibration implementation merely because both manipulate screen rectangles or use similar UI mechanics. For MPEG calibration, trace `experiments/media-harness-h1/mpeg_presentation_calibration/` and A004 audit authority. If any relationship is unclear, preserve the separation and flag the ambiguity rather than merging the systems.

## Future A004 acceptance criteria

Every A004 packet that touches calibration must include acceptance criteria proving, as applicable:

1. **DESKTOP CALIBRATION authority remains separate.** Ordinary safe-desktop/display-profile/screen-fit geometry is neither overwritten nor promoted into MPEG generation authority.
2. **MPEG CALIBRATION authority is sourced from the correct lineage.** MPEG base/inner/outer geometry and calibration lifecycle are reconstructed from the A004 audit plus frozen `mpeg_presentation_calibration/` evidence, not inferred from desktop calibration.
3. **No shared geometry store is invented solely because both systems use rectangles.** Any bridge between them must have explicit evidence and narrow semantics.
4. **MPEG CALIBRATION base, inner matte, and outer/suppression footprint remain distinct meanings.** They must not be collapsed into a generic screen-fit rectangle.
5. **MPEG CALIBRATION acceptance remains distinct from first physical presentation.** Accept/prepare does not by itself become the first-physically-presented-frame or MPEG ownership-promotion boundary.
6. **Ambiguity fails closed toward separation.** When repository evidence does not prove a relationship between DESKTOP CALIBRATION and MPEG CALIBRATION, the implementation/packet keeps them separate and records the unresolved relationship instead of merging them.
7. **Terminology is explicit in code/tests/docs where ownership could be confused.** New A004 work uses DESKTOP CALIBRATION or MPEG CALIBRATION qualifiers where necessary to make lineage and authority unambiguous.

## Relationship to A004 audit

This document does not replace, rewrite, or narrow `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md`. The A004 audit remains behavior authority for the presentation/calibration tranche. This invariant governs how the Foreman and Reconstruction workers interpret and packetize calibration so that the older desktop-calibration lineage is not accidentally substituted for the H1 MPEG-calibration lineage.

Where the A004 audit says simply "calibration" in MPEG presentation context, future packetization must resolve that language against the audit's own base/inner/outer generation geometry, accept-to-first-frame protection, suppression ownership, and the frozen `mpeg_presentation_calibration/` forensic evidence rather than the unrelated older desktop calibration implementation.
