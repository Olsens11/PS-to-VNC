/*
 * File synopsis:
 * White-box host proof for R29 Input-runtime composition of the accepted R28
 * semantic product-action resolver.
 *
 * The test includes input_runtime.c directly so it can exercise the exact
 * per-sample composition boundary without adding a production test API.
 * PS2 kernel/libpad ownership is replaced only by narrow deterministic stubs.
 */

#include <stdio.h>
#include <string.h>

#include "kernel.h"

unsigned char _gp;

#include "../../src/input/input_runtime.c"

static int failures;

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            failures++; \
        } \
    } while (0)

static int wait_calls;
static int signal_calls;
static int fail_wait_call;
static int fail_signal_call;
static int create_thread_calls;
static int start_thread_calls;
static int create_sema_calls;
static int delete_sema_calls;

static int fake_poll_result;
static uint16_t fake_poll_down;
static uint16_t fake_poll_pressed;
static uint16_t fake_poll_released;
static int fake_poll_calls;

static pstvnc_input_runtime_t *delay_stop_runtime;
static int stop_on_poll_delay;

typedef struct notify_probe {
    pstvnc_input_runtime_t *runtime;
    int calls;
    unsigned int first_count;
    pstvnc_input_event_type_t first_type;
    pstvnc_input_event_type_t second_type;
    pstvnc_input_event_type_t third_type;
} notify_probe_t;

static void reset_platform_stubs(void)
{
    wait_calls = 0;
    signal_calls = 0;
    fail_wait_call = 0;
    fail_signal_call = 0;
    create_thread_calls = 0;
    start_thread_calls = 0;
    create_sema_calls = 0;
    delete_sema_calls = 0;

    fake_poll_result = 0;
    fake_poll_down = 0u;
    fake_poll_pressed = 0u;
    fake_poll_released = 0u;
    fake_poll_calls = 0;

    delay_stop_runtime = NULL;
    stop_on_poll_delay = 0;
}

int CreateSema(ee_sema_t *semaphore)
{
    CHECK(semaphore != NULL);
    if (semaphore != NULL) {
        CHECK(semaphore->init_count == 1);
        CHECK(semaphore->max_count == 1);
    }
    create_sema_calls++;
    return 11;
}

int DeleteSema(int semaphore_id)
{
    CHECK(semaphore_id == 11);
    delete_sema_calls++;
    return 0;
}

int WaitSema(int semaphore_id)
{
    CHECK(semaphore_id == 11);
    wait_calls++;
    if (fail_wait_call != 0 && wait_calls == fail_wait_call)
        return -1;
    return 0;
}

int SignalSema(int semaphore_id)
{
    CHECK(semaphore_id == 11);
    signal_calls++;
    if (fail_signal_call != 0 && signal_calls == fail_signal_call)
        return -1;
    return 0;
}

int CreateThread(ee_thread_t *thread)
{
    CHECK(thread != NULL);
    create_thread_calls++;
    return 77;
}

int StartThread(int thread_id, void *argument)
{
    CHECK(thread_id == 77);
    CHECK(argument != NULL);
    start_thread_calls++;
    return 0;
}

int ReferThreadStatus(int thread_id, ee_thread_status_t *status)
{
    CHECK(thread_id == 77);
    CHECK(status != NULL);
    if (status != NULL)
        status->status = THS_DORMANT;
    return 0;
}

int DeleteThread(int thread_id)
{
    CHECK(thread_id == 77);
    return 0;
}

void ExitThread(void)
{
}

int DelayThread(int microseconds)
{
    if (
        stop_on_poll_delay &&
        delay_stop_runtime != NULL &&
        microseconds == INPUT_RUNTIME_POLL_DELAY_US)
        delay_stop_runtime->stop_requested = 1;

    return 0;
}

int pstvnc_pad_init(void)
{
    return 0;
}

void pstvnc_pad_shutdown(void)
{
}

int pstvnc_pad_open(
    pstvnc_pad_t *pad,
    int port,
    int slot)
{
    CHECK(pad != NULL);
    if (pad == NULL)
        return -1;

    pad->port = port;
    pad->slot = slot;
    pad->opened = 1;
    pad->history_valid = 0;
    return 0;
}

