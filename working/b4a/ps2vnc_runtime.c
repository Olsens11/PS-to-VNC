/*
 * PS-to-VNC M3G coarse runtime translation unit.
 *
 * Owns the complete remaining implementation envelope:
 * runtime_support + ui_controller + framebuffer_engine +
 * display_orchestration.
 */

#include "ps2vnc_runtime_compat.h"
#include "ps2vnc_cross_types.h"
#include "ps2vnc_services_imports.h"
#include "ps2vnc_services_exports.h"
#include "ps2vnc_ui_display_to_runtime.h"

/* M4G2: immutable video-mode model lives in src/video/mode.c. */

/* Recursive M3 subdivision scaffold remains intact. */
/*
 * M3I recursive coarse split:
 * this TU retains runtime_support + framebuffer_engine.
 */
#include "ps2vnc_runtime_tail_to_core.h"

#include "ps2vnc_runtime_core.inc"
/* M3K: framebuffer engine compiled in ps2vnc_framebuffer.c */
