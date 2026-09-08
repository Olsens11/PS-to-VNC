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
        --audio-priority 8 --media-rect 16,112,432,240 --duration 60
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

# Operator-facing explanations. Unknown future profile fields remain
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
    "audio_start_delay_us": "Additional audio start delay in microseconds.",
    "audio_chunk_bytes": "Bytes submitted to AUDSRV per audio consumer chunk.",
    "audio_idle_delay_us": "Audio consumer delay in microseconds while waiting for data.",
    "audio_thread_priority": "EE priority of H1 audio worker; lower number means higher EE priority.",
    "audio_thread_stack_size": "EE stack size of H1 audio worker in bytes.",
    "audio_rate": "PCM sample rate in Hz.",
    "audio_channels": "PCM channel count.",
    "audio_bits": "PCM bits per sample.",
    "audio_volume": "AUDSRV playback volume value used by H1.",
    "audio_presentation_offset_us": "Signed audio offset from shared media epoch in microseconds.",
    "mpeg_start_target_bytes": "MPEG queue prefill target before video decode begins.",
    "mpeg_prefill_wait_us": "Delay in microseconds between MPEG prefill checks.",
    "mpeg_prefill_max_loops": "Maximum MPEG prefill wait-loop count.",
    "mpeg_empty_delay_us": "Decoder/feed delay in microseconds while MPEG queue is temporarily empty.",
    "mpeg_feed_bytes": "Bytes requested per MPEG/IPU feed operation.",
    "video_fps_num": "Video frame-rate numerator.",
    "video_fps_den": "Video frame-rate denominator.",
    "video_scheduler_mode": "Video presentation scheduler selector; qualified mode is absolute scheduling.",
    "video_presentation_offset_us": "Signed video offset from shared media epoch in microseconds.",
    "video_pixel_mode": "Decoded/presentation pixel-mode selector; qualified path is RGB16.",
    "video_max_width": "Maximum decoded sequence width accepted by H1.",
    "video_max_height": "Maximum decoded sequence height accepted by H1.",
    "video_draw_width": "GS destination width for MPEG presentation in pixels.",
    "video_draw_height": "GS destination height for MPEG presentation in pixels.",
    "video_draw_x": "GS destination X coordinate for MPEG presentation in desktop pixels.",
    "video_draw_y": "GS destination Y coordinate for MPEG presentation in desktop pixels.",
    "video_stage_markers": "0=disable, 1=enable historical H1 stage-color markers.",
    "video_stage_hold_vsyncs": "Number of vsyncs to hold each enabled stage marker.",
    "video_ipu_reset_each_session": "0/1: reset IPU state at each video session start.",
    "video_drop_enabled": "0/1: experimental late-frame dropping; qualified media path keeps this off.",
    "video_drop_threshold_milliframes": "Late threshold for experimental frame dropping, in milliframes.",
    "receiver_thread_priority": "EE priority of the sole H1 PSTV receiver thread; lower number is higher priority.",
    "receiver_thread_stack_size": "EE stack size of H1 receiver thread in bytes.",
    "max_data_payload": "Maximum PSTV DATA payload in bytes; current wire authority is 8192.",
    "socket_receive_buffer_bytes": "Optional TCP receive-buffer request in bytes; 0 leaves platform/default behavior.",
    "socket_send_buffer_bytes": "Optional TCP send-buffer request in bytes; 0 leaves platform/default behavior.",
    "queue_allocation_order": "0=allocate AUDIO first, 1=allocate MPEG first.",
    "media_epoch_lead_us": "Lead time in microseconds before the shared media presentation epoch begins.",
    "rfb_mode": "0=off; 1=reserved RFB-on value (current H1 validator still rejects it).",
    "video_encode_width": "MPEG encoder output width in pixels; current H1 path requires 16-pixel alignment.",
    "video_encode_height": "MPEG encoder output height in pixels; current H1 path requires 16-pixel alignment.",
    "desktop_width": "Pi-owned active desktop width in pixels used for capture geometry validation.",
    "desktop_height": "Pi-owned active desktop height in pixels used for capture geometry validation.",
    "video_capture_x": "Pi X11 source-region X coordinate in active desktop pixels.",
    "video_capture_y": "Pi X11 source-region Y coordinate in active desktop pixels.",
    "video_capture_width": "Pi X11 source-region width in pixels before MPEG scaling/encoding.",
    "video_capture_height": "Pi X11 source-region height in pixels before MPEG scaling/encoding.",
}

