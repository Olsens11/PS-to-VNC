/*
 * File synopsis:
 * Proves R36's exact selected AUDIO runtime profile, owner separation,
 * fail-closed validation, distinct poll fields, and immutable-by-copy access.
 */

#include "config/audio_runtime_profile.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "CHECK failed line %d: %s\n", __LINE__, #x); failures++; } } while (0)

static void test_exact_selected_values(void)
{
    pstvnc_config_audio_runtime_profile_t profile;

    memset(&profile, 0, sizeof(profile));
    CHECK(pstvnc_config_audio_runtime_profile_selected(&profile));
    CHECK(pstvnc_config_audio_runtime_profile_valid(&profile));

    CHECK(profile.transport.queue_capacity == 524288u);
    CHECK(profile.transport.initial_credit_bytes == 524288u);
    CHECK(profile.transport.credit_batch_bytes == 4096u);
    CHECK(profile.transport.credit_flush_on_empty == 1);
    CHECK(profile.transport.credit_return_enabled == 1);

    CHECK(profile.pcm.rate_hz == 48000u);
    CHECK(profile.pcm.channels == 2u);
    CHECK(profile.pcm.bits_per_sample == 16u);
    CHECK(profile.pcm.volume_percent == 100u);

    CHECK(profile.session.playback_buffer_capacity == 4096u);
    CHECK(profile.session.startup_reservoir_bytes == 458752u);
    CHECK(profile.session.worker_priority == 65);
    CHECK(profile.session.worker_stack_bytes == 16384u);
    CHECK(profile.session.reservoir_poll_us == 1000u);
    CHECK(profile.session.clock_poll_us == 1000u);
    CHECK(&profile.session.reservoir_poll_us != &profile.session.clock_poll_us);
}

static void test_copy_cannot_mutate_selected_authority(void)
{
    pstvnc_config_audio_runtime_profile_t first;
    pstvnc_config_audio_runtime_profile_t second;

    CHECK(pstvnc_config_audio_runtime_profile_selected(&first));
    first.transport.queue_capacity = 1u;
    first.pcm.rate_hz = 1u;
    first.session.clock_poll_us = 7u;

    CHECK(pstvnc_config_audio_runtime_profile_selected(&second));
    CHECK(second.transport.queue_capacity == 524288u);
    CHECK(second.pcm.rate_hz == 48000u);
    CHECK(second.session.clock_poll_us == 1000u);
}

static void test_invalid_profiles_fail_closed(void)
{
    pstvnc_config_audio_runtime_profile_t p;

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.transport.queue_capacity = 0u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.transport.initial_credit_bytes = p.transport.queue_capacity + 1u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.transport.credit_batch_bytes = 0u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.transport.credit_return_enabled = 0;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.pcm.channels = 3u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.pcm.bits_per_sample = 24u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.pcm.volume_percent = 101u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.playback_buffer_capacity = 0u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.startup_reservoir_bytes = p.transport.queue_capacity + 1u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.worker_priority = 0;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.worker_stack_bytes = (size_t)INT_MAX + 1u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.reservoir_poll_us = 0u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));

    CHECK(pstvnc_config_audio_runtime_profile_selected(&p));
    p.session.clock_poll_us = 0u;
    CHECK(!pstvnc_config_audio_runtime_profile_valid(&p));
}

int main(void)
{
    test_exact_selected_values();
    test_copy_cannot_mutate_selected_authority();
    test_invalid_profiles_fail_closed();

    if (failures != 0) {
        fprintf(stderr, "CONFIG_AUDIO_RUNTIME_PROFILE_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS");
    return 0;
}
