/*
 * File synopsis:
 * Deterministic host tests for the pure production CONFIG/profile decoder.
 * Exercises complete decode, exact A001 Transport provenance, wire rejection,
 * owner-level Transport/PCM validation, activation contradictions, atomic
 * failure behavior, and signed common-clock offset preservation.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config/profile.h"

#define TEST_PAYLOAD_BYTES \
    (PSTVNC_CONFIG_PROFILE_HEADER_BYTES + \
     PSTVNC_CONFIG_PROFILE_FIELD_COUNT * PSTVNC_CONFIG_PROFILE_ENTRY_BYTES)

static int failures;

#define CHECK(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
            failures++; \
        } \
    } while (0)

static void write_be32(uint8_t output[4], uint32_t value)
{
    output[0] = (uint8_t)(value >> 24);
    output[1] = (uint8_t)(value >> 16);
    output[2] = (uint8_t)(value >> 8);
    output[3] = (uint8_t)value;
}

static uint32_t signed_raw(int32_t value)
{
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    return raw;
}

static size_t field_offset(uint32_t field_id)
{
    return PSTVNC_CONFIG_PROFILE_HEADER_BYTES +
        (size_t)(field_id - 1u) * PSTVNC_CONFIG_PROFILE_ENTRY_BYTES;
}

static void set_field(uint8_t *payload, uint32_t field_id, uint32_t value)
{
    size_t offset = field_offset(field_id);
    write_be32(&payload[offset], field_id);
    write_be32(&payload[offset + 4u], value);
}

static void make_valid_profile(uint8_t payload[TEST_PAYLOAD_BYTES])
{
    uint32_t field_id;

    memset(payload, 0, TEST_PAYLOAD_BYTES);
    write_be32(&payload[0], PSTVNC_CONFIG_PROFILE_VERSION);
    write_be32(&payload[4], PSTVNC_CONFIG_PROFILE_FIELD_COUNT);

    for (field_id = 1u; field_id <= PSTVNC_CONFIG_PROFILE_FIELD_COUNT; field_id++)
        set_field(payload, field_id, 0u);

    set_field(payload, PSTVNC_CONFIG_FIELD_SESSION_ID, 0x12345678u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_MODE, PSTVNC_CONFIG_RFB_ON);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_MODE, PSTVNC_CONFIG_AUDIO_PCM);
    set_field(payload, PSTVNC_CONFIG_FIELD_VIDEO_MODE, PSTVNC_CONFIG_VIDEO_ACTIVE);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_RATE, 48000u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS, 2u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_BITS, 16u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_VOLUME, 75u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US,
        signed_raw(-12500));
    set_field(payload, PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US,
        signed_raw(33000));
    set_field(payload, PSTVNC_CONFIG_FIELD_MEDIA_EPOCH_LEAD_US, 50000u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY, 16384u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES, 8192u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES, 2048u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY, 1u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_RETURN_ENABLED, 1u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE, 4096u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY, 64u);
    set_field(payload, PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD, 4096u);
}

static void test_complete_valid_profile(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;

    make_valid_profile(payload);
    memset(&profile, 0, sizeof(profile));

    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 1);
    CHECK(profile.composition.session_id == 0x12345678u);
    CHECK(profile.composition.rfb_mode == PSTVNC_CONFIG_RFB_ON);
    CHECK(profile.composition.audio_mode == PSTVNC_CONFIG_AUDIO_PCM);
    CHECK(profile.composition.video_mode == PSTVNC_CONFIG_VIDEO_ACTIVE);
    CHECK(profile.pcm.rate_hz == 48000u);
    CHECK(profile.pcm.channels == 2u);
    CHECK(profile.pcm.bits_per_sample == 16u);
    CHECK(profile.pcm.volume_percent == 75u);
    CHECK(profile.media_clock.audio_presentation_offset_us == -12500);
    CHECK(profile.media_clock.video_presentation_offset_us == 33000);
    CHECK(profile.media_clock.epoch_lead_us == 50000u);
    CHECK(profile.transport.rfb_queue_capacity == 16384u);
    CHECK(profile.transport.rfb_initial_credit_bytes == 8192u);
    CHECK(profile.transport.rfb_credit_batch_bytes == 2048u);
    CHECK(profile.transport.rfb_credit_flush_on_empty == 1);
    CHECK(profile.transport.rfb_credit_return_enabled == 1);
    CHECK(profile.transport.receiver_thread_stack_size == 4096u);
    CHECK(profile.transport.receiver_thread_priority == 64);
    CHECK(profile.transport.max_data_payload == 4096u);
}

static void test_wrong_version_and_malformed_length(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;

    make_valid_profile(payload);
    write_be32(&payload[0], PSTVNC_CONFIG_PROFILE_VERSION + 1u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload) - 1u) == 0);

    write_be32(&payload[4], PSTVNC_CONFIG_PROFILE_FIELD_COUNT - 1u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);
}

static void test_unknown_duplicate_and_missing_fields(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;
    size_t last = field_offset(PSTVNC_CONFIG_PROFILE_FIELD_COUNT);

    make_valid_profile(payload);
    write_be32(&payload[last], PSTVNC_CONFIG_PROFILE_FIELD_COUNT + 1u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    write_be32(&payload[last], PSTVNC_CONFIG_FIELD_SESSION_ID);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    write_be32(&payload[4], PSTVNC_CONFIG_PROFILE_FIELD_COUNT - 1u);
    CHECK(pstvnc_config_profile_decode(
        &profile, payload,
        sizeof(payload) - PSTVNC_CONFIG_PROFILE_ENTRY_BYTES) == 0);
}

static void test_transport_relationship_rejection(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY, 1024u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES, 20000u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES, 0u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE, 4100u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY, 128u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD, 8193u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);
}

static void test_no_default_transport_and_rfb_off_inert_policy(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;
    size_t last = field_offset(PSTVNC_CONFIG_PROFILE_FIELD_COUNT);

    make_valid_profile(payload);
    write_be32(&payload[last], PSTVNC_CONFIG_FIELD_SESSION_ID);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_MODE, PSTVNC_CONFIG_RFB_OFF);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_RFB_CREDIT_RETURN_ENABLED, 0u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 1);
    CHECK(profile.transport.receiver_thread_stack_size == 4096u);
    CHECK(profile.transport.receiver_thread_priority == 64);
    CHECK(profile.transport.max_data_payload == 4096u);
}

static void test_pcm_format_volume_and_activation_rejection(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS, 3u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_BITS, 24u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_RATE, 0u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_VOLUME, 101u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_MODE, PSTVNC_CONFIG_AUDIO_OFF);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_RATE, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_BITS, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_VOLUME, 0u);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US, 0u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 1);
}

static void test_video_activation_and_signed_offsets(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_VIDEO_MODE, PSTVNC_CONFIG_VIDEO_OFF);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);

    set_field(payload, PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US, 0u);
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 1);

    make_valid_profile(payload);
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US,
        signed_raw(INT32_MIN));
    set_field(payload, PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US,
        signed_raw(INT32_MAX));
    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 1);
    CHECK(profile.media_clock.audio_presentation_offset_us == INT32_MIN);
    CHECK(profile.media_clock.video_presentation_offset_us == INT32_MAX);
}

static void test_failure_does_not_publish_partial_profile(void)
{
    uint8_t payload[TEST_PAYLOAD_BYTES];
    pstvnc_config_session_profile_t profile;
    pstvnc_config_session_profile_t before;

    make_valid_profile(payload);
    memset(&profile, 0xa5, sizeof(profile));
    before = profile;
    set_field(payload, PSTVNC_CONFIG_FIELD_AUDIO_VOLUME, 101u);

    CHECK(pstvnc_config_profile_decode(&profile, payload, sizeof(payload)) == 0);
    CHECK(memcmp(&profile, &before, sizeof(profile)) == 0);
}

int main(void)
{
    test_complete_valid_profile();
    test_wrong_version_and_malformed_length();
    test_unknown_duplicate_and_missing_fields();
    test_transport_relationship_rejection();
    test_no_default_transport_and_rfb_off_inert_policy();
    test_pcm_format_volume_and_activation_rejection();
    test_video_activation_and_signed_offsets();
    test_failure_does_not_publish_partial_profile();

    if (failures != 0) {
        fprintf(stderr, "config_profile_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("config_profile_test: PASS");
    return 0;
}
