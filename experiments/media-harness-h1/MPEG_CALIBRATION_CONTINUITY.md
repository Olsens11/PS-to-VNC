# MPEG calibration continuity handoff

## Session identity

- Timestamp: 2026-09-12 07:15 EDT / 11:15 UTC
- Branch: `experiment/h1-cp2p-start-wire-accepted`
- Qualified source parent: `d3a6ba722695ab6f0f78f034bcf14b6daed7a422`
- Qualified source tree: `50dd7ebd1232aa4bcb36edecd252217ebc9c336a`
- Implementation/evidence HEAD before this handoff update: `71874a91f24bc706858f8c2ec739f80f98b4ad89`
- Implementation/evidence tree before this handoff update: `3fc23300a66a940d97f1d4e2ab5f554d31e1a4b6`
- The canonical handoff commit containing this file is the branch HEAD immediately after this update.
- The qualified branch `experiment/h1-rfb-mux-prep` was not moved or modified.
- No Pi worktree reset, clean, discard, overwrite, merge, or `src/` mutation was performed. Pi-local dirty state remains **unknown** because this tranche was implemented through the GitHub repository connection rather than David's live worktree.

## Continuity reconciliation

The previous copy of this handoff stopped at the September 11 calibration-binding milestone around `908250ae93a8a0801189b0113360adc0e21ba08a`. Direct work continued afterward on `experiment/h1-rfb-mux-prep` and produced the live calibration integration, native calibration presentation, CP2P presentation owner/start handoff, shared compositor preparation, generated CP2P MPEG runtime, and composed CP2P RFB ownership flow through qualified source commit `d3a6ba722695ab6f0f78f034bcf14b6daed7a422`.

One earlier source-inspection discrepancy is resolved: `h1_mpeg_start_handoff.c` does exist at `d3a6ba...`; its exact path is:

`experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_handoff.c`

It was introduced at commit `698929bfb08c4f03687016f7c660b8aacb1abaf9`. It is an in-memory generation/geometry ownership handoff, not a PSTV wire serializer.

## Larger push objective

Produce the first hardware-testable CP2P "all-guns" H1 composition with:

- visible RFB remote desktop and real through-Issue-39 interaction;
- canonical PCM audio;
- MPEG presentation inside the accepted calibrated region;
- one existing PSTV physical TCP connection;
- one PS2-side receive owner;
- generation-scoped MPEG ownership and RFB suppression;
- first-frame ownership only after physical GS presentation;
- exact-generation retirement and exactly one RFB full-refresh restoration path.

The final all-guns activation remains restricted to the intended visible-RFB + PCM + MPEG composition. Do not reintroduce competing PS2-facing sockets or independent receive owners.

## This session objective

Complete only the first narrow CP2P tranche after source reconciliation:

1. preserve and expose the calibration core's existing one-shot `accepted` edge through the runtime/binding boundary;
2. define and host-test the agreed fixed MPEG START wire payload;
3. send that payload only through H1's existing serialized PSTV writer;
4. preserve current CP2O behavior and prove its PS2 target still builds;
5. stop before Pi START decoding, RFB suppression, MPEG producer activation, CP2P worker integration, or hardware testing.

## Source facts established before implementation

### Transport authority

The existing PSTV protocol uses:

- 16-byte fixed frame header;
- `PSTVNC_TRANSPORT_FRAME_DATA = 3`;
- `PSTVNC_TRANSPORT_CHANNEL_MPEG2 = 4`;
- big-endian sequence and payload length;
- existing `pstvnc_transport_read_be32()` / `pstvnc_transport_write_be32()` helpers.

`h1_transport_runtime.c` already owns the socket send semaphore, sequence counter, framing, exact writes, and transport error accounting. The existing experiment-internal bridge:

`pstvnc_h1_transport_send_frame_internal(...)`

routes logical-channel writes through that same authority. No new raw socket writer is needed or permitted.

### Exact accepted-edge drop point

