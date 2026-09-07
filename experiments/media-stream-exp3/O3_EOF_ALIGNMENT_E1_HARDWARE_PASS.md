# EXP3 O3 EOF Alignment E1 — Hardware PASS

## Purpose

Characterize the O3 live-stream terminal hang without changing PS2 code, queue capacity, framing, codec content, or decoder path.

The failed live archive ended on an exact 16-byte boundary and contained no literal MPEG sequence-end start code. E1 replayed the exact same archived MPEG payload and appended exactly one `0x00` byte.

## Authorities

- O3 PS2 ELF commit: `6475ecf6b24e84e4e19eb3c27aed8239c49556c3`
- E1 apparatus commit: `90f59342ad6f500e6c50e757685ee5857fb09e0b`
- O3 ELF SHA-256: `3dab7e30086343f082c058adc8951142f7a6ca48d907a6cef9dad887ed6fe8c9`
- Original live archive SHA-256: `2c204d8ba20d593a25ab7ccee83a05cb8d1d746fb54bbabcebac387ffc15443c`
- Original live archive bytes: `5653840`
- Original live archive CRC32: `77725e5b`
- Original coded pictures: `498`
- Original sequence headers: `34`
- Original sequence-end codes: `0`

## Single intentional variable

Original failed stream:

- bytes: `5653840`
- `bytes mod 16 = 0`
- final 2048-byte feed payload: `1360`
- final DMA padding: `0`

E1 replay:

- appended bytes: exactly one `0x00`
- bytes: `5653841`
- `bytes mod 16 = 1`
- final feed payload: `1361`
- final DMA padding: `15`

No PS2 executable rebuild or redeployment occurred for E1.

## Hardware observation

Observed sequence:

1. normal startup color sequence;
2. recorded static desktop replay;
3. after approximately the expected playback interval, PURPLE;
4. then CYAN;
5. then the final decoded desktop frame held.

Interpretation:

- PURPLE: SMS returned/displayed one fewer picture than producer-coded picture count.
- CYAN: exact transport/integrity contract passed.
- final frame hold: harness reached normal terminal path.

The original live run using the exact same PS2 ELF and original 5,653,840-byte stream did not reach the decoder terminal path and the Pi timed out after 120 seconds waiting for the PS2 result.

## Pi/PS2 telemetry

E1 replay source:

- wire payload bytes: `5653841`
- DATA frames: `691`
- CRC32: `2ecc15af`
- coded pictures: `498`
- sequence headers: `34`
- sequence ends: `0`

Terminal result:

- error: `0`
- queue capacity: `32768`
- queue current: `0`
- queue high water: `32768`
- frames received: `691`
- bytes received: `5653841`
- bytes consumed: `5653841`
- CRC32: `2ecc15af`
- receiver done: `1`
- end received: `1`
- integrity pass: `1`
- pictures decoded: `497`
- pictures displayed: `497`
- decoder return delta: `1`
- feed calls: `2761`
- payload bytes submitted: `5653841`
- DMA bytes submitted: `5653856`
- result wait seconds: `2.178992`

Overall Pi evidence:

- `O3_EOF_REPLAY_RESULT_CONTRACT=PASS`
- `O3_EOF_REPLAY_OVERALL_PI_EVIDENCE=PASS`

## Result

**E1 HARDWARE PASS.**

Appending exactly one zero byte to the otherwise unchanged failed live MPEG stream converted the O3 terminal decoder hang into a normal SMS return and full transport/integrity PASS.

This strongly isolates the failure to a terminal bitstream/refill/DMA-boundary condition rather than live capture, MPEG encoding, network transport, queueing, CRC integrity, or normal picture decode/presentation.

E1 is diagnostic evidence only. Appending an arbitrary zero byte is not treated as the intended product termination mechanism.

The next narrow experiment should replay the original failed archive with a literal MPEG-2 sequence-end start code (`00 00 01 B7`) appended, because SMS explicitly recognizes that code in its header parser. If that passes, it provides a principled terminal mechanism for the live producer.
