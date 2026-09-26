/*
 * File synopsis:
 * Implements R41's Application-owned AUDIO lifecycle coordinator using only
 * accepted public lower-owner seams. Start sequencing is R37 runtime/operation
 * acquisition, then R40 exact-ticket Transport activation, then the accepted
 * AUDIO session. Steady service uses R38 nonblocking completion publication.
 *
 * The first-presentation gate observes Transport AUDIO status only; it never
 * consumes PCM or arms/resets the common media clock. Post-activation abnormal
 * cleanup cannot touch local AUDIO ownership until the exact old Transport
 * ticket is proven retained by enclosing-session abort.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41.
 */

#include "app_audio_product.h"

#include <string.h>

static int pstvnc_app_audio_product_session_has_ownership(
    const pstvnc_audio_session_t *session)
{
    return session != NULL &&
        (session->initialized ||
         session->thread_created ||
         session->thread_started ||
         session->playback_buffer != NULL ||
         session->worker_stack != NULL);
}

static void pstvnc_app_audio_product_record_first_failure(
    pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_result_t result)
{
    if (product->first_failure == PSTVNC_APP_AUDIO_PRODUCT_OK &&
        result != PSTVNC_APP_AUDIO_PRODUCT_OK)
        product->first_failure = result;
}

static pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_fault(
    pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_result_t result)
{
    pstvnc_app_audio_product_record_first_failure(product, result);
    product->state = PSTVNC_APP_AUDIO_PRODUCT_FAULTED;
    product->last_result = result;
    if (product->transport_activation_attempted)
        product->requires_session_abort = 1;
    return result;
}

static void pstvnc_app_audio_product_refresh_runtime_ownership(
    pstvnc_app_audio_product_t *product)
{
    product->runtime_owned = product->runtime.resources_owned != 0;
}

static void pstvnc_app_audio_product_refresh_session_ownership(
    pstvnc_app_audio_product_t *product)
{
    product->session_owned =
        pstvnc_app_audio_product_session_has_ownership(&product->session);
}

static pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_rollback_pre_activation(
    pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_result_t failure)
{
    pstvnc_app_audio_product_record_first_failure(product, failure);
    pstvnc_app_audio_product_refresh_runtime_ownership(product);

    if (product->runtime_owned) {
        if (pstvnc_audio_ps2_runtime_release(&product->runtime) != 0) {
            pstvnc_app_audio_product_refresh_runtime_ownership(product);
            product->state = PSTVNC_APP_AUDIO_PRODUCT_FAULTED;
            product->last_result =
                PSTVNC_APP_AUDIO_PRODUCT_LOCAL_CLEANUP_FAILED;
            return product->last_result;
        }
    }

    pstvnc_app_audio_product_refresh_runtime_ownership(product);
    product->operations_ready = 0;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_DORMANT;
    product->last_result = failure;
    return failure;
}

static int pstvnc_app_audio_product_outcome_is_clean_finite(
    const pstvnc_audio_session_outcome_t *outcome)
{
    if (outcome == NULL)
        return 0;

    if (outcome->kind == PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY)
        return 1;

    return outcome->kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK &&
        outcome->playback_result == PSTVNC_AUDIO_PLAYBACK_COMPLETE;
}

static void pstvnc_app_audio_product_reset_attempt_status(
    pstvnc_app_audio_product_t *product)
{
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    product->first_failure = PSTVNC_APP_AUDIO_PRODUCT_OK;
    product->last_transport_result = PSTVNC_TRANSPORT_OK;
    product->last_session_result = PSTVNC_AUDIO_SESSION_OK;
    product->outcome_valid = 0;
    product->operations_ready = 0;
    product->session_owned = 0;
    product->requires_session_abort = 0;
    product->retained_storage_proven = 0;
    memset(&product->memory_ops, 0, sizeof(product->memory_ops));
    memset(&product->thread_ops, 0, sizeof(product->thread_ops));
    memset(&product->sync, 0, sizeof(product->sync));
    memset(&product->session, 0, sizeof(product->session));
    memset(&product->outcome, 0, sizeof(product->outcome));
}

