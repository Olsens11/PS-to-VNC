# Reconstruction B interactive substitute — A001 sole receiver

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T22:58:52-04:00
COMPLETED_AT=2026-09-15T23:22:41-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive-b
STATUS=PARTIAL
FOREMAN_GOAL_RESULT=PARTIAL
STARTING_BRANCH_COMMIT=24e174ad19651df171ece4b56de30732d904814e
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
RECONSTRUCTION_CONTRACT_REVISION=0005
RECONSTRUCTION_STATE_REVISION=0007
FOREMAN_STATE_REVISION=0002

## Shift identity

This was the interactive substitute for the Reconstruction Foreman's 2026-09-15 23:00 `recon-b` packet. Work followed current committed repository authority rather than the prompt snapshot where they differed.

## Authority and collision handling

Startup consumed `AGENTS.md`, `CONTRIBUTING.md`, `docs/CLEAN_ARCHITECTURE.md`, reconstruction contract revision 0005, immutable work-log contract revision 0001, Foreman state revision 0002, reconstruction/global/validation authority, A001 audit authority, the 21:30 Reconstruction B log, the 22:30 Reconstruction A log, and newer continuity/validation evidence.

The branch began at `24e174ad19651df171ece4b56de30732d904814e`. Before each committed source write, branch/Foreman/reconstruction authority was re-read. Concurrent Validation later advanced the branch through V005 and its immutable validation log; those documentation-only commits were consumed rather than overwritten. Final pre-log branch authority was `5b42d3020304b2619642421fe3ea2c3d36ebf1fe`.

## Work already complete and not repeated

The 21:30 Reconstruction B work remained present and was not recreated or revert/reapplied:

- `57c900703a355d9d1d501213572d375e4e859bef` — RFB bridge finite-session quiesce operations;
- `4f64bf102a3f15763ba078c4c57056bac47b2209` — ordered REQUEST -> BOUNDARY -> COMMIT -> residual snapshot/discard -> COMPLETE coordination;
- `6c6359f3ef8ecf1d5ffb5ad024249247de0f7a6f` — bridge quiesce behavior-test source.

The 22:30 A tooling blocker was disproven. The exact `rfb_session.c` blob was obtained by SHA after bounded reads, so whole-file source authority was available without source-reassembly guesswork.

## Committed reconstruction work

### `73e1d65d7f276024ae919767dfaaffa956895e0c` — `reconstruct(a001): migrate RFB session to logical bridge`

Changed:

- `src/rfb/rfb_session.c`
- `src/rfb/rfb_session.h`
- `tests/unit/rfb_session_test.c`
- `tests/unit/rfb_initial_frame_test.c`
- `tests/unit/rfb_initial_coverage_test.c`
- `tests/unit/rfb_async_framing_test.c`

Result:

- removed `socket_fd` from RFB session state/start contract;
- removed direct `rfb_io` use from session protocol I/O;
- routed exact reads, readiness polling, and exact writes only through `pstvnc_rfb_bridge_*`;
- preserved handshake/security/ServerInit/name, GS555 pixel format, Raw encoding setup, initial non-incremental request, update requests, pointer/key wire serialization, Raw rectangle parsing, complete initial-frame coverage proof, live-idle result, framebuffer invalidation and fail-closed semantics;
- integrated quiesce only at the top of the complete-server-message service loop before poll/read of the next message type;
- once a server message begins, exact reads finish that message before quiesce can complete;
- migrated all four existing RFB session/parser fixtures to logical bridge stubs;
- added logical I/O failure and safe-boundary quiesce test source, including a Bell-boundary case proving the next message byte remains unread when REQUEST becomes visible after the Bell completes.

### `fe07a9c22b684a7bd2b9d59a24b7bc6ab328ac27` — `reconstruct(a001): adopt Transport in application lifecycle`

Changed:

- `src/app.c`
- `src/app.h`
- `src/platform/ps2_network.c`
- `src/platform/ps2_network.h`

Result:

