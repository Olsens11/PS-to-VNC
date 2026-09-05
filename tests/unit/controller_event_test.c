#include <stdio.h>
#include <string.h>

#include "input.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, \
                "controller_event_test:%d: CHECK failed: %s\n", \
                __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    pstvnc_input_queue_t queue;
    pstvnc_input_event_t controller_event;
    pstvnc_input_event_t mouse_event;
    pstvnc_input_event_t popped;

    pstvnc_input_queue_init(&queue);

    memset(&controller_event, 0, sizeof(controller_event));

    controller_event.type =
        PSTVNC_INPUT_EVENT_CONTROLLER_STATE;

    controller_event.payload.controller_state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_SELECT;

    controller_event.payload.controller_state.connection_epoch_started =
        1;

    memset(&mouse_event, 0, sizeof(mouse_event));

    mouse_event.type =
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE;

    mouse_event.payload.mouse_update.pointer_changed = 1;
    mouse_event.payload.mouse_update.cursor_x = 123;
    mouse_event.payload.mouse_update.cursor_y = 234;

    CHECK(
        pstvnc_input_queue_push(
            &queue,
            &controller_event));

    CHECK(
        pstvnc_input_queue_push(
            &queue,
            &mouse_event));

    CHECK(
        pstvnc_input_queue_event_count(
            &queue) == 2);

    memset(&popped, 0, sizeof(popped));

    CHECK(
        pstvnc_input_queue_pop(
            &queue,
            &popped));

    CHECK(
        popped.type ==
        PSTVNC_INPUT_EVENT_CONTROLLER_STATE);

    CHECK(
        popped.payload.controller_state.connection_epoch_started ==
        1);

    CHECK(
        popped.payload.controller_state.buttons_down ==
        PSTVNC_CONTROLLER_BUTTON_SELECT);

    memset(&popped, 0, sizeof(popped));

    CHECK(
        pstvnc_input_queue_pop(
            &queue,
            &popped));

    CHECK(
        popped.type ==
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE);

    CHECK(
        popped.payload.mouse_update.cursor_x == 123);

    CHECK(
        popped.payload.mouse_update.cursor_y == 234);

    puts("CONTROLLER_EVENT_TEST=PASS");
    return 0;
}
