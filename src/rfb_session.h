#ifndef PSTVNC_RFB_SESSION_H
#define PSTVNC_RFB_SESSION_H

#include <stdint.h>

#include "framebuffer.h"
#include "rfb.h"

#define PSTVNC_RFB_SESSION_TEXT_MAX 127u
#define PSTVNC_RFB_SESSION_MAX_ROW_PIXELS 1920u

typedef enum pstvnc_rfb_session_state {
    PSTVNC_RFB_SESSION_NEW = 0,
    PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME,
    PSTVNC_RFB_SESSION_READY,
    PSTVNC_RFB_SESSION_FAILED
} pstvnc_rfb_session_state_t;

typedef enum pstvnc_rfb_session_error {
    PSTVNC_RFB_SESSION_ERROR_NONE = 0,
    PSTVNC_RFB_SESSION_ERROR_IO,
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
    PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH
} pstvnc_rfb_session_error_t;

typedef struct pstvnc_rfb_session {
    int socket_fd;
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

int pstvnc_rfb_session_start(
    pstvnc_rfb_session_t *session,
    int socket_fd,
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

int pstvnc_rfb_session_receive_initial_frame(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer);

int pstvnc_rfb_session_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer);

#endif
