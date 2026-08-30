/*
 * PS-to-VNC M3I recursive migration translation unit.
 *
 * This is a coarse migration boundary, not the final API shape.
 *
 * Ownership:
 *   - ui_controller
 *   - display_orchestration
 *
 * The existing ps2vnc_runtime.c retains:
 *   - runtime_support
 *   - framebuffer_engine
 *
 * Cross-TU interface cleanup is deliberately deferred until the compiler
 * exposes the exact closure required by this real boundary.
 */

#include "ps2vnc_runtime_compat.h"
#include "ps2vnc_cross_types.h"
#include "ps2vnc_services_imports.h"
#include "ps2vnc_services_exports.h"
#include "ps2vnc_runtime_to_ui_display.h"

#include "ps2vnc_ui_controller.inc"
#include "ps2vnc_display_orchestration.inc"
