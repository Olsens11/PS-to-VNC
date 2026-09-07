# EXP3 O2 — Q8192 concurrent MPEG streaming hardware result

## Result

O2 at the minimum one-frame MPEG queue capacity is a **functional hardware PASS** and a **smooth-playback FAIL**.

The PS2 decoded MPEG while the exact fixture was arriving concurrently over Ethernet, completed the full stream, reproduced the expected SMS terminal picture count, passed exact end-to-end integrity, and held the final successfully presented frame. Playback was intentionally stressed by the minimum 8192-byte receiver reservoir and was visibly slow/choppy.

## Authority

- O2 code commit: `9a2680801c8e5b18f23a4c5f85f50db2edcbccec`
- ELF SHA-256: `7c5015832213414651a6aa7f98e2a63c05e1e1b4a01230e8abd4fe63d4d50fd4`
- ELF bytes: `2458092`
- archival ELF: `/mass/0/PS2VNC-exp3-sms-network-streaming-fixture-7c501583.ELF`
- fixture SHA-256: `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`
- fixture bytes: `8815372`
- fixture CRC32: `4bdc7859`

## Test configuration

- queue capacity: `8192` bytes
- receiver stack: `16384` bytes
- receiver priority: `63`
- server: `192.168.50.1:5904`
- PS2: `192.168.50.2`
- interface: `eth0`
- evidence directory: `build/evidence/exp3-network-streaming-q8192-20260907-044521`

The 8192-byte queue is exactly one maximum PSTV DATA payload and therefore the minimum meaningful queue capacity for this transport shape. It is not a guessed safety value.

## Hardware observation

Observed:

```text
full concurrent playback
-> PURPLE
-> CYAN
-> final frame held
```

Playback was slow and somewhat choppy. The operator estimated roughly three seconds of wall-clock time per one second of content.

## Pi sender telemetry

- CONFIG ACK: PASS
- initial credit: `8192`
- frames sent: `1077`
- payload bytes sent: `8815372`
- last DATA sequence: `1077`
- send elapsed: `72.156688 s`
- payload throughput: `0.116510 MiB/s`
- credit frames received before terminal result: `1077`
- credit bytes received before terminal result: `8822784`
- credit wait events: `1076`
- credit wait time: `72.052373 s`
- maximum credit wait: `2.160361 s`
- result wait: `2.302304 s`

The sender spent essentially the entire transfer waiting for credit: 72.052373 of 72.156688 seconds.

## PS2 telemetry

- error: `0`
- queue capacity: `8192`
- queue current at terminal: `0`
- queue high-water: `8192`
- frames received: `1077`
- bytes received: `8815372`
- last DATA sequence: `1077`
- CRC32: `4bdc7859`
- bytes consumed: `8815372`
- feed wait events: `1077`
- feed wait loops: `47418`
- feed wait max loops: `216`
- credit frames sent: `1078`
- credit bytes sent: `8823564`
- pictures decoded: `598`
- pictures displayed: `598`
- feed calls: `4305`
- payload bytes submitted: `8815372`
- DMA bytes submitted: `8815376`
- receiver done: `1`
- integrity pass: `1`

Credit accounting is exact: initial queue credit plus all bytes consumed equals `8192 + 8815372 = 8823564`, exactly matching PS2 `credit_bytes_sent`. The Pi's pre-result sender summary is 780 bytes lower because the final 780-byte credit is received during the terminal-result phase after DATA transmission is already complete.

## Interpretation

O2 proves the key product boundary: the PS2 can remain stable while SMS MPEG consumption and Ethernet MPEG production occur concurrently.

The minimum queue exposes near stop-and-wait behavior:

- queue high-water equals full capacity;
- the decoder records 1077 feed starvation events, essentially one per fixture DATA frame;
- the Pi records 1076 credit wait events, essentially one between each pair of DATA frames;
- exact byte count, CRC, queue drain, and 598-picture SMS result still pass.

The fixture is approximately 20 seconds long. At its average encoded rate, one 8192-byte DATA frame represents about 18.6 ms of stream data. The measured average Pi credit wait is about 67.0 ms, or about 3.6 DATA-frame times. Therefore the smallest evidence-derived next capacity is four DATA frames: `32768` bytes.

## Qualification

- concurrent network production + MPEG consumption: **PASS**
- exact transport framing/sequence/CRC: **PASS**
- exact decoder consumption: **PASS**
- SMS terminal behavior: **PASS**
- queue/credit accounting: **PASS**
- smooth playback at 8192-byte queue: **FAIL**
- overall O2 Q8192 functional result: **PASS**

## Next test

Repeat the exact same O2 ELF and fixture with runtime CONFIG queue capacity `32768` bytes (four maximum DATA frames). No rebuild is required. This test point is derived directly from the measured Q8192 credit latency rather than chosen as an arbitrary buffer size.
