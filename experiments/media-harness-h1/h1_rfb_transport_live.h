/*
 * File synopsis:
 * Defines the cumulative H1 live mechanics for logical RFB channel 1 while the
 * public CONFIG activation gate remains independently controlled.
 *
 * These functions prepare/release the CONFIG-sized RFB queue, accept channel-1
 * DATA from H1's sole physical receiver, return parser-consumed credit, and
 * serialize outbound logical RFB bytes through the existing H1 send path.
 * They own no RFB parsing, framebuffer presentation, input policy, or Pi VNC
 * bridge behavior.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_TRANSPORT_LIVE_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_TRANSPORT_LIVE_H

#include "h1_rfb_channel.h"

#include <stddef.h>
#include <stdint.h>

struct pstvnc_h1_transport_runtime;
typedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;

typedef struct pstvnc_h1_rfb_transport_snapshot {
    uint32_t active;
    uint32_t queue_current;
    uint32_t queue_high_water;
    pstvnc_h1_rfb_channel_stats_t channel;
} pstvnc_h1_rfb_transport_snapshot_t;

int pstvnc_h1_rfb_transport_prepare(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_rfb_transport_release(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_rfb_transport_accept_data(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *payload,
    size_t payload_length);

int pstvnc_h1_rfb_transport_send_initial_credit(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_rfb_transport_read_exact(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t count);

int pstvnc_h1_rfb_transport_poll_receive(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_rfb_transport_write_exact(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *buffer,
    size_t count);

/*
 * Take one semaphore-consistent diagnostic snapshot without exposing mutable
 * queue ownership to the RFB parser/session coordinator.
 */
int pstvnc_h1_rfb_transport_snapshot(
    pstvnc_h1_transport_runtime_t *runtime,
    pstvnc_h1_rfb_transport_snapshot_t *snapshot);

#endif
