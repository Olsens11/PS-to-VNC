/*
 * File synopsis:
 * Provides the first fail-closed H1 RFB-mux I/O seam checkpoint.
 *
 * The clean RFB session is compiled against these names in the cumulative H1
 * experiment so it cannot accidentally fall through to the direct VNC socket
 * implementation while channel-1 support is still absent. Every operation
 * fails immediately. Later checkpoints replace these bodies with reads from an
 * H1-owned RFB logical queue and writes through H1's existing serialized PSTV
 * send path.
 *
 * No RFB runtime is started by this file. Current H1 CONFIG still rejects
 * rfb_mode=ON, so these functions are unreachable in qualified RFB-OFF media
 * sessions.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md.
 */

#include "h1_rfb_mux_io.h"

int pstvnc_h1_rfb_mux_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    (void)socket_fd;
    (void)buffer;
    (void)count;
    return -1;
}

int pstvnc_h1_rfb_mux_io_poll_receive(
    int socket_fd)
{
    (void)socket_fd;
    return -1;
}

int pstvnc_h1_rfb_mux_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    (void)socket_fd;
    (void)buffer;
    (void)count;
    return -1;
}
