#ifndef PSTVNC_RFB_H
#define PSTVNC_RFB_H

#include "rfb_protocol.h"

#define PSTVNC_RFB_DESKTOP_NAME_CAPACITY 128u

typedef struct {
    int socket_fd;
    pstvnc_rfb_server_init_t server_init;
    char desktop_name[PSTVNC_RFB_DESKTOP_NAME_CAPACITY];
} pstvnc_rfb_session_t;

void pstvnc_rfb_session_init(pstvnc_rfb_session_t *session);
void pstvnc_rfb_session_close(pstvnc_rfb_session_t *session);

/*
 * Establish one synchronized post-ServerInit RFB 3.8 session using
 * SecurityType None. The socket is published in `session` only after the
 * variable-length desktop name has been consumed completely.
 */
int pstvnc_rfb_connect_and_handshake(
    pstvnc_rfb_session_t *session,
    const char *host_ipv4,
    unsigned short port,
    unsigned int max_width,
    unsigned int max_height);

#endif