The calibration core already produces `pstvnc_mpeg_cal_effects_t.accepted`. The adapter preserves the effects and the foreground layer preserves the adapter result. The edge was first dropped by:

`pstvnc_h1_mpeg_calibration_runtime_service_controller()`

because its public result previously retained only controller-consumption, RFB-freeze, and calibration-visible facts. The interaction binding then exposed only controller consumption.

The correct repair is propagation of the existing edge, not a second acceptance detector, callback system, polling rule, or inferred `has_committed` transition.

### Profile/config authority

The literal label `rfb-incremental-live-pcm` is not an H1 profile name at `d3a6...`. Current Pi profile names remain `P11_COMPAT_VIDEO_ONLY`, `P11_COMPAT_PLUS_PCM`, and `H1_RFB_ONLY`.

CP2O's real activation gate is structural: visible RFB plus optional canonical PCM, with MPEG OFF, while ordinary H1 validation remains authoritative for the normalized non-RFB fields. Future CP2P all-guns activation should extend the structural composition deliberately rather than compare an invented profile string in C.

## What changed this run

All final source changes are confined to:

`experiments/media-harness-h1/mpeg_presentation_calibration/`

No `src/`, interaction coordinator, CONFIG gate, Pi runner, RFB parser, permanent compositor, qualified branch, or deployment path was changed.

### Accepted-edge propagation

Updated:

- `h1_mpeg_calibration_runtime.h`
- `h1_mpeg_calibration_runtime.c`
- `h1_mpeg_calibration_interaction_binding.h`
- `h1_mpeg_calibration_interaction_binding.c`
- `h1_mpeg_calibration_runtime_test.c`
- `Makefile.host`

Added:

- `h1_mpeg_calibration_accept_edge_test.c`

Behavior:

- `pstvnc_h1_mpeg_calibration_runtime_result_t` now carries `accepted`.
- The runtime copies it directly from `foreground_result.adapter_result.calibration_effects.accepted`.
- A result-rich binding seam now exposes both `consume_controller_state` and `accepted`:
  `pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(...)`.
- The old consume-only `pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(...)` API remains and is implemented as a compatibility wrapper, so current CP2O coordinator behavior does not need to change in this tranche.
- `pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(...)` exposes the existing immutable committed-region accessor without duplicating geometry ownership.

### MPEG START wire contract

Added:

- `h1_mpeg_start_wire.h`
- `h1_mpeg_start_wire.c`
- `h1_mpeg_start_wire_test.c`

The fixed payload is version 1, exactly 11 big-endian 32-bit words / 44 bytes:

1. version
2. session ID
3. generation
4. draw X
5. draw Y
6. draw width
7. draw height
8. suppression X
9. suppression Y
10. suppression width
11. suppression height

The base draw rectangle remains the exact MPEG capture/presentation geometry. The suppression rectangle is separate. Inner matte remains PS2 presentation state and is intentionally not sent to the Pi.

Codec validation requires:

- nonzero generation;
- nonnegative draw coordinates;
- current MPEG-path draw dimensions at least 16 and 16-pixel aligned;
- positive suppression rectangle;
- suppression rectangle contains the complete base draw rectangle;
- decoded integer geometry fits the target `int` representation.

Canvas bounds are not redundantly serialized. The PS2 calibration/start owner and future Pi active-desktop validation remain responsible for their respective bounds.

### MPEG START transport seam

Added:

- `h1_mpeg_start_transport.h`
- `h1_mpeg_start_transport.c`
- `h1_mpeg_start_transport_test.c`

`pstvnc_h1_mpeg_start_transport_send(...)`:

- encodes the exact 44-byte payload;
- emits exactly one PSTV `DATA` frame;
- uses logical `MPEG2` channel 4;
- calls only `pstvnc_h1_transport_send_frame_internal(...)`;
- does not own or expose the socket, send semaphore, sequence counter, or raw write path.

Direction distinguishes this PS2->Pi START-control DATA frame from later Pi->PS2 MPEG2 elementary-stream DATA.

