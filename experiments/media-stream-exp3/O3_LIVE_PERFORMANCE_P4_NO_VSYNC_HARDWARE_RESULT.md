# EXP3 O3 P4 — no-presentation-VSYNC hardware result

## Scope

P4 replayed the exact P2 3M, 600-picture archive through the unchanged O3 transport/SMS/IPU/GS path while removing only the two per-picture `graph_wait_vsync()` calls at the end of `exp3_draw_picture_texture()`.

This was a throughput diagnostic, not a proposed product timing policy.

## Identity

- P4 generator authority: `3480cd11a36be9d4666684f53b362f627f43dc37`
- generated P4 source blob: `42f62c3facc084a8cc1a3dc8dfe28ca0219e6015`
- P4 ELF SHA-256: `abc7763aa9282ea9b3764ea522df2cc2f58d6b40e48218fbc03862dd78f2f368`
- P4 ELF bytes: `2460952`
- queue capacity: `32768`
- source archive SHA-256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- source archive bytes: `7732149`
- source coded pictures: `600`

## Hardware observation

- video completed;
- playback was noticeably fast;
- motion was choppy / fast-forward-like;
- terminal sequence was RED -> CYAN -> final frame.

The fast-forward-like behavior is expected from an intentionally unpaced presentation loop.

## Transport / decoder telemetry

- replay payload bytes: `7732165`
- replay send time: `16.747557 s`
- replay throughput: `0.440301 MiB/s`
- exact source rate required for 20 s playback: `0.368698 MiB/s`
- throughput / real-time requirement: `1.194204`
- credit-wait time: `16.674679 s` (`99.56%` of replay send interval)
- queue high water: `30720`
- received bytes = consumed bytes = `7732165`
- CRC32 exact: `3edf1ca9`
- receiver done: `1`
- integrity pass: `1`
- END received: `1`
- coded pictures: `600`
- decoded pictures: `598`
- displayed pictures: `598`
- coded-minus-returned delta: `2`

## P3 comparison

P3 used the same archive and same PS2 path with the original two per-picture VSYNC waits:

- P3: `27.827575 s`, `0.264988 MiB/s`
- P4: `16.747557 s`, `0.440301 MiB/s`
- P4 / P3 throughput multiplier: `1.661590`
- elapsed reduction: `11.080018 s`

Therefore the two retained presentation VSYNC waits materially throttled the O3 PS2 pipeline. Removing them gives enough raw throughput for this exact 704x480 source with about 19.4% throughput headroom over the source's 20-second byte-rate requirement.

This hardware result rejects the hypothesis that the current 704x480 MPEG path is fundamentally incapable of real-time decode/upload/draw throughput. The remaining problem is presentation timing/pacing.

## RED terminal marker

RED was caused by the current O3 classifier seeing `600 coded -> 598 returned/displayed`, while CYAN independently proved exact concurrent-stream integrity.

The returned-picture-count issue remains a separate EOF/reference-picture characterization task. It is not evidence of network corruption in this run.

## Next controlled test

P5 should retain the exact P2 archive, Q32768, decoder, GS upload/draw, transport, and EOF suffix while using exactly **one** per-picture `graph_wait_vsync()` call.

P3 (two waits) is too slow; P4 (zero waits) is too fast. P5 provides the direct hardware bracket before introducing a timestamp/PTS-based pacing policy.
