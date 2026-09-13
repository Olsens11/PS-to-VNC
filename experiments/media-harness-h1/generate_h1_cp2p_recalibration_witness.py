#!/usr/bin/env python3
"""
File synopsis:
    Generate observation-only CP2P recalibration/retirement witness sources.

Purpose:
    Preserve the exact priority-67 persistent-witness baseline while moving live
    telemetry word 39 from the MPEG D-stage witness to a deterministic E-stage
    witness once calibration entry/retirement is exercised. The generated
    session coordinator and MPEG worker differ only by persistent stage writes;
    the generated video runtime stops mirroring D-stages into word 39 so those
    writes cannot race the recalibration witness.

No transport ownership, queue sizing, timing, scheduling, geometry, decoder,
RFB flow, retirement semantics, or recovery behavior is changed.
"""

from __future__ import annotations

import argparse
from pathlib import Path


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def replace_between(
    text: str,
    start_marker: str,
    end_marker: str,
    replacement: str,
    label: str,
) -> str:
    start_count = text.count(start_marker)
    end_count = text.count(end_marker)
    if start_count != 1 or end_count != 1:
        raise RuntimeError(
            f"{label}: marker counts start={start_count} end={end_count}"
        )
    start = text.index(start_marker)
    end = text.index(end_marker, start)
    return text[:start] + replacement.rstrip() + "\n\n" + text[end:]


SESSION_CLEAR = r"""static int h1_cp2p_session_clear_mpeg_after_pi_retire(
    void *context,
    uint32_t generation)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    uint32_t discarded = 0u;

    if (coordinator == NULL || !coordinator->initialized ||
        coordinator->clear_mpeg == NULL || !coordinator->pi_retire_pending ||
        coordinator->pi_retire_generation != generation)
        return 0;

    h1_cp2p_recalibration_witness(coordinator, 0xE2000001u);
    h1_cp2p_recalibration_witness(coordinator, 0xE2000002u);

    /* Stop the old decoder/pixels before touching its transport queue. */
    if (!coordinator->clear_mpeg(
            coordinator->clear_mpeg_context, generation))
        return 0;

    h1_cp2p_recalibration_witness(coordinator, 0xE2000003u);
    h1_cp2p_recalibration_witness(coordinator, 0xE2000004u);

    /* ACK was the wire fence; discard only the now-unconsumed old epoch. */
    if (!pstvnc_h1_transport_mpeg_retire_finalize(
            coordinator->transport, generation, &discarded))
        return 0;

    h1_cp2p_recalibration_witness(coordinator, 0xE2000005u);
    return 1;
}"""

