/*
 * File synopsis:
 * Declares the Application-owned resident desired product-action binding
 * snapshot and the one-shot startup acquisition process that composes raw
 * Management retrieval with side-effect-free Configuration parsing.
 *
 * The snapshot is desired authority only. This module does not install live
 * Input bindings, route PRODUCT_ACTION, start MPEG, mutate UI/RFB/Transport,
 * persist configuration, or implement reload/recovery policy.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32.
 */

#ifndef PSTVNC_APP_PRODUCT_BINDINGS_H
#define PSTVNC_APP_PRODUCT_BINDINGS_H

#include "config/product_action_bindings.h"

typedef enum pstvnc_app_product_bindings_status {
    PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID = 0,
    PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO,
    PSTVNC_APP_PRODUCT_BINDINGS_STATUS_INVALID_CONFIG_ZERO
} pstvnc_app_product_bindings_status_t;

typedef struct pstvnc_app_product_bindings_snapshot {
    pstvnc_config_product_action_bindings_t desired;
    pstvnc_app_product_bindings_status_t status;
} pstvnc_app_product_bindings_snapshot_t;

/*
 * Acquire one resident desired-binding snapshot.
 *
 * Management retrieval and recognized-config validation failures are ordinary
 * nonfatal outcomes: they publish an explicit zero-binding model with the
 * corresponding status. A valid fetched document publishes R30's exact typed
 * model. Return zero only for an invalid caller-owned output pointer.
 */
int pstvnc_app_product_bindings_acquire(
    pstvnc_app_product_bindings_snapshot_t *snapshot);

#endif /* PSTVNC_APP_PRODUCT_BINDINGS_H */
