/*
 * File synopsis:
 * Defines A003 R3's concrete PS2 SMS-libmpeg/IPU backend for the accepted
 * synchronous MPEG decoder platform-ops seam.
 *
 * The backend owns only process-global SMS decoder binding, qualified IPU/DMAC
 * known-state preparation, decoder callback translation, TO_IPU feed DMA, RGB16
 * libmpeg selection, first/subsequent picture-call mechanics, and matching
 * destroy/release state. It owns no Transport reads, local-stop policy, run
 * generation, worker/thread lifecycle, Presentation/GS work, media clock,
 * scheduler/drop, RFB state, or Application orchestration.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_PS2_MPEG_DECODER_BACKEND_H
#define PSTVNC_PS2_MPEG_DECODER_BACKEND_H

#include "decoder.h"

#include <stddef.h>

typedef struct pstvnc_ps2_mpeg_decoder_backend {
    pstvnc_mpeg_feed_callback_t feed_callback;
    void *feed_context;
    pstvnc_mpeg_sequence_callback_t sequence_callback;
    void *sequence_context;

    void *sequence_picture;
    size_t sequence_picture_capacity;

    long long current_stream_pts;
    long long picture_pts;

    int known_state_prepared;
    int decoder_initialized;
    int first_picture_pending;
} pstvnc_ps2_mpeg_decoder_backend_t;

/* Reset one caller-owned backend context to an unacquired reusable state. */
void pstvnc_ps2_mpeg_decoder_backend_init(
    pstvnc_ps2_mpeg_decoder_backend_t *backend);

/*
 * Populate the accepted decoder platform-ops contract for this backend.
 *
 * SMS libmpeg exposes process-global decoder state, so only one backend may
 * successfully enter prepare/initialize ownership at a time. The platform ops
 * fail closed on overlapping ownership rather than pretending independent SMS
 * instances exist.
 */
int pstvnc_ps2_mpeg_decoder_backend_platform_ops(
    pstvnc_ps2_mpeg_decoder_backend_t *backend,
    pstvnc_mpeg_decoder_platform_ops_t *platform_ops);

#endif /* PSTVNC_PS2_MPEG_DECODER_BACKEND_H */
