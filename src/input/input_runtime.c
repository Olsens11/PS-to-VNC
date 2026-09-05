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

    /*
     * Packet length proves only that the native fields were copied. The
     * joystick bytes are meaningful only when this current libpad sample
     * identifies itself as DualShock analog data.
     *
     * In particular, a disconnect/reacquisition transition must never turn
     * zero-filled non-analog joystick bytes into maximum up-left movement.
     */
    return
        (size_t)pad->sample_length >= required_bytes &&
        (pad->buttons.mode >> 4) == PAD_TYPE_DUALSHOCK;
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

static uint16_t input_runtime_map_native_buttons(
    uint16_t native_buttons)
{
    uint16_t project_buttons = 0;

    /*
     * Native controller vocabulary terminates here. Higher layers receive only
     * project-owned physical facts.
     */
    if (native_buttons & PAD_SELECT)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_SELECT;
    if (native_buttons & PAD_L3)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_L3;
    if (native_buttons & PAD_R3)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_R3;
    if (native_buttons & PAD_START)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_START;

    if (native_buttons & PAD_UP)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_UP;
    if (native_buttons & PAD_RIGHT)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_RIGHT;
    if (native_buttons & PAD_DOWN)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_DOWN;
    if (native_buttons & PAD_LEFT)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_LEFT;

    if (native_buttons & PAD_L2)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_L2;
    if (native_buttons & PAD_R2)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_R2;
    if (native_buttons & PAD_L1)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_L1;
    if (native_buttons & PAD_R1)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_R1;

    if (native_buttons & PAD_TRIANGLE)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_TRIANGLE;
    if (native_buttons & PAD_CIRCLE)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_CIRCLE;
    if (native_buttons & PAD_CROSS)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_CROSS;
    if (native_buttons & PAD_SQUARE)
        project_buttons |= PSTVNC_CONTROLLER_BUTTON_SQUARE;

    return project_buttons;
}

