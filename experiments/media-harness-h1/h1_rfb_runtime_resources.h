/*
 * File synopsis:
 * Defines the H1-owned resource bundle for logical RFB channel 1.
 *
 * The bundle owns only the RFB queue storage, its mutex semaphore, and the
 * already host-tested logical-channel state. It deliberately owns no socket,
 * receiver thread, CONFIG policy, RFB parser, or presentation state. Queue
 * capacity is supplied by the session CONFIG when RFB is eventually enabled.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2; RFB_CREDIT_POLICY_DECISION.md.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_RUNTIME_RESOURCES_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_RUNTIME_RESOURCES_H

#include "h1_rfb_channel.h"

#include <stdint.h>

typedef struct pstvnc_h1_rfb_runtime_resources {
    int queue_sema_id;

    /*
     * Timer-free activity rendezvous for the sole RFB owner.
     *
     * activity_sequence changes whenever any producer publishes work relevant
     * to that owner. wait_thread_id is armed only immediately before sleeping.
     */
    int wait_thread_id;
    uint32_t activity_sequence;
    uint8_t *queue_storage;
    uint32_t queue_capacity;
    pstvnc_h1_rfb_channel_t channel;
    int active;
} pstvnc_h1_rfb_runtime_resources_t;

void pstvnc_h1_rfb_runtime_resources_init(
    pstvnc_h1_rfb_runtime_resources_t *resources);

/*
 * Allocate one caller-sized queue and one mutex semaphore only when enabled is
 * nonzero. Passing enabled==0 is a successful no-op and requires queue_capacity
 * to be zero, preserving the RFB-OFF no-allocation invariant.
 */
int pstvnc_h1_rfb_runtime_resources_activate(
    pstvnc_h1_rfb_runtime_resources_t *resources,
    int enabled,
    uint32_t queue_capacity);

/* Release only resources owned by this bundle; safe for an inactive bundle. */
int pstvnc_h1_rfb_runtime_resources_release(
    pstvnc_h1_rfb_runtime_resources_t *resources);

#endif
