/*
 * Focused R42 Application AUDIO composition stubs.
 *
 * Included by app_test.c after the established legacy fixture so these stubs
 * can preserve its existing event/counter authority while observing the new
 * public R36-R41 composition edges.
 */

#include "audio/ps2_runtime.h"
#include "app_audio_product.h"
#include "config/audio_runtime_profile.h"

static int selected_audio_profile_available;
static int selected_audio_profile_valid_result;
static size_t selected_audio_profile_calls;
static size_t selected_audio_profile_event_count;
static size_t selected_audio_profile_valid_calls;
static pstvnc_config_audio_runtime_profile_t selected_audio_profile;

static size_t r42_resident_prepare_calls;
static size_t r42_resident_prepare_event_count;
static int r42_resident_prepare_result;

static pstvnc_transport_audio_channel_config_t
    r42_observed_audio_transport_config;

static size_t r42_time_ops_calls;

static size_t r42_audio_product_init_calls;
static size_t r42_audio_start_calls;
static size_t r42_audio_start_event_count;
static size_t r42_audio_service_calls;
static size_t r42_audio_gate_calls;
static size_t r42_audio_abort_calls;
static size_t r42_audio_abort_event_count;
static size_t r42_audio_status_calls;

static pstvnc_app_audio_product_result_t r42_audio_start_result;
static pstvnc_app_audio_product_result_t r42_audio_service_result;
static pstvnc_app_audio_product_result_t r42_audio_gate_result;
static pstvnc_app_audio_product_result_t r42_audio_abort_result;
static pstvnc_app_audio_product_result_t r42_audio_abort_result_script[16];
static size_t r42_audio_abort_result_script_count;
static size_t r42_audio_abort_media_release_calls[16];
static size_t r42_audio_abort_transport_close_calls[16];
static int r42_audio_service_sets_finite;
static int r42_audio_gate_script[16];
static size_t r42_audio_gate_script_count;
static int r42_audio_abort_ready_script[16];
static size_t r42_audio_abort_ready_script_count;

static int r42_controller_starts_mpeg;
static int r42_live_arms_clock;

static const pstvnc_config_audio_runtime_profile_t r42_test_audio_profile = {
    {
        524288u,
        524288u,
        65536u,
        1,
        1
    },
    {
        48000u,
        2u,
        16u,
        80u
    },
    {
        32768u,
        64,
        32768u,
        458752u,
        1000u,
        1000u
    }
};

static int r42_test_read_ticks(void *context, uint64_t *ticks)
{
    if (context == NULL || ticks == NULL)
        return -1;
    *ticks = r34_current_tick_value;
    return 0;
}

static int r42_test_delay_us(void *context, uint32_t delay_us)
{
    if (context == NULL || delay_us == 0u)
        return -1;
    return 0;
}

int pstvnc_config_audio_runtime_profile_selected(
    pstvnc_config_audio_runtime_profile_t *profile)
{
    selected_audio_profile_calls++;
    selected_audio_profile_event_count = event_count;

    if (!selected_audio_profile_available || profile == NULL)
        return 0;

    *profile = selected_audio_profile;
    return 1;
}

int pstvnc_config_audio_runtime_profile_valid(
    const pstvnc_config_audio_runtime_profile_t *profile)
{
    selected_audio_profile_valid_calls++;
    return profile != NULL && selected_audio_profile_valid_result;
}

int pstvnc_audio_ps2_resident_prepare(
    pstvnc_audio_ps2_resident_t *resident)
{
    r42_resident_prepare_calls++;
    r42_resident_prepare_event_count = event_count;

    if (resident == NULL || r42_resident_prepare_result != 0)
        return -1;

    resident->libsd_loaded = 1;
    resident->audsrv_loaded = 1;
    return 0;
}

