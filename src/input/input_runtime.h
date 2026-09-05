/*
 * File synopsis:
 * Owns the PS2 controller/input runtime that composes physical libpad
 * acquisition, mouse interpretation, and synchronized semantic-event
 * publication.
 *
 * One runtime instance owns:
 *
 *   - one controller producer thread;
 *   - one physical pstvnc_pad_t endpoint;
 *   - one pstvnc_mouse_t interpreter;
 *   - one ordinary pstvnc_input_queue_t;
 *   - the semaphore protecting that queue across producer/consumer threads;
 *   - cooperative thread shutdown;
 *   - a mouse-interpretation suspension seam that keeps libpad polling live;
 *   - the explicit libpad ownership-handoff request/acknowledgement seam.
 *
 * This interface does not serialize RFB, touch the VNC socket, own UI/display
 * state, interpret general keyboard/hotkey meaning, or execute product actions.
 * Application/main remains responsible for consuming semantic events and
 * routing their cross-domain effects.
 *
 * Mouse-domain routing implemented by Issue #38 is intentionally explicit:
 *
 *   D-pad             -> pointer
 *   Triangle + D-pad  -> wheel
 *   left stick        -> pointer / persistent analog-wheel mode
 *   L3 pressed edge   -> toggle analog-wheel mode
 *   Cross             -> left mouse button
 *   Circle            -> right mouse button
 *
 * The urgent-control mailbox described by CLEAN_ARCHITECTURE is not created
 * speculatively here. It is a separate concurrency seam and should be added
 * when an actual urgent product intent is reconstructed.
 *
 * Context:
 *   docs/PROJECT_INTENT.md;
 *   docs/CLEAN_ARCHITECTURE.md, Input/controller and concurrency model;
 *   docs/adr/0002-use-libpad-directly-reference-opl.md;
 *   GitHub Issue #38.
 */

#ifndef PSTVNC_INPUT_RUNTIME_H
#define PSTVNC_INPUT_RUNTIME_H

#include "input.h"
#include "mouse.h"
#include "pad.h"

/*
 * Retain the historically qualified controller-thread stack size.
 *
 * The stack belongs to the runtime instance rather than a process-global array
 * so lifecycle ownership remains visible at the same place as the thread ID.
 */
#define PSTVNC_INPUT_RUNTIME_THREAD_STACK_SIZE 8192u

/*
 * Worker-side failures that make further semantic input publication unsafe.
 *
 * Kernel lifecycle failures on the application/main side are returned directly
 * by the corresponding API rather than being mixed into this worker error.
 */
typedef enum pstvnc_input_runtime_error {
    PSTVNC_INPUT_RUNTIME_ERROR_NONE = 0,
    PSTVNC_INPUT_RUNTIME_ERROR_PAD_POLL,
    PSTVNC_INPUT_RUNTIME_ERROR_MOUSE_UPDATE,
    PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_WAIT,
    PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_FULL,
    PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_SIGNAL,
    PSTVNC_INPUT_RUNTIME_ERROR_THREAD_DELAY
} pstvnc_input_runtime_error_t;

/*
 * Complete owner state for one local controller/input producer.
 *
 * Most fields are operational state owned by input_runtime.c. They remain in a
 * named ordinary struct rather than hidden globals so a future maintainer can
 * see the complete lifecycle and ownership relationship in one place.
 *
 * Cross-thread flags are volatile because the EE producer and application/main
 * consumer cooperatively observe them. They are simple single-writer control
 * facts, not substitutes for the semaphore protecting the ordinary event FIFO.
 */
