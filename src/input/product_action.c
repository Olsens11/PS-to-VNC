/*
 * File synopsis:
 * Implements the pure Input-owned semantic product-action binding resolver.
 *
 * The implementation reconstructs the qualified B4A arbitration semantics
 * without importing B4A's compiled product bindings or H1's temporary MPEG
 * calibration entry adapter. Exact-mask changes reset pending timing, RELEASE
 * can arm only through legitimate growth, SETTLE yields to eligible HOLD
 * extensions, and a fired gesture cannot leak a subset action before complete
 * relevant-button release.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28;
 * working/b4a/ps2vnc_runtime_core.inc and ps2vnc_ui_tail.inc as forensic
 * behavior authority only.
 */

#include "product_action.h"

#include <string.h>

static int product_action_trigger_is_valid(
    pstvnc_product_action_trigger_t trigger)
{
    return
        trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE ||
        trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE ||
        trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD;
}

static int product_action_context_is_valid(
    pstvnc_product_action_context_t context)
{
    return
        context == PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP ||
        context == PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL;
}

int pstvnc_product_action_is_valid(
    pstvnc_product_action_t action)
{
    return action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
}

static void product_action_reset_gesture(
    pstvnc_product_action_resolver_t *resolver)
{
    resolver->observed_mask = 0u;
    resolver->stable_polls = 0u;

    resolver->active_action = PSTVNC_PRODUCT_ACTION_NONE;
    resolver->active_trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_NONE;
    resolver->active_context = PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;
    resolver->hold_polls = 0u;

    resolver->release_armed_mask = 0u;
    resolver->release_armed_action = PSTVNC_PRODUCT_ACTION_NONE;
    resolver->release_armed_context = PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;

    resolver->latched = 0;
    resolver->gesture_started_in_desktop = 0;
    resolver->desktop_context_preserved = 0;
}

static int product_action_binding_context_is_eligible(
    const pstvnc_product_action_resolver_t *resolver,
    const pstvnc_product_action_binding_t *binding,
    int desktop_context)
{
    if (binding->context == PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL)
        return 1;

    if (binding->context != PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP)
        return 0;

    return
        desktop_context &&
        resolver->gesture_started_in_desktop &&
        resolver->desktop_context_preserved;
}

static const pstvnc_product_action_binding_t *
product_action_lookup_exact(
    const pstvnc_product_action_resolver_t *resolver,
    uint32_t mask,
    int desktop_context)
{
    size_t i;

    for (i = 0u; i < resolver->binding_count; i++) {
        const pstvnc_product_action_binding_t *binding =
            &resolver->bindings[i];

        if (binding->button_mask != mask)
            continue;

        if (!product_action_binding_context_is_eligible(
                resolver,
                binding,
                desktop_context))
            continue;

        return binding;
    }

    return NULL;
}

static int product_action_has_hold_extension(
    const pstvnc_product_action_resolver_t *resolver,
    uint32_t mask,
    int desktop_context)
{
    size_t i;

    for (i = 0u; i < resolver->binding_count; i++) {
        const pstvnc_product_action_binding_t *binding =
            &resolver->bindings[i];

        if (binding->trigger != PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD)
            continue;

        if (binding->button_mask == mask)
            continue;

        if ((binding->button_mask & mask) != mask)
            continue;

        if (!product_action_binding_context_is_eligible(
                resolver,
                binding,
                desktop_context))
            continue;

        return 1;
    }

    return 0;
}

int pstvnc_product_action_resolver_init(
    pstvnc_product_action_resolver_t *resolver,
    const pstvnc_product_action_binding_t *bindings,
    size_t binding_count)
{
    uint32_t universe_mask = 0u;
    size_t i;
    size_t j;

    if (resolver == NULL)
        return 0;

    memset(resolver, 0, sizeof(*resolver));

    if (binding_count != 0u && bindings == NULL)
        return 0;

    for (i = 0u; i < binding_count; i++) {
        const pstvnc_product_action_binding_t *binding = &bindings[i];

        if (
            binding->button_mask == 0u ||
            (binding->button_mask &
             ~(uint32_t)PSTVNC_CONTROLLER_BUTTON_MASK) != 0u ||
            !pstvnc_product_action_is_valid(binding->action) ||
            !product_action_trigger_is_valid(binding->trigger) ||
            !product_action_context_is_valid(binding->context))
            return 0;

        /*
         * One exact physical mask may resolve to at most one semantic binding.
         * GLOBAL overlaps DESKTOP while desktop is active, so even a
         * cross-context duplicate mask would be ambiguous at runtime.
         */
        for (j = 0u; j < i; j++) {
            if (bindings[j].button_mask == binding->button_mask)
                return 0;
        }

        universe_mask |= binding->button_mask;
    }

    resolver->bindings = bindings;
    resolver->binding_count = binding_count;
    resolver->binding_universe_mask = universe_mask;
    resolver->initialized = 1;
    return 1;
}

