/*
 * File synopsis:
 * Defines Configuration-owned human-readable product-action binding authority.
 * The model contains only validated Input-owned R28 binding values and performs
 * no persistence, runtime installation, product routing, or MPEG side effect.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30.
 */

#ifndef PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_H
#define PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_H

#include <stddef.h>

#include "input/product_action.h"

#define PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT 1u
#define PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES 4096u
#define PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES 256u

typedef struct pstvnc_config_product_action_bindings {
    pstvnc_product_action_binding_t
        bindings[PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT];
    size_t binding_count;
} pstvnc_config_product_action_bindings_t;

/*
 * Parse one explicit-length human-readable document atomically.
 *
 * Missing [bindings] or mpeg_calibration is a valid zero-binding model.
 * On failure, *bindings remains byte-for-byte unchanged.
 */
int pstvnc_config_product_action_bindings_parse(
    const char *document,
    size_t document_length,
    pstvnc_config_product_action_bindings_t *bindings);

/*
 * Format the recognized model in one canonical lowercase representation.
 * On failure, output bytes and *output_length remain unchanged.
 */
int pstvnc_config_product_action_bindings_format(
    const pstvnc_config_product_action_bindings_t *bindings,
    char *output,
    size_t output_capacity,
    size_t *output_length);

#endif /* PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_H */