int pstvnc_pad_poll(pstvnc_pad_t *pad)
{
    fake_poll_calls++;

    if (pad == NULL)
        return -1;

    if (fake_poll_result > 0) {
        memset(&pad->buttons, 0, sizeof(pad->buttons));
        pad->sample_length = 0u;
        pad->buttons_down = fake_poll_down;
        pad->buttons_pressed = fake_poll_pressed;
        pad->buttons_released = fake_poll_released;
        pad->history_valid = 1;
    }

    return fake_poll_result;
}

void pstvnc_pad_invalidate_connection_epoch(
    pstvnc_pad_t *pad)
{
    CHECK(pad != NULL);
    if (pad == NULL)
        return;

    pad->history_valid = 0;
    pad->connection_configured = 0;
    pad->dualshock_mode_request_pending = 0;
    pad->sample_length = 0u;
    memset(&pad->buttons, 0, sizeof(pad->buttons));
    pad->buttons_down = 0u;
    pad->buttons_pressed = 0u;
    pad->buttons_released = 0u;
}

void pstvnc_pad_close(pstvnc_pad_t *pad)
{
    if (pad != NULL)
        pad->opened = 0;
}

static void notify_probe_callback(void *context)
{
    notify_probe_t *probe =
        (notify_probe_t *)context;
    pstvnc_input_queue_t *queue;
    unsigned int index;

    CHECK(probe != NULL);
    if (probe == NULL)
        return;

    probe->calls++;
    if (probe->calls != 1)
        return;

    queue = &probe->runtime->event_queue;
    probe->first_count =
        pstvnc_input_queue_event_count(queue);

    index = queue->read_index;
    if (probe->first_count > 0u) {
        probe->first_type =
            queue->event_storage[index].type;
        index =
            (index + 1u) %
            PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;
    }

    if (probe->first_count > 1u) {
        probe->second_type =
            queue->event_storage[index].type;
        index =
            (index + 1u) %
            PSTVNC_INPUT_EVENT_QUEUE_CAPACITY;
    }

    if (probe->first_count > 2u)
        probe->third_type =
            queue->event_storage[index].type;
}

static void prepare_runtime(
    pstvnc_input_runtime_t *runtime)
{
    reset_platform_stubs();

    CHECK(
        pstvnc_input_runtime_init(
            runtime,
            640u,
            480u,
            0,
            0) == 0);

    CHECK(runtime->initialized);
    CHECK(runtime->product_action_resolver.initialized);
    CHECK(runtime->product_action_resolver.binding_count == 0u);
    CHECK(runtime->product_action_desktop_eligible == 0);
}

static void set_native_sample(
    pstvnc_input_runtime_t *runtime,
    uint16_t down,
    uint16_t pressed,
    uint16_t released)
{
    memset(&runtime->pad.buttons, 0, sizeof(runtime->pad.buttons));
    runtime->pad.sample_length = 0u;
    runtime->pad.buttons_down = down;
    runtime->pad.buttons_pressed = pressed;
    runtime->pad.buttons_released = released;
    runtime->pad.history_valid = 1;
}

static void discard_events(
    pstvnc_input_runtime_t *runtime)
{
    pstvnc_input_queue_discard_all(
        &runtime->event_queue);
}

static int pop_event(
    pstvnc_input_runtime_t *runtime,
    pstvnc_input_event_t *event)
{
    return pstvnc_input_queue_pop(
        &runtime->event_queue,
        event);
}

static pstvnc_product_action_binding_t make_binding(
    uint32_t mask,
    pstvnc_product_action_trigger_t trigger,
    pstvnc_product_action_context_t context)
{
    pstvnc_product_action_binding_t binding;

    binding.button_mask = mask;
    binding.action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    binding.trigger = trigger;
    binding.context = context;
    return binding;
}

static void process_sample(
    pstvnc_input_runtime_t *runtime,
    uint16_t down,
    uint16_t pressed,
    uint16_t released)
{
    set_native_sample(
        runtime,
        down,
        pressed,
        released);

    CHECK(input_runtime_process_pad_sample(runtime));
}

