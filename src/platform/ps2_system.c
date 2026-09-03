/*
 * File synopsis:
 * Owns deterministic IOP bootstrap and the final OSDSYS system-menu exit path.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "PS2 system and
 * private-Ethernet platform seam"; docs/CLEAN_ARCHITECTURE.md, "PS2 platform
 * mechanisms".
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

static void system_delay_alarm(
    s32 alarm_id,
    u16 time,
    void *common)
{
    (void)alarm_id;
    (void)time;

    iWakeupThread(*(int *)common);
}

void pstvnc_ps2_system_delay_ms(unsigned int milliseconds)
{
    int thread_id = GetThreadId();

    /*
     * Use the same alarm-driven EE sleep mechanism as the qualified Ethernet
     * link wait. The PS2 alarm clock uses 16 ticks per millisecond here, so the
     * reconnect backoff sleeps without busy-spinning the EE.
     */
    if (SetAlarm(
            milliseconds * 16u,
            &system_delay_alarm,
            &thread_id) < 0)
        return;

    SleepThread();
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
