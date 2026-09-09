/*
 * File synopsis:
 * Provides a semaphore-consistent diagnostic snapshot of H1 logical RFB
 * channel-1 state without exposing mutable queue ownership to the parser or
 * session coordinator.
 *
 * This is observation only: it performs no socket I/O, queue mutation, credit
 * return, allocation, or scheduling.
 */

#include "h1_rfb_transport_live.h"
#include "h1_transport_runtime.h"

#include <kernel.h>

#include <string.h>

int pstvnc_h1_rfb_transport_snapshot(
    pstvnc_h1_transport_runtime_t *runtime,
    pstvnc_h1_rfb_transport_snapshot_t *snapshot)
{
    if (runtime == NULL || snapshot == NULL)
        return 0;

    memset(snapshot, 0, sizeof(*snapshot));

    if (!runtime->rfb_resources.active)
        return runtime->config.rfb_mode == PSTVNC_H1_RFB_OFF;

    if (runtime->rfb_resources.queue_sema_id < 0)
        return 0;

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0)
        return 0;

    snapshot->active = 1u;
    snapshot->queue_current = (uint32_t)
        pstvnc_h1_rfb_channel_queue_size(
            &runtime->rfb_resources.channel);
    snapshot->queue_high_water = (uint32_t)
        pstvnc_h1_rfb_channel_queue_high_water(
            &runtime->rfb_resources.channel);
    snapshot->channel = runtime->rfb_resources.channel.stats;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0)
        return 0;

    return 1;
}