static void test_zero_binding_and_nonfiring_paths_match_existing_events(void)
{
    pstvnc_input_runtime_t zero_runtime;
    pstvnc_input_runtime_t configured_runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t zero_event;
    pstvnc_input_event_t configured_event;

    prepare_runtime(&zero_runtime);
    prepare_runtime(&configured_runtime);

    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &zero_runtime,
            NULL,
            0u) == 0);

    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &configured_runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &configured_runtime,
            1) == 0);

    process_sample(
        &zero_runtime,
        PAD_CROSS,
        PAD_CROSS,
        0u);
    process_sample(
        &configured_runtime,
        PAD_CROSS,
        PAD_CROSS,
        0u);

    process_sample(
        &zero_runtime,
        PAD_CROSS,
        0u,
        0u);
    process_sample(
        &configured_runtime,
        PAD_CROSS,
        0u,
        0u);

    process_sample(
        &zero_runtime,
        0u,
        0u,
        PAD_CROSS);
    process_sample(
        &configured_runtime,
        0u,
        0u,
        PAD_CROSS);

    CHECK(
        pstvnc_input_queue_event_count(
            &zero_runtime.event_queue) ==
        pstvnc_input_queue_event_count(
            &configured_runtime.event_queue));

    while (pop_event(&zero_runtime, &zero_event)) {
        CHECK(pop_event(&configured_runtime, &configured_event));
        CHECK(
            memcmp(
                &zero_event,
                &configured_event,
                sizeof(zero_event)) == 0);
    }

    CHECK(
        pstvnc_input_queue_event_count(
            &configured_runtime.event_queue) == 0u);
}

static void test_invalid_or_mutated_binding_blocks_worker_activation(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);

    prepare_runtime(&runtime);

    binding.button_mask = 0u;
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) < 0);
    CHECK(!runtime.product_action_resolver.initialized);

    create_thread_calls = 0;
    CHECK(pstvnc_input_runtime_start(&runtime) < 0);
    CHECK(create_thread_calls == 0);

    binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);

    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);

    /*
     * Caller authority is immutable for the runtime lifetime. R29 revalidates
     * it at worker activation, so a caller mutation cannot slip into a live
     * controller worker.
     */
    binding.button_mask = 0u;
    create_thread_calls = 0;
    CHECK(pstvnc_input_runtime_start(&runtime) < 0);
    CHECK(create_thread_calls == 0);

    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            NULL,
            0u) == 0);
    CHECK(runtime.product_action_resolver.initialized);
    CHECK(runtime.product_action_resolver.binding_count == 0u);
}

static void test_resolver_history_is_runtime_local(void)
{
    pstvnc_input_runtime_t runtime_a;
    pstvnc_input_runtime_t runtime_b;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    unsigned int i;

    prepare_runtime(&runtime_a);
    prepare_runtime(&runtime_b);

    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime_a,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime_b,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime_a,
            1) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime_b,
            1) == 0);

    process_sample(
        &runtime_a,
        PAD_L1,
        PAD_L1,
        0u);
    discard_events(&runtime_a);

    for (i = 0u; i < 4u; i++)
        process_sample(&runtime_a, PAD_L1, 0u, 0u);

    CHECK(runtime_a.product_action_resolver.stable_polls == 4u);
    CHECK(runtime_b.product_action_resolver.stable_polls == 0u);
    CHECK(runtime_b.product_action_resolver.observed_mask == 0u);
}

static void test_stable_no_edge_samples_drive_settle(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t event;
    unsigned int i;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(
        &runtime,
        PAD_L1,
        PAD_L1,
        0u);
    discard_events(&runtime);

    for (i = 1u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS; i++) {
        process_sample(&runtime, PAD_L1, 0u, 0u);
        CHECK(
            pstvnc_input_queue_event_count(
                &runtime.event_queue) == 0u);
    }

    process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 1u);
    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(
        event.payload.product_action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
}

