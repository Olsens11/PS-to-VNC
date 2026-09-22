/*
 * File synopsis:
 * Defines Transport's single cross-component bridge. The bridge exposes the
 * product Wire establishment/availability, application-requested rider-runtime
 * lifecycle, logical RFB byte-stream/quiesce/provider-terminal processes,
 * optional logical AUDIO/MPEG2 consumer seams, and exact MPEG generation-
 * control relay operations while keeping the physical PSTV descriptor, Pi
 * session identity, and sole physical-I/O runtime private to Transport.
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

#include "protocol.h"
#include "transport.h"

#include <stddef.h>
#include <stdint.h>

typedef enum pstvnc_transport_wire_establishment_status {
    PSTVNC_TRANSPORT_WIRE_ESTABLISHMENT_FAILED = 0,
    PSTVNC_TRANSPORT_WIRE_ESTABLISHED = 1,
    PSTVNC_TRANSPORT_WIRE_NOT_ACCEPTED = 2
} pstvnc_transport_wire_establishment_status_t;

typedef struct pstvnc_transport_wire_establishment_result {
    pstvnc_transport_wire_establishment_status_t status;
    pstvnc_wire_not_accepted_reason_t rejection_reason;
} pstvnc_transport_wire_establishment_result_t;

typedef enum pstvnc_transport_wire_availability {
    PSTVNC_TRANSPORT_WIRE_INACTIVE = 0,
    PSTVNC_TRANSPORT_WIRE_ACTIVE = 1
} pstvnc_transport_wire_availability_t;

/*
 * Transfer a fresh caller-owned TCP descriptor into Transport and perform Q4.
 * If physical adoption succeeds, *socket_fd is set to -1 on every later
 * outcome. The Pi-assigned session ID remains private to Transport.
 */
pstvnc_transport_wire_establishment_result_t pstvnc_transport_wire_establish(
    int *socket_fd);

pstvnc_transport_wire_availability_t pstvnc_transport_wire_availability(void);

/*
 * Rider-runtime opens retain the legacy descriptor pointer only as a migration
 * seam. If Wire is INACTIVE they first perform Q4 on that descriptor; if Wire
 * is already ACTIVE the pointer must be NULL or contain -1. No rider runtime
 * can therefore skip establishment.
 */
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

pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *transport_access);

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count);
pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(
    const pstvnc_transport_access_t *transport_access);
pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count);
/*
 * Query the first typed provider-terminal fact for this exact access ticket.
 * OK means reason is CONNECT/READ/WRITE. WOULD_BLOCK means the RFB provider is
 * still nonterminal while the Wire runtime is live. FAILED/CLOSED/STOPPED are
 * genuine Transport/Wire outcomes when no provider cause was latched first.
 */
pstvnc_transport_result_t pstvnc_transport_rfb_provider_failure(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_rfb_provider_failure_reason_t *reason);

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_audio_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done);
pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence);
pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence);

pstvnc_transport_result_t pstvnc_transport_mpeg_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_mpeg_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done);
pstvnc_transport_result_t pstvnc_transport_mpeg_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence);
pstvnc_transport_result_t pstvnc_transport_mpeg_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence);
/*
 * Publish real finite MPEG producer completion after a higher owner has proven
 * the ordered producer fence. This is not decoder cancellation and does not
 * invent a channel-4 wire marker.
 */
pstvnc_transport_result_t pstvnc_transport_mpeg_mark_producer_done(
    const pstvnc_transport_access_t *transport_access);

pstvnc_transport_result_t pstvnc_transport_mpeg_send_start(
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_mpeg_start_payload_t *start);
pstvnc_transport_result_t pstvnc_transport_mpeg_send_retire(
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_mpeg_retire_payload_t *retire);
pstvnc_transport_result_t pstvnc_transport_mpeg_take_retire_completion(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_mpeg_retire_payload_t *completion);

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(
    const pstvnc_transport_access_t *transport_access);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(
    const pstvnc_transport_access_t *transport_access);
pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(
    const pstvnc_transport_access_t *transport_access);
pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t *residual_count);
pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t expected_count,
    size_t *discarded_count);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(
    const pstvnc_transport_access_t *transport_access);

#endif
