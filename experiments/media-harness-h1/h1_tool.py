#!/usr/bin/env python3
"""
File synopsis:
    Canonical operator/debug control surface for the resident H1 media harness.

This tool deliberately does not reimplement H1 transport, media scheduling, or
validation. It turns the existing profile/configuration surface into a stable
human-facing CLI and delegates actual sessions to the current cumulative H1
runner.

Goals:
    * preserve discoverable knowledge of every H1 adjustable field;
    * make one-off hardware tests concise and reproducible;
    * provide friendly switches for frequently changed media geometry/modes;
    * generate repeatable multi-parameter sweeps without bespoke test scripts;
    * keep the underlying qualified runner as the behavioral authority.

Typical use:
    python3 experiments/media-harness-h1/h1_tool.py knobs
    python3 experiments/media-harness-h1/h1_tool.py show --profile P11_COMPAT_PLUS_PCM
    python3 experiments/media-harness-h1/h1_tool.py run --profile P11_COMPAT_PLUS_PCM \
        --audio-priority 8 --capture 16,112,432,240 --encode 432x240 \
        --draw 16,112,432,240 --duration 60
    python3 experiments/media-harness-h1/h1_tool.py sweep \
        --profile P11_COMPAT_PLUS_PCM --duration 20 \
        --vary audio_thread_priority=2,8,16 \
        --vary mpeg_queue_capacity=262144,524288

The current H1 CONFIG still treats RFB=ON as reserved/not implemented. The
friendly --rfb switch exposes that field so the operator interface will not
need redesign when channel-1 RFB activation arrives; current hardware sessions
will still be rejected by the authoritative H1 validator if an unsupported
combination is requested.
"""

from __future__ import annotations

import argparse
import itertools
import json
import os
from pathlib import Path
import subprocess
import sys
import threading
import time
from typing import Iterable, Mapping

THIS_DIR = Path(__file__).resolve().parent
if str(THIS_DIR) not in sys.path:
    sys.path.insert(0, str(THIS_DIR))

import h1_profiles as profiles

DEFAULT_RUNNER = THIS_DIR / "h1_mux_server_cumulative39_thread_census.py"
DEFAULT_DISPLAY = os.environ.get("DISPLAY", ":0.0")