static void test_hold_timing_uses_every_trustworthy_poll(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L2,
            PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
            PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL);
    pstvnc_input_event_t event;
    unsigned int stable_poll;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);

    process_sample(
        &runtime,
        PAD_L2,
        PAD_L2,
        0u);
    discard_events(&runtime);

    for (
        stable_poll = 1u;
        stable_poll <
            (
                PSTVNC_PRODUCT_ACTION_SETTLE_POLLS +
                PSTVNC_PRODUCT_ACTION_HOLD_POLLS -
                1u
            );
        stable_poll++) {

        process_sample(&runtime, PAD_L2, 0u, 0u);
        CHECK(
            pstvnc_input_queue_event_count(
                &runtime.event_queue) == 0u);
    }

    process_sample(&runtime, PAD_L2, 0u, 0u);
    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(
        event.payload.product_action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    discard_events(&runtime);
    process_sample(&runtime, PAD_L2, 0u, 0u);
    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);
}

static void test_release_action_precedes_same_sample_controller_and_mouse(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t event;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(
        &runtime,
        PAD_CROSS,
        PAD_CROSS,
        0u);
    CHECK(runtime.mouse.click_buttons == PSTVNC_MOUSE_BUTTON_LCLICK);
    discard_events(&runtime);

    process_sample(
        &runtime,
        0u,
        0u,
        PAD_CROSS);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 3u);

    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(
        event.payload.product_action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_CONTROLLER_STATE);
    CHECK(
        event.payload.controller_state.buttons_released ==
        PSTVNC_CONTROLLER_BUTTON_CROSS);

    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_MOUSE_UPDATE);
    CHECK(event.payload.mouse_update.click_buttons == 0u);
}

static void test_queue_failure_after_resolution_stops_same_sample_work(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(
        &runtime,
        PAD_CROSS,
        PAD_CROSS,
        0u);
    CHECK(runtime.mouse.click_buttons == PSTVNC_MOUSE_BUTTON_LCLICK);
    discard_events(&runtime);

    wait_calls = 0;
    fail_wait_call = 1;

    set_native_sample(
        &runtime,
        0u,
        0u,
        PAD_CROSS);

    CHECK(!input_runtime_process_pad_sample(&runtime));
    CHECK(wait_calls == 1);
    CHECK(
        runtime.worker_error ==
        PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_WAIT);
    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);

    /*
     * Mouse release belongs later in the same sample. Product-action queue
     * failure prevents that work from advancing.
     */
    CHECK(runtime.mouse.click_buttons == PSTVNC_MOUSE_BUTTON_LCLICK);
}

static void test_desktop_context_provenance_uses_live_caller_fact(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t event;
    unsigned int i;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            0) == 0);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);

    for (i = 0u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);

    /*
     * The fact remains live-changeable while a worker is nominally active, but
     * R28 provenance does not allow desktop authority to be acquired mid-gesture.
     */
    runtime.controller_thread_started = 1;
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);
    runtime.controller_thread_started = 0;

    for (i = 0u; i < 12u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);

    process_sample(&runtime, 0u, 0u, PAD_L1);
    discard_events(&runtime);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);

    for (i = 1u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    process_sample(&runtime, PAD_L1, 0u, 0u);
    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);

    /*
     * A gesture that begins in desktop and later loses admission cannot regain
     * it before full release either.
     */
    discard_events(&runtime);
    process_sample(&runtime, 0u, 0u, PAD_L1);
    discard_events(&runtime);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            0) == 0);
    process_sample(&runtime, PAD_L1, 0u, 0u);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    for (i = 0u; i < 12u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);
}