pstvnc_transport_result_t pstvnc_transport_session_open_with_audio_mpeg(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    log_event(EV_TRANSPORT_OPEN);
    transport_open_calls++;

    CHECK(socket_fd != NULL);
    CHECK(config != NULL);
    CHECK(audio_config != NULL);
    CHECK(mpeg_config != NULL);

    if (config != NULL)
        observed_transport_config = *config;
    if (audio_config != NULL)
        r42_observed_audio_transport_config = *audio_config;
    if (mpeg_config != NULL)
        observed_mpeg_transport_config = *mpeg_config;

    if (transport_open_calls <=
        sizeof(transport_open_socket_fds) /
        sizeof(transport_open_socket_fds[0]) &&
        socket_fd != NULL)
        transport_open_socket_fds[transport_open_calls - 1u] = *socket_fd;

    if (socket_fd != NULL &&
        (transport_open_result == PSTVNC_TRANSPORT_OK ||
         transport_open_adopts))
        *socket_fd = -1;

    return transport_open_result;
}

int pstvnc_ps2_media_clock_binding_time_ops(
    pstvnc_ps2_media_clock_binding_t *binding,
    pstvnc_media_clock_time_ops_t *time_ops)
{
    r42_time_ops_calls++;

    if (binding == NULL ||
        time_ops == NULL ||
        binding->state != PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE)
        return -1;

    memset(time_ops, 0, sizeof(*time_ops));
    time_ops->read_ticks = r42_test_read_ticks;
    time_ops->delay_us = r42_test_delay_us;
    time_ops->context = binding;
    return 0;
}

