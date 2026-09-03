/*
 * File synopsis:
 * Owns qualified PS2 Ethernet startup, fixed private-link configuration, VNC
 * connection, and buffered exact RFB I/O.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "PS2 system and
 * private-Ethernet platform seam".
 */

#include <kernel.h>
#include <loadfile.h>
#include <netman.h>
#include <ps2ip.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stddef.h>
#include <string.h>

#include "../rfb_io.h"
#include "ps2_network.h"

extern unsigned char DEV9_irx[];
extern unsigned int size_DEV9_irx;
extern unsigned char NETMAN_irx[];
extern unsigned int size_NETMAN_irx;
extern unsigned char SMAP_irx[];
extern unsigned int size_SMAP_irx;

#define PSTVNC_PS2_RFB_RX_BUFFER_SIZE 32768u

/*
 * TCP is a byte stream: one recv() need not equal one RFB field or message.
 * This private buffer lets the protocol layer request exact byte counts without
 * depending on packet boundaries. It belongs to the single main-thread socket
 * owner and is reset whenever that socket identity changes.
 */
static unsigned char rfb_rx_buffer[PSTVNC_PS2_RFB_RX_BUFFER_SIZE];
static size_t rfb_rx_pos;
static size_t rfb_rx_end;
static int rfb_rx_socket = -1;

static void reset_rfb_rx(int socket_fd)
{
    rfb_rx_pos = 0;
    rfb_rx_end = 0;
    rfb_rx_socket = socket_fd;
}

static void link_wait_alarm(s32 alarm_id, u16 time, void *common)
{
    (void)alarm_id;
    (void)time;
    iWakeupThread(*(int *)common);
}

static int link_is_up(void)
{
    return NetManIoctl(
        NETMAN_NETIF_IOCTL_GET_LINK_STATUS,
        NULL,
        0,
        NULL,
        0) == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

int pstvnc_ps2_network_init(void)
{
    struct ip4_addr local_ip;
    struct ip4_addr netmask;
    struct ip4_addr gateway;

    /*
     * These embedded IOP modules form the qualified PS2 Ethernet stack. Their
     * load order is a hardware/platform invariant: DEV9 hardware support,
     * network-manager abstraction, then the SMAP Ethernet driver.
     */
    if (SifExecModuleBuffer(
            DEV9_irx,
            size_DEV9_irx,
            0,
            NULL,
            NULL) < 0)
        return -1;

    if (SifExecModuleBuffer(
            NETMAN_irx,
            size_NETMAN_irx,
            0,
            NULL,
            NULL) < 0)
        return -1;

    if (SifExecModuleBuffer(
            SMAP_irx,
            size_SMAP_irx,
            0,
            NULL,
            NULL) < 0)
        return -1;

    if (NetManInit() < 0)
        return -1;

    /*
     * The first clean milestone uses a deliberately fixed private link. The PS2
     * is .2 and the directly attached Pi is both peer and gateway at .1; DHCP
     * and portable network configuration are later product responsibilities.
     */
    IP4_ADDR(&local_ip, 192, 168, 50, 2);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 50, 1);

    if (ps2ipInit(&local_ip, &netmask, &gateway) < 0)
        return -1;

    reset_rfb_rx(-1);
    return 0;
}

int pstvnc_ps2_network_wait_link(void)
{
    int thread_id = GetThreadId();
    int retry_cycles;

    /*
     * Sleep through bounded alarm-driven intervals rather than busy-spinning
     * the EE. Failure remains explicit after the startup allowance; this is not
     * the silent-stall auto-recovery policy deferred during debugging.
     */
    for (retry_cycles = 0; !link_is_up(); retry_cycles++) {
        if (SetAlarm(1000 * 16, &link_wait_alarm, &thread_id) < 0)
            return -1;

        SleepThread();

        if (retry_cycles >= 9)
            return -1;
    }

    return 0;
}

int pstvnc_ps2_network_connect_vnc(void)
{
    int socket_fd;
    struct sockaddr_in server;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        return -1;

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(PSTVNC_PS2_VNC_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(PSTVNC_PS2_VNC_SERVER_IP);

    if (connect(
            socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {
        close(socket_fd);
        return -1;
    }

    reset_rfb_rx(socket_fd);
    return socket_fd;
}

void pstvnc_ps2_network_close(int socket_fd)
{
    if (socket_fd < 0)
        return;

    close(socket_fd);

    if (rfb_rx_socket == socket_fd)
        reset_rfb_rx(-1);
}

int pstvnc_rfb_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    unsigned char *destination = (unsigned char *)buffer;
    size_t done = 0;

    if (socket_fd < 0 || (buffer == NULL && count != 0))
        return -1;

    if (rfb_rx_socket != socket_fd)
        reset_rfb_rx(socket_fd);

    /*
     * Satisfy the caller's protocol-sized request even when TCP returns fewer
     * or more bytes than requested. Extra received bytes stay buffered for the
     * next exact read, preserving RFB framing.
     */
    while (done < count) {
        size_t available = rfb_rx_end - rfb_rx_pos;

        if (available > 0) {
            size_t need = count - done;
            size_t take = available < need ? available : need;

            memcpy(
                destination + done,
                rfb_rx_buffer + rfb_rx_pos,
                take);

            rfb_rx_pos += take;
            done += take;

            if (rfb_rx_pos == rfb_rx_end) {
                rfb_rx_pos = 0;
                rfb_rx_end = 0;
            }

            continue;
        }

        {
            int received = recv(
                socket_fd,
                rfb_rx_buffer,
                sizeof(rfb_rx_buffer),
                0);

            if (received <= 0)
                return -1;

            rfb_rx_pos = 0;
            rfb_rx_end = (size_t)received;
        }
    }

    return 0;
}

int pstvnc_rfb_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    const unsigned char *source = (const unsigned char *)buffer;
    size_t done = 0;

    if (socket_fd < 0 || (buffer == NULL && count != 0))
        return -1;

    /*
     * A successful send() may still accept only part of the message. Continue
     * until every byte is owned by the socket or fail the synchronized session;
     * a truncated RFB client message cannot be treated as success.
     */
    while (done < count) {
        int sent = send(
            socket_fd,
            source + done,
            count - done,
            0);

        if (sent <= 0)
            return -1;

        done += (size_t)sent;
    }

    return 0;
}