SESSION_GATE = r"""static int h1_cp2p_session_calibration_entry_gate(
    void *context,
    int *enter_now)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    pstvnc_mpeg_calibration_t *calibration;
    uint32_t generation;
    int retire_poll;

    if (coordinator == NULL || !coordinator->initialized ||
        enter_now == NULL)
        return 0;

    h1_cp2p_recalibration_witness(coordinator, 0xE1000001u);

    if (coordinator->recalibration.awaiting_restored_full_rfb) {
        int entry_ready;

        h1_cp2p_recalibration_witness(coordinator, 0xE100000Au);
        entry_ready = pstvnc_h1_mpeg_recalibration_take_entry_ready(
            &coordinator->recalibration,
            &coordinator->rfb_flow);
        *enter_now = entry_ready ? 1 : 0;
        h1_cp2p_recalibration_witness(
            coordinator,
            entry_ready ? 0xE100000Bu : 0xE1000009u);
        return 1;
    }

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    if (coordinator->pi_retire_pending) {
        h1_cp2p_recalibration_witness(coordinator, 0xE1000004u);
        retire_poll = pstvnc_h1_transport_mpeg_retire_poll(
            coordinator->transport,
            coordinator->pi_retire_generation);
        if (retire_poll < 0)
            return 0;
        if (retire_poll == 0) {
            h1_cp2p_recalibration_witness(coordinator, 0xE1000005u);
            *enter_now = 0;
            return 1;
        }

        h1_cp2p_recalibration_witness(coordinator, 0xE1000006u);
        /* Keep the exact retirement latched through local worker + queue cleanup. */
    } else if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY) {
        generation = coordinator->mpeg_handoff.owner.generation;
        if (generation == 0u)
            return 0;

        h1_cp2p_recalibration_witness(coordinator, 0xE1000002u);
        if (!pstvnc_h1_transport_mpeg_retire_begin(
                coordinator->transport, generation))
            return 0;
        h1_cp2p_recalibration_witness(coordinator, 0xE1000003u);

        coordinator->pi_retire_generation = generation;
        coordinator->pi_retire_pending = 1;
        *enter_now = 0;
        return 1;
    }

    /*
     * Pi completion is now proven for the exact old generation. Only here may
     * local worker/presentation state retire and the one-full-RFB restoration
     * obligation become visible to the request scheduler.
     */
    calibration = &coordinator->interaction.mpeg_calibration.runtime
        .foreground.adapter.calibration;

    h1_cp2p_recalibration_witness(coordinator, 0xE1000007u);
    if (!pstvnc_h1_mpeg_recalibration_begin(
            &coordinator->recalibration,
            &coordinator->mpeg_handoff,
            calibration,
            &coordinator->rfb_flow,
            coordinator->pi_retire_pending
                ? h1_cp2p_session_clear_mpeg_after_pi_retire
                : coordinator->clear_mpeg,
            coordinator->pi_retire_pending
                ? coordinator
                : coordinator->clear_mpeg_context,
            &begin_result))
        return 0;
    h1_cp2p_recalibration_witness(coordinator, 0xE1000008u);

    if (coordinator->pi_retire_pending) {
        coordinator->pi_retire_pending = 0;
        coordinator->pi_retire_generation = 0u;
    }

    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW) {
        h1_cp2p_recalibration_witness(coordinator, 0xE100000Cu);
        *enter_now = 1;
        return 1;
    }

    if (begin_result != PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB)
        return 0;

    coordinator->current_start_contract_valid = 0;
    h1_cp2p_recalibration_witness(coordinator, 0xE1000009u);
    *enter_now = 0;
    return 1;
}"""

WORKER_JOIN = r"""static int h1_cp2p_mpeg_worker_join(
    pstvnc_h1_cp2p_mpeg_worker_t *worker)
{
    unsigned int loops = 0u;

    h1_cp2p_recalibration_worker_witness(worker, 0xE3000002u);

    if (!worker->thread_started) {
        h1_cp2p_recalibration_worker_witness(worker, 0xE3000000u);
        return 1;
    }

    worker->stop_requested = 1;
    h1_cp2p_recalibration_worker_witness(worker, 0xE3000003u);
    h1_cp2p_recalibration_worker_witness(worker, 0xE3000004u);

    while (!worker->finished && loops < H1_CP2P_MPEG_WORKER_STOP_MAX_LOOPS) {
        ee_thread_status_t status;

        /*
         * Memory-only monotonic witness. It rides the already-existing telemetry
         * cadence; it does not print or add traffic.
         */
        h1_cp2p_recalibration_worker_witness(
            worker, 0xE3100000u | (loops & 0x0000ffffu));

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(worker->thread_id, &status) < 0) {
            h1_cp2p_recalibration_worker_witness(worker, 0xE3FF0002u);
            return 0;
        }
        if (status.status == THS_DORMANT) {
            h1_cp2p_recalibration_worker_witness(worker, 0xE3000005u);
            worker->finished = 1;
            break;
        }
        if (DelayThread(H1_CP2P_MPEG_WORKER_STOP_POLL_US) < 0) {
            h1_cp2p_recalibration_worker_witness(worker, 0xE3FF0003u);
            return 0;
        }
        loops++;
    }

    if (!worker->finished) {
        h1_cp2p_recalibration_worker_witness(worker, 0xE3FF0001u);
        return 0;
    }

    h1_cp2p_recalibration_worker_witness(worker, 0xE3000006u);
    h1_cp2p_recalibration_worker_witness(worker, 0xE3000007u);
    if (DeleteThread(worker->thread_id) < 0) {
        h1_cp2p_recalibration_worker_witness(worker, 0xE3FF0004u);
        return 0;
    }
    h1_cp2p_recalibration_worker_witness(worker, 0xE3000008u);

    worker->thread_started = 0;
    worker->thread_id = -1;
    free(worker->thread_stack_allocation);
    worker->thread_stack_allocation = NULL;
    worker->thread_stack = NULL;
    return 1;
}"""

