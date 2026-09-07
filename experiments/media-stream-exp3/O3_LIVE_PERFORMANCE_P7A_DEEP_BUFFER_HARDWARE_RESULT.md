# EXP3 O3 P7A Deep-Buffer + Prefill Hardware Result

## Result

P7A is hardware-qualified as the best playback result in the P3-P7A performance series so far.

The only intentional playback-path change from P6/P6B was compressed-input buffering:

- queue capacity: 32 KiB -> 256 KiB
- mandatory prefill before first MPEG picture: 224 KiB
- source: exact P2 704x480 / 30000/1001 / 600-coded-picture archive
- presentation: P6 absolute clock, no frame dropping
- decoder, IPU feed, GS upload/draw, source bytes, queue credit protocol: otherwise unchanged

## Exact DUT / source identity

- branch: `experiment/media-stream-exp3`
- P7A generator / staging authority: `14541b5d93945be0b03a304d22f2781c12fb9b43`
- generated source blob: `f9bfcb3c9a967a51f4f09b04175faae136c25f7b`
- ELF SHA256: `efbf929323423757043dd195f9a7ee1471c18797085dc0a5ed108a54da59be20`
- ELF bytes: `2471776`
- run: `build/evidence/exp3-p7a-deep-buffer-prefill-20260907-081809`
- source bytes: `7732149`
- source SHA256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- source CRC32: `ce1e9d03`
- coded pictures: `600`
- returned/decoded/displayed pictures: `598 / 598 / 598`

## Operator observation

> that was our very best test yet

Visual smoothness improved substantially relative to P6/P6B.

## Clock result

P7A terminal clock telemetry:

- source FPS: `30000/1001`
- frame period ticks: `4920115`
- VSYNC waits: `1181`
- deadline misses: `6`
- max deadline lateness ticks: `1349120`
- max lateness: `0.274205` source-frame periods
- miss rate vs 598 returned pictures: `1.00%`
- prefill target: `229376` bytes
- prefill observed: `229376` bytes

Relative to P6B with the same source and absolute-clock no-drop presentation policy:

- deadline misses: `289 -> 6` (`97.92%` reduction)
- max lateness: `39.967649 -> 0.274205` frame periods (`99.31%` reduction)

## Transport / feed result

- queue capacity: `262144`
- queue high water: `260096`
- feed wait events: `7`
- feed wait loops: `7`
- max feed wait loops: `1`
- bytes received/consumed: `7732165 / 7732165`
- CRC32: `3edf1ca9`
- receiver done: `1`
- integrity pass: `1`
- end received: `1`

Relative to P6B 32 KiB buffering:

- feed wait events: `512 -> 7`
- feed wait loops: `5757 -> 7`
- max feed wait loops: `1213 -> 1`

## Interpretation

The prior 32 KiB compressed-byte reservoir was a dominant cause of playback starvation and the resulting fast/slow catch-up behavior. A 256 KiB queue with 224 KiB prefill almost eliminates decoder input starvation while preserving the exact decoder, GS path, source, and P6 absolute presentation clock.

This result rejects the hypothesis that the PS2's MPEG/IPU/GS path is fundamentally incapable of sustaining this 704x480 stream. With sufficient compressed input runway, the same path stays extremely close to the 29.97-fps clock.

The remaining six deadline misses are all less than one third of a source-frame period late. They are no longer consistent with the multi-frame starvation pathology seen in P6B and should be treated as a smaller downstream cadence/processing issue.

## Next experiment direction

Do not enable late-frame dropping yet and do not lower quality merely to hide starvation.

The compressed-buffer lever is now sufficiently qualified. The next clean performance lever should change picture cost while retaining the P7A deep-buffer/prefill baseline, so decoder/memory/GS work can be characterized independently of network starvation. A smaller macroblock-aligned MPEG-2 source replayed through the same P7A ELF is preferred because the current harness already accepts sequence dimensions up to 704x480.

The repeatable `600 coded -> 598 SMS returned` terminal-count behavior remains a separate EOF/reference-picture characterization and does not invalidate transport integrity.
