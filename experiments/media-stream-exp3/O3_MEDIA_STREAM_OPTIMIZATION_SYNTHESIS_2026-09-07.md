# EXP3 MPEG Optimization Synthesis and Product Integration Direction — 2026-09-07

## Status and purpose

This document consolidates the most valuable hardware knowledge produced by the
September 7, 2026 EXP3 MPEG-2 campaign. It exists because the individual result
records correctly preserve each isolated experiment, but the causal chain across
those experiments is itself critical project knowledge.

This is a synthesis of empirical results, not a replacement for the individual
hardware records. When an exact number, source identity, or device-under-test
identity matters, consult the named result file as the primary evidence record.

Current experiment branch at the start of this synthesis:

`experiment/media-stream-exp3`

Current hardware-result authority before this document:

`a002c3a5c74ba9111b081704768a1b0eafb66994`

That commit records the P12 true-live 608x416 4M-profile hardware result.

The central conclusion is:

> The PS2 can decode and present high-quality live MPEG-2 from the Pi at useful
> desktop/video resolutions with large real-time headroom. The dominant failures
> seen earlier were caused by presentation policy and insufficient compressed-data
> elasticity, not by a fundamental inability of the IPU/GS or IOP-to-EE network
> path to sustain MPEG video.

The best current product-shaped live result is P12:

- live Pi X11 capture;
- 608x416 MPEG-2 Main Profile/Main Level;
- yuv420p;
- 30000/1001 fps;
- 4,000,000 bit/s target;
- 5,000,000 bit/s maximum;
- 1,835,008-bit VBV;
- GOP 15;
- no B-frames;
- 512 KiB compressed-input capacity;
- 448 KiB startup prefill in the test harness;
- mature SMS libmpeg assembly implementation;
- SMS `_MPEG_Set16(1)` RGB16 output;
- PSMCT16 GS texture path;
- absolute monotonic 30000/1001 presentation clock;
- no frame dropping.

P12 produced zero decoder-feed waits, zero presentation deadline misses, zero
measured lateness, and visually very good live YouTube playback. The test was
bounded to 24 seconds and therefore does **not** yet qualify indefinite streaming,
audio/video synchronization, or combined RFB/audio/MPEG operation.

---

# 1. Hardware and software contracts that must survive future integration

## 1.1 Decoder implementation

The hardware-qualified MPEG decoder is the pinned mature SMS implementation,
not the modern PS2SDK libmpeg implementation tested during EXP3.

The modern PS2SDK lineage reached `MPEG_Initialize()` but hung in the first
`MPEG_Picture()` request under the same important external contracts that later
worked with SMS. The mature SMS path therefore remains the qualified decoder
until an isolated hardware experiment proves otherwise.

Public SMS source authority used during the study:

