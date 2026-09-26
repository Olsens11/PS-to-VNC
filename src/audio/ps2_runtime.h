/*
 * File synopsis:
 * Defines R37's AUDIO-owned PlayStation 2 execution binding for the accepted
 * session worker contract. One runtime owns aligned session allocations, one
 * session-state semaphore, one retained worker-completion semaphore, and one
 * exact EE thread slot. A separate caller-owned resident state records one-time
 * LIBSD/AUDSRV module preparation across AUDIO sessions.
 *
 * The binding supplies mechanism only. It does not own Transport activation,
 * PCM policy, common-media-clock time operations, Application sequencing, or
 * any timeout-as-success reclamation rule. Thread and semaphore ownership is
 * cleared only after concrete PS2 kernel success.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-AUDIO-EXECUTION-BINDING-R37.
 */

#ifndef PSTVNC_AUDIO_PS2_RUNTIME_H
#define PSTVNC_AUDIO_PS2_RUNTIME_H

#include "session.h"

#include <stddef.h>

#define PSTVNC_AUDIO_PS2_RUNTIME_MAX_ALLOCATIONS 2u

typedef struct pstvnc_audio_ps2_resident {
    int libsd_loaded;
    int audsrv_loaded;
} pstvnc_audio_ps2_resident_t;

typedef struct pstvnc_audio_ps2_allocation {
    void *raw_memory;
    void *aligned_memory;
} pstvnc_audio_ps2_allocation_t;

typedef struct pstvnc_audio_ps2_runtime {
    int session_lock_sema_id;
    int completion_sema_id;

    int thread_id;
    pstvnc_audio_session_thread_entry_t thread_entry;
    void *thread_argument;

    pstvnc_audio_ps2_allocation_t
        allocations[PSTVNC_AUDIO_PS2_RUNTIME_MAX_ALLOCATIONS];
    size_t live_allocations;

    int resources_owned;
    int initialized;
    int thread_slot_active;
    int thread_started;
    int completion_observed;
    int thread_dormant_proven;
} pstvnc_audio_ps2_runtime_t;

int pstvnc_audio_ps2_resident_prepare(
    pstvnc_audio_ps2_resident_t *resident);

int pstvnc_audio_ps2_runtime_init(
    pstvnc_audio_ps2_runtime_t *runtime);

int pstvnc_audio_ps2_runtime_operations(
    pstvnc_audio_ps2_runtime_t *runtime,
    pstvnc_audio_session_memory_ops_t *memory_ops,
    pstvnc_audio_session_thread_ops_t *thread_ops,
    pstvnc_audio_session_sync_t *sync);

int pstvnc_audio_ps2_runtime_release(
    pstvnc_audio_ps2_runtime_t *runtime);

#endif /* PSTVNC_AUDIO_PS2_RUNTIME_H */
