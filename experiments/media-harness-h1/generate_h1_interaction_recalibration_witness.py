#!/usr/bin/env python3
"""
File synopsis:
    Generate an observation-only H1 interaction coordinator with persistent
    Start+Select/deferred-calibration stage witnesses.

Purpose:
    Preserve the exact CP2P interaction behavior while making the outer
    calibration-entry transaction visible in live telemetry word 39. This is a
    disposable hardware-debugging derivative; it changes no input policy,
    timing, ownership, RFB behavior, or calibration semantics.
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


SERVICE_HOLD = r"""static int h1_interaction_service_calibration_entry_hold(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    int activate_calibration = 0;
    int enter_now = 0;
    int activation_result;

    if (coordinator == NULL || session == NULL)
        return 0;

    if (coordinator->calibration_entry_deferred) {
        h1_interaction_recalibration_witness(coordinator, 0xE0000007u);
        if (!h1_interaction_query_calibration_entry_gate(
                coordinator, &enter_now))
            return 0;
        h1_interaction_recalibration_witness(coordinator, 0xE0000008u);

        if (!enter_now)
            return 1;

        coordinator->calibration_entry_deferred = 0;
        h1_interaction_recalibration_witness(coordinator, 0xE0000009u);
        h1_interaction_recalibration_witness(coordinator, 0xE000000Au);
        activation_result = h1_interaction_activate_calibration(
            coordinator, session);
        h1_interaction_recalibration_witness(coordinator, 0xE000000Bu);
        return activation_result;
    }

    if (!pstvnc_h1_mpeg_calibration_entry_hold_poll(
            &coordinator->mpeg_calibration_entry_hold,
            h1_interaction_monotonic_us(),
            h1_interaction_calibration_entry_available(coordinator),
            &activate_calibration))
        return 0;

    if (!activate_calibration)
        return 1;

    h1_interaction_recalibration_witness(coordinator, 0xE0000002u);
    h1_interaction_recalibration_witness(coordinator, 0xE0000003u);
    if (!h1_interaction_query_calibration_entry_gate(
            coordinator, &enter_now))
        return 0;
    h1_interaction_recalibration_witness(coordinator, 0xE0000004u);

    if (!enter_now) {
        h1_interaction_recalibration_witness(coordinator, 0xE0000005u);
        coordinator->calibration_entry_deferred = 1;
        h1_interaction_recalibration_witness(coordinator, 0xE0000006u);
        return 1;
    }

    h1_interaction_recalibration_witness(coordinator, 0xE000000Au);
    activation_result = h1_interaction_activate_calibration(
        coordinator, session);
    h1_interaction_recalibration_witness(coordinator, 0xE000000Bu);
    return activation_result;
}"""


def generate(source: str) -> str:
    source = replace_once(
        source,
        '#include "h1_interaction_coordinator.h"\n',
        '#include "h1_interaction_coordinator.h"\n'
        '#include "h1_cp2p_session_coordinator.h"\n'
        '#include "h1_transport_runtime.h"\n',
        "diagnostic CP2P bridge includes",
    )

    chord_macro = r"""#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD \
    (PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT)
"""
    helper = r"""

/*
 * Observation-only bridge for the CP2P hardware-debugging derivative. The
 * installed calibration-entry gate context is the owning CP2P coordinator.
 * Telemetry word 39 is already diagnostic-only before MEDIA_END on this branch.
 */
static void h1_interaction_recalibration_witness(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    uint32_t stage)
{
    pstvnc_h1_cp2p_session_coordinator_t *cp2p;

    if (coordinator == NULL ||
        coordinator->calibration_entry_gate_context == NULL)
        return;

    cp2p = (pstvnc_h1_cp2p_session_coordinator_t *)
        coordinator->calibration_entry_gate_context;

    if (cp2p->transport != NULL && cp2p->transport->end_received == 0u)
        cp2p->transport->producer_stop_reason = stage;
}
"""
    source = replace_once(
        source,
        chord_macro,
        chord_macro + helper,
        "interaction witness helper",
    )

    chord_sample = r"""    entry_available =
        h1_interaction_calibration_entry_available(coordinator);
    entry_chord_down =
        (controller_state->buttons_down &
         PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD) ==
        PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD;
"""
    chord_sample_instrumented = chord_sample + r"""
    if (entry_available && entry_chord_down)
        h1_interaction_recalibration_witness(coordinator, 0xE0000001u);
"""
    source = replace_once(
        source,
        chord_sample,
        chord_sample_instrumented,
        "Start+Select chord witness",
    )

    source = replace_between(
        source,
        "static int h1_interaction_service_calibration_entry_hold(",
        "static int h1_interaction_resume_desktop_mouse_if_ready(",
        SERVICE_HOLD,
        "calibration-entry hold service",
    )

    return source


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = Path(args.input).read_text(encoding="utf-8")
    generated = generate(source)

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(generated, encoding="utf-8")

    print("H1_CP2P_INTERACTION_RECALIBRATION_WITNESS=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
