/*
 * File synopsis:
 * Owns GS/dmaKit resources for fixed Standard 480p initialization, complete
 * desktop presentation, one optional local overlay, synchronized flips, and
 * shutdown.
 *
 * Upstream code deals only in semantic project presentation surfaces.
 * gsKit texture formats, VRAM allocation, upload ordering, and draw depth stay
 * private to this PS2 platform owner.
 *
 * Context:
 *   docs/reconstruction/ISSUE7_MINIMAL_CORE.md;
 *   docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms";
 *   GitHub Issue #39 local presentation integration.
 */

#include <dmaKit.h>
#include <gsKit.h>

#include <stddef.h>
#include <string.h>

#include "../display.h"
#include "ps2_graphics.h"

static GSGLOBAL *display;

static GSTEXTURE desktop_texture;
static int desktop_texture_configured;

static GSTEXTURE local_overlay_texture;
static int local_overlay_texture_configured;
static unsigned int local_overlay_width;
static unsigned int local_overlay_height;

static void configure_desktop_texture(
    const uint16_t *desktop_pixels)
{
    memset(
        &desktop_texture,
        0,
        sizeof(desktop_texture));

    desktop_texture.Width =
        PSTVNC_DISPLAY_WIDTH;

    desktop_texture.Height =
        PSTVNC_DISPLAY_HEIGHT;

    desktop_texture.PSM =
        GS_PSM_CT16;

    desktop_texture.Mem =
        (u32 *)desktop_pixels;

    desktop_texture.Filter =
        GS_FILTER_NEAREST;

    desktop_texture.VramClut = 0;

    /*
     * Allocate the desktop texture's VRAM address once. Each presentation
     * uploads new EE-side pixels into the same allocation.
     */
    desktop_texture.Vram =
        gsKit_vram_alloc(
            display,
            gsKit_texture_size(
                desktop_texture.Width,
                desktop_texture.Height,
                desktop_texture.PSM),
            GSKIT_ALLOC_USERBUFFER);

    desktop_texture_configured = 1;
}

static int local_overlay_is_valid(
    const pstvnc_ps2_graphics_overlay_t *overlay)
{
    size_t expected_pixels;

    if (overlay == NULL)
        return 1;

    if (overlay->pixels == NULL ||
        overlay->width == 0u ||
        overlay->height == 0u ||
        overlay->width >
            PSTVNC_DISPLAY_WIDTH ||
        overlay->height >
            PSTVNC_DISPLAY_HEIGHT)
        return 0;

    if (overlay->x >
            PSTVNC_DISPLAY_WIDTH -
            overlay->width ||
        overlay->y >
            PSTVNC_DISPLAY_HEIGHT -
            overlay->height)
        return 0;

    expected_pixels =
        (size_t)overlay->width *
        (size_t)overlay->height;

    if (overlay->pixel_count !=
        expected_pixels)
        return 0;

    return 1;
}

static int configure_local_overlay_texture(
    const pstvnc_ps2_graphics_overlay_t *overlay)
{
    if (overlay == NULL)
        return 0;

    if (local_overlay_texture_configured) {
        if (overlay->width !=
                local_overlay_width ||
            overlay->height !=
                local_overlay_height)
            return -1;

        local_overlay_texture.Mem =
            (u32 *)overlay->pixels;

        return 0;
    }

    memset(
        &local_overlay_texture,
        0,
        sizeof(local_overlay_texture));

    local_overlay_texture.Width =
        overlay->width;

    local_overlay_texture.Height =
        overlay->height;

    local_overlay_texture.PSM =
        GS_PSM_CT16;

    local_overlay_texture.Mem =
        (u32 *)overlay->pixels;

    local_overlay_texture.Filter =
        GS_FILTER_NEAREST;

    local_overlay_texture.VramClut = 0;

    local_overlay_texture.Vram =
        gsKit_vram_alloc(
            display,
            gsKit_texture_size(
                local_overlay_texture.Width,
                local_overlay_texture.Height,
                local_overlay_texture.PSM),
            GSKIT_ALLOC_USERBUFFER);

    local_overlay_width =
        overlay->width;

    local_overlay_height =
        overlay->height;

    local_overlay_texture_configured = 1;
    return 0;
}

