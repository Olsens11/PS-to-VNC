#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "diagnostics.h"
#include "display.h"
#include "framebuffer.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb_session.h"

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
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
    EV_REQUEST_UPDATE,
    EV_RECEIVE_UPDATE,
    EV_CLOSE,
    EV_GRAPHICS_SHUTDOWN,
    EV_DIAG_SHUTDOWN
} event_id_t;

static event_id_t events[128];
static size_t event_count;
static char diagnostics[8][32];
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
static int request_results[4];
static size_t request_result_count;
static size_t request_calls;
static int receive_results[4];
static int receive_valid[4];
static int receive_dirty[4];
static size_t receive_result_count;
static size_t receive_calls;
static int closed_socket;

static void log_event(event_id_t event)
{
    CHECK(event_count < sizeof(events) / sizeof(events[0]));
    if (event_count < sizeof(events) / sizeof(events[0]))
        events[event_count++] = event;
}

static void reset_script(void)
{
    memset(events, 0, sizeof(events));
    memset(diagnostics, 0, sizeof(diagnostics));
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

    memset(request_results, 0, sizeof(request_results));
    request_results[0] = 0;
    request_result_count = 1;
    request_calls = 0;

    memset(receive_results, 0, sizeof(receive_results));
    memset(receive_valid, 0, sizeof(receive_valid));
    memset(receive_dirty, 0, sizeof(receive_dirty));
    receive_result_count = 0;
    receive_calls = 0;
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

int pstvnc_ps2_system_prepare_iop(void)
{
    log_event(EV_PREPARE_IOP);
    return prepare_iop_result;
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

int pstvnc_diagnostics_send(const void *data, size_t length)
{
    size_t take;

    log_event(EV_DIAG_SEND);
    CHECK(diagnostic_count < sizeof(diagnostics) / sizeof(diagnostics[0]));
    if (diagnostic_count >= sizeof(diagnostics) / sizeof(diagnostics[0]))
        return -1;

    take = length;
    if (take >= sizeof(diagnostics[diagnostic_count]))
        take = sizeof(diagnostics[diagnostic_count]) - 1u;

    memcpy(diagnostics[diagnostic_count], data, take);
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
    const uint16_t *gs_pixels,
    size_t pixel_count)
{
    (void)gs_pixels;
    (void)pixel_count;
    log_event(EV_PRESENT);
    return present_result;
}

void pstvnc_ps2_graphics_shutdown(void)
{
    log_event(EV_GRAPHICS_SHUTDOWN);
}

void pstvnc_rfb_session_init(pstvnc_rfb_session_t *session)
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
    (void)session;
    (void)socket_fd;
    (void)expected_width;
    (void)expected_height;
    log_event(EV_SESSION_START);
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

int pstvnc_rfb_session_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    size_t index = receive_calls;
    int result = 0;

    (void)session;
    log_event(EV_RECEIVE_UPDATE);
    if (index < receive_result_count) {
        result = receive_results[index];
        framebuffer->valid = receive_valid[index];
        framebuffer->dirty = receive_dirty[index];
    }
    receive_calls++;
    return result;
}

int pstvnc_display_prepare_gs16(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t *gs_pixels,
    size_t gs_capacity_pixels)
{
    (void)framebuffer;
    (void)gs_pixels;
    (void)gs_capacity_pixels;
    log_event(EV_DISPLAY_PREPARE);
    return display_prepare_result;
}

static void check_owned_cleanup(void)
{
    int fatal = -1;
    size_t i;

    for (i = 0; i < diagnostic_count; i++) {
        if (strcmp(diagnostics[i], "PSTVNC_STAGE FATAL") == 0)
            fatal = (int)i;
    }

    CHECK(fatal >= 0);
    CHECK(closed_socket == 7);
    CHECK(event_occurrences(EV_CLOSE) == 1);
    CHECK(event_occurrences(EV_GRAPHICS_SHUTDOWN) == 1);
    CHECK(event_occurrences(EV_DIAG_SHUTDOWN) == 1);
    CHECK(event_index(EV_CLOSE) < event_index(EV_GRAPHICS_SHUTDOWN));
    CHECK(event_index(EV_GRAPHICS_SHUTDOWN) < event_index(EV_DIAG_SHUTDOWN));
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
    CHECK(event_occurrences(EV_DIAG_SHUTDOWN) == 1);
    CHECK(diagnostic_count == 1);
    CHECK(strcmp(diagnostics[0], "PSTVNC_STAGE FATAL") == 0);
}

static void test_session_failure_cleanup_order(void)
{
    reset_script();
    session_start_result = 0;

    CHECK(pstvnc_app_run() == -1);
    check_owned_cleanup();
    CHECK(event_index(EV_SESSION_START) < event_index(EV_CLOSE));
    CHECK(event_occurrences(EV_INITIAL_FRAME) == 0);
    CHECK(event_occurrences(EV_PRESENT) == 0);
}

static void test_clean_update_does_not_represent(void)
{
    reset_script();
    request_results[0] = 1;
    request_results[1] = 0;
    request_result_count = 2;
    receive_results[0] = 1;
    receive_valid[0] = 1;
    receive_dirty[0] = 0;
    receive_result_count = 1;

    CHECK(pstvnc_app_run() == -1);
    CHECK(request_calls == 2);
    CHECK(receive_calls == 1);
    CHECK(event_occurrences(EV_PRESENT) == 1);
    CHECK(event_occurrences(EV_DISPLAY_PREPARE) == 1);
    CHECK(diagnostic_count == 4);
    CHECK(strcmp(diagnostics[2], "PSTVNC_STAGE DESKTOP_READY") == 0);
    check_owned_cleanup();
}

static void test_invalid_live_frame_fails_before_presentation(void)
{
    reset_script();
    request_results[0] = 1;
    request_result_count = 1;
    receive_results[0] = 1;
    receive_valid[0] = 0;
    receive_dirty[0] = 1;
    receive_result_count = 1;

    CHECK(pstvnc_app_run() == -1);
    CHECK(request_calls == 1);
    CHECK(receive_calls == 1);
    CHECK(event_occurrences(EV_PRESENT) == 1);
    CHECK(event_occurrences(EV_DISPLAY_PREPARE) == 1);
    check_owned_cleanup();
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

int main(void)
{
    test_iop_failure_owns_nothing();
    test_connect_failure_cleans_only_acquired_resources();
    test_session_failure_cleanup_order();
    test_clean_update_does_not_represent();
    test_invalid_live_frame_fails_before_presentation();
    test_diagnostics_are_best_effort();

    if (failures != 0) {
        fprintf(stderr, "app_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("app_test: PASS");
    return 0;
}
