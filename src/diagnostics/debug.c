/*
 * File synopsis:
 * Implements the richer retained debug-stage model and UDP transport without
 * owning the application behavior being observed.
 */

#include "diagnostics/debug.h"

#include <arpa/inet.h>
#include <string.h>

#include <ps2ip.h>

/*
 * Existing PS-to-VNC diagnostic transport contract.
 *
 * The identity sendto() wrapper recognizes diagnostic UDP traffic on this
 * exact port.  Keep this endpoint behavior unchanged during M4I-C2.
 */
#define PSTVNC_DEBUG_UDP_PORT 5999
#define PSTVNC_DEBUG_HOST_IPV4 "192.168.50.1"

static int pstvnc_debug_udp_socket = -1;
static struct sockaddr_in pstvnc_debug_udp_address;

static volatile pstvnc_debug_stage_t pstvnc_debug_current_stage =
    PSTVNC_DEBUG_STAGE_STARTUP;

int pstvnc_debug_init(void)
{
    pstvnc_debug_udp_socket =
        socket(AF_INET, SOCK_DGRAM, 0);

    if (pstvnc_debug_udp_socket < 0)
        return -1;

    memset(
        &pstvnc_debug_udp_address,
        0,
        sizeof(pstvnc_debug_udp_address));

    pstvnc_debug_udp_address.sin_len =
        sizeof(pstvnc_debug_udp_address);

    pstvnc_debug_udp_address.sin_family =
        AF_INET;

    pstvnc_debug_udp_address.sin_port =
        htons(PSTVNC_DEBUG_UDP_PORT);

    pstvnc_debug_udp_address.sin_addr.s_addr =
        inet_addr(PSTVNC_DEBUG_HOST_IPV4);

    return 0;
}

int pstvnc_debug_is_ready(void)
{
    return pstvnc_debug_udp_socket >= 0;
}

int pstvnc_debug_send(
    const void *data,
    size_t length)
{
    if (pstvnc_debug_udp_socket < 0)
        return -1;

    if (data == NULL || length == 0)
        return -1;

    return (int)sendto(
        pstvnc_debug_udp_socket,
        data,
        length,
        0,
        (struct sockaddr *)&pstvnc_debug_udp_address,
        sizeof(pstvnc_debug_udp_address));
}

void pstvnc_debug_set_stage(
    pstvnc_debug_stage_t stage)
{
    pstvnc_debug_current_stage = stage;
}

pstvnc_debug_stage_t pstvnc_debug_get_stage(void)
{
    return pstvnc_debug_current_stage;
}

const char *pstvnc_debug_stage_name(
    pstvnc_debug_stage_t stage)
{
    switch (stage) {
        case PSTVNC_DEBUG_STAGE_STARTUP:
            return "STARTUP";
        case PSTVNC_DEBUG_STAGE_NET_READY:
            return "NET_READY";
        case PSTVNC_DEBUG_STAGE_GS_INIT:
            return "GS_INIT";
        case PSTVNC_DEBUG_STAGE_INITIAL_TEXTURE:
            return "INITIAL_TEXTURE";
        case PSTVNC_DEBUG_STAGE_CONTROLLER_START:
            return "CONTROLLER_START";
        case PSTVNC_DEBUG_STAGE_LOOP_BEGIN:
            return "LOOP_BEGIN";
        case PSTVNC_DEBUG_STAGE_PRE_DRAW:
            return "PRE_DRAW";
        case PSTVNC_DEBUG_STAGE_POST_DRAW:
            return "POST_DRAW";
        case PSTVNC_DEBUG_STAGE_PRE_FLIP:
            return "PRE_FLIP";
        case PSTVNC_DEBUG_STAGE_POST_FLIP:
            return "POST_FLIP";
        case PSTVNC_DEBUG_STAGE_PRE_QUEUE:
            return "PRE_QUEUE";
        case PSTVNC_DEBUG_STAGE_POST_QUEUE:
            return "POST_QUEUE";
        case PSTVNC_DEBUG_STAGE_PRE_REQUEST:
            return "PRE_REQUEST";
        case PSTVNC_DEBUG_STAGE_POST_REQUEST:
            return "POST_REQUEST";
        case PSTVNC_DEBUG_STAGE_WAIT_RFB:
            return "WAIT_RFB";
        case PSTVNC_DEBUG_STAGE_POST_RECEIVE:
            return "POST_RECEIVE";
        case PSTVNC_DEBUG_STAGE_PRE_TEXTURE:
            return "PRE_TEXTURE";
        case PSTVNC_DEBUG_STAGE_POST_TEXTURE:
            return "POST_TEXTURE";
        case PSTVNC_DEBUG_STAGE_CONTROLLER_ERROR:
            return "CONTROLLER_ERROR";
        case PSTVNC_DEBUG_STAGE_REQUEST_ERROR:
            return "REQUEST_ERROR";
        case PSTVNC_DEBUG_STAGE_RECEIVE_ERROR:
            return "RECEIVE_ERROR";
        default:
            return "UNKNOWN";
    }
}
