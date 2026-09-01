/*
 * File synopsis:
 * Declares the top-level coordinator entry point without exposing its private
 * subsystem state.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Application coordinator".
 */

#ifndef PSTVNC_APP_H
#define PSTVNC_APP_H

/*
 * Run the first clean fixed-480p product loop.
 *
 * Returns only after a fatal startup/session/presentation failure. The process
 * entry point then converges to the ordinary PS2 system menu.
 */
int pstvnc_app_run(void);

#endif
