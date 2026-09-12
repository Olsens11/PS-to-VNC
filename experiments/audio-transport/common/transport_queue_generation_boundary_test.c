/* Host contract for the queue primitive used by CP2P generation retirement. */
#include "transport_queue.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    pstvnc_transport_queue_t queue;
    unsigned char storage[8];
    unsigned char first[6] = {0, 1, 2, 3, 4, 5};
    unsigned char second[4] = {6, 7, 8, 9};
    unsigned char out[3];

    memset(&queue, 0, sizeof(queue));
    assert(pstvnc_transport_queue_init(&queue, storage, sizeof(storage)));
    assert(pstvnc_transport_queue_write(&queue, first, sizeof(first)));
    assert(pstvnc_transport_queue_read(&queue, out, 2u));
    assert(pstvnc_transport_queue_write(&queue, second, sizeof(second)));
    assert(pstvnc_transport_queue_size(&queue) == 8u);
    assert(pstvnc_transport_queue_high_water(&queue) == 8u);

    assert(pstvnc_transport_queue_discard_all(&queue) == 8u);
    assert(pstvnc_transport_queue_size(&queue) == 0u);
    assert(pstvnc_transport_queue_free(&queue) == 8u);
    assert(pstvnc_transport_queue_high_water(&queue) == 8u);
    assert(queue.read_position == queue.write_position);

    assert(pstvnc_transport_queue_write(&queue, first, 3u));
    assert(pstvnc_transport_queue_read(&queue, out, 3u));
    assert(memcmp(out, first, 3u) == 0);
    assert(pstvnc_transport_queue_discard_all(&queue) == 0u);

    puts("H1_CP2P_QUEUE_GENERATION_BOUNDARY_HOST_TEST=PASS");
    return 0;
}
