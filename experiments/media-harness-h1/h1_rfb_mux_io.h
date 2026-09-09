/*
 * File synopsis:
 * Declares the H1 experiment-only RFB I/O adapter used to route the clean
 * through-Issue-39 RFB session through H1's single PSTV mux connection.
 *
 * The clean RFB parser keeps its historical int socket handle API. In the H1
 * cumulative build that handle identifies the already-owned physical PSTV
 * socket while this adapter resolves operations to the explicitly bound H1
 * runtime and logical channel 1. Binding is session-scoped and does not create
 * another socket or another physical receiver.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_MUX_IO_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_MUX_IO_H

#include <stddef.h>

struct pstvnc_h1_transport_runtime;
typedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;

int pstvnc_h1_rfb_mux_io_bind(
    pstvnc_h1_transport_runtime_t *runtime);

void pstvnc_h1_rfb_mux_io_unbind(
    pstvnc_h1_transport_runtime_t *runtime);

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
