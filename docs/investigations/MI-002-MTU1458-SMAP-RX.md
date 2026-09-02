# MI-002 — MTU1458 / SMAP receive corruption

Status: `OPEN_NON_BLOCKING`

GitHub working issue: #18 — Explain and eventually retire the MTU1458 workaround

Opened as successor major investigation: 2026-09-02

## Executive summary

Historical PS2VNC receive corruption under sustained incremental RFB traffic was
associated with the stock PS2 IP MTU1500 path. A narrow MTU1458 PS2IP variant
survived the paired historical stress and became part of the qualified
networking dependency used by later development.

The workaround remains intentionally preserved in the clean reconstruction, but
it is **not** considered an ideal permanent architecture. Standard Ethernet/IP
MTU1500 remains the preferred end state if the underlying receive defect can be
understood and corrected or avoided cleanly.

A 2026-09-02 forensic review recovered a highly significant clue already encoded
in historical Test33: under the observed header geometry, MTU1458 makes the
maximum received Ethernet frame exactly:

```text
14-byte Ethernet header + 1458-byte IP packet = 1472 bytes
1472 = 23 * 64 bytes
```

The PS2SDK SMAP receive path DMA-copies whole 64-byte blocks and then handles a
residual tail through PIO. At stock MTU1500, the maximum Ethernet frame is 1514
bytes, which is:

```text
1514 = 23 * 64 + 42
```

Thus the 42-byte MTU reduction exactly removes the maximum-frame residual after
the 23rd 64-byte SMAP DMA block.

The leading hypothesis is therefore no longer merely "smaller packets help."
The strongest current explanation is that corruption is associated with the
SMAP RX FIFO DMA-to-PIO residual-tail transition, or with a tightly coupled
cache/FIFO/timing condition exposed at that boundary.

This is a strong source-supported/test-supported hypothesis, not yet causal
proof.

## Symptom and impact

The original visible symptom was a frozen or malformed remote desktop. Early
investigation could reasonably implicate RFB framing, rectangle decode,
framebuffer mutation, GS upload, or networking.

Later instrumentation showed that at least one important failure class already
contained malformed bytes immediately after the application receive operation.
That moved the origin below ordinary RFB/framebuffer mutation.

Because MTU1458 became a known-good networking dependency, current successful
clean hardware tests do **not** independently prove MTU1500 safe: they still use
the preserved qualified MTU1458 stack.

## Historical investigation

### Test9H — receive-boundary clue

A preserved failure reported a maximum-sized receive unit and a nonsensical RFB
header near the end of that unit:

```text
last=1460
rb=1436/1460
hs=1424/1460
hdr=000200020002000200020002
```

Application-buffer offsets must not be mapped directly onto physical Ethernet
FIFO offsets, but the result was an early indication that receive geometry was
relevant.

### Test32 — corruption existed immediately after `recv()`

Test32 added a protected snapshot immediately after successful receive returns.
On a representative stock-MTU failure, the immediate receive hash, protected
snapshot hash, and later live-buffer hash were identical and the same malformed
bytes existed in both snapshot and live data.

This substantially weakened theories in which correct bytes reached the
application and were later corrupted solely by the RFB parser, framebuffer, or
GS presentation path.

### Test33 — deliberate 1472-byte boundary

Test33 kept the Test32 diagnostic shape and deliberately set the PS2 interface
MTU to 1458 before TCP connection establishment.

Its source records the reason: with the 36-byte TCP header observed in the
failure captures, the resulting maximum frame lands exactly on the 1472-byte,
23-by-64-byte boundary.

The preserved Test33 log shows the expected smaller receive geometry and no
retained `RECEIVE_ERROR` match.

### Test10A — MTU1240

The source for an MTU1240 experiment survives. MTU1240 produces a maximum
Ethernet frame of 1254 bytes, which is smaller but still leaves a residual tail:

```text
1254 = 19 * 64 + 38
```

Its exact historical operator outcome has not yet been recovered to the current
provenance standard. That result would be useful because it can help distinguish
"smaller is enough" from "64-byte boundary alignment matters."

Do not invent the missing result.

