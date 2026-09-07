# O3 P8 RGB16 hardware result

## Scope

P8 changed only the SMS/IPU decoded-picture and GS texture path from RGB32/PSMCT32 to the mature SMS RGB16/PSMCT16 mode via `_MPEG_Set16(1)`. The deterministic 24-second 608x416 Main/Main 2 Mb/s VBV stress reel, 256 KiB compressed ring, 224 KiB prefill, P6 absolute presentation clock, no-drop policy, and network/feed path were unchanged.

## Device under test

- EXP3 commit before this result record: `9a192be069e47126cecd60c293db5a008795165d`
- ELF SHA256: `ca6ad330ccb9925dee5423d3349316e919597156f2a1fcb1b444f4d4fbd2e023`
- ELF bytes: `2471704`
- Generated source blob: `26ab27a361961b350e549f8ac28027053b41ebc3`
- Remote: `/mass/0/PS2VNC-exp3-p8-rgb16.ELF`

## Exact source

- Stress reel bytes: `6209147`
- SHA256: `6bcfb93badf653a3d72de83e7d9253d01fe0027283f171954c9de6cefbf40aca`
- CRC32: `4f946a9a`
- Coded pictures: `720`
- Resolution: `608x416`
- FPS: `30000/1001`

## Hardware result

Operator observation:

- Motion looked just as good as the RGB32 benchmark.
- No obvious color difference was visible on this particular stress reel; the reel is not a strong color-precision test, so dedicated gradient/subtle-color qualification remains required.

Clock telemetry:

- `vsync_waits=1436`
- `deadline_misses=0`
- `max_deadline_late_ticks=0`
- `prefill_target=229376`
- `prefill_observed=229376`

Feed / transport telemetry:

- queue capacity `262144`
- queue high-water `260096`
- feed wait events `0`
- feed wait loops `0`
- feed wait max loops `0`
- integrity pass `1`
- receiver done `1`
- end received `1`
- coded pictures `720`
- SMS returned/decoded pictures `718`
- displayed pictures `718`
- decoder-return delta `2`

The persistent `720 coded -> 718 returned` relationship is the already-separated SMS sequence-end / returned-picture-count behavior. It is not transport corruption and is unrelated to RGB16 performance.

## A/B against RGB32 stress benchmark

Both RGB32 and RGB16 runs had:

- zero feed starvation;
- zero deadline misses;
- zero measured lateness;
- `718` decoded/displayed pictures from `720` coded pictures;
- visually smooth playback.

RGB16 changes the decoded/output storage contract from 4 bytes/pixel and 1024-byte / 64-qword macroblocks to 2 bytes/pixel and 512-byte / 32-qword macroblocks. At this conservative 2 Mb/s benchmark both paths are already inside the real-time envelope, so the hardware result proves RGB16 retains correctness and smoothness but does not by itself quantify the additional headroom.

## Classification

**P8 RGB16: HARDWARE FUNCTIONAL PASS / PERFORMANCE PASS.**

Provisionally retain RGB16 as the preferred performance path for subsequent stress, timing, and quality-headroom experiments. Dedicated color-precision testing is still required before making a final product-quality decision.
