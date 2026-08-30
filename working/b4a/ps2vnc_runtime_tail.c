/*
 * PS-to-VNC M3G coarse runtime translation unit.
 *
 * Owns the complete remaining implementation envelope:
 * runtime_support + ui_controller + framebuffer_engine +
 * display_orchestration.
 */

#include "ps2vnc_runtime_compat.h"
#include "ps2vnc_cross_types.h"
#include "diagnostics/debug.h"
#include "ps2vnc_services_imports.h"
#include "ps2vnc_services_exports.h"
#include "ps2vnc_ui_display_to_runtime.h"

/* M4G2: immutable video-mode catalog is provided by src/video/mode.c. */

extern int pstvnc_video_mode_supports_backend(
    const ps2vnc_video_mode_t *mode,
    ps2vnc_backend_t backend);

/* Recursive M3 subdivision scaffold remains intact. */
/*
 * M3I recursive coarse split:
 * this TU retains runtime_support + framebuffer_engine.
 */
#include "ps2vnc_runtime_core_to_tail.h"

#include "ps2vnc_runtime_tail.inc"
/* M3K: framebuffer engine compiled in ps2vnc_framebuffer.c */
