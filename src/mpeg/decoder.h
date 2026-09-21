/*
 * File synopsis:
 * Defines A003's clean synchronous MPEG decoder owner. The owner allocates and
 * bounds decoder-visible feed/picture resources, establishes one explicit
 * decoder/platform ownership interval, consumes MPEG only through Transport's
 * public logical channel, exposes one completed-picture step with a borrowed
 * read-only frame value, and observes local stop only at decoder-call
 * boundaries.
 *
 * This interface does not own a worker thread, exact producer generations,
 * application orchestration, GS/compositor presentation, common-clock arming,
 * scheduler/drop/calibration policy, or hardware qualification. Platform and
 * memory operations are injected so ownership/order remain host-testable without
 * manufacturing PS2 tuning defaults.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_MPEG_DECODER_H
#define PSTVNC_MPEG_DECODER_H

#include "transport/transport.h"

#include <stddef.h>
#include <stdint.h>

typedef int (*pstvnc_mpeg_feed_callback_t)(void *context);
typedef void *(*pstvnc_mpeg_sequence_callback_t)(
    void *context,
    uint32_t width,
    uint32_t height,
    size_t *picture_capacity);

typedef struct pstvnc_mpeg_decoder_config {
    uint32_t max_width;
    uint32_t max_height;
    uint32_t bytes_per_pixel;
    size_t feed_payload_capacity;
    size_t transfer_alignment;
    size_t buffer_alignment;
} pstvnc_mpeg_decoder_config_t;

typedef struct pstvnc_mpeg_decoder_memory_ops {
    void *(*allocate)(void *context, size_t byte_count, size_t alignment);
    void (*release)(void *context, void *pointer);
    void *context;
} pstvnc_mpeg_decoder_memory_ops_t;

typedef struct pstvnc_mpeg_decoder_sync_ops {
    int (*lock)(void *context);
    int (*unlock)(void *context);
    void *context;
} pstvnc_mpeg_decoder_sync_ops_t;

typedef struct pstvnc_mpeg_decoder_platform_ops {
    int (*prepare_known_state)(void *context);
    int (*initialize)(
        void *context,
        pstvnc_mpeg_feed_callback_t feed_callback,
        void *feed_context,
        pstvnc_mpeg_sequence_callback_t sequence_callback,
        void *sequence_context);
    int (*picture)(void *context);
    int (*submit_feed)(
        void *context,
        const uint8_t *bytes,
        size_t payload_bytes,
        size_t transfer_bytes);
    int (*destroy)(void *context);
    int (*release_known_state)(void *context);
    void *context;
} pstvnc_mpeg_decoder_platform_ops_t;

typedef enum pstvnc_mpeg_decoder_result {
    PSTVNC_MPEG_DECODER_COMPLETE = 0,
    PSTVNC_MPEG_DECODER_STOPPED = 1,
    PSTVNC_MPEG_DECODER_PICTURE_READY = 2,
    PSTVNC_MPEG_DECODER_INVALID = -1,
    PSTVNC_MPEG_DECODER_ALLOCATION_FAILED = -2,
    PSTVNC_MPEG_DECODER_PREPARE_FAILED = -3,
    PSTVNC_MPEG_DECODER_INITIALIZE_FAILED = -4,
    PSTVNC_MPEG_DECODER_SEQUENCE_INVALID = -5,
    PSTVNC_MPEG_DECODER_TRANSPORT_FAILED = -6,
    PSTVNC_MPEG_DECODER_TRANSFER_FAILED = -7,
    PSTVNC_MPEG_DECODER_PICTURE_FAILED = -8,
    PSTVNC_MPEG_DECODER_UNEXPECTED_END = -9,
    PSTVNC_MPEG_DECODER_SYNC_FAILED = -10,
    PSTVNC_MPEG_DECODER_CALL_ACTIVE = -11,
    PSTVNC_MPEG_DECODER_DESTROY_FAILED = -12,
    PSTVNC_MPEG_DECODER_STATE_RELEASE_FAILED = -13,
    PSTVNC_MPEG_DECODER_ACCOUNTING_FAILED = -14
} pstvnc_mpeg_decoder_result_t;

/*
 * Borrowed view of exactly one successfully decoded picture.
 *
 * pixels remains decoder-owned. This value is valid only until the next call to
 * pstvnc_mpeg_decoder_step() on the same decoder or until decoder release,
 * whichever occurs first. A caller that needs the picture beyond that boundary
 * must copy/consume it before allowing the decoder to advance.
 *
 * byte_count is the usable macroblock-backed byte extent for the validated
 * sequence. capacity_bytes is the decoder-owned allocation capacity. No MPEG
 * generation or Presentation authority is carried here.
 */
typedef struct pstvnc_mpeg_decoded_picture {
    const void *pixels;
    size_t byte_count;
    size_t capacity_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
    uint32_t picture_ordinal;
} pstvnc_mpeg_decoded_picture_t;

typedef struct pstvnc_mpeg_decoder_report {
    uint32_t sequence_width;
    uint32_t sequence_height;
    uint32_t pictures_decoded;
    uint32_t feed_callbacks;
    uint64_t payload_bytes_consumed;
    uint64_t transfer_bytes_submitted;
    pstvnc_transport_result_t transport_result;
} pstvnc_mpeg_decoder_report_t;

typedef struct pstvnc_mpeg_decoder {
    pstvnc_transport_access_t transport_access;
    pstvnc_mpeg_decoder_config_t config;
    pstvnc_mpeg_decoder_memory_ops_t memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t sync_ops;
    pstvnc_mpeg_decoder_platform_ops_t platform_ops;

    uint8_t *feed_buffer;
    void *picture_buffer;
    size_t feed_transfer_capacity;
    size_t picture_capacity;
    size_t sequence_picture_bytes;

    int initialized;
    int known_state_prepared;
    int decoder_initialized;
    volatile int stop_requested;
    volatile int decoder_call_active;

    int sequence_seen;
    int sequence_invalid;
    int feed_exhausted;
    int feed_failed;
    pstvnc_transport_result_t feed_transport_result;
    pstvnc_mpeg_decoder_report_t report;
} pstvnc_mpeg_decoder_t;

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_initialize(
    pstvnc_mpeg_decoder_t *decoder,
    const pstvnc_mpeg_decoder_config_t *config,
    const pstvnc_mpeg_decoder_memory_ops_t *memory_ops,
    const pstvnc_mpeg_decoder_sync_ops_t *sync_ops,
    const pstvnc_mpeg_decoder_platform_ops_t *platform_ops);

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_request_stop(
    pstvnc_mpeg_decoder_t *decoder);

/*
 * Execute at most one platform picture call.
 *
 * PICTURE_READY publishes exactly one borrowed picture and returns before any
 * later picture call can begin. COMPLETE/STOPPED/error results publish no
 * picture. A stop already visible before the step prevents the picture call.
 * A stop raised while picture() owns the call is observed only after picture()
 * returns and returns STOPPED without publishing that just-completed picture.
 */
pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_step(
    pstvnc_mpeg_decoder_t *decoder,
    pstvnc_mpeg_decoded_picture_t *picture,
    pstvnc_mpeg_decoder_report_t *report);

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_run(
    pstvnc_mpeg_decoder_t *decoder,
    pstvnc_mpeg_decoder_report_t *report);

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_release(
    pstvnc_mpeg_decoder_t *decoder);

#endif
