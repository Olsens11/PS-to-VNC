/*
 * File synopsis:
 * Defines the clean, immutable session-profile values decoded from one bounded
 * PSTV CONFIG payload. Configuration owns session composition and value
 * validation; Transport, future PCM audio, and the future media clock consume
 * only their narrow subprofiles. No runtime negotiation or hardware lifecycle
 * is implemented here.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_CONFIG_PROFILE_H
#define PSTVNC_CONFIG_PROFILE_H

#include <stddef.h>
#include <stdint.h>

#include "transport/transport.h"

#define PSTVNC_CONFIG_PROFILE_VERSION 1u
#define PSTVNC_CONFIG_PROFILE_HEADER_BYTES 8u
#define PSTVNC_CONFIG_PROFILE_ENTRY_BYTES 8u
#define PSTVNC_CONFIG_PROFILE_FIELD_COUNT 19u

typedef enum pstvnc_config_profile_field {
    PSTVNC_CONFIG_FIELD_SESSION_ID = 1,
    PSTVNC_CONFIG_FIELD_RFB_MODE = 2,
    PSTVNC_CONFIG_FIELD_AUDIO_MODE = 3,
    PSTVNC_CONFIG_FIELD_VIDEO_MODE = 4,
    PSTVNC_CONFIG_FIELD_AUDIO_RATE = 5,
    PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS = 6,
    PSTVNC_CONFIG_FIELD_AUDIO_BITS = 7,
    PSTVNC_CONFIG_FIELD_AUDIO_VOLUME = 8,
    PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US = 9,
    PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US = 10,
    PSTVNC_CONFIG_FIELD_MEDIA_EPOCH_LEAD_US = 11,
    PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY = 12,
    PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES = 13,
    PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES = 14,
    PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY = 15,
    PSTVNC_CONFIG_FIELD_RFB_CREDIT_RETURN_ENABLED = 16,
    PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE = 17,
    PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY = 18,
    PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD = 19
} pstvnc_config_profile_field_t;

typedef enum pstvnc_config_audio_mode {
    PSTVNC_CONFIG_AUDIO_OFF = 0,
    PSTVNC_CONFIG_AUDIO_PCM = 1
} pstvnc_config_audio_mode_t;

typedef enum pstvnc_config_rfb_mode {
    PSTVNC_CONFIG_RFB_OFF = 0,
    PSTVNC_CONFIG_RFB_ON = 1
} pstvnc_config_rfb_mode_t;

typedef enum pstvnc_config_video_mode {
    PSTVNC_CONFIG_VIDEO_OFF = 0,
    PSTVNC_CONFIG_VIDEO_ACTIVE = 1
} pstvnc_config_video_mode_t;

typedef struct pstvnc_config_composition_profile {
    uint32_t session_id;
    pstvnc_config_rfb_mode_t rfb_mode;
    pstvnc_config_audio_mode_t audio_mode;
    pstvnc_config_video_mode_t video_mode;
} pstvnc_config_composition_profile_t;

typedef struct pstvnc_config_pcm_profile {
    uint32_t rate_hz;
    uint32_t channels;
    uint32_t bits_per_sample;
    uint32_t volume_percent;
} pstvnc_config_pcm_profile_t;

typedef struct pstvnc_config_media_clock_profile {
    uint32_t epoch_lead_us;
    int32_t audio_presentation_offset_us;
    int32_t video_presentation_offset_us;
} pstvnc_config_media_clock_profile_t;

typedef struct pstvnc_config_session_profile {
    pstvnc_config_composition_profile_t composition;
    pstvnc_transport_session_config_t transport;
    pstvnc_config_pcm_profile_t pcm;
    pstvnc_config_media_clock_profile_t media_clock;
} pstvnc_config_session_profile_t;

/*
 * Decode and validate one complete production CONFIG payload atomically.
 * On failure, *profile is left unchanged. The payload is a version/count header
 * followed by fixed (field-id, raw-u32-value) big-endian entries in any order.
 */
int pstvnc_config_profile_decode(
    pstvnc_config_session_profile_t *profile,
    const uint8_t *payload,
    size_t payload_length);

#endif
