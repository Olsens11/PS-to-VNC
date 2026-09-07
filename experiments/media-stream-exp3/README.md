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

### Reference-derived one-picture smoke — first hardware result

Authority:
- EXP3 commit: `6ea2ea17686be29f9b57f33a55b6372b339c227b`
- ELF SHA-256: `e99085a9087207d04e3fcaaed94cfb429d5a7298b47d098d5f395f9d80b51d60`
- ELF bytes: `9028116`
- stream SHA-256: `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`

Observed PS2 hardware sequence:

`CYAN -> YELLOW -> ORANGE -> ORANGE indefinitely`

Interpretation:
- standalone graph chassis passed;
- explicit IPU/DMAC reset returned;
- `MPEG_Initialize()` returned successfully;
- failure boundary moved into the first `MPEG_Picture()` operation;
- no green first-picture-success state and no red decoder-return state were reached.

Post-test source comparison found that this probe returned an uncached
`0x2xxxxxxx` alias from its libmpeg sequence callback. That behavior had been
inferred from Sony retail `sceMpegGetPicture`, but both the PS2SDK libmpeg
sample and SMS use ordinary aligned EE pointers with the open-source libmpeg
decoder. The next run corrects only that decoder-output address contract.

### Reference one-picture smoke — cached output buffer result

Authority:
- EXP3 commit: `c932215eaded06babaff68e3362592f0ed456d8f`
- ELF SHA-256: `875446d2ef9715dff442bbb5b150d414e09484dfaa5e05899d57ccf8e7b55982`
- ELF bytes: `9027988`

Observed PS2 hardware sequence:

`CYAN -> YELLOW -> ORANGE -> ORANGE indefinitely`

No red state and no green state appeared.

Interpretation:
- graph chassis passed;
- explicit IPU/DMAC reset passed;
- `MPEG_Initialize()` passed;
- changing the picture destination from an uncached alias to the normal aligned
  pointer expected by PS2SDK libmpeg and SMS did not remove the first-picture
  hang;
- the normal aligned pointer remains the correct libmpeg contract and is
  retained.

The remaining experimental divergence from the PS2SDK libmpeg sample is the
input callback. This revision replaces the custom 64 KiB direct-register feed
with the PS2SDK sample's own DMA-channel initialization, wait, and normal-DMA
submission model using 2048-byte staging blocks.
