/*
 * File synopsis:
 * Defines H1's complete connection-scoped media profile for one physical PSTV
 * transport carrying PCM audio and MPEG-2 video as independent logical
 * channels.
 *
 * H1 is a laboratory harness. Queue sizes, buffer depths, delays, feed sizes,
 * priorities and presentation offsets are requested by the Pi for every
 * session. Validation rejects only contradictory, unrepresentable, or API-
 * impossible settings; it deliberately does not impose guessed safe ceilings.
 *
 * The existing Audio Transport EXP2 and qualified EXP3/P11 sources remain
 * unchanged. This is an H1-specific descendant.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_CONFIG_H
#define PSTVNC_MEDIA_HARNESS_H1_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_H1_CONFIG_VERSION 2u
#define PSTVNC_H1_CONFIG_ACK_FLAG 0x01u

/* H1-specific HELLO capability bits. */
#define PSTVNC_H1_CAP_MEDIA_MUX_DYNAMIC (1u << 5)
#define PSTVNC_H1_CAP_MPEG2_ES          (1u << 6)
#define PSTVNC_H1_CAP_REPEAT_SESSIONS   (1u << 7)

/*
 * audio_mode deliberately reserves the two compressed-audio architectures we
 * intend to compare later. This first H1 implementation accepts OFF and PCM;
 * future descendants may enable the reserved values without renumbering the
 * experiment vocabulary.
 */
typedef enum pstvnc_h1_audio_mode {
    PSTVNC_H1_AUDIO_OFF = 0,
    PSTVNC_H1_AUDIO_PCM = 1,
    PSTVNC_H1_AUDIO_MP2_SEPARATE_RESERVED = 2,
    PSTVNC_H1_AUDIO_MPEG_PS_RESERVED = 3
} pstvnc_h1_audio_mode_t;

typedef enum pstvnc_h1_video_mode {
    PSTVNC_H1_VIDEO_OFF = 0,
    PSTVNC_H1_VIDEO_MPEG2_ES = 1
} pstvnc_h1_video_mode_t;

typedef enum pstvnc_h1_audio_start_mode {
    PSTVNC_H1_AUDIO_START_IMMEDIATE = 0,
    PSTVNC_H1_AUDIO_START_TARGET = 1,
    PSTVNC_H1_AUDIO_START_DELAY = 2
} pstvnc_h1_audio_start_mode_t;

typedef enum pstvnc_h1_video_scheduler_mode {
    PSTVNC_H1_VIDEO_SCHED_ABSOLUTE = 0,
    PSTVNC_H1_VIDEO_SCHED_NONE = 1,
    PSTVNC_H1_VIDEO_SCHED_ONE_VSYNC = 2,
    PSTVNC_H1_VIDEO_SCHED_TWO_VSYNC = 3
} pstvnc_h1_video_scheduler_mode_t;

typedef enum pstvnc_h1_video_pixel_mode {
    PSTVNC_H1_VIDEO_RGB16 = 0,
    PSTVNC_H1_VIDEO_RGB32 = 1
} pstvnc_h1_video_pixel_mode_t;

typedef enum pstvnc_h1_queue_allocation_order {
    PSTVNC_H1_ALLOCATE_AUDIO_FIRST = 0,
    PSTVNC_H1_ALLOCATE_MPEG_FIRST = 1
} pstvnc_h1_queue_allocation_order_t;

/*
 * CONFIG wire payload:
 *
 *   u32 config_version
 *   u32 profile_id
 *   repeated complete field set:
 *       u32 field_id
 *       u32 value
 *
 * All integers are big endian. Signed presentation offsets use the raw two's
 * complement bits of an int32_t in the u32 value slot.
 */
