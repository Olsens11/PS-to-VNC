# EXP3 P12 true-live 608x416 4M hardware result

## Result summary

P12 changed only the producer source from fixed archive replay to true live X11 capture while preserving the qualified P11 playback path.

Hardware/operator observation:
- YouTube true-live playback looked surprisingly good.
- A few lulls/catch-ups were seen, but the operator was unsure whether they originated in the Pi/browser rendering itself.
- No obvious PS2-side quality failure was observed.

## Exact playback contract

- PS2 ELF SHA256: `5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`
- PS2 remote: `/mass/0/PS2VNC-exp3-p11-deep-4m.ELF`
- RGB16 / PSMCT16 retained
- Queue capacity: 524288 bytes
- Startup prefill: 458752 bytes
- Presentation: absolute 30000/1001 clock, no frame dropping
- P9 stage timing retained

## Live producer contract

- P12 wrapper commit: `8c9bca2549c3d99fd7b17030487015d5835f4893`
- Wrapper blob: `339498b14d4eb83a561a5930408c939038eab88e`
- X11 input: 704x462 at 30000/1001
- MPEG-2 output: 608x416 yuv420p
- Main Profile / Main Level
- target bitrate: 4,000,000 bit/s
- max bitrate: 5,000,000 bit/s
- VBV: 1,835,008 bits
- GOP: 15
- B frames: 0
- duration: 24 seconds
- terminal suffix: sequence end + 12 zero bytes, 16 bytes total

## Producer / transport evidence

- payload bytes: 6,588,742
- frames sent: 805
- coded picture starts: 720
- sequence headers: 48
- sequence ends: 1
- producer elapsed: 24.875855 s
- payload rate: 0.252595 MiB/s
- SHA256: `0883e79e1b81e5cae3de8e167ea80536a1caee623f2fd1e182eb4cb72c0a9f72`
- CRC32: `7aeec0e7`
- queue high water: 524288 / 524288 bytes
- feed-wait events: 0
- feed-wait loops: 0
- feed-wait max loops: 0
- decoded pictures: 719
- displayed pictures: 719
- producer coded pictures: 720
- decoder return delta: 1
- integrity pass: yes
- end received: yes
- result contract: PASS
- archive Pi decode: PASS
- overall Pi evidence: PASS

## PS2 timing evidence

Telemetry:

`EXP3_P9_TIMING fps_num=30000 fps_den=1001 frame_period_ticks=4920115 vsync_waits=1438 deadline_misses=0 max_deadline_late_ticks=0 prefill_target=458752 prefill_observed=458752 decode_n=720 decode_total=740235520 decode_max=3491584 upload_n=719 upload_total=87392512 upload_max=124672 draw_n=719 draw_total=890112 draw_max=1280`

Derived values:
- frame period: ~33.3667 ms
- deadline misses: 0
- maximum deadline lateness: 0 ms
- decode-call average: ~6.9723 ms
- decode-call maximum: ~23.6788 ms
- RGB16 GS upload average: ~0.8243 ms
- RGB16 GS upload maximum: ~0.8455 ms
- GS draw average: ~0.00840 ms
- GS draw maximum: ~0.00868 ms
- serialized decode+upload+draw average indicator: ~7.8050 ms
- average processing share of frame period: ~23.39%
- average frame headroom: ~25.5617 ms

## Interpretation

P12 is the strongest true-live MPEG result so far.

The PS2 playback side shows no evidence corresponding to the operator-observed occasional lulls/catch-ups:
- zero compressed-feed waits,
- zero presentation deadline misses,
- zero measured deadline lateness,
- healthy decoder and GS timing.

This strongly localizes any residual visible lulls upstream of the qualified PS2 playback path, such as browser/YouTube rendering, X11 capture scheduling, or live FFmpeg production cadence.

The 512 KiB EE-side reservoir reached full capacity while the PS2 decoder never starved, further supporting the conclusion that the existing network/SIF/EE path has adequate aggregate throughput at this operating point.

The archive contains exactly 720 coded pictures and software-decodes successfully. The PS2 returned/displayed 719 pictures, yielding the familiar qualified coded-minus-one terminal relationship.

## FFmpeg note

The X11 capture log emitted:

`16 bits per pixel screen is not implemented. Update your FFmpeg version to the newest one from Git. If the problem still occurs, it means that your file has a feature which has not been implemented.`

Despite this warning, capture completed with 720 coded pictures, the archived stream software-decoded successfully, and the PS2 transport/decode/integrity contract passed. The warning should nevertheless be investigated separately because the Pi X server is 16-bit and it may be relevant to upstream rendering/capture behavior.
