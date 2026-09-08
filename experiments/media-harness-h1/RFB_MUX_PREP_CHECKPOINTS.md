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

## CP1 — clean RFB parser mechanically isolated from direct socket I/O

Status: COMPLETE — REPOSITORY/BUILD VERIFIED, NOT HARDWARE QUALIFIED

Branch:

    experiment/h1-rfb-mux-prep

Source/build commits in this checkpoint:

    3ef8f7c4548183701a22b0089304baf6e47ba38b  h1: add fail-closed RFB mux I/O seam header
    e5438fc15691ab79ea82ab009d8c7d124fd041d8  h1: add fail-closed RFB mux I/O seam
    a0128c2a213c81d8138f432f0f40e9872f222b94  h1: bind cumulative RFB session to mux adapter seam
    2314fe6478b5eaae640e86f55b3d24fb78152826  h1: add RFB mux seam checker
    d22cb6b91707e1458a93ec7be70c1733b2779ae5  ci: add H1 RFB mux preparation checks
    1c59c81bd8ac0509862e1596e2ba45575b0e7c91  ci: install make for H1 RFB prep build
    e8dc3999248c468add4c561cbdfded0100a4c9de  ci: add readelf dependency for H1 fingerprint

What changed:

- `src/rfb/rfb_session.c` itself remains byte-for-byte untouched.
- Only the cumulative H1 build compiles that translation unit with mechanical
  preprocessor renames from the three clean `pstvnc_rfb_io_*` symbols to
  experiment-owned `pstvnc_h1_rfb_mux_io_*` symbols.
- `h1_rfb_mux_io.c` currently provides deliberately fail-closed adapter bodies;
  all three operations immediately fail.
- `rfb_mode=ON` is still rejected by the H1 CONFIG validator, so these adapter
  entry points are not reachable in current RFB-OFF media sessions.
- A saved checker verifies both the source/build rule and, after a PS2 build,
  the actual object-symbol ownership: `rfb_session39.o` references the mux names
  and not the direct socket names; `h1_rfb_mux_io.o` defines the mux names.

CI/build evidence:

    workflow=H1 RFB mux preparation checks
    run=34227486371
    result=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_toolchain=ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11
    ELF_SHA256=d5d17f7318b90468c44cc098cf6229090cfcf84e68c9fbf00a8ecc24f3f2204b
    PT_LOAD_SHA256=038b5048c10eff4802559abb79a41ec2b01a00a70b8cc97762b9c5b7a878cafc
    PT_LOAD_BYTES=479380
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf

What CP1 proves:

- H1 can compile/link the clean RFB session against an experiment-owned I/O
  adapter without editing RFB parser semantics.
- The cumulative target no longer needs to let that parser resolve its I/O calls
  to the direct VNC socket functions.
- The pinned PS2 build still links successfully with the through-Issue-39 module
  population plus the fail-closed seam.

What CP1 does **not** prove:

- RFB channel 1 can carry bytes.
- any RFB queue/credit behavior exists.
- the Pi can bridge VNC to PSTV channel 1.
- `--rfb on` is operational.
- the changed PT_LOAD works on real PS2 hardware.

The produced ELF is intentionally preserved as **unqualified** CI evidence only.
No deployment or hardware claim is authorized by this checkpoint.

Next permitted checkpoint: add the dedicated channel-1 RFB queue/credit and
serialized logical-write mechanics while preserving `rfb_mode=OFF` inertness;
keep CONFIG RFB=ON gated until those mechanics and their host/build checks are
complete.
