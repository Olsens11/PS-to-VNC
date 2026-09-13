/*
 * File synopsis:
 * Observation-only CP2P transport derivative for the MPEG consumer stall.
 *
 * The authoritative H1 transport implementation is included unchanged with
 * only its two public MPEG-read symbols mechanically renamed. This file then
 * re-exposes those two symbols with a byte-for-byte-equivalent copy of the
 * cancellable read loop plus persistent live stage writes.
 *
 * No queue capacity, semaphore operation, credit policy, delay, scheduling,
 * cancellation, transport ownership, or error behavior is changed. The stage
 * word uses the already-dedicated live producer_stop_reason witness established
 * by the parent diagnostic and stops writing once MEDIA_END is authoritative.
 */

#define pstvnc_h1_transport_mpeg_read_cancellable \
    pstvnc_h1_transport_mpeg_read_cancellable_uninstrumented
#define pstvnc_h1_transport_mpeg_read \
    pstvnc_h1_transport_mpeg_read_uninstrumented
#include "h1_transport_runtime.c"
#undef pstvnc_h1_transport_mpeg_read
#undef pstvnc_h1_transport_mpeg_read_cancellable

#define H1_MPEG_READ_DIAG_FAMILY 0xDA000000u

#define H1_MPEG_READ_DIAG_ENTER               0x01u
#define H1_MPEG_READ_DIAG_LOOP_ENTER          0x02u
#define H1_MPEG_READ_DIAG_BEFORE_QUEUE_WAIT   0x03u
#define H1_MPEG_READ_DIAG_QUEUE_WAIT_RETURN   0x04u
#define H1_MPEG_READ_DIAG_QUEUE_SAMPLED       0x05u
#define H1_MPEG_READ_DIAG_BEFORE_QUEUE_READ   0x06u
#define H1_MPEG_READ_DIAG_QUEUE_READ_RETURN   0x07u
#define H1_MPEG_READ_DIAG_BEFORE_QUEUE_SIGNAL 0x08u
#define H1_MPEG_READ_DIAG_QUEUE_SIGNAL_RETURN 0x09u
#define H1_MPEG_READ_DIAG_BEFORE_CREDIT       0x0Au
#define H1_MPEG_READ_DIAG_CREDIT_RETURN       0x0Bu
#define H1_MPEG_READ_DIAG_RETURN_SUCCESS      0x0Cu
#define H1_MPEG_READ_DIAG_BEFORE_EMPTY_DELAY  0x0Du
#define H1_MPEG_READ_DIAG_EMPTY_DELAY_RETURN  0x0Eu
#define H1_MPEG_READ_DIAG_CANCELLED           0x0Fu
#define H1_MPEG_READ_DIAG_END_EMPTY           0x10u
#define H1_MPEG_READ_DIAG_ERROR               0xFFu

static void h1_mpeg_read_diag(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t operation,
    unsigned int wait_loops)
{
    uint32_t word;

    if (runtime == NULL || runtime->end_received != 0u)
        return;

    word = H1_MPEG_READ_DIAG_FAMILY |
        ((operation & 0xffu) << 16) |
        ((uint32_t)wait_loops & 0xffffu);
    runtime->producer_stop_reason = word;
}

int pstvnc_h1_transport_mpeg_read_cancellable(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read,
    const volatile int *cancel_requested)
{
    uint8_t *destination = (uint8_t *)buffer;
    unsigned int wait_loops = 0u;

    if (runtime == NULL || buffer == NULL || bytes_read == NULL ||
        maximum_count == 0u ||
        runtime->config.video_mode != PSTVNC_H1_VIDEO_MPEG2_ES) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return 0;
    }

    *bytes_read = 0u;
    runtime->stats.mpeg_read_calls++;
    h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ENTER, wait_loops);

    for (;;) {
        size_t available;
        size_t take;
        int queue_empty;
        int end_received;

        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_LOOP_ENTER, wait_loops);

        if (cancel_requested != NULL && *cancel_requested) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_CANCELLED, wait_loops);
            return 0;
        }

        if (runtime->error != PSTVNC_H1_ERROR_NONE) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
            return 0;
        }

        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_BEFORE_QUEUE_WAIT, wait_loops);
        if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }
        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_QUEUE_WAIT_RETURN, wait_loops);

        available = pstvnc_transport_queue_size(&runtime->mpeg_queue);
        take = available < maximum_count ? available : maximum_count;
        end_received = runtime->end_received;
        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_QUEUE_SAMPLED, wait_loops);

        if (take != 0u) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_BEFORE_QUEUE_READ, wait_loops);
            if (!pstvnc_transport_queue_read(
                    &runtime->mpeg_queue,
                    destination,
                    take)) {
                (void)SignalSema(runtime->mpeg_queue_sema_id);
                h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
                h1_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
                return 0;
            }
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_QUEUE_READ_RETURN, wait_loops);
        }

        queue_empty = pstvnc_transport_queue_size(&runtime->mpeg_queue) == 0u;

        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_BEFORE_QUEUE_SIGNAL, wait_loops);
        if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }
        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_QUEUE_SIGNAL_RETURN, wait_loops);

        if (take != 0u) {
            runtime->stats.mpeg_bytes_consumed += (uint32_t)take;

            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_BEFORE_CREDIT, wait_loops);
            if (!h1_return_credit(
                    runtime,
                    PSTVNC_TRANSPORT_CHANNEL_MPEG2,
                    (uint32_t)take,
                    queue_empty)) {
                h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
                return 0;
            }
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_CREDIT_RETURN, wait_loops);

            if (wait_loops != 0u) {
                runtime->stats.mpeg_wait_events++;
                runtime->stats.mpeg_wait_loops += wait_loops;
                if (wait_loops > runtime->stats.mpeg_wait_max_loops)
                    runtime->stats.mpeg_wait_max_loops = wait_loops;
            }

            *bytes_read = take;
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_RETURN_SUCCESS, wait_loops);
            return 1;
        }

        if (end_received) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_END_EMPTY, wait_loops);
            return 0;
        }

        wait_loops++;

        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_BEFORE_EMPTY_DELAY, wait_loops);
        if (DelayThread(runtime->config.mpeg_empty_delay_us) < 0) {
            h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_ERROR, wait_loops);
            h1_record_error(runtime, PSTVNC_H1_ERROR_THREAD_DELAY);
            return 0;
        }
        h1_mpeg_read_diag(runtime, H1_MPEG_READ_DIAG_EMPTY_DELAY_RETURN, wait_loops);
    }
}

int pstvnc_h1_transport_mpeg_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read)
{
    return pstvnc_h1_transport_mpeg_read_cancellable(
        runtime, buffer, maximum_count, bytes_read, NULL);
}
