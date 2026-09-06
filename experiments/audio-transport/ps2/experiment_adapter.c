/*
 * File synopsis:
 * Attaches the mux experiment to existing clean PS2VNC through linker-wrapped
 * platform/RFB seams without modifying product source.
 */

#include "audio_runtime.h"
#include "transport_runtime.h"

#include <stddef.h>

static pstvnc_transport_runtime_t experiment_transport;
static pstvnc_audio_exp_runtime_t experiment_audio;
static int experiment_active;

int __wrap_pstvnc_ps2_network_connect_vnc(void)
{
    int socket_fd;

    if (experiment_active)
        return -1;

    if (pstvnc_transport_runtime_start(
            &experiment_transport) < 0)
        return -1;

    socket_fd =
        pstvnc_transport_runtime_socket_fd(
            &experiment_transport);

    if (socket_fd < 0)
        goto fail_transport;

    if (pstvnc_audio_exp_runtime_start(
            &experiment_audio,
            &experiment_transport) < 0)
        goto fail_transport;

    experiment_active = 1;
    return socket_fd;

fail_transport:
    (void)pstvnc_transport_runtime_shutdown(
        &experiment_transport);

    return -1;
}

void __wrap_pstvnc_ps2_network_close(int socket_fd)
{
    (void)socket_fd;

    if (!experiment_active)
        return;

    (void)pstvnc_audio_exp_runtime_shutdown(
        &experiment_audio);

    (void)pstvnc_transport_runtime_shutdown(
        &experiment_transport);

    experiment_active = 0;
}

int __wrap_pstvnc_rfb_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    if (!experiment_active ||
        socket_fd !=
            pstvnc_transport_runtime_socket_fd(
                &experiment_transport))
        return -1;

    return pstvnc_transport_runtime_rfb_read_exact(
        &experiment_transport,
        buffer,
        count);
}

int __wrap_pstvnc_rfb_io_poll_receive(
    int socket_fd)
{
    if (!experiment_active ||
        socket_fd !=
            pstvnc_transport_runtime_socket_fd(
                &experiment_transport))
        return -1;

    return pstvnc_transport_runtime_rfb_poll_receive(
        &experiment_transport);
}

int __wrap_pstvnc_rfb_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    if (!experiment_active ||
        socket_fd !=
            pstvnc_transport_runtime_socket_fd(
                &experiment_transport))
        return -1;

    return pstvnc_transport_runtime_rfb_write_exact(
        &experiment_transport,
        buffer,
        count);
}