typedef struct pstvnc_input_runtime {
    pstvnc_pad_t pad;
    pstvnc_mouse_t mouse;
    pstvnc_input_queue_t event_queue;

    int event_queue_sema_id;
    int controller_thread_id;

    int initialized;
    int controller_thread_started;

    /*
     * Worker-only continuity fact.
     *
     * A zero-return pad poll can mean either "no new sample this tick" or a
     * real physical-continuity loss. The runtime combines this flag with the
     * pad owner's history_valid fact so a disconnect releases any remote mouse
     * button exactly once rather than once per disconnected poll.
     */
    int physical_sample_active;

    volatile int stop_requested;

    /*
     * Mouse-interpretation suspension is distinct from libpad handoff.
     *
     * Application/main requests suspension. The controller worker acknowledges
     * only after pre-boundary queued input has been discarded and transient
     * controller-derived mouse response history has been reset.
     *
     * While acknowledged, the worker continues polling the physical controller
     * but does not mutate pstvnc_mouse_t. If physical continuity is lost during
     * that interval, the worker records the hard-boundary fact separately and
     * applies the ordinary hard mouse reset before interpretation can resume.
     *
     * Application/main must explicitly reconcile the frozen mouse interpreter
     * with its exact successfully-published remote pointer state before resume.
     */
    volatile int mouse_interpretation_suspend_requested;
    volatile int mouse_interpretation_suspended;
    volatile int suspended_mouse_state_rebased;

    /*
     * Worker-owned fact for the currently acknowledged mouse-suspension epoch.
     *
     * A physical controller continuity loss is a true hard mouse boundary, but
     * the worker must not mutate pstvnc_mouse_t while application/main owns the
     * suspended rebase interval. Record the loss here and apply the hard reset
     * before withdrawing the suspension acknowledgement.
     */
    int suspended_physical_continuity_lost;

    /*
     * Explicit libpad ownership handoff:
     *
     * application/main writes pad_handoff_requested;
     * controller worker alone writes pad_handoff_acknowledged.
     *
     * Acknowledgement means the worker has reached a safe boundary before its
     * next libpad access and will perform no libpad operation while the request
     * remains asserted.
     */
    volatile int pad_handoff_requested;
    volatile int pad_handoff_acknowledged;

    /*
     * Set only by application/main after acknowledgement and after it has
     * reconciled the paused mouse interpreter with the pointer state actually
     * published to the remote peer.
     *
     * Returning libpad ownership is rejected until this is true, preventing
     * discarded pre-boundary cursor/button state from reappearing afterward.
     */
    volatile int handoff_published_state_rebased;

    volatile pstvnc_input_runtime_error_t worker_error;

    unsigned char
        controller_thread_stack[PSTVNC_INPUT_RUNTIME_THREAD_STACK_SIZE]
        __attribute__((aligned(16)));
} pstvnc_input_runtime_t;

/*
 * Initialize input-family ownership without starting the producer thread.
 *
 * width/height define the remote mouse coordinate space.
 * port/slot identify the native libpad endpoint.
 */
int pstvnc_input_runtime_init(
    pstvnc_input_runtime_t *runtime,
    unsigned int width,
    unsigned int height,
    int port,
    int slot);

/*
 * Start the dedicated approximately-60-Hz controller producer.
 */
int pstvnc_input_runtime_start(
    pstvnc_input_runtime_t *runtime);

/*
 * Cooperatively stop the worker if necessary and release all resources owned
 * by this runtime.
 *
 * The implementation never force-terminates the worker. If dormancy cannot be
 * proven, shared resources remain owned and the function returns failure
 * instead of deleting memory/semaphore/libpad state from under a live thread.
 */
int pstvnc_input_runtime_shutdown(
    pstvnc_input_runtime_t *runtime);

/*
 * Application/main-side ordinary-event consumption.
 *
 * Returns:
 *   1 -> one complete event copied to *event;
 *   0 -> queue currently empty;
 *  -1 -> invalid state/argument or semaphore failure.
 */
int pstvnc_input_runtime_pop_event(
    pstvnc_input_runtime_t *runtime,
    pstvnc_input_event_t *event);

/*
 * Discard all currently queued ordinary semantic events under the runtime's
 * queue semaphore.
 *
 * This is queue history only; it does not itself alter physical, mouse, or RFB
 * state.
 */
int pstvnc_input_runtime_discard_events(
    pstvnc_input_runtime_t *runtime);

/*
 * Read the synchronized queue depth.
 */
int pstvnc_input_runtime_event_count(
    pstvnc_input_runtime_t *runtime,
    unsigned int *event_count);

/*
 * Return the first worker-side failure recorded by the controller producer.
 */
pstvnc_input_runtime_error_t
pstvnc_input_runtime_last_error(
    const pstvnc_input_runtime_t *runtime);

