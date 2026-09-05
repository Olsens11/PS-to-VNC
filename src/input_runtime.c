/*
 * File synopsis:
 * Implements the PS2 controller/input runtime owner.
 *
 * The dedicated EE worker polls one direct-libpad endpoint at the historically
 * qualified approximately-60-Hz cadence, maps the currently earned Issue #38
 * controller gestures into mouse-domain facts, advances the pure mouse state
 * machine, and publishes complete typed semantic events into a semaphore-
 * protected FIFO.
 *
 * The worker never serializes RFB, never touches the VNC socket, never mutates
 * GS/display state, and never executes product actions. Those effects remain
 * application/main-thread responsibilities.
 *
 * The implementation also owns two hard physical-continuity boundaries:
 *
 *   1. disconnect/reacquisition:
 *      stale controller-derived motion is discarded and any logically held
 *      remote mouse button is released once;
 *
 *   2. explicit libpad handoff:
 *      old queued input is discarded, any held remote button is represented by
 *      one semantic release, and the worker acknowledges only when it can stop
 *      all libpad access until ownership is returned.
 *
 * Context:
 *   docs/CLEAN_ARCHITECTURE.md;
 *   docs/audit/SOURCE_RESPONSIBILITY_MAP.md;
 *   docs/adr/0002-use-libpad-directly-reference-opl.md;
 *   GitHub Issue #38.
 */

#include "input_runtime.h"

#include <delaythread.h>
#include <kernel.h>
#include <stddef.h>
#include <string.h>

/*
 * Historical B4A controller production used approximately one poll every
 * 16.667 ms. The pure mouse response constants were reconstructed against that
 * cadence, so this runtime preserves it instead of silently changing pointer
 * acceleration/repeat behavior.
 */
#define INPUT_RUNTIME_POLL_DELAY_US 16667

/*
 * The worker historically ran at EE priority 64 with an 8192-byte stack.
 * Retaining that scheduling point avoids mixing controller reconstruction with
 * an unrelated scheduler experiment.
 */
#define INPUT_RUNTIME_THREAD_PRIORITY 64

/*
 * Bounded waits used by application/main when it must observe a controller
 * worker lifecycle transition rather than merely request one.
 *
 * One second is intentionally generous relative to the approximately-60-Hz
 * worker cadence. It is a failure-detection window, not a normal input timing
 * parameter.
 *
 * The same bound is used for:
 *
 *   - proving cooperative thread dormancy before DeleteThread();
 *   - proving libpad-handoff acknowledgement withdrawal before a release
 *     operation reports completion.
 *
 * Neither path force-terminates or guesses past an unproven ownership state.
 */
#define INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US 1000
#define INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS   1000u

static int input_runtime_mouse_update_is_meaningful(
    const pstvnc_mouse_update_t *mouse_update)
{
    return
        mouse_update->pointer_changed ||
        mouse_update->wheel_direction != PSTVNC_MOUSE_WHEEL_NONE;
}

