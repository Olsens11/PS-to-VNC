# EXP3 P9 RGB16 Stage-Timing Hardware Result

## Identity

- Experiment branch: `experiment/media-stream-exp3`
- P9 generator authority: `ece6fd08c5d1d7b19594e3852212cca4639e2d4e`
- Generated P9 source blob: `5567605e2068596cb943911c2c4fa20de9b9a056`
- P9 ELF SHA256: `cea181e37c89990e76d99d22a6fae65bd3a7c92cbea3bcc04e4fd9128d635d83`
- P9 ELF bytes: `2476288`
- Evidence run: `build/evidence/exp3-p9-stage-timing-20260907-092230`

## Fixed benchmark

The exact same deterministic 24-second stress reel used by the qualified RGB16 P8 run was replayed:

- 608x416
- 30000/1001 fps
- MPEG-2 Main/Main
- 2 Mb/s target, bounded VBV source
- source SHA256 `6bcfb93badf653a3d72de83e7d9253d01fe0027283f171954c9de6cefbf40aca`
- 720 coded MPEG picture starts
- 256 KiB compressed input queue
- 224 KiB mandatory prefill
- P6 absolute presentation clock
- RGB16 / PSMCT16 SMS/IPU output
- no frame dropping

P9 changed playback behavior only by adding timer reads and local timing accumulation around the already-qualified P8 stages. The timing datagram was transmitted only after playback ended.

## Operator observation

The playback remained very smooth, with no visible difference from P8. The instrumentation did not produce an observable playback perturbation.

## Transport and presentation result

- queue high water: 260096 bytes
- feed wait events: 0
- feed wait loops: 0
- feed wait max loops: 0
- deadline misses: 0
- maximum presentation lateness: 0
- returned MPEG pictures: 718
- displayed pictures: 718
- coded picture starts: 720
- decoder return delta: 2
- receiver done: 1
- transport integrity: PASS

The coded-minus-two terminal behavior remains the separate known SMS EOF/reference-picture characterization and is not transport corruption.

## Stage timing

Frame period at 30000/1001 fps: **33.366667 ms**.

### `MPEG_Picture()`

Timing samples: 719. This includes the first call with sequence/setup work and the final call that returns zero at EOF, so the arithmetic mean is a conservative call-level indicator rather than an exact ordinary-returned-picture mean.

- total ticks: 775752192
- average: **7.316977 ms**
- maximum: **19.118056 ms**
- average fraction of frame budget: **21.93%**
- maximum fraction of frame budget: **57.30%**

### RGB16 GS texture upload

718 samples, exactly one for each returned/displayed picture.

- total ticks: 87454208
- average: **0.826026 ms**
- maximum: **0.850694 ms**
- average fraction of frame budget: **2.48%**
- maximum fraction of frame budget: **2.55%**

### GS draw

718 samples, exactly one for each displayed picture.

- total ticks: 887296
- average: **0.008381 ms**
- maximum: **0.013889 ms**
- average fraction of frame budget: **0.03%**
- maximum fraction of frame budget: **0.04%**

### Serial processing indicator

Using the measured call-level decode mean plus one successful-picture RGB16 upload and draw mean:

- average decode + upload + draw indicator: **8.151384 ms**
- remaining nominal 33.366667 ms frame period after this indicator: about **25.215 ms**

Because the decode mean includes first-call setup and the terminal zero-return call, this should not be interpreted as a precise ordinary-frame critical-path average. It is nevertheless strong evidence that the current 608x416 / 2 Mb/s benchmark is well inside the PS2 processing envelope.

## Conclusion

**P9 PASS.**

The hardware evidence rejects the hypothesis that the PS2 MPEG decoder is near its real-time limit at the current operating point. With network starvation removed, `MPEG_Picture()` averages roughly 7.3 ms and never exceeds roughly 19.1 ms during this stress reel, while RGB16 GS upload plus draw consumes under 0.9 ms even at its individual maxima.

The next encoder-quality experiments may therefore spend substantial processing/bandwidth margin before decoder throughput itself becomes the primary concern. Decoder/output tuning remains worthwhile, but the present evidence does not justify lowering quality merely to protect decoder headroom.
