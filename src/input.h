/*
 * File synopsis:
 * Owns the ordinary semantic input-event vocabulary and bounded FIFO boundary
 * between input-side producers and application-side routing.
 *
 * This interface does not poll libpad, interpret controller motion, resolve
 * chords or bindings, own PS2 thread synchronization, serialize RFB messages,
 * manipulate UI/display state, or execute product actions.
 *
 * Producers first obtain meaning from their own responsibility. For example,
 * the mouse component converts already-routed controller facts into a semantic
 * mouse update. That result can then be placed in this ordinary event stream.
 * The application-side consumer decides where the semantic event is routed.
 *
 * Mouse updates are the first payload family earned by Issue #38. The envelope
 * is deliberately capable of receiving additional typed payload members when a
 * later reconstruction stage earns a real semantic event family; no unused
 * keyboard/UI/action vocabulary is declared in advance.
 *
 * Context:
 *   docs/PROJECT_INTENT.md, composition and complexity hierarchy;
 *   docs/CLEAN_ARCHITECTURE.md, Input/controller and concurrency model;
 *   GitHub Issues #37 and #38.
 */

#ifndef PSTVNC_INPUT_H
#define PSTVNC_INPUT_H

#include "mouse.h"

/*
 * The historical controller path qualified a bounded 256-entry producer /
 * consumer queue under ordinary interactive use.
 *
 * The clean reconstruction retains that conservative depth while deliberately
 * changing the queue contents from pre-encoded RFB packets to semantic input
 * events. Capacity remains an internal input-runtime choice, not a wire-format
 * or user-facing compatibility contract.
 */
#define PSTVNC_INPUT_EVENT_QUEUE_CAPACITY 256u

/*
 * Semantic event type discriminator.
 *
 * NONE deliberately occupies zero so a zero-initialized or otherwise
 * unfinished event cannot accidentally masquerade as valid application work.
 *
 * Add a new value only when a reconstructed behavior family genuinely needs to
 * cross the input -> application boundary.
 */
typedef enum pstvnc_input_event_type {
    PSTVNC_INPUT_EVENT_NONE = 0,
    PSTVNC_INPUT_EVENT_MOUSE_UPDATE
} pstvnc_input_event_type_t;

/*
 * Typed payload storage for one semantic input event.
 *
 * A named union makes the event envelope extensible without introducing a
 * generic byte buffer, casts, allocation, callback registry, or message-bus
 * abstraction. Each future payload remains an ordinary readable C value.
 *
 * Only mouse_update exists today because it is the only ordinary semantic
 * payload earned by the current reconstruction stage.
 */
typedef union pstvnc_input_event_payload {
    pstvnc_mouse_update_t mouse_update;
} pstvnc_input_event_payload_t;

/*
 * One application-routable semantic input event.
 *
 * type states what the event means at the input/application boundary.
 * payload contains the corresponding ordinary typed value.
 *
 * The queue treats this envelope atomically. It never splits one semantic
 * observation into separately fallible fragments.
 */
typedef struct pstvnc_input_event {
    pstvnc_input_event_type_t type;
    pstvnc_input_event_payload_t payload;
} pstvnc_input_event_t;

/*
 * Pure bounded FIFO mechanics for ordinary semantic events.
 *
 * This object deliberately contains no mutex/semaphore/thread primitive.
 * Synchronization belongs to the forthcoming PS2 input runtime that owns the
 * actual producer lifecycle and cross-thread access. Keeping the FIFO itself
 * pure makes ordering, wraparound, capacity, and discard semantics completely
 * host-testable.
 *
 * read_index identifies the oldest accepted event.
 * write_index identifies the next free publication slot.
 * event_count is the number of accepted events currently available to consume.
 */
typedef struct pstvnc_input_queue {
    pstvnc_input_event_t
        event_storage[PSTVNC_INPUT_EVENT_QUEUE_CAPACITY];

    unsigned int read_index;
    unsigned int write_index;
    unsigned int event_count;
} pstvnc_input_queue_t;

/*
 * Initialize one empty ordinary semantic-event FIFO.
 */
void pstvnc_input_queue_init(
    pstvnc_input_queue_t *queue);

/*
 * Discard all currently queued ordinary semantic input.
 *
 * This is a first-class operation because later hard ownership/context
 * boundaries must be able to prove that pre-boundary remote input cannot emerge
 * afterward as newly executed application work.
 *
 * Discarding the FIFO does not itself alter controller, mouse, UI, or remote
 * state. Each owner remains responsible for its own state boundary.
 */
void pstvnc_input_queue_discard_all(
    pstvnc_input_queue_t *queue);

/*
 * Report the number of accepted events currently waiting for consumption.
 *
 * When producer and consumer run on different threads, the input runtime must
 * provide synchronization around this pure queue operation.
 */
unsigned int pstvnc_input_queue_event_count(
    const pstvnc_input_queue_t *queue);

/*
 * Attempt to append one complete semantic event.
 *
 * Returns 1 when the complete event was accepted.
 * Returns 0 for an invalid argument, invalid/unrecognized event type, or a full
 * queue.
 *
 * Queue exhaustion never silently overwrites an older event and never silently
 * drops part of a new event. Operational policy for producer-side exhaustion
 * belongs to the controller/input runtime rather than these FIFO mechanics.
 *
 * The FIFO deliberately does not inspect payload meaning. For example, deciding
 * whether a mouse update represents useful work belongs to the producer that
 * understands mouse semantics, not to the transport queue.
 */
int pstvnc_input_queue_push(
    pstvnc_input_queue_t *queue,
    const pstvnc_input_event_t *event);

/*
 * Remove the oldest accepted event.
 *
 * Returns 1 after copying one complete event to *event.
 * Returns 0 when the queue is empty or an argument is invalid.
 */
int pstvnc_input_queue_pop(
    pstvnc_input_queue_t *queue,
    pstvnc_input_event_t *event);

#endif /* PSTVNC_INPUT_H */
