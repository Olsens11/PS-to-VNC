# H1 operator / optimization tool

`h1_tool.py` is the canonical human-facing control surface for the resident H1
media harness.

It exists so routine H1 testing does not require reconstructing long bespoke
commands from conversation history. It also preserves the current meaning and
usage of H1's adjustable CONFIG/profile fields in the repository itself.

The tool is an **operator wrapper**, not a replacement implementation. It
delegates actual transport, scheduling, capture, telemetry, result validation,
and evidence production to the current cumulative H1 runner. Changing the tool
does not by itself change the PS2 ELF.

## Start with help

From the repository root:

```sh
python3 experiments/media-harness-h1/h1_tool.py --help
```

Each subcommand also has detailed help:

```sh
python3 experiments/media-harness-h1/h1_tool.py knobs --help
python3 experiments/media-harness-h1/h1_tool.py show --help
python3 experiments/media-harness-h1/h1_tool.py run --help
python3 experiments/media-harness-h1/h1_tool.py sweep --help
```

The help text includes the friendly modifiers, exact accepted syntax and units,
sweep behavior, examples, and the complete raw H1 field dictionary.

## Commands

### `knobs`

Lists every currently adjustable profile field, its current value for a named
base profile, whether it is sent on the H1 wire or is Pi-only metadata, and a
plain-language description where one has been added to the operator dictionary.
New raw fields are discovered directly from `h1_profiles.py`, so they remain
reachable even before a convenience alias is added.

```sh
python3 experiments/media-harness-h1/h1_tool.py knobs \
    --profile P11_COMPAT_PLUS_PCM
```

This is the quickest way to answer "what knobs do we have now?" without reading
source or old test commands.

### `show`

Resolves a proposed command without starting hardware. It prints the resulting
profile, explicit overrides, and the underlying validate-only runner command.

```sh
python3 experiments/media-harness-h1/h1_tool.py show \
    --profile P11_COMPAT_PLUS_PCM \
    --audio-priority 8 \
    --media-rect 16,112,432,240
```

Use this before unfamiliar combinations.

### `run`

Runs one H1 session using the current cumulative/census runner.

Known-good-style combined example:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile P11_COMPAT_PLUS_PCM \
    --audio-priority 8 \
    --duration 60
```

The manually proven same-location YouTube patch from 2026-09-08 can be expressed
compactly as:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile P11_COMPAT_PLUS_PCM \
    --audio-priority 8 \
    --media-rect 16,112,432,240 \
    --duration 60
```

`--media-rect X,Y,W,H` is only a convenience for today's aligned experiments.
It sets source capture, MPEG encode dimensions, and destination draw geometry to
the same rectangle. When those geometries should differ, use the separate
`--capture`, `--encode`, and `--draw` modifiers.

Current semantic-media design notes deliberately do **not** make 16-pixel
alignment a future media-object requirement. Current H1 still requires aligned
MPEG encode dimensions because that is a codec-path constraint.

For a dry validation with no PS2 connection:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile P11_COMPAT_PLUS_PCM \
    --media-rect 16,112,432,240 \
    --validate-only
```

### `sweep`

Runs reproducible parameter batches through the same control path.

One-variable sweep:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile P11_COMPAT_PLUS_PCM \
    --duration 20 \
    --vary audio_thread_priority=2,8,16
```

Cartesian two-variable sweep (six cases):

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile P11_COMPAT_PLUS_PCM \
    --duration 20 \
    --vary audio_thread_priority=2,8,16 \
    --vary mpeg_queue_capacity=262144,524288
```

Paired/zip sweep:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile P11_COMPAT_PLUS_PCM \
    --mode zip \
    --vary audio_thread_priority=8,16 \
    --vary receiver_thread_priority=63,63
```

The sweep writes a complete `sweep-plan.json` before case 1, then records one
console log and evidence directory per case plus an append-only
`sweep-results.jsonl` summary. The default policy stops at the first failed or
timed-out case. `--continue-on-failure` deliberately changes that policy.

`--validate-only` expands and validates the full batch without running hardware.

## Friendly modifiers versus raw fields

Friendly modifiers cover common operations:

```text
--audio off|pcm
--video off|mpeg2
--rfb off|on
--audio-priority N
--receiver-priority N
--desktop WxH
--capture X,Y,W,H
--encode WxH
--draw X,Y,W,H
--media-rect X,Y,W,H
--markers off|on
```

