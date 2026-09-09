/*
 * File synopsis:
 * Build-local HELLO capability override for the cumulative H1 RFB activation
 * checkpoint.
 *
 * h1_transport_runtime.c already constructs HELLO from the H1 capability macros
 * exposed by h1_config.h. Rewriting that large transport source merely to OR one
 * experiment bit would enlarge the change surface. This preinclude deliberately
 * loads the normal headers once, then widens only the existing final H1
 * capability term to include CAP_RFB for this one cumulative object build.
 *
 * The override is mechanically scoped by the makefile to h1_transport_runtime.o
 * in the RFB-prep target. No ordinary H1 build sees it.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_CAPABILITY_OVERRIDE_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_CAPABILITY_OVERRIDE_H

#include "h1_config.h"
#include "transport_protocol.h"

#undef PSTVNC_H1_CAP_REPEAT_SESSIONS
#define PSTVNC_H1_CAP_REPEAT_SESSIONS \
    ((1u << 7) | PSTVNC_TRANSPORT_CAP_RFB)

#endif
