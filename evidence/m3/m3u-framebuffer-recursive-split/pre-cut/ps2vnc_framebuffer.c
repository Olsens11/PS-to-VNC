/*
 * PS-to-VNC M3K recursive migration translation unit.
 *
 * This TU gives the existing framebuffer-engine semantic leaf a real
 * compiler/linker boundary.  The leaf body remains the implementation
 * authority; this file is migration scaffolding only.
 */

#include "ps2vnc_runtime_compat.h"
#include "ps2vnc_cross_types.h"

#include "ps2vnc_services_imports.h"
#include "ps2vnc_services_exports.h"

#include "ps2vnc_runtime_to_ui_display.h"
#include "ps2vnc_ui_display_to_runtime.h"
#include "ps2vnc_runtime_to_framebuffer.h"

#include "ps2vnc_framebuffer_engine.inc"