### Test10B — NetMan pre-DMA cache cleaning

A paired control/experimental NetMan cache-clean experiment and its artifacts
survive. Its exact hardware conclusion has not yet been reconstructed to modern
provenance standards.

That result would help rank cache-coherency explanations relative to direct
SMAP FIFO/DMA-tail explanations.

### Test10C — narrow MTU1500 versus MTU1458 isolation

The preserved Test10C PS2IP discriminator is important because it changed the
effective interface MTU from 1500 to 1458 without the later B4A PBUF/window
changes.

The retained historical execution record associates:

```text
Test10C-A / MTU1500 -> receive corruption under stress
Test10C-B / MTU1458 -> survival of the paired workload
```

Exact build/source/dependency identities are preserved. Under the successor's
stricter provenance rules, the old deployed-ELF-to-hardware-result arrows remain
`CORROBORATED` rather than being retroactively promoted to modern hardware
`PROVEN` status.

### Test10D — known-good live incremental baseline

Test10D restored ordinary live incremental rendering while retaining MTU1458.
That result became part of the known-good networking foundation inherited by the
later qualified legacy dependency.

### Later qualified dependency

The later B4A networking archive also contains larger PBUF/window-scale changes.
Those later settings are part of the exact qualified dependency authority, but
they must not be incorrectly credited as the original Test10C MTU discriminator.

The clean Issue #7 reconstruction deliberately retains the known-good MTU1458
archive rather than silently substituting a nominally similar stock SDK build.

## What is known with high confidence

### Source-backed / directly preserved

- The relevant PS2SDK SMAP RX path transfers whole 64-byte blocks via DMA and
  handles residual bytes after those blocks through PIO when using the aligned
  receive path.
- Historical Test33 intentionally selected MTU1458 to land the maximum received
  Ethernet frame at 1472 bytes = 23 * 64.
- At MTU1500, a maximum 1514-byte Ethernet frame extends exactly 42 bytes beyond
  that same boundary.
- The original Test10C MTU patch is a narrow 1500 -> 1458 interface-MTU change.
- Exact historical source/library/ELF artifacts for the Test10C/Test10D lineage
  survive.
- Current clean PS2 tests still use the qualified MTU1458 networking dependency,
  so they do not disprove the historical need for the workaround.

### Test-supported / corroborated

- Malformed bytes were observed already present in receive data before ordinary
  later RFB/framebuffer processing in Test32.
- Historical Test10C execution associates stock MTU1500 with failure and MTU1458
  with survival under the paired stress.
- Test10D associates MTU1458 with restored normal live incremental behavior.

## What remains unknown

The exact low-level cause has not been proved.

Serious remaining possibilities include:

1. a defect in the residual PIO tail-copy operation itself;
2. FIFO pointer/order behavior during DMA-to-PIO transition;
3. cache/DMA coherency exposed at that boundary;
4. SMAP RX FIFO hardware behavior under sustained load;
5. a timing/load problem merely avoided by the changed segment geometry;
6. a combination of those mechanisms.

The old Test10A and Test10B operator outcomes also remain to be recovered or
cleanly rerun.

## Leading hypothesis

Current ranking:

### H1 — SMAP RX DMA-to-PIO residual-tail boundary

Strongest. The 42-byte difference is exactly the stock maximum-frame tail after
23 complete 64-byte DMA blocks, and historical Test33 deliberately targeted the
zero-tail boundary.

### H2 — cache/FIFO/timing defect coupled to the same transition

Also plausible. Alignment may be the trigger or avoidance mechanism without the
PIO copy itself being defective.

### H3 — generic smaller-segment/load reduction

Still possible, but less explanatory. A non-aligned smaller-MTU control such as
1240 and boundary-neighbor testing can distinguish it.

### Higher-layer RFB/framebuffer root cause

Substantially weakened for this failure class because Test32 found bad bytes in
the protected receive snapshot.

## The 42-byte headroom question

MTU1458 reduces the maximum IP packet by 42 bytes relative to standard MTU1500.
That does **not** mean every network transaction wastes exactly 42 application
bytes; real TCP behavior includes options, ACKs, partial segments, application
framing, congestion control, and pacing.