# Concise operator-facing explanations. Unknown future profile fields remain
# automatically discoverable through h1_profiles and receive a fallback label.
KNOB_HELP = {
    "profile_id": "Profile identity returned in H1 result validation; normally leave unchanged.",
    "session_id": "Per-session identity; normally generated automatically.",
    "audio_mode": "0=audio off, 1=48 kHz signed-16 stereo PCM path.",
    "video_mode": "0=video off, 1=MPEG-2 elementary-stream path.",
    "audio_queue_capacity": "PS2 EE audio queue capacity in bytes; 0 when audio is off.",
    "mpeg_queue_capacity": "PS2 EE MPEG queue capacity in bytes.",
    "audio_credit_batch_bytes": "Consumed audio bytes accumulated before returning mux credit.",
    "mpeg_credit_batch_bytes": "Consumed MPEG bytes accumulated before returning mux credit.",
    "audio_credit_flush_on_empty": "Return pending audio credit immediately when queue becomes empty.",
    "mpeg_credit_flush_on_empty": "Return pending MPEG credit immediately when queue becomes empty.",
    "audio_credit_return_enabled": "Enable AUDIO channel receiver-credit return.",
    "mpeg_credit_return_enabled": "Enable MPEG channel receiver-credit return.",
    "audio_initial_credit_bytes": "Initial AUDIO producer credit granted by PS2.",
    "mpeg_initial_credit_bytes": "Initial MPEG producer credit granted by PS2.",
    "audio_start_mode": "0=immediate audio start, 1=wait for configured target prefill.",
    "audio_start_target_bytes": "Audio queue target required by TARGET startup mode.",
    "audio_start_delay_us": "Additional audio presentation/start delay in microseconds.",
    "audio_chunk_bytes": "Bytes submitted to AUDSRV per audio consumer chunk.",
    "audio_idle_delay_us": "Audio consumer delay while waiting for data.",
    "audio_thread_priority": "EE priority of H1 audio worker; lower number is higher EE priority.",
    "audio_thread_stack_size": "EE stack size of H1 audio worker in bytes.",
    "audio_rate": "PCM sample rate in Hz.",
    "audio_channels": "PCM channel count.",
    "audio_bits": "PCM bits per sample.",
    "audio_volume": "AUDSRV playback volume percentage/value used by H1.",
    "audio_presentation_offset_us": "Signed audio offset from shared media epoch in microseconds.",
    "mpeg_start_target_bytes": "MPEG queue prefill target before video decode begins.",
    "mpeg_prefill_wait_us": "Delay between MPEG prefill checks.",
    "mpeg_prefill_max_loops": "Maximum MPEG prefill wait-loop count.",
    "mpeg_empty_delay_us": "Decoder/feed wait while MPEG queue is temporarily empty.",
    "mpeg_feed_bytes": "Bytes requested per MPEG/IPU feed operation.",
    "video_fps_num": "Video frame-rate numerator.",
    "video_fps_den": "Video frame-rate denominator.",
    "video_scheduler_mode": "Video presentation scheduler selector; qualified mode is absolute scheduling.",
    "video_presentation_offset_us": "Signed video offset from shared media epoch in microseconds.",
    "video_pixel_mode": "Decoded/presentation pixel-mode selector; qualified path is RGB16.",
    "video_max_width": "Maximum decoded sequence width accepted by H1.",
    "video_max_height": "Maximum decoded sequence height accepted by H1.",
    "video_draw_width": "GS destination width for MPEG presentation.",
    "video_draw_height": "GS destination height for MPEG presentation.",
    "video_draw_x": "GS destination X coordinate for MPEG presentation.",
    "video_draw_y": "GS destination Y coordinate for MPEG presentation.",
    "video_stage_markers": "Enable/disable historical H1 stage-color markers.",
    "video_stage_hold_vsyncs": "Number of vsyncs to hold each enabled stage marker.",
    "video_ipu_reset_each_session": "Reset IPU state at each video session start.",
    "video_drop_enabled": "Enable experimental late-frame dropping; qualified media path keeps this off.",
    "video_drop_threshold_milliframes": "Late threshold for experimental frame dropping, in milliframes.",
    "receiver_thread_priority": "EE priority of the sole H1 PSTV receiver thread.",
    "receiver_thread_stack_size": "EE stack size of H1 receiver thread in bytes.",
    "max_data_payload": "Maximum PSTV DATA payload; current wire authority is 8192 bytes.",
    "socket_receive_buffer_bytes": "Optional TCP receive-buffer request; 0 leaves platform/default behavior.",
    "socket_send_buffer_bytes": "Optional TCP send-buffer request; 0 leaves platform/default behavior.",
    "queue_allocation_order": "0=allocate AUDIO first, 1=allocate MPEG first.",
    "media_epoch_lead_us": "Lead time before the shared media presentation epoch begins.",
    "rfb_mode": "0=off; 1=reserved RFB-on value (current H1 validator still rejects it).",
    "video_encode_width": "MPEG encoder output width; current H1 path requires 16-pixel alignment.",
    "video_encode_height": "MPEG encoder output height; current H1 path requires 16-pixel alignment.",
    "desktop_width": "Pi-owned active desktop width used for capture geometry validation.",
    "desktop_height": "Pi-owned active desktop height used for capture geometry validation.",
    "video_capture_x": "Pi X11 source-region X coordinate in active desktop pixels.",
    "video_capture_y": "Pi X11 source-region Y coordinate in active desktop pixels.",
    "video_capture_width": "Pi X11 source-region width before MPEG scaling/encoding.",
    "video_capture_height": "Pi X11 source-region height before MPEG scaling/encoding.",
}