Every profile field remains accessible through:

```sh
--set NAME=INTEGER
```

For example:

```sh
--set mpeg_feed_bytes=2048
--set mpeg_queue_capacity=524288
--set audio_presentation_offset_us=-43000
```

Integer syntax accepts ordinary decimal and `0x...` values. Signed fields accept
negative decimal values where the profile permits them.

Friendly aliases and raw `--set` assignments are fail-closed. If two command
arguments attempt to assign different values to the same field, the tool exits
instead of silently choosing one.

Sweep dimensions use raw field names:

```sh
--vary NAME=V1,V2,V3
```

This makes the generated plan directly comparable to `h1_profiles.py`, H1 CONFIG
telemetry, and evidence records.

## CONFIG v4 RFB queue and credit controls

The operator selected full RFB transport-policy flexibility rather than hidden
first-milestone constants. CONFIG v4 therefore adds five ordinary H1 fields:

```text
rfb_queue_capacity
    PS2 EE byte capacity allocated to logical RFB channel 1 when RFB is ON.
    RFB-OFF profiles set this to 0. The evidence-based first value is 32768,
    matching the earlier qualified direct-RFB receive-prefetch size. H1 does
    not impose an arbitrary laboratory ceiling; allocation failure is evidence.

rfb_initial_credit_bytes
    Producer credit granted to the Pi at RFB session start. It must not exceed
    rfb_queue_capacity. First RFB-only value: 32768.

rfb_credit_return_enabled
    0 disables consumed-byte credit return; 1 enables it. First RFB-only value:
    1. RFB is not exempt from mux flow control.

rfb_credit_batch_bytes
    Number of consumed RFB bytes accumulated before a CREDIT frame is returned.
    When credit return is enabled this must be nonzero and no larger than the
    RFB queue. First value: 8192, matching current max_data_payload and MPEG's
    existing batch. This is a testable starting hypothesis, not a claimed
    optimum.

rfb_credit_flush_on_empty
    0/1. When enabled, pending consumed-byte credit is returned immediately when
    the RFB logical queue becomes empty so a small interactive update does not
    strand credit below the batch threshold. First value: 1.
```

These fields are intentionally ordinary raw knobs, so no ELF rebuild is needed
just to sweep queue/credit policy once RFB activation is live. Example future
RFB-only control surface:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --rfb on --audio off --video off \
    --set rfb_queue_capacity=32768 \
    --set rfb_initial_credit_bytes=32768 \
    --set rfb_credit_return_enabled=1 \
    --set rfb_credit_batch_bytes=8192 \
    --set rfb_credit_flush_on_empty=1
```

A future batch can vary those same values declaratively, for example:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --rfb on --audio off --video off \
    --vary rfb_queue_capacity=16384,32768,65536 \
    --vary rfb_credit_batch_bytes=4096,8192
```

Do not interpret those example sweep values as recommendations. They illustrate
why the controls exist: hardware evidence should determine useful policy rather
than source-code constants.

## Current RFB activation status

The operator interface exposes `--rfb off|on`, the raw `rfb_mode` field, and now
all five CONFIG v4 RFB queue/credit fields so the test surface is ready before
activation.

At the current CONFIG-v4 vocabulary checkpoint, `rfb_mode=1` is still gated by
the authoritative PS2 validator. This is deliberate: vocabulary/resource
plumbing is being build-verified before live channel-1 DATA/CREDIT, mux I/O, and
the Pi VNC bridge are enabled. The tool must not bypass that validator.

When RFB activation occurs, RFB remains logical channel 1 on the **same physical
PSTV connection**. This tool must never grow a second competing PS2-facing TCP
stream as a convenience.

## Adding future knobs

The source of truth for profile fields remains `h1_profiles.py`.

`h1_tool.py knobs` discovers the fields from that module. When a new field is
added, it therefore appears in the operator field list even before a friendly
alias exists. Add a plain-language entry to `KNOB_HELP` at the same time where
practical so `--help` preserves its semantics.

Only add a friendly CLI alias when it materially improves routine use. The raw
`--set`/`--vary` path intentionally remains complete so the command surface does
not need one bespoke argument for every experiment.

## Evidence / reproducibility intent

The tool prints resolved profiles and underlying commands before execution.
Batch runs record the entire expanded test plan before touching hardware.

The point is not merely convenience. It makes routine H1 experiments portable
between sessions and conversations while keeping the tested values explicit,
reviewable, and reproducible.
