#ifndef PSTVNC_VIDEO_MODE_H
#define PSTVNC_VIDEO_MODE_H

/*
 * Immutable description of a PS-to-VNC video mode.
 *
 * This header deliberately has no PS2SDK dependency. Platform-specific
 * numeric mode/interlace/field identifiers are stored as ordinary ints;
 * the PS2 implementation supplies those values when defining the catalog.
 */

typedef enum {
    PSTVNC_VIDEO_BACKEND_NONE = 0,
    PSTVNC_VIDEO_BACKEND_STANDARD = 1u << 0,
    PSTVNC_VIDEO_BACKEND_HIRES = 1u << 1
} pstvnc_video_backend_t;

typedef enum {
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV = 0,
    PSTVNC_VIDEO_MODE_GROUP_VGA_60,
    PSTVNC_VIDEO_MODE_GROUP_VGA_OTHER,
    PSTVNC_VIDEO_MODE_GROUP_COUNT
} pstvnc_video_mode_group_t;

typedef struct {
    const char *name;

    int gs_mode;
    int interlace;
    int field;

    /* Electrical / physical raster. */
    unsigned int raster_width;
    unsigned int raster_height;

    /*
     * Ordinary gsKit drawing/framebuffer geometry.
     *
     * A zero Standard geometry means that backend is unavailable.
     */
    unsigned int standard_fb_width;
    unsigned int standard_fb_height;
    int standard_offset_x;
    int standard_offset_y;

    /* Fixed RFB/logical desktop for the Standard backend. */
    unsigned int standard_logical_width;
    unsigned int standard_logical_height;

    /* HIRES drawing geometry and pass count. */
    unsigned int hires_width;
    unsigned int hires_height;
    unsigned int hires_passes;

    unsigned int allowed_backends;
    pstvnc_video_backend_t recommended_backend;

    /* Stable presentation grouping metadata for mode selection UIs. */
    pstvnc_video_mode_group_t menu_group;
} pstvnc_video_mode_t;

#define PSTVNC_VIDEO_MODE_COUNT 22u

/*
 * Read-only catalog.
 *
 * Direct catalog exposure is retained while migration-era consumers are
 * normalized. The catalog and every descriptor are immutable.
 */
extern const pstvnc_video_mode_t *const
    pstvnc_video_modes[PSTVNC_VIDEO_MODE_COUNT];

int pstvnc_video_mode_supports_backend(
    const pstvnc_video_mode_t *mode,
    pstvnc_video_backend_t backend);

pstvnc_video_backend_t pstvnc_video_mode_default_backend(
    const pstvnc_video_mode_t *mode);

const pstvnc_video_mode_t *pstvnc_video_mode_by_name(
    const char *name);

#endif /* PSTVNC_VIDEO_MODE_H */
