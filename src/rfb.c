#include "rfb.h"

#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ps2ip.h>
#include <sys/socket.h>

static int pstvnc_rfb_send_exact(
    int socket_fd,
    const void *data,
    int length)
{
    const unsigned char *cursor =
        (const unsigned char *)data;
    int complete = 0;

    while (complete < length) {
        int sent = (int)send(
            socket_fd,
            cursor + complete,
            length - complete,
            0);

        if (sent <= 0)
            return -1;

        complete += sent;
    }

    return complete;
}

static int pstvnc_rfb_recv_exact(
    int socket_fd,
    void *data,
    int length)
{
    unsigned char *cursor = (unsigned char *)data;
    int complete = 0;

    while (complete < length) {
        int received = (int)recv(
            socket_fd,
            cursor + complete,
            length - complete,
            0);

        if (received <= 0)
            return -1;

        complete += received;
    }

    return complete;
}

static unsigned int pstvnc_rfb_read_be32(
    const unsigned char *data)
{
    return
        ((unsigned int)data[0] << 24) |
        ((unsigned int)data[1] << 16) |
        ((unsigned int)data[2] << 8) |
        (unsigned int)data[3];
}

static int pstvnc_rfb_discard_exact(
    int socket_fd,
    unsigned int length)
{
    unsigned char discard[64];

    while (length > 0) {
        unsigned int chunk = length;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (pstvnc_rfb_recv_exact(
                socket_fd,
                discard,
                (int)chunk) != (int)chunk)
            return -1;

        length -= chunk;
    }

    return 0;
}

static int pstvnc_rfb_consume_rejection_reason(int socket_fd)
{
    unsigned char length_wire[4];
    unsigned int reason_length;

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            length_wire,
            sizeof(length_wire)) != sizeof(length_wire))
        return -1;

    reason_length = pstvnc_rfb_read_be32(length_wire);

    return pstvnc_rfb_discard_exact(
        socket_fd,
        reason_length);
}

static void pstvnc_rfb_sanitize_text(char *text)
{
    size_t index;

    if (text == NULL)
        return;

    for (index = 0; text[index] != '\0'; index++) {
        unsigned char value = (unsigned char)text[index];

        if (value < 32 || value > 126)
            text[index] = '.';
    }
}

void pstvnc_rfb_session_init(pstvnc_rfb_session_t *session)
{
    if (session == NULL)
        return;

    memset(session, 0, sizeof(*session));
    session->socket_fd = -1;
}

void pstvnc_rfb_session_close(pstvnc_rfb_session_t *session)
{
    if (session == NULL)
        return;

    if (session->socket_fd >= 0)
        close(session->socket_fd);

    pstvnc_rfb_session_init(session);
}

int pstvnc_rfb_connect_and_handshake(
    pstvnc_rfb_session_t *session,
    const char *host_ipv4,
    unsigned short port,
    unsigned int max_width,
    unsigned int max_height)
{
    static const unsigned char client_version[12] =
        "RFB 003.008\n";

    struct sockaddr_in server;
    int socket_fd = -1;
    unsigned char server_version[12];
    unsigned char security_count;
    unsigned char security_types[255];
    unsigned char security_choice = 1;
    unsigned char security_result[4];
    unsigned char shared_flag = 1;
    unsigned char server_init_wire[PSTVNC_RFB_SERVER_INIT_LENGTH];
    pstvnc_rfb_server_init_t server_init;
    unsigned int name_take;
    unsigned int name_remaining;

    if (session == NULL || host_ipv4 == NULL)
        return -1;

    if (session->socket_fd >= 0)
        return -1;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
        goto fail;

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host_ipv4);

    if (connect(
            socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0)
        goto fail;

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            server_version,
            sizeof(server_version)) != sizeof(server_version))
        goto fail;

    if (!pstvnc_rfb_version_is_38(server_version))
        goto fail;

    if (pstvnc_rfb_send_exact(
            socket_fd,
            client_version,
            sizeof(client_version)) != sizeof(client_version))
        goto fail;

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            &security_count,
            sizeof(security_count)) != sizeof(security_count))
        goto fail;

    if (security_count == 0) {
        (void)pstvnc_rfb_consume_rejection_reason(socket_fd);
        goto fail;
    }

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            security_types,
            security_count) != security_count)
        goto fail;

    if (!pstvnc_rfb_security_none_offered(
            security_types,
            security_count))
        goto fail;

    if (pstvnc_rfb_send_exact(
            socket_fd,
            &security_choice,
            sizeof(security_choice)) != sizeof(security_choice))
        goto fail;

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            security_result,
            sizeof(security_result)) != sizeof(security_result))
        goto fail;

    if (pstvnc_rfb_read_be32(security_result) != 0)
        goto fail;

    if (pstvnc_rfb_send_exact(
            socket_fd,
            &shared_flag,
            sizeof(shared_flag)) != sizeof(shared_flag))
        goto fail;

    if (pstvnc_rfb_recv_exact(
            socket_fd,
            server_init_wire,
            sizeof(server_init_wire)) != sizeof(server_init_wire))
        goto fail;

    if (pstvnc_rfb_parse_server_init(
            server_init_wire,
            max_width,
            max_height,
            &server_init) < 0)
        goto fail;

    name_take = server_init.name_length;

    if (name_take >= PSTVNC_RFB_DESKTOP_NAME_CAPACITY)
        name_take = PSTVNC_RFB_DESKTOP_NAME_CAPACITY - 1;

    if (name_take > 0) {
        if (pstvnc_rfb_recv_exact(
                socket_fd,
                session->desktop_name,
                (int)name_take) != (int)name_take)
            goto fail;
    }

    session->desktop_name[name_take] = '\0';
    pstvnc_rfb_sanitize_text(session->desktop_name);

    name_remaining = server_init.name_length - name_take;

    if (pstvnc_rfb_discard_exact(
            socket_fd,
            name_remaining) < 0)
        goto fail;

    session->server_init = server_init;
    session->socket_fd = socket_fd;

    return 0;

fail:
    if (socket_fd >= 0)
        close(socket_fd);

    pstvnc_rfb_session_init(session);
    return -1;
}
