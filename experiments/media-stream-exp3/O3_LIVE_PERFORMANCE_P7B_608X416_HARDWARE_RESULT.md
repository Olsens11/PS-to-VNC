# EXP3 P7B — 608x416 resolution-only hardware result

## Result

P7B is hardware-qualified as a successful resolution-only performance experiment.

The exact P7A PS2 ELF, 256 KiB compressed queue, 224 KiB prefill, absolute 30000/1001-fps presentation clock, decoder path, GS upload path, and no-drop policy were retained. Only the archived MPEG-2 source dimensions changed from 704x480 to 608x416.

Operator visual observation: **smooth playback**, followed by **PURPLE -> CYAN** terminal markers.

## Identities

- P7A ELF SHA256: `efbf929323423757043dd195f9a7ee1471c18797085dc0a5ed108a54da59be20`
- P7A ELF bytes: `2471776`
- P7B source: `608x416`, `30000/1001`, MPEG-2, yuv420p, nominal 3 Mbit/s, GOP 15, `-bf 0`
- P7B source bytes: `7704609`
- P7B source SHA256: `eb9521238271881f67991d2d240ba5226068ffd062db0c5b76c3f59cdbce495b`
- P7B source CRC32: `de9b8567`
- Coded pictures: `600`
- Queue capacity: `262144` bytes
- Prefill target/observed: `229376` bytes

## Hardware telemetry

- PS2 transport/integrity: PASS
- Received/consumed bytes: `7704625` including qualified sequence-end suffix
- CRC32: `eb93a934`
- Queue high water: `237568`
- Feed wait events: `13`
- Feed wait loops: `91`
- Feed wait max loops: `49`
- Pictures decoded/displayed: `599 / 599`
- Producer coded pictures: `600`
- Decoder return delta: `1`
- Deadline misses: `6`
- Maximum deadline lateness: `2.124958` source-frame periods
- VSync waits: `1186`

The delta of one produced the expected PURPLE terminal classifier and the replay contract passed completely, unlike the earlier repeatable 600 -> 598 archive result.

## Interpretation

P7B was visually smoother than P7A while changing no PS2 code or buffering policy. This establishes that reducing the decoded picture from 704x480 to 608x416 materially improves perceived presentation behavior under the qualified deep-buffer setup.

However, P7B did **not** materially reduce compressed source size: P7A's source was 7,732,149 bytes and P7B's source was 7,704,609 bytes because both were encoded at the same nominal 3 Mbit/s target. Feed starvation counters were therefore not lower than P7A, and one or more isolated stalls produced a larger maximum lateness outlier despite smooth subjective playback.

The clean next lever is therefore encoder/rate-control shaping while holding 608x416 and the exact P7A PS2 ELF constant: explicitly Main Profile/Main Level, yuv420p, no B-frames, bounded `maxrate`, and the MPEG-2 MP@ML VBV buffer size of 1,835,008 bits. This separates compressed-stream pressure/burstiness from the already-proven resolution gain.