typedef enum pstvnc_h1_config_field {
    PSTVNC_H1_FIELD_SESSION_ID = 1,
    PSTVNC_H1_FIELD_AUDIO_MODE = 2,
    PSTVNC_H1_FIELD_VIDEO_MODE = 3,

    PSTVNC_H1_FIELD_AUDIO_QUEUE_CAPACITY = 4,
    PSTVNC_H1_FIELD_MPEG_QUEUE_CAPACITY = 5,

    PSTVNC_H1_FIELD_AUDIO_CREDIT_BATCH_BYTES = 6,
    PSTVNC_H1_FIELD_MPEG_CREDIT_BATCH_BYTES = 7,
    PSTVNC_H1_FIELD_AUDIO_CREDIT_FLUSH_ON_EMPTY = 8,
    PSTVNC_H1_FIELD_MPEG_CREDIT_FLUSH_ON_EMPTY = 9,
    PSTVNC_H1_FIELD_AUDIO_CREDIT_RETURN_ENABLED = 10,
    PSTVNC_H1_FIELD_MPEG_CREDIT_RETURN_ENABLED = 11,
    PSTVNC_H1_FIELD_AUDIO_INITIAL_CREDIT_BYTES = 12,
    PSTVNC_H1_FIELD_MPEG_INITIAL_CREDIT_BYTES = 13,

    PSTVNC_H1_FIELD_AUDIO_START_MODE = 14,
    PSTVNC_H1_FIELD_AUDIO_START_TARGET_BYTES = 15,
    PSTVNC_H1_FIELD_AUDIO_START_DELAY_US = 16,
    PSTVNC_H1_FIELD_AUDIO_CHUNK_BYTES = 17,
    PSTVNC_H1_FIELD_AUDIO_IDLE_DELAY_US = 18,
    PSTVNC_H1_FIELD_AUDIO_THREAD_PRIORITY = 19,
    PSTVNC_H1_FIELD_AUDIO_THREAD_STACK_SIZE = 20,
    PSTVNC_H1_FIELD_AUDIO_RATE = 21,
    PSTVNC_H1_FIELD_AUDIO_CHANNELS = 22,
    PSTVNC_H1_FIELD_AUDIO_BITS = 23,
    PSTVNC_H1_FIELD_AUDIO_VOLUME = 24,
    PSTVNC_H1_FIELD_AUDIO_PRESENTATION_OFFSET_US = 25,

    PSTVNC_H1_FIELD_MPEG_START_TARGET_BYTES = 26,
    PSTVNC_H1_FIELD_MPEG_PREFILL_WAIT_US = 27,
    PSTVNC_H1_FIELD_MPEG_PREFILL_MAX_LOOPS = 28,
    PSTVNC_H1_FIELD_MPEG_EMPTY_DELAY_US = 29,
    PSTVNC_H1_FIELD_MPEG_FEED_BYTES = 30,

    PSTVNC_H1_FIELD_VIDEO_FPS_NUM = 31,
    PSTVNC_H1_FIELD_VIDEO_FPS_DEN = 32,
    PSTVNC_H1_FIELD_VIDEO_SCHEDULER_MODE = 33,
    PSTVNC_H1_FIELD_VIDEO_PRESENTATION_OFFSET_US = 34,
    PSTVNC_H1_FIELD_VIDEO_PIXEL_MODE = 35,
    PSTVNC_H1_FIELD_VIDEO_MAX_WIDTH = 36,
    PSTVNC_H1_FIELD_VIDEO_MAX_HEIGHT = 37,
    PSTVNC_H1_FIELD_VIDEO_DRAW_WIDTH = 38,
    PSTVNC_H1_FIELD_VIDEO_DRAW_HEIGHT = 39,
    PSTVNC_H1_FIELD_VIDEO_DRAW_X = 40,
    PSTVNC_H1_FIELD_VIDEO_DRAW_Y = 41,
    PSTVNC_H1_FIELD_VIDEO_STAGE_MARKERS = 42,
    PSTVNC_H1_FIELD_VIDEO_STAGE_HOLD_VSYNCS = 43,
    PSTVNC_H1_FIELD_VIDEO_IPU_RESET_EACH_SESSION = 44,
    PSTVNC_H1_FIELD_VIDEO_DROP_ENABLED = 45,
    PSTVNC_H1_FIELD_VIDEO_DROP_THRESHOLD_MILLIFRAMES = 46,

    PSTVNC_H1_FIELD_RECEIVER_THREAD_PRIORITY = 47,
    PSTVNC_H1_FIELD_RECEIVER_THREAD_STACK_SIZE = 48,
    PSTVNC_H1_FIELD_MAX_DATA_PAYLOAD = 49,
    PSTVNC_H1_FIELD_SOCKET_RECEIVE_BUFFER_BYTES = 50,
    PSTVNC_H1_FIELD_SOCKET_SEND_BUFFER_BYTES = 51,
    PSTVNC_H1_FIELD_QUEUE_ALLOCATION_ORDER = 52,
    PSTVNC_H1_FIELD_MEDIA_EPOCH_LEAD_US = 53
} pstvnc_h1_config_field_t;

