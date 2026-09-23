/*
 * File synopsis:
 * Publishes Configuration's one evidence-selected MPEG runtime profile using
 * only the existing narrow owner value types. The values are the initial clean
 * product selection recovered from qualified H1 runtime evidence; they are not
 * a user/network tuning surface and do not claim reconstructed-code hardware
 * qualification.
 *
 * This file contains values only. It performs no Transport open, decoder/worker
 * start, Presentation activation, scheduler initialization, START/RETIRE send,
 * Pi control, generation allocation, or Application-loop work.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7.
 */

#include "mpeg_runtime_profile.h"
#include "mpeg_runtime_profile_generated.h"

static const pstvnc_config_mpeg_runtime_profile_t
    pstvnc_config_selected_mpeg_runtime_profile = {
        .transport = {
            .queue_capacity = PSTVNC_CONFIG_MPEG_QUEUE_CAPACITY,
            .initial_credit_bytes = PSTVNC_CONFIG_MPEG_INITIAL_CREDIT_BYTES,
            .credit_batch_bytes = PSTVNC_CONFIG_MPEG_CREDIT_BATCH_BYTES,
            .credit_flush_on_empty = PSTVNC_CONFIG_MPEG_CREDIT_FLUSH_ON_EMPTY,
            .credit_return_enabled = PSTVNC_CONFIG_MPEG_CREDIT_RETURN_ENABLED
        },
        .decoder = {
            .max_width = PSTVNC_CONFIG_MPEG_MAX_WIDTH,
            .max_height = PSTVNC_CONFIG_MPEG_MAX_HEIGHT,
            .bytes_per_pixel = PSTVNC_CONFIG_MPEG_BYTES_PER_PIXEL,
            .feed_payload_capacity = PSTVNC_CONFIG_MPEG_FEED_PAYLOAD_CAPACITY,
            .transfer_alignment = PSTVNC_CONFIG_MPEG_TRANSFER_ALIGNMENT,
            .buffer_alignment = PSTVNC_CONFIG_MPEG_BUFFER_ALIGNMENT
        },
        .worker = {
            .worker_stack_bytes = PSTVNC_CONFIG_MPEG_WORKER_STACK_BYTES,
            .worker_priority = PSTVNC_CONFIG_MPEG_WORKER_PRIORITY
        },
        .ps2_worker_runtime = {
            .join_poll_delay_us = PSTVNC_CONFIG_MPEG_JOIN_POLL_DELAY_US,
            .join_poll_max_count = PSTVNC_CONFIG_MPEG_JOIN_POLL_MAX_COUNT
        },
        .scheduler = {
            .fps_numerator = PSTVNC_CONFIG_MPEG_FPS_NUMERATOR,
            .fps_denominator = PSTVNC_CONFIG_MPEG_FPS_DENOMINATOR,
            .drop_enabled = PSTVNC_CONFIG_MPEG_DROP_ENABLED,
            .drop_threshold_milliframes =
                PSTVNC_CONFIG_MPEG_DROP_THRESHOLD_MILLIFRAMES
        }
    };

const pstvnc_config_mpeg_runtime_profile_t *
pstvnc_config_mpeg_runtime_profile_selected(void)
{
    return &pstvnc_config_selected_mpeg_runtime_profile;
}
