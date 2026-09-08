/*
 * File synopsis:
 * Declares the H1 experiment-only RFB I/O adapter used to route the clean
 * through-Issue-39 RFB session through H1's single PSTV mux connection.
 *
 * This header does not alter RFB parsing semantics and does not enable RFB by
 * itself. The first checkpoint intentionally provides only fail-closed adapter
 * entry points so the cumulative H1 build can be mechanically disconnected
 * from the direct VNC socket implementation before channel-1 runtime support is
 * added.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_MUX_IO_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_MUX_IO_H

#include <stddef.h>

int pstvnc_h1_rfb_mux_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count);

int pstvnc_h1_rfb_mux_io_poll_receive(
    int socket_fd);

int pstvnc_h1_rfb_mux_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count);

#endif
