# EXP3 deterministic stress reel — 608x416 Main/Main 2M VBV hardware result

## Result

The deterministic 24-second 608x416 stress reel is hardware-qualified as the current smooth MPEG presentation benchmark.

Operator visual observation: **very smooth to the eye**, followed by **RED -> CYAN** terminal markers.

The RED terminal marker is the existing returned-picture-count classifier, not a transport/integrity failure. The stream contained 720 coded pictures and SMS returned/displayed 718 pictures (delta 2). Transport integrity and receiver completion were clean.

## Identities

- PS2 ELF: existing P7A deep-buffer ELF
- ELF SHA256: `efbf929323423757043dd195f9a7ee1471c18797085dc0a5ed108a54da59be20`
- Lossless FFV1 reference SHA256: `1d42d891f276918dc9a4d6211cc21eec4e546a490c426f2d880d832175b82f0c`
- MPEG-2 source SHA256: `6bcfb93badf653a3d72de83e7d9253d01fe0027283f171954c9de6cefbf40aca`
- MPEG-2 source bytes: `6209147`
- Source: 608x416, 30000/1001 fps, MPEG-2 Main/Main, yuv420p, target 2,000,000 bit/s, maxrate 2,516,000 bit/s, VBV 1,835,008 bits, GOP 15, no B-frames
- Coded pictures: `720`
- Sequence headers: `48`
- Queue capacity: `262144` bytes
- Prefill target/observed: `229376 / 229376` bytes

## Hardware telemetry

- Deadline misses: `0`
- Maximum deadline lateness: `0` ticks / `0.000000` frame periods
- VSync waits: `1436`
- Feed wait events: `0`
- Feed wait loops: `0`
- Feed wait maximum loops: `0`
- Queue high water: `260096`
- Pictures decoded/displayed: `718 / 718`
- Producer coded pictures: `720`
- Decoder return delta: `2`
- Bytes received/consumed: `6209163 / 6209163` including qualified sequence-end suffix
- CRC32: `a61c2dfb`
- Receiver done: `1`
- Integrity pass: `1`
- End received: `1`
- Result error: `0`

## Interpretation

This result is the strongest presentation-performance baseline so far. The deep compressed-data reservoir and 224 KiB prefill eliminated decoder feed starvation entirely, while the 608x416 geometry plus shaped 2 Mbit/s Main/Main stream produced zero presentation deadline misses across the deliberately difficult reel.

The repeatable RED terminal state is orthogonal to performance and remains an EOF/returned-picture characterization problem: this stream reproduced the earlier `coded - returned = 2` relationship even though byte transport, CRC, queue drain, receiver completion, and presentation timing all passed.

This benchmark should remain fixed while decoder/output-path changes are evaluated. In particular, the vendored SMS libmpeg core exports `_MPEG_Set16`; the next decoder investigation should determine its exact RGB16/CSC memory contract and qualify a 16-bit output/presentation path against this same stress reel before changing encoder quality again.