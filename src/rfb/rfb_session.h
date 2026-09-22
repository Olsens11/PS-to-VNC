/*
 * File synopsis:
 * Defines synchronized RFB session state, errors, bounded scratch storage, and
 * lifecycle over the logical RFB bridge while leaving Transport lifecycle and
 * application recovery policy with their owning components. Provider-terminal
 * mechanism failures are typed independently from generic physical I/O failure.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Shared Raw
 * server-message parser"; docs/CLEAN_ARCHITECTURE.md, "RFB client/session";
 * docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md, "RFB ownership under shared
 * transport".
 */

#ifndef PSTVNC_RFB_SESSION_H
#define PSTVNC_RFB_SESSION_H

#include <stdint.h>

#include "framebuffer.h"
#include "rfb.h"
#include "transport/transport.h"

#define PSTVNC_RFB_SESSION_TEXT_MAX 127u
#define PSTVNC_RFB_SESSION_MAX_ROW_PIXELS 1920u
/* Fixed Issue #7 baseline ceiling; expand deliberately with later display work. */
#define PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS (704u * 462u)

typedef enum pstvnc_rfb_session_state {
    PSTVNC_RFB_SESSION_NEW = 0,
    PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME,
    PSTVNC_RFB_SESSION_READY,
    PSTVNC_RFB_SESSION_FAILED
} pstvnc_rfb_session_state_t;

/*
 * Result of one nonblocking live-session receive-service attempt.
 *
 * IDLE is a normal scheduling result: no byte of the next server message was
 * consumed, the session remains READY, and application/main may service other
 * work before trying again. A completed finite-session quiesce also returns at
 * this same proven boundary without consuming the next RFB message.
 */
typedef enum pstvnc_rfb_session_receive_result {
    PSTVNC_RFB_SESSION_RECEIVE_FAILED = -1,
    PSTVNC_RFB_SESSION_RECEIVE_IDLE = 0,
    PSTVNC_RFB_SESSION_RECEIVE_UPDATE = 1
} pstvnc_rfb_session_receive_result_t;

typedef enum pstvnc_rfb_session_error {
    PSTVNC_RFB_SESSION_ERROR_NONE = 0,
    PSTVNC_RFB_SESSION_ERROR_IO,
    PSTVNC_RFB_SESSION_ERROR_PROVIDER_CONNECT,
    PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ,
    PSTVNC_RFB_SESSION_ERROR_PROVIDER_WRITE,
    PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION,
    PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED,
    PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE,
    PSTVNC_RFB_SESSION_ERROR_SECURITY_RESULT,
    PSTVNC_RFB_SESSION_ERROR_SERVER_INIT,
    PSTVNC_RFB_SESSION_ERROR_GEOMETRY,
    PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE,
    PSTVNC_RFB_SESSION_ERROR_EMPTY_UPDATE,
    PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_ENCODING,
    PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS,
    PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_SIZE,
    PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE,
    PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH
} pstvnc_rfb_session_error_t;

typedef struct pstvnc_rfb_session {
    pstvnc_transport_access_t transport_access;
    pstvnc_rfb_session_state_t state;
    pstvnc_rfb_session_error_t error;
    unsigned int server_major;
    unsigned int server_minor;
    pstvnc_rfb_server_init_t server_init;
    char desktop_name[PSTVNC_RFB_SESSION_TEXT_MAX + 1u];
    char server_rejection[PSTVNC_RFB_SESSION_TEXT_MAX + 1u];
    uint16_t row_scratch[PSTVNC_RFB_SESSION_MAX_ROW_PIXELS];
} pstvnc_rfb_session_t;

void pstvnc_rfb_session_init(
    pstvnc_rfb_session_t *session);

/*
 * Start RFB protocol negotiation over the already-established logical RFB
 * bridge. Physical descriptor ownership and Transport startup are deliberately
 * outside this protocol-session contract.
 */
int pstvnc_rfb_session_start(
    pstvnc_rfb_session_t *session,
    uint16_t expected_width,
    uint16_t expected_height);

/*
 * Serialize and send one full-desktop FramebufferUpdateRequest on the owned
 * synchronized session. The caller chooses full (0) or incremental (nonzero)
 * service; ordinary Issue #7 live operation uses incremental requests.
 */
int pstvnc_rfb_session_request_update(
    pstvnc_rfb_session_t *session,
    int incremental);

/*
 * Serialize and send one native RFB KeyEvent through the main-thread-owned
 * READY session.
 *
 * This boundary owns only session validity, exact wire serialization, and
 * logical-stream failure. The caller owns X11 keysym choice, logical
 * tap/modifier sequencing, OSK state, and controller/action meaning.
 */
int pstvnc_rfb_session_send_key_event(
    pstvnc_rfb_session_t *session,
    int down,
    uint32_t keysym);

/*
 * Serialize and send one already-mapped native RFB PointerEvent through the
 * main-thread-owned READY session.
 *
 * This is a protocol/wire boundary only. The caller owns semantic-to-RFB
 * button mapping, wheel press/release policy, and the distinction between
 * locally interpreted, queued, and successfully published pointer state.
 *
 * Coordinates must lie inside the negotiated server desktop.
 */
int pstvnc_rfb_session_send_pointer_event(
    pstvnc_rfb_session_t *session,
    uint8_t button_mask,
    uint16_t x,
    uint16_t y);

int pstvnc_rfb_session_receive_initial_frame(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer);

int pstvnc_rfb_session_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer);

/*
 * Service one live framebuffer response without blocking while the server is
 * idle.
 *
 * The session may return IDLE only at a complete server-message boundary,
 * before consuming the first byte of the next server message. At that boundary
 * it also observes Transport's finite-session quiesce request and, when
 * requested, performs the bridge's ordered boundary completion before any next
 * message byte is consumed. Once a message begins, exact protocol reads finish
 * that message before another benign scheduling/quiesce yield is permitted.
 *
 * This keeps RFB framing authoritative while allowing application/main to
 * publish controller input between idle receive attempts.
 */
pstvnc_rfb_session_receive_result_t
pstvnc_rfb_session_try_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer);

#endif
