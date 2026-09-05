/*
 * File synopsis:
 * Implements the pure ordinary semantic input-event FIFO.
 *
 * This translation unit owns only event-envelope validation and bounded FIFO
 * mechanics. It does not know how controller hardware is polled, how mouse
 * behavior is calculated, how application contexts route events, or how RFB
 * serializes remote input.
 *
 * Keeping those responsibilities outside this file is deliberate: the FIFO is
 * infrastructure inside the input family, not an alternate application
 * coordinator or protocol layer.
 *
 * Context:
 *   docs/PROJECT_INTENT.md, explicit composition;
 *   docs/CLEAN_ARCHITECTURE.md, ordinary semantic input queue;
 *   GitHub Issue #38.
 */

#include "input.h"

#include <stddef.h>

/*
 * Validate only the semantic envelope type owned by this module.
 *
 * Payload-specific policy remains with the producer/consumer that understands
 * that payload. In particular, this function does not decide whether a mouse
 * update contains motion, a button change, or a wheel notch.
 */
static int input_event_type_is_valid(
    pstvnc_input_event_type_t event_type)
{
    switch (event_type) {
    case PSTVNC_INPUT_EVENT_CONTROLLER_STATE:
    case PSTVNC_INPUT_EVENT_MOUSE_UPDATE:
    case PSTVNC_INPUT_EVENT_KEYBOARD_TAP:
        return 1;

    case PSTVNC_INPUT_EVENT_NONE:
    default:
        return 0;
    }
}

void pstvnc_input_queue_init(
    pstvnc_input_queue_t *queue)
{
    if (queue == NULL)
        return;

    queue->read_index = 0;
    queue->write_index = 0;
    queue->event_count = 0;
}

void pstvnc_input_queue_discard_all(
    pstvnc_input_queue_t *queue)
{
    if (queue == NULL)
        return;

    /*
     * event_count is the authority for which storage entries are live.
     *
     * Resetting the indices and count therefore establishes an empty queue
     * without needlessly clearing every inaccessible payload byte. No stale
     * entry can be returned unless it is accepted again through push().
     */
    queue->read_index = 0;
    queue->write_index = 0;
    queue->event_count = 0;
}

unsigned int pstvnc_input_queue_event_count(
    const pstvnc_input_queue_t *queue)
{
    if (queue == NULL)
        return 0;

    return queue->event_count;
}

int pstvnc_input_queue_push(
    pstvnc_input_queue_t *queue,
    const pstvnc_input_event_t *event)
{
    if (queue == NULL || event == NULL)
        return 0;

    if (!input_event_type_is_valid(event->type))
        return 0;

    /*
     * Reject the complete new semantic event when capacity is exhausted.
     *
     * Overwriting the oldest entry or silently accepting only part of new work
     * would invent an input-loss policy inside FIFO mechanics. The future input
     * runtime owns the operational consequence of queue exhaustion.
     */
    if (queue->event_count >=
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY)
        return 0;

    queue->event_storage[queue->write_index] = *event;

    queue->write_index =
        (queue->write_index + 1u) %
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;

    queue->event_count++;

    return 1;
}

int pstvnc_input_queue_pop(
    pstvnc_input_queue_t *queue,
    pstvnc_input_event_t *event)
{
    if (queue == NULL || event == NULL)
        return 0;

    if (queue->event_count == 0)
        return 0;

    *event = queue->event_storage[queue->read_index];

    queue->read_index =
        (queue->read_index + 1u) %
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;

    queue->event_count--;

    return 1;
}
