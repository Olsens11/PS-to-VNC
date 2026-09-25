/*
 * File synopsis:
 * Proves the R28 platform-neutral semantic product-action binding resolver:
 * validation, exact-mask timing, release growth/shrink safety, hold extension,
 * post-fire latching, and desktop-context provenance.
 */

#include <stdio.h>
#include <string.h>

#include "input/product_action.h"

static int failures;

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            failures++; \
        } \
    } while (0)

static int observe(
    pstvnc_product_action_resolver_t *resolver,
    uint16_t buttons_down,
    int desktop_context,
    int epoch_started,
    pstvnc_product_action_t *action)
{
    pstvnc_controller_state_t state;

    memset(&state, 0, sizeof(state));
    state.buttons_down = buttons_down;
    state.connection_epoch_started = epoch_started;

    return pstvnc_product_action_resolver_observe(
        resolver,
        &state,
        desktop_context,
        action);
}

static void test_binding_validation(void)
{
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_L1,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
    };
    pstvnc_product_action_binding_t pair[2];

    CHECK(pstvnc_product_action_resolver_init(&resolver, NULL, 0u));

    binding.button_mask = 0u;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.button_mask = 0x10000u;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.button_mask = PSTVNC_CONTROLLER_BUTTON_L1;
    binding.action = PSTVNC_PRODUCT_ACTION_NONE;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.action = (pstvnc_product_action_t)99;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.action = PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    binding.trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_NONE;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.trigger = (pstvnc_product_action_trigger_t)99;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE;
    binding.context = PSTVNC_PRODUCT_ACTION_CONTEXT_NONE;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    binding.context = (pstvnc_product_action_context_t)99;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    pair[0].button_mask = PSTVNC_CONTROLLER_BUTTON_L1;
    pair[0].action = PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    pair[0].trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE;
    pair[0].context = PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP;
    pair[1] = pair[0];

    CHECK(!pstvnc_product_action_resolver_init(&resolver, pair, 2u));

    pair[1].trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD;
    pair[1].context = PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL;
    CHECK(!pstvnc_product_action_resolver_init(&resolver, pair, 2u));

    CHECK(!pstvnc_product_action_resolver_init(NULL, pair, 2u));
    CHECK(!pstvnc_product_action_resolver_init(&resolver, NULL, 1u));
}

static void test_injected_mpeg_calibration_settle_resolves_semantic_only(void)
{
    const pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_L2,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action = PSTVNC_PRODUCT_ACTION_NONE;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, &binding, 1u));
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);

    for (i = 1u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 1);
    CHECK(action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    /* Fired gesture is latched while its relevant button remains down. */
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);
    CHECK(action == PSTVNC_PRODUCT_ACTION_NONE);

    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);
}

static void test_exact_mask_change_resets_settle_progress(void)
{
    const pstvnc_product_action_binding_t bindings[] = {
        {
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
        },
        {
            PSTVNC_CONTROLLER_BUTTON_R1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
            PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL
        }
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, bindings, 2u));
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    for (i = 0u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS - 1u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        1,
        0,
        &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    for (i = 0u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS - 1u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 1);
    CHECK(action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
}

static void test_release_arming_requires_growth_and_latches(void)
{
    const pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;

    CHECK(pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        1,
        0,
        &action) == 0);

    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1,
        1,
        0,
        &action) == 1);
    CHECK(action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    /* Remaining relevant button keeps the fired gesture latched. */
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R1, 1, 0, &action) == 0);
    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);
}

static void test_release_growth_cancels_and_shrink_cannot_rearm(void)
{
    const pstvnc_product_action_binding_t bindings[] = {
        {
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
        },
        {
            PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
            PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL
        }
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;

    CHECK(pstvnc_product_action_resolver_init(&resolver, bindings, 2u));

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);
    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        1,
        0,
        &action) == 0);

    /* Shrinking back to the release subset cannot manufacture a new arm. */
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);
    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);
    CHECK(action == PSTVNC_PRODUCT_ACTION_NONE);

    /* Starting from the larger chord and shrinking is equally inert. */
    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        1,
        0,
        &action) == 0);
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);
    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);
    CHECK(action == PSTVNC_PRODUCT_ACTION_NONE);
}