int pstvnc_ps2_graphics_init(void)
{
    if (display != NULL)
        return 0;

    dmaKit_init(
        D_CTRL_RELE_OFF,
        D_CTRL_MFD_OFF,
        D_CTRL_STS_UNSPEC,
        D_CTRL_STD_OFF,
        D_CTRL_RCYC_8,
        1 << DMA_CHANNEL_GIF);

    dmaKit_chan_init(
        DMA_CHANNEL_GIF);

    display = gsKit_init_global();

    if (display == NULL)
        return -1;

    /*
     * Issue #7 fixes one known-safe presentation contract: physical DTV 480p
     * with a 704x462 logical desktop. Mode selection, calibration, and
     * transaction/rollback policy intentionally remain outside this baseline.
     */
    display->Mode =
        GS_MODE_DTV_480P;

    display->Interlace =
        GS_NONINTERLACED;

    display->Field =
        GS_FRAME;

    display->Width =
        PSTVNC_DISPLAY_WIDTH;

    display->Height =
        PSTVNC_DISPLAY_HEIGHT;

    display->PSM =
        GS_PSM_CT32;

    display->PSMZ =
        GS_PSMZ_32;

    display->DoubleBuffering =
        GS_SETTING_ON;

    display->ZBuffering =
        GS_SETTING_OFF;

    display->PrimAlphaEnable =
        GS_SETTING_OFF;

    gsKit_vram_clear(display);
    gsKit_init_screen(display);

    gsKit_set_display_offset(
        display,
        PSTVNC_DISPLAY_OFFSET_X,
        PSTVNC_DISPLAY_OFFSET_Y);

    gsKit_mode_switch(
        display,
        GS_ONESHOT);

    gsKit_set_clamp(
        display,
        GS_CMODE_CLAMP);

    desktop_texture_configured = 0;

    memset(
        &desktop_texture,
        0,
        sizeof(desktop_texture));

    local_overlay_texture_configured = 0;
    local_overlay_width = 0u;
    local_overlay_height = 0u;

    memset(
        &local_overlay_texture,
        0,
        sizeof(local_overlay_texture));

    return 0;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay)
{
    const u64 clear_color =
        GS_SETREG_RGBAQ(
            0x00,
            0x00,
            0x00,
            0x80,
            0x00);

    const u64 texture_color =
        GS_SETREG_RGBAQ(
            0x80,
            0x80,
            0x80,
            0x80,
            0x00);

    if (display == NULL ||
        desktop_pixels == NULL ||
        desktop_pixel_count !=
            PSTVNC_DISPLAY_PIXEL_COUNT ||
        !local_overlay_is_valid(
            local_overlay))
        return -1;

    if (!desktop_texture_configured)
        configure_desktop_texture(
            desktop_pixels);
    else
        desktop_texture.Mem =
            (u32 *)desktop_pixels;

    if (configure_local_overlay_texture(
            local_overlay) < 0)
        return -1;

    /*
     * Upload complete coherent CPU-side surfaces before queueing the frame.
     * Remote desktop authority remains outside this module and is never
     * modified by local overlay rendering.
     */
    gsKit_texture_upload(
        display,
        &desktop_texture);

    if (local_overlay != NULL) {
        local_overlay_texture.Mem =
            (u32 *)local_overlay->pixels;

        gsKit_texture_upload(
            display,
            &local_overlay_texture);
    }

    gsKit_clear(
        display,
        clear_color);

    gsKit_prim_sprite_texture(
        display,
        &desktop_texture,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        (float)PSTVNC_DISPLAY_WIDTH,
        (float)PSTVNC_DISPLAY_HEIGHT,
        (float)PSTVNC_DISPLAY_WIDTH,
        (float)PSTVNC_DISPLAY_HEIGHT,
        1,
        texture_color);

    if (local_overlay != NULL) {
        const float x0 =
            (float)local_overlay->x;

        const float y0 =
            (float)local_overlay->y;

        const float x1 =
            (float)(
                local_overlay->x +
                local_overlay->width);

        const float y1 =
            (float)(
                local_overlay->y +
                local_overlay->height);

        gsKit_prim_sprite_texture(
            display,
            &local_overlay_texture,
            x0,
            y0,
            0.0f,
            0.0f,
            x1,
            y1,
            (float)local_overlay->width,
            (float)local_overlay->height,
            2,
            texture_color);
    }

    /*
     * Queue execution submits both surfaces as one coherent presentation.
     * The synchronized flip is the application-visible completion boundary.
     */
    gsKit_queue_exec(display);
    gsKit_sync_flip(display);

    return 0;
}

void pstvnc_ps2_graphics_shutdown(void)
{
    if (display != NULL)
        gsKit_deinit_global(display);

    display = NULL;

    desktop_texture_configured = 0;

    memset(
        &desktop_texture,
        0,
        sizeof(desktop_texture));

    local_overlay_texture_configured = 0;
    local_overlay_width = 0u;
    local_overlay_height = 0u;

    memset(
        &local_overlay_texture,
        0,
        sizeof(local_overlay_texture));
}
