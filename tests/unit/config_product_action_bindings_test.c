#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/product_action_bindings.h"

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed line %d: %s\n", __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int parse_text(
    const char *text,
    pstvnc_config_product_action_bindings_t *model)
{
    return pstvnc_config_product_action_bindings_parse(
        text,
        strlen(text),
        model);
}

static void check_binding(
    const pstvnc_config_product_action_bindings_t *model,
    uint32_t mask,
    pstvnc_product_action_trigger_t trigger,
    pstvnc_product_action_context_t context)
{
    CHECK(model->binding_count == 1u);
    CHECK(model->bindings[0].button_mask == mask);
    CHECK(model->bindings[0].action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(model->bindings[0].trigger == trigger);
    CHECK(model->bindings[0].context == context);
}

static void test_zero_binding_documents(void)
{
    pstvnc_config_product_action_bindings_t model;

    memset(&model, 0xa5, sizeof(model));
    CHECK(parse_text("", &model));
    CHECK(model.binding_count == 0u);

    memset(&model, 0xa5, sizeof(model));
    CHECK(parse_text("# only a comment\n[other]\nx = y\n", &model));
    CHECK(model.binding_count == 0u);

    memset(&model, 0xa5, sizeof(model));
    CHECK(parse_text("[bindings]\nfuture_action = value\n", &model));
    CHECK(model.binding_count == 0u);
}

static void test_all_buttons_and_multi_button_chord(void)
{
    static const struct {
        const char *name;
        uint32_t mask;
    } cases[] = {
        {"select", PSTVNC_CONTROLLER_BUTTON_SELECT},
        {"l3", PSTVNC_CONTROLLER_BUTTON_L3},
        {"r3", PSTVNC_CONTROLLER_BUTTON_R3},
        {"start", PSTVNC_CONTROLLER_BUTTON_START},
        {"up", PSTVNC_CONTROLLER_BUTTON_UP},
        {"right", PSTVNC_CONTROLLER_BUTTON_RIGHT},
        {"down", PSTVNC_CONTROLLER_BUTTON_DOWN},
        {"left", PSTVNC_CONTROLLER_BUTTON_LEFT},
        {"l2", PSTVNC_CONTROLLER_BUTTON_L2},
        {"r2", PSTVNC_CONTROLLER_BUTTON_R2},
        {"l1", PSTVNC_CONTROLLER_BUTTON_L1},
        {"r1", PSTVNC_CONTROLLER_BUTTON_R1},
        {"triangle", PSTVNC_CONTROLLER_BUTTON_TRIANGLE},
        {"circle", PSTVNC_CONTROLLER_BUTTON_CIRCLE},
        {"cross", PSTVNC_CONTROLLER_BUTTON_CROSS},
        {"square", PSTVNC_CONTROLLER_BUTTON_SQUARE}
    };
    size_t i;

    for (i = 0u; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char document[160];
        pstvnc_config_product_action_bindings_t model;

        snprintf(
            document,
            sizeof(document),
            "[bindings]\nmpeg_calibration = %s, settle, desktop\n",
            cases[i].name);

        CHECK(parse_text(document, &model));
        check_binding(
            &model,
            cases[i].mask,
            PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
            PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    }

    {
        pstvnc_config_product_action_bindings_t model;
        CHECK(parse_text(
            "[bindings]\n"
            "mpeg_calibration = l1+r1+cross, hold, global\n",
            &model));
        check_binding(
            &model,
            PSTVNC_CONTROLLER_BUTTON_L1 |
                PSTVNC_CONTROLLER_BUTTON_R1 |
                PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
            PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL);
    }
}

static void test_chord_rejections(void)
{
    static const char * const invalid_values[] = {
        "l1+l1, settle, desktop",
        "+l1, settle, desktop",
        "l1+, settle, desktop",
        "l1++r1, settle, desktop",
        "bogus, settle, desktop",
        ", settle, desktop"
    };
    size_t i;

    for (i = 0u; i < sizeof(invalid_values) / sizeof(invalid_values[0]); i++) {
        char document[192];
        pstvnc_config_product_action_bindings_t model;
        memset(&model, 0x5a, sizeof(model));
        snprintf(document, sizeof(document),
            "[bindings]\nmpeg_calibration = %s\n", invalid_values[i]);
        CHECK(!parse_text(document, &model));
        CHECK(((unsigned char *)&model)[0] == 0x5a);
    }
}

static void test_trigger_and_context_mapping(void)
{
    pstvnc_config_product_action_bindings_t model;

    CHECK(parse_text("[bindings]\nmpeg_calibration=l1,settle,desktop\n", &model));
    check_binding(&model, PSTVNC_CONTROLLER_BUTTON_L1,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);

    CHECK(parse_text("[bindings]\nmpeg_calibration=l1,release,global\n", &model));
    check_binding(&model, PSTVNC_CONTROLLER_BUTTON_L1,
        PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL);

    CHECK(parse_text("[bindings]\nmpeg_calibration=l1,hold,desktop\n", &model));
    check_binding(&model, PSTVNC_CONTROLLER_BUTTON_L1,
        PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);

    CHECK(!parse_text("[bindings]\nmpeg_calibration=l1,,desktop\n", &model));
    CHECK(!parse_text("[bindings]\nmpeg_calibration=l1,unknown,desktop\n", &model));
    CHECK(!parse_text("[bindings]\nmpeg_calibration=l1,settle,\n", &model));
    CHECK(!parse_text("[bindings]\nmpeg_calibration=l1,settle,unknown\n", &model));
}

static void test_normalized_example_and_document_tolerance(void)
{
    pstvnc_config_product_action_bindings_t model;

    CHECK(parse_text(
        "  # heading\r\n"
        "[future]\r\n"
        "mpeg_calibration = this,is,opaque\r\n"
        "\r\n"
        " [bindings] \r\n"
        " future_setting = anything = remains opaque # comment\r\n"
        " mpeg_calibration = l1 + r1 , settle , desktop # chosen explicitly\r\n",
        &model));

    check_binding(
        &model,
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_R1,
        PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE,
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
}

static void test_recognized_ambiguity_and_atomic_failure(void)
{
    static const char * const invalid_documents[] = {
        "[bindings]\nmpeg_calibration=l1,settle,desktop\n[bindings]\n",
        "[bindings]\nmpeg_calibration=l1,settle,desktop\nmpeg_calibration=r1,hold,global\n",
        "[bindings\nmpeg_calibration=l1,settle,desktop\n",
        "[bindings] trailing\nmpeg_calibration=l1,settle,desktop\n",
        "[bindings]\nmpeg_calibration l1,settle,desktop\n",
        "[bindings]\nmpeg_calibration=l1,settle\n",
        "[bindings]\nmpeg_calibration=l1,settle,desktop,extra\n",
        "[bindings]\nmpeg_calibration=l1,settle,desktop=extra\n"
    };
    size_t i;

    for (i = 0u; i < sizeof(invalid_documents) / sizeof(invalid_documents[0]); i++) {
        pstvnc_config_product_action_bindings_t before;
        pstvnc_config_product_action_bindings_t model;

        memset(&before, 0xa7, sizeof(before));
        model = before;
        CHECK(!parse_text(invalid_documents[i], &model));
        CHECK(memcmp(&model, &before, sizeof(model)) == 0);
    }
}

static void test_bounds_and_embedded_nul(void)
{
    pstvnc_config_product_action_bindings_t before;
    pstvnc_config_product_action_bindings_t model;
    char embedded[] = "[bindings]\nmpeg_calibration=l1,settle,desktop\nX";
    char *oversized;
    char line[PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES + 2u];

    memset(&before, 0x3c, sizeof(before));

    embedded[12] = '\0';
    model = before;
    CHECK(!pstvnc_config_product_action_bindings_parse(
        embedded, sizeof(embedded) - 1u, &model));
    CHECK(memcmp(&model, &before, sizeof(model)) == 0);

    oversized = malloc(
        PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES + 1u);
    CHECK(oversized != NULL);
    if (oversized != NULL) {
        memset(oversized, 'x',
            PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES + 1u);
        model = before;
        CHECK(!pstvnc_config_product_action_bindings_parse(
            oversized,
            PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES + 1u,
            &model));
        CHECK(memcmp(&model, &before, sizeof(model)) == 0);
        free(oversized);
    }

    memset(line, 'x', sizeof(line));
    line[sizeof(line) - 1u] = '\n';
    model = before;
    CHECK(!pstvnc_config_product_action_bindings_parse(
        line, sizeof(line), &model));
    CHECK(memcmp(&model, &before, sizeof(model)) == 0);
}

static void test_canonical_round_trip_and_output_atomicity(void)
{
    pstvnc_config_product_action_bindings_t model;
    pstvnc_config_product_action_bindings_t reparsed;
    char output[160];
    size_t output_length = 999u;

    CHECK(parse_text(
        "[bindings]\n"
        "mpeg_calibration = cross+l1+select, release, global\n",
        &model));

    memset(output, 0xcc, sizeof(output));
    CHECK(pstvnc_config_product_action_bindings_format(
        &model, output, sizeof(output), &output_length));
    CHECK(strcmp(
        output,
        "[bindings]\n"
        "mpeg_calibration = select+l1+cross, release, global\n") == 0);
    CHECK(output_length == strlen(output));

    CHECK(pstvnc_config_product_action_bindings_parse(
        output, output_length, &reparsed));
    CHECK(reparsed.binding_count == model.binding_count);
    CHECK(memcmp(&reparsed.bindings[0], &model.bindings[0],
        sizeof(model.bindings[0])) == 0);

    {
        char too_small[12];
        char before[sizeof(too_small)];
        size_t length_before = 321u;
        size_t length = length_before;

        memset(too_small, 0x6d, sizeof(too_small));
        memcpy(before, too_small, sizeof(before));
        CHECK(!pstvnc_config_product_action_bindings_format(
            &model, too_small, sizeof(too_small), &length));
        CHECK(memcmp(too_small, before, sizeof(before)) == 0);
        CHECK(length == length_before);
    }

    memset(&model, 0, sizeof(model));
    CHECK(pstvnc_config_product_action_bindings_format(
        &model, output, sizeof(output), &output_length));
    CHECK(strcmp(output, "[bindings]\n") == 0);
    CHECK(pstvnc_config_product_action_bindings_parse(
        output, output_length, &reparsed));
    CHECK(reparsed.binding_count == 0u);
}

int main(void)
{
    test_zero_binding_documents();
    test_all_buttons_and_multi_button_chord();
    test_chord_rejections();
    test_trigger_and_context_mapping();
    test_normalized_example_and_document_tolerance();
    test_recognized_ambiguity_and_atomic_failure();
    test_bounds_and_embedded_nul();
    test_canonical_round_trip_and_output_atomicity();

    if (failures != 0) {
        fprintf(stderr, "CONFIG_PRODUCT_ACTION_BINDINGS_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS");
    return 0;
}
