/*
 * File synopsis:
 * Implements the fixed-storage byte ring used by experimental logical
 * transport channels.
 *
 * Context:
 *   experiments/audio-transport/README.md
 */

#include "transport_queue.h"

#include <string.h>

int pstvnc_transport_queue_init(
    pstvnc_transport_queue_t *queue,
    uint8_t *storage,
    size_t capacity)
{
    if (queue == NULL ||
        storage == NULL ||
        capacity == 0)
        return 0;

    queue->storage = storage;
    queue->capacity = capacity;
    queue->read_position = 0;
    queue->write_position = 0;
    queue->byte_count = 0;
    queue->high_water = 0;

    return 1;
}

size_t pstvnc_transport_queue_size(
    const pstvnc_transport_queue_t *queue)
{
    if (queue == NULL)
        return 0;

    return queue->byte_count;
}

size_t pstvnc_transport_queue_free(
    const pstvnc_transport_queue_t *queue)
{
    if (queue == NULL ||
        queue->byte_count > queue->capacity)
        return 0;

    return queue->capacity - queue->byte_count;
}

size_t pstvnc_transport_queue_high_water(
    const pstvnc_transport_queue_t *queue)
{
    if (queue == NULL)
        return 0;

    return queue->high_water;
}

int pstvnc_transport_queue_write(
    pstvnc_transport_queue_t *queue,
    const void *data,
    size_t count)
{
    const uint8_t *source =
        (const uint8_t *)data;

    size_t first_count;

    if (queue == NULL ||
        (data == NULL && count != 0) ||
        count > pstvnc_transport_queue_free(queue))
        return 0;

    if (count == 0)
        return 1;

    first_count =
        queue->capacity -
        queue->write_position;

    if (first_count > count)
        first_count = count;

    memcpy(
        queue->storage +
            queue->write_position,
        source,
        first_count);

    if (count > first_count) {
        memcpy(
            queue->storage,
            source + first_count,
            count - first_count);
    }

    queue->write_position =
        (queue->write_position + count) %
        queue->capacity;

    queue->byte_count += count;

    if (queue->byte_count > queue->high_water)
        queue->high_water = queue->byte_count;

    return 1;
}

int pstvnc_transport_queue_read(
    pstvnc_transport_queue_t *queue,
    void *data,
    size_t count)
{
    uint8_t *destination =
        (uint8_t *)data;

    size_t first_count;

    if (queue == NULL ||
        (data == NULL && count != 0) ||
        count > queue->byte_count)
        return 0;

    if (count == 0)
        return 1;

    first_count =
        queue->capacity -
        queue->read_position;

    if (first_count > count)
        first_count = count;

    memcpy(
        destination,
        queue->storage +
            queue->read_position,
        first_count);

    if (count > first_count) {
        memcpy(
            destination + first_count,
            queue->storage,
            count - first_count);
    }

    queue->read_position =
        (queue->read_position + count) %
        queue->capacity;

    queue->byte_count -= count;

    return 1;
}
