/*
 * File synopsis:
 * Defines the portable wire-frame vocabulary for the dedicated-link transport
 * experiment.
 *
 * This module owns only frame representation and byte encoding. It deliberately
 * knows nothing about TCP, UDP, raw Ethernet, PS2IP, RFB parsing, audio
 * playback, scheduling policy, or threading.
 *
 * Context:
 *   experiments/audio-transport/README.md
 */

#ifndef PSTVNC_EXPERIMENT_TRANSPORT_PROTOCOL_H
#define PSTVNC_EXPERIMENT_TRANSPORT_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

/*
 * "PSTV" on the wire.
 */
#define PSTVNC_TRANSPORT_MAGIC       0x50535456u
#define PSTVNC_TRANSPORT_VERSION     1u

/*
 * Fixed header size keeps framing cheap and deterministic on the PS2.
 *
 * Payload size is a protocol safety ceiling, not a Pi scheduling quantum.
 * The Pi may choose substantially smaller frames at runtime.
 */
#define PSTVNC_TRANSPORT_HEADER_SIZE 16u
#define PSTVNC_TRANSPORT_MAX_PAYLOAD 8192u

/*
 * Fixed v1 control payload sizes.
 *
 * HELLO advertises PS2 mechanism and physical-capacity facts.
 * CREDIT grants one increment of newly available receiver capacity.
 * TELEMETRY is a Pi-requested snapshot; the Pi controls request cadence.
 */
#define PSTVNC_TRANSPORT_HELLO_PAYLOAD_SIZE      24u
#define PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE      4u
#define PSTVNC_TRANSPORT_TELEMETRY_PAYLOAD_SIZE  96u
#define PSTVNC_TRANSPORT_TELEMETRY_VERSION        1u

#define PSTVNC_TRANSPORT_CAP_RFB                 (1u << 0)
#define PSTVNC_TRANSPORT_CAP_AUDIO_PCM_S16       (1u << 1)
#define PSTVNC_TRANSPORT_CAP_RECEIVER_CREDIT     (1u << 2)
#define PSTVNC_TRANSPORT_CAP_TELEMETRY_SNAPSHOT  (1u << 3)

typedef enum pstvnc_transport_frame_kind {
    PSTVNC_TRANSPORT_FRAME_HELLO = 1,
    PSTVNC_TRANSPORT_FRAME_CONFIG = 2,
    PSTVNC_TRANSPORT_FRAME_DATA = 3,
    PSTVNC_TRANSPORT_FRAME_CREDIT = 4,
    PSTVNC_TRANSPORT_FRAME_TELEMETRY = 5,
    PSTVNC_TRANSPORT_FRAME_HEARTBEAT = 6,
    PSTVNC_TRANSPORT_FRAME_ERROR = 7
} pstvnc_transport_frame_kind_t;

typedef enum pstvnc_transport_channel {
    PSTVNC_TRANSPORT_CHANNEL_CONTROL = 0,
    PSTVNC_TRANSPORT_CHANNEL_RFB = 1,
    PSTVNC_TRANSPORT_CHANNEL_AUDIO = 2,
    PSTVNC_TRANSPORT_CHANNEL_TELEMETRY = 3,

    /*
     * Reserved now so later MPEG experimentation does not require redefining
     * the existing channel identities.
     */
    PSTVNC_TRANSPORT_CHANNEL_MPEG2 = 4
} pstvnc_transport_channel_t;

/*
 * Host representation of one decoded wire header.
 *
 * kind and channel intentionally remain raw bytes instead of enum-typed fields.
 * Framing can therefore report a syntactically valid but unsupported value to
 * the runtime, where capability/error policy actually belongs.
 */
typedef struct pstvnc_transport_header {
    uint8_t version;
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    uint32_t sequence;
    uint32_t payload_length;
} pstvnc_transport_header_t;

/*
 * Wire layout:
 *
 *   bytes 0..3   magic "PSTV"
 *   byte  4      version
 *   byte  5      frame kind
 *   byte  6      logical channel
 *   byte  7      flags
 *   bytes 8..11  direction-local sequence, big endian
 *   bytes 12..15 payload length, big endian
 */
int pstvnc_transport_header_encode(
    uint8_t output[PSTVNC_TRANSPORT_HEADER_SIZE],
    const pstvnc_transport_header_t *header);

int pstvnc_transport_header_decode(
    pstvnc_transport_header_t *header,
    const uint8_t input[PSTVNC_TRANSPORT_HEADER_SIZE]);

uint32_t pstvnc_transport_read_be32(
    const uint8_t input[4]);

void pstvnc_transport_write_be32(
    uint8_t output[4],
    uint32_t value);

#endif