- repository: `ps2homebrew/SMS`;
- studied commit: `c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- Eugene Plotnikov;
- AFL-2.0 licensed source.

Do not silently replace this path with system `-lmpeg` merely because both
export similarly named APIs.

## 1.2 IPU input DMA contract

The qualified feed model is deliberately conservative and follows the proven
sample/SMS ownership model:

- one 64-byte-aligned EE staging buffer;
- ordinary TO_IPU DMA;
- normal 2048-byte input blocks when enough compressed data is available;
- the final short transfer rounded to the required 16-byte/qword DMA boundary;
- `dma_channel_wait(DMA_CHANNEL_toIPU, 0)` before submission;
- `dma_channel_send_normal()`;
- no source-chain DMA into IPU.

A broad claim that every IPU transfer length must be a multiple of 64 bytes is
**not** supported by the qualified implementation. Alignment of the staging
buffer and transfer-length requirements are separate issues. The proven final
short-block behavior is 16-byte/qword rounding, not a universal 64-byte payload
rule.

## 1.3 `dma_wait_fast()` prohibition

A hardware bisection demonstrated that the fast-wait helper can hang in this
MPEG/display environment. Qualified MPEG code uses ordinary DMA-channel waits.

Do not reintroduce `dma_channel_fast_waits()` or equivalent fast-wait behavior
as an optimization without a dedicated hardware qualification.

## 1.4 Decoded-picture ownership

The libmpeg output pointer is an ordinary appropriately aligned EE pointer. An
earlier uncached-pointer assumption imported from a different Sony MPEG API was
incorrect for this path.

The bounded SMS experiments initially reused one decoded-picture buffer. The GIF
upload source-chain REF tags point into that EE buffer; therefore the ordinary
GIF-channel wait after upload is an ownership boundary. Only after the upload
has retired may the decoder safely overwrite the same picture storage.

This remains important if future product work introduces multiple decode/render
buffers: ownership must become more concurrent, not less explicit.

## 1.5 RGB16 is an SMS-native optimization

P8 qualified `_MPEG_Set16(1)` and PSMCT16 presentation on real hardware.

The path changes decoded/output storage from:

- RGB32 / PSMCT32;
- 4 bytes/pixel;
- 1024-byte / 64-qword 16x16 macroblocks;

to:

- RGB16 / PSMCT16;
- 2 bytes/pixel;
- 512-byte / 32-qword 16x16 macroblocks.

This is not an EE-side post-conversion shortcut. The mature SMS core explicitly
supports the 16-bit path. P8 preserved the exact smooth stress-reel behavior.

A dedicated gradient/subtle-color benchmark is still required before declaring
RGB16 visually lossless for every desktop use case. The existing stress reel is
excellent for motion and compression but not for fine color-depth judgment.

## 1.6 Explicit stream termination

The qualified bounded raw MPEG elementary-stream terminal suffix is:

`00 00 01 B7` followed by twelve zero bytes.

The literal sequence-end code makes SMS terminal behavior deterministic enough
for the current harness. A temporarily empty live receive queue is **not** EOF.
Future indefinite sessions must distinguish temporary lack of bytes from an
explicit media-session stop/end transition.

## 1.7 Terminal colors are two independent classifiers

The experimental terminal behavior must not be simplified into “red means bad”:

- PURPLE: coded picture count = returned/displayed + 1;
- WHITE: coded picture count = returned/displayed;
- RED: another returned-picture relationship;
- CYAN afterward: exact concurrent transport/integrity success.

Multiple clean streams have produced coded-minus-two while passing byte count,
CRC, queue drain, receiver completion, and transport integrity. Returned-picture
count at MPEG sequence end remains a separate SMS/reference-picture/EOF
characterization issue.

---

# 2. Performance campaign — complete causal benchmark chain

The table below summarizes the experiment sequence. “Feed waits” are PS2 decoder
feed-callback waits and are more directly relevant to compressed-input starvation
than Pi-side sender credit waits. Sender credit waits may simply mean the PS2
queue is full and backpressure is working correctly.

| Test | Workload / key variable | Queue / prefill | Presentation / output | Principal measured result | Operator result / conclusion |
|---|---|---|---|---|---|
| O1 | Full 8.82 MB fixture acquired before decode | whole fixture | SMS RGB32 | 3.415 MiB/s network acquisition | Full moving MPEG PASS; proves raw transfer bandwidth is not the basic limit |
| O2 Q8192 | Concurrent receive+decode, one 8192-byte DATA frame of capacity | 8 KiB / none | original serialized path | 0.1165 MiB/s; 1077 feed events / 47418 loops / max 216 | Slow/choppy; near stop-and-wait |
| O2 Q32768 | Same ELF/source, only queue 8→32 KiB | 32 KiB / none | same | 0.33467 MiB/s; 382 /1593 / max48 | Smooth/snappy; first major queue-depth clue |
| O3 live 4M | True X11→ffmpeg live 704x480 | 32 KiB | original fixed presentation | 0.25026 MiB/s; ~96% producer credit-wait; 566 /4306/max53 feed | Live correctness PASS, visibly clunky |
| P1 | Same live path, bitrate 4M→3M only | 32 KiB | same | 0.24871 MiB/s; 501/4624/max52 | First 2–3 s better, then clunky; lowering nominal bitrate did not fix rate |
| P2 | Pi-only X11→ffmpeg 3M control | n/a | no PS2 | 600 frames in 20.44 s; 0.3607 MiB/s wall write | Pi encoder is real-time; slowdown is downstream |
| P3 | Exact P2 archive replay | 32 KiB | 2 VSYNC waits/picture | 27.83 s, 0.26499 MiB/s | Smoother than live but still clunky; downstream sustained limit remains |
| P4 | Same replay, remove both per-picture VSYNC waits | 32 KiB | 0 waits | 16.75 s, 0.44030 MiB/s = 1.194× required real-time rate | Fast/choppy; proves raw decode/upload/draw can exceed real time |
| P5 | Same replay, exactly one VSYNC | 32 KiB | 1 wait | 19.21 s, 0.38391 MiB/s | Constant speed-up/slow-down; fixed-N waiting is wrong despite correct average |
| P6/P6B | Absolute 30000/1001 deadline clock, no drops | 32 KiB | absolute clock | P6B: 289 misses; max lateness 39.97 frames | Burst/catch-up behavior now explained quantitatively |
| P7 | Late-frame presentation drop experiment | 32 KiB | skip late GS draw | **NOT RUN** | Deliberately stopped before masking root cause |
| P7A | Buffer-only change | 256 KiB /224 KiB | same P6 absolute clock, RGB32 | misses 289→6; feed 512→7 events; max 1213→1; max late 39.97→0.274 frame | “Very best test yet”; dominant starvation cause found |
| P7B | Resolution 704x480→608x416 only | 256/224 KiB | same P7A ELF | 6 misses; 13/91/max49 feed; 599/600 returned | Smooth; 608x416 established as good geometry |
| Stress 2M | Deterministic 24 s stress reel; Main/Main/VBV shaping | 256/224 KiB | RGB32; absolute clock | 0 feed waits; 0 misses; 0 lateness | Very smooth; fixed benchmark established |
| P8 | RGB32→SMS-native RGB16 only | 256/224 KiB | PSMCT16 | same zero waits/misses/lateness | Visually indistinguishable on motion reel |
| P9 | Instrument P8 stages only | 256/224 KiB | RGB16 | decode avg 7.317 ms, max 19.118; upload 0.826; draw 0.0084 | Decoder has large real-time headroom |
| P10 | Stress quality 2M→4M | 256/224 KiB | RGB16 | decode max 451 ms; feed max436 loops; 56 misses; max late434.5 ms | Looked great but telemetry exposed starvation at higher-rate bursts |
| P11 | Same exact P10 bitstream; queue/prefill doubled | 512/448 KiB | RGB16 | decode max42.7 ms; feed max5; 39 misses; max late89.8 ms | Looked great; PURPLE→CYAN; pathological starvation collapsed |
| P12 | Exact P11 PS2 path; fixed replay→true live X11 | 512/448 KiB | RGB16; 4M/5M/VBV live profile | **0 feed waits, 0 misses, 0 lateness**; decode avg6.97,max23.68 ms | True-live YouTube surprisingly good; strongest live result |

The sections below retain exact identities and settings where they are most
important.

---

# 3. Detailed benchmark records and lessons

## 3.1 O1 — network MPEG fixture acquisition before decode

Primary record: `O1_HARDWARE_PASS.md`.

- code authority: `3c9ff944eb53f55dd903d361d626e923ccba1359`;
- ELF SHA-256: `3a40ca6bea39db5cdf8f0482a9028e21cf646a84d4c531ef2f768e413d791089`;
- ELF bytes: 2,433,192;
- fixture SHA-256: `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`;
- fixture CRC32: `4bdc7859`;
- fixture bytes: 8,815,372;
- coded pictures: 599;
- PSTV logical channel: MPEG2 channel 4;
- TCP port: 5904;
- 1077 DATA frames: 1076 x 8192 bytes plus final 780 bytes;
- transfer elapsed: 2.461478 s;
- payload throughput: 3.415425 MiB/s.

O1 deliberately acquired the entire fixture before MPEG decoding began. It
therefore proved transport bandwidth/integrity and successful playback of
network-received bytes, but not concurrent producer/consumer streaming.

Most important lesson: raw Ethernet/PS2 transfer capability is far above the
few-megabit MPEG operating points used later. A later throughput problem should
not automatically be called a physical-link or SIF bandwidth ceiling.

## 3.2 O2 Q8192 — minimum concurrent receive/decode queue

Primary record: `O2_Q8192_HARDWARE_PASS.md`.

- code commit: `9a2680801c8e5b18f23a4c5f85f50db2edcbccec`;
- ELF SHA-256: `7c5015832213414651a6aa7f98e2a63c05e1e1b4a01230e8abd4fe63d4d50fd4`;
- same exact fixture as O1;
- queue: 8192 bytes, exactly one maximum PSTV DATA payload;
- receiver stack: 16384;
- receiver priority: 63;
- send elapsed: 72.156688 s;
- throughput: 0.116510 MiB/s;
- feed wait events: 1077;
- feed wait loops: 47418;
- maximum feed wait loops: 216;
- decoded/displayed: 598/598.

Correctness and exact transport accounting passed, but playback was roughly
three times slower than content time. The one-frame transport reservoir exposed
near stop-and-wait behavior.

## 3.3 O2 Q32768 — queue depth alone transforms playback

Primary record: `O2_Q32768_HARDWARE_PASS.md`.

Same ELF and same exact fixture as Q8192. Only queue capacity changed to 32768.

- send elapsed: 25.120161 s;
- throughput: 0.334671 MiB/s;
- feed wait events: 382;
- feed wait loops: 1593;
- max feed wait loops: 48;
- queue high-water: 22528;
- operator: “smooth and snappy.”

Q8192→Q32768:

- send time 72.16→25.12 s;
- feed loops 47418→1593 (~96.6% reduction);
- max feed wait 216→48 loops.

This was the first empirical indication that **elastic buffering is a first-class
performance variable**, not merely a safety reserve.

## 3.4 Initial O3 true-live YouTube 4M

Primary record: `O3_TRUE_LIVE_YOUTUBE_4M_HARDWARE_PASS.md`.

Configuration:

- X11 source `:0.0`, 704x462 padded to 704x480;
- 30000/1001 fps;
- MPEG-2 elementary video;
- nominal `-b:v 4M`;
- GOP 15;
- `-bf 0`;
- queue 32768;
- explicit `B7 + 12 zeros` termination;
- requested 20 s.

Results:

- 7,017,283 payload bytes;
- 551 coded → 550 displayed;
- elapsed 26.741170 s;
- 0.250258 MiB/s;
- producer credit wait 25.693999 s (~96.1% of run);
- feed waits 566 events /4306 loops /max53;
- operator: live motion was correct but visibly clunky.

This qualified the complete live path but not real-time performance.

## 3.5 P1 — lowering nominal bitrate alone was ineffective

Primary record: `O3_LIVE_PERFORMANCE_P1_3M_HARDWARE_RESULT.md`.

Only `4M→3M` nominal bitrate changed. Queue remained 32 KiB, geometry 704x480,
30000/1001, GOP15, B-frames disabled.

- payload 6,809,062 bytes;
- 520 coded→519 displayed;
- elapsed 26.109300 s;
- 0.248709 MiB/s;
- 95.99% producer credit-wait fraction;
- feed 501/4624/max52;
- first 2–3 s looked relatively smooth, then clunky.

The sustained rate barely changed. Lowering a target bitrate was not the
root-cause fix.

## 3.6 P2 — Pi-side X11/ffmpeg can sustain real-time production

Primary record: `O3_LIVE_PERFORMANCE_P2_LOCAL_CAPTURE_RESULT.md`.

Same 3M capture path but no PS2/network:

- X11 704x462 → 704x480;
- yuv420p;
- 30000/1001;
- 3M;
- GOP15;
- `-bf 0`;
- nominal 20 s;
- wall time 20.441630 s;
- 600 pictures;
- 7,732,149 bytes;
- SHA-256 `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`;
- CRC32 `ce1e9d03`;
- wall write 0.360732 MiB/s;
- nominal 20-second payload rate 0.368698 MiB/s;
- software decode PASS.

This moved the bottleneck downstream of standalone capture/encoding.

## 3.7 P3/P4/P5 — presentation timing bracket

P3 replayed the exact P2 archive through the 32 KiB live decoder path with the
original two per-picture VSYNC waits.

P3:

- 27.827575 s;
- 0.264988 MiB/s;
- visually smoother than live, still clunky.

P4 changed only presentation to zero per-picture VSYNC waits:

- generator `3480cd11a36be9d4666684f53b362f627f43dc37`;
- ELF SHA `abc7763aa9282ea9b3764ea522df2cc2f58d6b40e48218fbc03862dd78f2f368`;
- 16.747557 s;
- 0.440301 MiB/s;
- source real-time requirement 0.368698 MiB/s;
- 1.194204x the required rate;
- visibly too fast / choppy.

P5 changed only to one fixed VSYNC wait:

- apparatus commit `f3ce759e68a28b6df6a5d596e72e5036df5052ad`;
- ELF SHA `efd680caa9d207754fd2dbcfc2efe731353c66d1898948421f227b190893484c`;
- 19.207771 s;
- 0.383905 MiB/s;
- 1.041245x required real-time average;
- feed 527/5063/max50;
- operator: repeatedly “shooting forward then slowing down.”

This hardware bracket established three facts:

1. the decoder/IPU/GS path has enough aggregate speed;
2. two fixed waits over-throttle it;
3. one fixed wait can average close to the target rate while still producing
   poor instantaneous cadence.

Therefore presentation must be scheduled against a clock, not implemented as a
fixed sleep after variable-cost work.

## 3.8 P6/P6B — absolute clock reveals the backlog mechanism

P6 introduced a source-cadence absolute clock:

- epoch immediately before picture 1;
- frame rate 30000/1001;
- deadline = epoch + `(index - 1) * period`;
- wait while early;
- when late, record lateness and return immediately;
- still display the late picture;
- no frame dropping.

A fixed known cadence was used because the harness passed a non-advancing
`current_stream_pts` into SMS; returned PTS values were not yet a trustworthy
playback clock.

P6B added post-playback timing telemetry only.

Primary record: `O3_LIVE_PERFORMANCE_P6B_CLOCK_TELEMETRY_HARDWARE_RESULT.md`.

- apparatus `0972062020ebd9c10eac01e87926221e85fcb264`;
- ELF SHA `523ab812971641e397ac4e207df9a349380193fdc2b57c6ce87a65819428c5f2`;
- exact P2 archive;
- queue 32768;
- deadline misses 289 of 598 returned pictures (48.33%);
- maximum lateness 39.967649 frame periods (~1.33 s);
- associated campaign feed counters: 512 events /5757 loops /max1213.

This explained the fast/slow visual behavior. Once late, the no-drop clock does
not wait, so stale decoded frames appear rapidly while the scheduler catches up.
The correct conclusion was not “drop frames immediately”; it was “find why the
pipeline became so late.”

## 3.9 P7 was deliberately not run

A late-presentation-drop experiment was prepared, but not hardware-run. The user
correctly stopped the sequence before using frame dropping to mask a potentially
fixable starvation problem.

Do not later cite P7 as qualified evidence.

## 3.10 P7A — the dominant starvation breakthrough

Primary record: `O3_LIVE_PERFORMANCE_P7A_DEEP_BUFFER_HARDWARE_RESULT.md`.

Only compressed buffering changed relative to the P6/P6B behavior:

- queue 32 KiB → 256 KiB;
- mandatory startup prefill 224 KiB;
- exact same P2 704x480/3M archive;
- same SMS decoder;
- same GS path;
- same absolute clock;
- no dropping.

Identity:

- generator/staging authority `14541b5d93945be0b03a304d22f2781c12fb9b43`;
- generated blob `f9bfcb3c9a967a51f4f09b04175faae136c25f7b`;
- ELF SHA `efbf929323423757043dd195f9a7ee1471c18797085dc0a5ed108a54da59be20`;
- ELF bytes 2,471,776.

Results:

- deadline misses 289→6;
- max lateness 39.967649→0.274205 frame periods;
- feed wait events 512→7;
- feed wait loops 5757→7;
- max feed-wait loops 1213→1;
- operator: “that was our very best test yet.”

This is one of the most important causal results in the project. A 32 KiB
compressed reservoir was a dominant cause of decoder starvation and visible
catch-up. The same decoder/IPU/GS path becomes nearly perfectly paced when it
has enough compressed-input runway.

## 3.11 P7B — 608x416 becomes the preferred current geometry

Primary record: `O3_LIVE_PERFORMANCE_P7B_608X416_HARDWARE_RESULT.md`.

The exact P7A ELF and buffer policy were reused. Only source resolution changed:

- 704x480 → 608x416;
- 30000/1001;
- MPEG-2 yuv420p;
- nominal 3M;
- GOP15;
- no B-frames;
- 600 coded pictures;
- source bytes 7,704,609;
- SHA `eb9521238271881f67991d2d240ba5226068ffd062db0c5b76c3f59cdbce495b`.

608x416 contains about 25.15% fewer pixels than 704x480 while preserving nearly
the same aspect ratio and remaining macroblock-aligned. The run was visually
smooth and ended PURPLE→CYAN.

Compressed source size did not materially fall because nominal bitrate remained
3M. This separated picture-cost reduction from compressed-rate reduction.

## 3.12 Deterministic 608x416 stress reel — stable benchmark source

Primary record: `O3_STRESS_REEL_608X416_2M_VBV_HARDWARE_RESULT.md`.

A 24-second deterministic benchmark was created to make micro-stutter and
compression defects easier to see than arbitrary web video. Its sections include:

1. general test pattern;
2. fine 8-pixel grid over moving imagery;
3. constant horizontal scrolling of hard edges;
4. deliberately difficult high-detail temporal noise/grid content.

Lossless reference:

- FFV1;
- SHA `1d42d891f276918dc9a4d6211cc21eec4e546a490c426f2d880d832175b82f0c`;
- 720 frames.

MPEG benchmark:

- 608x416;
- 30000/1001;
- MPEG-2 Main/Main;
- yuv420p;
- target 2,000,000 bit/s;
- maxrate 2,516,000 bit/s;
- VBV 1,835,008 bits;
- GOP15;
- B-frames 0;
- 6,209,147 bytes;
- SHA `6bcfb93badf653a3d72de83e7d9253d01fe0027283f171954c9de6cefbf40aca`;
- 720 coded pictures;
- queue256KiB / prefill224KiB.

Hardware:

- zero feed waits;
- zero deadline misses;
- zero lateness;
- visually very smooth;
- 720 coded→718 returned/displayed;
- RED→CYAN solely because of returned-picture delta2.

This should remain a fixed benchmark source for future decoder/output-quality
A/B tests.

## 3.13 P8 — RGB16 output qualification

Primary record: `O3_P8_RGB16_HARDWARE_RESULT.md`.

P8 changed only SMS/IPU/GS output format against the exact stress reel:

- `_MPEG_Set16(1)`;
- RGB16 / PSMCT16;
- 2 bytes/pixel;
- 512-byte /32-QW macroblocks.

Identity:

- source-change authority `9a192be069e47126cecd60c293db5a008795165d`;
- ELF SHA `ca6ad330ccb9925dee5423d3349316e919597156f2a1fcb1b444f4d4fbd2e023`.

The run retained zero feed starvation, zero deadline misses, and zero lateness.
Motion looked identical to the RGB32 baseline. The benchmark colors were not
sufficiently subtle to qualify color precision.

## 3.14 P9 — direct stage timing demonstrates processing headroom

Primary record: `O3_P9_RGB16_STAGE_TIMING_HARDWARE_RESULT.md`.

P9 added timing reads and post-playback telemetry around the already-qualified
RGB16 path without changing playback behavior.

Identity:

- generator authority `ece6fd08c5d1d7b19594e3852212cca4639e2d4e`;
- generated source blob `5567605e2068596cb943911c2c4fa20de9b9a056`;
- ELF SHA `cea181e37c89990e76d99d22a6fae65bd3a7c92cbea3bcc04e4fd9128d635d83`;
- ELF bytes 2,476,288.

At the zero-starvation 2M benchmark:

- frame period: 33.366667 ms;
- `MPEG_Picture()` average: 7.316977 ms;
- `MPEG_Picture()` maximum: 19.118056 ms;
- RGB16 GS upload average: 0.826026 ms;
- upload max: 0.850694 ms;
- draw average: 0.008381 ms;
- draw max: 0.013889 ms;
- serial decode+upload+draw average indicator: 8.151384 ms;
- nominal remaining frame period: about 25.215 ms.

`MPEG_Picture()` wall time can include feed-callback waiting. P9 is meaningful
precisely because feed waits were zero.

This rejected the hypothesis that the decoder itself was close to its 29.97-fps
limit at this operating point.

## 3.15 P10 — quality increase reveals a new reservoir boundary

Primary record: `O3_LIVE_PERFORMANCE_P10_4M_HARDWARE_RESULT.md`.

P10 reused the exact P9 RGB16 timing ELF and 256/224 KiB buffer policy, changing
only encoder quality against the same lossless stress content:

- target 4,000,000;
- maxrate 5,000,000;
- minrate 0;
- VBV 1,835,008 bits;
- Main/Main;
- yuv420p;
- GOP15;
- `-bf 0`;
- 608x416 / 30000/1001;
- 10,910,594 bytes;
- actual average 3.636865 Mbit/s;
- SHA `d98c21b50400236d5617b2bf42d612413909b79bff5887f507589060cb39321f`.

Operator observation: motion remained very smooth and image quality looked great.

Telemetry:

- decode average 9.586989 ms;
- decode maximum 451.017379 ms;
- GS upload average 0.824846 ms;
- GS draw average 0.008367 ms;
- deadline misses 56;
- max deadline lateness 434.505925 ms;
- feed waits 409 events /955 loops /max436.

The near one-to-one relationship between ~436 ms worst feed waiting and 451 ms
worst `MPEG_Picture()` wall time is the key. The large “decode” spike was not a
451-ms intrinsic MPEG decode; it was predominantly a decoder call blocked waiting
for compressed input.

## 3.16 P11 — 512 KiB capacity removes pathological 4M starvation

Primary record: `O3_LIVE_PERFORMANCE_P11_DEEP_4M_HARDWARE_RESULT.md`.

P11 replayed the **exact P10 bitstream** on the same decoder/RGB16/GS/clock/no-drop
path. Only buffering changed:

- queue 256→512 KiB;
- mandatory startup prefill 224→448 KiB.

Identity:

- generator authority `bc36d8f33c567d48500000502572463e847dead6`;
- source blob `007a439f6070e1b37e75c742cd551c290fe69b47`;
- ELF SHA `5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`;
- ELF bytes 2,476,164;
- rolling path `/mass/0/PS2VNC-exp3-p11-deep-4m.ELF`.

Operator: looked great; PURPLE→CYAN; final frame held.

Telemetry:

- queue high-water 522240 /524288;
- feed waits 332 events /348 loops /max5;
- deadline misses 39;
- max lateness ~89.805 ms /2.691 frames;
- `MPEG_Picture()` average ~8.670 ms;
- maximum ~42.724 ms;
- upload ~0.825 ms;
- draw ~0.0084 ms;
- serial average ~9.504 ms;
- average processing share ~28.5%;
- average nominal frame headroom ~23.863 ms.

P10→P11:

- feed loops 955→348 (-63.6%);
- worst feed span 436→5 loops (-98.9%);
- decode max 451→42.7 ms (-90.5%);
- max lateness 434.5→89.8 ms (-79.3%).

The remaining feed-event count is misleading if read without duration: average
loops/event fell from ~2.34 to ~1.05 and the pathological hundreds-of-ms wait was
removed.

This strongly supports “adequate aggregate throughput + bursty delivery +
insufficient elasticity” rather than “the IOP-to-EE bridge cannot carry MPEG.”
The queue nearly filled and Pi-side credit backpressure engaged, proving the
upstream path can deliver bursts faster than the consumer drains them.

## 3.17 P12 — product-shaped true-live breakthrough

Primary record: `O3_LIVE_P12_TRUE_LIVE_4M_HARDWARE_RESULT.md`.

P12 reused the exact P11 PS2 ELF and changed the producer from fixed archive to
true X11 capture.

Producer wrapper:

- commit `8c9bca2549c3d99fd7b17030487015d5835f4893`;
- blob `339498b14d4eb83a561a5930408c939038eab88e`.

Actual FFmpeg shape:

- X11 input 704x462 at 30000/1001;
- scale 608x416 bicubic;
- yuv420p;
- `mpeg2video`;
- Main Profile / Main Level;
- `-b:v 4000000`;
- `-maxrate:v 5000000`;
- `-minrate:v 0`;
- `-bufsize:v 1835008`;
- GOP15;
- `-bf 0`;
- output rate 30000/1001;
- bounded duration 24 seconds for this qualification.

PS2:

- exact P11 ELF SHA `5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`;
- 512 KiB queue;
- 448 KiB startup prefill;
- RGB16 / PSMCT16;
- P6 absolute clock;
- P9 timing instrumentation;
- no frame dropping.

Producer/archive evidence:

- 6,588,742 bytes;
- SHA `0883e79e1b81e5cae3de8e167ea80536a1caee623f2fd1e182eb4cb72c0a9f72`;
- CRC `7aeec0e7`;
- 720 coded pictures;
- producer elapsed 24.875855 s;
- 0.252595 MiB/s;
- about 2.20 Mbit/s average over the nominal 24 s source;
- 719 decoded/displayed;
- queue high-water exactly 524288 /524288;
- **feed waits 0/0/0**;
- **deadline misses 0**;
- **lateness 0**;
- decode average ~6.9723 ms;
- decode maximum ~23.6788 ms;
- upload average ~0.8243 ms;
- draw average ~0.0084 ms;
- serial average ~7.805 ms;
- average processing share ~23.39%;
- average nominal headroom ~25.56 ms.

Operator observation: true-live YouTube looked surprisingly good. A few subtle
lulls/catch-ups may have been seen, but there were **no PS2 feed waits or
presentation misses corresponding to them**. Those residual visual imperfections
therefore localize upstream of the qualified PS2 presentation scheduler, such as
browser/YouTube rendering, X11 capture cadence, or FFmpeg production cadence.

Important nuance: P12 used a 4M-target/5M-max VBR profile, but the actual live
content averaged only ~2.2 Mbit/s. P11 remains the harder ~3.64-Mbit/s average
compressed-bandwidth stress case.

The X11 capture log emitted a 16-bpp warning. Capture nevertheless produced all
720 coded pictures, software decode passed, and PS2 transport/decode integrity
passed. The warning should be investigated separately rather than treated as a
failed capture.

---

# 4. Optimization conclusions that future work must not lose

## 4.1 Capacity, target occupancy, and latency are different variables

The 512 KiB physical queue is strongly justified as an elasticity mechanism.
The 448 KiB startup prefill is **not** yet justified as a product latency target.

Approximate compressed-video time represented by 448 KiB:

- at P10's actual 3.636865 Mbit/s: roughly 1.0 second;
- at P12's ~2.20 Mbit/s actual average: roughly 1.67 seconds.

Those are large latencies for an interactive desktop. P11 changed both capacity
and prefill together, so the campaign has not yet separated how much success came
from larger emergency headroom versus deeper initial runway.

Product direction:

- retain a large physical queue capacity as a burst absorber;
- separately qualify the **smallest startup prefill** that remains stable;
- separately qualify a lower **steady-state occupancy target**;
- describe target video runway in milliseconds where practical rather than only
  bytes, using recent encoded rate as an estimator;
- let Pi-side credit/scheduling policy intentionally use less than truthful PS2
  physical capacity.

Large capacity should not automatically mean a permanently full queue.

## 4.2 Do not build a custom IOP/SIF networking path without evidence

The current results do not support the claim that a hard IOP→EE bandwidth limit
is the dominant MPEG problem:

- O1 accepted >3.4 MiB/s before decode;
- P11's 512 KiB queue nearly filled;
- P12's 512 KiB queue filled completely;
- Pi-side sender credit waits prove backpressure because data is available faster
  than the PS2 consumer currently asks for it;
- P12 had zero decoder starvation.

A custom IOP-side raw TCP/SIF-DMA producer may someday reduce CPU cost or jitter,
but it is not justified as a root-cause fix by current evidence.

## 4.3 Frame dropping is a sync/recovery tool, not the starvation fix

The prepared P7 drop experiment was intentionally not run after deeper buffering
was identified as a root-cause lever.

Future A/V synchronization may legitimately skip **presentation** of obsolete
video frames while continuing MPEG reference decoding. That is a different use:
keeping video aligned to a media clock after buffering is healthy.

Do not discard coded reference frames from decoder state merely because their
presentation deadline has passed.

## 4.4 Fixed post-frame waits are fundamentally the wrong scheduler

The P3/P4/P5 bracket proved that decode/upload work has variable duration. A
fixed one- or two-VSYNC delay after each picture transfers that variability into
visible cadence.

Retain an absolute monotonic presentation timeline. Future A/V integration should
replace the temporary source-frame-index clock with a proper shared media epoch,
not return to fixed sleeps.

## 4.5 Encoder shaping matters more than a nominal bitrate label

The current best qualified encoder shape is:

- MPEG-2 Main Profile / Main Level;
- yuv420p;
- 30000/1001 fps;
- 608x416 for current full-desktop scaled tests;
- target 4M;
- maxrate 5M;
- minrate 0;
- VBV 1,835,008 bits;
- GOP15;
- B-frames disabled.

A 4M target is not a constant 4M wire rate. P12 live video averaged ~2.2 Mbit/s.
That is desirable: static/simple desktop imagery should not consume an arbitrary
constant bitrate.

`-bf 0` is intentional during the low-latency/debug phase. B-frames can improve
compression but add decode reordering and presentation latency/complexity. They
should be reconsidered only after indefinite sessions and A/V timing are
qualified.

## 4.6 Resolution remains a quality/performance lever, not a permanent ceiling

608x416 is the best currently qualified full-desktop video geometry. It reduced
pixel/decode/memory/GS cost materially while maintaining useful apparent quality.

For region video mode, source rectangles smaller than this should generally not
be upscaled merely to hit 608x416. Encode near the region's natural size,
macroblock-align the encoded capture bounds, and map the decoded texture back to
the logical desktop rectangle.

Larger/fullscreen resolutions and higher bitrates can be explored later after
open-ended A/V latency is under control.

---

# 5. Product implementation direction from the evidence

The target product is not “replace VNC with MPEG.” The evidence supports a
hybrid thin-client architecture:

- RFB remains the ordinary interactive desktop path;
- audio remains continuous while media is active;
- MPEG-2 becomes an accelerated full-motion presentation path for one selected
  region or the whole desktop;
- the authoritative RFB desktop image remains intact underneath;
- local PS2 UI/cursor/OSK remain PS2-rendered overlays above either desktop or
  MPEG presentation;
- the Pi owns expensive inference/scheduling policy;
- the PS2 owns bounded queueing, decode, presentation, audio playback, and
  truthful telemetry.

This is consistent with the existing dedicated-link transport experiment and
clean-architecture ownership model.

## 5.1 Introduce one explicit media-session concept

Do not scatter MPEG start/stop state through RFB, display, audio, and controller
modules.

A media session should have explicit semantic state such as:

- `OFF`;
- `AUTO_REGION`;
- `MANUAL_REGION`;
- `FULLSCREEN`.

Pi-side policy/session state should own:

- session ID / generation;
- selected logical desktop rectangle;
- encoded capture rectangle and dimensions;
- video encoder process/lifecycle;
- audio capture source/lifecycle;
- shared media epoch/timebase;
- desired startup runway and steady queue targets;
- transport scheduling/priority policy;
- detector confidence/hysteresis for auto mode.

PS2 media mechanisms should own:

- bounded MPEG compressed queue;
- SMS decoder/IPU state;
- decoded-video presentation resources;
- video scheduling state;
- bounded audio queue/player state;
- audio-consumption clock;
- current media region presented on the GS;
- media-specific telemetry.

The application coordinator should orchestrate transitions between domains, not
implement private MPEG/audio/RFB mechanisms itself.

## 5.2 Open-ended MPEG sessions

P12 is bounded because qualification needed a deterministic terminal result.
The product needs an indefinite session whose temporary lack of input is not EOF.

Recommended initial session protocol semantics:

### `MEDIA_BEGIN`

Carries at minimum:

- version;
- session/generation ID;
- logical desktop rectangle `(x,y,w,h)`;
- encoded width/height;
- frame-rate/timebase;
- codec/profile contract;
- initial media epoch/timestamp;
- declared buffer-policy parameters or negotiated capabilities.

The Pi launches FFmpeg without `-t` and continues sending MPEG2 DATA indefinitely.

### active streaming

While the session is active:

- an empty MPEG queue means “wait/yield for more bytes,” not EOF;
- receiver credits continue to represent truthful queue capacity;
- the Pi scheduler chooses how much capacity to use as a latency/performance
  policy;
- video presentation follows media timestamps/deadlines rather than byte arrival;
- telemetry continues over a small control/telemetry channel.

### `MEDIA_END`

For a clean stop/reconfiguration:

1. stop the encoder at a controlled boundary;
2. provide the qualified MPEG sequence-end suffix where needed;
3. send an explicit `MEDIA_END(session_id, reason)`;
4. let SMS drain/return normally;
5. hold the last MPEG texture until the corresponding RFB region is ready;
6. atomically hand presentation ownership back to RFB;
7. retire decoder/session resources.

For the first product implementation, a rectangle/resolution change should be a
controlled end/new-session transition rather than attempting arbitrary in-stream
MPEG sequence-geometry switching.

Use session IDs so stale packets/control from an old session cannot mutate the
new one.

## 5.3 Audio: separate logical payload, shared media timeline first

The first integrated A/V version should **not** put audio inside the MPEG video
container merely to obtain synchronization.

Reasons:

- current SMS video path consumes a raw MPEG-2 video elementary stream;
- the project already has a proven logical AUDIO transport path;
- existing audio prototype semantics are 48 kHz signed 16-bit stereo PCM;
- muxing audio into MPEG Program Stream would require a new PS2 demux layer and a
  separately qualified audio codec/decode path;
- compressed A/V container integration would confound several independent
  variables at once.

Recommended first A/V product experiment:

- MPEG2 video on the MPEG2 logical channel;
- 48 kHz signed 16-bit stereo PCM on AUDIO initially;
- CONTROL messages for begin/end/region/timing;
- one shared media epoch and timestamps across both streams.

Audio compression can be studied later as its own bandwidth/CPU tradeoff.
A future combined MPEG system/program stream is optional, not required for good
A/V synchronization.

## 5.4 A/V synchronization and latency matching

The current P6 absolute frame-index scheduler was a necessary stepping stone.
Integrated media should use one shared capture/playback timeline.

Recommended first timing model:

1. The Pi chooses media epoch `T0` when starting the session.
2. Audio blocks are tagged by first-sample timestamp or absolute sample index
   relative to `T0`.
3. Video access units/pictures are assigned intended presentation timestamps
   relative to the same `T0`.
4. The PS2 audio player exposes consumed sample count.
5. Audio playback time is therefore concrete: `samples_played / 48000` plus the
   session epoch.
6. Video presentation compares each video timestamp against the audio-derived
   media clock plus a calibrated offset.

Audio should initially be the master clock because:

- audible underflow/discontinuity is highly noticeable;
- audio hardware consumption provides a continuous measurable timeline;
- video can be held or, when truly obsolete, presentation-skipped more safely
  than audio can be arbitrarily sped/slowed.

Video policy:

- if early: hold until due;
- if slightly late: display immediately;
- if severely obsolete after the pipeline is otherwise healthy: future policy
  may skip **presentation** of the late picture while still decoding required
  MPEG reference state;
- do not routinely stall audio to wait for video.

On catastrophic audio underflow/restart, re-establish the media epoch and rebuffer
both streams rather than silently accumulating A/V drift.

### Matching fixed latency

Measure the two paths independently:

Audio latency includes:

- Pi audio capture buffering;
- transport scheduling;
- PS2 audio receive queue;
- SPU2 submission/playback buffering.

Video latency includes:

- X11 capture;
- FFmpeg encode delay;
- compressed-video target runway;
- PS2 receive queue;
- MPEG decode;
- presentation scheduling.

The lower-latency path receives intentional delay so sound and picture align.
Do **not** make the current 448 KiB prefill a permanent sync offset. First reduce
and characterize the product video target latency.

A deterministic flash+beep source should be used to qualify A/V offset and drift.

## 5.5 Dirty-rectangle inference for automatic accelerated-video regions

Automatic video detection should live primarily on the Pi because it has more
CPU/memory and can observe RFB update metadata before deciding what to send.

Start with **one active accelerated region**, not arbitrary many regions.

The detector should use temporal evidence, not one rectangle. Useful signals:

- update frequency over a recent window;
- spatial overlap/persistence of dirty rectangles;
- stable union/bounding box;
- fraction of the rectangle updated repeatedly;
- total area threshold;
- changed-pixel density/entropy if later needed;
- rejection of tiny cursor/blink-like regions;
- hysteresis so the mode does not flap on/off.

A candidate region can be grown from repeated overlapping dirty rectangles and
then aligned outward to 16x16 MPEG macroblock boundaries, clamped to desktop
bounds.

Do not hard-code arbitrary detector durations as facts yet. Reasonable start/stop
windows are hypotheses to tune from real browser/video workloads.

The detector must continue seeing the **original dirty-rectangle metadata even
while RFB presentation for the active video region is suppressed**, otherwise it
cannot know whether motion continues or has stopped.

## 5.6 Presentation ownership: suppress the video region, not the RFB protocol

This is a critical architecture rule.

When MPEG owns a rectangle, do **not** discard arbitrary bytes from the ordered RFB
stream. The RFB parser must remain synchronized and the authoritative desktop
framebuffer must remain coherent.

Preferred PS2-side semantic model:

- RFB still parses every complete update/rectangle;
- validated RFB pixels still update the authoritative CPU desktop framebuffer;
- display/presentation has an explicit “media-owned region” mask/rectangle;
- normal desktop presentation is clipped/suppressed only where that region is
  currently owned by MPEG;
- MPEG texture is drawn into the logical desktop region;
- local PS2 cursor/OSK/UI are drawn after MPEG and remain visible.

An alternative Pi-side semantic RFB adapter may later clip/split complete RFB
rectangles before forwarding, but it must operate at rectangle semantics, never
by dropping arbitrary stream bytes. The detector must inspect unsuppressed
original updates before any such filtering.

### Entering video ownership

1. auto detector or manual action selects a region;
2. Pi begins video encoder/session for that region;
3. MPEG queue reaches the qualified startup target;
4. PS2 decodes a first usable picture;
5. RFB remains visible until that first MPEG frame is ready;
6. presentation ownership switches atomically to MPEG for that rectangle;
7. local overlays remain above it.

This avoids a black/stale hole during startup.

### Leaving video ownership

1. initiate controlled media stop;
2. hold the last decoded MPEG texture;
3. ensure a fresh RFB update/current authoritative desktop pixels exist for the
   region;
4. atomically redraw/release the region to RFB ownership;
5. only then discard the last media texture/session state.

This avoids revealing stale underlying pixels or a blank rectangle.

## 5.7 Manual override modes

Automatic inference must never be the only way to use accelerated video.
Provide explicit user overrides:

- `AUTO` — infer one active video region from dirty-rectangle behavior;
- `MANUAL_RECT` — user explicitly chooses a rectangle;
- `FULLSCREEN` — force the whole logical desktop through MPEG video mode;
- `OFF` / `FORCE_RFB` — prohibit media acceleration.

Manual selection can be exposed through the existing semantic input/UI model:

- controller hotkey enters a rectangle-selection overlay;
- controller moves/resizes corners or a selection box;
- UI emits a semantic “set video rectangle” action;
- application/media coordinator performs the transition.

The human-readable configuration may optionally allow an explicit rectangle,
but the runtime active rectangle remains live session state rather than desired
configuration authority.

The logical selected rectangle and the macroblock-aligned capture rectangle may
be different values. Capture can expand outward to safe 16x16 bounds while
presentation maps/clips back to the user's intended logical region.

## 5.8 Transport scheduling with RFB + AUDIO + MPEG2

Reuse the existing dedicated-link transport principles; do not create unrelated
per-feature networking architectures.

Logical priority intent:

1. CONTROL/input — tiny, reliable, latency-critical, never starved;
2. AUDIO — deadline-sensitive, regular service, bounded latency;
3. MPEG2 — real-time bulk, maintain enough runway without monopolizing the link;
4. RFB — reliable bulk for non-video regions, yields to urgent media/control;
5. TELEMETRY — small and low-priority.

Receiver credit reports truthful PS2 physical capacity. Pi-side policy may impose
a lower working ceiling. The fact that 512 KiB is free does not mean the Pi should
always fill all 512 KiB if product latency is better at a smaller target runway.

TCP remains acceptable as the first integrated backend. Current evidence does
not justify replacing it before combined-workload tests demonstrate a remaining
transport limitation.

## 5.9 Encoder policy for arbitrary regions

Current starting profile:

- codec: MPEG-2 video;
- Main Profile / Main Level;
- yuv420p;
- 30000/1001 fps;
- current maximum proven full-desktop scaled geometry: 608x416;
- target 4M;
- maxrate 5M;
- minrate0;
- VBV 1,835,008 bits;
- GOP15;
- B-frames0.

For a smaller video region:

- do not upscale needlessly;
- align encoded bounds to macroblocks;
- preserve the logical desktop region separately for placement/clipping;
- permit lower actual bitrate when the scene is simple;
- retain a bounded maxrate/VBV so compressed burst pressure remains predictable.

Quality/resolution can be pushed further only after indefinite streaming,
latency, and A/V synchronization are stable enough that higher bitrate is the
only meaningful variable.

---

# 6. What is still unproven

Do not infer these properties from P12:

- indefinite/open-ended MPEG session lifetime;
- clean start/stop/restart repeatedly without process or PS2 state leakage;
- long-duration clock drift;
- combined audio + MPEG playback;
- audio/video synchronization;
- final product latency;
- ideal video prefill/steady target;
- automatic dirty-rectangle video detection;
- RFB presentation suppression while preserving authoritative pixels;
- manual region/fullscreen switching;
- simultaneous RFB outside region + MPEG inside region + audio + input/control;
- local cursor/OSK overlay ordering over regional MPEG in integrated product;
- RGB16 subtle-gradient/color precision;
- B-frame behavior;
- higher resolutions/bitrates under product-shaped load;
- a need for UDP/raw Ethernet/custom IOP SIF DMA.

P12 is a major live-video qualification, not permission to collapse all remaining
work into one large integration patch.

---

# 7. Recommended next experimental sequence

Preserve causality. The safest high-value sequence is:

1. **Indefinite video-only session** — exact P12 encoder profile and P11 PS2
   playback mechanism, remove the 24-second producer duration, add an explicit
   controlled stop after a long operator-selected interval. First target: several
   minutes, then progressively longer soak runs. Measure queue occupancy,
   starvation, deadline drift, memory stability, and repeated session restart.
2. **Separate capacity from latency** — keep 512 KiB physical capacity while
   reducing startup prefill/steady-state target in isolated steps. Record actual
   end-to-end frame age as well as feed/deadline counters. The goal is a safe
   burst absorber with substantially less steady latency.
3. **Integrate existing audio mechanism using separate AUDIO + MPEG2 logical
   channels** while initially keeping video selection simple (fullscreen or one
   fixed manual rectangle). Preserve audio format first; do not combine audio
   codec replacement with transport/session integration.
4. **Qualify an audio-derived media clock** using consumed sample count and a
   shared Pi capture epoch.
5. **A/V sync benchmark** using deterministic visual flash + audible impulse.
   Measure initial offset, drift, underflow behavior, and resynchronization.
6. **Manual rectangle ownership** before automatic inference. Prove region
   capture, MPEG placement, RFB update/parsing continuity, presentation
   suppression, entry/exit restoration, and local overlay ordering.
7. **Automatic dirty-rectangle detector** on Pi after region ownership itself is
   reliable. Add hysteresis and telemetry before adding clever heuristics.
8. **Full combined stress**: controller/input, control, AUDIO, regional MPEG,
   RFB outside the region, local cursor/OSK/UI, repeated region start/stop.
9. Only after the above: investigate B-frames, higher quality/resolution,
   decoded-frame overlap/double buffering, alternate wire backend, or custom
   IOP/SIF transport optimizations if measured evidence warrants them.

The prepared P7 late-frame-drop experiment is historical apparatus, not the next
step.

---

# 8. The most important facts in one page

If only a short subset of this document survives, preserve these:

1. **Mature SMS libmpeg assembly is the hardware-qualified decoder.**
2. **RGB16 `_MPEG_Set16(1)` is qualified and halves decoded/output storage and GS
   texture-transfer size relative to RGB32.**
3. **The feed callback uses 2048-byte normal TO_IPU DMA and final 16-byte
   alignment; `dma_wait_fast()` is prohibited.**
4. **P4 proved raw 704x480 decode/upload/draw can run ~19% faster than the tested
   source's real-time requirement when fixed presentation waits are removed.**
5. **Fixed 0/1/2 VSYNC policies are not acceptable presentation clocks. Absolute
   deadlines are the right basis.**
6. **P7A was the huge breakthrough: 32→256 KiB +224 KiB prefill reduced 289
   deadline misses to 6 and max lateness from ~40 frames to 0.274 frame.**
7. **608x416 is the current preferred tested geometry.**
8. **The shaped Main/Main 2M stress reel achieved zero feed waits and zero timing
   misses.**
9. **P9 showed zero-starvation RGB16 decode averages ~7.3 ms and GS upload+draw
   <0.9 ms against a 33.37-ms frame period. The decoder is not near its limit.**
10. **P10's apparent 451-ms decode spike was really starvation: feed max436 ms.**
11. **P11 doubled capacity/prefill; feed max fell to5 ms and decode max to42.7 ms
    with the exact same 4M stress bitstream.**
12. **P12 true-live YouTube on the exact P11 PS2 path achieved zero feed waits,
    zero deadline misses, zero lateness, decode avg~6.97 ms/max~23.68 ms, and
    looked very good.**
13. **There is no current evidence that a hard IOP→EE/SIF bandwidth ceiling
    requires a custom IOP DMA networking redesign.**
14. **512 KiB physical capacity is useful; 448 KiB prefill is a proven stability
    point but not a proven product latency target.**
15. **Next architecture: indefinite media session + separate AUDIO/MPEG channels
    with shared epoch + audio-master sync + explicit region presentation
    ownership + manual/fullscreen override + later Pi dirty-rectangle inference.**

This campaign transformed MPEG from a speculative alternative rendering path
into a technically credible accelerated-media subsystem for PS-to-VNC. Future
work should preserve the causal lessons rather than merely preserving the final
numbers.