#define PSTVNC_H1_CONFIG_FIELD_COUNT 53u

typedef struct pstvnc_h1_config {
    uint32_t version;
    uint32_t profile_id;

    uint32_t session_id;
    uint32_t audio_mode;
    uint32_t video_mode;

    uint32_t audio_queue_capacity;
    uint32_t mpeg_queue_capacity;

    uint32_t audio_credit_batch_bytes;
    uint32_t mpeg_credit_batch_bytes;
    uint32_t audio_credit_flush_on_empty;
    uint32_t mpeg_credit_flush_on_empty;
    uint32_t audio_credit_return_enabled;
    uint32_t mpeg_credit_return_enabled;
    uint32_t audio_initial_credit_bytes;
    uint32_t mpeg_initial_credit_bytes;

    uint32_t audio_start_mode;
    uint32_t audio_start_target_bytes;
    uint32_t audio_start_delay_us;
    uint32_t audio_chunk_bytes;
    uint32_t audio_idle_delay_us;
    uint32_t audio_thread_priority;
    uint32_t audio_thread_stack_size;
    uint32_t audio_rate;
    uint32_t audio_channels;
    uint32_t audio_bits;
    uint32_t audio_volume;
    uint32_t audio_presentation_offset_us;

    uint32_t mpeg_start_target_bytes;
    uint32_t mpeg_prefill_wait_us;
    uint32_t mpeg_prefill_max_loops;
    uint32_t mpeg_empty_delay_us;
    uint32_t mpeg_feed_bytes;

    uint32_t video_fps_num;
    uint32_t video_fps_den;
    uint32_t video_scheduler_mode;
    uint32_t video_presentation_offset_us;
    uint32_t video_pixel_mode;
    uint32_t video_max_width;
    uint32_t video_max_height;
    uint32_t video_draw_width;
    uint32_t video_draw_height;
    uint32_t video_draw_x;
    uint32_t video_draw_y;
    uint32_t video_stage_markers;
    uint32_t video_stage_hold_vsyncs;
    uint32_t video_ipu_reset_each_session;
    uint32_t video_drop_enabled;
    uint32_t video_drop_threshold_milliframes;

    uint32_t receiver_thread_priority;
    uint32_t receiver_thread_stack_size;
    uint32_t max_data_payload;
    uint32_t socket_receive_buffer_bytes;
    uint32_t socket_send_buffer_bytes;
    uint32_t queue_allocation_order;
    uint32_t media_epoch_lead_us;
} pstvnc_h1_config_t;

int pstvnc_h1_config_decode(
    pstvnc_h1_config_t *config,
    const uint8_t *payload,
    size_t payload_length);

int pstvnc_h1_config_validate(
    const pstvnc_h1_config_t *config);

uint32_t pstvnc_h1_config_digest(
    const uint8_t *payload,
    size_t payload_length);

uint32_t pstvnc_h1_config_audio_frame_bytes(
    const pstvnc_h1_config_t *config);

int32_t pstvnc_h1_config_audio_offset_us(
    const pstvnc_h1_config_t *config);

int32_t pstvnc_h1_config_video_offset_us(
    const pstvnc_h1_config_t *config);

#endif
