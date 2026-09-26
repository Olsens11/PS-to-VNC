/*
 * File synopsis:
 * Defines R41's Application-owned AUDIO lifecycle coordinator. It composes the
 * accepted selected AUDIO profile, R37 PS2 execution binding, R40 Transport
 * activation, R38 completion publication, AUDSRV playback service boundary and
 * session common clock without enabling ordinary app.c product activation.
 *
 * The coordinator owns only cross-domain sequencing. AUDIO/Transport/media
 * internals remain in their existing owners, and the first-presentation query
 * observes reservoir state without consuming PCM or arming the common clock.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41.
 */

#ifndef PSTVNC_APP_AUDIO_PRODUCT_H
#define PSTVNC_APP_AUDIO_PRODUCT_H

#include "audio/audsrv_service.h"
#include "audio/ps2_runtime.h"
#include "config/audio_runtime_profile.h"
#include "media/clock.h"
#include "transport/bridge.h"

typedef enum pstvnc_app_audio_product_state {
    PSTVNC_APP_AUDIO_PRODUCT_DORMANT = 0,
    PSTVNC_APP_AUDIO_PRODUCT_STARTING = 1,
    PSTVNC_APP_AUDIO_PRODUCT_ACTIVE = 2,
    PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE = 3,
    PSTVNC_APP_AUDIO_PRODUCT_FAULTED = 4,
    PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY = 5
} pstvnc_app_audio_product_state_t;

typedef enum pstvnc_app_audio_product_result {
    PSTVNC_APP_AUDIO_PRODUCT_OK = 0,
    PSTVNC_APP_AUDIO_PRODUCT_INVALID = -1,
    PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_INIT_FAILED = -2,
    PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_OPS_FAILED = -3,
    PSTVNC_APP_AUDIO_PRODUCT_LOCAL_CLEANUP_FAILED = -4,
    PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_ACTIVATION_FAILED = -5,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_START_FAILED = -6,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED = -7,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_JOIN_FAILED = -8,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILED = -9,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_RELEASE_FAILED = -10,
    PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_RELEASE_FAILED = -11,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILURE = -12,
    PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_STATUS_FAILED = -13,
    PSTVNC_APP_AUDIO_PRODUCT_RETAINED_STORAGE_REQUIRED = -14,
    PSTVNC_APP_AUDIO_PRODUCT_SESSION_STOP_FAILED = -15
} pstvnc_app_audio_product_result_t;

typedef struct pstvnc_app_audio_product_status {
    pstvnc_app_audio_product_state_t state;
    pstvnc_app_audio_product_result_t last_result;
    pstvnc_app_audio_product_result_t first_failure;
    pstvnc_transport_result_t last_transport_result;
    pstvnc_audio_session_result_t last_session_result;
    pstvnc_audio_session_outcome_t outcome;
    int outcome_valid;
    int runtime_owned;
    int session_owned;
    int transport_activation_attempted;
    int transport_activated;
    int session_start_attempted;
    int requires_session_abort;
    int retained_storage_proven;
} pstvnc_app_audio_product_status_t;

typedef struct pstvnc_app_audio_product {
    pstvnc_transport_access_t transport_access;
    pstvnc_config_audio_runtime_profile_t profile;
    const pstvnc_media_clock_t *media_clock;
    pstvnc_media_clock_time_ops_t time_ops;

    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    pstvnc_audio_session_t session;

    pstvnc_audio_session_outcome_t outcome;
    pstvnc_app_audio_product_state_t state;
    pstvnc_app_audio_product_result_t last_result;
    pstvnc_app_audio_product_result_t first_failure;
    pstvnc_transport_result_t last_transport_result;
    pstvnc_audio_session_result_t last_session_result;

    int initialized;
    int runtime_owned;
    int operations_ready;
    int session_owned;
    int outcome_valid;
    int transport_activation_attempted;
    int transport_activated;
    int session_start_attempted;
    int requires_session_abort;
    int retained_storage_proven;
} pstvnc_app_audio_product_t;

/*
 * Inert session construction. This copies the selected R36 profile and exact
 * caller ticket/time authority but performs no Transport activation, R37
 * allocation/thread work, AUDSRV operation or media-clock arm.
 */
int pstvnc_app_audio_product_init(
    pstvnc_app_audio_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_media_clock_t *media_clock,
    const pstvnc_media_clock_time_ops_t *time_ops);

/*
 * Explicit one-shot AUDIO start for the current Wire Session. A fully rolled
 * back pre-activation failure may be retried while DORMANT. Once Transport
 * activation is attempted this coordinator never starts again in that session.
 */
pstvnc_app_audio_product_result_t pstvnc_app_audio_product_start(
    pstvnc_app_audio_product_t *product);

/*
 * Nonblocking steady-state owner service. PENDING completion returns OK
 * immediately. DONE is joined before outcome observation; clean finite AUDIO
 * completion retires local owners into FINITE_COMPLETE.
 */
pstvnc_app_audio_product_result_t pstvnc_app_audio_product_service(
    pstvnc_app_audio_product_t *product);

/*
 * Non-consuming first-MPEG-presentation gate. READY is true only when the
 * current exact Transport ticket reports the selected startup reservoir or a
 * finite producer-done fact. This call never dequeues AUDIO bytes.
 */
pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_first_presentation_ready(
    pstvnc_app_audio_product_t *product,
    int *ready);

/*
 * Retryable abnormal local teardown. After any Transport activation attempt,
 * this refuses every local stop/reclaim touch until Transport proves that the
 * stored old ticket is the retained terminal runtime. The caller owns
 * begin_abort()/final Transport close sequencing.
 */
pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_service_session_abort(
    pstvnc_app_audio_product_t *product,
    int *abort_ready);

int pstvnc_app_audio_product_transport_activated(
    const pstvnc_app_audio_product_t *product);

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_status(
    const pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_status_t *status);

#endif /* PSTVNC_APP_AUDIO_PRODUCT_H */