int pstvnc_app_audio_product_init(
    pstvnc_app_audio_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_media_clock_t *media_clock,
    const pstvnc_media_clock_time_ops_t *time_ops)
{
    pstvnc_config_audio_runtime_profile_t selected_profile;

    if (product == NULL ||
        transport_access == NULL ||
        transport_access->opaque_ticket == 0u ||
        media_clock == NULL ||
        time_ops == NULL ||
        time_ops->read_ticks == NULL ||
        time_ops->delay_us == NULL)
        return 0;

    memset(&selected_profile, 0, sizeof(selected_profile));
    if (!pstvnc_config_audio_runtime_profile_selected(&selected_profile) ||
        !pstvnc_config_audio_runtime_profile_valid(&selected_profile))
        return 0;

    memset(product, 0, sizeof(*product));
    product->transport_access = *transport_access;
    product->profile = selected_profile;
    product->media_clock = media_clock;
    product->time_ops = *time_ops;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_DORMANT;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    product->first_failure = PSTVNC_APP_AUDIO_PRODUCT_OK;
    product->last_transport_result = PSTVNC_TRANSPORT_OK;
    product->last_session_result = PSTVNC_AUDIO_SESSION_OK;
    product->initialized = 1;
    return 1;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_start(
    pstvnc_app_audio_product_t *product)
{
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_session_result_t session_result;
    pstvnc_transport_result_t transport_result;

    if (product == NULL ||
        !product->initialized ||
        product->state != PSTVNC_APP_AUDIO_PRODUCT_DORMANT ||
        product->transport_activation_attempted ||
        product->runtime_owned ||
        product->session_owned)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    pstvnc_app_audio_product_reset_attempt_status(product);
    product->state = PSTVNC_APP_AUDIO_PRODUCT_STARTING;

    if (pstvnc_audio_ps2_runtime_init(&product->runtime) != 0) {
        pstvnc_app_audio_product_refresh_runtime_ownership(product);
        return pstvnc_app_audio_product_rollback_pre_activation(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_INIT_FAILED);
    }
    product->runtime_owned = 1;

    if (pstvnc_audio_ps2_runtime_operations(
            &product->runtime,
            &product->memory_ops,
            &product->thread_ops,
            &product->sync) != 0) {
        return pstvnc_app_audio_product_rollback_pre_activation(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_OPS_FAILED);
    }
    product->operations_ready = 1;

    /*
     * From this assignment onward no in-session retry is legal, even when the
     * activation call itself reports failure: R40 may already have published
     * ACTIVATING and terminalized Transport.
     */
    product->transport_activation_attempted = 1;
    transport_result =
        pstvnc_transport_audio_activate(&product->transport_access);
    product->last_transport_result = transport_result;
    if (transport_result != PSTVNC_TRANSPORT_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_ACTIVATION_FAILED);

    product->transport_activated = 1;
    memset(&service, 0, sizeof(service));
    service = pstvnc_audio_audsrv_service_ops();

    product->session_start_attempted = 1;
    session_result = pstvnc_audio_session_start(
        &product->session,
        &product->profile.session,
        &product->profile.pcm,
        &service,
        product->media_clock,
        &product->time_ops,
        &product->memory_ops,
        &product->thread_ops,
        &product->sync);
    product->last_session_result = session_result;
    pstvnc_app_audio_product_refresh_session_ownership(product);

    if (session_result != PSTVNC_AUDIO_SESSION_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_START_FAILED);

    product->session_owned = 1;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_ACTIVE;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_service(
    pstvnc_app_audio_product_t *product)
{
    pstvnc_audio_session_completion_state_t completion;
    pstvnc_audio_session_result_t session_result;

    if (product == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    if (product->state == PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE)
        return PSTVNC_APP_AUDIO_PRODUCT_OK;

    if (product->state != PSTVNC_APP_AUDIO_PRODUCT_ACTIVE ||
        !product->session_owned)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    completion = PSTVNC_AUDIO_SESSION_COMPLETION_PENDING;
    session_result = pstvnc_audio_session_poll(
        &product->session,
        &completion);
    product->last_session_result = session_result;
    if (session_result != PSTVNC_AUDIO_SESSION_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED);

    if (completion == PSTVNC_AUDIO_SESSION_COMPLETION_PENDING) {
        product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
        return PSTVNC_APP_AUDIO_PRODUCT_OK;
    }

    if (completion != PSTVNC_AUDIO_SESSION_COMPLETION_DONE)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED);

    session_result = pstvnc_audio_session_join(&product->session);
    product->last_session_result = session_result;
    if (session_result != PSTVNC_AUDIO_SESSION_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_JOIN_FAILED);

    session_result = pstvnc_audio_session_outcome(
        &product->session,
        &product->outcome);
    product->last_session_result = session_result;
    if (session_result != PSTVNC_AUDIO_SESSION_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILED);

    product->outcome_valid = 1;

    /*
     * A failed worker outcome is enclosing-session failure. Preserve the joined
     * session/runtime ownership until retained Transport proof authorizes local
     * reclaim; a clean finite outcome may retire locally now.
     */
    if (!pstvnc_app_audio_product_outcome_is_clean_finite(
            &product->outcome))
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILURE);

    session_result = pstvnc_audio_session_release(&product->session);
    product->last_session_result = session_result;
    pstvnc_app_audio_product_refresh_session_ownership(product);
    if (session_result != PSTVNC_AUDIO_SESSION_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_SESSION_RELEASE_FAILED);

    product->session_owned = 0;

    if (product->runtime_owned &&
        pstvnc_audio_ps2_runtime_release(&product->runtime) != 0) {
        pstvnc_app_audio_product_refresh_runtime_ownership(product);
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_RELEASE_FAILED);
    }

    pstvnc_app_audio_product_refresh_runtime_ownership(product);
    product->operations_ready = 0;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_first_presentation_ready(
    pstvnc_app_audio_product_t *product,
    int *ready)
{
    size_t available_count = 0u;
    int producer_done = 0;
    pstvnc_transport_result_t transport_result;

    if (product == NULL ||
        ready == NULL ||
        !product->initialized ||
        (product->state != PSTVNC_APP_AUDIO_PRODUCT_ACTIVE &&
         product->state != PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE))
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    *ready = 0;

    transport_result = pstvnc_transport_audio_status(
        &product->transport_access,
        &available_count,
        &producer_done);
    product->last_transport_result = transport_result;
    if (transport_result != PSTVNC_TRANSPORT_OK)
        return pstvnc_app_audio_product_fault(
            product,
            PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_STATUS_FAILED);

    if (available_count >=
            product->profile.session.startup_reservoir_bytes ||
        producer_done)
        *ready = 1;

    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_service_session_abort(
    pstvnc_app_audio_product_t *product,
    int *abort_ready)
{
    pstvnc_audio_session_completion_state_t completion;
    pstvnc_audio_session_result_t session_result;
    pstvnc_transport_result_t transport_result;

    if (product == NULL ||
        abort_ready == NULL ||
        !product->initialized ||
        product->state == PSTVNC_APP_AUDIO_PRODUCT_DORMANT ||
        product->state == PSTVNC_APP_AUDIO_PRODUCT_STARTING)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    *abort_ready = 0;

    if (product->state == PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY) {
        *abort_ready = 1;
        return PSTVNC_APP_AUDIO_PRODUCT_OK;
    }

    /*
     * After the R40 activation edge is even attempted, Transport must prove
     * exact retained old-session storage before this coordinator touches local
     * session/runtime ownership.
     */
    if (product->transport_activation_attempted &&
        !product->retained_storage_proven) {
        transport_result =
            pstvnc_transport_session_abort_storage_retained(
                &product->transport_access);
        product->last_transport_result = transport_result;
        if (transport_result != PSTVNC_TRANSPORT_OK) {
            product->last_result =
                PSTVNC_APP_AUDIO_PRODUCT_RETAINED_STORAGE_REQUIRED;
            return product->last_result;
        }

        product->retained_storage_proven = 1;
    }

    if (product->state == PSTVNC_APP_AUDIO_PRODUCT_ACTIVE)
        product->state = PSTVNC_APP_AUDIO_PRODUCT_FAULTED;

    pstvnc_app_audio_product_refresh_session_ownership(product);

    if (product->session_owned &&
        product->session.thread_started &&
        !product->session.thread_joined) {
        if (!product->session.stop_requested) {
            session_result =
                pstvnc_audio_session_request_stop(&product->session);
            product->last_session_result = session_result;
            if (session_result != PSTVNC_AUDIO_SESSION_OK)
                return pstvnc_app_audio_product_fault(
                    product,
                    PSTVNC_APP_AUDIO_PRODUCT_SESSION_STOP_FAILED);
        }

        completion = PSTVNC_AUDIO_SESSION_COMPLETION_PENDING;
        session_result = pstvnc_audio_session_poll(
            &product->session,
            &completion);
        product->last_session_result = session_result;
        if (session_result != PSTVNC_AUDIO_SESSION_OK)
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED);

        if (completion == PSTVNC_AUDIO_SESSION_COMPLETION_PENDING) {
            product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
            return PSTVNC_APP_AUDIO_PRODUCT_OK;
        }

        if (completion != PSTVNC_AUDIO_SESSION_COMPLETION_DONE)
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED);

        session_result = pstvnc_audio_session_join(&product->session);
        product->last_session_result = session_result;
        if (session_result != PSTVNC_AUDIO_SESSION_OK)
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_SESSION_JOIN_FAILED);
    }

    if (product->session_owned &&
        product->session.thread_joined &&
        !product->outcome_valid) {
        session_result = pstvnc_audio_session_outcome(
            &product->session,
            &product->outcome);
        product->last_session_result = session_result;
        if (session_result != PSTVNC_AUDIO_SESSION_OK)
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILED);
        product->outcome_valid = 1;
    }

    if (product->session_owned) {
        session_result = pstvnc_audio_session_release(&product->session);
        product->last_session_result = session_result;
        pstvnc_app_audio_product_refresh_session_ownership(product);
        if (session_result != PSTVNC_AUDIO_SESSION_OK)
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_SESSION_RELEASE_FAILED);
        product->session_owned = 0;
    }

    if (product->runtime_owned) {
        if (pstvnc_audio_ps2_runtime_release(&product->runtime) != 0) {
            pstvnc_app_audio_product_refresh_runtime_ownership(product);
            return pstvnc_app_audio_product_fault(
                product,
                PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_RELEASE_FAILED);
        }
        pstvnc_app_audio_product_refresh_runtime_ownership(product);
    }

    product->operations_ready = 0;
    product->requires_session_abort = 0;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    *abort_ready = 1;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

int pstvnc_app_audio_product_transport_activated(
    const pstvnc_app_audio_product_t *product)
{
    return product != NULL &&
        product->initialized &&
        product->transport_activated;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_status(
    const pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_status_t *status)
{
    if (product == NULL || status == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    memset(status, 0, sizeof(*status));
    status->state = product->state;
    status->last_result = product->last_result;
    status->first_failure = product->first_failure;
    status->last_transport_result = product->last_transport_result;
    status->last_session_result = product->last_session_result;
    status->outcome = product->outcome;
    status->outcome_valid = product->outcome_valid;
    status->runtime_owned = product->runtime_owned;
    status->session_owned = product->session_owned;
    status->transport_activation_attempted =
        product->transport_activation_attempted;
    status->transport_activated = product->transport_activated;
    status->session_start_attempted = product->session_start_attempted;
    status->requires_session_abort = product->requires_session_abort;
    status->retained_storage_proven = product->retained_storage_proven;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}
