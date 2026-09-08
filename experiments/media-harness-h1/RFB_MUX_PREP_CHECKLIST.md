# H1 RFB mux preparation checklist

This checklist is a repository-only staging surface for the work described in
`RFB_MUX_INTEGRATION_PREP.md`. It intentionally separates tasks that can be
completed without PS2 hardware from claims that require operator/hardware
evidence.

## Repository-only preparation

- [x] Record one-socket/channel ownership invariant.
- [x] Identify current RFB CONFIG/HELLO/queue/bridge/presentation blockers.
- [x] Preserve the existing through-Issue-39 `rfb_io.h` seam as the adapter
      boundary rather than changing parser semantics.
- [x] Choose the existing 32768-byte direct-RFB prefetch capacity as the initial
      evidence-based queue reference, not an arbitrary larger buffer.
- [x] Bind the cumulative H1 RFB-session translation unit to experiment-owned
      mux-adapter symbol names while leaving clean parser source unchanged.
- [x] Add source/object build checks proving the cumulative RFB session resolves
      to the H1 adapter names rather than direct VNC socket I/O.
- [x] Establish a pinned-toolchain CI build/fingerprint path and preserve each
      resulting candidate ELF explicitly as unqualified.
- [ ] Replace CP1 fail-closed adapter bodies with real channel-1 queue reads and
      serialized logical writes.
- [ ] Add channel-1 queue/credit accounting that is inert when RFB is OFF.
- [ ] Add channel-1 logical write fragmentation through H1's existing send lock.
- [ ] Add Pi raw-byte VNC<->PSTV channel-1 bridge.
- [ ] Add RFB channel telemetry/counters without disturbing existing AUDIO/MPEG
      scheduler behavior.
- [ ] Add RFB-only H1 profile and make `h1_tool.py --rfb on` operational for it.
- [ ] Rebuild/fingerprint the final pre-hardware candidate after all repository
      preparation changes are complete.
- [ ] Keep all AUDIO/MPEG known-good profiles behavior-compatible where RFB
      remains OFF; any changed PT_LOAD remains hardware-pending until qualified.

## Hardware/operator gate

- [ ] Deploy/read back exact candidate ELF using established apparatus.
- [ ] RFB-only handshake and initial-frame pass.
- [ ] Incremental update pass.
- [ ] Input write pass if included in the milestone.
- [ ] Packet evidence confirms one PS2-facing PSTV TCP stream.
- [ ] Queue/credit telemetry remains coherent under moving desktop workload.
- [ ] No silent-stall recovery is added; any stall is classified/instrumented.

## Later work after RFB transport qualification

- [ ] Shared presentation owner for RFB + MPEG.
- [ ] RFB presentation suppression only beneath active MPEG semantic object.
- [ ] Continue authoritative RFB decode beneath MPEG.
- [ ] Preserve individual RFB update rectangles for media activity detection.
- [ ] Report-only dynamic media-region detector.
- [ ] Host-window attachment and exact-pixel semantic media geometry.
- [ ] Optional visibility/shape mask for rounded/clipped media regions.
- [ ] Automatic MPEG capture/draw steering only after detector qualification.
