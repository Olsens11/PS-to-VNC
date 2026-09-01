/*
 * File synopsis:
 * Implements the immutable video-mode catalog and lookup helpers without
 * activating any mode.
 */

#include <string.h>

#include <gsKit.h>

#include "video/mode.h"

/*
 * D17AL large display-mode matrix.
 *
 * Raster geometry, renderer drawing geometry, and RFB logical geometry are
 * deliberately independent concepts.
 *
 * TV/DTV
 * -------
 * 480i Standard reconstructs the hardware-proven Test13C contract:
 *     physical timing : NTSC 480i
 *     GS drawing      : 704x232 FRAME
 *     RFB desktop     : 704x464
 *
 * 576i begins as HIRES so its full-frame logical geometry can be exercised
 * without inventing an unproven Standard PAL safe-area contract.
 *
 * VGA
 * ---
 * The initial VGA matrix is HIRES-only. It intentionally covers both widths
 * divisible by the GS 64-pixel FBW unit and widths that are not, giving the
 * mode survey useful presentation-path controls.
 */

static const pstvnc_video_mode_t pstvnc_video_mode_480i = {
    "480i", GS_MODE_NTSC, GS_INTERLACED, GS_FRAME,
    720, 480,
    704, 232, -32, 8,
    704, 464,
    0, 0, 0,
    PSTVNC_VIDEO_BACKEND_STANDARD,
    PSTVNC_VIDEO_BACKEND_STANDARD,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

static const pstvnc_video_mode_t pstvnc_video_mode_480p = {
    "480p", GS_MODE_DTV_480P, GS_NONINTERLACED, GS_FRAME,
    720, 480,
    704, 462, -4, 3,
    704, 462,
    0, 0, 0,
    PSTVNC_VIDEO_BACKEND_STANDARD,
    PSTVNC_VIDEO_BACKEND_STANDARD,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

static const pstvnc_video_mode_t pstvnc_video_mode_480p_hires = {
    "480p-hires", GS_MODE_DTV_480P, GS_NONINTERLACED, GS_FRAME,
    720, 480,
    0, 0, 0, 0,
    0, 0,

    /*
     * D17AL-F7A:
     *
     * Keep the DTV 480p physical timing at 720x480, but give gsHires
     * the same native 704x462 drawing geometry already proven by the
     * ordinary-gsKit 480p backend.
     *
     * F7A is a local-primitive geometry probe only. RFB/background
     * publication is deliberately disabled for this one exact mode below.
     */
    704, 462, 3,

    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

static const pstvnc_video_mode_t pstvnc_video_mode_576i = {
    "576i", GS_MODE_PAL, GS_INTERLACED, GS_FRAME,
    720, 576,
    0, 0, 0, 0,
    0, 0,
    720, 288, 3,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

static const pstvnc_video_mode_t pstvnc_video_mode_576p = {
    "576p", GS_MODE_DTV_576P, GS_NONINTERLACED, GS_FRAME,
    720, 576,
    0, 0, 0, 0,
    0, 0,
    720, 576, 3,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

static const pstvnc_video_mode_t pstvnc_video_mode_720p = {
    "720p", GS_MODE_DTV_720P, GS_NONINTERLACED, GS_FRAME,
    1280, 720,
    0, 0, 0, 0,
    0, 0,
    1280, 720, 3,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};

const pstvnc_video_mode_t pstvnc_video_mode_1080i = {
    "1080i", GS_MODE_DTV_1080I, GS_INTERLACED, GS_FRAME,
    1920, 1080,
    0, 0, 0, 0,
    0, 0,
    1920, 540, 3,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV
};


/* VGA 60 Hz family. */

static const pstvnc_video_mode_t pstvnc_video_mode_vga_640_60 = {
    "640x480-60", GS_MODE_VGA_640_60, GS_NONINTERLACED, GS_FRAME,
    640, 480,
    0, 0, 0, 0,
    0, 0,
    640, 480, 3,
    PSTVNC_VIDEO_BACKEND_HIRES, PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_VGA_60
};

static const pstvnc_video_mode_t pstvnc_video_mode_vga_800_60 = {
    "800x600-60", GS_MODE_VGA_800_60, GS_NONINTERLACED, GS_FRAME,
    800, 600,
    0, 0, 0, 0,
    0, 0,
    800, 600, 3,
    PSTVNC_VIDEO_BACKEND_HIRES, PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_VGA_60
};

static const pstvnc_video_mode_t pstvnc_video_mode_vga_1024_60 = {
    "1024x768-60", GS_MODE_VGA_1024_60, GS_NONINTERLACED, GS_FRAME,
    1024, 768,
    0, 0, 0, 0,
    0, 0,
    1024, 768, 3,
    PSTVNC_VIDEO_BACKEND_HIRES, PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_VGA_60
};

static const pstvnc_video_mode_t pstvnc_video_mode_vga_1280_60 = {
    "1280x1024-60", GS_MODE_VGA_1280_60, GS_NONINTERLACED, GS_FRAME,
    1280, 1024,
    0, 0, 0, 0,
    0, 0,
    1280, 1024, 3,
    PSTVNC_VIDEO_BACKEND_HIRES, PSTVNC_VIDEO_BACKEND_HIRES,
    PSTVNC_VIDEO_MODE_GROUP_VGA_60
};


/* VGA alternate-refresh family. */

#define PSTVNC_DEFINE_VGA_OTHER_MODE(var, label, gsconst, w, h) \
static const pstvnc_video_mode_t var = { \
    label, gsconst, GS_NONINTERLACED, GS_FRAME, \
    w, h, \
    0, 0, 0, 0, \
    0, 0, \
    w, h, 3, \
    PSTVNC_VIDEO_BACKEND_HIRES, PSTVNC_VIDEO_BACKEND_HIRES, \
    PSTVNC_VIDEO_MODE_GROUP_VGA_OTHER \
}

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_640_72,
    "640x480-72", GS_MODE_VGA_640_72, 640, 480);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_640_75,
    "640x480-75", GS_MODE_VGA_640_75, 640, 480);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_640_85,
    "640x480-85", GS_MODE_VGA_640_85, 640, 480);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_800_56,
    "800x600-56", GS_MODE_VGA_800_56, 800, 600);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_800_72,
    "800x600-72", GS_MODE_VGA_800_72, 800, 600);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_800_75,
    "800x600-75", GS_MODE_VGA_800_75, 800, 600);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_800_85,
    "800x600-85", GS_MODE_VGA_800_85, 800, 600);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_1024_70,
    "1024x768-70", GS_MODE_VGA_1024_70, 1024, 768);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_1024_75,
    "1024x768-75", GS_MODE_VGA_1024_75, 1024, 768);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_1024_85,
    "1024x768-85", GS_MODE_VGA_1024_85, 1024, 768);

