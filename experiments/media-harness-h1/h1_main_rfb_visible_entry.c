/*
 * File synopsis:
 * CP2K build entry that binds the specialized visible RFB-only coordinator to
 * CONFIG mode 2 without changing its otherwise mode-1-qualified control flow.
 *
 * Parse the common CONFIG enum first, then mechanically alias the coordinator's
 * RFB-ON token while compiling it. This keeps CP2J mode 1 headless and CP2K mode
 * 2 visible as separate hardware authorities.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_main_rfb_visible.c"
