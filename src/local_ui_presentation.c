/*
 * File synopsis:
 * Converts local foreground/UI state into a platform-neutral optional overlay
 * description without modifying remote desktop pixels.
 *
 * The historical OSK placement is retained as a product-layout invariant, not
 * as historical graphics implementation:
 *
 *   x = (704 - 600) / 2 = 52
 *   y = 462 - 178 - 6 = 278
 *
 * Multi-mode scaling and safe-area policy remain outside Issue #39 and belong
 * to the later display-mode/calibration workstream.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "display.h"
#include "local_ui_presentation.h"

#define PSTVNC_LOCAL_UI_BOTTOM_INSET 6u

static void clear_presentation(
    pstvnc_local_ui_presentation_t *presentation)
{
    if (presentation != NULL)
        memset(presentation, 0, sizeof(*presentation));
}

int pstvnc_local_ui_prepare_presentation(
    const pstvnc_local_ui_t *ui,
    const pstvnc_osk_t *osk,
    uint16_t *overlay_surface_pixels,
    size_t overlay_surface_capacity_pixels,
    pstvnc_local_ui_presentation_t *presentation)
{
    unsigned int horizontal_space;

    if (presentation == NULL)
        return 0;

    clear_presentation(presentation);

    if (ui == NULL)
        return 0;

    presentation->generation =
        pstvnc_local_ui_generation(ui);

    if (ui->foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP) {

        return 1;
    }

    if (ui->foreground !=
        PSTVNC_LOCAL_UI_FOREGROUND_OSK) {

        return 0;
    }

    if (osk == NULL ||
        overlay_surface_pixels == NULL ||
        overlay_surface_capacity_pixels <
            PSTVNC_OSK_SURFACE_PIXEL_COUNT)
        return 0;

    if (PSTVNC_OSK_WIDTH >
            PSTVNC_DISPLAY_WIDTH ||
        PSTVNC_OSK_HEIGHT +
            PSTVNC_LOCAL_UI_BOTTOM_INSET >
            PSTVNC_DISPLAY_HEIGHT)
        return 0;

    if (!pstvnc_osk_render_surface(
            osk,
            overlay_surface_pixels,
            overlay_surface_capacity_pixels))
        return 0;

    horizontal_space =
        PSTVNC_DISPLAY_WIDTH -
        PSTVNC_OSK_WIDTH;

    presentation->overlay_visible = 1;
    presentation->overlay_pixels =
        overlay_surface_pixels;

    presentation->overlay_pixel_count =
        PSTVNC_OSK_SURFACE_PIXEL_COUNT;

    presentation->overlay_width =
        PSTVNC_OSK_WIDTH;

    presentation->overlay_height =
        PSTVNC_OSK_HEIGHT;

    presentation->overlay_x =
        horizontal_space / 2u;

    presentation->overlay_y =
        PSTVNC_DISPLAY_HEIGHT -
        PSTVNC_OSK_HEIGHT -
        PSTVNC_LOCAL_UI_BOTTOM_INSET;

    return 1;
}
