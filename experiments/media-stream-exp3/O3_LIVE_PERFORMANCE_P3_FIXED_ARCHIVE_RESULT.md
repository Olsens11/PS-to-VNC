# EXP3 O3 Performance P3 — Fixed P2 Archive Replay

## Purpose

Replay the exact P2 local X11/ffmpeg archive through the unchanged O3 PS2 transport/decoder/presentation path. This removes live capture/encode timing while preserving the exact encoded content, 32 KiB queue, SMS decoder, IPU feed, GS upload/draw path, and PS2 ELF.

## Source authority

- P2 archive: `build/evidence/exp3-local-x11-3m-control-20260907-062342/local-stream.m2v`
- bytes: `7732149`
- SHA-256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- CRC32: `ce1e9d03`
- coded pictures: `600`
- sequence headers: `40`
- original sequence ends: `0`

The generic replay apparatus appends the hardware-qualified terminal suffix `00 00 01 B7` plus twelve zero bytes, producing `7732165` wire payload bytes and one sequence-end code.

## Hardware run

Evidence directory:

`build/evidence/exp3-p3-local-3m-archive-replay-q32768-retry-20260907-063522`

PS2 ELF remained unchanged:

`/mass/0/PS2VNC-exp3-sms-network-live-streaming-3dab7e30.ELF`

Queue capacity remained `32768` bytes.

## Observed hardware behavior

Operator observation:

- playback was noticeably smoother than the live producer run;
- playback was still somewhat clunky;
- terminal marker sequence was RED -> CYAN -> final frame.

RED was caused only by the decoder-return-count classifier. CYAN confirms terminal transport/integrity success.

## Pi / PS2 telemetry

- DATA frames: `944`
- payload bytes: `7732165`
- send elapsed: `27.827575 s`
- replay throughput: `0.264988 MiB/s`
- credit wait: `27.746210 s` (`99.71%` of send elapsed)
- queue high-water: `30720 / 32768`
- bytes received: `7732165`
- bytes consumed: `7732165`
- CRC32: `3edf1ca9` exact on both sides
- receiver done: `1`
- END received: `1`
- integrity pass: `1`
- stream error: `0`
- coded pictures: `600`
- decoded pictures: `598`
- displayed pictures: `598`
- coded-minus-returned delta: `2`

## Interpretation

P3 proves that removing live capture/producer coupling improves visual smoothness, but does not remove the sustained-rate limitation. The fixed replay path still sustained only about `0.265 MiB/s`, below the P2 source production requirement of about `0.369 MiB/s` for the same 20-second stream. Therefore the dominant sustained-throughput limit is downstream of the Pi encoder, in the PS2 consumption/presentation path or its coupling to credit release.

The RED marker is not transport corruption. Exact bytes, exact CRC, empty terminal queue, `error=0`, and equal decoded/displayed counts all passed. RED occurred because the current O3 classifier accepts only coded-all or coded-minus-one, while this stream produced coded-minus-two (`600 -> 598`). SMS sequence-end/reference-picture behavior therefore needs separate characterization before changing the classifier.

The server-send elapsed includes O3 startup color/hold phases and must not be divided directly by displayed-picture count as a pure per-frame presentation time.

## Next controlled bisection

Keep the exact P2 archive, transport, queue, decoder, upload, draw path, and EOF termination unchanged, while removing only the two per-picture `graph_wait_vsync()` waits in `exp3_draw_picture_texture()`. This measures whether the retained two-VSYNC presentation cadence is the primary limiter. If PS2 consumption rises above the source-required rate, the decoder/upload path has sufficient headroom and presentation scheduling is the bottleneck. If it remains below the source-required rate, continue inward by separating decode cost from GS upload/draw cost.