static void test_settle_waits_for_hold_extension(void)
{
    const pstvnc_product_action_binding_t bindings[] = {
        {
            PSTVNC_CONTROLLER_BUTTON_L1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
        },
        {
            PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
            PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
            PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
            PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL
        }
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, bindings, 2u));
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    for (i = 1u; i < PSTVNC_PRODUCT_ACTION_EXTENDABLE_SETTLE_POLLS; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 1, 0, &action) == 1);
    CHECK(action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
}

static void test_hold_fires_once_at_full_threshold_then_latches(void)
{
    const pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
        PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL
    };
    const unsigned int fire_repeat =
        PSTVNC_PRODUCT_ACTION_SETTLE_POLLS +
        PSTVNC_PRODUCT_ACTION_HOLD_POLLS - 1u;
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, &binding, 1u));
    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        0,
        0,
        &action) == 0);

    for (i = 1u; i < fire_repeat; i++) {
        CHECK(observe(
            &resolver,
            PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
            0,
            0,
            &action) == 0);
    }

    CHECK(observe(
        &resolver,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        0,
        0,
        &action) == 1);
    CHECK(action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L1, 0, 0, &action) == 0);
    CHECK(action == PSTVNC_PRODUCT_ACTION_NONE);
    CHECK(observe(&resolver, 0u, 0, 0, &action) == 0);
}

static void test_desktop_context_cannot_be_acquired_or_reacquired_mid_gesture(void)
{
    const pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_L2,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, &binding, 1u));

    /* Gesture starts outside desktop and never becomes desktop-owned. */
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 0, 0, &action) == 0);
    for (i = 0u; i < 32u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);
    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);

    /* Gesture starts in desktop, loses context before firing, and cannot regain it. */
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);
    for (i = 0u; i < 3u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 0, 0, &action) == 0);
    for (i = 0u; i < 32u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_L2, 1, 0, &action) == 0);
    CHECK(observe(&resolver, 0u, 1, 0, &action) == 0);
}

static void test_connection_epoch_resets_pending_progress(void)
{
    const pstvnc_product_action_binding_t binding = {
        PSTVNC_CONTROLLER_BUTTON_R2,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP
    };
    pstvnc_product_action_resolver_t resolver;
    pstvnc_product_action_t action;
    unsigned int i;

    CHECK(pstvnc_product_action_resolver_init(&resolver, &binding, 1u));
    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R2, 1, 0, &action) == 0);

    for (i = 0u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS - 1u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R2, 1, 0, &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R2, 1, 1, &action) == 0);

    for (i = 0u; i < PSTVNC_PRODUCT_ACTION_SETTLE_POLLS - 1u; i++)
        CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R2, 1, 0, &action) == 0);

    CHECK(observe(&resolver, PSTVNC_CONTROLLER_BUTTON_R2, 1, 0, &action) == 1);
}

int main(void)
{
    test_binding_validation();
    test_injected_mpeg_calibration_settle_resolves_semantic_only();
    test_exact_mask_change_resets_settle_progress();
    test_release_arming_requires_growth_and_latches();
    test_release_growth_cancels_and_shrink_cannot_rearm();
    test_settle_waits_for_hold_extension();
    test_hold_fires_once_at_full_threshold_then_latches();
    test_desktop_context_cannot_be_acquired_or_reacquired_mid_gesture();
    test_connection_epoch_resets_pending_progress();

    if (failures != 0) {
        fprintf(stderr, "PRODUCT_ACTION_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("PRODUCT_ACTION_TEST=PASS");
    return 0;
}
