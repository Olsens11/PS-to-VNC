/*
 * File synopsis:
 * Proves R28's PRODUCT_ACTION semantic input-event representation across
 * validation, FIFO ordering, ring wraparound, copying, and discard boundaries.
 */

#include <stdio.h>
#include <string.h>

#include "input/input.h"

static int failures;

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            failures++; \
        } \
    } while (0)

static pstvnc_input_event_t product_action_event(
    pstvnc_product_action_t action)
{
    pstvnc_input_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = PSTVNC_INPUT_EVENT_PRODUCT_ACTION;
    event.payload.product_action = action;
    return event;
}

static pstvnc_input_event_t controller_event(unsigned int value)
{
    pstvnc_input_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = PSTVNC_INPUT_EVENT_CONTROLLER_STATE;
    event.payload.controller_state.buttons_down = (uint16_t)value;
    return event;
}

static void test_product_action_validates_and_round_trips(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    pstvnc_input_event_t popped;

    pstvnc_input_queue_init(&queue);

    event = product_action_event(PSTVNC_PRODUCT_ACTION_NONE);
    CHECK(!pstvnc_input_queue_push(&queue, &event));

    event = product_action_event((pstvnc_product_action_t)99);
    CHECK(!pstvnc_input_queue_push(&queue, &event));

    event = product_action_event(PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(pstvnc_input_queue_push(&queue, &event));
    CHECK(pstvnc_input_queue_event_count(&queue) == 1u);
    CHECK(pstvnc_input_queue_pop(&queue, &popped));
    CHECK(popped.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(
        popped.payload.product_action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
}

static void test_all_event_families_preserve_fifo_order(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t events[4];
    pstvnc_input_event_t popped;
    unsigned int i;

    memset(events, 0, sizeof(events));

    events[0].type = PSTVNC_INPUT_EVENT_CONTROLLER_STATE;
    events[0].payload.controller_state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1;

    events[1].type = PSTVNC_INPUT_EVENT_MOUSE_UPDATE;
    events[1].payload.mouse_update.cursor_x = 101u;
    events[1].payload.mouse_update.pointer_changed = 1;

    events[2].type = PSTVNC_INPUT_EVENT_KEYBOARD_TAP;
    events[2].payload.keyboard_tap.keysym = 0x0000ff09u;

    events[3] =
        product_action_event(PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    pstvnc_input_queue_init(&queue);

    for (i = 0u; i < 4u; i++)
        CHECK(pstvnc_input_queue_push(&queue, &events[i]));

    for (i = 0u; i < 4u; i++) {
        CHECK(pstvnc_input_queue_pop(&queue, &popped));
        CHECK(popped.type == events[i].type);
    }
}

static void test_product_action_survives_ring_wrap(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    pstvnc_input_event_t popped;
    unsigned int i;
    int saw_product_action = 0;

    pstvnc_input_queue_init(&queue);

    for (i = 0u; i < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY; i++) {
        event = controller_event(i);
        CHECK(pstvnc_input_queue_push(&queue, &event));
    }

    for (i = 0u; i < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u; i++)
        CHECK(pstvnc_input_queue_pop(&queue, &popped));

    for (i = 0u; i < PSTVNC_INPUT_EVENT_QUEUE_CAPACITY / 2u; i++) {
        if (i == 17u)
            event =
                product_action_event(
                    PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
        else
            event = controller_event(1000u + i);

        CHECK(pstvnc_input_queue_push(&queue, &event));
    }

    CHECK(
        pstvnc_input_queue_event_count(&queue) ==
        PSTVNC_INPUT_EVENT_QUEUE_CAPACITY);

    while (pstvnc_input_queue_pop(&queue, &popped)) {
        if (popped.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION) {
            CHECK(!saw_product_action);
            CHECK(
                popped.payload.product_action ==
                PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
            saw_product_action = 1;
        }
    }

    CHECK(saw_product_action);
}

static void test_product_action_discard_is_exact(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t event;
    pstvnc_input_event_t popped;

    pstvnc_input_queue_init(&queue);

    event = product_action_event(PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(pstvnc_input_queue_push(&queue, &event));
    CHECK(pstvnc_input_queue_event_count(&queue) == 1u);

    pstvnc_input_queue_discard_all(&queue);

    CHECK(pstvnc_input_queue_event_count(&queue) == 0u);
    CHECK(!pstvnc_input_queue_pop(&queue, &popped));

    /* Existing event families remain accepted immediately after discard. */
    event = controller_event(PSTVNC_CONTROLLER_BUTTON_R1);
    CHECK(pstvnc_input_queue_push(&queue, &event));
    CHECK(pstvnc_input_queue_pop(&queue, &popped));
    CHECK(popped.type == PSTVNC_INPUT_EVENT_CONTROLLER_STATE);
}

int main(void)
{
    test_product_action_validates_and_round_trips();
    test_all_event_families_preserve_fifo_order();
    test_product_action_survives_ring_wrap();
    test_product_action_discard_is_exact();

    if (failures != 0) {
        fprintf(stderr, "PRODUCT_ACTION_EVENT_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("PRODUCT_ACTION_EVENT_TEST=PASS");
    return 0;
}