static void input_runtime_record_worker_error(
    pstvnc_input_runtime_t *runtime,
    pstvnc_input_runtime_error_t error)
{
    /*
     * Preserve the first failure. Later cleanup errors should not erase the
     * event that originally caused semantic input production to stop.
     */
    if (runtime->worker_error == PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        runtime->worker_error = error;
}

static uint16_t input_runtime_mouse_directions_from_pad(
    uint16_t buttons_down)
{
    uint16_t directions = 0;

    if (buttons_down & PAD_UP)
        directions |= PSTVNC_MOUSE_DIRECTION_UP;

    if (buttons_down & PAD_DOWN)
        directions |= PSTVNC_MOUSE_DIRECTION_DOWN;

    if (buttons_down & PAD_LEFT)
        directions |= PSTVNC_MOUSE_DIRECTION_LEFT;

    if (buttons_down & PAD_RIGHT)
        directions |= PSTVNC_MOUSE_DIRECTION_RIGHT;

    return directions;
}

static int input_runtime_left_stick_is_available(
    const pstvnc_pad_t *pad)
{
    size_t required_bytes;

    /*
     * padRead() is variable-length. Do not infer analog availability merely
     * from controller type or from zero-filled bytes in the cached structure.
     *
     * ljoy_v is the last left-stick byte consumed by the mouse path; requiring
     * the packet to reach through that field proves both native left-stick
     * coordinates came from the current PADMAN sample.
     */
    required_bytes =
        offsetof(struct padButtonStatus, ljoy_v) +
        sizeof(pad->buttons.ljoy_v);

    return (size_t)pad->sample_length >= required_bytes;
}

static void input_runtime_build_mouse_input(
    const pstvnc_pad_t *pad,
    pstvnc_mouse_input_t *mouse_input)
{
    memset(mouse_input, 0, sizeof(*mouse_input));

    mouse_input->dpad_mode =
        (pad->buttons_down & PAD_TRIANGLE)
        ? PSTVNC_MOUSE_DPAD_WHEEL
        : PSTVNC_MOUSE_DPAD_POINTER;

    mouse_input->dpad_directions =
        input_runtime_mouse_directions_from_pad(
            pad->buttons_down);

    if (input_runtime_left_stick_is_available(pad)) {
        mouse_input->stick_available = 1;
        mouse_input->stick_x = pad->buttons.ljoy_h;
        mouse_input->stick_y = pad->buttons.ljoy_v;
    }

    /*
     * L3 is intentionally edge-triggered because the clean mouse policy treats
     * it as clicking a mouse wheel to toggle persistent analog-wheel mode.
     */
    mouse_input->wheel_click_pressed =
        (pad->buttons_pressed & PAD_L3) != 0;

    if (pad->buttons_down & PAD_CROSS)
        mouse_input->click_buttons |= PSTVNC_MOUSE_BUTTON_LCLICK;

    if (pad->buttons_down & PAD_CIRCLE)
        mouse_input->click_buttons |= PSTVNC_MOUSE_BUTTON_RCLICK;
}

static int input_runtime_publish_mouse_update(
    pstvnc_input_runtime_t *runtime,
    const pstvnc_mouse_update_t *mouse_update)
{
    pstvnc_input_event_t event;
    int pushed;

    if (!input_runtime_mouse_update_is_meaningful(mouse_update))
        return 1;

    memset(&event, 0, sizeof(event));

    event.type = PSTVNC_INPUT_EVENT_MOUSE_UPDATE;
    event.payload.mouse_update = *mouse_update;

    if (WaitSema(runtime->event_queue_sema_id) < 0) {
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_WAIT);
        return 0;
    }

    pushed =
        pstvnc_input_queue_push(
            &runtime->event_queue,
            &event);

    if (SignalSema(runtime->event_queue_sema_id) < 0) {
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_SIGNAL);
        return 0;
    }

    if (!pushed) {
        /*
         * Ordinary semantic input is never silently overwritten or dropped.
         * Stop production so application/main can treat queue exhaustion as an
         * explicit failure instead of receiving a deceptively complete stream.
         */
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_FULL);
        return 0;
    }

    return 1;
}

static int input_runtime_process_pad_sample(
    pstvnc_input_runtime_t *runtime)
{
    pstvnc_mouse_input_t mouse_input;
    pstvnc_mouse_update_t mouse_update;

    input_runtime_build_mouse_input(
        &runtime->pad,
        &mouse_input);

    if (!pstvnc_mouse_update(
            &runtime->mouse,
            &mouse_input,
            &mouse_update)) {

        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_MOUSE_UPDATE);
        return 0;
    }

    if (!input_runtime_publish_mouse_update(
            runtime,
            &mouse_update))
        return 0;

    runtime->physical_sample_active = 1;
    return 1;
}

static int input_runtime_handle_physical_loss(
    pstvnc_input_runtime_t *runtime)
{
    pstvnc_mouse_input_t neutral_input;
    pstvnc_mouse_update_t mouse_update;

    if (!runtime->physical_sample_active)
        return 1;

    /*
     * A physical-continuity loss invalidates fractional motion, repeat state,
     * and persistent analog-wheel mode immediately.
     *
     * Cursor position remains authoritative. The following neutral mouse sample
     * changes click state to zero; when a button was remotely held this creates
     * exactly one semantic pointer-state release at the current cursor.
     */
    pstvnc_mouse_reset_derived(&runtime->mouse);

    memset(&neutral_input, 0, sizeof(neutral_input));

    if (!pstvnc_mouse_update(
            &runtime->mouse,
            &neutral_input,
            &mouse_update)) {

        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_MOUSE_UPDATE);
        return 0;
    }

    runtime->physical_sample_active = 0;

    return input_runtime_publish_mouse_update(
        runtime,
        &mouse_update);
}

