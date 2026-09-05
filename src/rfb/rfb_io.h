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

/*
 * Check for receive data without blocking the main RFB owner.
 *
 * The transport may prefetch available socket bytes into its private exact-read
 * buffer. Prefetched bytes remain unconsumed from the protocol layer's point of
 * view and are returned by later pstvnc_rfb_io_read_exact() calls.
 *
 * Returns:
 *   1 -> at least one protocol byte is buffered and ready;
 *   0 -> no protocol byte is currently available;
 *  -1 -> socket closure, invalid state, or transport failure.
 *
 * This operation is intended for complete RFB server-message boundaries. It
 * does not make partially consumed protocol structures resumable.
 */
int pstvnc_rfb_io_poll_receive(
    int socket_fd);

int pstvnc_rfb_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count);

#endif
