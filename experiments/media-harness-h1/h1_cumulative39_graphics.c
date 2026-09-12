/*
 * File synopsis:
 * Implements CP2P's single-owner RFB/MPEG/local-UI PS2 compositor.
 *
 * This is a direct evolution of the historical cumulative39 compositor from
 * generator commit 8d6f0ff. The unchanged through-Issue-39 graphics owner is
 * wrapped rather than modified. One semaphore serializes every GS presentation.
 * The latest remote desktop and local overlay remain cached, while MPEG RGB16
 * macroblocks are detiled into one texture.
 *
 * Composition order while MPEG is visible is deliberately:
 *   remote desktop
 *   -> black outer suppression footprint
 *   -> MPEG base rectangle
 *   -> black symmetric inner matte
 *   -> local UI / OSK overlay
 *
 * Thus remote desktop updates may continue outside the suppression footprint
 * without ever becoming visible from that footprint's outer edge inward.
 */

#define pstvnc_ps2_graphics_init pstvnc_ps2_graphics_init_base_c39
#define pstvnc_ps2_graphics_present pstvnc_ps2_graphics_present_base_c39
#define pstvnc_ps2_graphics_shutdown pstvnc_ps2_graphics_shutdown_base_c39
#include "../../src/platform/ps2_graphics.c"
#undef pstvnc_ps2_graphics_init
#undef pstvnc_ps2_graphics_present
#undef pstvnc_ps2_graphics_shutdown

#include "h1_cumulative39_graphics.h"

#include <kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define H1_VIDEO_MAX_WIDTH 704u
#define H1_VIDEO_MAX_HEIGHT 480u
#define H1_VIDEO_MAX_PIXELS (H1_VIDEO_MAX_WIDTH * H1_VIDEO_MAX_HEIGHT)

static GSTEXTURE h1_video_texture;
static int h1_video_texture_configured;
static unsigned int h1_video_source_width;
static unsigned int h1_video_source_height;
static int h1_video_visible;
static uint16_t h1_video_linear[H1_VIDEO_MAX_PIXELS]
    __attribute__((aligned(128)));

static pstvnc_h1_mpeg_start_contract_t h1_video_contract;
static const uint16_t *h1_latest_desktop;
static pstvnc_ps2_graphics_overlay_t h1_latest_overlay;
static int h1_latest_overlay_visible;
static int h1_graphics_sema = -1;

static int h1_create_graphics_mutex(void)
{
    ee_sema_t sema;

    memset(&sema, 0, sizeof(sema));
    sema.init_count = 1;
    sema.max_count = 1;
    return CreateSema(&sema);
}

static int h1_lock(void)
{
    return h1_graphics_sema >= 0 && WaitSema(h1_graphics_sema) >= 0;
}

static int h1_unlock(void)
{
    return h1_graphics_sema >= 0 && SignalSema(h1_graphics_sema) >= 0;
}

static int h1_contract_valid(
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    if (contract == NULL ||
        contract->draw_x < 0 || contract->draw_y < 0 ||
        contract->draw_width <= 0 || contract->draw_height <= 0 ||
        contract->inner_matte_x < 0 || contract->inner_matte_y < 0 ||
        contract->inner_matte_x > contract->draw_width / 2 ||
        contract->inner_matte_y > contract->draw_height / 2 ||
        contract->suppression_rect.x < 0 ||
        contract->suppression_rect.y < 0 ||
        contract->suppression_rect.width <= 0 ||
        contract->suppression_rect.height <= 0)
        return 0;

    if (contract->draw_x >
            (int)PSTVNC_DISPLAY_WIDTH - contract->draw_width ||
        contract->draw_y >
            (int)PSTVNC_DISPLAY_HEIGHT - contract->draw_height ||
        contract->suppression_rect.x >
            (int)PSTVNC_DISPLAY_WIDTH - contract->suppression_rect.width ||
        contract->suppression_rect.y >
            (int)PSTVNC_DISPLAY_HEIGHT - contract->suppression_rect.height)
        return 0;

    return 1;
}

