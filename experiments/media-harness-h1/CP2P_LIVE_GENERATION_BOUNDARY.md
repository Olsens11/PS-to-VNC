# H1 CP2P Live MPEG Generation Boundary — Item #11B

Status: implementation/proof contract for the pre-public-gate #11B tranche.

## Purpose

The H1 transport MPEG queue is intentionally one bounded session-scoped ring. Item
#11B prevents stale generation N bytes from becoming generation N+1 input without
adding a second queue, second socket, or a generation field to every MPEG DATA
frame.

## Ordered-fence mechanism

The existing one PSTV connection is ordered TCP. Pi retirement therefore closes
new scheduler admission for generation N and waits for any send already holding an
emission lease to finish **before** stopping/draining the local producer. The exact
RETIRE completion is sent only after that Pi cleanup succeeds. Every generation-N
DATA frame written before completion is therefore earlier than completion on the
same Pi->PS2 byte stream.

The sole PS2 receiver continues accepting channel-4 DATA for N until it observes
the exact RETIRE completion. When that completion is accepted it immediately
closes MPEG DATA admission. At that instant all earlier N DATA has already either
been consumed by the N worker or is resident in the existing PS2 MPEG ring; any
later channel-4 DATA is a protocol error.

The application then stops/joins the exact N worker. With no producer-side sends
in flight and no consumer reading the ring, transport finalization atomically
discards the residual N bytes under the existing MPEG queue semaphore, returns all
withheld credit (previous batched credit plus discarded queued bytes), and clears
the exact retirement latch. A new generation may open only when no prior data
generation/retirement is active, the ring is empty, and no old MPEG credit remains
pending.

This is an epoch boundary, not per-packet tagging.

## Public-gate boundary

Item #10 remains closed in this tranche. `H1Cp2pMpegProducer` therefore starts with
its emission fence closed and the CP2P runner does not call `open_emission_exact()`.
The future #10 scheduler must acquire/release the exact-generation emission lease
around each MPEG `_send_from`/PSTV write and may open the fence only for the current
prepared generation after the public MPEG CONFIG gate accepts the all-guns mode.

## Required proof

- producer emission fence is closed by default and exact-generation only;
- retirement closes admission and waits for an in-flight send lease before process
  stop/drain;
- queue discard empties wrapped data while preserving high-water telemetry;
- coordinator opens transport generation before START and aborts it on START-send
  failure;
- exact Pi ACK closes PS2 channel-4 admission but leaves retirement latched;
- old worker clear precedes residual queue discard/final-credit return;
- fresh generation cannot open until the old boundary is fully finalized;
- pinned PS2 build links the generation-open/abort/finalize and queue-discard APIs;
- public #10 gate remains closed and no live MPEG DATA transmission is claimed.
