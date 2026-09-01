#include "diagnostics.h"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <ps2ip.h>

#define PSTVNC_DIAGNOSTICS_HOST_IPV4 "192.168.50.1"
#define PSTVNC_DIAGNOSTICS_UDP_PORT 5999

static int diagnostics_socket = -1;
static struct sockaddr_in diagnostics_address;

int pstvnc_diagnostics_init(void)
{
    if (diagnostics_socket >= 0)
        return 0;

    diagnostics_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (diagnostics_socket < 0)
        return -1;

    memset(&diagnostics_address, 0, sizeof(diagnostics_address));
    diagnostics_address.sin_len = sizeof(diagnostics_address);
    diagnostics_address.sin_family = AF_INET;
    diagnostics_address.sin_port = htons(PSTVNC_DIAGNOSTICS_UDP_PORT);
    diagnostics_address.sin_addr.s_addr =
        inet_addr(PSTVNC_DIAGNOSTICS_HOST_IPV4);

    return 0;
}

int pstvnc_diagnostics_is_ready(void)
{
    return diagnostics_socket >= 0;
}

int pstvnc_diagnostics_send(const void *data, size_t length)
{
    if (diagnostics_socket < 0 || data == NULL || length == 0)
        return -1;

    return (int)sendto(
        diagnostics_socket,
        data,
        length,
        0,
        (struct sockaddr *)&diagnostics_address,
        sizeof(diagnostics_address));
}

void pstvnc_diagnostics_shutdown(void)
{
    if (diagnostics_socket >= 0)
        close(diagnostics_socket);

    diagnostics_socket = -1;
    memset(&diagnostics_address, 0, sizeof(diagnostics_address));
}
