/*
 * File synopsis:
 * CP2K-only mechanical adapter for the CP2J RFB session coordinator plus its
 * optional complete-frame presentation callback seam.
 *
 * Shared CONFIG vocabulary is parsed before the token alias so the mode enum is
 * unchanged. The included coordinator then treats mode 2 exactly as its already
 * qualified mode-1 RFB transport/parser state machine; only the caller-supplied
 * presenter distinguishes CP2K from CP2J.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_rfb_session_runtime.c"
