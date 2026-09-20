/*
 * File synopsis:
 * Owns the single GS/dmaKit presentation mechanism for the fixed Standard 480p
 * desktop, optional MPEG suppression/video/inner-matte layers, local UI overlay,
 * synchronized flips, post-sync timer observation, and shutdown.
 *
 * Desktop and overlay textures are refreshed only when their owners submit new
 * surfaces. MPEG uses one maximum-size reusable VRAM allocation plus one bounded
 * EE-side detile buffer. No semaphore or second presenter exists because GS
 * ownership remains singular at this platform seam.
 *
 * Context:
 *   docs/reconstruction/ISSUE7_MINIMAL_CORE.md;
 *   docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms";
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4.
 */

#include <dmaKit.h>
#include <gsKit.h>
#include <timer.h>

#include <stddef.h>
#include <string.h>

#include "display.h"
#include "ps2_graphics.h"

static GSGLOBAL *display;

static GSTEXTURE desktop_texture;
static int desktop_texture_configured;

static GSTEXTURE local_overlay_texture;
static int local_overlay_texture_configured;
static unsigned int local_overlay_width;
static unsigned int local_overlay_height;
static unsigned int local_overlay_x;
static unsigned int local_overlay_y;
static int local_overlay_visible;

static GSTEXTURE video_texture;
static int video_texture_configured;
static int video_visible;
static pstvnc_ps2_graphics_rect_t video_base;
static pstvnc_ps2_graphics_rect_t video_inner_content;
static pstvnc_ps2_graphics_rect_t video_suppression;

static uint16_t video_linear[PSTVNC_MPEG_RGB16_MAX_PIXELS]
    __attribute__((aligned(128)));

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
        overlay->width > PSTVNC_DISPLAY_WIDTH ||
        overlay->height > PSTVNC_DISPLAY_HEIGHT)
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

    return overlay->pixel_count == expected_pixels;
}

