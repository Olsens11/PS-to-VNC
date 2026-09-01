#ifndef PSTVNC_PS2_SYSTEM_H
#define PSTVNC_PS2_SYSTEM_H

/*
 * Establish the ordinary PS2SDK IOP state required before product modules are
 * loaded. Returns 0 on success and -1 on failure.
 */
int pstvnc_ps2_system_prepare_iop(void);

#endif
