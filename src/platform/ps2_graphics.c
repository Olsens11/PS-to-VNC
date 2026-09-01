/*
 * File synopsis:
 * Owns GS/dmaKit resources for fixed Standard 480p initialization, complete
 * CT16 texture presentation, synchronized flips, and shutdown.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Fixed Standard 480p
 * presentation"; docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms".
 */

#include <dmaKit.h>
#include <gsKit.h>

#include <stddef.h>
#include <string.h>

#include "../display.h"
#include "ps2_graphics.h"

/*
 * This module owns the PS2 presentation resources. The application supplies
 * completed pixels, but it neither allocates GS VRAM nor manipulates gsKit
 * state directly.
 */
static GSGLOBAL *display;
static GSTEXTURE desktop_texture;
static int texture_configured;

static void configure_texture(const uint16_t *gs_pixels)
{
    memset(&desktop_texture, 0, sizeof(desktop_texture));

    desktop_texture.Width = PSTVNC_DISPLAY_WIDTH;
    desktop_texture.Height = PSTVNC_DISPLAY_HEIGHT;
    desktop_texture.PSM = GS_PSM_CT16;
    desktop_texture.Mem = (u32 *)gs_pixels;
    desktop_texture.Filter = GS_FILTER_NEAREST;
    desktop_texture.VramClut = 0;

    /*
     * Allocate the texture's VRAM address once. Each presentation uploads new
     * EE-side pixels into that same GS allocation rather than accumulating a
     * new texture allocation per framebuffer update.
     */
    desktop_texture.Vram = gsKit_vram_alloc(
        display,
        gsKit_texture_size(
            desktop_texture.Width,
            desktop_texture.Height,
            desktop_texture.PSM),
        GSKIT_ALLOC_USERBUFFER);

    texture_configured = 1;
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

    dmaKit_chan_init(DMA_CHANNEL_GIF);

    display = gsKit_init_global();
    if (display == NULL)
        return -1;

    /*
     * Issue #7 fixes one known-safe presentation contract: physical DTV 480p
     * with a 704x462 logical desktop. Mode selection, calibration, and
     * transaction/rollback policy intentionally remain outside this baseline.
     */
    display->Mode = GS_MODE_DTV_480P;
    display->Interlace = GS_NONINTERLACED;
    display->Field = GS_FRAME;
    display->Width = PSTVNC_DISPLAY_WIDTH;
    display->Height = PSTVNC_DISPLAY_HEIGHT;
    display->PSM = GS_PSM_CT32;
    display->PSMZ = GS_PSMZ_32;
    display->DoubleBuffering = GS_SETTING_ON;
    display->ZBuffering = GS_SETTING_OFF;
    display->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_vram_clear(display);
    gsKit_init_screen(display);
    gsKit_set_display_offset(
        display,
        PSTVNC_DISPLAY_OFFSET_X,
        PSTVNC_DISPLAY_OFFSET_Y);
    gsKit_mode_switch(display, GS_ONESHOT);
    gsKit_set_clamp(display, GS_CMODE_CLAMP);

    texture_configured = 0;
    memset(&desktop_texture, 0, sizeof(desktop_texture));
    return 0;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *gs_pixels,
    size_t pixel_count)
{
    const u64 clear_color =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);
    const u64 texture_color =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);

    if (display == NULL ||
        gs_pixels == NULL ||
        pixel_count != PSTVNC_DISPLAY_PIXEL_COUNT)
        return -1;

    if (!texture_configured)
        configure_texture(gs_pixels);
    else
        desktop_texture.Mem = (u32 *)gs_pixels;

    /*
     * Upload and draw a complete coherent desktop on every dirty update. This
     * deliberately favors a simple authoritative path over historical partial
     * or direct-to-GS optimizations that could make CPU and GS state disagree.
     */
    gsKit_texture_upload(display, &desktop_texture);

    gsKit_clear(display, clear_color);
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

    /*
     * Queue execution submits the draw; synchronized flip makes completion the
     * presentation boundary before the application reuses the EE-side buffer.
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
    texture_configured = 0;
    memset(&desktop_texture, 0, sizeof(desktop_texture));
}
