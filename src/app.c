/*
 * File synopsis:
 * Runs the application coordinator: ordered startup, semantic controller-input
 * routing, main-thread RFB publication, responsive framebuffer service,
 * presentation, failure policy, and owned cleanup. Subsystems retain their own
 * private mechanisms.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Complete application
 * coordinator"; docs/CLEAN_ARCHITECTURE.md, "Application coordinator", "Live
 * loop", and input/controller concurrency model; GitHub Issue #38.
 */

#include "app.h"

#include <stdint.h>

#include "diagnostics.h"
#include "display.h"
#include "framebuffer.h"
#include "input_runtime.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb_session.h"

#define PSTVNC_APP_CONTROLLER_PORT       0
#define PSTVNC_APP_CONTROLLER_SLOT       0
#define PSTVNC_APP_IDLE_POLL_DELAY_US    1000u

/*
 * These buffers have deliberately different authority. remote_pixels backs the
 * one CPU-side image trusted to represent the complete VNC desktop. gs_pixels
 * is disposable presentation data with the GS alpha bit added; display
 * conversion must never modify remote authority in place. The 128-byte
 * alignment satisfies the PS2 DMA/cache-facing presentation path.
 */
static uint16_t remote_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));

/*
 * Application-owned pointer state that is known to have completed publication
 * to the remote RFB peer.
 *
 * click_buttons deliberately remains in semantic mouse vocabulary because this
 * state is the authority a later libpad handoff/rebase must feed back into the
 * input interpreter. Native RFB button bits exist only during serialization.
 */
typedef struct app_published_pointer_state {
    unsigned int cursor_x;
    unsigned int cursor_y;
    unsigned char click_buttons;
} app_published_pointer_state_t;

static void send_diagnostic_literal(
    int diagnostics_ready,
    const char *text,
    size_t length)
{
    if (diagnostics_ready)
        (void)pstvnc_diagnostics_send(text, length);
}

static int map_semantic_clicks_to_rfb_buttons(
    unsigned char semantic_clicks,
    uint8_t *rfb_buttons)
{
    uint8_t mapped = 0;

    if (rfb_buttons == NULL ||
        (semantic_clicks &
         (unsigned char)~PSTVNC_MOUSE_BUTTON_MASK) != 0)
        return 0;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_LCLICK)
        mapped |= PSTVNC_RFB_POINTER_BUTTON_LEFT;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_RCLICK)
        mapped |= PSTVNC_RFB_POINTER_BUTTON_RIGHT;

    *rfb_buttons = mapped;
    return 1;
}

static int map_wheel_direction_to_rfb_button(
    pstvnc_mouse_wheel_direction_t direction,
    uint8_t *rfb_button)
{
    if (rfb_button == NULL)
        return 0;

    switch (direction) {
        case PSTVNC_MOUSE_WHEEL_NONE:
            *rfb_button = 0;
            return 1;

        case PSTVNC_MOUSE_WHEEL_UP:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_UP;
            return 1;

        case PSTVNC_MOUSE_WHEEL_DOWN:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_DOWN;
            return 1;

        case PSTVNC_MOUSE_WHEEL_LEFT:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_LEFT;
            return 1;

        case PSTVNC_MOUSE_WHEEL_RIGHT:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_RIGHT;
            return 1;

        default:
            return 0;
    }
}

static int publish_semantic_mouse_update(
    pstvnc_rfb_session_t *session,
    const pstvnc_mouse_update_t *mouse_update,
    app_published_pointer_state_t *published_pointer)
{
    uint8_t ordinary_rfb_buttons;
    uint8_t wheel_rfb_button;

    if (session == NULL ||
        mouse_update == NULL ||
        published_pointer == NULL ||
        mouse_update->cursor_x > UINT16_MAX ||
        mouse_update->cursor_y > UINT16_MAX)
        return 0;

    if (!map_semantic_clicks_to_rfb_buttons(
            mouse_update->click_buttons,
            &ordinary_rfb_buttons))
        return 0;

    if (!map_wheel_direction_to_rfb_button(
            mouse_update->wheel_direction,
            &wheel_rfb_button))
        return 0;

    /*
     * Pointer movement/button state is one durable remote state publication.
     * Update application authority only after the exact six-byte message has
     * completed successfully.
     */
    if (mouse_update->pointer_changed) {
        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        published_pointer->cursor_x =
            mouse_update->cursor_x;

        published_pointer->cursor_y =
            mouse_update->cursor_y;

        published_pointer->click_buttons =
            mouse_update->click_buttons;
    }

    if (wheel_rfb_button != 0) {
        /*
         * RFB wheel motion is a momentary button pulse at the current pointer
         * position. Preserve any ordinary held buttons through both messages.
         *
         * If the press succeeds but release fails, the RFB session itself
         * fails closed. Do not falsely advance the durable published-pointer
         * authority through that uncertain partial pulse.
         */
        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                (uint8_t)(
                    ordinary_rfb_buttons |
                    wheel_rfb_button),
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        published_pointer->cursor_x =
            mouse_update->cursor_x;

        published_pointer->cursor_y =
            mouse_update->cursor_y;

        published_pointer->click_buttons =
            mouse_update->click_buttons;
    }

    return 1;
}