static int configure_local_overlay_texture(
    const pstvnc_ps2_graphics_overlay_t *overlay)
{
    if (overlay == NULL)
        return 0;

    if (local_overlay_texture_configured) {
        if (overlay->width != local_overlay_width ||
            overlay->height != local_overlay_height)
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

    local_overlay_width = overlay->width;
    local_overlay_height = overlay->height;
    local_overlay_texture_configured = 1;
    return 0;
}

static int rect_is_valid(
    const pstvnc_ps2_graphics_rect_t *rect)
{
    if (rect == NULL ||
        rect->x < 0 ||
        rect->y < 0 ||
        rect->width <= 0 ||
        rect->height <= 0 ||
        rect->x >
            (int32_t)PSTVNC_DISPLAY_WIDTH -
            rect->width ||
        rect->y >
            (int32_t)PSTVNC_DISPLAY_HEIGHT -
            rect->height)
        return 0;

    return 1;
}

static int video_is_valid(
    const pstvnc_ps2_graphics_video_t *video)
{
    int64_t base_right;
    int64_t base_bottom;
    int64_t inner_right;
    int64_t inner_bottom;
    int64_t suppression_right;
    int64_t suppression_bottom;

    if (video == NULL ||
        !pstvnc_mpeg_rgb16_macroblock_surface_valid(
            &video->surface) ||
        !rect_is_valid(&video->base) ||
        !rect_is_valid(&video->inner_content) ||
        !rect_is_valid(&video->suppression) ||
        video->surface.width !=
            (unsigned int)video->base.width ||
        video->surface.height !=
            (unsigned int)video->base.height)
        return 0;

    base_right =
        (int64_t)video->base.x +
        video->base.width;

    base_bottom =
        (int64_t)video->base.y +
        video->base.height;

    inner_right =
        (int64_t)video->inner_content.x +
        video->inner_content.width;

    inner_bottom =
        (int64_t)video->inner_content.y +
        video->inner_content.height;

    suppression_right =
        (int64_t)video->suppression.x +
        video->suppression.width;

    suppression_bottom =
        (int64_t)video->suppression.y +
        video->suppression.height;

    if (video->inner_content.x < video->base.x ||
        video->inner_content.y < video->base.y ||
        inner_right > base_right ||
        inner_bottom > base_bottom ||
        video->suppression.x > video->base.x ||
        video->suppression.y > video->base.y ||
        suppression_right < base_right ||
        suppression_bottom < base_bottom)
        return 0;

    return 1;
}

static int configure_video_texture(
    unsigned int width,
    unsigned int height)
{
    if (!video_texture_configured) {
        memset(
            &video_texture,
            0,
            sizeof(video_texture));

        video_texture.PSM =
            GS_PSM_CT16;

        video_texture.Filter =
            GS_FILTER_NEAREST;

        video_texture.VramClut = 0;

        video_texture.Vram =
            gsKit_vram_alloc(
                display,
                gsKit_texture_size(
                    PSTVNC_MPEG_RGB16_MAX_WIDTH,
                    PSTVNC_MPEG_RGB16_MAX_HEIGHT,
                    GS_PSM_CT16),
                GSKIT_ALLOC_USERBUFFER);

        video_texture_configured = 1;
    }

    video_texture.Width = width;
    video_texture.Height = height;
    video_texture.Mem = (u32 *)video_linear;
    return 0;
}

static void draw_black_rect(
    const pstvnc_ps2_graphics_rect_t *rect,
    int z,
    u64 black)
{
    if (rect == NULL ||
        rect->width <= 0 ||
        rect->height <= 0)
        return;

    gsKit_prim_sprite(
        display,
        (float)rect->x,
        (float)rect->y,
        (float)(rect->x + rect->width),
        (float)(rect->y + rect->height),
        z,
        black);
}

static void draw_inner_matte(u64 black)
{
    pstvnc_ps2_graphics_rect_t matte;
    int base_right;
    int base_bottom;
    int inner_right;
    int inner_bottom;

    base_right =
        video_base.x +
        video_base.width;

    base_bottom =
        video_base.y +
        video_base.height;

    inner_right =
        video_inner_content.x +
        video_inner_content.width;

    inner_bottom =
        video_inner_content.y +
        video_inner_content.height;

    matte.x = video_base.x;
    matte.y = video_base.y;
    matte.width = video_base.width;
    matte.height =
        video_inner_content.y -
        video_base.y;
    draw_black_rect(&matte, 4, black);

    matte.x = video_base.x;
    matte.y = inner_bottom;
    matte.width = video_base.width;
    matte.height =
        base_bottom -
        inner_bottom;
    draw_black_rect(&matte, 4, black);

    matte.x = video_base.x;
    matte.y = video_base.y;
    matte.width =
        video_inner_content.x -
        video_base.x;
    matte.height = video_base.height;
    draw_black_rect(&matte, 4, black);

    matte.x = inner_right;
    matte.y = video_base.y;
    matte.width =
        base_right -
        inner_right;
    matte.height = video_base.height;
    draw_black_rect(&matte, 4, black);
}

static int render_frame(
    int upload_desktop,
    int upload_video,
    int upload_overlay,
    pstvnc_ps2_graphics_sync_result_t *sync_result)
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

    const u64 matte_color =
        GS_SETREG_RGBAQ(
            0x00,
            0x00,
            0x00,
            0x80,
            0x00);

    if (sync_result != NULL)
        memset(sync_result, 0, sizeof(*sync_result));

    if (display == NULL ||
        !desktop_texture_configured)
        return -1;

    if (upload_desktop)
        gsKit_texture_upload(
            display,
            &desktop_texture);

    if (video_visible && upload_video)
        gsKit_texture_upload(
            display,
            &video_texture);

    if (local_overlay_visible && upload_overlay)
        gsKit_texture_upload(
            display,
            &local_overlay_texture);

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

    if (video_visible) {
        draw_black_rect(
            &video_suppression,
            2,
            matte_color);

        gsKit_prim_sprite_texture(
            display,
            &video_texture,
            (float)video_base.x,
            (float)video_base.y,
            0.0f,
            0.0f,
            (float)(video_base.x +
                    video_base.width),
            (float)(video_base.y +
                    video_base.height),
            (float)video_texture.Width,
            (float)video_texture.Height,
            3,
            texture_color);

        draw_inner_matte(matte_color);
    }

    if (local_overlay_visible) {
        gsKit_prim_sprite_texture(
            display,
            &local_overlay_texture,
            (float)local_overlay_x,
            (float)local_overlay_y,
            0.0f,
            0.0f,
            (float)(local_overlay_x +
                    local_overlay_width),
            (float)(local_overlay_y +
                    local_overlay_height),
            (float)local_overlay_width,
            (float)local_overlay_height,
            5,
            texture_color);
    }

    gsKit_queue_exec(display);
    gsKit_sync_flip(display);

    if (sync_result != NULL) {
        sync_result->observed_sync_tick =
            (uint64_t)GetTimerSystemTime();

        sync_result->ticks_per_second =
            (uint32_t)kBUSCLK;

        sync_result->synchronized = 1;
    }

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
    local_overlay_x = 0u;
    local_overlay_y = 0u;
    local_overlay_visible = 0;
    memset(
        &local_overlay_texture,
        0,
        sizeof(local_overlay_texture));

    video_texture_configured = 0;
    video_visible = 0;
    memset(
        &video_texture,
        0,
        sizeof(video_texture));
    memset(
        &video_base,
        0,
        sizeof(video_base));
    memset(
        &video_inner_content,
        0,
        sizeof(video_inner_content));
    memset(
        &video_suppression,
        0,
        sizeof(video_suppression));

    return 0;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay)
{
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

    if (local_overlay != NULL) {
        local_overlay_x = local_overlay->x;
        local_overlay_y = local_overlay->y;
        local_overlay_visible = 1;
    } else {
        local_overlay_visible = 0;
    }

    return render_frame(
        1,
        0,
        local_overlay_visible,
        NULL);
}

int pstvnc_ps2_graphics_present_video_macroblocks(
    const pstvnc_ps2_graphics_video_t *video,
    pstvnc_ps2_graphics_sync_result_t *sync_result)
{
    if (display == NULL ||
        sync_result == NULL ||
        !desktop_texture_configured ||
        !video_is_valid(video))
        return -1;

    if (!pstvnc_mpeg_rgb16_detile(
            &video->surface,
            video_linear,
            PSTVNC_MPEG_RGB16_MAX_PIXELS))
        return -1;

    if (configure_video_texture(
            video->surface.width,
            video->surface.height) < 0)
        return -1;

    video_base = video->base;
    video_inner_content =
        video->inner_content;
    video_suppression =
        video->suppression;
    video_visible = 1;

    return render_frame(
        0,
        1,
        0,
        sync_result);
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
    local_overlay_x = 0u;
    local_overlay_y = 0u;
    local_overlay_visible = 0;
    memset(
        &local_overlay_texture,
        0,
        sizeof(local_overlay_texture));

    video_texture_configured = 0;
    video_visible = 0;
    memset(
        &video_texture,
        0,
        sizeof(video_texture));
    memset(
        &video_base,
        0,
        sizeof(video_base));
    memset(
        &video_inner_content,
        0,
        sizeof(video_inner_content));
    memset(
        &video_suppression,
        0,
        sizeof(video_suppression));
}
