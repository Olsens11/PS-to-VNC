# EXP3 O2 Q32768 hardware PASS

O2 Q32768 qualifies smooth concurrent network-fed MPEG playback on real PS2 hardware using the same O2 ELF and exact fixture as Q8192. The only intentional runtime variable changed from Q8192 was queue capacity: 8192 -> 32768 bytes.

## Authority

- O2 code commit: `9a2680801c8e5b18f23a4c5f85f50db2edcbccec`
- Prior documentation head before this result: `115f6660fd737d2414689ce86667f8eb21a9fd9b`
- ELF SHA-256: `7c5015832213414651a6aa7f98e2a63c05e1e1b4a01230e8abd4fe63d4d50fd4`
- ELF bytes: `2458092`
- archival ELF: `/mass/0/PS2VNC-exp3-sms-network-streaming-fixture-7c501583.ELF`

## Exact fixture

- SHA-256: `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`
- bytes: `8815372`
- CRC32: `4bdc7859`

## Runtime configuration

- queue capacity: `32768`
- receiver stack: `16384`
- receiver priority: `63`
- server: `192.168.50.1:5904`
- PS2: `192.168.50.2`
- interface: `eth0`

Evidence directory:

`build/evidence/exp3-network-streaming-q32768-20260907-045432`

## Pi sender evidence

- CONFIG ACK: PASS
- initial credit: `32768`
- frames sent: `1077`
- payload bytes sent: `8815372`
- last DATA sequence: `1077`
- send elapsed: `25.120161 s`
- payload throughput: `0.334671 MiB/s`
- credit frames received: `1074`
- credit bytes received: `8822784`
- credit wait events: `1073`
- credit wait seconds: `25.048432 s`
- maximum single credit wait: `2.160254 s`
- terminal result wait: `2.191418 s`

The high credit-wait count is expected under the credit-window design after the initial four-frame grant and must not be interpreted as equivalent to decoder starvation.

## PS2 telemetry

- stream error: `0`
- queue current at terminal: `0`
- queue high-water: `22528`
- frames received: `1077`
- bytes received: `8815372`
- last DATA sequence: `1077`
- CRC32: `4bdc7859`
- bytes consumed: `8815372`
- feed wait events: `382`
- feed wait loops: `1593`
- feed wait max loops: `48`
- credit frames sent: `1078`
- credit bytes sent: `8848140`
- pictures decoded: `598`
- pictures displayed: `598`
- feed calls: `4305`
- payload bytes submitted: `8815372`
- DMA bytes submitted: `8815376`
- receiver done: `1`
- integrity pass: `1`
- Pi result contract: PASS
- Pi overall evidence: PASS

## Packet capture

- PCAP bytes: `9507878`
- packet records: `9060`

## Hardware observation

Observed playback was **smooth and snappy**, completed normally, and reached the expected terminal sequence:

`PURPLE -> CYAN -> final frame held`

Hardware result:

- concurrent functional result: **PASS**
- smooth playback result: **PASS**
- exact terminal integrity result: **PASS**

## Comparison with Q8192

Q8192 was correct but visibly slow/choppy. Q32768 preserved exact correctness while dramatically reducing real decoder starvation pressure:

- feed wait events: `1077 -> 382` (~64.5% reduction)
- feed wait loops: `47418 -> 1593` (~96.6% reduction)
- maximum feed wait: `216 -> 48` loops (~77.8% reduction)
- fixture send time: `72.156688 -> 25.120161 s` (~65.2% reduction)

Q32768 queue high-water was only `22528` bytes, so the 32 KiB allocation was not filled during this run.

## Qualification

Q32768 is the first hardware-proven smooth operating point for the EXP3 concurrent MPEG fixture-replay path.

This does **not** imply that 32768 bytes is a universal product hard-code or optimal queue size. The queue remains a runtime-configured resource whose appropriate value should be selected from measured behavior of the eventual live producer/mux workload.

The next meaningful step should move toward the real producer path rather than continuing to optimize the finite fixture harness in isolation: retain the qualified concurrent receiver/decoder mechanism and introduce a live Pi MPEG-2 elementary-stream producer, while preserving strong queue/starvation/credit telemetry.
