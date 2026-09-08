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
