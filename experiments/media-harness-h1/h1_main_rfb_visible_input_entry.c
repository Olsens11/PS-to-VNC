/*
 * File synopsis:
 * CP2L build entry that binds the visible-RFB plus mouse-input coordinator to
 * CONFIG mode 2 while retaining the CP2K/CP2J transport-mode scoping.
 *
 * The shared CONFIG enum is parsed before the token alias. CP2J mode 1 remains
 * headless; CP2K's exact tested mode-2 ELF remains historical hardware authority;
 * this CP2L descendant requires its own changed-PT_LOAD hardware qualification.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_main_rfb_visible_input.c"
