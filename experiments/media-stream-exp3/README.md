# Media Stream EXP3

## Directory purpose

This directory contains isolated experiments for the proposed PS2VNC
persistent-media architecture.

The product architecture under investigation is:

- RFB remains the ordinary interactive desktop path.
- Audio remains continuously active.
- MPEG-2 video becomes an optional full-motion video path.
- Audio and video will eventually share one media playback timeline.
- The existing dedicated-link mux remains the transport foundation.

Nothing in this directory is product-qualified merely because it builds.

## Files

### `libmpeg_embedded_smoke.c`

First PS2 hardware proof for MPEG-2.

It is deliberately independent of:

- PS2 networking;
- the PS2VNC RFB parser;
- the audio runtime;
- USB mass-storage drivers;
- the mux transport;
- the qualified Issue39 application runtime.

The MPEG-2 elementary stream is linked directly into the ELF as a binary
object. The decoder callback copies each input block into a 64-byte-aligned
2048-byte staging area before DMA to the IPU.

The display behavior is intentionally diagnostic:

- blue before MPEG decoding begins;
- MPEG video while pictures decode;
- green after a successful long decode;
- red if decoding stops before the minimum expected picture count.

## Hardware acceptance contract

A successful first hardware test requires:

1. the ELF launches;
2. a blue setup screen may appear briefly;
3. the moving test pattern displays continuously and smoothly;
4. playback lasts approximately 20 seconds;
5. the final display becomes green;
6. the PS2 does not freeze or reset.

The initial threshold is 550 decoded pictures. The generated stream contains
approximately 599 pictures at 30000/1001 frames/sec.

## Source basis

The MPEG decode/display mechanism is intentionally derived from the PS2SDK
`ee/mpeg/samples/mpeg.c` example by Eugene Plotnikov.

EXP3 changes the input mechanism and adds diagnostic state only. The eventual
network implementation will replace the embedded input with the reserved
MPEG2 transport channel.
