/*
 * File synopsis:
 * Defines the generic RFB live-flow policy used at the existing complete
 * server-message/request scheduling boundary.
 *
 * This module owns only request accounting, global freeze/thaw state, remote
 * visual-publication permission, and one-shot post-thaw FULL-refresh debt. It
 * does not own the RFB parser, socket/Transport, framebuffer storage, MPEG,
 * calibration, presentation/composition, input, or application orchestration.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md, A004-RFB-FREEZE-REFRESH-R2;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q7.
 */

#ifndef PSTVNC_RFB_FLOW_POLICY_H
#define PSTVNC_RFB_FLOW_POLICY_H

typedef enum pstvnc_rfb_flow_request {
    PSTVNC_RFB_FLOW_REQUEST_HOLD = 0,
    PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL,
    PSTVNC_RFB_FLOW_REQUEST_FULL
} pstvnc_rfb_flow_request_t;

typedef struct pstvnc_rfb_flow_policy {
    unsigned frozen : 1;
    unsigned request_outstanding : 1;
    unsigned full_refresh_pending : 1;
} pstvnc_rfb_flow_policy_t;

/* Initialize ordinary thawed live cadence with no request outstanding. */
void pstvnc_rfb_flow_policy_init(
    pstvnc_rfb_flow_policy_t *policy);

/*
 * Apply generic global freeze/thaw intent.
 *
 * Only 0 and 1 are accepted. A genuine frozen->thawed transition records one
 * FULL-refresh obligation. Repeating the already-current state is idempotent.
 * A pending obligation is coalesced rather than multiplied until recovery work
 * advances.
 */
int pstvnc_rfb_flow_policy_set_frozen(
    pstvnc_rfb_flow_policy_t *policy,
    int frozen);

/*
 * Report the request decision at the existing RFB safe scheduling boundary.
 *
 * This function is deliberately non-mutating. Repeated inspection can never
 * consume FULL-refresh debt. HOLD is returned while frozen or while a prior
 * framebuffer request remains outstanding.
 */
pstvnc_rfb_flow_request_t
pstvnc_rfb_flow_policy_next_request(
    const pstvnc_rfb_flow_policy_t *policy);

/*
 * Record successful wire transmission of exactly the request currently owed.
 *
 * FULL-refresh debt is consumed here, never by next_request(). Invalid,
 * duplicate, frozen, or wrong-kind accounting fails without changing state.
 */
int pstvnc_rfb_flow_policy_record_request_sent(
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_rfb_flow_request_t request);

/*
 * Record completion of the one outstanding framebuffer-update response.
 *
 * Callers invoke this only after the RFB parser has completed that server
 * message at its existing safe boundary. Completion remains valid while frozen:
 * protocol/framebuffer truth may advance even when visual publication is
 * suppressed.
 */
int pstvnc_rfb_flow_policy_record_update_complete(
    pstvnc_rfb_flow_policy_t *policy);

/* Report whether a framebuffer request is still protocol-owned/in flight. */
int pstvnc_rfb_flow_policy_has_outstanding_request(
    const pstvnc_rfb_flow_policy_t *policy);

/*
 * Report whether newly completed remote framebuffer visuals may be published.
 * This is a global ownership-gap gate only; callers must not equate it with
 * MPEG-active or MPEG-retiring state.
 */
int pstvnc_rfb_flow_policy_allows_remote_publication(
    const pstvnc_rfb_flow_policy_t *policy);

#endif /* PSTVNC_RFB_FLOW_POLICY_H */