## Final source diff boundary

After verification, the temporary branch-only CI workflow was deleted. Relative to qualified source parent `d3a6ba...`, the final implementation diff before this handoff contained exactly 13 paths, all under `experiments/media-harness-h1/mpeg_presentation_calibration/`:

- modified `Makefile.host`;
- added `h1_mpeg_calibration_accept_edge_test.c`;
- modified interaction-binding `.c/.h`;
- modified calibration-runtime `.c/.h` and runtime test;
- added START transport `.c/.h/test.c`;
- added START wire `.c/.h/test.c`.

This continuity-file update is the only additional documentation path.

## Tests/checks and exact evidence

### Strict host suite

Temporary branch-only workflow run:

- workflow run `34690484266` — **SUCCESS**
- host job `103544731318` — **SUCCESS**
- tested source HEAD `a9037fd12115d48113423394703dfb3e313631b7`
- strict flags remained `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`

Exact relevant PASS markers included:

- `MPEG_CALIBRATION_HOST_TEST=PASS`
- `MPEG_CALIBRATION_ENTRY_HOLD_HOST_TEST=PASS`
- `MPEG_PRESENTATION_OWNER_HOST_TEST=PASS`
- `H1_MPEG_START_HANDOFF_HOST_TEST=PASS`
- `H1_MPEG_START_WIRE_HOST_TEST=PASS`
- `H1_MPEG_START_TRANSPORT_HOST_TEST=PASS`
- `MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS`
- `MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RFB_SCHEDULE_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RFB_FLOW_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RENDER_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RASTER_HOST_TEST=PASS`
- `MPEG_CALIBRATION_RUNTIME_HOST_TEST=PASS`
- `MPEG_CALIBRATION_INTERACTION_BINDING_HOST_TEST=PASS`
- `MPEG_CALIBRATION_ACCEPT_EDGE_HOST_TEST=PASS`
- `MPEG_CALIBRATION_LIVE_WIRING_SOURCE_TEST=PASS`

The acceptance tests prove the edge is true exactly on the REVIEW acceptance observation and false afterward even though committed geometry remains available.

The START transport spy test proves invalid contracts never reach the writer and writer failure propagates to the caller.

### Pinned PS2 CP2O regression build

The same workflow run also executed:

- PS2 regression job `103544731394` — **SUCCESS**
- pinned toolchain image `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Checks passed:

- `CP2O_RFB_PCM_SOURCE=PASS`
- `CP2O_RFB_PCM_OBJECTS=PASS`
- `H1_RFB_MUX_SEAM_SOURCE=PASS`
- `H1_RFB_MUX_SEAM_OBJECTS=PASS`
- `H1_RFB_MUX_SEAM=PASS`
- `TESTKIT_PT_LOAD_FINGERPRINT=PASS`

Regression ELF identity from that build:

- SHA256: `0f30ae41ac7cf77ddf7f0a9d8e2c40639b6cfd03d232363cca3799e79f6969ad`
- bytes: `3180016`
- PT_LOAD segments: `1`
- PT_LOAD SHA256: `8737d4c3735f165d667b63d3a1c3f174ebd0db96ba17d402b55cabdd5e34d08c`
- PT_LOAD bytes: `507412`

This is **regression-build evidence only**. It is not a new CP2P candidate and is not hardware-qualified. The new START wire/transport modules are deliberately not linked into this CP2O ELF.

The temporary branch-only workflow file used to obtain this evidence was removed after both jobs passed. The Actions runs remain repository evidence.

## Proven through this tranche

- The real calibration one-shot accepted edge is no longer lost at the runtime boundary.
- The accepted edge reaches an experiment-local CP2P-capable binding result without changing the existing CP2O consume-only API.
- Committed calibration geometry remains owned by the existing runtime and can be borrowed through the binding.
- The intended MPEG START payload is now an explicit versioned 44-byte BE32 contract rather than conversation memory.
- The START payload preserves exact base MPEG geometry separately from the RFB suppression footprint.
- The START sender uses the existing H1 serialized PSTV writer and logical MPEG2 channel 4; it introduces no competing socket or send owner.
- Existing strict calibration host contracts remain green.
- Existing CP2O source/object ownership checks remain green.
- The current CP2O PS2 target still compiles and fingerprints under the pinned PS2 toolchain.
- Final implementation remains experiment-local and `src/` remains untouched.

## Still unproven / incomplete

- No CP2P coordinator yet consumes the new result-rich binding edge.
- No live PS2 path yet calls `pstvnc_h1_mpeg_start_handoff_prepare_start()` from the accepted edge.
- No live PS2 path yet calls `pstvnc_h1_mpeg_start_transport_send()`.
- The new START modules are not linked into a hardware candidate ELF.
- Pi-side START decode/validation is not implemented in the current qualified tree.
- Pi-side generation-scoped RFB suppression is not implemented.
- The actual RFB suppression mechanism still requires evidence-based selection after inspecting negotiated encodings/request behavior; do not guess clipping semantics.
- MPEG producer activation is not lifecycle-driven from START yet.
- No concurrent CP2P MPEG worker is integrated into the live RFB+PCM coordinator.
- No CP2P CONFIG activation gate is open for visible RFB + PCM + MPEG.
- No CP2P candidate ELF/PT_LOAD identity exists.
- No deployment, Pi execution, physical PS2 observation, or hardware qualification occurred in this tranche.

## Hardware-qualification boundary

Nothing in this tranche is hardware-qualified. The qualified source/provenance anchor remains `d3a6ba722695ab6f0f78f034bcf14b6daed7a422` and its existing qualification record. The isolated branch is development evidence only until a later exact CP2P candidate is built, fingerprinted, deployed, read back, and physically tested.

Do not describe the CP2O regression ELF above as the CP2P candidate merely because it was rebuilt after these source changes.

## Important design decisions / reconciliations

- Propagate the real accepted edge; do not infer acceptance from `has_committed` or geometry changes.
- Keep the old consume-only interaction-binding API as the CP2O compatibility surface.
- Keep START wire representation separate from presentation ownership and transport ownership.
- Keep the START payload fixed and versioned; use existing PSTV BE32 helpers.
- Keep inner matte PS2-local; Pi needs exact source/capture rectangle plus suppression footprint, not presentation-only inner matte.
- Use the existing PSTV send semaphore/sequence/frame writer; never add a second socket writer.
- Keep the qualified interaction coordinator unchanged until the next isolated CP2P composition tranche.
- Do not modify `src/` for this experimental composition.
- Do not choose an RFB suppression algorithm until actual qualified encoding/request evidence is inspected.

## Current overall push status / milestone

Milestone: **the CP2P coordinator now has a source-proven path available to receive the existing calibration acceptance edge, and the PS2 now has a strict-host-proven 44-byte START codec plus a narrow serialized PSTV send seam. Existing CP2O behavior still builds under the pinned PS2 toolchain. No live START is sent yet.**

This is the intended stop boundary for the accepted-edge + START-wire tranche.

## Single best next action

Create the next isolated CP2P coordinator slice from this branch. In that slice only:

1. consume `pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result()` in a CP2P-specific coordinator/application seam;
2. on the one-shot `accepted` edge, borrow the committed region;
3. call `pstvnc_h1_mpeg_start_handoff_prepare_start()` exactly once to freeze generation, base geometry, and suppression footprint;
4. call `pstvnc_h1_mpeg_start_transport_send()` with the current session ID;
5. if the send fails, abort that exact generation and preserve the existing RFB restoration/full-refresh contract;
6. prove this accepted -> prepare -> START ordering with focused host/source tests and a pinned PS2 compile;
7. stop again before Pi producer activation or live all-guns hardware testing.

After that seam is green, implement the matching Pi START decoder/validator and generation-scoped suppression installation before any MPEG byte is emitted.