static int input_runtime_publish_controller_state(
    pstvnc_input_runtime_t *runtime,
    const pstvnc_controller_state_t *controller_state)
{
    pstvnc_input_event_t event;
    int pushed;

    if (runtime == NULL ||
        controller_state == NULL)
        return 0;

    /*
     * Ordinary desktop operation only requires transition-bearing controller
     * facts plus the authoritative first sample of a connection epoch.
     *
     * While mouse interpretation is suspended, publish every trustworthy
     * physical state. The suspension entry boundary intentionally discards
     * queued pre-boundary work; complete post-boundary state guarantees that a
     * release cannot disappear in that discard and strand local quarantine.
     */
    if (!runtime->mouse_interpretation_suspended &&
        !controller_state->connection_epoch_started &&
        controller_state->buttons_pressed == 0 &&
        controller_state->buttons_released == 0)
        return 1;

    memset(&event, 0, sizeof(event));

    event.type =
        PSTVNC_INPUT_EVENT_CONTROLLER_STATE;

    event.payload.controller_state =
        *controller_state;

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
        input_runtime_record_worker_error(
            runtime,
            PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_FULL);
        return 0;
    }

    return 1;
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
    pstvnc_controller_state_t controller_state;
    pstvnc_mouse_input_t mouse_input;
    pstvnc_mouse_update_t mouse_update;

    /*
     * Publish trustworthy controller state before any mouse interpretation from
     * the same physical sample.
     *
     * Main can therefore consume a foreground-transition fact, establish the
     * synchronized mouse-suspension boundary, and cause any still-queued
     * same-sample pointer work to be discarded before remote publication.
     */
    memset(&controller_state, 0, sizeof(controller_state));

    controller_state.buttons_down =
        input_runtime_map_native_buttons(
            runtime->pad.buttons_down);

    controller_state.buttons_pressed =
        input_runtime_map_native_buttons(
            runtime->pad.buttons_pressed);

    controller_state.buttons_released =
        input_runtime_map_native_buttons(
            runtime->pad.buttons_released);

    controller_state.connection_epoch_started =
        runtime->physical_sample_active ? 0 : 1;

    if (!input_runtime_publish_controller_state(
            runtime,
            &controller_state))
        return 0;

    /*
     * Physical acquisition remains live during a local-controller foreground.
     * Suspension applies only to mouse interpretation.
     *
     * A trustworthy sample still proves physical continuity, but none of its
     * D-pad, stick, click, wheel, or repeat meaning may advance pstvnc_mouse_t.
     */
    if (runtime->mouse_interpretation_suspended) {
        runtime->physical_sample_active = 1;
        return 1;
    }

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
     * Application/main owns the frozen mouse interpreter during an acknowledged
     * suspension epoch. The worker must therefore not mutate pstvnc_mouse_t
     * concurrently with the application's authoritative pointer rebase.
     *
     * A physical-continuity loss is still a true hard boundary. Record that
     * fact immediately and revoke the physical sample epoch; the worker will
     * apply the ordinary hard mouse reset before suspension acknowledgement is
     * withdrawn.
     */
    if (runtime->mouse_interpretation_suspended) {
        runtime->physical_sample_active = 0;
        runtime->suspended_physical_continuity_lost = 1;
        return 1;
    }

    /*
     * Outside suspension there is no competing mouse owner, so physical loss
     * applies the existing hard reset immediately.
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

static int input_runtime_enter_mouse_interpretation_suspension(
    pstvnc_input_runtime_t *runtime)
{
    /*
     * Queued mouse interpretation may be newer than the pointer state
     * application/main successfully published. No pre-boundary semantic work
     * may emerge after another foreground takes controller ownership.
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
     * Preserve durable cursor/button state and the user's persistent analog-
     * wheel mode while invalidating transient movement, repeat, fractional,
     * direction, and wheel-repeat history immediately.
     *
     * Physical controller continuity remains live during this suspension, so
     * foreground ownership alone is not a reason to revoke the selected mode.
     */
    pstvnc_mouse_reset_transient_history(
        &runtime->mouse);

    /*
     * A controller event capable of opening local foreground normally arrives
     * inside an active physical sample epoch. Preserve that fact explicitly so
     * even an abnormal suspension request without continuity fails toward the
     * hard-reset side of the contract.
     */
    runtime->suspended_physical_continuity_lost =
        runtime->physical_sample_active ? 0 : 1;

    runtime->suspended_mouse_state_rebased = 0;
    return 1;
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
        /*
         * Mouse interpretation and physical controller acquisition are
         * separate ownership questions.
         *
         * Suspension clears pre-boundary semantic work and mouse-derived
         * history, then acknowledges. The worker continues through this loop
         * and reaches pstvnc_pad_poll() normally.
         */
        if (runtime->mouse_interpretation_suspend_requested) {
            if (!runtime->mouse_interpretation_suspended) {
                if (!input_runtime_enter_mouse_interpretation_suspension(
                        runtime))
                    break;

                runtime->mouse_interpretation_suspended = 1;
            }

        } else if (runtime->mouse_interpretation_suspended) {
            /*
             * Application/main has withdrawn the request only after completing
             * its authoritative suspended-state rebase and release quarantine.
             *
             * If physical continuity broke anywhere during that suspension
             * epoch, apply the true hard mouse boundary now, before dropping the
             * acknowledgement that allows interpretation to resume.
             */
            if (runtime->suspended_physical_continuity_lost) {
                pstvnc_mouse_reset_derived(
                    &runtime->mouse);
            }

            runtime->suspended_physical_continuity_lost = 0;
            runtime->mouse_interpretation_suspended = 0;
            runtime->suspended_mouse_state_rebased = 0;
        }

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

    runtime->suspended_physical_continuity_lost = 0;
    runtime->mouse_interpretation_suspended = 0;
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

            runtime->mouse_interpretation_suspend_requested = 0;
            runtime->mouse_interpretation_suspended = 0;
            runtime->suspended_mouse_state_rebased = 0;
            runtime->suspended_physical_continuity_lost = 0;

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

    runtime->mouse_interpretation_suspend_requested = 0;
    runtime->mouse_interpretation_suspended = 0;
    runtime->suspended_mouse_state_rebased = 0;
    runtime->suspended_physical_continuity_lost = 0;

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

