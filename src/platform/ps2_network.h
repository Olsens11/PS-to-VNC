/*
 * File synopsis:
 * Declares PS2 private-link setup and pre-adoption PSTV socket lifecycle
 * operations. After successful Transport adoption, the platform network seam no
 * longer owns or receives from the physical PSTV descriptor.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "PS2 system and
 * private-Ethernet platform seam"; docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md.
 */

#ifndef PSTVNC_PS2_NETWORK_H
#define PSTVNC_PS2_NETWORK_H

#define PSTVNC_PS2_LOCAL_IP "192.168.50.2"
#define PSTVNC_PS2_NETMASK "255.255.255.0"
#define PSTVNC_PS2_GATEWAY_IP "192.168.50.1"
#define PSTVNC_PS2_PSTV_SERVER_IP "192.168.50.1"
#define PSTVNC_PS2_PSTV_SERVER_PORT 5902

int pstvnc_ps2_network_init(void);
int pstvnc_ps2_network_wait_link(void);

/* Return one caller-owned PSTV descriptor. Transport adoption follows. */
int pstvnc_ps2_network_connect_pstv(void);

/* Close only a descriptor still owned by the caller (never an adopted one). */
void pstvnc_ps2_network_close(int socket_fd);

#endif