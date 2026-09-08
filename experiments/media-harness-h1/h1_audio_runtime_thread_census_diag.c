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

static void h1_census_emit_next(pstvnc_h1_transport_runtime_t *transport)
{
    unsigned int attempt;
    int audio_thread_id = GetThreadId();

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

        if ((int)thread_id == audio_thread_id)
            marker = H1_CENSUS_MARKER_AUDIO;
        else if ((int)thread_id == transport->receiver_thread_id)
            marker = H1_CENSUS_MARKER_RECEIVER;

        pstvnc_h1_transport_set_diagnostic_word(
            transport,
            h1_census_pack(marker, thread_id, &status));
        return;
    }

    /* OTHER marker + thread ID 255 + zero fields means no valid thread found. */
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
    }

    if (s_h1_census_hold_chunks != 0u) {
        s_h1_census_hold_chunks--;
        return;
    }

    h1_census_emit_next(transport);
    s_h1_census_hold_chunks = H1_CENSUS_HOLD_CHUNKS;
}
