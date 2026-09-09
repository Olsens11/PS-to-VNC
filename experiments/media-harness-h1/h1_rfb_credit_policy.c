/*
 * File synopsis:
 * Implements the host-testable RFB receiver-credit return decision.
 *
 * Returned credit is based only on bytes already consumed by the RFB parser.
 * The configured batch threshold is the ordinary return trigger; optional
 * flush-on-empty permits a smaller pending amount to be returned when the
 * logical RFB receive queue has drained completely.
 */

#include "h1_rfb_credit_policy.h"

int pstvnc_h1_rfb_credit_should_return(
    uint32_t pending_bytes,
    uint32_t batch_bytes,
    int flush_on_empty,
    int return_enabled,
    int queue_empty)
{
    if (!return_enabled || pending_bytes == 0u)
        return 0;

    if (batch_bytes != 0u && pending_bytes >= batch_bytes)
        return 1;

    return flush_on_empty && queue_empty;
}
