/*
 * File synopsis:
 * Defines configurable Audio Transport EXP2's complete, connection-scoped
 * Pi -> PS2 operating profile.
 *
 * Queue sizes, experimental thread stacks, and the audio consumer buffer are
 * runtime allocations. There is deliberately no artificial experiment queue
 * ceiling: the requested configuration succeeds only if the running PS2 can
 * actually allocate the requested resources.
 *
 * Validation rejects contradictory or unrepresentable settings. It does not
 * impose a guessed "safe" queue-size ceiling.
 */

#ifndef PSTVNC_EXPERIMENT_TRANSPORT_CONFIG_H
#define PSTVNC_EXPERIMENT_TRANSPORT_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_TRANSPORT_CONFIG_VERSION 1u

/*
 * CONFIG reply flag.
 *
 * A successful ACK echoes the complete accepted CONFIG payload byte-for-byte.
 */
#define PSTVNC_TRANSPORT_CONFIG_ACK_FLAG 0x01u

/*
 * HELLO capability:
 *
 * When this capability is present and HELLO's RFB/audio capacity words are
 * zero, queue capacity is selected by CONFIG and backed by runtime allocation.
 */
#define PSTVNC_TRANSPORT_CAP_CONFIG_DYNAMIC  (1u << 4)

typedef enum pstvnc_transport_audio_start_mode {
    PSTVNC_TRANSPORT_AUDIO_START_IMMEDIATE = 0,
    PSTVNC_TRANSPORT_AUDIO_START_TARGET = 1,
    PSTVNC_TRANSPORT_AUDIO_START_DELAY = 2
} pstvnc_transport_audio_start_mode_t;

typedef enum pstvnc_transport_queue_allocation_order {
    PSTVNC_TRANSPORT_ALLOCATE_RFB_FIRST = 0,
    PSTVNC_TRANSPORT_ALLOCATE_AUDIO_FIRST = 1
} pstvnc_transport_queue_allocation_order_t;

/*
 * CONFIG wire payload:
 *
 *   u32 config_version
 *   u32 profile_id
 *
 *   repeated complete field set:
 *       u32 field_id
 *       u32 value
 *
 * All integers are big endian.
 *
 * Profiles are intentionally complete rather than patch-like. Missing,
 * duplicate, or unknown fields fail closed.
 */
typedef enum pstvnc_transport_config_field {
    PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY = 1,
    PSTVNC_CONFIG_FIELD_AUDIO_QUEUE_CAPACITY = 2,

    PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES = 3,
    PSTVNC_CONFIG_FIELD_AUDIO_CREDIT_BATCH_BYTES = 4,

    PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY = 5,
    PSTVNC_CONFIG_FIELD_AUDIO_CREDIT_FLUSH_ON_EMPTY = 6,

    PSTVNC_CONFIG_FIELD_AUDIO_START_MODE = 7,
    PSTVNC_CONFIG_FIELD_AUDIO_START_TARGET_BYTES = 8,
    PSTVNC_CONFIG_FIELD_AUDIO_START_DELAY_US = 9,

    PSTVNC_CONFIG_FIELD_AUDIO_CHUNK_BYTES = 10,
    PSTVNC_CONFIG_FIELD_AUDIO_IDLE_DELAY_US = 11,
    PSTVNC_CONFIG_FIELD_RFB_EMPTY_DELAY_US = 12,

    PSTVNC_CONFIG_FIELD_AUDIO_THREAD_PRIORITY = 13,
    PSTVNC_CONFIG_FIELD_AUDIO_THREAD_STACK_SIZE = 14,

    PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY = 15,
    PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE = 16,

    PSTVNC_CONFIG_FIELD_AUDIO_RATE = 17,
    PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS = 18,
    PSTVNC_CONFIG_FIELD_AUDIO_BITS = 19,
    PSTVNC_CONFIG_FIELD_AUDIO_VOLUME = 20,

    PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD = 21,

    PSTVNC_CONFIG_FIELD_SOCKET_RECEIVE_BUFFER_BYTES = 22,
    PSTVNC_CONFIG_FIELD_SOCKET_SEND_BUFFER_BYTES = 23,

    PSTVNC_CONFIG_FIELD_QUEUE_ALLOCATION_ORDER = 24
} pstvnc_transport_config_field_t;

#define PSTVNC_TRANSPORT_CONFIG_FIELD_COUNT 24u

typedef struct pstvnc_transport_config {
    uint32_t version;
    uint32_t profile_id;

    uint32_t rfb_queue_capacity;
    uint32_t audio_queue_capacity;

    uint32_t rfb_credit_batch_bytes;
    uint32_t audio_credit_batch_bytes;

    uint32_t rfb_credit_flush_on_empty;
    uint32_t audio_credit_flush_on_empty;

    uint32_t audio_start_mode;
    uint32_t audio_start_target_bytes;
    uint32_t audio_start_delay_us;

    uint32_t audio_chunk_bytes;
    uint32_t audio_idle_delay_us;
    uint32_t rfb_empty_delay_us;

    uint32_t audio_thread_priority;
    uint32_t audio_thread_stack_size;

    uint32_t receiver_thread_priority;
    uint32_t receiver_thread_stack_size;

    uint32_t audio_rate;
    uint32_t audio_channels;
    uint32_t audio_bits;
    uint32_t audio_volume;

    uint32_t max_data_payload;

    uint32_t socket_receive_buffer_bytes;
    uint32_t socket_send_buffer_bytes;

    uint32_t queue_allocation_order;
} pstvnc_transport_config_t;

int pstvnc_transport_config_decode(
    pstvnc_transport_config_t *config,
    const uint8_t *payload,
    size_t payload_length);

int pstvnc_transport_config_validate(
    const pstvnc_transport_config_t *config);

uint32_t pstvnc_transport_config_digest(
    const uint8_t *payload,
    size_t payload_length);

uint32_t pstvnc_transport_config_audio_frame_bytes(
    const pstvnc_transport_config_t *config);

#endif
