/*
 * File synopsis:
 * Exercises the pure Issue #38 semantic input-event envelope and bounded FIFO.
 *
 * These tests intentionally require no PS2 hardware, controller thread,
 * semaphore, mouse implementation, or RFB transport. They prove the ordinary
 * producer/consumer data boundary before PS2 concurrency is added around it.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

static pstvnc_input_event_t make_mouse_event(
    unsigned int cursor_x,
    unsigned int cursor_y,
    unsigned char click_buttons,
    int pointer_changed,
    pstvnc_mouse_wheel_direction_t wheel_direction)
{
    pstvnc_input_event_t event;

    memset(&event, 0, sizeof(event));

    event.type = PSTVNC_INPUT_EVENT_MOUSE_UPDATE;

    event.payload.mouse_update.cursor_x = cursor_x;
    event.payload.mouse_update.cursor_y = cursor_y;
    event.payload.mouse_update.click_buttons = click_buttons;
    event.payload.mouse_update.pointer_changed = pointer_changed;
    event.payload.mouse_update.wheel_direction = wheel_direction;

    return event;
}

static void test_initial_state_is_empty(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;

    pstvnc_input_queue_init(&queue);

    assert(pstvnc_input_queue_event_count(&queue) == 0u);
    assert(!pstvnc_input_queue_pop(&queue, &event));
}

static void test_one_typed_event_round_trips(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t input_event;
    pstvnc_input_event_t output_event;

    pstvnc_input_queue_init(&queue);

    input_event =
        make_mouse_event(
            123u,
            234u,
            PSTVNC_MOUSE_BUTTON_LCLICK,
            1,
            PSTVNC_MOUSE_WHEEL_NONE);

    assert(
        pstvnc_input_queue_push(
            &queue,
            &input_event));

    assert(pstvnc_input_queue_event_count(&queue) == 1u);

    assert(
        pstvnc_input_queue_pop(
            &queue,
            &output_event));

    assert(
        output_event.type ==
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE);

    assert(
        output_event.payload.mouse_update.cursor_x ==
        123u);

    assert(
        output_event.payload.mouse_update.cursor_y ==
        234u);

    assert(
        output_event.payload.mouse_update.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    assert(
        output_event.payload.mouse_update.pointer_changed ==
        1);

    assert(
        output_event.payload.mouse_update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    assert(pstvnc_input_queue_event_count(&queue) == 0u);
}

static void test_queue_preserves_payload_without_interpreting_it(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t input_event;
    pstvnc_input_event_t output_event;

    pstvnc_input_queue_init(&queue);

    /*
     * This mouse payload deliberately describes no pointer change and no wheel
     * notch. The FIFO still transports it because deciding whether that sample
     * is useful belongs to the mouse-aware producer, not generic queue
     * mechanics.
     */
    input_event =
        make_mouse_event(
            320u,
            240u,
            0,
            0,
            PSTVNC_MOUSE_WHEEL_NONE);

    assert(
        pstvnc_input_queue_push(
            &queue,
            &input_event));

    assert(pstvnc_input_queue_event_count(&queue) == 1u);

    assert(
        pstvnc_input_queue_pop(
            &queue,
            &output_event));

    assert(
        output_event.payload.mouse_update.cursor_x ==
        320u);

    assert(
        output_event.payload.mouse_update.cursor_y ==
        240u);

    assert(
        output_event.payload.mouse_update.pointer_changed ==
        0);

    assert(
        output_event.payload.mouse_update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);
}

static void test_fifo_order_is_preserved(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    unsigned int event_index;

    pstvnc_input_queue_init(&queue);

    for (event_index = 0; event_index < 16u; event_index++) {
        event =
            make_mouse_event(
                event_index,
                1000u + event_index,
                0,
                1,
                PSTVNC_MOUSE_WHEEL_NONE);

        assert(
            pstvnc_input_queue_push(
                &queue,
                &event));
    }

    assert(pstvnc_input_queue_event_count(&queue) == 16u);

    for (event_index = 0; event_index < 16u; event_index++) {
        assert(
            pstvnc_input_queue_pop(
                &queue,
                &event));

        assert(
            event.payload.mouse_update.cursor_x ==
            event_index);

        assert(
            event.payload.mouse_update.cursor_y ==
            1000u + event_index);
    }

    assert(pstvnc_input_queue_event_count(&queue) == 0u);
}

static void test_full_queue_rejects_new_event_without_mutation(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    pstvnc_input_event_t rejected_event;
    unsigned int event_index;

    pstvnc_input_queue_init(&queue);

    for (event_index = 0;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;
         event_index++) {

        event =
            make_mouse_event(
                event_index,
                event_index + 1u,
                0,
                1,
                PSTVNC_MOUSE_WHEEL_NONE);

        assert(
            pstvnc_input_queue_push(
                &queue,
                &event));
    }

    assert(
        pstvnc_input_queue_event_count(&queue) ==
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY);

    rejected_event =
        make_mouse_event(
            9999u,
            9999u,
            PSTVNC_MOUSE_BUTTON_LCLICK,
            1,
            PSTVNC_MOUSE_WHEEL_UP);

    assert(
        !pstvnc_input_queue_push(
            &queue,
            &rejected_event));

    assert(
        pstvnc_input_queue_event_count(&queue) ==
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY);

    /*
     * Overflow must not evict the oldest accepted input.
     */
    assert(
        pstvnc_input_queue_pop(
            &queue,
            &event));

    assert(event.payload.mouse_update.cursor_x == 0u);
    assert(event.payload.mouse_update.cursor_y == 1u);
}