FRIENDLY_HELP = """Frequently used convenience switches map onto normal profile fields:
  --audio off|pcm
  --video off|mpeg2
  --rfb off|on              (on is reserved/currently rejected by H1)
  --audio-priority N
  --receiver-priority N
  --desktop WxH
  --capture X,Y,W,H
  --encode WxH
  --draw X,Y,W,H
  --markers off|on

Every field remains reachable with repeated --set NAME=INTEGER.
Use `h1_tool.py knobs --profile ...` to see all fields and current defaults.
"""


def parse_int(text: str) -> int:
    try:
        return int(text, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc


def parse_assignment(text: str) -> tuple[str, int]:
    if "=" not in text:
        raise argparse.ArgumentTypeError("expected NAME=INTEGER")
    name, raw = text.split("=", 1)
    name = name.strip()
    if not name:
        raise argparse.ArgumentTypeError("field name may not be empty")
    return name, parse_int(raw.strip())


def parse_size(text: str) -> tuple[int, int]:
    normalized = text.lower().replace("×", "x")
    if "x" not in normalized:
        raise argparse.ArgumentTypeError("expected WIDTHxHEIGHT")
    left, right = normalized.split("x", 1)
    try:
        width = int(left, 0)
        height = int(right, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("width and height must be positive")
    return width, height


def parse_rect(text: str) -> tuple[int, int, int, int]:
    parts = [part.strip() for part in text.split(",")]
    if len(parts) != 4:
        raise argparse.ArgumentTypeError("expected X,Y,WIDTH,HEIGHT")
    try:
        x, y, width, height = (int(part, 0) for part in parts)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    if x < 0 or y < 0 or width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("rectangle requires nonnegative X/Y and positive width/height")
    return x, y, width, height


def parse_vary(text: str) -> tuple[str, tuple[int, ...]]:
    if "=" not in text:
        raise argparse.ArgumentTypeError("expected NAME=VALUE[,VALUE...]")
    name, raw_values = text.split("=", 1)
    name = name.strip()
    raw_items = [item.strip() for item in raw_values.split(",") if item.strip()]
    if not name or not raw_items:
        raise argparse.ArgumentTypeError("sweep variation needs a name and at least one value")
    try:
        values = tuple(int(item, 0) for item in raw_items)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    return name, values


def new_session_id(salt: int = 0) -> int:
    value = (time.time_ns() ^ (os.getpid() << 16) ^ salt) & 0xFFFFFFFF
    return value if value != 0 else 1


def set_override(target: dict[str, int], name: str, value: int, source: str) -> None:
    if name in target and target[name] != value:
        raise SystemExit(
            f"conflicting value for {name}: existing={target[name]} new={value} source={source}"
        )
    target[name] = value


def overrides_from_args(args: argparse.Namespace) -> dict[str, int]:
    result: dict[str, int] = {}

    for name, value in getattr(args, "set_values", []) or []:
        set_override(result, name, value, "--set")

    audio = getattr(args, "audio", None)
    if audio is not None:
        set_override(result, "audio_mode", 0 if audio == "off" else 1, "--audio")

    video = getattr(args, "video", None)
    if video is not None:
        set_override(result, "video_mode", 0 if video == "off" else 1, "--video")

    rfb = getattr(args, "rfb", None)
    if rfb is not None:
        set_override(result, "rfb_mode", 0 if rfb == "off" else 1, "--rfb")

    audio_priority = getattr(args, "audio_priority", None)
    if audio_priority is not None:
        set_override(result, "audio_thread_priority", audio_priority, "--audio-priority")

    receiver_priority = getattr(args, "receiver_priority", None)
    if receiver_priority is not None:
        set_override(result, "receiver_thread_priority", receiver_priority, "--receiver-priority")

    desktop = getattr(args, "desktop", None)
    if desktop is not None:
        set_override(result, "desktop_width", desktop[0], "--desktop")
        set_override(result, "desktop_height", desktop[1], "--desktop")

    capture = getattr(args, "capture", None)
    if capture is not None:
        x, y, width, height = capture
        set_override(result, "video_capture_x", x, "--capture")
        set_override(result, "video_capture_y", y, "--capture")
        set_override(result, "video_capture_width", width, "--capture")
        set_override(result, "video_capture_height", height, "--capture")

    encode = getattr(args, "encode", None)
    if encode is not None:
        set_override(result, "video_encode_width", encode[0], "--encode")
        set_override(result, "video_encode_height", encode[1], "--encode")

    draw = getattr(args, "draw", None)
    if draw is not None:
        x, y, width, height = draw
        set_override(result, "video_draw_x", x, "--draw")
        set_override(result, "video_draw_y", y, "--draw")
        set_override(result, "video_draw_width", width, "--draw")
        set_override(result, "video_draw_height", height, "--draw")

    markers = getattr(args, "markers", None)
    if markers is not None:
        set_override(result, "video_stage_markers", 0 if markers == "off" else 1, "--markers")

    return result


def profile_field_order(name: str) -> tuple[int, str]:
    if name == "profile_id":
        return (-1, name)
    field_id = profiles.FIELD_IDS.get(name)
    if field_id is not None:
        return (field_id, name)
    return (10000, name)


def resolved_profile(args: argparse.Namespace, overrides: Mapping[str, int], session_id: int) -> dict[str, int]:
    return profiles.resolve_profile(args.profile, session_id, overrides)


def add_profile_arguments(parser: argparse.ArgumentParser, include_evidence: bool = True) -> None:
    parser.add_argument(
        "--profile",
        choices=profiles.profile_names(),
        default="P11_COMPAT_VIDEO_ONLY",
        help="named base profile",
    )
    parser.add_argument("--session-id", type=parse_int, help="explicit session ID; normally generated")
    parser.add_argument("--duration", type=float, default=24.0, help="media duration in seconds")
    parser.add_argument("--display", default=DEFAULT_DISPLAY, help="X11 display captured by Pi")
    parser.add_argument("--listen", default="0.0.0.0", help="PS2-facing listen address")
    parser.add_argument("--port", type=int, default=5902, help="PS2-facing PSTV port")
    if include_evidence:
        parser.add_argument("--evidence", help="explicit evidence directory for a single run")
    parser.add_argument("--runner", type=Path, default=DEFAULT_RUNNER, help="underlying H1 runner; normally leave unchanged")

    parser.add_argument("--set", dest="set_values", action="append", default=[], type=parse_assignment, metavar="NAME=INTEGER", help="override any H1 profile field; repeatable")
    parser.add_argument("--audio", choices=("off", "pcm"), help="friendly audio_mode switch")
    parser.add_argument("--video", choices=("off", "mpeg2"), help="friendly video_mode switch")
    parser.add_argument("--rfb", choices=("off", "on"), help="friendly rfb_mode switch; current on value remains reserved")
    parser.add_argument("--audio-priority", type=parse_int, help="friendly audio_thread_priority override")
    parser.add_argument("--receiver-priority", type=parse_int, help="friendly receiver_thread_priority override")
    parser.add_argument("--desktop", type=parse_size, metavar="WxH", help="active Pi desktop geometry")
    parser.add_argument("--capture", type=parse_rect, metavar="X,Y,W,H", help="Pi MPEG source rectangle")
    parser.add_argument("--encode", type=parse_size, metavar="WxH", help="MPEG encoded dimensions")
    parser.add_argument("--draw", type=parse_rect, metavar="X,Y,W,H", help="PS2 MPEG destination rectangle")
    parser.add_argument("--markers", choices=("off", "on"), help="historical H1 stage-color markers")


def runner_command(
    args: argparse.Namespace,
    session_id: int,
    overrides: Mapping[str, int],
    evidence: Path | None,
    validate_only: bool,
) -> list[str]:
    runner = Path(args.runner).expanduser()
    if not runner.is_absolute():
        runner = (Path.cwd() / runner).resolve()
    if not runner.exists():
        raise SystemExit(f"H1 runner does not exist: {runner}")

    command = [
        sys.executable,
        str(runner),
        "--profile",
        args.profile,
        "--session-id",
        str(session_id),
        "--duration",
        str(args.duration),
        "--display",
        args.display,
        "--listen",
        args.listen,
        "--port",
        str(args.port),
    ]

    if evidence is not None:
        command.extend(("--evidence", str(evidence)))

    for name in sorted(overrides, key=profile_field_order):
        command.extend(("--override", f"{name}={overrides[name]}"))

    if validate_only:
        command.append("--validate-only")

    return command


def print_knobs(args: argparse.Namespace) -> int:
    session_id = args.session_id if args.session_id is not None else 1
    base_profile = profiles.resolve_profile(args.profile, session_id)

    print(f"H1_TOOL_PROFILE={args.profile}")
    print("H1_TOOL_KNOBS_BEGIN")
    print(f"{'FIELD':36} {'DEFAULT':>12} {'SCOPE':8} DESCRIPTION")

    for name in sorted(base_profile, key=profile_field_order):
        scope = "PI_ONLY" if name in profiles.PI_ONLY_FIELDS else "WIRE"
        description = KNOB_HELP.get(name, "H1 profile field; see source/profile validator for current semantics.")
        print(f"{name:36} {str(base_profile[name]):>12} {scope:8} {description}")

    print("H1_TOOL_KNOBS_END")
    print()
    print(FRIENDLY_HELP.rstrip())
    return 0


def print_show(args: argparse.Namespace) -> int:
    if args.duration <= 0:
        raise SystemExit("--duration must be positive")
    overrides = overrides_from_args(args)
    session_id = args.session_id if args.session_id is not None else new_session_id()
    profile = resolved_profile(args, overrides, session_id)
    command = runner_command(args, session_id, overrides, None, True)

    print("H1_TOOL_RESOLVED_PROFILE=" + json.dumps(profile, sort_keys=True))
    print("H1_TOOL_OVERRIDES=" + json.dumps(overrides, sort_keys=True))
    print("H1_TOOL_VALIDATE_COMMAND=" + json.dumps(command))
    return 0


def run_once(args: argparse.Namespace) -> int:
    if args.duration <= 0:
        raise SystemExit("--duration must be positive")
    overrides = overrides_from_args(args)
    session_id = args.session_id if args.session_id is not None else new_session_id()
    profile = resolved_profile(args, overrides, session_id)
    evidence = Path(args.evidence).expanduser() if args.evidence else None
    command = runner_command(args, session_id, overrides, evidence, args.validate_only)

    print("H1_TOOL_MODE=RUN", flush=True)
    print("H1_TOOL_PROFILE=" + json.dumps(profile, sort_keys=True), flush=True)
    print("H1_TOOL_OVERRIDES=" + json.dumps(overrides, sort_keys=True), flush=True)
    print("H1_TOOL_COMMAND=" + json.dumps(command), flush=True)
    return subprocess.call(command, cwd=THIS_DIR)


def build_sweep_cases(
    base_overrides: Mapping[str, int],
    variations: Iterable[tuple[str, tuple[int, ...]]],
    mode: str,
) -> list[dict[str, int]]:
    variation_list = list(variations)
    if not variation_list:
        raise SystemExit("sweep requires at least one --vary NAME=VALUE[,VALUE...]")

    names = [name for name, _ in variation_list]
    if len(set(names)) != len(names):
        raise SystemExit("each --vary field may appear only once")

    for name in names:
        if name in base_overrides:
            raise SystemExit(f"field {name} appears in both base overrides and --vary")

    value_sets = [values for _, values in variation_list]
    if mode == "zip":
        lengths = {len(values) for values in value_sets}
        if len(lengths) != 1:
            raise SystemExit("--mode zip requires every --vary list to have the same length")
        combinations = zip(*value_sets, strict=True)
    else:
        combinations = itertools.product(*value_sets)

    cases: list[dict[str, int]] = []
    for combination in combinations:
        case = dict(base_overrides)
        for name, value in zip(names, combination, strict=True):
            case[name] = value
        cases.append(case)
    return cases


def pump_process(process: subprocess.Popen[str], log_path: Path, markers: set[str]) -> None:
    assert process.stdout is not None
    with log_path.open("w", encoding="utf-8") as log:
        for line in process.stdout:
            print(line, end="", flush=True)
            log.write(line)
            log.flush()
            if "H1_SESSION_VALIDATION=PASS" in line:
                markers.add("session_validation_pass")
            if "H1_PI_MUX_SESSION=PASS" in line:
                markers.add("pi_mux_session_pass")


def run_sweep(args: argparse.Namespace) -> int:
    if args.duration <= 0:
        raise SystemExit("--duration must be positive")

    base_overrides = overrides_from_args(args)
    cases = build_sweep_cases(base_overrides, args.vary, args.mode)

    # Validate all case shapes before touching hardware.
    seed = args.session_id if args.session_id is not None else new_session_id()
    planned = []
    for index, overrides in enumerate(cases, start=1):
        session_id = (seed + index - 1) & 0xFFFFFFFF
        if session_id == 0:
            session_id = index
        profile = resolved_profile(args, overrides, session_id)
        planned.append((index, session_id, overrides, profile))

    stamp = time.strftime("%Y%m%dT%H%M%SZ", time.gmtime())
    root = (
        Path(args.evidence_root).expanduser()
        if args.evidence_root
        else Path.home() / "ps2vnc-evidence" / f"h1-sweep-{stamp}"
    )
    root.mkdir(parents=True, exist_ok=False)

    plan_json = {
        "tool": "h1_tool.py",
        "mode": args.mode,
        "profile": args.profile,
        "duration": args.duration,
        "display": args.display,
        "runner": str(Path(args.runner)),
        "variations": [{"field": name, "values": list(values)} for name, values in args.vary],
        "base_overrides": base_overrides,
        "cases": [
            {
                "case": index,
                "session_id": session_id,
                "overrides": overrides,
                "profile": profile,
            }
            for index, session_id, overrides, profile in planned
        ],
    }
    (root / "sweep-plan.json").write_text(json.dumps(plan_json, indent=2, sort_keys=True) + "\n")

    timeout_seconds = args.timeout_seconds
    if timeout_seconds is None:
        timeout_seconds = max(120.0, args.duration + 90.0)
    if timeout_seconds <= 0:
        raise SystemExit("--timeout-seconds must be positive")

    print(f"H1_TOOL_MODE=SWEEP cases={len(planned)} mode={args.mode}", flush=True)
    print(f"H1_TOOL_SWEEP_ROOT={root}", flush=True)
    print(f"H1_TOOL_SWEEP_TIMEOUT_SECONDS={timeout_seconds}", flush=True)

    results_path = root / "sweep-results.jsonl"
    failures = 0

    for index, session_id, overrides, profile in planned:
        case_dir = root / f"case-{index:03d}"
        log_path = root / f"case-{index:03d}.console.log"
        command = runner_command(args, session_id, overrides, case_dir, args.validate_only)

        print("", flush=True)
        print(
            f"===== H1 SWEEP CASE {index}/{len(planned)} session_id={session_id} =====",
            flush=True,
        )
        print("H1_TOOL_CASE_OVERRIDES=" + json.dumps(overrides, sort_keys=True), flush=True)
        print("H1_TOOL_CASE_COMMAND=" + json.dumps(command), flush=True)

        markers: set[str] = set()
        timed_out = False
        process = subprocess.Popen(
            command,
            cwd=THIS_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        pump = threading.Thread(
            target=pump_process,
            args=(process, log_path, markers),
            name=f"h1-sweep-pump-{index}",
            daemon=True,
        )
        pump.start()

        try:
            returncode = process.wait(timeout=timeout_seconds)
        except subprocess.TimeoutExpired:
            timed_out = True
            process.terminate()
            try:
                returncode = process.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                process.kill()
                returncode = process.wait(timeout=5.0)
        pump.join(timeout=5.0)

        passed = returncode == 0
        if not args.validate_only:
            passed = passed and "session_validation_pass" in markers and "pi_mux_session_pass" in markers

        result = {
            "case": index,
            "session_id": session_id,
            "overrides": overrides,
            "returncode": returncode,
            "timed_out": timed_out,
            "session_validation_pass": "session_validation_pass" in markers,
            "pi_mux_session_pass": "pi_mux_session_pass" in markers,
            "passed": passed,
            "evidence": str(case_dir),
            "console_log": str(log_path),
        }
        with results_path.open("a", encoding="utf-8") as handle:
            handle.write(json.dumps(result, sort_keys=True) + "\n")

        print("H1_TOOL_CASE_RESULT=" + json.dumps(result, sort_keys=True), flush=True)

        if not passed:
            failures += 1
            if not args.continue_on_failure:
                print("H1_TOOL_SWEEP_STOP=FIRST_FAILURE", flush=True)
                break

        if args.inter_case_delay > 0 and index != len(planned):
            time.sleep(args.inter_case_delay)

    print(
        f"H1_TOOL_SWEEP_COMPLETE failures={failures} results={results_path}",
        flush=True,
    )
    return 0 if failures == 0 else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Reusable H1 resident-media control and optimization tool.",
        epilog=FRIENDLY_HELP,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    knobs = subparsers.add_parser(
        "knobs",
        help="list every adjustable H1 field and current profile defaults",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    knobs.add_argument("--profile", choices=profiles.profile_names(), default="P11_COMPAT_VIDEO_ONLY")
    knobs.add_argument("--session-id", type=parse_int)
    knobs.set_defaults(func=print_knobs)

    show = subparsers.add_parser(
        "show",
        help="resolve modifiers and print the exact profile/validation command without running hardware",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(show, include_evidence=False)
    show.set_defaults(func=print_show)

    run = subparsers.add_parser(
        "run",
        help="run one H1 session through the current cumulative runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(run, include_evidence=True)
    run.add_argument("--validate-only", action="store_true", help="validate profile and generated FFmpeg command without listening for PS2")
    run.set_defaults(func=run_once)

    sweep = subparsers.add_parser(
        "sweep",
        help="run a reproducible multi-parameter H1 batch using the same control surface",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(sweep, include_evidence=False)
    sweep.add_argument("--vary", action="append", default=[], type=parse_vary, metavar="NAME=V1,V2,...", help="parameter values to sweep; repeat for multiple fields")
    sweep.add_argument("--mode", choices=("cartesian", "zip"), default="cartesian", help="cartesian=all combinations; zip=paired rows")
    sweep.add_argument("--evidence-root", help="root directory for plan, per-case evidence, logs, and results")
    sweep.add_argument("--timeout-seconds", type=float, help="hard wall-clock timeout per case; default=max(120,duration+90)")
    sweep.add_argument("--inter-case-delay", type=float, default=1.0, help="seconds between successful cases")
    sweep.add_argument("--continue-on-failure", action="store_true", help="continue after a failed/timed-out case; default stops at first failure")
    sweep.add_argument("--validate-only", action="store_true", help="validate every generated case without PS2 hardware sessions")
    sweep.set_defaults(func=run_sweep)

    return parser


def main() -> int:
    args = build_parser().parse_args()
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
