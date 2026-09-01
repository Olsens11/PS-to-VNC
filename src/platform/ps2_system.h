/*
 * File synopsis:
 * Declares the PS2 system-lifecycle seam and hides IOP and OSDSYS mechanisms.
 */

#ifndef PSTVNC_PS2_SYSTEM_H
#define PSTVNC_PS2_SYSTEM_H

/*
 * Establish the ordinary PS2SDK IOP state required before product modules are
 * loaded. Returns 0 on success and -1 on failure.
 */
int pstvnc_ps2_system_prepare_iop(void);

/*
 * Converge to the ordinary PS2 system menu. If LoadExecPS2 unexpectedly
 * returns, park the current thread rather than continuing in unknown state.
 */
void pstvnc_ps2_system_exit_to_menu(void);

#endif
