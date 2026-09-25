/*
 * File synopsis:
 * Parses and formats the bounded human-readable [bindings] configuration model.
 * It maps exact symbolic controller/trigger/context tokens onto accepted R28
 * typed binding values, publishes only after complete validation, and supplies
 * no default physical policy or runtime/product effect.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30;
 * docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md.
 */

#include <stdint.h>
#include <string.h>

#include "product_action_bindings.h"
#include "text.h"

#define PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_FORMAT_BYTES 128u

typedef enum config_binding_section {
    CONFIG_BINDING_SECTION_NONE = 0,
    CONFIG_BINDING_SECTION_BINDINGS = 1,
    CONFIG_BINDING_SECTION_UNKNOWN = 2
} config_binding_section_t;

typedef struct config_button_token {
    const char *name;
    uint32_t mask;
} config_button_token_t;

static const config_button_token_t config_button_tokens[] = {
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

static size_t config_button_token_count(void)
{
    return sizeof(config_button_tokens) / sizeof(config_button_tokens[0]);
}

static const config_button_token_t *config_find_button_token(
    const char *name)
{
    size_t i;

    for (i = 0u; i < config_button_token_count(); i++) {
        if (strcmp(name, config_button_tokens[i].name) == 0)
            return &config_button_tokens[i];
    }

    return NULL;
}

static int config_parse_chord(
    char *text,
    uint32_t *button_mask)
{
    char *token;
    char *separator;
    uint32_t mask = 0u;

    if (text == NULL || button_mask == NULL || text[0] == '\0')
        return 0;

    token = text;

    for (;;) {
        const config_button_token_t *button;

        separator = strchr(token, '+');
        if (separator != NULL)
            *separator = '\0';

        token = pstvnc_config_trim_left(token);
        pstvnc_config_trim_right(token);

        if (token[0] == '\0')
            return 0;

        button = config_find_button_token(token);
        if (button == NULL || (mask & button->mask) != 0u)
            return 0;

        mask |= button->mask;

        if (separator == NULL)
            break;

        token = separator + 1;
    }

    if (mask == 0u ||
        (mask & ~(uint32_t)PSTVNC_CONTROLLER_BUTTON_MASK) != 0u)
        return 0;

    *button_mask = mask;
    return 1;
}

static int config_parse_trigger(
    const char *text,
    pstvnc_product_action_trigger_t *trigger)
{
    if (text == NULL || trigger == NULL)
        return 0;

    if (strcmp(text, "settle") == 0)
        *trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE;
    else if (strcmp(text, "release") == 0)
        *trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE;
    else if (strcmp(text, "hold") == 0)
        *trigger = PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD;
    else
        return 0;

    return 1;
}

static int config_parse_context(
    const char *text,
    pstvnc_product_action_context_t *context)
{
    if (text == NULL || context == NULL)
        return 0;

    if (strcmp(text, "desktop") == 0)
        *context = PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP;
    else if (strcmp(text, "global") == 0)
        *context = PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL;
    else
        return 0;

    return 1;
}

static int config_validate_binding(
    const pstvnc_product_action_binding_t *binding)
{
    pstvnc_product_action_resolver_t validator;

    return pstvnc_product_action_resolver_init(
        &validator,
        binding,
        1u);
}

static int config_parse_binding_value(
    char *value,
    pstvnc_product_action_binding_t *binding)
{
    char *first_comma;
    char *second_comma;
    char *chord;
    char *trigger_text;
    char *context_text;
    pstvnc_product_action_binding_t candidate;

    if (value == NULL || binding == NULL)
        return 0;

    if (strchr(value, '=') != NULL)
        return 0;

    first_comma = strchr(value, ',');
    if (first_comma == NULL)
        return 0;

    *first_comma = '\0';
    second_comma = strchr(first_comma + 1, ',');
    if (second_comma == NULL || strchr(second_comma + 1, ',') != NULL)
        return 0;

    *second_comma = '\0';

    chord = pstvnc_config_trim_left(value);
    pstvnc_config_trim_right(chord);

    trigger_text = pstvnc_config_trim_left(first_comma + 1);
    pstvnc_config_trim_right(trigger_text);

    context_text = pstvnc_config_trim_left(second_comma + 1);
    pstvnc_config_trim_right(context_text);

    memset(&candidate, 0, sizeof(candidate));
    candidate.action = PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;

    if (!config_parse_chord(chord, &candidate.button_mask) ||
        !config_parse_trigger(trigger_text, &candidate.trigger) ||
        !config_parse_context(context_text, &candidate.context) ||
        !config_validate_binding(&candidate))
        return 0;

    *binding = candidate;
    return 1;
}

static int config_line_is_recognized_key_without_assignment(
    const char *line)
{
    static const char key[] = "mpeg_calibration";
    size_t key_length = sizeof(key) - 1u;

    if (strncmp(line, key, key_length) != 0)
        return 0;

    return
        line[key_length] == '\0' ||
        line[key_length] == ' ' ||
        line[key_length] == '\t';
}

int pstvnc_config_product_action_bindings_parse(
    const char *document,
    size_t document_length,
    pstvnc_config_product_action_bindings_t *bindings)
{
    pstvnc_config_product_action_bindings_t candidate;
    config_binding_section_t section = CONFIG_BINDING_SECTION_NONE;
    size_t offset = 0u;
    int bindings_section_seen = 0;
    int recognized_key_seen = 0;

    if (document == NULL || bindings == NULL)
        return 0;

    if (document_length >
            PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES)
        return 0;

    if (memchr(document, '\0', document_length) != NULL)
        return 0;

    memset(&candidate, 0, sizeof(candidate));

    while (offset < document_length) {
        char line[PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES + 1u];
        size_t line_length = 0u;
        size_t copy_length;
        char *text;
        char *comment;

        while (offset + line_length < document_length &&
               document[offset + line_length] != '\n')
            line_length++;

        if (line_length > PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES)
            return 0;

        copy_length = line_length;
        if (copy_length > 0u && document[offset + copy_length - 1u] == '\r')
            copy_length--;

        memcpy(line, document + offset, copy_length);
        line[copy_length] = '\0';

        offset += line_length;
        if (offset < document_length && document[offset] == '\n')
            offset++;

        comment = strchr(line, '#');
        if (comment != NULL)
            *comment = '\0';

        text = pstvnc_config_trim_left(line);
        pstvnc_config_trim_right(text);

        if (text[0] == '\0')
            continue;

        if (text[0] == '[') {
            size_t text_length = strlen(text);
            char *section_name;

            if (text_length < 3u || text[text_length - 1u] != ']')
                return 0;

            text[text_length - 1u] = '\0';
            section_name = text + 1;

            if (section_name[0] == '\0' || strchr(section_name, '[') != NULL ||
                strchr(section_name, ']') != NULL)
                return 0;

            if (strcmp(section_name, "bindings") == 0) {
                if (bindings_section_seen)
                    return 0;

                bindings_section_seen = 1;
                section = CONFIG_BINDING_SECTION_BINDINGS;
            } else {
                section = CONFIG_BINDING_SECTION_UNKNOWN;
            }

            continue;
        }

        if (section == CONFIG_BINDING_SECTION_BINDINGS) {
            char *equals = strchr(text, '=');
            char *key;
            char *value;

            if (equals == NULL) {
                if (config_line_is_recognized_key_without_assignment(text))
                    return 0;
                continue;
            }

            *equals = '\0';
            key = pstvnc_config_trim_left(text);
            pstvnc_config_trim_right(key);

            if (strcmp(key, "mpeg_calibration") != 0)
                continue;

            if (recognized_key_seen)
                return 0;

            value = pstvnc_config_trim_left(equals + 1);
            pstvnc_config_trim_right(value);

            if (value[0] == '\0' ||
                !config_parse_binding_value(value, &candidate.bindings[0]))
                return 0;

            candidate.binding_count = 1u;
            recognized_key_seen = 1;
        }
    }

    *bindings = candidate;
    return 1;
}

static int config_append_text(
    char *buffer,
    size_t capacity,
    size_t *length,
    const char *text)
{
    size_t text_length;

    if (buffer == NULL || length == NULL || text == NULL)
        return 0;

    text_length = strlen(text);
    if (*length > capacity || text_length > capacity - *length)
        return 0;

    memcpy(buffer + *length, text, text_length);
    *length += text_length;
    return 1;
}

static const char *config_trigger_name(
    pstvnc_product_action_trigger_t trigger)
{
    switch (trigger) {
        case PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE:
            return "settle";
        case PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE:
            return "release";
        case PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD:
            return "hold";
        default:
            return NULL;
    }
}

static const char *config_context_name(
    pstvnc_product_action_context_t context)
{
    switch (context) {
        case PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP:
            return "desktop";
        case PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL:
            return "global";
        default:
            return NULL;
    }
}

int pstvnc_config_product_action_bindings_format(
    const pstvnc_config_product_action_bindings_t *bindings,
    char *output,
    size_t output_capacity,
    size_t *output_length)
{
    char formatted[PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_FORMAT_BYTES];
    size_t length = 0u;

    if (bindings == NULL || output == NULL || output_length == NULL)
        return 0;

    if (bindings->binding_count >
            PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT)
        return 0;

    if (!config_append_text(
            formatted,
            sizeof(formatted),
            &length,
            "[bindings]\n"))
        return 0;

    if (bindings->binding_count == 1u) {
        const pstvnc_product_action_binding_t *binding = &bindings->bindings[0];
        const char *trigger_name;
        const char *context_name;
        uint32_t remaining_mask;
        size_t i;
        int first_button = 1;

        if (binding->action != PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION ||
            !config_validate_binding(binding))
            return 0;

        trigger_name = config_trigger_name(binding->trigger);
        context_name = config_context_name(binding->context);
        if (trigger_name == NULL || context_name == NULL)
            return 0;

        if (!config_append_text(
                formatted,
                sizeof(formatted),
                &length,
                "mpeg_calibration = "))
            return 0;

        remaining_mask = binding->button_mask;

        for (i = 0u; i < config_button_token_count(); i++) {
            const config_button_token_t *button = &config_button_tokens[i];

            if ((remaining_mask & button->mask) == 0u)
                continue;

            if (!first_button &&
                !config_append_text(formatted, sizeof(formatted), &length, "+"))
                return 0;

            if (!config_append_text(
                    formatted,
                    sizeof(formatted),
                    &length,
                    button->name))
                return 0;

            remaining_mask &= ~button->mask;
            first_button = 0;
        }

        if (first_button || remaining_mask != 0u)
            return 0;

        if (!config_append_text(formatted, sizeof(formatted), &length, ", ") ||
            !config_append_text(formatted, sizeof(formatted), &length, trigger_name) ||
            !config_append_text(formatted, sizeof(formatted), &length, ", ") ||
            !config_append_text(formatted, sizeof(formatted), &length, context_name) ||
            !config_append_text(formatted, sizeof(formatted), &length, "\n"))
            return 0;
    }

    if (output_capacity <= length)
        return 0;

    memcpy(output, formatted, length);
    output[length] = '\0';
    *output_length = length;
    return 1;
}
