/*
 * File synopsis:
 * CP2N build entry that binds the real through-Issue-39 interaction composition
 * to H1 visible-RFB CONFIG mode 2 while preserving the CP2J/CP2K/CP2L mux and
 * quiesce implementation.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_main_rfb_visible_interaction.c"
