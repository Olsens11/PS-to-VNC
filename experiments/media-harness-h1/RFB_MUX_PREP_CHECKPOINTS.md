# H1 RFB mux preparation checkpoints

This file is the lightweight chronological checkpoint ledger for repository-only
RFB-over-mux preparation. Each completed checkpoint should record the exact
commit, what changed, what was verified without PS2 hardware, what remains
unproven, and the next permitted step.

The governing plan is `RFB_MUX_INTEGRATION_PREP.md`; this ledger must not become
a second design authority.

## CP0 — preparation authority captured

Status: COMPLETE

Repository-only preparation documents now record the one-physical-stream
invariant, current blockers, the existing `rfb_io.h` seam, the 32768-byte
receive-buffer precedent, the RFB-only first hardware milestone, the later
single-compositor requirement, and the hardware/PT_LOAD gate.

No PS2 runtime source behavior was changed by CP0.

Next permitted checkpoint: isolate subsequent source preparation on a dedicated
H1 RFB-mux prep branch, then perform a compile-time seam audit before enabling
RFB CONFIG or channel-1 runtime behavior.
