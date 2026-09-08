/*
 * File synopsis:
 * Implements H1 resource ownership for logical RFB channel 1.
 *
 * The resources are intentionally separable from CONFIG activation and live
 * transport dispatch. An OFF path allocates nothing. An ON preparation path
 * owns exactly one queue allocation of the CONFIG-selected capacity and one
 * mutex semaphore, then initializes the tested logical RFB channel over that
 * storage. No socket I/O or RFB protocol work occurs here.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2; RFB_CREDIT_POLICY_DECISION.md.
 */

#include "h1_rfb_runtime_resources.h"

#include <kernel.h>

#include <stdlib.h>
#include <string.h>

static int h1_rfb_create_mutex(void)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;
    return CreateSema(&semaphore);
}

void pstvnc_h1_rfb_runtime_resources_init(
    pstvnc_h1_rfb_runtime_resources_t *resources)
{
    if (resources == NULL)
        return;

    memset(resources, 0, sizeof(*resources));
    resources->queue_sema_id = -1;
}

int pstvnc_h1_rfb_runtime_resources_activate(
    pstvnc_h1_rfb_runtime_resources_t *resources,
    int enabled,
    uint32_t queue_capacity)
{
    if (resources == NULL)
        return 0;

    if (!enabled)
        return queue_capacity == 0u &&
            !resources->active &&
            resources->queue_storage == NULL &&
            resources->queue_capacity == 0u &&
            resources->queue_sema_id < 0;

    if (queue_capacity == 0u ||
        resources->active ||
        resources->queue_storage != NULL ||
        resources->queue_capacity != 0u ||
        resources->queue_sema_id >= 0)
        return 0;

    resources->queue_storage = (uint8_t *)malloc((size_t)queue_capacity);
    if (resources->queue_storage == NULL)
        return 0;

    resources->queue_capacity = queue_capacity;
    resources->queue_sema_id = h1_rfb_create_mutex();
    if (resources->queue_sema_id < 0) {
        free(resources->queue_storage);
        resources->queue_storage = NULL;
        resources->queue_capacity = 0u;
        return 0;
    }

    if (!pstvnc_h1_rfb_channel_init(
            &resources->channel,
            resources->queue_storage,
            (size_t)resources->queue_capacity)) {
        (void)DeleteSema(resources->queue_sema_id);
        resources->queue_sema_id = -1;
        free(resources->queue_storage);
        resources->queue_storage = NULL;
        resources->queue_capacity = 0u;
        return 0;
    }

    resources->active = 1;
    return 1;
}

int pstvnc_h1_rfb_runtime_resources_release(
    pstvnc_h1_rfb_runtime_resources_t *resources)
{
    int result = 1;

    if (resources == NULL)
        return 0;

    if (resources->queue_sema_id >= 0 &&
        DeleteSema(resources->queue_sema_id) < 0)
        result = 0;

    free(resources->queue_storage);
    pstvnc_h1_rfb_runtime_resources_init(resources);
    return result;
}
