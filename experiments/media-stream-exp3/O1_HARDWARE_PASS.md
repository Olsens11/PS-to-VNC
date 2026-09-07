# EXP3 O1 Network MPEG Fixture Replay — Hardware PASS

Hardware result: **PASS** on September 7, 2026.

## Code authority

- Branch: `experiment/media-stream-exp3`
- O1 code commit: `3c9ff944eb53f55dd903d361d626e923ccba1359`
- Parent: `6cd082320fa9476799eb3219487bad0e95e0efbf`

## Hardware-tested ELF

- SHA-256: `3a40ca6bea39db5cdf8f0482a9028e21cf646a84d4c531ef2f768e413d791089`
- Bytes: `2433192`
- Archival path: `/mass/0/PS2VNC-exp3-sms-network-fixture-replay-3a40ca6b.ELF`

## Exact MPEG fixture

- SHA-256: `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`
- CRC32: `4bdc7859`
- Payload bytes: `8815372`
- Coded pictures: `599`

## O1 wire shape

- PSTV version 1 DATA frames
- MPEG2 logical channel 4
- Dedicated experimental TCP port 5904
- Sequence 0 through 1076
- 1077 DATA frames
- 1076 full 8192-byte payloads
- Final 780-byte payload

## Pi replay evidence

Evidence directory:

`build/evidence/exp3-network-fixture-replay-20260907-035445`

Observed sender evidence:

- Client: `192.168.50.2:52432`
- Frames sent: `1077`
- MPEG payload bytes sent: `8815372`
- PSTV header bytes: `17232`
- Total PSTV wire bytes: `8832604`
- Last sequence: `1076`
- Replay elapsed: `2.461478 s`
- Payload throughput: `3.415425 MiB/s`
- Peer closed cleanly: yes

Packet capture:

- Interface: `eth0`
- PCAP bytes: `9548382`
- Packet records: `9594`

## Real-hardware observation

Observed visible sequence:

```text
blank briefly
-> CYAN
-> YELLOW
-> ORANGE
-> GREEN
-> BLUE
-> full moving MPEG
-> PURPLE
-> CYAN
-> final frame held
```

Interpretation:

- The initial blank interval covered network initialization and complete network fixture acquisition.
- First CYAN proved successful PSTV framing, sequence validation, exact byte count, and PS2-side CRC32 verification.
- YELLOW through BLUE reproduced the already-qualified IPU, SMS libmpeg, and GS presentation boundaries.
- Full moving MPEG proved that the network-received bitstream was accepted by the qualified decoder/presentation path.
- PURPLE reproduced the characterized SMS terminal behavior: 598 pictures returned/displayed from this 599-coded-picture fixture.
- Terminal CYAN proved exact decoder-input exhaustion.
- The last successfully presented GS texture remained valid and was redrawn.

## Qualification

O1 qualifies:

- network initialization on the project PS2 networking seam;
- TCP delivery of the exact MPEG fixture;
- PSTV version-1 framing on logical MPEG2 channel 4;
- exact frame sequencing;
- exact byte transfer;
- end-to-end PS2-side CRC32 integrity;
- successful SMS/IPU/GS playback from network-received MPEG bytes;
- unchanged natural-EOF behavior relative to the local qualified path.

O1 overall result: **PASS**.

## Deliberate limitation / O2 boundary

O1 intentionally buffers the complete MPEG fixture before starting decode. It therefore does **not** yet qualify concurrent network producer / MPEG decoder operation.

O2 changes only that remaining boundary: network reception and SMS MPEG consumption will operate concurrently through a bounded byte queue, retaining the same decoder/feed/GS behavior and the same exact MPEG fixture for the first streaming test.