int pstvnc_app_audio_product_init(
    pstvnc_app_audio_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_media_clock_t *media_clock,
    const pstvnc_media_clock_time_ops_t *time_ops)
{
    r42_audio_product_init_calls++;

    if (product == NULL ||
        transport_access == NULL ||
        transport_access->opaque_ticket == 0u ||
        media_clock == NULL ||
        time_ops == NULL ||
        time_ops->read_ticks == NULL ||
        time_ops->delay_us == NULL)
        return 0;

    memset(product, 0, sizeof(*product));
    product->initialized = 1;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_DORMANT;
    product->transport_access = *transport_access;
    product->media_clock = media_clock;
    product->time_ops = *time_ops;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    product->first_failure = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return 1;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_start(
    pstvnc_app_audio_product_t *product)
{
    r42_audio_start_calls++;
    r42_audio_start_event_count = event_count;

    if (product == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    product->transport_activation_attempted = 1;
    if (r42_audio_start_result != PSTVNC_APP_AUDIO_PRODUCT_OK) {
        product->state = PSTVNC_APP_AUDIO_PRODUCT_FAULTED;
        product->last_result = r42_audio_start_result;
        return r42_audio_start_result;
    }

    product->transport_activated = 1;
    product->runtime_owned = 1;
    product->session_owned = 1;
    product->session_start_attempted = 1;
    product->state = PSTVNC_APP_AUDIO_PRODUCT_ACTIVE;
    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_service(
    pstvnc_app_audio_product_t *product)
{
    r42_audio_service_calls++;

    if (product == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    if (r42_audio_service_result != PSTVNC_APP_AUDIO_PRODUCT_OK) {
        product->state = PSTVNC_APP_AUDIO_PRODUCT_FAULTED;
        product->last_result = r42_audio_service_result;
        return r42_audio_service_result;
    }

    if (r42_audio_service_sets_finite) {
        product->state = PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE;
        product->runtime_owned = 0;
        product->session_owned = 0;
        product->outcome_valid = 1;
        product->outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY;
    }

    product->last_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_first_presentation_ready(
    pstvnc_app_audio_product_t *product,
    int *ready)
{
    size_t index = r42_audio_gate_calls++;

    if (product == NULL || ready == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    if (r42_audio_gate_result != PSTVNC_APP_AUDIO_PRODUCT_OK)
        return r42_audio_gate_result;

    if (index < r42_audio_gate_script_count)
        *ready = r42_audio_gate_script[index];
    else
        *ready = 1;

    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t
pstvnc_app_audio_product_service_session_abort(
    pstvnc_app_audio_product_t *product,
    int *abort_ready)
{
    size_t index = r42_audio_abort_calls++;
    pstvnc_app_audio_product_result_t result = r42_audio_abort_result;

    r42_audio_abort_event_count = event_count;
    if (index < sizeof(r42_audio_abort_media_release_calls) /
            sizeof(r42_audio_abort_media_release_calls[0])) {
        r42_audio_abort_media_release_calls[index] =
            media_binding_release_calls;
        r42_audio_abort_transport_close_calls[index] =
            r34_transport_close_calls;
    }

    if (product == NULL || abort_ready == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    if (index < r42_audio_abort_result_script_count)
        result = r42_audio_abort_result_script[index];

    if (result != PSTVNC_APP_AUDIO_PRODUCT_OK)
        return result;

    if (index < r42_audio_abort_ready_script_count)
        *abort_ready = r42_audio_abort_ready_script[index];
    else
        *abort_ready = 1;

    if (*abort_ready) {
        product->runtime_owned = 0;
        product->session_owned = 0;
        product->state = PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY;
    }

    return PSTVNC_APP_AUDIO_PRODUCT_OK;
}

pstvnc_app_audio_product_result_t pstvnc_app_audio_product_status(
    const pstvnc_app_audio_product_t *product,
    pstvnc_app_audio_product_status_t *status)
{
    r42_audio_status_calls++;

    if (product == NULL || status == NULL || !product->initialized)
        return PSTVNC_APP_AUDIO_PRODUCT_INVALID;

    memset(status, 0, sizeof(*status));
    status->state = product->state;
    status->last_result = product->last_result;
    status->first_failure = product->first_failure;
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

static void reset_r42_audio_fixture(void)
{
    selected_audio_profile_available = 1;
    selected_audio_profile_valid_result = 1;
    selected_audio_profile_calls = 0u;
    selected_audio_profile_event_count = 0u;
    selected_audio_profile_valid_calls = 0u;
    selected_audio_profile = r42_test_audio_profile;

    r42_resident_prepare_calls = 0u;
    r42_resident_prepare_event_count = 0u;
    r42_resident_prepare_result = 0;

    memset(
        &r42_observed_audio_transport_config,
        0,
        sizeof(r42_observed_audio_transport_config));

    r42_time_ops_calls = 0u;

    r42_audio_product_init_calls = 0u;
    r42_audio_start_calls = 0u;
    r42_audio_start_event_count = 0u;
    r42_audio_service_calls = 0u;
    r42_audio_gate_calls = 0u;
    r42_audio_abort_calls = 0u;
    r42_audio_abort_event_count = 0u;
    r42_audio_status_calls = 0u;

    r42_audio_start_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    r42_audio_service_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    r42_audio_gate_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    r42_audio_abort_result = PSTVNC_APP_AUDIO_PRODUCT_OK;
    memset(
        r42_audio_abort_result_script,
        0,
        sizeof(r42_audio_abort_result_script));
    r42_audio_abort_result_script_count = 0u;
    memset(
        r42_audio_abort_media_release_calls,
        0,
        sizeof(r42_audio_abort_media_release_calls));
    memset(
        r42_audio_abort_transport_close_calls,
        0,
        sizeof(r42_audio_abort_transport_close_calls));
    r42_audio_service_sets_finite = 0;

    memset(r42_audio_gate_script, 0, sizeof(r42_audio_gate_script));
    r42_audio_gate_script_count = 0u;
    memset(
        r42_audio_abort_ready_script,
        0,
        sizeof(r42_audio_abort_ready_script));
    r42_audio_abort_ready_script_count = 0u;

    r42_controller_starts_mpeg = 0;
    r42_live_arms_clock = 0;
}