FRIENDLY_HELP = """Frequently used convenience switches map onto normal profile fields:
  --audio off|pcm
      Select audio_mode. `pcm` uses the configured H1 PCM fields; it does not
      silently replace their values with a second preset.

  --video off|mpeg2
      Select video_mode.

  --rfb off|on
      Select rfb_mode. `on` is intentionally exposed for continuity, but the
      current H1 PS2 validator still rejects RFB=ON because channel-1 RFB has
      not yet been activated in this harness.

  --audio-priority N
      Set audio_thread_priority. PS2 EE priorities are inverse-numbered:
      smaller numeric values are higher scheduler priority.

  --receiver-priority N
      Set receiver_thread_priority for H1's sole physical PSTV receiver.

  --desktop WxH
      Set the Pi-owned active desktop dimensions, e.g. --desktop 704x462.
      These values validate capture bounds; they are not a permanent product
      maximum.

  --capture X,Y,W,H
      Select the exact X11 source rectangle inside the active desktop.
      Coordinates are desktop pixels. Current cumulative capture grabs the
      desktop then crops in FFmpeg; this option describes the semantic source
      region regardless of that current implementation detail.

  --encode WxH
      Set MPEG encoded dimensions. Current H1 requires both dimensions to be
      multiples of 16. This is codec geometry, not a rule that future media
      objects themselves must have 16-pixel-aligned bounds.

  --draw X,Y,W,H
      Set the PS2 MPEG destination rectangle.

  --media-rect X,Y,W,H
      Convenience for today's same-location media experiments. It sets
      --capture X,Y,W,H, --encode WxH, and --draw X,Y,W,H together. Therefore
      W/H must satisfy the current 16-pixel encode constraint. Use separate
      --capture/--encode/--draw when source, coding, and presentation geometry
      intentionally differ.

  --markers off|on
      Toggle the historical H1 stage-color markers only. GS setup is no longer
      dependent on markers being enabled.

Every H1 profile field remains reachable with repeated:

  --set NAME=INTEGER

INTEGER accepts normal decimal or Python-style base prefixes such as 0x2000.
Signed fields such as *_presentation_offset_us accept negative decimal values.
Friendly switches and --set are fail-closed: if two arguments try to assign
conflicting values to the same field, the tool exits rather than guessing.

For the complete field dictionary plus the selected profile's current defaults:

  h1_tool.py knobs --profile P11_COMPAT_PLUS_PCM
"""

RUN_HELP = """RUN executes exactly one session by translating these arguments
into the existing cumulative H1 runner. It does not rebuild, patch, or upload an
ELF. The PS2 must already be running a compatible resident H1 ELF and connect to
the selected --listen/--port.

Use --validate-only first when trying unfamiliar geometry or raw fields. It
runs profile validation and prints the generated FFmpeg command without waiting
for PS2 hardware.

Examples:

  # Known-good combined media baseline with audio priority 8
  h1_tool.py run --profile P11_COMPAT_PLUS_PCM --audio-priority 8 --duration 60

  # Same-location YouTube rectangle proved during the 2026-09-08 experiment
  h1_tool.py run --profile P11_COMPAT_PLUS_PCM --audio-priority 8 \
      --media-rect 16,112,432,240 --duration 60

  # Change a raw field that has no friendly alias
  h1_tool.py run --profile P11_COMPAT_PLUS_PCM \
      --set mpeg_feed_bytes=2048 --set mpeg_queue_capacity=524288
"""

