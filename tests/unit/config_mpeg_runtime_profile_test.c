/*
 * File synopsis:
 * Deterministic host proof for Configuration's selected MPEG runtime profile.
 * Verifies every R7-selected owner value, compile-time reuse of the existing
 * narrow owner types, immutable/copy-safe publication, and preservation of the
 * existing 19-field production CONFIG wire schema.
 *
 * No mechanism is initialized or started by this fixture.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config/mpeg_runtime_profile.h"
#include "config/profile.h"

static int failures;

#define CHECK(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
            failures++; \
        } \
    } while (0)

static void test_exact_selected_values_and_owner_types(void)
{
    const pstvnc_config_mpeg_runtime_profile_t *selected =
        pstvnc_config_mpeg_runtime_profile_selected();
    pstvnc_transport_mpeg_channel_config_t transport;
    pstvnc_mpeg_decoder_config_t decoder;
    pstvnc_mpeg_worker_values_t worker;
    pstvnc_mpeg_ps2_worker_runtime_values_t ps2_worker_runtime;
    pstvnc_mpeg_scheduler_profile_t scheduler;

    CHECK(selected != NULL);
    if (selected == NULL)
        return;

    /*
     * These assignments are deliberately typed: the aggregate reuses the
     * mechanism owners' public values instead of parallel Config copies.
     */
    transport = selected->transport;
    decoder = selected->decoder;
    worker = selected->worker;
    ps2_worker_runtime = selected->ps2_worker_runtime;
    scheduler = selected->scheduler;

    CHECK(transport.queue_capacity == 524288u);
    CHECK(transport.initial_credit_bytes == 524288u);
    CHECK(transport.credit_batch_bytes == 8192u);
    CHECK(transport.credit_flush_on_empty == 1);
    CHECK(transport.credit_return_enabled == 1);

    CHECK(decoder.max_width == 704u);
    CHECK(decoder.max_height == 480u);
    CHECK(decoder.bytes_per_pixel == 2u);
    CHECK(decoder.feed_payload_capacity == 2048u);
    CHECK(decoder.transfer_alignment == 16u);
    CHECK(decoder.buffer_alignment == 64u);

    CHECK(worker.worker_stack_bytes == 65536u);
    CHECK(worker.worker_priority == 67);

    CHECK(ps2_worker_runtime.join_poll_delay_us == 1000u);
    CHECK(ps2_worker_runtime.join_poll_max_count == 3000u);

    CHECK(scheduler.fps_numerator == 30000u);
    CHECK(scheduler.fps_denominator == 1001u);
    CHECK(scheduler.drop_enabled == 0);
    CHECK(scheduler.drop_threshold_milliframes == 0u);
}

static void test_selected_profile_is_copy_safe(void)
{
    const pstvnc_config_mpeg_runtime_profile_t *selected =
        pstvnc_config_mpeg_runtime_profile_selected();
    pstvnc_config_mpeg_runtime_profile_t copy;
    pstvnc_config_mpeg_runtime_profile_t before;

    CHECK(selected != NULL);
    if (selected == NULL)
        return;

    before = *selected;
    copy = *selected;

    copy.transport.queue_capacity = 1u;
    copy.decoder.max_width = 1u;
    copy.worker.worker_stack_bytes = 1u;
    copy.ps2_worker_runtime.join_poll_max_count = 1u;
    copy.scheduler.fps_numerator = 1u;

    CHECK(copy.transport.queue_capacity == 1u);
    CHECK(copy.decoder.max_width == 1u);
    CHECK(copy.worker.worker_stack_bytes == 1u);
    CHECK(copy.ps2_worker_runtime.join_poll_max_count == 1u);
    CHECK(copy.scheduler.fps_numerator == 1u);
    CHECK(memcmp(selected, &before, sizeof(before)) == 0);
    CHECK(pstvnc_config_mpeg_runtime_profile_selected() == selected);
}

static void test_config_wire_schema_is_unchanged(void)
{
    CHECK(PSTVNC_CONFIG_PROFILE_VERSION == 1u);
    CHECK(PSTVNC_CONFIG_PROFILE_HEADER_BYTES == 8u);
    CHECK(PSTVNC_CONFIG_PROFILE_ENTRY_BYTES == 8u);
    CHECK(PSTVNC_CONFIG_PROFILE_FIELD_COUNT == 19u);
    CHECK(PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD == 19);
}

int main(void)
{
    test_exact_selected_values_and_owner_types();
    test_selected_profile_is_copy_safe();
    test_config_wire_schema_is_unchanged();

    if (failures != 0) {
        fprintf(stderr,
            "config_mpeg_runtime_profile_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("config_mpeg_runtime_profile_test: PASS");
    return 0;
}
