/*
 * File synopsis:
 * Defines the narrow CP2P seam that sends one already-prepared MPEG-start
 * contract through H1's existing serialized PSTV writer.
 *
 * This module does not own a socket, sequence counter, send semaphore, MPEG
 * producer, or calibration lifecycle. Those remain with the existing H1
 * transport and CP2P ownership modules.
 */
#ifndef PSTVNC_H1_MPEG_START_TRANSPORT_H
#define PSTVNC_H1_MPEG_START_TRANSPORT_H

#include "h1_mpeg_start_wire.h"

#include <stdint.h>

struct pstvnc_h1_transport_runtime;
typedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;

int pstvnc_h1_mpeg_start_transport_send(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t session_id,
    const pstvnc_h1_mpeg_start_contract_t *contract);

#endif /* PSTVNC_H1_MPEG_START_TRANSPORT_H */
