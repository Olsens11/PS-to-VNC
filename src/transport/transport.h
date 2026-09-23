/*
 * File synopsis:
 * Defines stable cross-component Transport values used to establish one ledge
 * PSTV session. Physical socket mechanics, sole receive ownership, logical RFB
 * plus optional AUDIO/MPEG2 storage, and lifecycle implementation remain private
 * to Transport.
 *
 * The caller supplies already validated values. This header deliberately does
 * not manufacture CONFIG/profile defaults or expose the adopted physical
 * descriptor to protocol/media components.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md; docs/ledge/
 * LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_TRANSPORT_H
#define PSTVNC_TRANSPORT_H

#include <stdint.h>

typedef enum pstvnc_transport_result {
    PSTVNC_TRANSPORT_OK = 0,
    PSTVNC_TRANSPORT_WOULD_BLOCK = 1,
    PSTVNC_TRANSPORT_STOPPED = 2,
    PSTVNC_TRANSPORT_EXHAUSTED = 3,
    PSTVNC_TRANSPORT_CLOSED = -1,
    PSTVNC_TRANSPORT_INVALID = -2,
    PSTVNC_TRANSPORT_FAILED = -3
} pstvnc_transport_result_t;

/*
 * Opaque module-facing authority for cross-Wire Transport work.
 *
 * A module acquires this value during its ordinary startup and only passes it
 * back to Transport. Consumers must not inspect, compare, increment, or assign
 * semantic meaning to opaque_ticket. Transport binds the ticket to the Wire
 * Session active when access is acquired and permanently rejects it after that
 * session ends.
 */
typedef struct pstvnc_transport_access {
    uint32_t opaque_ticket;
} pstvnc_transport_access_t;

/*
 * Validated A001 session values crossing from product/config ownership into
 * Transport. These are the clean subset of H1 CONFIG v4 required by the
 * logical-RFB runtime: no consumer reconstructs them from unrelated globals.
 *
 * RFB enable/disable remains a higher session-composition decision. Creating
 * this A001 RFB session means the logical RFB channel is enabled; later media
 * composition may add AUDIO/MPEG resources explicitly.
 */
typedef struct pstvnc_transport_session_config {
    uint32_t rfb_queue_capacity;
    uint32_t rfb_initial_credit_bytes;
    uint32_t rfb_credit_batch_bytes;
    int rfb_credit_flush_on_empty;
    int rfb_credit_return_enabled;
    uint32_t receiver_thread_stack_size;
    int receiver_thread_priority;
    uint32_t max_data_payload;
} pstvnc_transport_session_config_t;

/*
 * Immutable A002 logical-AUDIO resource/flow-control authority. These values are
 * supplied explicitly by the caller/profile layer when AUDIO is enabled; the
 * Transport runtime has no queue/credit defaults and does not infer playback
 * chunk, reservoir, or AUDSRV policy from them.
 */
typedef struct pstvnc_transport_audio_channel_config {
    uint32_t queue_capacity;
    uint32_t initial_credit_bytes;
    uint32_t credit_batch_bytes;
    int credit_flush_on_empty;
    int credit_return_enabled;
} pstvnc_transport_audio_channel_config_t;

/*
 * Immutable A003 logical-MPEG2 resource/flow-control authority. These values
 * govern only bounded channel-4 storage and credit return. Feed sizing,
 * sequence geometry, DMA alignment, decoder resources, generation identity and
 * presentation policy remain outside Transport and have no hidden defaults here.
 */
typedef struct pstvnc_transport_mpeg_channel_config {
    uint32_t queue_capacity;
    uint32_t initial_credit_bytes;
    uint32_t credit_batch_bytes;
    int credit_flush_on_empty;
    int credit_return_enabled;
} pstvnc_transport_mpeg_channel_config_t;

/*
 * R20 owner-correct MPEG generation-control values.
 *
 * These values intentionally contain only run meaning owned above Transport.
 * Wire session identity and generation-control protocol version are private
 * Transport representation and are stamped below the bridge.
 */
typedef struct pstvnc_transport_mpeg_start_request {
    uint32_t generation;
    uint32_t base_x;
    uint32_t base_y;
    uint32_t base_width;
    uint32_t base_height;
    uint32_t suppression_x;
    uint32_t suppression_y;
    uint32_t suppression_width;
    uint32_t suppression_height;
} pstvnc_transport_mpeg_start_request_t;

typedef struct pstvnc_transport_mpeg_retire_request {
    uint32_t generation;
} pstvnc_transport_mpeg_retire_request_t;

typedef struct pstvnc_transport_mpeg_retire_completion {
    uint32_t generation;
} pstvnc_transport_mpeg_retire_completion_t;

#endif
