#ifndef PSTVNC_PS2_NETWORK_H
#define PSTVNC_PS2_NETWORK_H

#define PSTVNC_PS2_IPV4 "192.168.50.2"
#define PSTVNC_PI_IPV4  "192.168.50.1"

int pstvnc_ps2_network_start(void);
int pstvnc_ps2_network_link_is_up(void);
int pstvnc_ps2_network_wait_for_link(unsigned int timeout_ms);

#endif