static int service_semantic_input_events(
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer)
{
    if (pstvnc_input_runtime_last_error(input_runtime) !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return 0;

    for (;;) {
        pstvnc_input_event_t event;
        int pop_result =
            pstvnc_input_runtime_pop_event(
                input_runtime,
                &event);

        if (pop_result < 0)
            return 0;

        if (pop_result == 0)
            break;

        if (event.type != PSTVNC_INPUT_EVENT_MOUSE_UPDATE)
            return 0;

        if (!publish_semantic_mouse_update(
                session,
                &event.payload.mouse_update,
                published_pointer))
            return 0;
    }

    /*
     * A producer failure can race with the final queue-empty observation.
     * Re-check after draining so queue exhaustion/pad failure cannot be hidden
     * behind a superficially complete set of older events.
     */
    return
        pstvnc_input_runtime_last_error(input_runtime) ==
        PSTVNC_INPUT_RUNTIME_ERROR_NONE;
}

int pstvnc_app_run(void)
{
    static const char net_ready[] = "PSTVNC_STAGE NET_READY";
    static const char gs_ready[] = "PSTVNC_STAGE GS_READY";
    static const char desktop_ready[] = "PSTVNC_STAGE DESKTOP_READY";
    static const char input_ready[] = "PSTVNC_STAGE INPUT_READY";
    static const char fatal[] = "PSTVNC_STAGE FATAL";

    /*
     * The runtime contains both a producer-thread stack and the bounded event
     * queue. Static function-local storage keeps that ownership out of broad
     * global scope and, critically, keeps the storage alive if cooperative
     * shutdown cannot prove worker dormancy on a fatal path.
     */
    static pstvnc_input_runtime_t input_runtime;

    pstvnc_framebuffer_t framebuffer;
    pstvnc_rfb_session_t session;
    app_published_pointer_state_t published_pointer;

    int socket_fd = -1;
    int graphics_ready = 0;
    int diagnostics_ready = 0;
    int input_runtime_ready = 0;

    /*
     * The coordinator owns ordering and failure policy. Each subsystem owns its
     * mechanism, but none may independently advance the product to READY or
     * decide how a fatal startup failure converges.
     */
    if (pstvnc_ps2_system_prepare_iop() < 0)
        goto fail;

    if (pstvnc_ps2_network_init() < 0)
        goto fail;

    if (pstvnc_ps2_network_wait_link() < 0)
        goto fail;

    if (pstvnc_diagnostics_init() == 0)
        diagnostics_ready = 1;

    /*
     * Connect to the Pi before startup diagnostics. The frozen PS2IP stack can
     * replace pending UDP packets while ARP is unresolved.
     * See Issue #7 startup-order hardware qualification evidence.
     */
    socket_fd = pstvnc_ps2_network_connect_vnc();
    if (socket_fd < 0)
        goto fail;

    send_diagnostic_literal(
        diagnostics_ready,
        net_ready,
        sizeof(net_ready) - 1u);

    if (!pstvnc_framebuffer_init(
            &framebuffer,
            remote_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        goto fail;

    if (!pstvnc_framebuffer_set_geometry(
            &framebuffer,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        goto fail;

    if (pstvnc_ps2_graphics_init() < 0)
        goto fail;

    graphics_ready = 1;

    send_diagnostic_literal(
        diagnostics_ready,
        gs_ready,
        sizeof(gs_ready) - 1u);

    pstvnc_rfb_session_init(&session);

    if (!pstvnc_rfb_session_start(
            &session,
            socket_fd,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        goto fail;

    /*
     * Session startup has requested a non-incremental desktop. The receive call
     * publishes framebuffer.valid only after proving exact coverage of every
     * pixel; presenting before that point could expose unwritten or stale data.
     */
    if (!pstvnc_rfb_session_receive_initial_frame(
            &session,
            &framebuffer))
        goto fail;

    if (!pstvnc_display_prepare_gs16(
            &framebuffer,
            gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        goto fail;

    if (pstvnc_ps2_graphics_present(
            gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT) < 0)
        goto fail;

    send_diagnostic_literal(
        diagnostics_ready,
        desktop_ready,
        sizeof(desktop_ready) - 1u);

    /*
     * Input starts only after the RFB session is READY and a complete
     * authoritative desktop is already visible. The local mouse interpreter
     * begins at the logical desktop center. Publish that same neutral state
     * before starting the worker so application/main has exact remote pointer
     * authority from the first controller-produced event onward.
     */
    if (pstvnc_input_runtime_init(
            &input_runtime,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT,
            PSTVNC_APP_CONTROLLER_PORT,
            PSTVNC_APP_CONTROLLER_SLOT) < 0)
        goto fail;

    input_runtime_ready = 1;

    published_pointer.cursor_x =
        PSTVNC_DISPLAY_WIDTH / 2u;

    published_pointer.cursor_y =
        PSTVNC_DISPLAY_HEIGHT / 2u;

    published_pointer.click_buttons = 0;

    if (!pstvnc_rfb_session_send_pointer_event(
            &session,
            0,
            (uint16_t)published_pointer.cursor_x,
            (uint16_t)published_pointer.cursor_y))
        goto fail;

    if (pstvnc_input_runtime_start(
            &input_runtime) < 0)
        goto fail;

    send_diagnostic_literal(
        diagnostics_ready,
        input_ready,
        sizeof(input_ready) - 1u);

    /*
     * Keep exactly one ordinary incremental framebuffer request outstanding.
     * While it is legitimately idle, the safe-boundary receive service returns
     * control so this same application/main thread can drain semantic input and
     * serialize PointerEvents on the sole owned RFB socket.
     */
    if (!pstvnc_rfb_session_request_update(
            &session,
            1))
        goto fail;

    for (;;) {
        pstvnc_rfb_session_receive_result_t
            receive_result;

        if (!service_semantic_input_events(
                &input_runtime,
                &session,
                &published_pointer))
            goto fail;

        receive_result =
            pstvnc_rfb_session_try_receive_update(
                &session,
                &framebuffer);

        if (receive_result ==
            PSTVNC_RFB_SESSION_RECEIVE_FAILED)
            goto fail;

        if (receive_result ==
            PSTVNC_RFB_SESSION_RECEIVE_IDLE) {

            /*
             * The controller worker continues at its independent ~60-Hz
             * cadence. Yield briefly instead of turning an idle desktop into a
             * maximum-speed nonblocking recv() spin.
             */
            if (pstvnc_ps2_system_delay_us(
                    PSTVNC_APP_IDLE_POLL_DELAY_US) < 0)
                goto fail;

            continue;
        }

        if (receive_result !=
            PSTVNC_RFB_SESSION_RECEIVE_UPDATE)
            goto fail;

        if (!framebuffer.valid)
            goto fail;

        if (framebuffer.dirty) {
            if (!pstvnc_display_prepare_gs16(
                    &framebuffer,
                    gs_pixels,
                    PSTVNC_DISPLAY_PIXEL_COUNT))
                goto fail;

            if (pstvnc_ps2_graphics_present(
                    gs_pixels,
                    PSTVNC_DISPLAY_PIXEL_COUNT) < 0)
                goto fail;
        }

        if (!pstvnc_rfb_session_request_update(
                &session,
                1))
            goto fail;
    }

fail:
    /*
     * Every fatal path converges here. Stop controller production before
     * releasing the network/display resources used by application routing.
     *
     * Runtime storage is static, so an unproven cooperative shutdown cannot
     * leave a live worker executing on reclaimed application-stack storage.
     */
    send_diagnostic_literal(
        diagnostics_ready,
        fatal,
        sizeof(fatal) - 1u);

    if (input_runtime_ready)
        (void)pstvnc_input_runtime_shutdown(
            &input_runtime);

    if (socket_fd >= 0)
        pstvnc_ps2_network_close(socket_fd);

    if (graphics_ready)
        pstvnc_ps2_graphics_shutdown();

    if (diagnostics_ready)
        pstvnc_diagnostics_shutdown();

    return -1;
}
