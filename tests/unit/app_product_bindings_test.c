/*
 * File synopsis:
 * Proves R32's Application-owned desired-binding acquisition composition with
 * the real R30 parser and a deterministic R31 Management stub. Tests cover
 * exact-byte parsing, zero fallbacks, explicit non-default binding retention,
 * empty documents, and fresh publication without live Input installation.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "app_product_bindings.h"
#include "input/controller.h"
#include "management/config_get.h"

static int failures;
static int management_result;
static unsigned char management_body[
    PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES];
static size_t management_body_length;
static size_t management_calls;
static size_t management_observed_capacity;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed line %d: %s\n", __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void reset_management(void)
{
    management_result = 0;
    memset(management_body, 0, sizeof(management_body));
    management_body_length = 0u;
    management_calls = 0u;
    management_observed_capacity = 0u;
}

static void set_management_text(const char *text)
{
    size_t length = strlen(text);

    CHECK(length <= sizeof(management_body));
    if (length > sizeof(management_body))
        return;

    memcpy(management_body, text, length);
    management_body_length = length;
}

int pstvnc_management_config_get(
    unsigned char *body,
    size_t body_capacity,
    size_t *body_length)
{
    management_calls++;
    management_observed_capacity = body_capacity;

    if (management_result < 0)
        return -1;

    CHECK(body != NULL);
    CHECK(body_length != NULL);
    CHECK(body_capacity == PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES);
    CHECK(management_body_length <= body_capacity);

    if (body == NULL ||
        body_length == NULL ||
        management_body_length > body_capacity)
        return -1;

    /*
     * Poison bytes after the returned body. A helper that incorrectly passes
     * the full capacity rather than the exact returned length to R30 will fail
     * the otherwise-valid tests below.
     */
    memset(body, 0xa5, body_capacity);
    if (management_body_length > 0u)
        memcpy(body, management_body, management_body_length);

    *body_length = management_body_length;
    return 0;
}

static void check_zero_snapshot(
    const pstvnc_app_product_bindings_snapshot_t *snapshot,
    pstvnc_app_product_bindings_status_t expected_status)
{
    CHECK(snapshot != NULL);
    if (snapshot == NULL)
        return;

    CHECK(snapshot->status == expected_status);
    CHECK(snapshot->desired.binding_count == 0u);
    CHECK(snapshot->desired.bindings[0].button_mask == 0u);
    CHECK(snapshot->desired.bindings[0].action == PSTVNC_PRODUCT_ACTION_NONE);
    CHECK(snapshot->desired.bindings[0].trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_NONE);
    CHECK(snapshot->desired.bindings[0].context == PSTVNC_PRODUCT_ACTION_CONTEXT_NONE);
}

static void test_invalid_output_pointer_does_not_fetch(void)
{
    reset_management();

    CHECK(!pstvnc_app_product_bindings_acquire(NULL));
    CHECK(management_calls == 0u);
}

static void test_management_failure_publishes_nonfatal_zero(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;

    reset_management();
    management_result = -1;
    memset(&snapshot, 0x7c, sizeof(snapshot));

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(management_calls == 1u);
    CHECK(management_observed_capacity ==
        PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES);
    check_zero_snapshot(
        &snapshot,
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO);
}

static void test_invalid_config_publishes_nonfatal_zero(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;
    static const unsigned char invalid_document[] = {
        '[','b','i','n','d','i','n','g','s',']','\n',
        'm','p','e','g','_','c','a','l','i','b','r','a','t','i','o','n',
        ' ','=',' ','l','1','+',0,'c','i','r','c','l','e',',',' ',
        'h','o','l','d',',',' ','g','l','o','b','a','l','\n'
    };

    reset_management();
    memcpy(
        management_body,
        invalid_document,
        sizeof(invalid_document));
    management_body_length = sizeof(invalid_document);
    memset(&snapshot, 0x6b, sizeof(snapshot));

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(management_calls == 1u);
    check_zero_snapshot(
        &snapshot,
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_INVALID_CONFIG_ZERO);
}

static void test_explicit_nondefault_binding_is_retained_exactly(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;

    reset_management();
    set_management_text(
        "# caller-selected binding\n"
        "[bindings]\n"
        "mpeg_calibration = l1+circle, hold, global\n");
    memset(&snapshot, 0x5a, sizeof(snapshot));

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(management_calls == 1u);
    CHECK(snapshot.status == PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID);
    CHECK(snapshot.desired.binding_count == 1u);
    CHECK(snapshot.desired.bindings[0].button_mask ==
        (PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_CIRCLE));
    CHECK(snapshot.desired.bindings[0].action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(snapshot.desired.bindings[0].trigger ==
        PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD);
    CHECK(snapshot.desired.bindings[0].context ==
        PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL);
}

static void test_exact_returned_length_excludes_poison_tail(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;

    reset_management();
    set_management_text(
        "[bindings]\n"
        "mpeg_calibration = square+r2, release, desktop\n");

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(snapshot.status == PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID);
    CHECK(snapshot.desired.binding_count == 1u);
    CHECK(snapshot.desired.bindings[0].button_mask ==
        (PSTVNC_CONTROLLER_BUTTON_SQUARE | PSTVNC_CONTROLLER_BUTTON_R2));
    CHECK(snapshot.desired.bindings[0].trigger ==
        PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE);
    CHECK(snapshot.desired.bindings[0].context ==
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
}

static void test_empty_success_is_valid_zero_document(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;

    reset_management();

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(management_calls == 1u);
    check_zero_snapshot(
        &snapshot,
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID);
}

static void test_later_fallback_does_not_inherit_prior_binding(void)
{
    pstvnc_app_product_bindings_snapshot_t snapshot;

    reset_management();
    set_management_text(
        "[bindings]\n"
        "mpeg_calibration = triangle+down, settle, global\n");

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    CHECK(snapshot.desired.binding_count == 1u);

    reset_management();
    management_result = -1;

    CHECK(pstvnc_app_product_bindings_acquire(&snapshot));
    check_zero_snapshot(
        &snapshot,
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO);
}

int main(void)
{
    test_invalid_output_pointer_does_not_fetch();
    test_management_failure_publishes_nonfatal_zero();
    test_invalid_config_publishes_nonfatal_zero();
    test_explicit_nondefault_binding_is_retained_exactly();
    test_exact_returned_length_excludes_poison_tail();
    test_empty_success_is_valid_zero_document();
    test_later_fallback_does_not_inherit_prior_binding();

    if (failures != 0) {
        fprintf(
            stderr,
            "APP_PRODUCT_BINDINGS_TEST=FAIL count=%d\n",
            failures);
        return 1;
    }

    puts("APP_PRODUCT_BINDINGS_TEST=PASS");
    return 0;
}
