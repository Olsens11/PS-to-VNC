# O3 EOF E2 — explicit MPEG sequence-end hardware PASS

## Purpose

E2 tested whether EXP3 O3's live-stream EOF hang was caused by the final DMA alignment itself or by the absence of an explicit MPEG sequence-end code.

The original failed live archive was replayed through the exact same O3 PS2 ELF and runtime. E2 appended a 16-byte suffix consisting of the MPEG sequence-end start code `00 00 01 B7` followed by twelve zero bytes. The 16-byte suffix deliberately preserved the original stream's `mod 16 = 0` geometry and zero final DMA padding.

## Authorities

- E2 apparatus commit: `45af82149ade0302cad8394a20601ceb9e0eb3ef`
- O3 PS2 archival ELF: `/mass/0/PS2VNC-exp3-sms-network-live-streaming-3dab7e30.ELF`
- Original failed live archive: `build/evidence/exp3-network-live-stream-q32768-20260907-052311/live-stream.m2v`
- Original bytes: `5653840`
- Original SHA256: `2c204d8ba20d593a25ab7ccee83a05cb8d1d746fb54bbabcebac387ffc15443c`
- Original CRC32: `77725e5b`
- E2 evidence run: `build/evidence/exp3-o3-eof-sequence-end-aligned-20260907-055144`

## Single-variable geometry

Original failed stream:

- bytes: `5653840`
- `mod16 = 0`
- final DMA pad: `0`
- literal sequence ends: `0`

E2 stream:

- suffix: `000001b7000000000000000000000000`
- suffix bytes: `16`
- total bytes: `5653856`
- `mod16 = 0`
- final DMA pad: `0`
- literal sequence ends: `1`

Thus E2 preserved the exact qword-aligned terminal geometry that hung in the original run while introducing a real MPEG sequence-end marker.

## Pi / transport result

The replay server reported:

- DATA frames: `691`
- bytes: `5653856`
- CRC32: `963a76e9`
- picture starts: `498`
- sequence headers: `34`
- sequence ends: `1`
- send elapsed: `23.712830 s`
- result wait: `2.180596 s`

PS2 terminal telemetry:

- error: `0`
- queue capacity: `32768`
- queue current: `0`
- queue high water: `32768`
- frames received: `691`
- bytes received: `5653856`
- bytes consumed: `5653856`
- CRC32: `963a76e9`
- pictures decoded: `497`
- pictures displayed: `497`
- feed calls: `2761`
- payload bytes submitted: `5653856`
- DMA bytes submitted: `5653856`
- receiver done: `1`
- end received: `1`
- integrity pass: `1`
- producer picture starts: `498`
- producer sequence ends: `1`
- decoder return delta: `1`
- result contract: PASS
- overall Pi evidence: PASS

PCAP:

- bytes: `6104204`
- packet records: `5857`

## Hardware observation

Observed sequence:

`recorded desktop -> PURPLE -> CYAN -> final desktop frame held`

This is the expected successful O3 terminal sequence:

- PURPLE: SMS returned/displayed one fewer picture than the producer's coded-picture count (`497` returned from `498` coded), matching the previously characterized delayed-reference behavior.
- CYAN: exact concurrent transport/integrity contract passed.
- Final desktop frame: last decoded GS texture remained held as designed.

## Conclusion

**E2 is a hardware PASS.**

The original hang is not caused by qword-aligned terminal DMA geometry. E2 retained `mod16 = 0` and zero DMA padding yet terminated normally once the stream contained an explicit MPEG sequence-end code.

Combined with E1, the evidence is:

1. no sequence-end + aligned terminal DMA -> decoder hang;
2. no sequence-end + deliberately unaligned terminal DMA -> decoder returns through SMS's callback-exhaustion/synthetic-EOF path;
3. explicit sequence-end + aligned terminal DMA -> decoder returns normally.

Therefore the correct O3 finite-live producer behavior is to provide deterministic MPEG elementary-stream termination with a real sequence-end marker, rather than depending on DMA padding or callback exhaustion side effects.