- added a configuration-parametric application entry accepting `const pstvnc_transport_session_config_t *`;
- left the no-argument production entry fail-closed while no validated clean CONFIG producer exists;
- application transfers its accepted descriptor through `pstvnc_transport_session_open(&socket_fd, config)` and requires caller state to become `-1` after adoption;
- Transport open precedes RFB session init/start and RFB receives no physical descriptor;
- post-adoption application cleanup never directly closes the descriptor;
- platform direct RFB exact-read/poll/write ownership was removed, eliminating the former second physical receive path;
- cleanup structurally waits for receiver completion before Transport close/resource reclamation.

### `9790ae22f6db4d0aee20f822c9fd6c51d1c7d129` — `test(a001): cover application Transport ownership`

Added `tests/unit/app_transport_lifecycle_test.c` with test-only CONFIG sentinels and fail-closed production-entry coverage. During the same shift I re-evaluated this fixture and concluded it is insufficient by itself for B5 descriptor-transfer/teardown proof and therefore do **not** count it as completing B5. The existing `tests/unit/app_test.c` also still carries its pre-migration RFB-session stub/signature and requires coherent migration. This is recorded as residual work rather than falsely claiming test completion.

## Foreman A acceptance criteria

- **A1 — MET by committed source.** `rfb_session` no longer owns/requires a physical descriptor or old `rfb_io` seam; protocol bytes cross only `pstvnc_rfb_bridge_*`.
- **A2 — MET by committed source/test preservation.** Handshake, security, ServerInit/name, pixel/encoding setup, Raw parsing, initial coverage proof, requests, input serialization, live-idle and fail-closed behavior remain represented. Executable PASS is not claimed.
- **A3 — MET by committed source.** Quiesce completion is called only at the explicit complete-message boundary before consuming the next message byte; Transport still does not know RFB framing.
- **A4 — MET at behavior-test-source level / PENDING_LOCAL executable evidence.** Existing session/framing/initial-frame fixtures were migrated and logical failure plus safe-boundary quiesce cases were added. No host execution was available from this worker surface.
- **A5 — MET.** No Transport CONFIG defaults, timeout, second receive owner, or inherited receive-poison workaround was introduced.

## Foreman B acceptance criteria

- **B1 — MET by source shape.** Configured application path adopts the physical descriptor through Transport exactly once, RFB receives no descriptor, and caller state prevents a post-adoption direct close.
- **B2 — PARTIAL.** Transport open precedes RFB use and source structurally withholds resource reclamation until receiver completion. However newer Validation finding V005 correctly identifies that an application-local fatal path can wait for completion without first causing a healthy blocked receiver to converge; deterministic teardown is therefore not complete.
- **B3 — PARTIAL / exact authority gap established.** No numeric values were invented. Pinned H1 forensic authority `3426f28...` shows `h1_config.h` CONFIG v4 carries the relevant RFB queue/credit and receiver thread/payload fields and explicitly states that those values are requested by the Pi for every session rather than fixed safe defaults. `h1_transport_runtime.c` receives/validates CONFIG before allocating channel resources and starting the receiver. Current clean `src/config/` still has no adopted typed producer for the eight `pstvnc_transport_session_config_t` members. The configured application lifecycle is therefore intentionally parameterized and production no-arg startup remains fail-closed.
- **B4 — MET for descriptor ownership; PARTIAL for complete fatal lifecycle.** Pre-adoption failure keeps caller ownership; post-adoption caller state is `-1`, preventing double close. V005 remains the separate convergence defect: ownership is unambiguous but a healthy blocked receiver may not be caused to terminate before the wait.
- **B5 — PARTIAL.** Existing Transport bridge tests already cover Transport-level adoption/failure/receiver-close guards, and this shift added a small application CONFIG/fail-closed fixture, but the legacy `app_test.c` fixture has not yet been coherently migrated to the new session/Transport API and the new small fixture is not sufficient to claim descriptor/teardown edge coverage.
- **B6 — MET.** No A002 behavior was started.

## New validation finding consumed during this shift