static int input_runtime_enter_pad_handoff(
    pstvnc_input_runtime_t *runtime)
{
    /*
     * The queue is the boundary between locally interpreted input and work the
     * application may still publish. Throw away every pre-handoff event before
     * acknowledgement so none can emerge later as a new remote action.
     */
    if (WaitSema(runtime->event_queue_sema_id) < 0) {
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_WAIT);
        return 0;
    }

    pstvnc_input_queue_discard_all(
        &runtime->event_queue);

    if (SignalSema(runtime->event_queue_sema_id) < 0) {
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_SIGNAL);
        return 0;
    }

    /*
     * Derived movement/repeat history is definitely local controller history
     * and cannot survive the handoff.
     *
     * Cursor and click state are intentionally NOT guessed here. They may have
     * advanced farther than application/main successfully published. The
     * acknowledged owner will explicitly rebase them from its authoritative
     * published pointer state before this worker can resume.
     */
    pstvnc_mouse_reset_derived(
        &runtime->mouse);

    pstvnc_pad_invalidate_connection_epoch(
        &runtime->pad);

    runtime->physical_sample_active = 0;
    runtime->handoff_published_state_rebased = 0;

    return 1;
}

static void input_runtime_controller_thread(
    void *argument)
{
    pstvnc_input_runtime_t *runtime =
        (pstvnc_input_runtime_t *)argument;

    for (;;) {
        int pad_poll_result;

        if (runtime->stop_requested)
            break;

        /*
         * A padSetMainMode() command is asynchronous. Do not acknowledge a
         * handoff while this runtime still owns one such command: the temporary
         * owner must never inherit a PADMAN operation that was started by the
         * controller worker but not yet observed complete.
         *
         * While a mode request remains pending, normal bounded polling advances
         * it. Once no command is pending, acknowledgement occurs immediately
         * before the next possible libpad access.
         */
        if (runtime->pad_handoff_requested &&
            !runtime->pad.dualshock_mode_request_pending) {

            if (!input_runtime_enter_pad_handoff(runtime))
                break;

            runtime->pad_handoff_acknowledged = 1;

            while (runtime->pad_handoff_requested &&
                   !runtime->stop_requested) {

                if (DelayThread(INPUT_RUNTIME_POLL_DELAY_US) < 0) {
                    input_runtime_record_worker_error(
                        runtime,
                        PSTVNC_INPUT_RUNTIME_ERROR_THREAD_DELAY);

                    runtime->stop_requested = 1;
                    break;
                }
            }

            runtime->pad_handoff_acknowledged = 0;

            if (runtime->stop_requested)
                break;

            /*
             * The temporary owner has returned libpad. Its observations and
             * any mode it used are not controller-worker continuity.
             *
             * The caller's handoff contract requires a clean physical release
             * before returning ownership, so the next sample is allowed to
             * establish a new baseline without pre-handoff edges.
             */
            pstvnc_pad_invalidate_connection_epoch(
                &runtime->pad);

            pstvnc_mouse_reset_derived(
                &runtime->mouse);

            runtime->physical_sample_active = 0;
            runtime->handoff_published_state_rebased = 0;
            continue;
        }

        pad_poll_result =
            pstvnc_pad_poll(&runtime->pad);

        if (pad_poll_result < 0) {
            input_runtime_record_worker_error(
                runtime,
                PSTVNC_INPUT_RUNTIME_ERROR_PAD_POLL);
            break;
        }

        if (pad_poll_result > 0) {
            if (!input_runtime_process_pad_sample(runtime))
                break;

        } else if (
            runtime->physical_sample_active &&
            !runtime->pad.history_valid) {

            /*
             * A zero-return poll with history still valid merely means there
             * was no new trustworthy sample this tick. A transition from an
             * active sample epoch to history_valid == 0 is the stronger fact:
             * physical continuity was explicitly revoked by the pad owner.
             */
            if (!input_runtime_handle_physical_loss(runtime))
                break;
        }

        if (DelayThread(INPUT_RUNTIME_POLL_DELAY_US) < 0) {
            input_runtime_record_worker_error(
                runtime,
                PSTVNC_INPUT_RUNTIME_ERROR_THREAD_DELAY);
            break;
        }
    }

    runtime->pad_handoff_acknowledged = 0;

    ExitThread();
}

