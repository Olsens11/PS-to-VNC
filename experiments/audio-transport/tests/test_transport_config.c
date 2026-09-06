/*
 * File synopsis:
 * Host contract tests for configurable Audio Transport EXP2's complete profile.
 *
 * Includes a deliberately large queue profile to prove validation contains no
 * arbitrary RFB/audio capacity ceiling.
 */

#include "transport_config.h"
#include "transport_protocol.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ENTRY_COUNT PSTVNC_TRANSPORT_CONFIG_FIELD_COUNT
#define PAYLOAD_BYTES (8u + (ENTRY_COUNT * 8u))

static void put_entry(
    uint8_t *payload,
    size_t index,
    uint32_t field_id,
    uint32_t value)
{
    size_t offset =
        8u + (index * 8u);

    pstvnc_transport_write_be32(
        &payload[offset],
        field_id);

    pstvnc_transport_write_be32(
        &payload[offset + 4u],
        value);
}

static void build_profile(
    uint8_t payload[PAYLOAD_BYTES],
    uint32_t profile_id,
    uint32_t rfb_queue,
    uint32_t audio_queue)
{
    memset(
        payload,
        0,
        PAYLOAD_BYTES);

    pstvnc_transport_write_be32(
        &payload[0],
        PSTVNC_TRANSPORT_CONFIG_VERSION);

    pstvnc_transport_write_be32(
        &payload[4],
        profile_id);

    put_entry(payload, 0, 1, rfb_queue);
    put_entry(payload, 1, 2, audio_queue);
    put_entry(payload, 2, 3, 4096u);
    put_entry(payload, 3, 4, 4096u);
    put_entry(payload, 4, 5, 1u);
    put_entry(payload, 5, 6, 1u);
    put_entry(payload, 6, 7, 2u);
    put_entry(payload, 7, 8, audio_queue);
    put_entry(payload, 8, 9, 3000000u);
    put_entry(payload, 9, 10, 4096u);
    put_entry(payload, 10, 11, 1000u);
    put_entry(payload, 11, 12, 1000u);
    put_entry(payload, 12, 13, 65u);
    put_entry(payload, 13, 14, 16384u);
    put_entry(payload, 14, 15, 63u);
    put_entry(payload, 15, 16, 16384u);
    put_entry(payload, 16, 17, 48000u);
    put_entry(payload, 17, 18, 2u);
    put_entry(payload, 18, 19, 16u);
    put_entry(payload, 19, 20, 100u);
    put_entry(payload, 20, 21, 8192u);
    put_entry(payload, 21, 22, 0u);
    put_entry(payload, 22, 23, 0u);
    put_entry(payload, 23, 24, 0u);
}

int main(void)
{
    uint8_t payload[PAYLOAD_BYTES];
    uint8_t duplicate[PAYLOAD_BYTES];
    uint8_t large[PAYLOAD_BYTES];

    pstvnc_transport_config_t config;

    build_profile(
        payload,
        0u,
        32768u,
        524288u);

    if (!pstvnc_transport_config_decode(
            &config,
            payload,
            sizeof(payload))) {

        fprintf(
            stderr,
            "profile0 decode failed\n");

        return 1;
    }

    if (!pstvnc_transport_config_validate(
            &config)) {

        fprintf(
            stderr,
            "profile0 validation failed\n");

        return 1;
    }

    if (config.profile_id != 0u ||
        config.rfb_queue_capacity != 32768u ||
        config.audio_queue_capacity != 524288u ||
        config.audio_start_delay_us != 3000000u ||
        config.audio_chunk_bytes != 4096u ||
        config.audio_thread_priority != 65u ||
        config.receiver_thread_priority != 63u) {

        fprintf(
            stderr,
            "profile0 values mismatch\n");

        return 1;
    }

    if (pstvnc_transport_config_digest(
            payload,
            sizeof(payload)) == 0u) {

        fprintf(
            stderr,
            "digest unexpectedly zero\n");

        return 1;
    }

    memcpy(
        duplicate,
        payload,
        sizeof(payload));

    /*
     * Make the second entry repeat field id 1.
     */
    pstvnc_transport_write_be32(
        &duplicate[16],
        1u);

    if (pstvnc_transport_config_decode(
            &config,
            duplicate,
            sizeof(duplicate))) {

        fprintf(
            stderr,
            "duplicate field accepted\n");

        return 1;
    }

    /*
     * Validation must not contain the arbitrary 256 KiB/1 MiB/etc ceilings
     * discussed during design. Actual PS2 malloc() success is the authority.
     *
     * This host test does not allocate these queues; it proves only that the
     * profile vocabulary does not reject them in advance.
     */
    build_profile(
        large,
        999u,
        24u * 1024u * 1024u,
        4u * 1024u * 1024u);

    if (!pstvnc_transport_config_decode(
            &config,
            large,
            sizeof(large)) ||
        !pstvnc_transport_config_validate(
            &config)) {

        fprintf(
            stderr,
            "artificial queue ceiling detected\n");

        return 1;
    }

    puts(
        "TRANSPORT_CONFIG_TEST=PASS");

    puts(
        "NO_ARTIFICIAL_QUEUE_CAPACITY_CEILING=PASS");

    return 0;
}