Validation revision 0005 / finding revision 0004 opened **V005 HIGH** against `fe07a9c...`: application fatal teardown may block forever because `pstvnc_transport_session_wait_receiver_done()` waits for an event but application-local failure does not currently request/cause the healthy receiver blocked in physical receive to terminate.

Second-pass review found relevant pinned H1 evidence: H1 fatal transport shutdown explicitly initiates socket shutdown before waiting for receiver/thread convergence and only then releases the socket/resources. Current clean `physical_stream` already privately owns the adopted socket, but the reconstructed Transport API has no equivalent application-requested fatal-abort/convergence operation yet. That is the next behavior-design seam. No timeout, forced reclaim, second receiver, or silent receive-poison workaround was added in response to V005.

## Self-corrections / non-committed attempts

- Rejected the earlier Reconstruction A claim that truncated ordinary file responses made the RFB migration tooling-blocked; exact blob-SHA retrieval supplied the full source safely.
- Did not count `app_transport_lifecycle_test.c` as B5 completion after recognizing that its first version is only a CONFIG/fail-closed sentinel fixture, not full lifecycle proof.
- During final review, detected that the platform connection constants still identify the pre-ledge direct-VNC endpoint (`5900`) while pinned H1 PSTV authority uses `192.168.50.1:5902`. A safe header correction was attempted but GitHub returned a 409 because concurrent Validation had advanced the branch/file authority. No overwrite occurred. The correction was not retried after V005 landed because the newer HIGH teardown finding and the approaching 23:30 Foreman wake made a larger unreviewed product write inappropriate.

## Tests / checks actually run

- Repository/authority/static source inspection: performed.
- Exact GitHub blob-SHA retrieval for the full RFB session source: performed.
- Host compilation/unit execution: **PENDING_LOCAL**. A local `git clone` attempt from the execution shell failed because that shell could not resolve `github.com`; the connected GitHub repository API remained usable for source work. No host PASS is claimed.
- Canonical `scripts/check.sh`: **PENDING_LOCAL**, not run.
- PS2DEV compile/link: **PENDING_LOCAL**, not run and no PASS claimed.
- Reproducible ELF/PT_LOAD/runtime identity: **PENDING_LOCAL**, not run and no PASS claimed.
- Hardware qualification: **HARDWARE_PENDING**, not run and no PASS claimed.

## Exact residual work for the 23:30 Foreman

1. Route V005 back to Reconstruction as the highest-priority A001 behavior defect. Reconstruct an authority-consistent application-requested fatal Transport convergence operation using pinned H1 shutdown behavior as evidence: Transport must cause its privately owned physical receive to unblock/terminate, then prove receiver completion, then reclaim resources. Do not add a timer, unsafe close-before-completion, second receive owner, or conceal the inherited receive-poison boundary.
2. Migrate `tests/unit/app_test.c` coherently to `pstvnc_app_run_with_transport_config(...)`, the descriptor-free `pstvnc_rfb_session_start(...)`, and Transport lifecycle stubs/assertions. Cover pre-adoption failure, post-adoption failure/no double-close, fatal convergence ordering, and no reclaim before receiver completion. Treat the small `app_transport_lifecycle_test.c` as insufficient until replaced/expanded or removed.
3. Establish/adopt a real clean typed producer for all eight `pstvnc_transport_session_config_t` members from explicit reviewed authority. H1 proves dynamic per-session CONFIG semantics, not fixed defaults; do not copy arbitrary historical profile numbers into application code.
4. Correct the physical endpoint/call-site naming from direct VNC to PSTV and use the H1-authoritative `192.168.50.1:5902` only through a deliberate current clean-source change after re-reading authority.
5. Foreman-owned integration remains: dictionary/topology/portal/build/check wiring, executable host evidence, PS2DEV/reproducibility/PT_LOAD evidence, then independent Validation. V004 and V005 both remain open.
6. Keep A001 active; do not begin A002.

PENDING_LOCAL=host unit execution; canonical scripts/check.sh; PS2DEV compile/link; reproducibility and exact ELF/PT_LOAD identity; migrated application lifecycle fixture execution.
HARDWARE_PENDING=YES; no reconstructed A001 hardware qualification is claimed.
