#include "ps2_system.h"

#include <stdarg.h>
#include <stdio.h>

#include <debug.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <kernel.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <sifrpc.h>

static int pstvnc_console_row = 0;

int pstvnc_ps2_system_bootstrap(void)
{
    sceSifInitRpc(0);

    while (!SifIopReset("", 0)) {
    }

    while (!SifIopSync()) {
    }

    sceSifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();
    sbv_patch_enable_lmb();

    return 0;
}

void pstvnc_ps2_console_init(void)
{
    init_scr();
    scr_setCursor(0);
    pstvnc_console_row = 0;
}

void pstvnc_ps2_console_printf(const char *format, ...)
{
    char buffer[79];
    va_list arguments;

    if (format == NULL)
        return;

    va_start(arguments, format);
    vsnprintf(buffer, sizeof(buffer), format, arguments);
    va_end(arguments);

    buffer[sizeof(buffer) - 1] = '\0';

    if (pstvnc_console_row >= 27)
        pstvnc_console_row = 0;

    scr_setXY(0, pstvnc_console_row++);
    scr_printf("%s", buffer);
}

void pstvnc_ps2_delay_ms(unsigned int milliseconds)
{
    DelayThread(milliseconds * 1000u);
}

void pstvnc_ps2_exit_to_system_menu(void)
{
    LoadExecPS2("rom0:OSDSYS", 0, NULL);

    pstvnc_ps2_console_printf(
        "ERROR: OSDSYS return failed");

    SleepThread();
}
