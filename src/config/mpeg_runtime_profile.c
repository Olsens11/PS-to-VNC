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

static const pstvnc_config_mpeg_runtime_profile_t
    pstvnc_config_selected_mpeg_runtime_profile = {
        .transport = {
            .queue_capacity = 524288u,
            .initial_credit_bytes = 524288u,
            .credit_batch_bytes = 8192u,
            .credit_flush_on_empty = 1,
            .credit_return_enabled = 1
        },
        .decoder = {
            .max_width = 704u,
            .max_height = 480u,
            .bytes_per_pixel = 2u,
            .feed_payload_capacity = 2048u,
            .transfer_alignment = 16u,
            .buffer_alignment = 64u
        },
        .worker = {
            .worker_stack_bytes = 65536u,
            .worker_priority = 67
        },
        .ps2_worker_runtime = {
            .join_poll_delay_us = 1000u,
            .join_poll_max_count = 3000u
        },
        .scheduler = {
            .fps_numerator = 30000u,
            .fps_denominator = 1001u,
            .drop_enabled = 0,
            .drop_threshold_milliframes = 0u
        }
    };

const pstvnc_config_mpeg_runtime_profile_t *
pstvnc_config_mpeg_runtime_profile_selected(void)
{
    return &pstvnc_config_selected_mpeg_runtime_profile;
}
