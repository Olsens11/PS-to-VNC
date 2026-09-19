/*
 * File synopsis:
 * Implements the ordered logical-RFB finite-session quiesce process owned by
 * Transport: Pi REQUEST, PS2 BOUNDARY, Pi COMMIT, PS2 COMPLETE. Residual queued
 * bytes after COMMIT are snapshotted and discarded explicitly without being
 * misclassified as parser consumption or receiver credit.
 *
 * This file coordinates only Transport-owned runtime state and the serialized
 * physical send path. It does not decide when the application has reached a
 * safe RFB message boundary; the caller supplies that policy boundary.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include "runtime.h"

#include <kernel.h>

static int pstvnc_transport_runtime_send_quiesce_marker(
    pstvnc_transport_runtime_t *runtime)
{
    return pstvnc_transport_runtime_submit_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
}

int pstvnc_transport_runtime_rfb_quiesce_requested(
    pstvnc_transport_runtime_t *runtime)
{
    int requested;

    if (runtime == NULL || !runtime->initialized)
        return -1;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return -1;

    requested = runtime->rfb_quiesce_request_received != 0u;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return -1;
    }

    return requested;
}

int pstvnc_transport_runtime_rfb_send_quiesce_boundary(
    pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->initialized || runtime->failed)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_quiesce_request_received == 0u ||
        runtime->rfb_quiesce_boundary_sent != 0u ||
        runtime->rfb_quiesce_commit_received != 0u ||
        runtime->rfb_quiesce_complete_sent != 0u) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        return 0;
    }

    /*
     * Publish BOUNDARY state before the wire send so an immediately returning
     * COMMIT can be accepted by the sole receiver. A failed send is fatal to the
     * session, so no recoverable state must be restored afterward.
     */
    runtime->rfb_quiesce_boundary_sent = 1u;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (!pstvnc_transport_runtime_send_quiesce_marker(runtime)) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_rfb_wait_quiesce_commit(
    pstvnc_transport_runtime_t *runtime)
{
    uint32_t activity_sequence;

    if (runtime == NULL || !runtime->initialized)
        return 0;

    if (!pstvnc_transport_runtime_rfb_activity_snapshot(
            runtime, &activity_sequence))
        return 0;

    for (;;) {
        int committed;
        int terminal;

        if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
            return 0;

        committed = runtime->rfb_quiesce_commit_received != 0u;
        terminal = runtime->failed || runtime->receiver_done;

        if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }

        if (committed)
            return 1;
        if (terminal)
            return 0;

        if (!pstvnc_transport_runtime_rfb_wait_activity(
                runtime, &activity_sequence))
            return 0;
    }
}

int pstvnc_transport_runtime_rfb_snapshot_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t *residual_count)
{
    if (runtime == NULL || residual_count == NULL ||
        !runtime->initialized || runtime->failed)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_quiesce_commit_received == 0u ||
        runtime->rfb_quiesce_complete_sent != 0u) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        return 0;
    }

    *residual_count =
        pstvnc_transport_rfb_channel_available(&runtime->rfb_channel);

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_rfb_discard_quiesce_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t expected_count,
    size_t *discarded_count)
{
    int discarded;

    if (discarded_count != NULL)
        *discarded_count = 0u;

    if (runtime == NULL || !runtime->initialized || runtime->failed)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_quiesce_commit_received == 0u ||
        runtime->rfb_quiesce_complete_sent != 0u) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        return 0;
    }

    discarded = pstvnc_transport_rfb_channel_discard_residual(
        &runtime->rfb_channel,
        expected_count,
        discarded_count) == 0;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return discarded;
}

int pstvnc_transport_runtime_rfb_send_quiesce_complete(
    pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->initialized || runtime->failed)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_quiesce_request_received == 0u ||
        runtime->rfb_quiesce_boundary_sent == 0u ||
        runtime->rfb_quiesce_commit_received == 0u ||
        runtime->rfb_quiesce_complete_sent != 0u ||
        pstvnc_transport_rfb_channel_available(&runtime->rfb_channel) != 0u) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        return 0;
    }

    runtime->rfb_quiesce_complete_sent = 1u;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (!pstvnc_transport_runtime_send_quiesce_marker(runtime)) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}
