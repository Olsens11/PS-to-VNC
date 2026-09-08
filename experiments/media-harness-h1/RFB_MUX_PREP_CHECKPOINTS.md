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

## CP2A — logical RFB channel mechanics isolated and host-tested

Status: COMPLETE — HOST VERIFIED, NOT RUNTIME-WIRED, NOT HARDWARE QUALIFIED

Commits:

    826a4130c1a55702f5ee7455a856929fd8138138  h1: add testable RFB channel mechanics
    b9665290eb44c4f465bdb91d7d1612ab625a16cd  h1: implement testable RFB channel mechanics
    956427caf74a7fcb58f8036f2f5b677a54649c59  h1: test RFB channel mechanics
    5de62aa3eca1c8dc027cdd05653e1efc99ce8faf  ci: test H1 RFB channel mechanics

What changed:

- Added an experiment-owned, platform-neutral logical RFB channel component.
- Queue storage remains caller-owned; the component itself performs no
  allocation, socket I/O, semaphore/thread work, CONFIG activation, or RFB
  parsing.
- The only accepted queue capacity is the already-recorded 32768-byte direct-RFB
  precedent; this checkpoint does not enlarge it.
- Inbound DATA is all-or-nothing queued and independently counted.
- Consumed bytes accumulate exact receiver credit for the runtime to return.
- Outbound logical RFB writes are fragmented at a caller-provided maximum
  payload and handed to a sender callback in exact byte order.
- Partial sender failure fails the logical write rather than reporting success.

Verification:

    workflow=H1 RFB mux preparation checks
    run=34230117829
    result=PASS
    host_rfb_channel_test=PASS
    existing_rfb_mux_seam_check=PASS
    pinned_ps2_build=PASS

The host test proves queue ordering, high-water accounting, consumed-byte
credit, exact outbound fragmentation, and fail-closed partial-send behavior.
The cumulative PS2 build also remains green, but this new component is not yet
linked into the live H1 runtime, so no PT_LOAD/runtime qualification claim is
made from CP2A.

What remains unproven:

- H1 receiver-thread dispatch of PSTV channel-1 DATA into this queue;
- semaphore ownership around the queue;
- actual CREDIT frames on channel 1;
- actual serialized channel-1 DATA writes through H1's existing send lock;
- adapter binding/read/poll behavior;
- Pi VNC bridge behavior;
- `rfb_mode=ON` activation or any hardware behavior.

Next permitted checkpoint: wire this tested component into the H1 transport
runtime while keeping CONFIG `rfb_mode=ON` rejected. The runtime integration
must create RFB-specific resources only when RFB is enabled, use the existing
single receiver and send-lock owners, and add build/host checks before the
activation gate is relaxed.

## CP2B — logical RFB channel mechanics resident in the pinned PS2 build

Status: COMPLETE — BUILD VERIFIED, STILL NOT LIVE-RUNTIME-WIRED, NOT HARDWARE QUALIFIED

Commits:

    8e6ddad3219ee2eff768a3e29745850e9c9e1204  h1: link RFB channel mechanics into cumulative prep build
    197b7a80f741a493ee13be876352e1902c3ec585  h1: verify resident RFB channel mechanics object

What changed:

- The host-tested `h1_rfb_channel.c` component is now compiled and linked into
  the cumulative through-Issue-39 H1 preparation ELF.
- The existing seam checker now requires the resident `h1_rfb_channel.o` object
  and verifies the expected queue/credit/fragment function definitions in the
  actual PS2 object.
- The fail-closed mux adapter remains unchanged and `rfb_mode=ON` remains
  rejected. No RFB queue storage, semaphore, receiver dispatch, credit frame,
  or channel-1 send is activated by this checkpoint.
- No queue capacity changed; the channel component still accepts only the
  recorded 32768-byte reference capacity.

Verification:

    workflow=H1 RFB mux preparation checks
    run=34241770439
    result=PASS
    host_preflight=PASS
    host_rfb_channel_test=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_ps2_build=PASS
    ELF_SHA256=200dc880f15495dd31866f65c2ac082070e6f964a6620e57efcab6ddbd5bf0a0
    PT_LOAD_SHA256=00574addb8a96e350db14046ce1acca013d25d59cb196f47104109e9be8d7262
    PT_LOAD_BYTES=480532
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf

What CP2B proves:

- The exact logical RFB channel implementation already proven by host tests also
  compiles and links under the pinned PS2 toolchain in the resident H1 source
  population.
- Future live runtime wiring can reference this component without first changing
  the cumulative ELF's source population again.

What CP2B does **not** prove:

- H1 transport runtime resource ownership for RFB;
- receiver-thread channel-1 dispatch;
- channel-1 CREDIT or DATA transmission;
- mux-adapter read/poll/write operation;
- Pi bridge behavior;
- `RFB=ON` activation;
- any real-PS2 behavior of this changed PT_LOAD.

The recorded ELF remains an **unqualified** build artifact only.

Next permitted checkpoint: add the RFB-specific resource/lifecycle and
receiver/send integration inside `h1_transport_runtime` while continuing to
reject `rfb_mode=ON`. That checkpoint must preserve the sole physical `recv()`
owner and existing send semaphore, allocate no RFB queue while RFB is OFF, and
must not activate the mux adapter until those runtime mechanics are separately
verified.