PSTVNC_DEFINE_VGA_OTHER_MODE(
    pstvnc_video_mode_vga_1280_75,
    "1280x1024-75", GS_MODE_VGA_1280_75, 1280, 1024);

#undef PSTVNC_DEFINE_VGA_OTHER_MODE


/*
 * Registry order is also the stable transaction/request identity.
 * Keep entries grouped and sorted for human inspection; the UI derives its
 * columns from menu_group rather than assuming a particular row count.
 */
const pstvnc_video_mode_t *const pstvnc_video_modes[PSTVNC_VIDEO_MODE_COUNT] = {
    /* TV / DTV */
    &pstvnc_video_mode_480i,
    &pstvnc_video_mode_480p,
    &pstvnc_video_mode_480p_hires,
    &pstvnc_video_mode_576i,
    &pstvnc_video_mode_576p,
    &pstvnc_video_mode_720p,
    &pstvnc_video_mode_1080i,

    /* VGA 60 Hz */
    &pstvnc_video_mode_vga_640_60,
    &pstvnc_video_mode_vga_800_60,
    &pstvnc_video_mode_vga_1024_60,
    &pstvnc_video_mode_vga_1280_60,

    /* VGA alternate refresh */
    &pstvnc_video_mode_vga_640_72,
    &pstvnc_video_mode_vga_640_75,
    &pstvnc_video_mode_vga_640_85,
    &pstvnc_video_mode_vga_800_56,
    &pstvnc_video_mode_vga_800_72,
    &pstvnc_video_mode_vga_800_75,
    &pstvnc_video_mode_vga_800_85,
    &pstvnc_video_mode_vga_1024_70,
    &pstvnc_video_mode_vga_1024_75,
    &pstvnc_video_mode_vga_1024_85,
    &pstvnc_video_mode_vga_1280_75
};



typedef char pstvnc_video_mode_catalog_count_must_match[
    (sizeof(pstvnc_video_modes) /
         sizeof(pstvnc_video_modes[0]) ==
     PSTVNC_VIDEO_MODE_COUNT)
        ? 1
        : -1
];

int pstvnc_video_mode_supports_backend(
    const pstvnc_video_mode_t *mode,
    pstvnc_video_backend_t backend)
{
    if (mode == NULL || backend == PSTVNC_VIDEO_BACKEND_NONE)
        return 0;

    return (mode->allowed_backends & (unsigned int)backend) != 0;
}

pstvnc_video_backend_t pstvnc_video_mode_default_backend(
    const pstvnc_video_mode_t *mode)
{
    if (mode == NULL)
        return PSTVNC_VIDEO_BACKEND_NONE;

    if (!pstvnc_video_mode_supports_backend(
            mode, mode->recommended_backend))
        return PSTVNC_VIDEO_BACKEND_NONE;

    return mode->recommended_backend;
}

const pstvnc_video_mode_t *pstvnc_video_mode_by_name(
    const char *name)
{
    unsigned int i;

    if (name == NULL || name[0] == '\0')
        return NULL;

    for (i = 0; i < PSTVNC_VIDEO_MODE_COUNT; i++) {
        if (strcmp(name, pstvnc_video_modes[i]->name) == 0)
            return pstvnc_video_modes[i];
    }

    return NULL;
}
