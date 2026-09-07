# O3 live performance P2 — local 3M capture control

## Purpose

Remove the PS2 and network from the path while keeping the same live X11 → ffmpeg MPEG-2 capture/encode settings used by O3 P1. This determines whether the observed live slowdown is already present on the Pi capture/encode side or is introduced by downstream backpressure.

## Authority

- Branch: `experiment/media-stream-exp3`
- P1 record parent: `9aac8821042eee7be4f06b6428baca69d4e9107e`
- Test date: 2026-09-07
- Source: Pi X11 `:0`, YouTube visibly playing through TigerVNC
- Capture: 704x462 X11 → pad 704x480 → yuv420p → MPEG-2 video
- Framerate: `30000/1001`
- Bitrate setting: `3M`
- GOP: 15
- B-frames: 0
- Nominal duration: 20 s
- PS2: not involved
- Network: not involved

## Evidence

Run directory:

`build/evidence/exp3-local-x11-3m-control-20260907-062342`

Archive:

`local-stream.m2v`

Measured result:

- ffmpeg exit: 0
- local sink exit: 0
- wall elapsed: `20.441630 s`
- archive bytes: `7732149`
- SHA-256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- CRC32: `ce1e9d03`
- picture starts: `600`
- sequence headers: `40`
- literal sequence ends: `0`
- actual wall write rate: `0.360732 MiB/s`
- payload rate over nominal 20 s: `0.368698 MiB/s`
- pictures per wall second: `29.351867`
- pictures per nominal source second: `30.000000`
- software decode: PASS

The only ffmpeg diagnostic was the previously observed x11grab 16-bpp warning.

## Result

**P2 local real-time capability: PASS.**

The Pi capture/encode path produced the full 600-picture nominal 20-second source in 20.44 seconds with no PS2 or network present. Therefore the major slowdown observed in P1 is not intrinsic to the X11 capture/ffmpeg encode path.

P1 with the PS2 attached produced only 520 coded pictures and required 26.1093 seconds while spending 95.99% of producer elapsed time waiting for receiver credit. P2 proves that this loss of real-time behavior is introduced downstream of the local encoder, through the live transport / PS2 consumption path.

## Next discriminator

Replay this exact P2 archive through the same O3 PS2 transport/decoder with the hardware-qualified explicit MPEG sequence-end suffix and the same 32768-byte queue.

- If replay throughput remains near the P1 ~0.249 MiB/s rate, the limiting rate is on the PS2 decode/presentation/credit-consumption side for this content.
- If replay sustains roughly the source requirement (~0.369 MiB/s) or better, the bottleneck is specific to live producer/credit interaction rather than raw PS2 decode capacity.

Do not change the PS2 ELF, queue size, resolution, decoder, or MPEG settings for that replay.