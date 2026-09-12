/*
 * File synopsis:
 * Defines a bounded caller-storage byte ring for one logical transport channel.
 *
 * The queue performs no allocation and owns no threading or synchronization.
 * A PS2 runtime wrapper may protect it with the appropriate semaphore while
 * host tests can exercise the mechanism without PS2 dependencies.
 *
 * Context:
 *   experiments/audio-transport/README.md
 */

#ifndef PSTVNC_EXPERIMENT_TRANSPORT_QUEUE_H
#define PSTVNC_EXPERIMENT_TRANSPORT_QUEUE_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_queue {
    uint8_t *storage;
    size_t capacity;

    size_t read_position;
    size_t write_position;
    size_t byte_count;

    /*
     * Historical maximum occupancy since initialization. This is mechanism
     * telemetry; it does not alter queue policy.
     */
    size_t high_water;
} pstvnc_transport_queue_t;

int pstvnc_transport_queue_init(
    pstvnc_transport_queue_t *queue,
    uint8_t *storage,
    size_t capacity);

size_t pstvnc_transport_queue_size(
    const pstvnc_transport_queue_t *queue);

size_t pstvnc_transport_queue_free(
    const pstvnc_transport_queue_t *queue);

size_t pstvnc_transport_queue_high_water(
    const pstvnc_transport_queue_t *queue);

/*
 * All-or-nothing operations.
 *
 * A failed write does not discard old data or partially enqueue new data.
 * A failed read does not partially consume existing data.
 */
int pstvnc_transport_queue_write(
    pstvnc_transport_queue_t *queue,
    const void *data,
    size_t count);

int pstvnc_transport_queue_read(
    pstvnc_transport_queue_t *queue,
    void *data,
    size_t count);

/*
 * Discard every currently queued byte without reallocating or lowering the
 * historical high-water mark. Returns the number of discarded bytes.
 */
size_t pstvnc_transport_queue_discard_all(
    pstvnc_transport_queue_t *queue);

#endif
