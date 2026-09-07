# EXP3 P10 — 608x416 4 Mb/s Quality Hardware Result

## Purpose

P10 increased only the MPEG-2 source bitrate/quality relative to the P9 608x416 RGB16 timing baseline. The same P9 ELF, SMS RGB16 path, 256 KiB compressed queue, 224 KiB prefill, absolute 30000/1001 presentation clock, no-drop policy, and deterministic stress-reel content were retained.

## Source identity

- Stress-reel content: exact same deterministic 24 second source used by P9
- Resolution: 608x416
- Frame rate: 30000/1001
- MPEG-2 Main Profile / Main Level
- YUV420P
- GOP: 15
- B-frames: 0
- Target bitrate: 4,000,000 bit/s
- Maximum bitrate: 5,000,000 bit/s
- VBV buffer: 1,835,008 bits
- Encoded bytes: 10,910,594
- Encoded SHA256: `d98c21b50400236d5617b2bf42d612413909b79bff5887f507589060cb39321f`
- Encoded CRC32: `a2b0eb70`
- Coded pictures: 720
- Actual average payload rate over 24 seconds: 3.636865 Mbit/s

## PS2 executable identity

P10 reused the exact P9 timing ELF:

- SHA256: `cea181e37c89990e76d99d22a6fae65bd3a7c92cbea3bcc04e4fd9128d635d83`
- RGB16 / PSMCT16 output
- 512-byte / 32-QW macroblocks
- 256 KiB compressed queue
- 224 KiB mandatory prefill
- P6 absolute presentation clock
- no frame dropping
- P9 post-playback stage timing instrumentation

## Operator observation

- Motion: very smooth
- Image quality: great
- No visible motion regression relative to P9 2 Mb/s baseline

## Timing result

Frame period: 33.366667 ms.

### MPEG_Picture()

- Timing samples: 720
- Average: 9.586989 ms
- Maximum: 451.017379 ms
- Average frame-budget share: 28.73%

### GS upload

- Samples: 719
- Average: 0.824846 ms
- Maximum: 0.847222 ms

### GS draw

- Samples: 719
- Average: 0.008367 ms
- Maximum: 0.010417 ms

### Serialized stage indicator

- Average decode + upload + draw: 10.420202 ms
- Average frame-budget share: 31.23%
- Average nominal headroom: 22.946465 ms

## Presentation / feed result

- Deadline misses: 56
- Maximum deadline lateness: 434.505925 ms
- Feed wait events: 409
- Feed wait loops: 955
- Maximum feed wait loops: 436
- Queue capacity: 262,144 bytes
- Queue high-water mark: 260,096 bytes
- Prefill target: 229,376 bytes
- Prefill observed: 229,376 bytes

Transport remained exact:

- error: 0
- receiver done: 1
- integrity pass: 1
- terminal metadata received: 1
- bytes received/consumed matched producer metadata

EOF characterization changed from the P9 stress-reel result:

- coded pictures: 720
- returned/displayed: 719
- coded-minus-returned: 1

## Interpretation

The 4 Mb/s quality increase is visually successful, but P10 is **not** a clean decoder-cost measurement at its worst case because `MPEG_Picture()` includes compressed-input waits through the feed callback.

The strongest evidence is the near-direct correspondence between:

- maximum feed wait loops: 436 (approximately 436 ms at the 1 ms feed-wait loop cadence), and
- maximum measured `MPEG_Picture()` duration: 451.017 ms.

Therefore the 451 ms maximum must not be interpreted as intrinsic IPU/libmpeg decode cost. It is predominantly an input-starvation event observed while inside the decoder call.

The average `MPEG_Picture()` time also rose from P9's 7.316977 ms to 9.586989 ms, but because P10 experienced 409 feed-wait events while P9 experienced none, the 31.02% average increase mixes actual higher-bitstream decode/parsing cost with waiting for compressed input. A deeper-buffer rerun at the exact same 4 Mb/s source is required before attributing the increase to decoder work.

The GS path is effectively unchanged:

- P9 upload average: 0.826026 ms
- P10 upload average: 0.824846 ms
- P9 draw average: 0.008381 ms
- P10 draw average: 0.008367 ms

This confirms the bitrate increase did not alter the RGB16 GS workload.

## Decision

Do **not** advance to 6 Mb/s yet.

First rerun the exact P10 4 Mb/s stream with a deeper compressed reservoir and proportionally deeper startup prefill. The next experiment should keep source, decoder, RGB16 output, GS path, presentation clock, and instrumentation unchanged while increasing only compressed-buffer policy.

Recommended next isolated buffer policy:

- queue: 512 KiB
- mandatory prefill: 448 KiB

At the P10 actual average rate, 448 KiB represents roughly one second of compressed data. Even at the configured 5 Mb/s peak ceiling it represents roughly 0.73 seconds, which is comfortably larger than the approximately 0.44 second worst starvation span observed here.

If that rerun collapses feed waits and deadline misses while preserving the P10 image quality, P10's visual quality can be retained and the true 4 Mb/s decoder cost can then be read from the P9 stage timing instrumentation without starvation contamination.
