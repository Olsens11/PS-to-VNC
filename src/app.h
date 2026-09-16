/*
 * File synopsis:
 * Declares the top-level coordinator entry points while keeping concrete
 * Transport configuration authority outside application implementation.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Application coordinator";
 * docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md.
 */

#ifndef PSTVNC_APP_H
#define PSTVNC_APP_H

#include "transport/transport.h"

/*
 * Run the product lifecycle with one explicitly validated Transport session
 * configuration supplied by the eventual configuration/composition owner.
 * Application forwards this value unchanged; it does not invent defaults.
 */
int pstvnc_app_run_with_transport_config(
    const pstvnc_transport_session_config_t *transport_config);

/*
 * Current process entry remains deliberately fail-gated until repository
 * authority supplies a validated all-guns Transport configuration owner.
 * No queue/credit/thread/payload defaults are manufactured here.
 */
int pstvnc_app_run(void);

#endif
