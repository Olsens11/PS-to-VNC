/*
 * File synopsis:
 * Defines Transport's single cross-component bridge. The bridge exposes the
 * application-requested session lifecycle, logical RFB byte-stream/quiesce
 * processes, and optional logical AUDIO/MPEG2 consumer seams while keeping the
 * physical PSTV descriptor and sole receiver runtime private to Transport.
 *
 * The bridge does not parse RFB, invent configuration defaults, play PCM,
 * decode MPEG, decide media timing/presentation/generation policy, or perform
 * application recovery.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md; docs/ledge/
 * LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_TRANSPORT_BRIDGE_H
#define PSTVNC_TRANSPORT_BRIDGE_H

#include "transport.h"

#include <stddef.h>
#include <stdint.h>

pstvnc_transport_result_t pstvnc_transport_session_open(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config);

pstvnc_transport_result_t pstvnc_transport_session_open_with_audio(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config);

pstvnc_transport_result_t pstvnc_transport_session_open_with_mpeg(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);

pstvnc_transport_result_t pstvnc_transport_session_open_with_audio_mpeg(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);

pstvnc_transport_result_t pstvnc_transport_session_abort(void);
pstvnc_transport_result_t pstvnc_transport_session_wait_receiver_done(void);
pstvnc_transport_result_t pstvnc_transport_session_close(void);

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count);
pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void);
pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count);

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_audio_status(
    size_t *available_count,
    int *producer_done);
pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    uint32_t *activity_sequence);
pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    uint32_t *activity_sequence);

pstvnc_transport_result_t pstvnc_transport_mpeg_read_available(
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_mpeg_status(
    size_t *available_count,
    int *producer_done);
pstvnc_transport_result_t pstvnc_transport_mpeg_activity_snapshot(
    uint32_t *activity_sequence);
pstvnc_transport_result_t pstvnc_transport_mpeg_wait_activity(
    uint32_t *activity_sequence);
/*
 * Publish real finite MPEG producer completion after a higher owner has proven
 * the ordered producer fence. This is not decoder cancellation and does not
 * invent a channel-4 wire marker.
 */
pstvnc_transport_result_t pstvnc_transport_mpeg_mark_producer_done(void);

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void);
pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void);
pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    size_t *residual_count);
pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void);

#endif
