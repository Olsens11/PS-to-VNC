/*
 * File synopsis:
 * Diagnostic H1 audio runtime that preserves the canonical PCM/AUDSRV path
 * byte-for-byte while sampling the EE thread table from the already-running
 * audio worker. One census record is held long enough for heartbeat telemetry
 * to observe it through the existing diagnostic_word field.
 *
 * This file deliberately includes the canonical audio runtime rather than
 * copying it. The only interception point is the existing
 * pstvnc_h1_transport_record_audio_played() call after a successful AUDSRV
 * handoff. No extra EE thread is created, so the observer does not introduce a
 * new runnable priority into the scheduler topology being measured.
 *
 * diagnostic_word packing while media is active:
 *   bits 31..28 = census marker / known role
 *                 0xC other, 0xD H1 audio, 0xE H1 receiver
 *   bits 27..20 = EE thread ID
 *   bits 19..14 = thread status
 *   bits 13..7  = initial priority
 *   bits 6..0   = current priority
 */

#include <kernel.h>
#include <stdint.h>

/*
 * Rename the canonical runtime's accounting call while compiling it into this
 * translation unit. The wrapper below performs the canonical accounting first
 * and then takes the low-frequency census sample.
 */
#define pstvnc_h1_transport_record_audio_played \
    pstvnc_h1_thread_census_record_audio_played
#include "h1_audio_runtime.c"
#undef pstvnc_h1_transport_record_audio_played

/* The canonical transport function remains linked from h1_transport_runtime.o. */
void pstvnc_h1_transport_record_audio_played(
    pstvnc_h1_transport_runtime_t *runtime,
    size_t byte_count);

#define H1_CENSUS_MARKER_OTHER 0xC0000000u
#define H1_CENSUS_MARKER_AUDIO 0xD0000000u
#define H1_CENSUS_MARKER_RECEIVER 0xE0000000u
#define H1_CENSUS_HOLD_CHUNKS 64u

static uint32_t s_h1_census_session_id;
static unsigned int s_h1_census_next_thread_id = 1u;
static unsigned int s_h1_census_hold_chunks;
static unsigned int s_h1_census_phase;

static uint32_t h1_census_pack(
    uint32_t marker,
    unsigned int thread_id,
    const ee_thread_status_t *status)
{
    return marker |
        (((uint32_t)thread_id & 0xffu) << 20) |
        (((uint32_t)status->status & 0x3fu) << 14) |
        (((uint32_t)status->initial_priority & 0x7fu) << 7) |
        ((uint32_t)status->current_priority & 0x7fu);
}

static int h1_census_status_is_documented(int status)
{
    switch (status) {
        case THS_RUN:
        case THS_READY:
        case THS_WAIT:
        case THS_SUSPEND:
        case THS_WAITSUSPEND:
        case THS_DORMANT:
            return 1;

        default:
            return 0;
    }
}

static int h1_census_emit_thread(
    pstvnc_h1_transport_runtime_t *transport,
    int lookup_thread_id,
    unsigned int reported_thread_id,
    uint32_t marker)
{
    ee_thread_status_t status;

    memset(&status, 0, sizeof(status));

    /*
     * The EE API documents only RUN/READY/WAIT/SUSPEND/WAITSUSPEND/DORMANT
     * as thread states. A successful-looking lookup that leaves status zero
     * is therefore not accepted as a census record.
     */
    if (ReferThreadStatus(lookup_thread_id, &status) < 0)
        return 0;

    if (!h1_census_status_is_documented(status.status))
        return 0;

    pstvnc_h1_transport_set_diagnostic_word(
        transport,
        h1_census_pack(
            marker,
            reported_thread_id,
            &status));

    return 1;
}

static void h1_census_emit_next(pstvnc_h1_transport_runtime_t *transport)
{
    unsigned int attempt;
    int audio_thread_id = GetThreadId();

    /*
     * Self-validation comes first. TH_SELF asks the EE kernel about the
     * currently executing audio worker while we still report its real ID.
     * This should produce RUN and proves the status ABI before enumeration.
     */
    if (s_h1_census_phase == 0u) {
        s_h1_census_phase = 1u;

        if (audio_thread_id > 0 &&
            audio_thread_id < MAX_THREADS &&
            h1_census_emit_thread(
                transport,
                TH_SELF,
                (unsigned int)audio_thread_id,
                H1_CENSUS_MARKER_AUDIO))
            return;
    }

    /*
     * Next sample the already-known mux receiver directly. This establishes
     * the second H1-owned thread before scanning anonymous kernel/library IDs.
     */
    if (s_h1_census_phase == 1u) {
        int receiver_thread_id = transport->receiver_thread_id;

        s_h1_census_phase = 2u;

        if (receiver_thread_id > 0 &&
            receiver_thread_id < MAX_THREADS &&
            h1_census_emit_thread(
                transport,
                receiver_thread_id,
                (unsigned int)receiver_thread_id,
                H1_CENSUS_MARKER_RECEIVER))
            return;
    }

    for (attempt = 0u; attempt < (MAX_THREADS - 1u); ++attempt) {
        ee_thread_status_t status;
        unsigned int thread_id = s_h1_census_next_thread_id;
        uint32_t marker = H1_CENSUS_MARKER_OTHER;

        s_h1_census_next_thread_id++;
        if (s_h1_census_next_thread_id >= MAX_THREADS)
            s_h1_census_next_thread_id = 1u;

        memset(&status, 0, sizeof(status));

        if (ReferThreadStatus((int)thread_id, &status) < 0)
            continue;

        if (!h1_census_status_is_documented(status.status))
            continue;

        if ((int)thread_id == audio_thread_id)
            marker = H1_CENSUS_MARKER_AUDIO;
        else if ((int)thread_id == transport->receiver_thread_id)
            marker = H1_CENSUS_MARKER_RECEIVER;

        pstvnc_h1_transport_set_diagnostic_word(
            transport,
            h1_census_pack(marker, thread_id, &status));
        return;
    }

    /*
     * OTHER + ID 255 + zero fields is retained as the explicit sentinel for
     * no documented thread state found during a complete table pass.
     */
    pstvnc_h1_transport_set_diagnostic_word(
        transport,
        H1_CENSUS_MARKER_OTHER | (0xffu << 20));
}

void pstvnc_h1_thread_census_record_audio_played(
    pstvnc_h1_transport_runtime_t *transport,
    size_t byte_count)
{
    const pstvnc_h1_config_t *config;

    /* Preserve canonical accounting before performing any observation. */
    pstvnc_h1_transport_record_audio_played(transport, byte_count);

    config = pstvnc_h1_transport_config(transport);
    if (config == NULL)
        return;

    if (s_h1_census_session_id != config->session_id) {
        s_h1_census_session_id = config->session_id;
        s_h1_census_next_thread_id = 1u;
        s_h1_census_hold_chunks = 0u;
        s_h1_census_phase = 0u;
    }

    if (s_h1_census_hold_chunks != 0u) {
        s_h1_census_hold_chunks--;
        return;
    }

    h1_census_emit_next(transport);
    s_h1_census_hold_chunks = H1_CENSUS_HOLD_CHUNKS;
}
