# EXP3 P11 — 4 Mb/s deeper compressed-reservoir hardware result

## Purpose

P11 re-ran the exact P10 608x416 / 4 Mb/s MPEG-2 stress reel on the P9 RGB16 stage-timing harness while changing only compressed-input buffering:

- replay queue: 256 KiB -> 512 KiB;
- mandatory startup prefill: 224 KiB -> 448 KiB.

The decoder, RGB16 output path, GS upload/draw path, source bitstream, 30000/1001 absolute presentation clock, no-drop policy, and post-playback timing instrumentation were retained.

## Hardware identity

- P11 generator authority commit: `bc36d8f33c567d48500000502572463e847dead6`
- generated source blob: `007a439f6070e1b37e75c742cd551c290fe69b47`
- ELF SHA-256: `5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`
- ELF bytes: `2476164`
- unique PS2 path: `/mass/0/PS2VNC-exp3-p11-deep-4m-5f554624.ELF`
- rolling PS2 path: `/mass/0/PS2VNC-exp3-p11-deep-4m.ELF`

## Exact source

- source: exact P10 608x416 Main/Main MPEG-2 stress reel
- bytes: `10910594`
- SHA-256: `d98c21b50400236d5617b2bf42d612413909b79bff5887f507589060cb39321f`
- CRC32: `a2b0eb70`
- coded pictures: `720`
- fps: `30000/1001`
- target bitrate: `4000000`
- max bitrate: `5000000`
- output: `RGB16 / PSMCT16`

## Operator observation

Playback looked great and remained visually smooth. Terminal sequence was PURPLE then CYAN with the last frame left displayed.

## PS2 timing

`EXP3_P9_TIMING fps_num=30000 fps_den=1001 frame_period_ticks=4920115 vsync_waits=1374 deadline_misses=39 max_deadline_late_ticks=13242317 prefill_target=458752 prefill_observed=458752 decode_n=720 decode_total=920521728 decode_max=6299904 upload_n=719 upload_total=87437824 upload_max=125184 draw_n=719 draw_total=894208 draw_max=1792`

Derived against a 33.366667 ms frame period:

- deadline misses: `39`
- maximum deadline lateness: about `89.805 ms` (`2.691` frame periods)
- MPEG_Picture average: about `8.670 ms`
- MPEG_Picture maximum: about `42.724 ms`
- GS upload average: about `0.825 ms`
- GS upload maximum: about `0.849 ms`
- GS draw average: about `0.0084 ms`
- GS draw maximum: about `0.0122 ms`
- serialized decode + upload + draw average indicator: about `9.504 ms`
- average processing share of frame period: about `28.5%`
- average frame-time headroom: about `23.863 ms`

The decode timing remains wall-clock time around `MPEG_Picture()` and can include feed-callback waiting; it is not pure IPU execution time.

## Transport / replay result

- queue capacity: `524288`
- queue high-water: `522240`
- queue current at result: `0`
- feed-wait events: `332`
- feed-wait loops: `348`
- maximum feed-wait loops: `5`
- payload bytes: `10910610` including explicit sequence-end suffix
- send elapsed: `26.602914 s`
- producer coded pictures: `720`
- pictures decoded: `719`
- pictures displayed: `719`
- decoder return delta: `1`
- receiver done: `1`
- stream integrity: PASS
- result contract: PASS
- generic sequence-end replay: PASS

## P10 -> P11 comparison

P10 used the same bitstream and playback path with a 256 KiB queue / 224 KiB prefill. P10 measured:

- feed waits: `409 events / 955 loops / 436 max loops`;
- deadline misses: `56`;
- max lateness: about `434.506 ms`;
- MPEG_Picture average: about `9.587 ms`;
- MPEG_Picture max: about `451.017 ms`.

P11 therefore produced:

- feed-wait events: `409 -> 332` (about `18.8%` lower);
- feed-wait loops: `955 -> 348` (about `63.6%` lower);
- max feed-wait span: `436 -> 5` loops (about `98.9%` lower);
- deadline misses: `56 -> 39` (about `30.4%` lower);
- maximum lateness: `434.506 -> 89.805 ms` (about `79.3%` lower);
- MPEG_Picture max: `451.017 -> 42.724 ms` (about `90.5%` lower).

The event count alone is misleading: P11's remaining feed waits are overwhelmingly shallow. Average loops per feed-wait event fell from about `2.34` in P10 to about `1.05` in P11, and the single worst feed starvation span collapsed from roughly 436 ms to roughly 5 ms.

## Interpretation

P11 strongly supports the hypothesis that P10's pathological ~451 ms `MPEG_Picture()` call was dominated by compressed-input starvation rather than intrinsic MPEG/IPU decode cost. Doubling only the compressed reservoir removed nearly all of the pathological feed-wait span and reduced the maximum measured decode-call wall time by about 90% without changing the decoder or source stream.

The 512 KiB queue also reached `522240 / 524288` bytes high-water and the Pi sender spent substantial time waiting for credit. This is evidence that the upstream transport can deliver bursts faster than the paced consumer drains them. The current evidence therefore supports `adequate aggregate throughput + bursty delivery + insufficient elasticity` much more strongly than a hard IOP-to-EE bandwidth ceiling at this 4 Mb/s operating point.

P11 does not eliminate all presentation misses. With maximum feed waiting now only about 5 ms, the remaining 39 misses and ~89.8 ms maximum lateness should not be attributed automatically to network starvation. They may reflect ordinary decode-complexity variation, accumulated scheduling phase, or another downstream timing effect. A per-picture timing/feed correlation would be needed to localize those residual misses.

## Result

P11 is a qualified, visually excellent 608x416 / 4 Mb/s / RGB16 operating point with exact transport integrity and dramatically reduced compressed-input starvation. Preserve this result before further bitrate, buffering, or decoder experiments.
