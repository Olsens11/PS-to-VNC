/*
 * File synopsis:
 * Defines Input-owned semantic product actions, typed caller-supplied
 * controller bindings, and the pure platform-neutral binding resolver.
 *
 * R28 deliberately supplies no product binding table. A binding is an explicit
 * value owned by a later configuration/composition caller: physical button
 * mask, semantic action, trigger kind, and context. The resolver recognizes
 * meaning only; it performs no UI, RFB, MPEG, media-clock, Transport, or
 * application side effect.
 *
 * The timing constants preserve the qualified B4A hotkey arbitration policy:
 * ordinary settle 8 polls, extendable settle 18 polls, and HOLD completion
 * after 120 hold polls following ordinary settle admission.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28;
 * docs/ledge/LEDGE_AUDIT_A005_INTERACTION_INPUT.md.
 */

#ifndef PSTVNC_PRODUCT_ACTION_H
#define PSTVNC_PRODUCT_ACTION_H

#include <stddef.h>
#include <stdint.h>

#include "controller.h"

#define PSTVNC_PRODUCT_ACTION_SETTLE_POLLS 8u
#define PSTVNC_PRODUCT_ACTION_EXTENDABLE_SETTLE_POLLS 18u
#define PSTVNC_PRODUCT_ACTION_HOLD_POLLS 120u

typedef enum pstvnc_product_action {
    PSTVNC_PRODUCT_ACTION_NONE = 0,
    PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION = 1
} pstvnc_product_action_t;

typedef enum pstvnc_product_action_trigger {
    PSTVNC_PRODUCT_ACTION_TRIGGER_NONE = 0,
    PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE = 1,
    PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE = 2,
    PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD = 3
} pstvnc_product_action_trigger_t;

typedef enum pstvnc_product_action_context {
    PSTVNC_PRODUCT_ACTION_CONTEXT_NONE = 0,
    PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP = 1,
    PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL = 2
} pstvnc_product_action_context_t;

/*
 * One caller-owned binding value.
 *
 * button_mask uses project-owned PSTVNC_CONTROLLER_BUTTON_* bits. R28 accepts
 * no hidden/default bindings; the binding array supplied to resolver_init()
 * remains immutable for the resolver lifetime.
 */
typedef struct pstvnc_product_action_binding {
    uint32_t button_mask;
    pstvnc_product_action_t action;
    pstvnc_product_action_trigger_t trigger;
    pstvnc_product_action_context_t context;
} pstvnc_product_action_binding_t;

typedef struct pstvnc_product_action_resolver {
    const pstvnc_product_action_binding_t *bindings;
    size_t binding_count;
    uint32_t binding_universe_mask;

    uint32_t observed_mask;
    unsigned int stable_polls;

    pstvnc_product_action_t active_action;
    pstvnc_product_action_trigger_t active_trigger;
    pstvnc_product_action_context_t active_context;
    unsigned int hold_polls;

    uint32_t release_armed_mask;
    pstvnc_product_action_t release_armed_action;
    pstvnc_product_action_context_t release_armed_context;

    int latched;
    int gesture_started_in_desktop;
    int desktop_context_preserved;
    int initialized;
} pstvnc_product_action_resolver_t;

/* Return nonzero only for a routable semantic product action. */
int pstvnc_product_action_is_valid(
    pstvnc_product_action_t action);

/*
 * Validate and attach one immutable caller-supplied binding set.
 *
 * count==0 is a valid resolver with no bindings. This makes the absence of a
 * compiled product mapping explicit. Non-empty sets reject invalid fields and
 * any duplicate/ambiguous exact physical mask.
 */
int pstvnc_product_action_resolver_init(
    pstvnc_product_action_resolver_t *resolver,
    const pstvnc_product_action_binding_t *bindings,
    size_t binding_count);

/*
 * Observe one trustworthy physical controller sample.
 *
 * desktop_context is exactly 0 or 1 and describes current product admission,
 * not UI meaning encoded in the physical sample.
 *
 * Returns:
 *   1 -> exactly one semantic action copied to *action;
 *   0 -> no action on this sample;
 *  -1 -> invalid resolver/sample/context/output.
 */
int pstvnc_product_action_resolver_observe(
    pstvnc_product_action_resolver_t *resolver,
    const pstvnc_controller_state_t *controller_state,
    int desktop_context,
    pstvnc_product_action_t *action);

#endif /* PSTVNC_PRODUCT_ACTION_H */