WORKER_CLEAR = r"""int pstvnc_h1_cp2p_mpeg_worker_clear(void *context, uint32_t generation)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)context;

    if (worker == NULL || !worker->initialized || !worker->armed ||
        generation == 0u || worker->generation != generation)
        return 0;

    h1_cp2p_recalibration_worker_witness(worker, 0xE3000001u);
    if (!h1_cp2p_mpeg_worker_join(worker))
        return 0;

    if (worker->live_decode) {
        h1_cp2p_recalibration_worker_witness(worker, 0xE3000009u);
        if (pstvnc_h1_graphics_clear_video() < 0) {
            h1_cp2p_recalibration_worker_witness(worker, 0xE3FF0005u);
            return 0;
        }
        h1_cp2p_recalibration_worker_witness(worker, 0xE300000Au);
    }

    memset(&worker->contract, 0, sizeof(worker->contract));
    worker->handoff = NULL;
    worker->generation = 0u;
    worker->live_decode = 0;
    worker->armed = 0;
    h1_cp2p_recalibration_worker_witness(worker, 0xE300000Bu);
    return 1;
}"""


def generate_session(source: str) -> str:
    source = replace_once(
        source,
        '#include "h1_config.h"\n',
        '#include "h1_config.h"\n#include "h1_transport_runtime.h"\n',
        "session transport include",
    )
    helper_anchor = '#include <string.h>\n'
    helper = r"""
/*
 * Observation-only live recalibration witness. Word 39 is already reserved by
 * the current disposable persistent-witness branch before MEDIA_END.
 */
static void h1_cp2p_recalibration_witness(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    uint32_t stage)
{
    if (coordinator != NULL && coordinator->transport != NULL &&
        coordinator->transport->end_received == 0u)
        coordinator->transport->producer_stop_reason = stage;
}
"""
    source = replace_once(
        source,
        helper_anchor,
        helper_anchor + helper,
        "session witness helper",
    )
    source = replace_between(
        source,
        "static int h1_cp2p_session_clear_mpeg_after_pi_retire(",
        "static int h1_cp2p_session_calibration_entry_gate(",
        SESSION_CLEAR,
        "session clear-after-retire",
    )
    source = replace_between(
        source,
        "static int h1_cp2p_session_calibration_entry_gate(",
        "static int h1_cp2p_session_start_accepted_calibration(",
        SESSION_GATE,
        "session calibration gate",
    )
    return source


def generate_worker(source: str) -> str:
    helper_anchor = "static unsigned char *h1_worker_align64(void *allocation)\n"
    helper = r"""/*
 * Observation-only live recalibration witness. This uses the existing
 * pre-MEDIA_END word-39 diagnostic slot and does not alter worker policy.
 */
static void h1_cp2p_recalibration_worker_witness(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    uint32_t stage)
{
    if (worker != NULL && worker->transport != NULL &&
        worker->transport->end_received == 0u)
        worker->transport->producer_stop_reason = stage;
}

"""
    source = replace_once(
        source,
        helper_anchor,
        helper + helper_anchor,
        "worker witness helper",
    )
    source = replace_between(
        source,
        "static int h1_cp2p_mpeg_worker_join(",
        "int pstvnc_h1_cp2p_mpeg_worker_init(",
        WORKER_JOIN,
        "worker join",
    )
    source = replace_between(
        source,
        "int pstvnc_h1_cp2p_mpeg_worker_clear(",
        "int pstvnc_h1_cp2p_mpeg_worker_shutdown(",
        WORKER_CLEAR,
        "worker clear",
    )
    return source


def generate_video(source: str) -> str:
    old = "            session->transport->producer_stop_reason = stage;\n"
    new = (
        "            /* Recalibration witness owns live telemetry word 39 in this "
        "derivative. */\n"
        "            (void)stage;\n"
    )
    return replace_once(
        source,
        old,
        new,
        "disable competing persistent MPEG word-39 writer",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", required=True, choices=("session", "worker", "video"))
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = Path(args.input).read_text(encoding="utf-8")

    if args.mode == "session":
        generated = generate_session(source)
    elif args.mode == "worker":
        generated = generate_worker(source)
    else:
        generated = generate_video(source)

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(generated, encoding="utf-8")

    print(f"H1_CP2P_RECALIBRATION_WITNESS_GENERATED={args.mode}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
