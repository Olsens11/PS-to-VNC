# O3 live performance P5 — one presentation VSYNC hardware result

## Scope

P5 is a one-variable timing experiment derived from the hardware-qualified O3 live MPEG harness. It changes only the per-picture presentation pacing from two `graph_wait_vsync()` calls to one. Decode, SMS/libmpeg, IPU feed, GS upload/draw, network transport, queue capacity, source archive, and explicit sequence-end handling remain otherwise unchanged.

## Source / build identity

- Experiment branch: `experiment/media-stream-exp3`
- P5 apparatus commit: `f3ce759e68a28b6df6a5d596e72e5036df5052ad`
- Generated source blob: `fe9a56b269bc5dbf9d9f4a29ebf6da61228b6ca5`
- P5 ELF SHA-256: `efd680caa9d207754fd2dbcfc2efe731353c66d1898948421f227b190893484c`
- P5 ELF bytes: `2460984`
- Archival deployment: `/mass/0/PS2VNC-exp3-p5-one-present-vsync-efd680ca.ELF`
- Rolling experiment deployment: `/mass/0/PS2VNC-exp3-p5-one-vsync.ELF`

## Fixed replay source

Exact P2 local 3M capture archive:

- bytes: `7732149`
- SHA-256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- CRC32: `ce1e9d03`
- MPEG picture starts: `600`
- sequence headers: `40`
- replay suffix: literal `00 00 01 B7` plus twelve zero bytes
- wire payload bytes: `7732165`
- queue capacity: `32768`

## Hardware observation

Operator observation:

> it was as if it was shooting forward then slowing down constantly

The video completed. The fixed one-VSYNC policy did not produce stable presentation cadence.

Classification:

- functional completion: YES
- average throughput near real-time: YES
- stable presentation cadence: NO
- fixed one-VSYNC policy acceptable: NO

## Telemetry

Replay sender:

- frames: `944`
- payload bytes: `7732165`
- elapsed: `19.207771 s`
- sustained payload rate: `0.383905 MiB/s`
- required rate for the nominal 20-second source: `0.368698 MiB/s`
- rate / real-time requirement: `1.041245`
- credit wait: `19.125172 s` (`99.57%` of replay send interval)

PS2 result:

- queue capacity: `32768`
- queue current at end: `0`
- queue high water: `30720`
- bytes received: `7732165`
- bytes consumed: `7732165`
- CRC32: `3edf1ca9`
- feed wait events: `527`
- feed wait loops: `5063`
- feed wait max loops: `50`
- pictures decoded: `598`
- pictures displayed: `598`
- receiver done: `1`
- integrity pass: `1`
- END received: `1`
- producer picture starts: `600`
- decoder return delta: `2`

## P3 / P4 / P5 bracket

Same fixed source and queue:

| Experiment | Per-picture VSYNC waits | Replay seconds | MiB/s | Visual behavior |
|---|---:|---:|---:|---|
| P3 | 2 | 27.827575 | 0.264988 | slower, still somewhat clunky |
| P4 | 0 | 16.747557 | 0.440301 | noticeably fast / fast-forward-like, choppy |
| P5 | 1 | 19.207771 | 0.383905 | repeatedly speeds up then slows down |

P5 is `1.448766x` P3 throughput. P4 is `1.146900x` P5 throughput.

## Interpretation

The timing bracket is now hardware-established:

1. The PS2 decode/upload/draw path can exceed the source's real-time byte requirement when presentation waits are removed (P4).
2. Two fixed VSYNC waits per picture over-throttle the path (P3).
3. One fixed VSYNC wait gives an average transport rate close to real-time (P5) but does not stabilize instantaneous presentation cadence.

Therefore the remaining problem is not simply raw decode capacity or a choice of fixed `N` VSYNC waits. Presentation is currently serialized as decode -> upload -> draw -> fixed wait -> next decode, so decode/network variability directly modulates when frames appear. The next timing experiment should schedule presentation against an absolute monotonic video clock / picture deadlines rather than adding another fixed delay after each frame.

## Separate EOF count issue

The exact fixed source repeatably produces:

- coded picture starts: `600`
- SMS returned/displayed: `598`
- delta: `2`
- transport/integrity: PASS

This causes the current O3 terminal classifier to show RED before CYAN. It is a separate returned-picture / sequence-end characterization issue and should not be conflated with presentation timing or stream corruption.
