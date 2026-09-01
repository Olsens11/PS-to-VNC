/*
 * File synopsis:
 * Declares PS2 private-link and socket lifecycle operations while leaving RFB
 * semantics to the protocol layer.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "PS2 system and
 * private-Ethernet platform seam".
 */

#ifndef PSTVNC_PS2_NETWORK_H
#define PSTVNC_PS2_NETWORK_H

#define PSTVNC_PS2_LOCAL_IP "192.168.50.2"
#define PSTVNC_PS2_NETMASK "255.255.255.0"
#define PSTVNC_PS2_GATEWAY_IP "192.168.50.1"
#define PSTVNC_PS2_VNC_SERVER_IP "192.168.50.1"
#define PSTVNC_PS2_VNC_SERVER_PORT 5900

/*
 * Load the qualified PS2 Ethernet module stack and initialize PS2IP for the
 * fixed private-link bootstrap topology. The IOP must already be prepared.
 */
int pstvnc_ps2_network_init(void);

/* Wait for the Ethernet carrier using the bounded qualified startup policy. */
int pstvnc_ps2_network_wait_link(void);

/* Open one blocking TCP connection to the fixed PS2-facing VNC endpoint. */
int pstvnc_ps2_network_connect_vnc(void);

/* Close a socket and discard any buffered RFB bytes owned by that socket. */
void pstvnc_ps2_network_close(int socket_fd);

#endif
