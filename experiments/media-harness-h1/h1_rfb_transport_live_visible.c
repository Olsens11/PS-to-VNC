/*
 * File synopsis:
 * CP2K-only mechanical adapter for the already-qualified CP2J logical RFB
 * transport implementation.
 *
 * CONFIG mode 2 means "the same RFB transport semantics, with visible
 * presentation selected by the specialized coordinator."  Rather than broaden
 * the CP2J translation unit before visible hardware qualification, this wrapper
 * first parses the shared CONFIG vocabulary and then mechanically aliases the
 * CP2J mode-1 token to the reserved mode-2 token while compiling that exact
 * implementation. No queue, credit, mux framing, socket, or quiesce algorithm
 * changes occur here.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_rfb_transport_live.c"
