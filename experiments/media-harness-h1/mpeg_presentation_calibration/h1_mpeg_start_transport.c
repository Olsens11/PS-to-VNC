/*
 * File synopsis:
 * Sends the fixed CP2P MPEG-start payload as one PS2->Pi PSTV DATA frame on
 * logical MPEG2 channel 4 using H1's existing serialized frame writer.
 *
 * Direction distinguishes this start-control frame from Pi->PS2 MPEG2 DATA.
 * The shared H1 writer remains the sole owner of socket writes, frame sequence
 * numbers, the send semaphore, framing, and transport error accounting.
 */
#include "h1_mpeg_start_transport.h"

#include "transport_protocol.h"

#include <stddef.h>
#include <stdint.h>

/* Implemented by h1_transport_runtime.c around its existing send semaphore. */
int pstvnc_h1_transport_send_frame_internal(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    const void *payload,
    size_t payload_length);

int pstvnc_h1_mpeg_start_transport_send(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t session_id,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    uint8_t payload[PSTVNC_H1_MPEG_START_WIRE_BYTES];

    if (runtime == NULL ||
        !pstvnc_h1_mpeg_start_wire_encode(payload, session_id, contract))
        return 0;

    return pstvnc_h1_transport_send_frame_internal(
        runtime,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2,
        payload,
        sizeof(payload));
}
