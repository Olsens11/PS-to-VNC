/*
 * File synopsis:
 * Defines the dormant H1-owned resource bundle for logical RFB channel 1.
 *
 * The bundle owns only the RFB queue storage, its mutex semaphore, and the
 * already host-tested logical-channel state. It deliberately owns no socket,
 * receiver thread, CONFIG policy, RFB parser, or presentation state. The H1
 * transport runtime will embed/use this bundle only after the activation gate
 * is relaxed by a later checkpoint.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2; RFB_MUX_PREP_CHECKPOINTS.md.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_RUNTIME_RESOURCES_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_RUNTIME_RESOURCES_H

#include "h1_rfb_channel.h"

#include <stdint.h>

typedef struct pstvnc_h1_rfb_runtime_resources {
    int queue_sema_id;
    uint8_t *queue_storage;
    pstvnc_h1_rfb_channel_t channel;
    int active;
} pstvnc_h1_rfb_runtime_resources_t;

void pstvnc_h1_rfb_runtime_resources_init(
    pstvnc_h1_rfb_runtime_resources_t *resources);

/*
 * Allocate the fixed evidence-based 32768-byte queue and one mutex semaphore
 * only when enabled is nonzero. Passing enabled==0 is a successful no-op and
 * leaves the bundle completely inactive/unallocated.
 */
int pstvnc_h1_rfb_runtime_resources_activate(
    pstvnc_h1_rfb_runtime_resources_t *resources,
    int enabled);

/* Release only resources owned by this bundle; safe for an inactive bundle. */
int pstvnc_h1_rfb_runtime_resources_release(
    pstvnc_h1_rfb_runtime_resources_t *resources);

#endif