static int input_runtime_stop_controller_thread(
    pstvnc_input_runtime_t *runtime)
{
    unsigned int wait_step;

    if (!runtime->controller_thread_started)
        return 0;

    runtime->stop_requested = 1;

    for (wait_step = 0;
         wait_step < INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS;
         wait_step++) {

        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));

        if (ReferThreadStatus(
                runtime->controller_thread_id,
                &status) < 0)
            return -1;

        if (status.status == THS_DORMANT) {
            if (DeleteThread(
                    runtime->controller_thread_id) < 0)
                return -1;

            runtime->controller_thread_id = -1;
            runtime->controller_thread_started = 0;
            runtime->pad_handoff_requested = 0;
            runtime->pad_handoff_acknowledged = 0;

            return 0;
        }

        if (DelayThread(INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US) < 0)
            return -1;
    }

    /*
     * Do not force-terminate a worker whose state we cannot prove.
     * Leaving ownership intact is safer than freeing its stack, semaphore, or
     * libpad resources while it may still execute.
     */
    return -1;
}

int pstvnc_input_runtime_init(
    pstvnc_input_runtime_t *runtime,
    unsigned int width,
    unsigned int height,
    int port,
    int slot)
{
    ee_sema_t semaphore;

    if (runtime == NULL)
        return -1;

    memset(runtime, 0, sizeof(*runtime));

    runtime->event_queue_sema_id = -1;
    runtime->controller_thread_id = -1;

    if (!pstvnc_mouse_init(
            &runtime->mouse,
            width,
            height))
        return -1;

    pstvnc_input_queue_init(
        &runtime->event_queue);

    if (pstvnc_pad_init() < 0)
        return -1;

    if (pstvnc_pad_open(
            &runtime->pad,
            port,
            slot) < 0) {

        pstvnc_pad_shutdown();
        return -1;
    }

    memset(&semaphore, 0, sizeof(semaphore));

    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    runtime->event_queue_sema_id =
        CreateSema(&semaphore);

    if (runtime->event_queue_sema_id < 0) {
        pstvnc_pad_close(&runtime->pad);
        pstvnc_pad_shutdown();
        return -1;
    }

    runtime->initialized = 1;
    return 0;
}

int pstvnc_input_runtime_start(
    pstvnc_input_runtime_t *runtime)
{
    ee_thread_t thread;

    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->controller_thread_started ||
        runtime->controller_thread_id >= 0)
        return -1;

    runtime->stop_requested = 0;
    runtime->pad_handoff_requested = 0;
    runtime->pad_handoff_acknowledged = 0;
    runtime->handoff_published_state_rebased = 0;
    runtime->worker_error = PSTVNC_INPUT_RUNTIME_ERROR_NONE;
    runtime->physical_sample_active = 0;

    memset(&thread, 0, sizeof(thread));

    thread.func =
        (void *)input_runtime_controller_thread;

    thread.stack =
        (void *)runtime->controller_thread_stack;

    thread.stack_size =
        (int)sizeof(runtime->controller_thread_stack);

    thread.gp_reg = &_gp;
    thread.initial_priority = INPUT_RUNTIME_THREAD_PRIORITY;
    thread.attr = 0;
    thread.option = 0;

    runtime->controller_thread_id =
        CreateThread(&thread);

    if (runtime->controller_thread_id < 0)
        return -1;

    if (StartThread(
            runtime->controller_thread_id,
            runtime) < 0) {

        (void)DeleteThread(
            runtime->controller_thread_id);

        runtime->controller_thread_id = -1;
        return -1;
    }

    runtime->controller_thread_started = 1;
    return 0;
}

int pstvnc_input_runtime_shutdown(
    pstvnc_input_runtime_t *runtime)
{
    int result = 0;

    if (runtime == NULL)
        return -1;

    if (!runtime->initialized)
        return 0;

    if (input_runtime_stop_controller_thread(runtime) < 0)
        return -1;

    pstvnc_pad_close(
        &runtime->pad);

    if (runtime->event_queue_sema_id >= 0) {
        if (DeleteSema(
                runtime->event_queue_sema_id) < 0)
            result = -1;

        runtime->event_queue_sema_id = -1;
    }

    pstvnc_pad_shutdown();

    runtime->initialized = 0;
    return result;
}

