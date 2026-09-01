#ifndef PSTVNC_PS2_SYSTEM_H
#define PSTVNC_PS2_SYSTEM_H

int pstvnc_ps2_system_bootstrap(void);

void pstvnc_ps2_console_init(void);
void pstvnc_ps2_console_printf(const char *format, ...);

void pstvnc_ps2_delay_ms(unsigned int milliseconds);

/*
 * Converge the application to the ordinary PS2 system menu. If LoadExecPS2()
 * unexpectedly returns, this function leaves a visible failure marker and
 * parks the thread instead of continuing in an unknown lifecycle state.
 */
void pstvnc_ps2_exit_to_system_menu(void);

#endif
