/*
 * File synopsis:
 * Exercises application-owned lifecycle, semantic mouse-to-RFB routing,
 * successful-publication sequencing, responsive receive scheduling, and
 * cleanup ordering without linking PS2 platform implementations.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md application coordinator and
 * input/controller concurrency model; GitHub Issue #38.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "diagnostics.h"
#include "display.h"
#include "framebuffer.h"
#include "input_runtime.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb.h"
#include "rfb_session.h"

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(                                                    \
                stderr,                                                 \
                "FAIL %s:%d: %s\n",                                    \
                __FILE__,                                               \
                __LINE__,                                               \
                #expr);                                                 \
            failures++;                                                 \
        }                                                               \
    } while (0)

typedef enum event_id {
    EV_PREPARE_IOP,
    EV_NETWORK_INIT,
    EV_WAIT_LINK,
    EV_DIAG_INIT,
    EV_DIAG_SEND,
    EV_FB_INIT,
    EV_FB_GEOMETRY,
    EV_GRAPHICS_INIT,
    EV_CONNECT,
    EV_SESSION_INIT,
    EV_SESSION_START,
    EV_INITIAL_FRAME,
    EV_DISPLAY_PREPARE,
    EV_PRESENT,
    EV_INPUT_INIT,
    EV_POINTER_SEND,
    EV_INPUT_START,
    EV_REQUEST_UPDATE,
    EV_INPUT_POP,
    EV_TRY_RECEIVE,
    EV_IDLE_DELAY,
    EV_INPUT_SHUTDOWN,
    EV_CLOSE,
    EV_GRAPHICS_SHUTDOWN,
    EV_DIAG_SHUTDOWN
} event_id_t;

typedef struct pointer_call {
    uint8_t button_mask;
    uint16_t x;
    uint16_t y;
} pointer_call_t;

static event_id_t events[256];
static size_t event_count;

static char diagnostics[16][32];
static size_t diagnostic_count;

static int prepare_iop_result;
static int network_init_result;
static int wait_link_result;
static int diagnostics_init_result;
static int graphics_init_result;
static int connect_result;
static int session_start_result;
static int initial_frame_result;
static int display_prepare_result;
static int present_result;

static int input_init_result;
static int input_start_result;
static int input_shutdown_result;
static pstvnc_input_runtime_error_t input_worker_error;

static int idle_delay_result;
static size_t idle_delay_calls;

static int request_results[16];
static size_t request_result_count;
static size_t request_calls;

static pstvnc_rfb_session_receive_result_t
    try_receive_results[16];

static int try_receive_valid[16];
static int try_receive_dirty[16];
static size_t try_receive_result_count;
static size_t try_receive_calls;

static pstvnc_input_event_t input_events[16];
static size_t input_event_count;
static size_t input_event_index;

static int pointer_send_results[32];
static size_t pointer_send_result_count;
static pointer_call_t pointer_calls[32];
static size_t pointer_call_count;

static int closed_socket;

static void log_event(event_id_t event)
{
    CHECK(
        event_count <
        sizeof(events) / sizeof(events[0]));

    if (event_count <
        sizeof(events) / sizeof(events[0]))
        events[event_count++] = event;
}

static void reset_script(void)
{
    memset(events, 0, sizeof(events));
    memset(diagnostics, 0, sizeof(diagnostics));
    memset(request_results, 0, sizeof(request_results));
    memset(try_receive_results, 0, sizeof(try_receive_results));
    memset(try_receive_valid, 0, sizeof(try_receive_valid));
    memset(try_receive_dirty, 0, sizeof(try_receive_dirty));
    memset(input_events, 0, sizeof(input_events));
    memset(pointer_send_results, 0, sizeof(pointer_send_results));
    memset(pointer_calls, 0, sizeof(pointer_calls));

    event_count = 0;
    diagnostic_count = 0;

    prepare_iop_result = 0;
    network_init_result = 0;
    wait_link_result = 0;
    diagnostics_init_result = 0;
    graphics_init_result = 0;
    connect_result = 7;
    session_start_result = 1;
    initial_frame_result = 1;
    display_prepare_result = 1;
    present_result = 0;

    input_init_result = 0;
    input_start_result = 0;
    input_shutdown_result = 0;
    input_worker_error =
        PSTVNC_INPUT_RUNTIME_ERROR_NONE;

    idle_delay_result = 0;
    idle_delay_calls = 0;

    request_results[0] = 0;
    request_result_count = 1;
    request_calls = 0;

    try_receive_result_count = 0;
    try_receive_calls = 0;

    input_event_count = 0;
    input_event_index = 0;

    pointer_send_result_count = 0;
    pointer_call_count = 0;

    closed_socket = -1;
}

static int event_index(event_id_t event)
{
    size_t i;

    for (i = 0; i < event_count; i++) {
        if (events[i] == event)
            return (int)i;
    }

    return -1;
}

static size_t event_occurrences(event_id_t event)
{
    size_t i;
    size_t count = 0;

    for (i = 0; i < event_count; i++) {
        if (events[i] == event)
            count++;
    }

    return count;
}

static int diagnostic_index(const char *text)
{
    size_t i;

    for (i = 0; i < diagnostic_count; i++) {
        if (strcmp(diagnostics[i], text) == 0)
            return (int)i;
    }

    return -1;
}

int pstvnc_ps2_system_prepare_iop(void)
{
    log_event(EV_PREPARE_IOP);
    return prepare_iop_result;
}

int pstvnc_ps2_system_delay_us(
    unsigned int microseconds)
{
    log_event(EV_IDLE_DELAY);
    idle_delay_calls++;

    CHECK(microseconds == 1000u);
    return idle_delay_result;
}

void pstvnc_ps2_system_exit_to_menu(void)
{
}

int pstvnc_ps2_network_init(void)
{
    log_event(EV_NETWORK_INIT);
    return network_init_result;
}

int pstvnc_ps2_network_wait_link(void)
{
    log_event(EV_WAIT_LINK);
    return wait_link_result;
}

int pstvnc_ps2_network_connect_vnc(void)
{
    log_event(EV_CONNECT);
    return connect_result;
}

void pstvnc_ps2_network_close(int socket_fd)
{
    log_event(EV_CLOSE);
    closed_socket = socket_fd;
}

int pstvnc_diagnostics_init(void)
{
    log_event(EV_DIAG_INIT);
    return diagnostics_init_result;
}

int pstvnc_diagnostics_is_ready(void)
{
    return diagnostics_init_result == 0;
}

int pstvnc_diagnostics_send(
    const void *data,
    size_t length)
{
    size_t take;

    log_event(EV_DIAG_SEND);

    CHECK(
        diagnostic_count <
        sizeof(diagnostics) / sizeof(diagnostics[0]));

    if (diagnostic_count >=
        sizeof(diagnostics) / sizeof(diagnostics[0]))
        return -1;

    take = length;

    if (take >= sizeof(diagnostics[diagnostic_count]))
        take =
            sizeof(diagnostics[diagnostic_count]) - 1u;

    memcpy(
        diagnostics[diagnostic_count],
        data,
        take);

    diagnostics[diagnostic_count][take] = '\0';
    diagnostic_count++;

    return (int)length;
}

void pstvnc_diagnostics_shutdown(void)
{
    log_event(EV_DIAG_SHUTDOWN);
}

int pstvnc_framebuffer_init(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels,
    size_t pixel_capacity)
{
    log_event(EV_FB_INIT);

    memset(framebuffer, 0, sizeof(*framebuffer));
    framebuffer->pixels = pixels;
    framebuffer->pixel_capacity = pixel_capacity;

    return 1;
}

int pstvnc_framebuffer_set_geometry(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t width,
    uint16_t height)
{
    log_event(EV_FB_GEOMETRY);

    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->valid = 0;
    framebuffer->dirty = 0;

    return 1;
}

int pstvnc_ps2_graphics_init(void)
{
    log_event(EV_GRAPHICS_INIT);
    return graphics_init_result;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *pixels,
    size_t pixel_count)
{
    (void)pixels;
    (void)pixel_count;

    log_event(EV_PRESENT);
    return present_result;
}

void pstvnc_ps2_graphics_shutdown(void)
{
    log_event(EV_GRAPHICS_SHUTDOWN);
}

void pstvnc_rfb_session_init(
    pstvnc_rfb_session_t *session)
{
    log_event(EV_SESSION_INIT);
    memset(session, 0, sizeof(*session));
}

int pstvnc_rfb_session_start(
    pstvnc_rfb_session_t *session,
    int socket_fd,
    uint16_t expected_width,
    uint16_t expected_height)
{
    log_event(EV_SESSION_START);

    if (session_start_result) {
        session->socket_fd = socket_fd;
        session->state = PSTVNC_RFB_SESSION_READY;
        session->server_init.width = expected_width;
        session->server_init.height = expected_height;
    }

    return session_start_result;
}

int pstvnc_rfb_session_receive_initial_frame(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    (void)session;

    log_event(EV_INITIAL_FRAME);

    if (initial_frame_result) {
        framebuffer->valid = 1;
        framebuffer->dirty = 1;
    }

    return initial_frame_result;
}

int pstvnc_rfb_session_send_pointer_event(
    pstvnc_rfb_session_t *session,
    uint8_t button_mask,
    uint16_t x,
    uint16_t y)
{
    int result = 1;

    (void)session;
    log_event(EV_POINTER_SEND);

    CHECK(
        pointer_call_count <
        sizeof(pointer_calls) /
        sizeof(pointer_calls[0]));

    if (pointer_call_count <
        sizeof(pointer_calls) /
        sizeof(pointer_calls[0])) {

        pointer_calls[pointer_call_count].button_mask =
            button_mask;

        pointer_calls[pointer_call_count].x = x;
        pointer_calls[pointer_call_count].y = y;
    }

    if (pointer_call_count <
        pointer_send_result_count)
        result =
            pointer_send_results[pointer_call_count];

    pointer_call_count++;
    return result;
}

int pstvnc_rfb_session_request_update(
    pstvnc_rfb_session_t *session,
    int incremental)
{
    int result = 0;

    (void)session;

    CHECK(incremental != 0);
    log_event(EV_REQUEST_UPDATE);

    if (request_calls < request_result_count)
        result = request_results[request_calls];

    request_calls++;
    return result;
}

pstvnc_rfb_session_receive_result_t
pstvnc_rfb_session_try_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    pstvnc_rfb_session_receive_result_t result =
        PSTVNC_RFB_SESSION_RECEIVE_FAILED;

    size_t index = try_receive_calls;

    (void)session;
    log_event(EV_TRY_RECEIVE);

    if (index < try_receive_result_count) {
        result = try_receive_results[index];

        if (result ==
            PSTVNC_RFB_SESSION_RECEIVE_UPDATE) {

            framebuffer->valid =
                try_receive_valid[index];

            framebuffer->dirty =
                try_receive_dirty[index];
        }
    }

    try_receive_calls++;
    return result;
}

int pstvnc_display_prepare_gs16(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels,
    size_t pixel_capacity)
{
    (void)framebuffer;
    (void)pixels;
    (void)pixel_capacity;

    log_event(EV_DISPLAY_PREPARE);
    return display_prepare_result;
}

int pstvnc_input_runtime_init(
    pstvnc_input_runtime_t *runtime,
    unsigned int width,
    unsigned int height,
    int port,
    int slot)
{
    log_event(EV_INPUT_INIT);

    CHECK(width == PSTVNC_DISPLAY_WIDTH);
    CHECK(height == PSTVNC_DISPLAY_HEIGHT);
    CHECK(port == 0);
    CHECK(slot == 0);

    if (input_init_result < 0)
        return input_init_result;

    memset(runtime, 0, sizeof(*runtime));
    runtime->initialized = 1;
    runtime->controller_thread_id = -1;

    return 0;
}

int pstvnc_input_runtime_start(
    pstvnc_input_runtime_t *runtime)
{
    log_event(EV_INPUT_START);

    if (input_start_result < 0)
        return input_start_result;

    runtime->controller_thread_started = 1;
    return 0;
}

int pstvnc_input_runtime_shutdown(
    pstvnc_input_runtime_t *runtime)
{
    log_event(EV_INPUT_SHUTDOWN);

    runtime->initialized = 0;
    runtime->controller_thread_started = 0;

    return input_shutdown_result;
}

int pstvnc_input_runtime_pop_event(
    pstvnc_input_runtime_t *runtime,
    pstvnc_input_event_t *event)
{
    (void)runtime;

    log_event(EV_INPUT_POP);

    if (input_event_index >= input_event_count)
        return 0;

    *event = input_events[input_event_index++];
    return 1;
}

pstvnc_input_runtime_error_t
pstvnc_input_runtime_last_error(
    const pstvnc_input_runtime_t *runtime)
{
    (void)runtime;
    return input_worker_error;
}

static void check_core_owned_cleanup(void)
{
    CHECK(diagnostic_index("PSTVNC_STAGE FATAL") >= 0);
    CHECK(closed_socket == 7);
    CHECK(event_occurrences(EV_CLOSE) == 1);
    CHECK(event_occurrences(EV_GRAPHICS_SHUTDOWN) == 1);
    CHECK(event_occurrences(EV_DIAG_SHUTDOWN) == 1);

    CHECK(
        event_index(EV_CLOSE) <
        event_index(EV_GRAPHICS_SHUTDOWN));

    CHECK(
        event_index(EV_GRAPHICS_SHUTDOWN) <
        event_index(EV_DIAG_SHUTDOWN));
}

static void check_input_cleanup_precedes_socket_close(void)
{
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 1);

    CHECK(
        event_index(EV_INPUT_SHUTDOWN) <
        event_index(EV_CLOSE));
}

static void test_iop_failure_owns_nothing(void)
{
    reset_script();
    prepare_iop_result = -1;

    CHECK(pstvnc_app_run() == -1);
    CHECK(event_count == 1);
    CHECK(events[0] == EV_PREPARE_IOP);
    CHECK(diagnostic_count == 0);
}

static void test_connect_failure_cleans_only_acquired_resources(void)
{
    reset_script();
    connect_result = -1;

    CHECK(pstvnc_app_run() == -1);
    CHECK(event_occurrences(EV_CLOSE) == 0);
    CHECK(event_occurrences(EV_GRAPHICS_SHUTDOWN) == 0);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 0);
    CHECK(event_occurrences(EV_DIAG_SHUTDOWN) == 1);
    CHECK(diagnostic_count == 1);

    CHECK(
        strcmp(
            diagnostics[0],
            "PSTVNC_STAGE FATAL") == 0);
}

static void test_session_failure_cleanup_order(void)
{
    reset_script();
    session_start_result = 0;

    CHECK(pstvnc_app_run() == -1);

    check_core_owned_cleanup();

    CHECK(
        event_index(EV_SESSION_START) <
        event_index(EV_CLOSE));

    CHECK(event_occurrences(EV_INITIAL_FRAME) == 0);
    CHECK(event_occurrences(EV_PRESENT) == 0);
    CHECK(event_occurrences(EV_INPUT_INIT) == 0);
}

static void test_clean_update_does_not_represent(void)
{
    reset_script();

    request_results[0] = 1;
    request_results[1] = 0;
    request_result_count = 2;

    try_receive_results[0] =
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE;

    try_receive_valid[0] = 1;
    try_receive_dirty[0] = 0;
    try_receive_result_count = 1;

    CHECK(pstvnc_app_run() == -1);

    CHECK(request_calls == 2);
    CHECK(try_receive_calls == 1);

    CHECK(event_occurrences(EV_PRESENT) == 1);
    CHECK(event_occurrences(EV_DISPLAY_PREPARE) == 1);

    CHECK(pointer_call_count == 1);
    CHECK(pointer_calls[0].button_mask == 0);
    CHECK(
        pointer_calls[0].x ==
        PSTVNC_DISPLAY_WIDTH / 2u);
    CHECK(
        pointer_calls[0].y ==
        PSTVNC_DISPLAY_HEIGHT / 2u);

    CHECK(
        diagnostic_index(
            "PSTVNC_STAGE DESKTOP_READY") >= 0);

    CHECK(
        diagnostic_index(
            "PSTVNC_STAGE INPUT_READY") >= 0);

    CHECK(diagnostic_count == 5);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_invalid_live_frame_fails_before_presentation(void)
{
    reset_script();

    request_results[0] = 1;
    request_result_count = 1;

    try_receive_results[0] =
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE;

    try_receive_valid[0] = 0;
    try_receive_dirty[0] = 1;
    try_receive_result_count = 1;

    CHECK(pstvnc_app_run() == -1);

    CHECK(request_calls == 1);
    CHECK(try_receive_calls == 1);
    CHECK(event_occurrences(EV_PRESENT) == 1);
    CHECK(event_occurrences(EV_DISPLAY_PREPARE) == 1);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_diagnostics_are_best_effort(void)
{
    reset_script();

    diagnostics_init_result = -1;
    session_start_result = 0;

    CHECK(pstvnc_app_run() == -1);

    CHECK(diagnostic_count == 0);
    CHECK(event_occurrences(EV_DIAG_SHUTDOWN) == 0);
    CHECK(event_occurrences(EV_CLOSE) == 1);
    CHECK(event_occurrences(EV_GRAPHICS_SHUTDOWN) == 1);
}

static void test_semantic_pointer_and_wheel_mapping(void)
{
    pstvnc_mouse_update_t *mouse_update;

    reset_script();

    request_results[0] = 1;
    request_result_count = 1;

    try_receive_results[0] =
        PSTVNC_RFB_SESSION_RECEIVE_FAILED;

    try_receive_result_count = 1;

    input_event_count = 2;

    input_events[0].type =
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE;

    mouse_update =
        &input_events[0].payload.mouse_update;

    mouse_update->pointer_changed = 1;
    mouse_update->cursor_x = 100;
    mouse_update->cursor_y = 50;
    mouse_update->click_buttons =
        PSTVNC_MOUSE_BUTTON_RCLICK;

    mouse_update->wheel_direction =
        PSTVNC_MOUSE_WHEEL_NONE;

    input_events[1].type =
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE;

    mouse_update =
        &input_events[1].payload.mouse_update;

    mouse_update->pointer_changed = 0;
    mouse_update->cursor_x = 100;
    mouse_update->cursor_y = 50;
    mouse_update->click_buttons =
        PSTVNC_MOUSE_BUTTON_RCLICK;

    mouse_update->wheel_direction =
        PSTVNC_MOUSE_WHEEL_UP;

    CHECK(pstvnc_app_run() == -1);

    CHECK(pointer_call_count == 4);

    CHECK(pointer_calls[0].button_mask == 0);

    CHECK(
        pointer_calls[0].x ==
        PSTVNC_DISPLAY_WIDTH / 2u);

    CHECK(
        pointer_calls[0].y ==
        PSTVNC_DISPLAY_HEIGHT / 2u);

    CHECK(
        pointer_calls[1].button_mask ==
        PSTVNC_RFB_POINTER_BUTTON_RIGHT);

    CHECK(pointer_calls[1].x == 100);
    CHECK(pointer_calls[1].y == 50);

    CHECK(
        pointer_calls[2].button_mask ==
        (
            PSTVNC_RFB_POINTER_BUTTON_RIGHT |
            PSTVNC_RFB_POINTER_WHEEL_UP
        ));

    CHECK(pointer_calls[2].x == 100);
    CHECK(pointer_calls[2].y == 50);

    CHECK(
        pointer_calls[3].button_mask ==
        PSTVNC_RFB_POINTER_BUTTON_RIGHT);

    CHECK(pointer_calls[3].x == 100);
    CHECK(pointer_calls[3].y == 50);

    CHECK(try_receive_calls == 1);
    CHECK(idle_delay_calls == 0);

    CHECK(
        event_index(EV_INPUT_INIT) <
        event_index(EV_POINTER_SEND));

    CHECK(
        event_index(EV_POINTER_SEND) <
        event_index(EV_INPUT_START));

    CHECK(
        event_index(EV_INPUT_START) <
        event_index(EV_REQUEST_UPDATE));

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_idle_receive_yields_without_busy_spin(void)
{
    reset_script();

    request_results[0] = 1;
    request_result_count = 1;

    try_receive_results[0] =
        PSTVNC_RFB_SESSION_RECEIVE_IDLE;

    try_receive_results[1] =
        PSTVNC_RFB_SESSION_RECEIVE_FAILED;

    try_receive_result_count = 2;

    CHECK(pstvnc_app_run() == -1);

    CHECK(try_receive_calls == 2);
    CHECK(idle_delay_calls == 1);
    CHECK(event_occurrences(EV_IDLE_DELAY) == 1);

    CHECK(
        event_index(EV_TRY_RECEIVE) <
        event_index(EV_IDLE_DELAY));

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_input_init_failure_does_not_claim_runtime(void)
{
    reset_script();

    input_init_result = -1;

    CHECK(pstvnc_app_run() == -1);

    CHECK(event_occurrences(EV_INPUT_INIT) == 1);
    CHECK(event_occurrences(EV_POINTER_SEND) == 0);
    CHECK(event_occurrences(EV_INPUT_START) == 0);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 0);

    check_core_owned_cleanup();
}

static void test_initial_pointer_failure_shuts_initialized_runtime(void)
{
    reset_script();

    pointer_send_results[0] = 0;
    pointer_send_result_count = 1;

    CHECK(pstvnc_app_run() == -1);

    CHECK(event_occurrences(EV_INPUT_INIT) == 1);
    CHECK(pointer_call_count == 1);
    CHECK(event_occurrences(EV_INPUT_START) == 0);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_input_start_failure_shuts_initialized_runtime(void)
{
    reset_script();

    input_start_result = -1;

    CHECK(pstvnc_app_run() == -1);

    CHECK(event_occurrences(EV_INPUT_INIT) == 1);
    CHECK(pointer_call_count == 1);
    CHECK(event_occurrences(EV_INPUT_START) == 1);

    CHECK(
        diagnostic_index(
            "PSTVNC_STAGE INPUT_READY") < 0);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_worker_error_fails_before_receive_service(void)
{
    reset_script();

    request_results[0] = 1;
    request_result_count = 1;

    input_worker_error =
        PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_FULL;

    CHECK(pstvnc_app_run() == -1);

    CHECK(request_calls == 1);
    CHECK(event_occurrences(EV_INPUT_POP) == 0);
    CHECK(try_receive_calls == 0);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

static void test_wheel_release_failure_stops_before_receive(void)
{
    pstvnc_mouse_update_t *mouse_update;

    reset_script();

    request_results[0] = 1;
    request_result_count = 1;

    input_event_count = 1;

    input_events[0].type =
        PSTVNC_INPUT_EVENT_MOUSE_UPDATE;

    mouse_update =
        &input_events[0].payload.mouse_update;

    mouse_update->pointer_changed = 0;
    mouse_update->cursor_x = 352;
    mouse_update->cursor_y = 231;
    mouse_update->click_buttons = 0;
    mouse_update->wheel_direction =
        PSTVNC_MOUSE_WHEEL_DOWN;

    pointer_send_results[0] = 1;
    pointer_send_results[1] = 1;
    pointer_send_results[2] = 0;
    pointer_send_result_count = 3;

    CHECK(pstvnc_app_run() == -1);

    CHECK(pointer_call_count == 3);

    CHECK(
        pointer_calls[1].button_mask ==
        PSTVNC_RFB_POINTER_WHEEL_DOWN);

    CHECK(pointer_calls[2].button_mask == 0);

    /*
     * The failed release terminates application publication immediately. The
     * server-receive path is never entered with an uncertain wheel pulse.
     */
    CHECK(try_receive_calls == 0);

    check_input_cleanup_precedes_socket_close();
    check_core_owned_cleanup();
}

int main(void)
{
    test_iop_failure_owns_nothing();
    test_connect_failure_cleans_only_acquired_resources();
    test_session_failure_cleanup_order();
    test_clean_update_does_not_represent();
    test_invalid_live_frame_fails_before_presentation();
    test_diagnostics_are_best_effort();

    test_semantic_pointer_and_wheel_mapping();
    test_idle_receive_yields_without_busy_spin();
    test_input_init_failure_does_not_claim_runtime();
    test_initial_pointer_failure_shuts_initialized_runtime();
    test_input_start_failure_shuts_initialized_runtime();
    test_worker_error_fails_before_receive_service();
    test_wheel_release_failure_stops_before_receive();

    if (failures != 0) {
        fprintf(
            stderr,
            "app_test: %d failure(s)\n",
            failures);

        return 1;
    }

    puts("app_test: PASS");
    return 0;
}
