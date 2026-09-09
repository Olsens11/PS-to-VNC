/*
 * File synopsis:
 * Binds the unchanged clean RFB session I/O seam to H1 logical channel 1.
 *
 * No physical socket is opened here. The cumulative H1 transport remains the
 * sole owner of the one PSTV TCP connection and physical receive thread. The
 * historical RFB session's integer socket handle is used only as a fail-closed
 * identity check against the bound H1 runtime; all actual reads/writes are
 * delegated to logical channel-1 mechanics.
 */

#include "h1_rfb_mux_io.h"
#include "h1_rfb_transport_live.h"
#include "h1_transport_runtime.h"

static pstvnc_h1_transport_runtime_t *h1_rfb_bound_runtime;

static pstvnc_h1_transport_runtime_t *h1_rfb_resolve(int socket_fd)
{
    if (h1_rfb_bound_runtime == NULL ||
        socket_fd < 0 ||
        h1_rfb_bound_runtime->socket_fd != socket_fd ||
        !h1_rfb_bound_runtime->rfb_resources.active)
        return NULL;

    return h1_rfb_bound_runtime;
}

int pstvnc_h1_rfb_mux_io_bind(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        runtime->socket_fd < 0 ||
        !runtime->rfb_resources.active)
        return 0;

    if (h1_rfb_bound_runtime != NULL &&
        h1_rfb_bound_runtime != runtime)
        return 0;

    h1_rfb_bound_runtime = runtime;
    return 1;
}

void pstvnc_h1_rfb_mux_io_unbind(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (h1_rfb_bound_runtime == runtime)
        h1_rfb_bound_runtime = NULL;
}

int pstvnc_h1_rfb_mux_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);

    if (runtime == NULL)
        return -1;

    return pstvnc_h1_rfb_transport_read_exact(runtime, buffer, count);
}

int pstvnc_h1_rfb_mux_io_poll_receive(
    int socket_fd)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);

    if (runtime == NULL)
        return -1;

    return pstvnc_h1_rfb_transport_poll_receive(runtime);
}

int pstvnc_h1_rfb_mux_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    pstvnc_h1_transport_runtime_t *runtime = h1_rfb_resolve(socket_fd);

    if (runtime == NULL)
        return -1;

    return pstvnc_h1_rfb_transport_write_exact(runtime, buffer, count);
}
