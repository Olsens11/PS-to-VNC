/*
 * File synopsis:
 * Implements the Application-owned one-shot desired product-binding
 * acquisition process. It starts from explicit zero authority, retrieves one
 * bounded raw human-config document through Management, parses the exact byte
 * count through Configuration, and publishes one immutable-by-convention
 * resident snapshot or a nonfatal zero fallback.
 *
 * Raw document storage is temporary stack state. Live Input installation,
 * PRODUCT_ACTION routing, media effects, persistence and runtime reload policy
 * are deliberately outside this owner.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32.
 */

#include <stddef.h>
#include <string.h>

#include "app_product_bindings.h"
#include "management/config_get.h"

#if PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES !=     PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES
#error "R32 requires one shared 4096-byte management/config document bound"
#endif

int pstvnc_app_product_bindings_acquire(
    pstvnc_app_product_bindings_snapshot_t *snapshot)
{
    unsigned char document[PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES];
    pstvnc_app_product_bindings_snapshot_t candidate;
    size_t document_length = 0u;

    if (snapshot == NULL)
        return 0;

    /*
     * Zero desired authority exists before Management is touched. R30 defines
     * an all-zero model as the valid no-binding result, so neither retrieval nor
     * parse failure can accidentally inherit or guess a physical mapping.
     */
    memset(&candidate, 0, sizeof(candidate));
    candidate.status = PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO;

    if (pstvnc_management_config_get(
            document,
            sizeof(document),
            &document_length) < 0) {
        *snapshot = candidate;
        return 1;
    }

    candidate.status = PSTVNC_APP_PRODUCT_BINDINGS_STATUS_INVALID_CONFIG_ZERO;

    /*
     * Pass exactly the Management-owned body bytes and count to R30. No NUL
     * terminator, whitespace normalization or other reinterpretation occurs at
     * the Application boundary.
     *
     * R30 publication is atomic. Because candidate.desired began zero, a parse
     * rejection leaves the fallback model exact and complete.
     */
    if (!pstvnc_config_product_action_bindings_parse(
            (const char *)document,
            document_length,
            &candidate.desired)) {
        *snapshot = candidate;
        return 1;
    }

    candidate.status = PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID;
    *snapshot = candidate;
    return 1;
}