static void test_hard_handoff_resets_pending_gesture_history(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t event;
    unsigned int i;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(runtime.product_action_resolver.stable_polls == 4u);
    CHECK(input_runtime_enter_pad_handoff(&runtime));
    CHECK(runtime.product_action_resolver.stable_polls == 0u);
    CHECK(runtime.product_action_resolver.observed_mask == 0u);
    CHECK(runtime.physical_sample_active == 0);

    /*
     * Even if a test supplies a held first post-handoff baseline, it starts a
     * new R28 gesture rather than continuing the four pre-handoff polls.
     */
    process_sample(&runtime, PAD_L1, 0u, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);

    for (i = 4u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
}

static void test_physical_loss_resets_pending_gesture_history(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    unsigned int i;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(runtime.product_action_resolver.stable_polls == 4u);
    CHECK(input_runtime_handle_physical_loss(&runtime));
    CHECK(runtime.product_action_resolver.stable_polls == 0u);
    CHECK(runtime.product_action_resolver.observed_mask == 0u);
    CHECK(runtime.physical_sample_active == 0);

    process_sample(&runtime, PAD_L1, 0u, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(
        pstvnc_input_queue_event_count(
            &runtime.event_queue) == 0u);
}

static void test_mouse_suspension_does_not_create_second_gesture_owner(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    pstvnc_input_event_t event;
    unsigned int i;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(&runtime, PAD_L1, PAD_L1, 0u);
    discard_events(&runtime);
    for (i = 0u; i < 4u; i++)
        process_sample(&runtime, PAD_L1, 0u, 0u);

    runtime.mouse_interpretation_suspended = 1;

    for (i = 0u; i < 3u; i++) {
        process_sample(&runtime, PAD_L1, 0u, 0u);
        discard_events(&runtime);
    }

    process_sample(&runtime, PAD_L1, 0u, 0u);

    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(pop_event(&runtime, &event));
    CHECK(event.type == PSTVNC_INPUT_EVENT_CONTROLLER_STATE);
}

static void test_activity_notify_sees_published_product_action(void)
{
    pstvnc_input_runtime_t runtime;
    pstvnc_product_action_binding_t binding =
        make_binding(
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    notify_probe_t probe;

    prepare_runtime(&runtime);
    CHECK(
        pstvnc_input_runtime_set_product_action_bindings(
            &runtime,
            &binding,
            1u) == 0);
    CHECK(
        pstvnc_input_runtime_set_product_action_desktop_eligible(
            &runtime,
            1) == 0);

    process_sample(&runtime, PAD_CROSS, PAD_CROSS, 0u);
    discard_events(&runtime);

    memset(&probe, 0, sizeof(probe));
    probe.runtime = &runtime;

    CHECK(
        pstvnc_input_runtime_set_activity_notify(
            &runtime,
            notify_probe_callback,
            &probe) == 0);

    fake_poll_result = 1;
    fake_poll_down = 0u;
    fake_poll_pressed = 0u;
    fake_poll_released = PAD_CROSS;

    delay_stop_runtime = &runtime;
    stop_on_poll_delay = 1;
    runtime.stop_requested = 0;

    input_runtime_controller_thread(&runtime);

    CHECK(fake_poll_calls == 1);
    CHECK(probe.calls == 2);
    CHECK(probe.first_count == 3u);
    CHECK(probe.first_type == PSTVNC_INPUT_EVENT_PRODUCT_ACTION);
    CHECK(probe.second_type == PSTVNC_INPUT_EVENT_CONTROLLER_STATE);
    CHECK(probe.third_type == PSTVNC_INPUT_EVENT_MOUSE_UPDATE);
}

int main(void)
{
    test_zero_binding_and_nonfiring_paths_match_existing_events();
    test_invalid_or_mutated_binding_blocks_worker_activation();
    test_resolver_history_is_runtime_local();
    test_stable_no_edge_samples_drive_settle();
    test_hold_timing_uses_every_trustworthy_poll();
    test_release_action_precedes_same_sample_controller_and_mouse();
    test_queue_failure_after_resolution_stops_same_sample_work();
    test_desktop_context_provenance_uses_live_caller_fact();
    test_hard_handoff_resets_pending_gesture_history();
    test_physical_loss_resets_pending_gesture_history();
    test_mouse_suspension_does_not_create_second_gesture_owner();
    test_activity_notify_sees_published_product_action();

    if (failures != 0) {
        fprintf(
            stderr,
            "INPUT_RUNTIME_PRODUCT_ACTION_TEST=FAIL count=%d\n",
            failures);
        return 1;
    }

    puts("INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS");
    return 0;
}