SWEEP_HELP = """SWEEP expands one or more --vary fields into repeatable cases
and runs each case through the same underlying runner used by `run`.

Syntax:

  --vary NAME=V1,V2,V3

Use the canonical raw profile field name. Values are integers and accept decimal
or 0x... notation. A field may not simultaneously appear in --set/friendly base
overrides and --vary; that ambiguity fails closed.

Modes:

  --mode cartesian   every combination of every --vary list (default)
  --mode zip         pair first values together, second values together, etc.;
                     all lists must have equal length

Examples:

  # Three one-variable cases
  h1_tool.py sweep --profile P11_COMPAT_PLUS_PCM --duration 20 \
      --vary audio_thread_priority=2,8,16

  # Six cases: 3 priorities x 2 MPEG queue capacities
  h1_tool.py sweep --profile P11_COMPAT_PLUS_PCM --duration 20 \
      --vary audio_thread_priority=2,8,16 \
      --vary mpeg_queue_capacity=262144,524288

  # Two paired cases instead of a Cartesian product
  h1_tool.py sweep --profile P11_COMPAT_PLUS_PCM --mode zip \
      --vary audio_thread_priority=8,16 \
      --vary receiver_thread_priority=63,63

Before touching hardware, all expanded profile shapes are validated. The sweep
writes sweep-plan.json before case 1, one console log and evidence directory per
case, and sweep-results.jsonl as cases complete. By default it stops on the
first failed/timed-out case; --continue-on-failure changes that policy.
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
        raise argparse.ArgumentTypeError(
            "rectangle requires nonnegative X/Y and positive width/height"
        )
    return x, y, width, height


def parse_vary(text: str) -> tuple[str, tuple[int, ...]]:
    if "=" not in text:
        raise argparse.ArgumentTypeError("expected NAME=VALUE[,VALUE...]")
    name, raw_values = text.split("=", 1)
    name = name.strip()
    raw_items = [item.strip() for item in raw_values.split(",") if item.strip()]
    if not name or not raw_items:
        raise argparse.ArgumentTypeError(
            "sweep variation needs a name and at least one value"
        )
    try:
        values = tuple(int(item, 0) for item in raw_items)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    return name, values


def new_session_id(salt: int = 0) -> int:
    value = (time.time_ns() ^ (os.getpid() << 16) ^ salt) & 0xFFFFFFFF
    return value if value != 0 else 1


def set_override(
    target: dict[str, int],
    name: str,
    value: int,
    source: str,
) -> None:
    if name in target and target[name] != value:
        raise SystemExit(
            f"conflicting value for {name}: "
            f"existing={target[name]} new={value} source={source}"
        )
    target[name] = value


def overrides_from_args(args: argparse.Namespace) -> dict[str, int]:
    result: dict[str, int] = {}

    for name, value in getattr(args, "set_values", []) or []:
        set_override(result, name, value, "--set")

    audio = getattr(args, "audio", None)
    if audio is not None:
        set_override(
            result,
            "audio_mode",
            0 if audio == "off" else 1,
            "--audio",
        )

    video = getattr(args, "video", None)
    if video is not None:
        set_override(
            result,
            "video_mode",
            0 if video == "off" else 1,
            "--video",
        )

    rfb = getattr(args, "rfb", None)
    if rfb is not None:
        set_override(
            result,
            "rfb_mode",
            0 if rfb == "off" else 1,
            "--rfb",
        )

    audio_priority = getattr(args, "audio_priority", None)
    if audio_priority is not None:
        set_override(
            result,
            "audio_thread_priority",
            audio_priority,
            "--audio-priority",
        )

    receiver_priority = getattr(args, "receiver_priority", None)
    if receiver_priority is not None:
        set_override(
            result,
            "receiver_thread_priority",
            receiver_priority,
            "--receiver-priority",
        )

    desktop = getattr(args, "desktop", None)
    if desktop is not None:
        set_override(result, "desktop_width", desktop[0], "--desktop")
        set_override(result, "desktop_height", desktop[1], "--desktop")

    media_rect = getattr(args, "media_rect", None)
    if media_rect is not None:
        x, y, width, height = media_rect
        for name, value in (
            ("video_capture_x", x),
            ("video_capture_y", y),
            ("video_capture_width", width),
            ("video_capture_height", height),
            ("video_encode_width", width),
            ("video_encode_height", height),
            ("video_draw_x", x),
            ("video_draw_y", y),
            ("video_draw_width", width),
            ("video_draw_height", height),
        ):
            set_override(result, name, value, "--media-rect")

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
        set_override(
            result,
            "video_stage_markers",
            0 if markers == "off" else 1,
            "--markers",
        )

    return result


def profile_field_order(name: str) -> tuple[int, str]:
    if name == "profile_id":
        return (-1, name)
    field_id = profiles.FIELD_IDS.get(name)
    if field_id is not None:
        return (field_id, name)
    return (10000, name)


def raw_field_help() -> str:
    lines = [
        "RAW --set / --vary FIELD DICTIONARY",
        "----------------------------------",
        "These names come directly from h1_profiles.py. WIRE fields are sent in",
        "CONFIG; PI_ONLY fields affect capture/test harness behavior only.",
        "",
    ]

    names = set(profiles.FIELD_IDS) | set(profiles.PI_ONLY_FIELDS) | {"profile_id"}
    for name in sorted(names, key=profile_field_order):
        scope = "PI_ONLY" if name in profiles.PI_ONLY_FIELDS else "WIRE"
        description = KNOB_HELP.get(
            name,
            "H1 profile field; see h1_profiles.py validator for current semantics.",
        )
        lines.append(f"  {name} [{scope}]")
        lines.append(f"      {description}")
    return "\n".join(lines)


def full_help(extra: str = "") -> str:
    sections = [FRIENDLY_HELP.rstrip()]
    if extra:
        sections.append(extra.rstrip())
    sections.append(raw_field_help())
    return "\n\n".join(sections) + "\n"


def resolved_profile(
    args: argparse.Namespace,
    overrides: Mapping[str, int],
    session_id: int,
) -> dict[str, int]:
    return profiles.resolve_profile(args.profile, session_id, overrides)


def add_profile_arguments(
    parser: argparse.ArgumentParser,
    include_evidence: bool = True,
) -> None:
    parser.add_argument(
        "--profile",
        choices=profiles.profile_names(),
        default="P11_COMPAT_VIDEO_ONLY",
        help=(
            "base profile whose defaults are modified by friendly switches/--set; "
            "use `knobs --profile NAME` to inspect defaults"
        ),
    )
    parser.add_argument(
        "--session-id",
        type=parse_int,
        help=(
            "explicit uint32 session identity; decimal or 0x...; normally omit "
            "and let the tool generate a unique value"
        ),
    )
    parser.add_argument(
        "--duration",
        type=float,
        default=24.0,
        help="finite capture/session duration in seconds; must be >0 (default: 24)",
    )
    parser.add_argument(
        "--display",
        default=DEFAULT_DISPLAY,
        help=(
            "Pi X11 display captured by FFmpeg, e.g. :0.0 or :1; default is "
            "$DISPLAY when set, otherwise :0.0"
        ),
    )
    parser.add_argument(
        "--listen",
        default="0.0.0.0",
        help="PS2-facing bind address for the existing PSTV mux runner (default: 0.0.0.0)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=5902,
        help="PS2-facing PSTV TCP port (default: 5902)",
    )
    if include_evidence:
        parser.add_argument(
            "--evidence",
            help=(
                "explicit evidence directory for this run; must not already exist; "
                "omit to use the underlying runner's timestamped default"
            ),
        )
    parser.add_argument(
        "--runner",
        type=Path,
        default=DEFAULT_RUNNER,
        help=(
            "underlying H1 Python runner; normally leave unchanged so transport, "
            "scheduler, census and validation stay on current authority"
        ),
    )

    parser.add_argument(
        "--set",
        dest="set_values",
        action="append",
        default=[],
        type=parse_assignment,
        metavar="NAME=INTEGER",
        help=(
            "override any H1 field; repeatable; INTEGER accepts decimal/0x... and "
            "signed decimal where the profile permits it"
        ),
    )
    parser.add_argument(
        "--audio",
        choices=("off", "pcm"),
        help="friendly audio_mode switch: off=0, pcm=1",
    )
    parser.add_argument(
        "--video",
        choices=("off", "mpeg2"),
        help="friendly video_mode switch: off=0, mpeg2=1",
    )
    parser.add_argument(
        "--rfb",
        choices=("off", "on"),
        help=(
            "friendly rfb_mode switch; on maps to reserved value 1 and is still "
            "rejected by the current H1 PS2 validator"
        ),
    )
    parser.add_argument(
        "--audio-priority",
        type=parse_int,
        help=(
            "audio_thread_priority; lower numeric EE priority means higher scheduler "
            "priority (known-good combined-media experiments use 8)"
        ),
    )
    parser.add_argument(
        "--receiver-priority",
        type=parse_int,
        help="receiver_thread_priority for the sole PSTV receiver (qualified value: 63)",
    )
    parser.add_argument(
        "--desktop",
        type=parse_size,
        metavar="WxH",
        help="Pi active desktop dimensions in pixels, e.g. 704x462",
    )
    parser.add_argument(
        "--capture",
        type=parse_rect,
        metavar="X,Y,W,H",
        help="exact Pi/X11 source rectangle in active-desktop pixels",
    )
    parser.add_argument(
        "--encode",
        type=parse_size,
        metavar="WxH",
        help="MPEG encoded size; current H1 requires width and height divisible by 16",
    )
    parser.add_argument(
        "--draw",
        type=parse_rect,
        metavar="X,Y,W,H",
        help="PS2 MPEG destination rectangle in presentation/desktop pixels",
    )
    parser.add_argument(
        "--media-rect",
        type=parse_rect,
        metavar="X,Y,W,H",
        help=(
            "set capture, encode, and draw to the same X/Y/W/H in one modifier; "
            "current W/H must be 16-pixel aligned because they also become encode size"
        ),
    )
    parser.add_argument(
        "--markers",
        choices=("off", "on"),
        help="toggle historical video stage-color markers; independent of GS setup",
    )


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
        description = KNOB_HELP.get(
            name,
            "H1 profile field; see h1_profiles.py validator for current semantics.",
        )
        print(f"{name:36} {str(base_profile[name]):>12} {scope:8} {description}")

    print("H1_TOOL_KNOBS_END")
    print()
    print(FRIENDLY_HELP.rstrip())
    return 0


def print_show(args: argparse.Namespace) -> int:
    if args.duration <= 0:
        raise SystemExit("--duration must be positive")
    overrides = overrides_from_args(args)
    session_id = (
        args.session_id if args.session_id is not None else new_session_id()
    )
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
    session_id = (
        args.session_id if args.session_id is not None else new_session_id()
    )
    profile = resolved_profile(args, overrides, session_id)
    evidence = Path(args.evidence).expanduser() if args.evidence else None
    command = runner_command(
        args,
        session_id,
        overrides,
        evidence,
        args.validate_only,
    )

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
        raise SystemExit(
            "sweep requires at least one --vary NAME=VALUE[,VALUE...]"
        )

    names = [name for name, _ in variation_list]
    if len(set(names)) != len(names):
        raise SystemExit("each --vary field may appear only once")

    for name in names:
        if name in base_overrides:
            raise SystemExit(
                f"field {name} appears in both base overrides and --vary"
            )

    value_sets = [values for _, values in variation_list]
    if mode == "zip":
        lengths = {len(values) for values in value_sets}
        if len(lengths) != 1:
            raise SystemExit(
                "--mode zip requires every --vary list to have the same length"
            )
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


def pump_process(
    process: subprocess.Popen[str],
    log_path: Path,
    markers: set[str],
) -> None:
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
    if args.inter_case_delay < 0:
        raise SystemExit("--inter-case-delay may not be negative")

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
        "variations": [
            {"field": name, "values": list(values)}
            for name, values in args.vary
        ],
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
    (root / "sweep-plan.json").write_text(
        json.dumps(plan_json, indent=2, sort_keys=True) + "\n"
    )

    timeout_seconds = args.timeout_seconds
    if timeout_seconds is None:
        timeout_seconds = max(120.0, args.duration + 90.0)
    if timeout_seconds <= 0:
        raise SystemExit("--timeout-seconds must be positive")

    print(
        f"H1_TOOL_MODE=SWEEP cases={len(planned)} mode={args.mode}",
        flush=True,
    )
    print(f"H1_TOOL_SWEEP_ROOT={root}", flush=True)
    print(f"H1_TOOL_SWEEP_TIMEOUT_SECONDS={timeout_seconds}", flush=True)

    results_path = root / "sweep-results.jsonl"
    failures = 0

    for index, session_id, overrides, profile in planned:
        case_dir = root / f"case-{index:03d}"
        log_path = root / f"case-{index:03d}.console.log"
        command = runner_command(
            args,
            session_id,
            overrides,
            case_dir,
            args.validate_only,
        )

        print("", flush=True)
        print(
            f"===== H1 SWEEP CASE {index}/{len(planned)} "
            f"session_id={session_id} =====",
            flush=True,
        )
        print(
            "H1_TOOL_CASE_OVERRIDES="
            + json.dumps(overrides, sort_keys=True),
            flush=True,
        )
        print(
            "H1_TOOL_CASE_COMMAND=" + json.dumps(command),
            flush=True,
        )

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
            passed = (
                passed
                and "session_validation_pass" in markers
                and "pi_mux_session_pass" in markers
            )

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

        print(
            "H1_TOOL_CASE_RESULT=" + json.dumps(result, sort_keys=True),
            flush=True,
        )

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
        description=(
            "Canonical reusable control surface for H1 CONFIG/profile knobs, "
            "single hardware sessions, and parameter sweeps."
        ),
        epilog=full_help(),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    knobs = subparsers.add_parser(
        "knobs",
        help="list every adjustable H1 field and current profile defaults",
        description=(
            "Print the complete H1 field dictionary with the selected profile's "
            "current default value and whether each field is WIRE or PI_ONLY."
        ),
        epilog=full_help(),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    knobs.add_argument(
        "--profile",
        choices=profiles.profile_names(),
        default="P11_COMPAT_VIDEO_ONLY",
        help="profile whose current defaults should be shown",
    )
    knobs.add_argument(
        "--session-id",
        type=parse_int,
        help="optional value used only when displaying the session_id default row",
    )
    knobs.set_defaults(func=print_knobs)

    show = subparsers.add_parser(
        "show",
        help="resolve modifiers and print the exact profile/validation command without running hardware",
        description=(
            "Resolve all friendly/raw modifiers and show the exact profile plus "
            "underlying validate-only command. No listener is opened and no PS2 "
            "connection is required."
        ),
        epilog=full_help(
            "SHOW is the safest first step when constructing a new test. It proves "
            "which fields a command will change without starting a session."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(show, include_evidence=False)
    show.set_defaults(func=print_show)

    run = subparsers.add_parser(
        "run",
        help="run one H1 session through the current cumulative runner",
        description=(
            "Run one resident-H1 session using the current cumulative/census Pi "
            "runner. This is the normal interactive hardware-test command."
        ),
        epilog=full_help(RUN_HELP),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(run, include_evidence=True)
    run.add_argument(
        "--validate-only",
        action="store_true",
        help=(
            "validate profile and generated FFmpeg command only; do not listen "
            "for/connect to PS2 hardware"
        ),
    )
    run.set_defaults(func=run_once)

    sweep = subparsers.add_parser(
        "sweep",
        help="run a reproducible multi-parameter H1 batch using the same control surface",
        description=(
            "Expand raw H1 field value lists into a recorded hardware-test batch. "
            "The same runner/control path is used for every case."
        ),
        epilog=full_help(SWEEP_HELP),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    add_profile_arguments(sweep, include_evidence=False)
    sweep.add_argument(
        "--vary",
        action="append",
        default=[],
        type=parse_vary,
        metavar="NAME=V1,V2,...",
        help=(
            "raw profile field plus comma-separated integer values to test; "
            "repeat for multiple independent fields"
        ),
    )
    sweep.add_argument(
        "--mode",
        choices=("cartesian", "zip"),
        default="cartesian",
        help=(
            "cartesian=all combinations (default); zip=pair values by list "
            "position and require equal list lengths"
        ),
    )
    sweep.add_argument(
        "--evidence-root",
        help=(
            "new root directory for sweep-plan.json, per-case evidence/logs, and "
            "sweep-results.jsonl; omit for timestamped ~/ps2vnc-evidence path"
        ),
    )
    sweep.add_argument(
        "--timeout-seconds",
        type=float,
        help=(
            "hard wall-clock timeout per case; default is max(120, duration+90); "
            "a timeout counts as failure"
        ),
    )
    sweep.add_argument(
        "--inter-case-delay",
        type=float,
        default=1.0,
        help="nonnegative pause in seconds between completed cases (default: 1)",
    )
    sweep.add_argument(
        "--continue-on-failure",
        action="store_true",
        help=(
            "continue to later cases after failure/timeout; default stops at "
            "first failure to preserve hardware evidence"
        ),
    )
    sweep.add_argument(
        "--validate-only",
        action="store_true",
        help=(
            "expand and validate every generated case without PS2 hardware sessions"
        ),
    )
    sweep.set_defaults(func=run_sweep)

    return parser


def main() -> int:
    args = build_parser().parse_args()
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
