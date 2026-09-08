# H1 Resident Media Harness

H1 is the experiment-only resident audio/video harness descended from two
qualified parents:

- EXP3/P11 MPEG-2 video playback; and
- configurable Audio Transport EXP2 PCM playback/mux transport.

The parent sources remain unchanged. H1-specific descendants live here.

## Transport architecture lock

H1 does **not** return to competing PS2-facing TCP sockets.

One physical PSTV TCP connection carries independent logical channels:

- CONTROL
- TELEMETRY
- AUDIO
- MPEG2

RFB is intentionally absent from the first media-harness profile so the first
combined workload isolates audio + MPEG video. RFB can be reintroduced later
on the same physical mux after the media path is qualified.

This preserves the transport-mux lesson already learned by the audio work. A
future A/B comparison may change audio *encoding* or place compressed audio in
an MPEG program stream, but it will not use separate PS2-facing TCP sockets as
the reference architecture.

## Configuration philosophy

Every connection carries one complete CONFIG v2 profile from the Pi. The PS2
ACKs the exact accepted bytes only after structurally validating the profile
and successfully allocating requested runtime resources.

H1 deliberately does not clamp experimental queue sizes, prefill depths,
delays, feed sizes or presentation offsets to known-good values. Invalid means
contradictory/unrepresentable for the current mechanism, not merely unqualified.
A freeze, crash, underrun, missed deadline, allocation failure or decoder error
at an aggressive profile is legitimate experimental evidence.

## Latency and synchronization

Buffering and synchronization are separate variables.

Buffer/resilience controls include independent audio and MPEG queue capacities,
initial credits, credit batching, startup targets and consumer/feed sizes.

Presentation controls include independent signed audio/video presentation
offsets around one shared media epoch. H1 therefore does not use queue prefill
as a surrogate lip-sync knob.

## First compatibility profiles

The first video-only profile reproduces P11's effective settings:

- MPEG queue 524288 bytes
- startup target 458752 bytes
- 2048-byte IPU feed
- 30000/1001 absolute scheduler
- RGB16 / PSMCT16 presentation
- no frame dropping
- 704x480 maximum decoded geometry
- 640x512 draw geometry

The first combined profile adds EXP2-compatible 48000 Hz / 16-bit / stereo PCM
on the AUDIO logical channel while retaining those P11 video settings.

The first hardware sequence is:

1. old qualified P11 control;
2. H1 video-only P11-compatible session;
3. repeated identical H1 video-only sessions without ELF restart;
4. H1 PCM-audio-only compatibility session;
5. H1 combined PCM + MPEG session on the same physical mux;
6. only then begin automated parameter sweeps.

## Reusable operator / optimization tool

`h1_tool.py` is the canonical human-facing control surface for H1 testing. It
wraps the current cumulative runner rather than reimplementing transport or
media behavior.

It provides:

- `knobs` -- list every adjustable field, current profile default and meaning;
- `show` -- resolve modifiers without starting hardware;
- `run` -- execute one parameterized resident-H1 session; and
- `sweep` -- generate and execute repeatable multi-parameter batches with a
  recorded plan, per-case evidence/logs and machine-readable results.

Frequently used settings have friendly switches such as `--audio-priority`,
`--capture`, `--encode`, `--draw`, and `--media-rect`. Every current/future H1
profile field remains reachable through `--set NAME=INTEGER`, while sweeps use
`--vary NAME=V1,V2,...`.

The CLI is intentionally self-documenting:

```sh
python3 experiments/media-harness-h1/h1_tool.py --help
python3 experiments/media-harness-h1/h1_tool.py run --help
python3 experiments/media-harness-h1/h1_tool.py sweep --help
python3 experiments/media-harness-h1/h1_tool.py knobs \
    --profile P11_COMPAT_PLUS_PCM
```

See `H1_TOOL_GUIDE.md` for the compact operator guide.

## Future hybrid media-object composition note

The 2026-09-08 geometry experiments motivated a future model in which media is
represented as a semantic desktop object attached to a host window rather than
as a macroblock-aligned MPEG rectangle. The concept includes generic dynamic-
region discovery, exact host-relative media geometry, codec-only padding,
optional shape/visibility masks for rounded corners, and eventual compositor
layering of RFB -> MPEG media object -> local cursor/OSK.

This is a design note only; it is not current implemented behavior and does not
begin Issue #40.

See `MEDIA_OBJECT_COMPOSITION_NOTES.md` for the full rationale and proposed
separation between detection, host attachment, semantic geometry, codec
surfaces and presentation.