Nevertheless, the project should not treat those 42 bytes as permanently
unavailable by design. They may ultimately be recoverable if the underlying
receive defect is fixed or if a standards-conforming mechanism avoids the bad
SMAP transfer geometry.

The architecture must therefore preserve an easy path back to MTU1500.

## Current workaround

The qualified workaround is:

```text
PS2 networking / PS2IP effective MTU = 1458
```

It is owned at the PS2 networking seam.

It is **not** an RFB protocol rule, framebuffer geometry rule, Pi Ethernet rule,
or application message-size rule.

## Design guardrails while open

Until this case is resolved:

- do not leak 1458, 1418, 1402, or current MSS values into RFB semantics;
- do not size framebuffer rectangles or parser/application messages around one
  TCP segment geometry;
- keep the Pi's private Ethernet conventional unless evidence requires otherwise;
- allow normal TCP/MSS negotiation to adapt to the PS2 peer;
- keep any future per-socket MSS/segment experiment isolated and replaceable;
- do not silently swap out the qualified PS2IP dependency during unrelated work;
- record effective MTU, SYN/MSS/options, packet-size distribution, exact DUT and
  networking-library identity, workload, and PCAP identity during qualification;
- do not call MTU1458 a permanent hardware requirement until the root mechanism
  is demonstrated;
- do not call it a red herring merely because a later VNC provider works through
  the already-qualified MTU1458 stack.

## Investigation plan

When PS2 networking is deliberately reopened, prefer boundary-focused tests over
a simple 1500/1458 repeat.

Suggested matrix:

```text
MTU 1500 -> max Ethernet frame 1514 -> 23*64 + 42
MTU 1457 -> max Ethernet frame 1471 -> 22*64 + 63
MTU 1458 -> max Ethernet frame 1472 -> 23*64 + 0
MTU 1459 -> max Ethernet frame 1473 -> 23*64 + 1

MTU 1393 -> max Ethernet frame 1407 -> 21*64 + 63
MTU 1394 -> max Ethernet frame 1408 -> 22*64 + 0
MTU 1395 -> max Ethernet frame 1409 -> 22*64 + 1
```

Run identical deterministic receive/incremental stress with exact DUT identity,
wire capture, and packet-length analysis.

If aligned MTUs are safe while their one-byte neighbors reproduce corruption,
the 64-byte transfer-boundary explanation becomes very strong.

A complementary experiment should retain interface MTU1500 while constraining
inbound TCP segment geometry. That can separate "interface MTU itself matters"
from "the resulting maximum SMAP receive-frame length matters."

If necessary, follow with targeted SMAP instrumentation around:

- DMA block count;
- residual PIO count;
- RX FIFO pointers/status;
- cache ownership;
- exact bytes at the DMA/PIO boundary.

## Closure criteria

The case can close when the project can state, with controlled evidence:

- what receive condition causes or exposes corruption;
- whether the 64-byte DMA/PIO boundary is causal or merely correlated;
- whether the defect is hardware, driver, cache, FIFO-ordering, or timing related
  to the degree needed for engineering action;
- whether a clean fix or standards-conforming avoidance permits reliable MTU1500;
- whether the MTU1458 workaround can be removed from the qualified dependency;
- what qualification matrix protects against regression.

If MTU1500 cannot be made reliable without invasive or unjustified complexity,
closure may instead document MTU1458 as the deliberate permanent adaptation—but
only after the root mechanism and tradeoff are sufficiently understood.

## Evidence and authority

Successor working thread:

- GitHub Issue #18.

Successor durable lessons:

- `docs/audit/HISTORICAL_LESSONS.md` L03;
- clean Issue #7 build/dependency authority.

Legacy PS2VNC remains read-only historical evidence. Relevant historical roots
include Test9, Test32, Test33, Test10A/B/C/D, the frozen Test10D known-good
archive, preserved PS2IP/NetMan patches, logs, and PCAPs.

This case file is intentionally non-blocking for unrelated Pi lifecycle and
clean-core work. The qualified MTU1458 dependency remains protected until this
investigation is explicitly resumed.