int pstvnc_input_runtime_suspend_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    unsigned int wait_step;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started)
        return -1;

    if (runtime->worker_error !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return -1;

    /*
     * Mouse suspension and temporary libpad handoff are distinct, mutually
     * exclusive ownership epochs.
     */
    if (runtime->mouse_interpretation_suspend_requested ||
        runtime->mouse_interpretation_suspended ||
        runtime->pad_handoff_requested ||
        runtime->pad_handoff_acknowledged)
        return -1;

    runtime->suspended_mouse_state_rebased = 0;
    runtime->mouse_interpretation_suspend_requested = 1;

    for (wait_step = 0;
         wait_step < INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS;
         wait_step++) {

        if (runtime->worker_error !=
            PSTVNC_INPUT_RUNTIME_ERROR_NONE)
            return -1;

        if (runtime->mouse_interpretation_suspended)
            return 0;

        if (DelayThread(
                INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US) < 0)
            return -1;
    }

    /*
     * Fail closed. Leave the request asserted because the worker may still
     * establish the boundary after the caller's proof window expires.
     */
    return -1;
}

int pstvnc_input_runtime_rebase_suspended_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons)
{
    int wheel_mode_enabled;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started ||
        !runtime->mouse_interpretation_suspend_requested ||
        !runtime->mouse_interpretation_suspended)
        return -1;

    /*
     * A local-controller foreground must not leave an ordinary remote mouse
     * button logically held behind it.
     */
    if (published_click_buttons != 0)
        return -1;

    /*
     * pstvnc_mouse_rebase_published_state() is also used for true hard
     * ownership boundaries and therefore deliberately clears wheel mode.
     *
     * A local-foreground suspension is narrower: application/main is
     * authoritative only for the published cursor/button state. Preserve the
     * current persistent wheel-mode selection across that reconciliation.
     *
     * While suspension acknowledgement is asserted, the worker never mutates
     * pstvnc_mouse_t. Physical loss is recorded separately and its hard reset is
     * deferred until the worker processes resume. The save/rebase/restore
     * sequence is therefore race-free, and a recorded continuity loss will
     * still clear the mode before interpretation resumes.
     */
    wheel_mode_enabled =
        runtime->mouse.wheel_mode_enabled;

    if (!pstvnc_mouse_rebase_published_state(
            &runtime->mouse,
            published_cursor_x,
            published_cursor_y,
            published_click_buttons))
        return -1;

    runtime->mouse.wheel_mode_enabled =
        wheel_mode_enabled;

    runtime->suspended_mouse_state_rebased = 1;
    return 0;
}

int pstvnc_input_runtime_resume_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    unsigned int wait_step;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->controller_thread_started ||
        !runtime->mouse_interpretation_suspend_requested ||
        !runtime->mouse_interpretation_suspended ||
        !runtime->suspended_mouse_state_rebased)
        return -1;

    if (runtime->worker_error !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return -1;

    runtime->mouse_interpretation_suspend_requested = 0;

    for (wait_step = 0;
         wait_step < INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS;
         wait_step++) {

        if (runtime->worker_error !=
            PSTVNC_INPUT_RUNTIME_ERROR_NONE)
            return -1;

        if (!runtime->mouse_interpretation_suspended)
            return 0;

        if (DelayThread(
                INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US) < 0)
            return -1;
    }

    /*
     * The request remains withdrawn, but an unproven acknowledgement
     * withdrawal is still a real failure.
     */
    return -1;
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
    if (runtime->mouse_interpretation_suspend_requested ||
        runtime->mouse_interpretation_suspended ||
        runtime->pad_handoff_requested ||
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
