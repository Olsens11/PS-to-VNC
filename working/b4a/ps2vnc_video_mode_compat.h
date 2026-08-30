#ifndef PS2VNC_VIDEO_MODE_COMPAT_H
#define PS2VNC_VIDEO_MODE_COMPAT_H

/*
 * Transitional M4G compatibility vocabulary.
 *
 * Permanent ownership lives in src/video/mode.h. Existing migration
 * modules may continue using historical type/constant/catalog spellings
 * until their own responsibility tranche is normalized.
 */

#include "video/mode.h"

typedef pstvnc_video_backend_t ps2vnc_backend_t;
typedef pstvnc_video_mode_group_t ps2vnc_display_group_t;
typedef pstvnc_video_mode_t ps2vnc_video_mode_t;

#define PS2VNC_BACKEND_NONE \
    PSTVNC_VIDEO_BACKEND_NONE

#define PS2VNC_BACKEND_STANDARD \
    PSTVNC_VIDEO_BACKEND_STANDARD

#define PS2VNC_BACKEND_HIRES \
    PSTVNC_VIDEO_BACKEND_HIRES

#define PS2VNC_DISPLAY_GROUP_TV_DTV \
    PSTVNC_VIDEO_MODE_GROUP_TV_DTV

#define PS2VNC_DISPLAY_GROUP_VGA_60 \
    PSTVNC_VIDEO_MODE_GROUP_VGA_60

#define PS2VNC_DISPLAY_GROUP_VGA_OTHER \
    PSTVNC_VIDEO_MODE_GROUP_VGA_OTHER

#define PS2VNC_DISPLAY_GROUP_COUNT \
    PSTVNC_VIDEO_MODE_GROUP_COUNT

#ifndef VIDEO_MODE_COUNT
#define VIDEO_MODE_COUNT \
    PSTVNC_VIDEO_MODE_COUNT
#endif

#define video_modes \
    pstvnc_video_modes

/*
 * Transitional static-initializer bridge. active_video_mode remains
 * migration-owned until the later live-state ownership tranche.
 */
extern const pstvnc_video_mode_t
    pstvnc_video_mode_1080i;

#define video_mode_1080i \
    pstvnc_video_mode_1080i

#endif /* PS2VNC_VIDEO_MODE_COMPAT_H */
