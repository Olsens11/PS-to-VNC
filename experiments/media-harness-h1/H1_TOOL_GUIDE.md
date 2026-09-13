# H1 operator / optimization tool

`h1_tool.py` is the canonical human-facing control surface for the resident H1
media/RFB harness.

It exists so routine H1 testing does not require reconstructing long bespoke
commands from conversation history. It also preserves the current meaning and
usage of H1's adjustable CONFIG/profile fields in the repository itself.

The tool is an **operator wrapper**, not a replacement implementation. It
delegates actual transport, scheduling, capture, RFB bridging/quiescence,
telemetry, result validation, and evidence production to the cumulative H1
runner. Changing the tool does not by itself change the PS2 ELF.

The default cumulative runner supports both:

- the existing RFB-OFF media profiles; and
- the first deliberately unqualified `H1_RFB_ONLY` channel-1 profile.

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
    --profile H1_RFB_ONLY
```

This is the quickest way to answer "what knobs do we have now?" without reading
source or old test commands.

### `show`

Resolves a proposed command without starting hardware. It prints the resulting
profile, explicit overrides, and the underlying validate-only runner command.

```sh
python3 experiments/media-harness-h1/h1_tool.py show \
    --profile H1_RFB_ONLY \
    --duration 30
```

Use this before unfamiliar combinations.

### `run`

Runs one H1 session using the cumulative runner.

Known-good-style combined media example:

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

The first headless RFB-over-mux test is intentionally simpler:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile H1_RFB_ONLY \
    --duration 30
```

That profile starts RFB on logical channel 1 while AUDIO and MPEG remain off.
The current milestone decodes into the authoritative CPU framebuffer only; it
does not yet present RFB through the GS or start controller/input/OSK behavior.
A successful run therefore proves transport/parser/framebuffer/quiesce behavior,
not visible RFB presentation.

For a dry validation with no PS2 connection:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile H1_RFB_ONLY \
    --duration 30 \
    --validate-only
```

### `sweep`

Runs reproducible parameter batches through the same control path.

One-variable media sweep:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile P11_COMPAT_PLUS_PCM \
    --duration 20 \
    --vary audio_thread_priority=2,8,16
```

Cartesian two-variable media sweep (six cases):

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile P11_COMPAT_PLUS_PCM \
    --duration 20 \
    --vary audio_thread_priority=2,8,16 \
    --vary mpeg_queue_capacity=262144,524288
```

RFB credit-policy sweep:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile H1_RFB_ONLY \
    --duration 20 \
    --vary rfb_credit_batch_bytes=4096,8192,16384
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
--set rfb_credit_batch_bytes=4096
```

Integer syntax accepts ordinary decimal and `0x...` values. Signed fields accept
negative decimal values where the profile permits them.

Friendly aliases and raw `--set` assignments are fail-closed. If two command
arguments attempt to assign different values to the same field, the tool exits
instead of silently choosing one.

`--rfb on` changes only `rfb_mode`; it does **not** silently zero AUDIO/MPEG or
invent an RFB credit policy. For the first RFB hardware authority, prefer
`--profile H1_RFB_ONLY`. Deliberately incompatible combinations are rejected by
the cumulative PS2 activation gate rather than normalized by the operator tool.

Sweep dimensions use raw field names:

```sh
--vary NAME=V1,V2,V3
```

This makes the generated plan directly comparable to `h1_profiles.py`, H1 CONFIG
telemetry, and evidence records.

## CONFIG v4 RFB queue and credit controls

The operator selected full RFB transport-policy flexibility rather than hidden
first-milestone constants. CONFIG v4 therefore contains five ordinary H1 fields:

```text
rfb_queue_capacity
    PS2 EE byte capacity allocated to logical RFB channel 1 when RFB is ON.
    RFB-OFF profiles set this to 0. The evidence-based first value is 32768,
    matching the earlier direct-RFB receive-prefetch size. H1 does not impose an
    arbitrary laboratory ceiling; allocation failure is evidence.

rfb_initial_credit_bytes
    Producer credit granted to the Pi at RFB session start. It must not exceed
    rfb_queue_capacity. H1_RFB_ONLY starts at 32768.

rfb_credit_return_enabled
    0 disables consumed-byte credit return; 1 enables it. H1_RFB_ONLY uses 1.
    RFB is not exempt from mux flow control.

rfb_credit_batch_bytes
    Number of consumed RFB bytes accumulated before a CREDIT frame is returned.
    When credit return is enabled this must be nonzero and no larger than the
    RFB queue. H1_RFB_ONLY starts at 8192. This is a testable starting
    hypothesis, not a claimed optimum.

rfb_credit_flush_on_empty
    0/1. When enabled, pending consumed-byte credit is returned immediately when
    the RFB logical queue becomes empty so a small interactive update does not
    strand credit below the batch threshold. H1_RFB_ONLY uses 1.
```

These fields are ordinary raw knobs, so no ELF rebuild is needed merely to test
another valid policy. Example:

```sh
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile H1_RFB_ONLY \
    --duration 30 \
    --set rfb_credit_batch_bytes=4096
```

A batch can vary those values declaratively:

```sh
python3 experiments/media-harness-h1/h1_tool.py sweep \
    --profile H1_RFB_ONLY \
    --duration 20 \
    --vary rfb_credit_batch_bytes=4096,8192,16384
```

Do not interpret example sweep values as recommendations. Hardware evidence
should determine useful policy rather than source-code constants.

## First RFB activation boundary

The ordinary H1 CONFIG validator remains RFB-OFF-only. The cumulative H1 RFB
experiment build adds a narrow activation wrapper that accepts `rfb_mode=1`
**only** when AUDIO and MPEG are off and the explicit RFB queue/credit
relationships are valid. Every unrelated field is still delegated to the
ordinary validator.

The cumulative PS2 build alone advertises `CAP_RFB`; ordinary H1 builds do not.
The Pi RFB-only runner verifies that capability before it sends CONFIG, so using
the wrong resident ELF fails before an RFB session starts.

RFB remains logical channel 1 on the **same physical PSTV connection**. The
operator tool must never grow a second competing PS2-facing TCP stream as a
convenience.

## Clean finite RFB termination

RFB sessions cannot be ended by arbitrarily cutting the raw VNC byte stream at a
wall-clock deadline. The cumulative runner uses a four-phase channel-1 quiesce
handshake:

```text
Pi REQUEST
PS2 stops at a complete RFB server-message boundary
PS2 BOUNDARY
Pi stops and joins the upstream VNC reader
Pi COMMIT
PS2 proves the channel-1 queue is empty
PS2 COMPLETE
Pi sends normal H1 MEDIA_END
```

The markers are zero-length PSTV channel-1 DATA frames and never enter the RFB
parser. Non-empty channel-1 DATA remains raw RFB bytes. The Pi bridge therefore
does not need to understand RFB message framing; the unchanged through-Issue-39
PS2 parser remains boundary authority.

The PS2 can quiesce at an ordinary completed-update boundary or at parser IDLE
before the next server message is consumed. The latter matters for a completely
static desktop, where an outstanding incremental request may have no response.
The post-COMMIT queue-empty check still fails closed if bytes from a new server
message raced through before the Pi bridge stopped.

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

`H1_RFB_ONLY` and the current RFB-capable cumulative ELF remain **unqualified**
until an exact ELF/PT_LOAD is run and observed on real PS2 hardware. CI and host
tests establish readiness; they do not transfer physical qualification.

---

## Current CP2P all-guns qualification note — 2026-09-13

A 600-second MPEG+PCM+RFB active-runtime hardware run passed with the
receiver-driven MPEG event-wake candidate. RFB quiesce also completed.

Successive H1 sessions are **not yet qualified**: after ordinary MEDIA_END the
PS2 did not return SESSION_RESULT. Until that lifecycle defect is fixed and
hardware-tested, do not assume a completed workload leaves the resident H1 ELF
ready for the next automated session.

See `CP2P_MPEG_EVENT_WAKE_HARDWARE_RESULT.md` for exact authority and remaining
pre-ledge qualification.
