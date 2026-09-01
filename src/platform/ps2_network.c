#include "ps2_network.h"

#include <kernel.h>
#include <loadfile.h>
#include <netman.h>
#include <ps2ip.h>

extern unsigned char DEV9_irx[];
extern unsigned int size_DEV9_irx;

extern unsigned char NETMAN_irx[];
extern unsigned int size_NETMAN_irx;

extern unsigned char SMAP_irx[];
extern unsigned int size_SMAP_irx;

int pstvnc_ps2_network_start(void)
{
    struct ip4_addr ip;
    struct ip4_addr netmask;
    struct ip4_addr gateway;

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

    NetManInit();

    IP4_ADDR(&ip, 192, 168, 50, 2);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 50, 1);

    if (ps2ipInit(&ip, &netmask, &gateway) < 0)
        return -1;

    return 0;
}

int pstvnc_ps2_network_link_is_up(void)
{
    return NetManIoctl(
        NETMAN_NETIF_IOCTL_GET_LINK_STATUS,
        NULL,
        0,
        NULL,
        0) == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

int pstvnc_ps2_network_wait_for_link(unsigned int timeout_ms)
{
    unsigned int waited_ms = 0;

    while (!pstvnc_ps2_network_link_is_up()) {
        if (waited_ms >= timeout_ms)
            return -1;

        DelayThread(100000);
        waited_ms += 100;
    }

    return 0;
}
