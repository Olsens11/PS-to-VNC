/*
 * File synopsis:
 * Defines the experiment-owned RFB session coordinator used to prove the
 * through-Issue-39 parser against H1 logical channel 1. Headless operation
 * remains the default; callers may optionally publish complete framebuffer
 * states and service application work only at complete RFB message boundaries.
 *
 * This runtime owns the CPU framebuffer allocation and RFB session loop. It
 * never initializes graphics, input, OSK, or another socket. The H1 mux
 * transport remains the sole physical PSTV owner and the unchanged RFB parser
 * reaches it only through the already-verified mux I/O seam. Presentation and
 * application-side input routing, when selected, remain outside this module
 * behind deliberately narrow callbacks.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_SESSION_RUNTIME_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_SESSION_RUNTIME_H

#include "framebuffer.h"
#include "rfb_session.h"

#include <stddef.h>
#include <stdint.h>

struct pstvnc_h1_transport_runtime;
typedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;

typedef int (*pstvnc_h1_rfb_present_callback_t)(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);

/*
 * Optional application/main-thread service seam.
 *
 * The coordinator invokes this callback only at complete RFB server-message
 * boundaries. The callback may drain semantic input and publish ordinary RFB
 * input writes through the already-synchronized session. It must not read the
 * physical PSTV socket or consume server bytes.
 */
typedef int (*pstvnc_h1_rfb_service_callback_t)(
    void *context,
    pstvnc_rfb_session_t *session);

typedef struct pstvnc_h1_rfb_session_runtime_stats {
    uint32_t handshake_complete;
    uint32_t initial_frame_complete;
    uint32_t incremental_requests_sent;
    uint32_t incremental_updates_complete;
    uint32_t idle_polls;
    uint32_t initial_presentations;
    uint32_t incremental_presentations;
    uint32_t application_service_calls;
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
 * Run the already-qualified headless 704x462 through-Issue-39 RFB session until
 * the Pi requests clean quiescence and the four-phase channel-1 marker handshake
 * completes. No graphics or application-service callback is invoked.
 */
int pstvnc_h1_rfb_session_runtime_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport);

/*
 * Run the same RFB transport/parser/quiesce state machine while publishing only
 * complete authoritative framebuffer states through the caller-owned callback.
 * The callback is invoked once after the complete initial frame and thereafter
 * only for completed incremental updates that left framebuffer.dirty set.
 *
 * This is the CP2K-visible entry point. It deliberately performs no application
 * service so the exact CP2K behavior remains available after the service seam is
 * added for a later checkpoint.
 */
int pstvnc_h1_rfb_session_runtime_run_with_presenter(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context);

/*
 * Run the same visible RFB state machine and additionally service caller-owned
 * application work at clean server-message boundaries.
 *
 * The service callback runs on the coordinator/main thread after any already
 * observed quiesce request has been honored, and before another framebuffer
 * request is sent. This keeps pointer/key serialization ordered with RFB
 * requests while preserving the sole physical PSTV recv() owner.
 */
int pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context);

void pstvnc_h1_rfb_session_runtime_shutdown(
    pstvnc_h1_rfb_session_runtime_t *runtime);

#endif
