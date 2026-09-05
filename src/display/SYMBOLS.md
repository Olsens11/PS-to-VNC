# Clean symbols — `src/display`

DIRECTORY=src/display
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns platform-neutral display conversion and presentation-facing desktop geometry/state used above the PS2 graphics platform seam.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_display_prepare_gs16 | function | src/display/display.c | display conversion | public | Converts an authoritative remote framebuffer into opaque GS CT16 pixels. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| framebuffer | parameter | src/display/display.c | pstvnc_display_prepare_gs16 | local | Supplies the valid authoritative remote image without permitting mutation. | authoritative framebuffer semantics |
| gs_pixels | parameter | src/display/display.c | pstvnc_display_prepare_gs16 | local | Supplies destination storage for disposable GS presentation pixels. | display presentation seam |
| gs_capacity_pixels | parameter | src/display/display.c | pstvnc_display_prepare_gs16 | local | Gives the available presentation-buffer capacity for bounds validation. | display presentation seam |
| i | variable | src/display/display.c | pstvnc_display_prepare_gs16 | local | Iterates exactly once over every pixel in the fixed logical desktop. | fixed 480p presentation |
| PSTVNC_DISPLAY_H | include guard | src/display/display.h | display conversion interface | file | Prevents repeated inclusion of fixed display-contract declarations. | clean source interface |
| PSTVNC_DISPLAY_PHYSICAL_WIDTH | macro | src/display/display.h | display contract | public | Names the 720-pixel physical width of Standard 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_PHYSICAL_HEIGHT | macro | src/display/display.h | display contract | public | Names the 480-line physical height of Standard 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_WIDTH | macro | src/display/display.h | display contract | public | Names the fixed 704-pixel Issue #7 logical desktop width. | ISSUE7_MINIMAL_CORE: Fixed first target |
| PSTVNC_DISPLAY_HEIGHT | macro | src/display/display.h | display contract | public | Names the fixed 462-line Issue #7 logical desktop height. | ISSUE7_MINIMAL_CORE: Fixed first target |
| PSTVNC_DISPLAY_OFFSET_X | macro | src/display/display.h | display contract | public | Names the fixed horizontal placement offset within physical 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_OFFSET_Y | macro | src/display/display.h | display contract | public | Names the fixed vertical placement offset within physical 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_PIXEL_COUNT | macro | src/display/display.h | display contract | public | Computes the exact logical desktop pixel count used for storage and presentation. | fixed Standard 480p presentation |
| pstvnc_display_prepare_gs16 | function declaration | src/display/display.h | display conversion interface | public | Declares non-destructive conversion from authoritative RFB pixels to GS CT16. | CLEAN_ARCHITECTURE: Display model and presentation |
| framebuffer | parameter | src/display/display.h | pstvnc_display_prepare_gs16 | public | Declares the authoritative framebuffer input to display conversion. | display presentation seam |
| gs_pixels | parameter | src/display/display.h | pstvnc_display_prepare_gs16 | public | Declares presentation destination storage for display conversion. | display presentation seam |
| gs_capacity_pixels | parameter | src/display/display.h | pstvnc_display_prepare_gs16 | public | Declares destination capacity used to reject undersized presentation storage. | display presentation seam |
