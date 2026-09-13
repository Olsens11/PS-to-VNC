/*
 * File synopsis:
 * Binds the unchanged clean RFB session I/O seam to H1 logical channel 1.
 *
 * No physical socket is opened here. The cumulative H1 transport remains the
 * sole owner of the one PSTV TCP connection and physical receive thread. The
 * historical RFB session's integer socket handle is used only as a fail-closed
 * identity check against the bound H1 runtime; all actual reads/writes are
 * delegated to logical channel-1 mechanics.
 *
 * Diagnostic derivative note:
 * This branch also records observation-only RFB I/O witnesses in three
 * live-only producer telemetry fields while MEDIA_END has not yet arrived.
 * producer_stop_reason is deliberately left untouched for the dedicated MPEG
 * last-stage witness. MEDIA_END remains authoritative and overwrites the live
 * diagnostic producer fields with real terminal producer metadata later.
 * No RFB framing, queue, credit, or scheduling policy is changed. The witness
 * identifies whether the parser is polling at a complete message boundary,
 * blocked on one exact read (including requested byte count), or publishing
 * one exact client message.
 */

#include "h1_rfb_mux_io.h"
#include "h1_rfb_transport_live.h"
#include "h1_transport_runtime.h"

#include <stdint.h>

#define H1_RFB_DIAG_READ_ENTER       0xE1010001u
#define H1_RFB_DIAG_READ_RETURN_OK   0xE1010002u
#define H1_RFB_DIAG_READ_RETURN_FAIL 0xE10100FFu
#define H1_RFB_DIAG_POLL_ENTER       0xE1020001u
#define H1_RFB_DIAG_POLL_IDLE        0xE1020002u
#define H1_RFB_DIAG_POLL_READY       0xE1020003u
#define H1_RFB_DIAG_POLL_FAIL        0xE10200FFu
#define H1_RFB_DIAG_WRITE_ENTER_BASE 0xE1030000u
#define H1_RFB_DIAG_WRITE_OK_BASE    0xE1040000u
#define H1_RFB_DIAG_WRITE_FAIL_BASE  0xE1FF0000u

static pstvnc_h1_transport_runtime_t *h1_rfb_bound_runtime;
static uint32_t h1_rfb_diag_operation_sequence;

static pstvnc_h1_transport_runtime_t *h1_rfb_resolve(int socket_fd)
{
    if (h1_rfb_bound_runtime == NULL ||
        socket_fd < 0 ||
        h1_rfb_bound_runtime->socket_fd != socket_fd ||
        !h1_rfb_bound_runtime->rfb_resources.active)
        return NULL;

    return h1_rfb_bound_runtime;
}

/*
 * Reuse three live-only producer telemetry slots as an RFB observation channel.
 * The real producer metadata is not authoritative until MEDIA_END, so stop
 * writing these witnesses as soon as end_received becomes true.
 *
 * producer_audio_bytes    = stage marker
 * producer_mpeg_bytes     = exact byte count associated with this operation
 * producer_picture_starts = monotonically increasing RFB-I/O witness sequence
 * producer_stop_reason    = reserved for persistent MPEG last-stage witness
 */
static void h1_rfb_diag_record(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t stage,
    size_t count)
{
    if (runtime == NULL || runtime->end_received != 0u)
        return;

    if (h1_rfb_diag_operation_sequence != UINT32_MAX)
        h1_rfb_diag_operation_sequence++;

    runtime->producer_audio_bytes = stage;
    runtime->producer_mpeg_bytes =
        count > UINT32_MAX ? UINT32_MAX : (uint32_t)count;
    runtime->producer_picture_starts = h1_rfb_diag_operation_sequence;
}

void pstvnc_h1_rfb_mux_io_diag_stage(
    int socket_fd,
    uint32_t stage,
    size_t count)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);

    if (runtime == NULL)
        return;

    h1_rfb_diag_record(runtime, stage, count);
}

int pstvnc_h1_rfb_mux_io_bind(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        runtime->socket_fd < 0 ||
        !runtime->rfb_resources.active)
        return 0;

    if (h1_rfb_bound_runtime != NULL &&
        h1_rfb_bound_runtime != runtime)
        return 0;

    h1_rfb_bound_runtime = runtime;
    h1_rfb_diag_operation_sequence = 0u;
    return 1;
}

void pstvnc_h1_rfb_mux_io_unbind(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (h1_rfb_bound_runtime == runtime)
        h1_rfb_bound_runtime = NULL;
}

int pstvnc_h1_rfb_mux_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);
    int result;

    if (runtime == NULL)
        return -1;

    h1_rfb_diag_record(runtime, H1_RFB_DIAG_READ_ENTER, count);
    result = pstvnc_h1_rfb_transport_read_exact(runtime, buffer, count);
    h1_rfb_diag_record(
        runtime,
        result == 0 ? H1_RFB_DIAG_READ_RETURN_OK : H1_RFB_DIAG_READ_RETURN_FAIL,
        count);
    return result;
}

int pstvnc_h1_rfb_mux_io_poll_receive(
    int socket_fd)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);
    int result;

    if (runtime == NULL)
        return -1;

    h1_rfb_diag_record(runtime, H1_RFB_DIAG_POLL_ENTER, 0u);
    result = pstvnc_h1_rfb_transport_poll_receive(runtime);

    if (result < 0) {
        h1_rfb_diag_record(runtime, H1_RFB_DIAG_POLL_FAIL, 0u);
    } else if (result == 0) {
        h1_rfb_diag_record(runtime, H1_RFB_DIAG_POLL_IDLE, 0u);
    } else {
        h1_rfb_diag_record(runtime, H1_RFB_DIAG_POLL_READY, 0u);
    }

    return result;
}

int pstvnc_h1_rfb_mux_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);
    const uint8_t *bytes = (const uint8_t *)buffer;
    uint32_t message_type = count != 0u && bytes != NULL ? bytes[0] : 0xffu;
    int result;

    if (runtime == NULL)
        return -1;

    h1_rfb_diag_record(
        runtime,
        H1_RFB_DIAG_WRITE_ENTER_BASE | (message_type & 0xffu),
        count);

    result = pstvnc_h1_rfb_transport_write_exact(runtime, buffer, count);

    h1_rfb_diag_record(
        runtime,
        (result == 0 ? H1_RFB_DIAG_WRITE_OK_BASE : H1_RFB_DIAG_WRITE_FAIL_BASE) |
            (message_type & 0xffu),
        count);

    return result;
}
