/*
 * File synopsis:
 * Owns deterministic IOP bootstrap and the final OSDSYS system-menu exit path.
 */

#include <iopcontrol.h>
#include <iopheap.h>
#include <kernel.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <sifrpc.h>

#include "ps2_system.h"

int pstvnc_ps2_system_prepare_iop(void)
{
    /*
     * Resetting the IOP discards whatever module/RPC state the launching
     * environment left behind. Re-establishing that foundation makes startup
     * deterministic before PS-to-VNC loads its own network modules.
     */
    sceSifInitRpc(0);

    while (!SifIopReset("", 0)) {
        /* The qualified startup waits until the reset request is accepted. */
    }

    while (!SifIopSync()) {
        /* The IOP must be synchronized before loading product modules. */
    }

    sceSifInitRpc(0);

    if (SifLoadFileInit() < 0)
        return -1;

    if (SifInitIopHeap() < 0)
        return -1;

    if (sbv_patch_enable_lmb() < 0)
        return -1;

    return 0;
}

void pstvnc_ps2_system_exit_to_menu(void)
{
    LoadExecPS2("rom0:OSDSYS", 0, NULL);

    /*
     * Returning from OSDSYS launch is not a valid continuation path. Park the
     * thread instead of running on in partially shut-down product state.
     */
    SleepThread();
}