/*
 * Asynchronously request temporary libpad ownership from the controller worker.
 *
 * A later acknowledgement guarantees:
 *
 *   - no runtime-owned libpad command remains outstanding;
 *   - the worker performs no libpad calls while the request remains asserted;
 *   - pre-handoff queued ordinary input was discarded;
 *   - controller-derived mouse motion/repeat history was reset.
 *
 * Acknowledgement deliberately does NOT claim that the runtime's local
 * cursor/button state equals the remote state. Application/main is the only
 * owner that knows which semantic pointer events were successfully serialized
 * and published.
 *
 * After acknowledgement, application/main must reconcile the remote logical
 * pointer state and call pstvnc_input_runtime_rebase_published_mouse_state()
 * before returning libpad ownership.
 */
/*
 * Establish a synchronous mouse-interpretation suspension boundary while the
 * controller worker retains libpad ownership and continues physical polling.
 *
 * Success proves:
 *
 *   - pre-boundary queued semantic input has been discarded;
 *   - transient mouse movement/repeat/fractional history has been reset
 *     without suspension itself revoking persistent analog-wheel mode;
 *   - the worker will not advance pstvnc_mouse_t while suspension remains
 *     requested;
 *   - ordinary libpad polling continues;
 *   - any later physical-continuity loss is remembered for this suspension
 *     epoch and cannot allow pre-loss persistent mouse mode to resume.
 *
 * Durable cursor/button state is deliberately not guessed at this boundary.
 * Application/main owns the last successfully-published remote pointer state.
 */
int pstvnc_input_runtime_suspend_mouse_interpretation(
    pstvnc_input_runtime_t *runtime);

/*
 * Reconcile the suspended mouse interpreter to application/main's exact
 * successfully-published remote pointer state.
 *
 * Local foreground entry must first publish any necessary remote click release
 * at the frozen coordinates. Accordingly, this suspension boundary accepts
 * only a neutral published click state.
 *
 * This pointer-state reconciliation does not itself revoke the persistent
 * analog-wheel mode selected before local foreground ownership.
 */
int pstvnc_input_runtime_rebase_suspended_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons);

/*
 * Resume mouse interpretation after the higher-level foreground owner has
 * proven its physical-release quarantine complete.
 *
 * This function deliberately does not know which controller buttons belong to
 * any local UI. Product meaning remains above input_runtime.
 *
 * Before acknowledgement withdrawal, the worker applies the ordinary hard
 * mouse reset if physical continuity was lost at any point during this
 * suspension epoch.
 *
 * Success includes acknowledgement withdrawal, so the completed suspension
 * epoch cannot be mistaken for a later request.
 */
int pstvnc_input_runtime_resume_mouse_interpretation(
    pstvnc_input_runtime_t *runtime);

int pstvnc_input_runtime_request_pad_handoff(
    pstvnc_input_runtime_t *runtime);

/*
 * Report whether the controller worker has acknowledged the current handoff.
 */
int pstvnc_input_runtime_pad_handoff_acknowledged(
    const pstvnc_input_runtime_t *runtime);

/*
 * Reconcile the paused input interpreter with application/main's exact
 * successfully published remote pointer state.
 *
 * This may be called only while a pad handoff is acknowledged.
 *
 * Application/main should first publish any required remote button release
 * using its own last-published pointer state. For a handoff boundary,
 * published_click_buttons must therefore be zero.
 *
 * The supplied cursor coordinates become the new durable mouse origin and all
 * controller-derived mouse history remains cleared.
 */
int pstvnc_input_runtime_rebase_published_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons);

/*
 * Return libpad ownership to the controller worker.
 *
 * Release is accepted only after:
 *
 *   - the worker acknowledged the handoff;
 *   - application/main rebased the mouse interpreter to published remote state;
 *   - the temporary owner established the clean physical-release condition
 *     required by its transition.
 *
 * The request is withdrawn and this function then waits for the worker's
 * acknowledgement to drop. Success therefore proves the completed ownership
 * epoch cannot be confused with a later handoff request.
 *
 * The worker starts its next physical connection epoch from reset history
 * before ordinary semantic publication resumes.
 *
 * Returns 0 on proven release completion and -1 if the ownership transition
 * cannot be proven within the bounded lifecycle wait.
 */
int pstvnc_input_runtime_release_pad_handoff(
    pstvnc_input_runtime_t *runtime);

#endif /* PSTVNC_INPUT_RUNTIME_H */
