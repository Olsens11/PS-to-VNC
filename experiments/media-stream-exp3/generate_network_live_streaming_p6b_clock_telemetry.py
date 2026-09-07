#!/usr/bin/env python3
"""
File synopsis:
    Generates the EXP3 P6B instrumentation-only harness from the exact P6
    absolute-clock generator output.

Purpose:
    P6 improved visible cadence, but its clock diagnostics were printed only
    to the PS2 console and therefore were not observable by the Pi replay
    harness. P6B leaves the P6 presentation scheduler unchanged and adds one
    terminal UDP datagram after playback has already completed. The datagram
    reports P6 VSYNC-wait/deadline-miss counters without changing the MPEG
    transport protocol or playback-time behavior.

    This is instrumentation only. It does not alter source MPEG bytes, queue
    size, decoder behavior, GS upload/draw behavior, frame deadlines, or frame
    dropping policy.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P6_GENERATOR_BLOB = "c4b29689c64e78c2d09d1d052e17ac77ac4c3118"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one target, found {count}")
    return text.replace(old, new, 1)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = pathlib.Path(args.input)
    output = pathlib.Path(args.output)
    p6_generator = source.parent / "generate_network_live_streaming_p6_absolute_clock.py"

    actual_generator_blob = subprocess.check_output(
        ["git", "hash-object", str(p6_generator)],
        text=True,
    ).strip()

    print(f"P6B_P6_GENERATOR_BLOB={actual_generator_blob}")

    if actual_generator_blob != EXPECTED_P6_GENERATOR_BLOB:
        raise SystemExit(
            "P6 generator authority mismatch: "
            f"expected={EXPECTED_P6_GENERATOR_BLOB} actual={actual_generator_blob}"
        )

    base = output.with_name(output.name + ".p6-base.tmp")

    try:
        subprocess.run(
            [
                sys.executable,
                str(p6_generator),
                "--input",
                str(source),
                "--output",
                str(base),
            ],
            check=True,
        )

        text = base.read_text()

        helper = r'''
#define EXP3_P6B_CLOCK_TELEMETRY_IP   "192.168.50.1"
#define EXP3_P6B_CLOCK_TELEMETRY_PORT 5999

static void exp3_p6b_send_clock_telemetry(
    const Exp3DecoderState *state)
{
    struct sockaddr_in address;
    char payload[256];
    int socket_fd;
    int payload_length;
    int sent;

    if (state == NULL)
        return;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        printf("EXP3_P6B_CLOCK_UDP=SOCKET_FAIL\n");
        return;
    }

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(EXP3_P6B_CLOCK_TELEMETRY_PORT);
    address.sin_addr.s_addr = inet_addr(EXP3_P6B_CLOCK_TELEMETRY_IP);

    payload_length = snprintf(
        payload,
        sizeof(payload),
        "EXP3_P6B_CLOCK "
        "fps_num=%u fps_den=%u "
        "frame_period_ticks=%llu "
        "vsync_waits=%u "
        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu",
        EXP3_P6_SOURCE_FPS_NUMERATOR,
        EXP3_P6_SOURCE_FPS_DENOMINATOR,
        (unsigned long long)(
            ((u64)kBUSCLK *
             (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR) /
            (u64)EXP3_P6_SOURCE_FPS_NUMERATOR),
        state->p6_vsync_waits,
        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks);

    if (
        payload_length <= 0 ||
        payload_length >= (int)sizeof(payload)
    ) {
        printf("EXP3_P6B_CLOCK_UDP=FORMAT_FAIL\n");
        close(socket_fd);
        return;
    }

    sent = sendto(
        socket_fd,
        payload,
        (size_t)payload_length,
        0,
        (struct sockaddr *)&address,
        sizeof(address));

    if (sent == payload_length)
        printf("EXP3_P6B_CLOCK_UDP=PASS bytes=%d\n", sent);
    else
        printf("EXP3_P6B_CLOCK_UDP=SEND_FAIL sent=%d expected=%d\n", sent, payload_length);

    close(socket_fd);
}

'''

        text = replace_once(
            text,
            "int main(void)\n",
            helper + "int main(void)\n",
            "P6B telemetry helper",
        )

        terminal_marker = '''        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
'''

        telemetry_call = '''        /*
         * P6B instrumentation-only side channel. Playback is already over,
         * so this datagram cannot perturb the measured presentation cadence.
         */
        exp3_p6b_send_clock_telemetry(
            &decoder);

        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
'''

        text = replace_once(
            text,
            terminal_marker,
            telemetry_call,
            "P6B terminal telemetry call",
        )

        if text.count("exp3_p6b_send_clock_telemetry(") != 2:
            raise SystemExit("P6B expected helper definition plus one call")

        if text.count("EXP3_P6B_CLOCK ") != 1:
            raise SystemExit("P6B clock payload marker missing")

        if text.count("exp3_p6_wait_for_picture(") != 4:
            raise SystemExit("P6B unexpectedly changed P6 scheduler call structure")

        if text.count("graph_wait_vsync();") != 5:
            raise SystemExit("P6B unexpectedly changed P6 VSYNC call sites")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P6B_OUTPUT={output}")
        print(f"P6B_OUTPUT_BLOB={generated_blob}")
        print("P6B_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