static int h1_configure_video_texture(
    unsigned int width,
    unsigned int height)
{
    if (!h1_video_texture_configured) {
        /*
         * Allocate one maximum-size VRAM backing store once. Each immutable MPEG
         * generation may then choose a different valid source size without
         * reallocating/leaking VRAM or inheriting dimensions from its predecessor.
         */
        memset(&h1_video_texture, 0, sizeof(h1_video_texture));
        h1_video_texture.PSM = GS_PSM_CT16;
        h1_video_texture.Mem = (u32 *)h1_video_linear;
        h1_video_texture.Filter = GS_FILTER_NEAREST;
        h1_video_texture.VramClut = 0;
        h1_video_texture.Vram = gsKit_vram_alloc(
            display,
            gsKit_texture_size(
                H1_VIDEO_MAX_WIDTH,
                H1_VIDEO_MAX_HEIGHT,
                GS_PSM_CT16),
            GSKIT_ALLOC_USERBUFFER);
        h1_video_texture_configured = 1;
    }

    h1_video_texture.Width = width;
    h1_video_texture.Height = height;
    h1_video_texture.Mem = (u32 *)h1_video_linear;
    h1_video_source_width = width;
    h1_video_source_height = height;
    return 0;
}

static void h1_detile_macroblocks(
    const uint16_t *source,
    unsigned int width,
    unsigned int height)
{
    unsigned int mb_columns = width >> 4;
    unsigned int mb_rows = height >> 4;
    unsigned int mb_y;
    unsigned int mb_x;

    for (mb_y = 0u; mb_y < mb_rows; ++mb_y) {
        for (mb_x = 0u; mb_x < mb_columns; ++mb_x) {
            const uint16_t *block =
                source + ((mb_y * mb_columns + mb_x) * 256u);
            unsigned int row;

            for (row = 0u; row < 16u; ++row) {
                uint16_t *destination = h1_video_linear +
                    ((mb_y * 16u + row) * width) + (mb_x * 16u);

                memcpy(
                    destination,
                    block + row * 16u,
                    16u * sizeof(uint16_t));
            }
        }
    }
}

static void h1_draw_black_rect(
    int x,
    int y,
    int width,
    int height,
    int z,
    u64 black)
{
    if (width <= 0 || height <= 0)
        return;

    gsKit_prim_sprite(
        display,
        (float)x,
        (float)y,
        (float)(x + width),
        (float)(y + height),
        z,
        black);
}

static void h1_draw_inner_matte(u64 black)
{
    int x = h1_video_contract.draw_x;
    int y = h1_video_contract.draw_y;
    int width = h1_video_contract.draw_width;
    int height = h1_video_contract.draw_height;
    int matte_x = h1_video_contract.inner_matte_x;
    int matte_y = h1_video_contract.inner_matte_y;

    /* Draw the symmetric crop/matte over MPEG, never by changing capture. */
    h1_draw_black_rect(x, y, width, matte_y, 4, black);
    h1_draw_black_rect(
        x,
        y + height - matte_y,
        width,
        matte_y,
        4,
        black);
    h1_draw_black_rect(x, y, matte_x, height, 4, black);
    h1_draw_black_rect(
        x + width - matte_x,
        y,
        matte_x,
        height,
        4,
        black);
}

static int h1_render_locked(
    int upload_desktop,
    int upload_video,
    int upload_overlay)
{
    const u64 clear_color =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);
    const u64 texture_color =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);
    const u64 matte_color =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);

    if (display == NULL || h1_latest_desktop == NULL)
        return -1;

    if (!desktop_texture_configured)
        configure_desktop_texture(h1_latest_desktop);
    else
        desktop_texture.Mem = (u32 *)h1_latest_desktop;

    if (upload_desktop)
        gsKit_texture_upload(display, &desktop_texture);

    if (h1_video_visible) {
        h1_video_texture.Mem = (u32 *)h1_video_linear;
        if (upload_video)
            gsKit_texture_upload(display, &h1_video_texture);
    }

    if (h1_latest_overlay_visible) {
        if (configure_local_overlay_texture(&h1_latest_overlay) < 0)
            return -1;

        local_overlay_texture.Mem = (u32 *)h1_latest_overlay.pixels;
        if (upload_overlay)
            gsKit_texture_upload(display, &local_overlay_texture);
    }

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

    if (h1_video_visible) {
        const pstvnc_mpeg_cal_rect_t *suppression =
            &h1_video_contract.suppression_rect;

        /* RFB cannot remain visible anywhere inside this outer footprint. */
        h1_draw_black_rect(
            suppression->x,
            suppression->y,
            suppression->width,
            suppression->height,
            2,
            matte_color);

        gsKit_prim_sprite_texture(
            display,
            &h1_video_texture,
            (float)h1_video_contract.draw_x,
            (float)h1_video_contract.draw_y,
            0.0f,
            0.0f,
            (float)(h1_video_contract.draw_x +
                    h1_video_contract.draw_width),
            (float)(h1_video_contract.draw_y +
                    h1_video_contract.draw_height),
            (float)h1_video_source_width,
            (float)h1_video_source_height,
            3,
            texture_color);

        h1_draw_inner_matte(matte_color);
    }

    if (h1_latest_overlay_visible) {
        gsKit_prim_sprite_texture(
            display,
            &local_overlay_texture,
            (float)h1_latest_overlay.x,
            (float)h1_latest_overlay.y,
            0.0f,
            0.0f,
            (float)(h1_latest_overlay.x + h1_latest_overlay.width),
            (float)(h1_latest_overlay.y + h1_latest_overlay.height),
            (float)h1_latest_overlay.width,
            (float)h1_latest_overlay.height,
            5,
            texture_color);
    }

    gsKit_queue_exec(display);
    gsKit_sync_flip(display);
    return 0;
}

