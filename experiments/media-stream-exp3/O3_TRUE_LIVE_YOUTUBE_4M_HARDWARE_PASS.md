# O3 true-live YouTube 4M hardware result

## Result

Hardware-qualified functional PASS, real-time performance FAIL.

A playing YouTube video on the Raspberry Pi X11 `:0` desktop was visibly moving in the Windows TigerVNC observer and was independently observed moving on the PlayStation 2 through the O3 live MPEG path. The PS2 completed the run with PURPLE -> CYAN -> final frame held.

This proves the full live path:

Pi X11 -> ffmpeg MPEG-2 elementary stream -> PSTV channel 4 over Ethernet -> 32768-byte EE queue -> SMS/libmpeg -> IPU -> GS.

The PS2 ELF was unchanged:

`/mass/0/PS2VNC-exp3-sms-network-live-streaming-3dab7e30.ELF`

The Pi producer used the E2-qualified explicit MPEG termination wrapper at commit `479babe5c7d83a97ef5be3d20df66c6ddea291db` and appended the exact 16-byte terminal suffix:

`00 00 01 B7` + twelve `00` bytes.

## Evidence identity

Run directory:

`build/evidence/exp3-network-live-sequence-end-q32768-20260907-061054`

Configuration:

- queue capacity: 32768 bytes
- receiver stack: 16384
- receiver priority: 63
- source: X11 `:0.0`, 704x462 padded to 704x480
- requested duration: 20 s
- ffmpeg input rate: 30000/1001
- MPEG-2 target bitrate: 4M
- GOP: 15
- B frames: 0

Archive:

- bytes: 7,017,283
- SHA-256: `ae080760dbea17d50bafe7b9d3d7feed90d0b3fae76ceb467f96b8ffb02e8964`
- CRC32: `6559595f`
- picture starts: 551
- sequence headers: 37
- sequence ends: 1
- archive mod 16: 3
- Pi software decode: PASS

## Pi producer telemetry

- DATA frames: 857
- producer elapsed: 26.741170 s
- payload rate: 0.250258 MiB/s
- credit wait events: 853
- credit wait seconds: 25.693999 s
- max credit wait: 1.479217 s
- initial credit: 32768

The producer spent about 96.1% of its elapsed time blocked waiting for receiver credit.

The 7,017,283-byte archive would require about 0.33461 MiB/s to transmit in exactly 20 seconds. That is essentially equal to the previously measured ~0.33467 MiB/s O2 q32768 fixed-stream throughput, leaving effectively no burst headroom at this live 4M setting.

## PS2 terminal telemetry

- error: 0
- queue capacity: 32768
- queue current: 0
- queue high-water: 32768
- frames received: 857
- bytes received: 7,017,283
- CRC32: `6559595f`
- bytes consumed: 7,017,283
- feed wait events: 566
- feed wait loops: 4306
- max feed wait loops: 53
- pictures decoded: 550
- pictures displayed: 550
- feed calls: 3427
- payload bytes submitted: 7,017,283
- DMA bytes submitted: 7,017,296
- receiver done: 1
- integrity pass: 1
- END received: 1
- producer picture starts: 551
- producer sequence ends: 1
- decoder return delta: 1
- terminal result contract: PASS

The 551 coded -> 550 returned/displayed relationship matches the already-qualified SMS delayed-reference terminal behavior.

## Hardware observation

- Windows TigerVNC showed the source YouTube video playing on the Pi desktop.
- The PS2 visibly displayed live motion from that source.
- Playback was visibly clunky.
- Terminal sequence was PURPLE -> CYAN -> final frame held.

Therefore O3 live correctness is qualified, including explicit sequence-end termination. The remaining issue is throughput/pacing performance rather than transport integrity, decoder correctness, or EOF correctness.

## Next controlled experiment

Hold PS2 ELF, resolution, frame rate, GOP, queue capacity, receiver stack/priority, transport, decoder, presentation path, and sequence-end wrapper constant. Change only ffmpeg target bitrate from 4M to 3M and repeat the same 20-second live-motion test.
