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

/*
 * Optional request/presentation flow-policy seam.
 *
 * The default entry points pass no policy and retain their qualified behavior:
 * one incremental request is sent after each complete boundary and every dirty
 * completed framebuffer update is published.
 *
 * A policy is evaluated only at the same already-qualified complete-message or
 * pre-message IDLE boundaries. It does not own the parser, socket, framebuffer,
 * presentation callback, or application service callback.
 */
typedef enum pstvnc_h1_rfb_request_policy_decision {
    PSTVNC_H1_RFB_REQUEST_POLICY_HOLD = 0,
    PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL,
    PSTVNC_H1_RFB_REQUEST_POLICY_FULL
} pstvnc_h1_rfb_request_policy_decision_t;

typedef pstvnc_h1_rfb_request_policy_decision_t
(*pstvnc_h1_rfb_next_request_callback_t)(void *context);

typedef int (*pstvnc_h1_rfb_request_sent_callback_t)(
    void *context,
    pstvnc_h1_rfb_request_policy_decision_t decision);

typedef int (*pstvnc_h1_rfb_update_complete_callback_t)(void *context);

typedef int (*pstvnc_h1_rfb_allow_present_callback_t)(void *context);

typedef struct pstvnc_h1_rfb_flow_policy {
    pstvnc_h1_rfb_next_request_callback_t next_request;
    pstvnc_h1_rfb_request_sent_callback_t request_sent;
    pstvnc_h1_rfb_update_complete_callback_t update_complete;
    pstvnc_h1_rfb_allow_present_callback_t allow_present;
    void *context;
} pstvnc_h1_rfb_flow_policy_t;

typedef struct pstvnc_h1_rfb_session_runtime_stats {
    uint32_t handshake_complete;
    uint32_t initial_frame_complete;
    uint32_t incremental_requests_sent;
    uint32_t full_requests_sent;
    uint32_t held_request_boundaries;
    uint32_t incremental_updates_complete;
    uint32_t idle_polls;
    uint32_t initial_presentations;
    uint32_t incremental_presentations;
    uint32_t suppressed_presentations;
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

/*
 * Calibration/experiment integration entry point.
 *
 * Existing entry points remain behaviorally unchanged. A non-NULL flow policy
 * may HOLD request issuance, choose incremental/full requests, account for
 * successful sends/completed updates, and suppress only the visual publication
 * of a completed dirty update while parser/framebuffer state still advances.
 */
int pstvnc_h1_rfb_session_runtime_run_with_flow_policy(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context,
    const pstvnc_h1_rfb_flow_policy_t *flow_policy);

void pstvnc_h1_rfb_session_runtime_shutdown(
    pstvnc_h1_rfb_session_runtime_t *runtime);

#endif
