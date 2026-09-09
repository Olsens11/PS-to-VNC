/*
 * File synopsis:
 * Wraps cumulative H1 transport lifecycle cleanup for live logical RFB channel 1.
 *
 * Resource preparation no longer occurs after the inner transport returns: the
 * cumulative h1_transport_runtime.c hook now prepares/binds channel 1 before
 * CONFIG ACK and before the sole receiver thread starts. This wrapper therefore
 * owns only failure/normal cleanup after the physical receiver/socket has been
 * stopped by the unchanged inner shutdown path.
 *
 * CONFIG still rejects RFB ON at this checkpoint, so accepted media sessions
 * continue to exercise an allocation-free RFB OFF path.
 */

#include "h1_rfb_transport_live.h"
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
    int result;

    if (runtime == NULL)
        return -1;

    result = pstvnc_h1_transport_start_inner(runtime);
    if (result < 0)
        (void)pstvnc_h1_rfb_transport_release(runtime);

    return result;
}

int pstvnc_h1_transport_shutdown(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int transport_result;
    int rfb_result;

    if (runtime == NULL)
        return -1;

    /*
     * Stop the sole physical recv()/socket owner first. Only then unbind the
     * parser adapter and release the logical RFB queue/semaphore bundle.
     */
    transport_result = pstvnc_h1_transport_shutdown_inner(runtime);
    rfb_result = pstvnc_h1_rfb_transport_release(runtime);

    return transport_result < 0 || !rfb_result ? -1 : 0;
}
