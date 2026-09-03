/*
 * File synopsis:
 * Declares the PS2 system-lifecycle seam and hides IOP and OSDSYS mechanisms.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "PS2 system and
 * private-Ethernet platform seam"; docs/CLEAN_ARCHITECTURE.md, "PS2 platform
 * mechanisms".
 */

#ifndef PSTVNC_PS2_SYSTEM_H
#define PSTVNC_PS2_SYSTEM_H

/*
 * Establish the ordinary PS2SDK IOP state required before product modules are
 * loaded. Returns 0 on success and -1 on failure.
 */
int pstvnc_ps2_system_prepare_iop(void);

/*
 * Sleep the current EE thread for a coordinator-selected delay. Timing policy
 * belongs to the caller; this seam only hides the PS2SDK mechanism.
 */
void pstvnc_ps2_system_delay_ms(unsigned int milliseconds);

/*
 * Converge to the ordinary PS2 system menu. If LoadExecPS2 unexpectedly
 * returns, park the current thread rather than continuing in unknown state.
 */
void pstvnc_ps2_system_exit_to_menu(void);

#endif