int pstvnc_ps2_graphics_init(void)
{
    if (pstvnc_ps2_graphics_init_base_c39() < 0)
        return -1;

    if (h1_graphics_sema < 0) {
        h1_graphics_sema = h1_create_graphics_mutex();
        if (h1_graphics_sema < 0)
            return -1;
    }

    h1_video_texture_configured = 0;
    h1_video_source_width = 0u;
    h1_video_source_height = 0u;
    h1_video_visible = 0;
    h1_latest_desktop = NULL;
    h1_latest_overlay_visible = 0;
    memset(&h1_video_texture, 0, sizeof(h1_video_texture));
    memset(&h1_video_contract, 0, sizeof(h1_video_contract));
    memset(&h1_latest_overlay, 0, sizeof(h1_latest_overlay));
    return 0;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay)
{
    int result;

    if (desktop_pixels == NULL ||
        desktop_pixel_count != PSTVNC_DISPLAY_PIXEL_COUNT ||
        !local_overlay_is_valid(local_overlay))
        return -1;

    if (!h1_lock())
        return -1;

    h1_latest_desktop = desktop_pixels;

    if (local_overlay != NULL) {
        h1_latest_overlay = *local_overlay;
        h1_latest_overlay_visible = 1;
    } else {
        memset(&h1_latest_overlay, 0, sizeof(h1_latest_overlay));
        h1_latest_overlay_visible = 0;
    }

    result = h1_render_locked(
        1,
        0,
        h1_latest_overlay_visible);

    if (!h1_unlock())
        return -1;

    return result;
}

int pstvnc_h1_graphics_present_video_macroblocks(
    const uint16_t *macroblock_pixels,
    unsigned int source_width,
    unsigned int source_height,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    int result;

    if (macroblock_pixels == NULL ||
        source_width == 0u || source_height == 0u ||
        source_width > H1_VIDEO_MAX_WIDTH ||
        source_height > H1_VIDEO_MAX_HEIGHT ||
        (source_width & 15u) != 0u ||
        (source_height & 15u) != 0u ||
        !h1_contract_valid(contract))
        return -1;

    if (!h1_lock())
        return -1;

    if (h1_latest_desktop == NULL ||
        h1_configure_video_texture(source_width, source_height) < 0) {
        (void)h1_unlock();
        return -1;
    }

    h1_detile_macroblocks(
        macroblock_pixels,
        source_width,
        source_height);
    h1_video_contract = *contract;
    h1_video_visible = 1;

    /* Return success only after the synchronized physical presentation. */
    result = h1_render_locked(0, 1, 0);

    if (!h1_unlock())
        return -1;

    return result;
}

int pstvnc_h1_graphics_clear_video(void)
{
    int result = 0;

    if (!h1_lock())
        return -1;

    h1_video_visible = 0;
    memset(&h1_video_contract, 0, sizeof(h1_video_contract));

    if (h1_latest_desktop != NULL)
        result = h1_render_locked(0, 0, 0);

    if (!h1_unlock())
        return -1;

    return result;
}

void pstvnc_ps2_graphics_shutdown(void)
{
    if (h1_graphics_sema >= 0) {
        (void)DeleteSema(h1_graphics_sema);
        h1_graphics_sema = -1;
    }

    h1_video_texture_configured = 0;
    h1_video_source_width = 0u;
    h1_video_source_height = 0u;
    h1_video_visible = 0;
    h1_latest_desktop = NULL;
    h1_latest_overlay_visible = 0;
    memset(&h1_video_texture, 0, sizeof(h1_video_texture));
    memset(&h1_video_contract, 0, sizeof(h1_video_contract));
    memset(&h1_latest_overlay, 0, sizeof(h1_latest_overlay));

    pstvnc_ps2_graphics_shutdown_base_c39();
}
