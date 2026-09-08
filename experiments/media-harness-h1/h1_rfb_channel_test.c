/* Host test for H1 logical RFB queue, credit, and fragmentation mechanics. */

#include "h1_rfb_channel.h"

#include <stdio.h>
#include <string.h>

static int failures;
static unsigned char sent[64];
static size_t sent_count;
static size_t fragment_sizes[8];
static size_t fragment_count;
static int fail_fragment_index = -1;

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); failures++; \
} } while (0)

static int capture_fragment(void *context, const void *payload, size_t length)
{
    (void)context;
    if ((int)fragment_count == fail_fragment_index)
        return 0;
    if (fragment_count >= 8u || sent_count + length > sizeof(sent))
        return 0;
    fragment_sizes[fragment_count++] = length;
    memcpy(sent + sent_count, payload, length);
    sent_count += length;
    return 1;
}

int main(void)
{
    static unsigned char storage[PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES];
    pstvnc_h1_rfb_channel_t channel;
    unsigned char out[16];
    const unsigned char first[] = {1,2,3,4,5};
    const unsigned char second[] = {6,7,8};
    const unsigned char logical[] = {10,11,12,13,14,15,16,17,18,19};

    CHECK(!pstvnc_h1_rfb_channel_init(&channel, storage, 0u));
    CHECK(pstvnc_h1_rfb_channel_init(&channel, storage, 1024u));
    CHECK(channel.queue.capacity == 1024u);
    CHECK(pstvnc_h1_rfb_channel_init(&channel, storage, sizeof(storage)));
    CHECK(channel.queue.capacity == PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES);
    CHECK(pstvnc_h1_rfb_channel_poll(&channel) == 0);
    CHECK(pstvnc_h1_rfb_channel_accept_data(&channel, first, sizeof(first)));
    CHECK(pstvnc_h1_rfb_channel_accept_data(&channel, second, sizeof(second)));
    CHECK(pstvnc_h1_rfb_channel_queue_size(&channel) == 8u);
    CHECK(pstvnc_h1_rfb_channel_queue_high_water(&channel) == 8u);

    memset(out, 0, sizeof(out));
    CHECK(pstvnc_h1_rfb_channel_read_available(&channel, out, 3u) == 3u);
    CHECK(memcmp(out, first, 3u) == 0);
    CHECK(pstvnc_h1_rfb_channel_take_credit(&channel) == 3u);
    CHECK(pstvnc_h1_rfb_channel_queue_size(&channel) == 5u);

    memset(out, 0, sizeof(out));
    CHECK(pstvnc_h1_rfb_channel_read_available(&channel, out, sizeof(out)) == 5u);
    CHECK(out[0] == 4u && out[1] == 5u && out[2] == 6u &&
          out[3] == 7u && out[4] == 8u);
    CHECK(pstvnc_h1_rfb_channel_take_credit(&channel) == 5u);
    CHECK(pstvnc_h1_rfb_channel_poll(&channel) == 0);
    CHECK(channel.stats.bytes_enqueued == 8u);
    CHECK(channel.stats.bytes_consumed == 8u);

    memset(sent, 0, sizeof(sent));
    sent_count = 0u;
    fragment_count = 0u;
    fail_fragment_index = -1;
    CHECK(pstvnc_h1_rfb_channel_write_logical(
        &channel, logical, sizeof(logical), 4u, capture_fragment, NULL));
    CHECK(fragment_count == 3u);
    CHECK(fragment_sizes[0] == 4u && fragment_sizes[1] == 4u &&
          fragment_sizes[2] == 2u);
    CHECK(sent_count == sizeof(logical));
    CHECK(memcmp(sent, logical, sizeof(logical)) == 0);
    CHECK(channel.stats.logical_write_calls == 1u);
    CHECK(channel.stats.logical_bytes_written == sizeof(logical));
    CHECK(channel.stats.data_frames_sent == 3u);

    fragment_count = 0u;
    sent_count = 0u;
    fail_fragment_index = 1;
    CHECK(!pstvnc_h1_rfb_channel_write_logical(
        &channel, logical, sizeof(logical), 4u, capture_fragment, NULL));
    CHECK(fragment_count == 1u);

    if (failures != 0) {
        fprintf(stderr, "H1_RFB_CHANNEL_TEST=FAIL failures=%d\n", failures);
        return 1;
    }

    puts("H1_RFB_CHANNEL_TEST=PASS");
    return 0;
}
