/*
 * File synopsis:
 * Defines the experiment-owned headless RFB session coordinator used to prove
 * the through-Issue-39 parser against H1 logical channel 1 before any GS/input
 * composition is enabled.
 *
 * This runtime owns only the CPU framebuffer allocation and RFB session loop.
 * It never initializes graphics, input, OSK, or another socket. The H1 mux
 * transport remains the sole physical PSTV owner and the unchanged RFB parser
 * reaches it only through the already-verified mux I/O seam.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_SESSION_RUNTIME_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_SESSION_RUNTIME_H

#include "framebuffer.h"
#include "rfb_session.h"

#include <stddef.h>
#include <stdint.h>

struct pstvnc_h1_transport_runtime;
typedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;

typedef struct pstvnc_h1_rfb_session_runtime_stats {
    uint32_t handshake_complete;
    uint32_t initial_frame_complete;
    uint32_t incremental_requests_sent;
    uint32_t incremental_updates_complete;
    uint32_t idle_polls;
    uint32_t quiesce_boundary_sent;
    uint32_t quiesce_commit_observed;
    uint32_t quiesce_complete_sent;
} pstvnc_h1_rfb_session_runtime_stats_t;

typedef struct pstvnc_h1_rfb_session_runtime {
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    uint16_t *pixels;
    size_t pixel_capacity;
    pstvnc_h1_rfb_session_runtime_stats_t stats;
    int initialized;
} pstvnc_h1_rfb_session_runtime_t;

void pstvnc_h1_rfb_session_runtime_init(
    pstvnc_h1_rfb_session_runtime_t *runtime);

/*
 * Run a headless 704x462 through-Issue-39 RFB session until the Pi requests
 * clean quiescence and the four-phase channel-1 marker handshake completes.
 * Presentation and input remain outside this checkpoint.
 */
int pstvnc_h1_rfb_session_runtime_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport);

void pstvnc_h1_rfb_session_runtime_shutdown(
    pstvnc_h1_rfb_session_runtime_t *runtime);

#endif
