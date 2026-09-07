# O3 Live Performance P1 — 3M Hardware Result

## Synopsis

Records the first live-video performance follow-up after O3 true-live correctness was proven. P1 changed only the Pi ffmpeg nominal MPEG-2 bitrate from 4M to 3M. The PS2 ELF, 32768-byte queue, receiver stack/priority, 704x480 output geometry, 30000/1001 capture rate, GOP 15, B-frames disabled, and the hardware-qualified explicit MPEG sequence-end suffix remained unchanged.

## Hardware observation

- Source: playing YouTube video visible on Pi X11 and Windows TigerVNC.
- The PS2 played the complete live stream.
- The first approximately 2–3 seconds appeared relatively smooth.
- The remainder remained visibly clunky.
- Terminal sequence: PURPLE -> CYAN -> final frame held.
- Functional correctness: PASS.
- Real-time performance: FAIL.

## Evidence run

`build/evidence/exp3-network-live-3m-q32768-20260907-061659`

Runtime identity:

- Record authority before this result: `bb6bf11f8482f88faf74c0198ada9e02201673c0`
- PS2 ELF: `/mass/0/PS2VNC-exp3-sms-network-live-streaming-3dab7e30.ELF`
- bitrate: 3M
- queue capacity: 32768
- receiver stack: 16384
- receiver priority: 63
- requested duration: 20 s

## Pi/live telemetry

- frames sent: 832
- payload bytes: 6,809,062
- payload SHA256: `1dd87fc4235b6cebac7f43c07bbb37004af785dd6308a1146d99d42203934274`
- CRC32: `c67cf489`
- picture starts: 520
- sequence headers: 35
- sequence ends: 1
- producer elapsed: 26.109300 s
- actual payload rate: 0.248709 MiB/s
- credit wait events: 828
- credit wait seconds: 25.061232 s
- maximum single credit wait: 1.479908 s
- result wait: 2.169274 s

Derived from the exact payload and requested 20-second source duration:

- payload rate required for real time: 0.324681 MiB/s
- measured pipeline rate: 0.248709 MiB/s
- real-time speed ratio: 0.766011
- credit-wait fraction: 0.959858 (95.99%)

## PS2 telemetry

- queue capacity: 32768
- queue high-water: 32768
- queue current at terminal: 0
- frames received: 832
- bytes received / consumed: 6,809,062 / 6,809,062
- feed wait events: 501
- feed wait loops: 4624
- feed wait max loops: 52
- pictures decoded / displayed: 519 / 519
- feed calls: 3325
- payload bytes submitted: 6,809,062
- DMA bytes submitted: 6,809,072
- receiver done: 1
- integrity pass: 1
- END received: 1
- producer sequence ends: 1
- decoder return delta: 1
- result contract: PASS
- Pi archive software decode: PASS

## Comparison to the 4M true-live run

4M true-live run:

- payload bytes: 7,017,283
- producer elapsed: 26.741170 s
- actual payload rate: 0.250258 MiB/s
- required real-time payload rate: 0.334610 MiB/s
- credit wait: 25.693999 s

3M P1:

- payload bytes: 6,809,062
- producer elapsed: 26.109300 s
- actual payload rate: 0.248709 MiB/s
- required real-time payload rate: 0.324681 MiB/s
- credit wait: 25.061232 s

Changing the nominal bitrate from 4M to 3M reduced total payload by only about 2.97% and producer elapsed by about 2.36%. It did not materially change the measured sustained pipeline rate or the approximately 96% credit-wait fraction.

## Interpretation

P1 does not support the hypothesis that lowering the nominal ffmpeg bitrate from 4M to 3M is sufficient to restore real-time playback. The live producer remains almost continuously backpressured by receiver credit.

The observation that only the first 2–3 seconds looked relatively smooth should not by itself be attributed to the 32768-byte initial queue reserve. At the measured 3M-run rate deficit, 32768 bytes accounts for only roughly 0.4 seconds of buffering. A larger-queue experiment is therefore not yet the cleanest next discriminator.

The next controlled test should remove PS2/network backpressure while preserving the same X11 -> ffmpeg capture/encode path, using the same 3M settings and a nonblocking/local sink. That distinguishes whether the Pi capture/encoder can itself sustain a 20-second source in approximately 20 seconds or whether the slowdown already exists before the transport/decoder path.