int pstvnc_product_action_resolver_observe(
    pstvnc_product_action_resolver_t *resolver,
    const pstvnc_controller_state_t *controller_state,
    int desktop_context,
    pstvnc_product_action_t *action)
{
    const pstvnc_product_action_binding_t *binding;
    uint32_t mask;

    if (
        resolver == NULL ||
        controller_state == NULL ||
        action == NULL ||
        !resolver->initialized ||
        (desktop_context != 0 && desktop_context != 1))
        return -1;

    *action = PSTVNC_PRODUCT_ACTION_NONE;

    if (controller_state->connection_epoch_started)
        product_action_reset_gesture(resolver);

    mask =
        (uint32_t)controller_state->buttons_down &
        resolver->binding_universe_mask;

    if (resolver->latched) {
        if (mask == 0u)
            product_action_reset_gesture(resolver);
        return 0;
    }

    /*
     * Desktop provenance is monotonic inside one physical gesture. Once the
     * current gesture leaves desktop admission it cannot reacquire a desktop
     * binding merely because foreground later changes back.
     */
    if (resolver->observed_mask != 0u && !desktop_context)
        resolver->desktop_context_preserved = 0;

    if (mask != resolver->observed_mask) {
        uint32_t old_mask = resolver->observed_mask;
        int pure_growth = ((old_mask & mask) == old_mask);
        int pure_shrink = ((mask & old_mask) == mask);
        pstvnc_product_action_t release_action =
            PSTVNC_PRODUCT_ACTION_NONE;

        if (old_mask == 0u && mask != 0u) {
            resolver->gesture_started_in_desktop = desktop_context;
            resolver->desktop_context_preserved = desktop_context;
        }

        if (resolver->release_armed_action !=
                PSTVNC_PRODUCT_ACTION_NONE) {
            int release_context_ok =
                resolver->release_armed_context ==
                    PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL ||
                (
                    resolver->release_armed_context ==
                        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP &&
                    desktop_context &&
                    resolver->gesture_started_in_desktop &&
                    resolver->desktop_context_preserved
                );

            if (
                resolver->release_armed_mask == old_mask &&
                pure_shrink &&
                release_context_ok)
                release_action = resolver->release_armed_action;

            /* Any exact-mask change consumes/cancels this release arm. */
            resolver->release_armed_mask = 0u;
            resolver->release_armed_action =
                PSTVNC_PRODUCT_ACTION_NONE;
            resolver->release_armed_context =
                PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;
        }

        resolver->observed_mask = mask;
        resolver->stable_polls = 0u;
        resolver->active_action = PSTVNC_PRODUCT_ACTION_NONE;
        resolver->active_trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_NONE;
        resolver->active_context = PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;
        resolver->hold_polls = 0u;

        /*
         * RELEASE may arm only while building the gesture. Shrinking from a
         * superset can never manufacture a release shortcut.
         */
        if (pure_growth && mask != 0u) {
            binding = product_action_lookup_exact(
                resolver,
                mask,
                desktop_context);

            if (
                binding != NULL &&
                binding->trigger ==
                    PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE) {
                resolver->release_armed_mask = mask;
                resolver->release_armed_action = binding->action;
                resolver->release_armed_context = binding->context;
            }
        }

        if (release_action != PSTVNC_PRODUCT_ACTION_NONE) {
            *action = release_action;

            if (mask != 0u) {
                resolver->latched = 1;
            } else {
                /*
                 * This sample already proves complete relevant release, so the
                 * next nonzero sample may begin a genuinely new gesture.
                 */
                product_action_reset_gesture(resolver);
            }

            return 1;
        }

        if (mask == 0u)
            product_action_reset_gesture(resolver);

        return 0;
    }

    if (mask == 0u)
        return 0;

    resolver->stable_polls++;

    binding =
        product_action_lookup_exact(
            resolver,
            mask,
            desktop_context);

    if (resolver->active_action != PSTVNC_PRODUCT_ACTION_NONE) {
        if (
            binding == NULL ||
            binding->action != resolver->active_action ||
            binding->trigger != resolver->active_trigger ||
            binding->context != resolver->active_context) {
            resolver->active_action = PSTVNC_PRODUCT_ACTION_NONE;
            resolver->active_trigger =
                PSTVNC_PRODUCT_ACTION_TRIGGER_NONE;
            resolver->active_context =
                PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;
            resolver->hold_polls = 0u;
        }
    }

    if (
        resolver->active_action == PSTVNC_PRODUCT_ACTION_NONE &&
        binding != NULL &&
        binding->trigger != PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE) {
        unsigned int settle_polls =
            PSTVNC_PRODUCT_ACTION_SETTLE_POLLS;

        if (
            binding->trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE &&
            product_action_has_hold_extension(
                resolver,
                mask,
                desktop_context))
            settle_polls =
                PSTVNC_PRODUCT_ACTION_EXTENDABLE_SETTLE_POLLS;

        if (resolver->stable_polls >= settle_polls) {
            resolver->active_action = binding->action;
            resolver->active_trigger = binding->trigger;
            resolver->active_context = binding->context;

            if (binding->trigger ==
                    PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE) {
                *action = binding->action;
                resolver->latched = 1;
                return 1;
            }

            resolver->hold_polls = 0u;
        }
    }

    if (
        resolver->active_action != PSTVNC_PRODUCT_ACTION_NONE &&
        resolver->active_trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD) {
        resolver->hold_polls++;

        if (resolver->hold_polls >= PSTVNC_PRODUCT_ACTION_HOLD_POLLS) {
            *action = resolver->active_action;
            resolver->latched = 1;
            return 1;
        }
    }

    return 0;
}