static void test_ring_wraparound_preserves_order(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    unsigned int event_index;

    pstvnc_input_queue_init(&queue);

    for (event_index = 0;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;
         event_index++) {

        event =
            make_mouse_event(
                event_index,
                0,
                0,
                1,
                PSTVNC_MOUSE_WHEEL_NONE);

        assert(
            pstvnc_input_queue_push(
                &queue,
                &event));
    }

    for (event_index = 0;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u;
         event_index++) {

        assert(
            pstvnc_input_queue_pop(
                &queue,
                &event));

        assert(
            event.payload.mouse_update.cursor_x ==
            event_index);
    }

    for (event_index = 0;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u;
         event_index++) {

        event =
            make_mouse_event(
                1000u + event_index,
                0,
                0,
                1,
                PSTVNC_MOUSE_WHEEL_NONE);

        assert(
            pstvnc_input_queue_push(
                &queue,
                &event));
    }

    assert(
        pstvnc_input_queue_event_count(&queue) ==
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY);

    for (event_index =
             PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;
         event_index++) {

        assert(
            pstvnc_input_queue_pop(
                &queue,
                &event));

        assert(
            event.payload.mouse_update.cursor_x ==
            event_index);
    }

    for (event_index = 0;
         event_index < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u;
         event_index++) {

        assert(
            pstvnc_input_queue_pop(
                &queue,
                &event));

        assert(
            event.payload.mouse_update.cursor_x ==
            1000u + event_index);
    }

    assert(!pstvnc_input_queue_pop(&queue, &event));
}

static void test_discard_all_is_explicit_queue_boundary(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    unsigned int event_index;

    pstvnc_input_queue_init(&queue);

    for (event_index = 0; event_index < 8u; event_index++) {
        event =
            make_mouse_event(
                50u + event_index,
                75u + event_index,
                0,
                1,
                PSTVNC_MOUSE_WHEEL_NONE);

        assert(
            pstvnc_input_queue_push(
                &queue,
                &event));
    }

    assert(pstvnc_input_queue_event_count(&queue) == 8u);

    pstvnc_input_queue_discard_all(&queue);

    assert(pstvnc_input_queue_event_count(&queue) == 0u);
    assert(!pstvnc_input_queue_pop(&queue, &event));

    /*
     * The same queue object remains usable immediately after the boundary.
     */
    event =
        make_mouse_event(
            400u,
            300u,
            0,
            1,
            PSTVNC_MOUSE_WHEEL_RIGHT);

    assert(
        pstvnc_input_queue_push(
            &queue,
            &event));

    assert(
        pstvnc_input_queue_pop(
            &queue,
            &event));

    assert(event.payload.mouse_update.cursor_x == 400u);

    assert(
        event.payload.mouse_update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_RIGHT);
}

static void test_invalid_event_type_is_rejected(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;

    pstvnc_input_queue_init(&queue);

    memset(&event, 0, sizeof(event));

    assert(event.type == PSTVNC_INPUT_EVENT_NONE);

    assert(
        !pstvnc_input_queue_push(
            &queue,
            &event));

    assert(pstvnc_input_queue_event_count(&queue) == 0u);

    event.type = (pstvnc_input_event_type_t)999;

    assert(
        !pstvnc_input_queue_push(
            &queue,
            &event));

    assert(pstvnc_input_queue_event_count(&queue) == 0u);
}

static void test_null_argument_contract(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;

    event =
        make_mouse_event(
            1u,
            2u,
            0,
            1,
            PSTVNC_MOUSE_WHEEL_NONE);

    pstvnc_input_queue_init(NULL);
    pstvnc_input_queue_discard_all(NULL);

    assert(pstvnc_input_queue_event_count(NULL) == 0u);

    pstvnc_input_queue_init(&queue);

    assert(!pstvnc_input_queue_push(NULL, &event));
    assert(!pstvnc_input_queue_push(&queue, NULL));

    assert(!pstvnc_input_queue_pop(NULL, &event));
    assert(!pstvnc_input_queue_pop(&queue, NULL));
}

int main(void)
{
    test_initial_state_is_empty();
    test_one_typed_event_round_trips();
    test_queue_preserves_payload_without_interpreting_it();
    test_fifo_order_is_preserved();
    test_full_queue_rejects_new_event_without_mutation();
    test_ring_wraparound_preserves_order();
    test_discard_all_is_explicit_queue_boundary();
    test_invalid_event_type_is_rejected();
    test_null_argument_contract();

    puts("INPUT_TEST=PASS");
    return 0;
}
