/*
 * File synopsis:
 * Wraps the cumulative H1 transport start/shutdown lifecycle with ownership of
 * the already-prepared logical RFB channel-1 resource bundle.
 *
 * This checkpoint establishes only lifecycle ownership. The underlying H1
 * transport implementation still owns the one physical PSTV socket, sole recv()
 * thread, send semaphore, AUDIO/MPEG behavior, CONFIG exchange, and all live
 * frame dispatch. CONFIG still rejects RFB ON, so activation currently resolves
 * to the explicit OFF no-op. No RFB credit, DATA dispatch, mux I/O adapter, Pi
 * bridge, parser, or presentation behavior is enabled here.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2;
 * RFB_MUX_CP2C_DORMANT_RUNTIME_RESOURCES.md.
 */

#include "h1_transport_runtime.h"

/*
 * The cumulative prep build mechanically renames the original implementation's
 * public lifecycle symbols to these inner names. All other H1 builds continue
 * compiling the original implementation normally.
 */
int pstvnc_h1_transport_start_inner(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_shutdown_inner(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_start(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int enabled;

    if (runtime == NULL)
        return -1;

    if (pstvnc_h1_transport_start_inner(runtime) < 0) {
        /*
         * The inner start may have zeroed the whole runtime before failing.
         * Restore the dormant bundle's explicit inactive sentinel state so a
         * later public shutdown remains deterministic.
         */
        pstvnc_h1_rfb_runtime_resources_init(&runtime->rfb_resources);
        return -1;
    }

    pstvnc_h1_rfb_runtime_resources_init(&runtime->rfb_resources);

    enabled = runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED;
    if (!pstvnc_h1_rfb_runtime_resources_activate(
            &runtime->rfb_resources,
            enabled)) {
        (void)pstvnc_h1_transport_shutdown_inner(runtime);
        (void)pstvnc_h1_rfb_runtime_resources_release(
            &runtime->rfb_resources);
        return -1;
    }

    return 0;
}

int pstvnc_h1_transport_shutdown(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int transport_result;
    int rfb_result;

    if (runtime == NULL)
        return -1;

    /*
     * Stop the sole physical receiver/socket owner before releasing a logical
     * channel resource bundle that a later checkpoint may make live.
     */
    transport_result = pstvnc_h1_transport_shutdown_inner(runtime);
    rfb_result = pstvnc_h1_rfb_runtime_resources_release(
        &runtime->rfb_resources);

    return transport_result < 0 || !rfb_result ? -1 : 0;
}
