/*
 * File synopsis:
 * CP2M build entry that binds visible RFB plus mouse/keyboard input to CONFIG
 * mode 2 while retaining CP2K/CP2L/CP2J transport-mode scoping.
 *
 * The shared CONFIG enum is parsed before the token alias. CP2J mode 1 remains
 * headless; CP2K and CP2L remain separately preserved historical hardware
 * authorities. CP2M changes PT_LOAD and therefore requires its own hardware run.
 */

#include "h1_config.h"

#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE
#include "h1_main_rfb_visible_keyboard_input.c"
