/*
 * File synopsis:
 * Declares the concrete PS2SDK AUDSRV operation adapter used by the clean A002
 * PCM playback core. The adapter exposes stream setup/wait/play/stop only; the
 * resident AUDSRV/LIBSD service has no per-session quit operation here.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_AUDIO_AUDSRV_SERVICE_H
#define PSTVNC_AUDIO_AUDSRV_SERVICE_H

#include "playback.h"

pstvnc_audio_service_ops_t pstvnc_audio_audsrv_service_ops(void);

#endif
