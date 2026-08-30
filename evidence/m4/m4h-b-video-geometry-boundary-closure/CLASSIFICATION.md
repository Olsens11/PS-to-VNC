# M4H-B video-geometry boundary closure

## Result

The pure `video/geometry` responsibility is complete.

M4H-A established the permanent host-testable safe-area and presentation
geometry model in `src/video/geometry.c` and `src/video/geometry.h`.

M4H-B then audited residual geometry-like responsibilities. The valid third
census and the subsequent boundary-confirmation inspection did not identify a
second coherent pure-geometry extraction.

Therefore:

    PURE_VIDEO_GEOMETRY_MODEL=COMPLETE
    ADDITIONAL_PURE_GEOMETRY_EXTRACTION=NO

## Evidence quality

Attempt 1 is preserved as an apparatus failure. `rg` was unavailable and the
fallback Python regular expression was invalid.

Attempt 2 is preserved as an invalid structural-parser result. Its line-level
inventories remain useful, but its zero-function structural census is invalid
and may not be used for boundary selection.

Attempt 3 is the valid self-tested census and is the mechanical evidence used
for M4H-B review.

## Residual responsibility classification

Residual coordinate/size-related code is not moved into `video/geometry`
merely because it performs arithmetic.

- interactive safe-area clamping belongs with calibration policy;
- preset application and `screen_fit_*` mutation belong with calibration/video
  state;
- HIRES effective raster/storage dimensions and mapping predicates belong with
  framebuffer/mapping/presentation responsibilities;
- framebuffer and RFB update paths remain outside pure geometry;
- GS drawing and transport remain presentation/platform mechanisms;
- calibration rendering remains calibration/UI/platform work;
- `display_live_desktop()` remains orchestration.

This classification follows `docs/STATE_OWNERSHIP.md`,
`docs/DEPENDENCY_RULES.md`, `docs/ARCHITECTURE.md`, and
`docs/TARGET_ARCHITECTURE.md`.

## Validation consequence

M4H-B is a classification/evidence closure only.

It changes no product source, executable, or PT_LOAD image. The existing
hardware-qualified M4H executable remains authoritative, so no build,
deployment, or hardware rerun is created solely for this closure.

## Next tranche

`docs/NORMALIZATION_SEQUENCE.md` defines diagnostics as the next normalization
tranche after video mode and pure geometry.

The next work is therefore a diagnostics boundary census before source
movement.
