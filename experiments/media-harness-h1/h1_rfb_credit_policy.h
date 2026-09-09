/*
 * File synopsis:
 * Defines the pure decision rule for returning logical RFB channel-1 credit.
 *
 * The policy owns no queue, semaphore, socket, or CONFIG state. It answers only
 * whether the currently accumulated parser-consumed bytes should be returned
 * now. Keeping this rule host-testable makes RFB flow-control experiments
 * explicit without coupling policy tests to PS2 kernel primitives.
 *
 * Context: RFB_CREDIT_POLICY_DECISION.md; RFB_MUX_CP2F_CONFIG_V4.md.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_CREDIT_POLICY_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_CREDIT_POLICY_H

#include <stdint.h>

int pstvnc_h1_rfb_credit_should_return(
    uint32_t pending_bytes,
    uint32_t batch_bytes,
    int flush_on_empty,
    int return_enabled,
    int queue_empty);

#endif
