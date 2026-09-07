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

### Canonical PS2SDK-feed one-picture result

Authority:
- EXP3 commit: `7eca3a09e229fd8f615c97b319e0190c9b0fdba3`
- ELF SHA-256: `beb979c29ae84fb4c0bf81d5a5a27cbe2e4761a67089a1e41fcfcf668939e884`
- ELF bytes: `9027988`
- MPEG stream SHA-256:
  `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`

This revision used the PS2SDK sample input model:
- 2048-byte staging blocks;
- `dma_channel_wait(DMA_CHANNEL_toIPU, 0)`;
- `dma_channel_send_normal()`;
- normal aligned decoded-picture pointer;
- no `dma_wait_fast()` in the decoder path.

Observed hardware result:

The display appeared to proceed directly to ORANGE and remained ORANGE
indefinitely. No GREEN first-picture-success state and no RED decoder-return
state were observed.

Therefore the first `MPEG_Picture()` call still failed to return after both
major external libmpeg contracts were aligned with the PS2SDK sample.

The next controlled A/B keeps the harness and MPEG stream bit-identical while
replacing modern PS2SDK libmpeg with the mature SMS implementation from pinned
SMS commit `c1898094725ad750ec20e10cc148b39d7c8a9c65`.

### SMS A/B preflight — rejected G0 build

A preliminary SMS-core ELF was successfully assembled and linked:

- SHA-256:
  `19bb512dbab3b0741d18ff4e57b98b5a35aeed5fa0045adfb9b7619daec67168`
- bytes: `9022116`

This ELF was NOT deployed or hardware-tested.

Reason for rejection:

The first isolated SMS target inherited the current PS2SDK sample `-G0`
compiler setting instead of SMS's historical small-data build settings.
The modern assembler consequently reported many expanded pseudo-instructions,
including expansions occurring inside MIPS branch delay slots.

SMS's own build uses `-G8192 -mgpopt`. Because the assembly core deliberately
places symbolic memory operations in delay slots, changing the small-data
model can change instruction expansion and therefore execution semantics.

The upstream SMS source remains byte-identical. The corrected A/B compiles
only the SMS decoder objects with the historical small-data settings while
the control harness remains bit-identical.

### SMS assembly-core build preflight — PASS

The pinned mature SMS MPEG decoder was compiled against the current EXP3
one-picture harness without modifying any SMS decoder source.

SMS source authority:
- repository: `ps2homebrew/SMS`
- commit: `c1898094725ad750ec20e10cc148b39d7c8a9c65`

Control invariants:
- one-picture harness remained bit-identical;
- MPEG elementary stream remained bit-identical;
- system PS2SDK `-lmpeg` was not linked;
- SMS `libmpeg.c` and `libmpeg_core.S` supplied the public/core decoder symbols.

Required decoder-object build model:
- `-G8192`
- `-mgpopt`
- `-mno-abicalls`

The explicit `-mno-abicalls` is required with the modern compiler because
small-data `-G` accesses cannot be compiled in ABICALLS mode.

Successful preflight:
- ELF SHA-256:
  `9d3c5726fb3310851cea539115c4db324ad229a08c6701cbed9fc5cbc6e3b56f`
- ELF bytes: `10241688`
- ABICALL/small-data errors: `0`
- multi-instruction macro expansions: `0`
- branch-delay-slot macro expansions: `0`
- assembler `$at` diagnostics: `213`
- mixed ABICALL/non-ABICALL static-link diagnostics: `3`
- final MPEG symbol resolution: PASS
- SMS `.sdata` / `.sbss` sections: present

The `$at` messages are retained as build evidence. They did not correspond to
pseudo-instruction expansion, and no multi-instruction expansion occurred.

This ELF was initially a build-only preflight. The next operation rebuilds the
same target from committed source authority and requires exact ELF SHA/byte
reproduction before hardware deployment.

### SMS assembly-core first-picture hardware result — PASS

Authority:
- EXP3 commit:
  `ab88f0432edc0d343fb116bebd84f86cc770a641`
- ELF SHA-256:
  `9d3c5726fb3310851cea539115c4db324ad229a08c6701cbed9fc5cbc6e3b56f`
- ELF bytes:
  `10241688`
- archival path:
  `/mass/0/PS2VNC-exp3-sms-core-one-picture-9d3c5726.ELF`

Observed hardware sequence:

- an initial black screen with a thin green line appeared before the experiment
  color stages; the operator notes the same transition when FMCB launches
  wLaunchELF 4.43x_ISR, so it is treated separately from the MPEG-stage
  contract;
- CYAN;
- YELLOW;
- ORANGE;
- GREEN, persistent.

The GREEN state is entered only after `MPEG_Picture()` returns nonzero.
The test deliberately sleeps on GREEN.

Result:

`SMS_FIRST_MPEG_PICTURE_DECODE=PASS`

This is the first hardware-proven complete MPEG-2 picture decode in EXP3.

Controlled A/B conclusion:

The modern PS2SDK libmpeg control remained ORANGE indefinitely inside the
first `MPEG_Picture()` call.

The pinned mature SMS assembly libmpeg implementation returned successfully
from that same call while preserving:
- the bit-identical one-picture harness;
- the bit-identical MPEG elementary stream;
- the same 2048-byte TO_IPU feed callback;
- the same normal aligned picture buffer contract.

Therefore the observed first-picture failure tracks with the modern PS2SDK
libmpeg implementation/build lineage rather than those shared external
contracts.

The next increment does not attempt continuous playback. It presents this one
already-proven decoded picture through GS.