int pstvnc_input_runtime_pop_event(
    pstvnc_input_runtime_t *runtime,
    pstvnc_input_event_t *event)
{
    int popped;

    if (runtime == NULL ||
        event == NULL ||
        !runtime->initialized ||
        runtime->event_queue_sema_id < 0)
        return -1;

    if (WaitSema(runtime->event_queue_sema_id) < 0)
        return -1;

    popped =
        pstvnc_input_queue_pop(
            &runtime->event_queue,
            event);

    if (SignalSema(runtime->event_queue_sema_id) < 0)
        return -1;

    return popped ? 1 : 0;
}

int pstvnc_input_runtime_discard_events(
    pstvnc_input_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->event_queue_sema_id < 0)
        return -1;

    if (WaitSema(runtime->event_queue_sema_id) < 0)
        return -1;

    pstvnc_input_queue_discard_all(
        &runtime->event_queue);

    if (SignalSema(runtime->event_queue_sema_id) < 0)
        return -1;

    return 0;
}

int pstvnc_input_runtime_event_count(
    pstvnc_input_runtime_t *runtime,
    unsigned int *event_count)
{
    if (runtime == NULL ||
        event_count == NULL ||
        !runtime->initialized ||
        runtime->event_queue_sema_id < 0)
        return -1;

    if (WaitSema(runtime->event_queue_sema_id) < 0)
        return -1;

    *event_count =
        pstvnc_input_queue_event_count(
            &runtime->event_queue);

    if (SignalSema(runtime->event_queue_sema_id) < 0)
        return -1;

    return 0;
}

pstvnc_input_runtime_error_t
pstvnc_input_runtime_last_error(
    const pstvnc_input_runtime_t *runtime)
{
    if (runtime == NULL)
        return PSTVNC_INPUT_RUNTIME_ERROR_NONE;

    return runtime->worker_error;
}

int pstvnc_input_runtime_request_pad_handoff(
    pstvnc_input_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started)
        return -1;

    if (runtime->worker_error !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return -1;

    /*
     * Each request/acknowledgement pair is one ownership epoch.
     *
     * Do not permit a second request while either side still reports state from
     * the previous epoch. Otherwise a lingering acknowledgement could be
     * mistaken for acknowledgement of a newly issued request.
     */
    if (runtime->pad_handoff_requested ||
        runtime->pad_handoff_acknowledged)
        return -1;

    runtime->handoff_published_state_rebased = 0;
    runtime->pad_handoff_requested = 1;
    return 0;
}

int pstvnc_input_runtime_pad_handoff_acknowledged(
    const pstvnc_input_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started)
        return 0;

    return runtime->pad_handoff_acknowledged != 0;
}

int pstvnc_input_runtime_rebase_published_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started ||
        !runtime->pad_handoff_requested ||
        !runtime->pad_handoff_acknowledged)
        return -1;

    /*
     * A pad handoff must not leave a remote button logically held while local
     * controller ownership belongs elsewhere. Application/main owns the actual
     * RFB release and then supplies the resulting published neutral state here.
     */
    if (published_click_buttons != 0)
        return -1;

    if (!pstvnc_mouse_rebase_published_state(
            &runtime->mouse,
            published_cursor_x,
            published_cursor_y,
            published_click_buttons))
        return -1;

    runtime->handoff_published_state_rebased = 1;
    return 0;
}

int pstvnc_input_runtime_release_pad_handoff(
    pstvnc_input_runtime_t *runtime)
{
    unsigned int wait_step;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started ||
        !runtime->pad_handoff_requested ||
        !runtime->pad_handoff_acknowledged ||
        !runtime->handoff_published_state_rebased)
        return -1;

    /*
     * Application/main has finished its temporary libpad ownership. Withdraw
     * the request, but do not report success merely because the write occurred.
     *
     * The worker owns pad_handoff_acknowledged. Observing it return to zero is
     * the proof that the previous ownership epoch is closed and its ack cannot
     * be mistaken for a future request.
     */
    runtime->pad_handoff_requested = 0;

    for (wait_step = 0;
         wait_step < INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS;
         wait_step++) {

        if (!runtime->pad_handoff_acknowledged) {
            if (runtime->worker_error !=
                PSTVNC_INPUT_RUNTIME_ERROR_NONE)
                return -1;

            return 0;
        }

        if (DelayThread(
                INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US) < 0)
            return -1;
    }

    /*
     * Fail closed. The request remains withdrawn, but the caller is explicitly
     * told that acknowledgement withdrawal could not be proven.
     */
    return -1;
}
