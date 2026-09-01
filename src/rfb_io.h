/*
 * File synopsis:
 * Defines exact-read/exact-write transport operations that preserve protocol
 * framing across arbitrary TCP packet boundaries.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "RFB connection and
 * wire contract" and "PS2 system and private-Ethernet platform seam".
 */

#ifndef PSTVNC_RFB_IO_H
#define PSTVNC_RFB_IO_H

#include <stddef.h>

/*
 * Exact startup/session I/O seam.
 *
 * Return 0 only when exactly count bytes were transferred. Any other return
 * means the current operation failed. The PS2 implementation owns the socket
 * mechanics; host tests provide a scripted implementation of these two direct
 * functions.
 */
int pstvnc_rfb_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count);

int pstvnc_rfb_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count);

#endif
