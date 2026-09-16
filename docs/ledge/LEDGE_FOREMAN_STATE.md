# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0002
RECORDED_AT=2026-09-15T22:04:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0011
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This state is the current planning baton for the Reconstruction Foreman. It owns quantified A/B shift goals plus the non-behavioral integration/evidence chore queue. It does not supersede reconstruction product-source authority, audit dispositions, validation findings, or global continuity authority.

## Current foreman phase

`A001_GOAL_DIRECTED_LIVE_PATH_MIGRATION`

The first Foreman goal epoch is based on branch HEAD `39d9cfac52e232a1e158eb83dc17a11acc788ce1`, reconstruction state revision 0007, and the transition Reconstruction B shift that ended after approximately 2 minutes 8 seconds. That B shift added RFB-side quiesce bridge operations and behavior tests but explicitly left the coherent `rfb_session.*` live-path migration undone. The Foreman therefore sizes the next goals around whole behavioral seams rather than another bridge micro-edit.

## Crew cadence

- Reconstruction B anchor: `2026-09-15T21:30:00-04:00`, every 90 minutes;
- Foreman anchor: `2026-09-15T22:00:00-04:00`, every 90 minutes;
- Reconstruction A anchor: `2026-09-15T22:30:00-04:00`, every 90 minutes.

## Goal packet — Reconstruction A

TARGET_START=2026-09-15T22:30:00-04:00
WORKER=recon-a
WORK_ITEM_KEY=a001-sole-receiver
GOAL_STATUS=ASSIGNED
ASSIGNING_HEAD=39d9cfac52e232a1e158eb83dc17a11acc788ce1
ASSIGNING_RECONSTRUCTION_STATE_REVISION=0007

### Objective

Convert the complete clean RFB session protocol path from the pre-ledge socket-shaped `rfb_io` seam to the logical `src/rfb/bridge.*` stream, including parser-safe finite-session quiesce at complete server-message boundaries, without changing RFB protocol semantics or inventing Transport configuration.

### REQUIRED_DELIVERABLES

1. Migrate `src/rfb/rfb_session.h/.c` so session protocol mechanics no longer own/accept a physical socket descriptor or `pstvnc_rfb_io_t`; exact reads, readiness polling, and exact writes use the RFB bridge.
2. Preserve the existing handshake, ServerInit/name handling, Raw framebuffer parser, update-request behavior, pointer/key serialization, initial-frame/coverage behavior, and fail-closed result semantics while changing only the stream seam.
3. Integrate the already reconstructed quiesce bridge into session servicing: observe quiesce requests only at parser-safe service points and execute complete quiescence only after a complete server message with no partially consumed next message.
4. Update/add behavior-specific RFB session tests necessary to prove the new logical-stream seam and safe-boundary behavior; test-source work is Reconstruction ownership even though canonical Makefile registration remains Foreman ownership.

### ACCEPTANCE_CRITERIA

A1. No public or private `rfb_session` runtime contract requires `socket_fd` or `pstvnc_rfb_io_t` for RFB protocol I/O; the session implementation reaches transport bytes only through `pstvnc_rfb_bridge_*`.
A2. Existing RFB handshake/parser/update/input behavior remains represented in source and relevant tests rather than being deleted or bypassed during migration.
A3. Quiesce completion is reachable only from an explicit complete-message/idle-safe boundary; no quiesce operation can consume a partial next server message.
A4. Behavior-specific tests cover at least logical exact-I/O failure propagation and quiesce request/completion at a safe session boundary, in addition to preserving existing framing/initial-frame tests.
A5. No Transport CONFIG defaults, physical receive ownership, generic timeout, or inherited receive-poison workaround is introduced.

### INVARIANTS / NON_GOALS

- One physical PSTV socket owner remains Transport.
- RFB owns complete-server-message parser boundary policy; Transport does not learn RFB framing.
- Parser-consumed bytes and quiesce residual discard remain distinct credit facts.
- Do not migrate `app.c`/platform Transport lifecycle in this packet unless required only to keep a behavior-specific host fixture compiling; that coherent ownership transfer is reserved for the following B packet.
- Do not begin A002.

### EXPECTED_EVIDENCE

Source/test evidence for the migrated RFB session seam. Executable host/PS2DEV results are welcome if genuinely available, but absent tool execution is PENDING_LOCAL rather than a reason to weaken the source acceptance criteria.

### BLOCKER_CONDITIONS

Stop/pivot only for contradictory newer authority, an actual collision on the same RFB session source, or discovery that the existing parser contract cannot identify a complete-message boundary without a separately authorized behavioral redesign. A missing Transport CONFIG authority is not a blocker for this RFB-only packet.

### FALLBACK_OR_STRETCH

If A1-A5 are met early, trace and remove any now-dead socket-shaped RFB platform adapter declarations/callers that are purely consequences of the completed session migration and do not yet require choosing Transport session configuration. Otherwise leave exact dead-call-chain evidence for the Foreman/B packet.

## Goal packet — Reconstruction B

TARGET_START=2026-09-15T23:00:00-04:00
WORKER=recon-b
WORK_ITEM_KEY=a001-sole-receiver
GOAL_STATUS=ASSIGNED_CONDITIONAL_ON_A
ASSIGNING_HEAD=39d9cfac52e232a1e158eb83dc17a11acc788ce1
ASSIGNING_RECONSTRUCTION_STATE_REVISION=0007

### Objective

Assuming Reconstruction A completes the RFB logical-stream conversion, migrate application/session ownership onto the reconstructed Transport lifecycle so the accepted physical socket is transferred exactly once to Transport and the application drives RFB through the logical session path, using explicit validated configuration authority rather than guessed defaults.

### REQUIRED_DELIVERABLES

1. Re-read A's result and current HEAD. If A is MET, migrate the coherent `src/app.c` / relevant platform/session call chain so application hands the accepted physical descriptor to `pstvnc_transport_session_open()` and does not later double-close a successfully adopted descriptor.
2. Establish the concrete source of `pstvnc_transport_session_config_t` from existing explicit validated/qualified authority. If no such authority exists in current clean source, do not invent values: implement only the ownership/lifecycle portion that can remain configuration-parametric and record the exact missing authority as a blocker to live startup.
3. Route application RFB startup/service/shutdown through the newly logical RFB session plus Transport lifecycle, preserving explicit receiver completion before Transport reclamation and first-failure/fail-closed semantics.
4. Add/update behavior-specific application/lifecycle tests for descriptor transfer, failed-open ownership, no double-close, and teardown ordering as appropriate to the implemented seam.

### ACCEPTANCE_CRITERIA

B1. On the coherent live path, application no longer gives RFB a physical VNC/PSTV socket for protocol I/O; a successfully adopted descriptor is Transport-owned and caller state prevents double close.
B2. Transport session open precedes logical RFB use; Transport close/reclamation does not occur until receiver completion is established by the reconstructed lifecycle.
B3. The source of every populated Transport session-config field is explicit and reviewable. No numeric queue/credit/thread/payload value is fabricated merely to make the path compile.
B4. Failure paths preserve unambiguous ownership: failed pre-adoption leaves caller ownership; failed post-adoption does not cause application/platform to close the same descriptor again.
B5. Behavior-specific tests exercise the ownership/teardown edge cases introduced by this migration, or the log identifies a genuine executable-fixture boundary with exact source evidence.
B6. A001 remains the only active tranche; no A002 behavior is started.

### INVARIANTS / NON_GOALS

- Preserve one physical owner, RFB safe-boundary ownership, parser-consumption credit semantics, receiver-completion-before-reclaim, and the unresolved inherited receive-poison boundary.
- Do not choose new tuning/default policy in application code.
- Do not perform Foreman-owned dictionary/topology/Makefile chores as substitute work.

### BLOCKER_CONDITIONS

If A is PARTIAL/BLOCKED such that `rfb_session` still has socket-shaped protocol authority, do not create a competing half-live application path. Instead execute the fallback below. If validated Transport configuration authority is genuinely absent, complete all safe parametric ownership/lifecycle work, record the exact configuration gap, then use fallback rather than guessing defaults.

### FALLBACK_OR_STRETCH

If A has not completed the RFB seam, finish the remaining A packet acceptance criteria where safe and non-colliding. If configuration authority blocks live app startup after A is complete, perform the next safe A001 behavior tranche: consolidate/clarify the application-facing session lifecycle API so configuration remains an explicit caller-supplied value and add ownership/failure tests, without assigning numeric defaults. If B1-B6 are met early, trace obsolete direct RFB socket/platform adapters and remove only those proven unreachable from the clean live path.

## Foreman chore queue

Highest priority after publishing these packets:

1. wire `transport_bridge_test` and `rfb_bridge_test` into canonical `tests/Makefile` without changing DUT behavior;
2. reconcile `src/rfb/SYMBOLS.md` with new bridge definitions and complete the current `src/transport` definition inventory;
3. deliberately adopt `src/transport` into source topology/checker/domain allowlists and regenerate the source-dictionary portal through canonical tooling where executable;
4. preserve repository-native host/static evidence actually executable from this worker surface;
5. prepare build/linkage/reproducibility evidence plumbing as A001 becomes coherent;
6. preflight the exact validated Transport configuration owner/source for later B/live-path work without inventing policy.

## Current evidence boundary

A001 remains not `VALIDATION_READY`. No host test/build/PS2DEV/ELF/PT_LOAD/reproducibility/hardware PASS is implied by these assignments. PENDING_LOCAL and HARDWARE_PENDING remain distinct from source completion.

## Exact next Foreman pickup

Inspect A's 22:30 result, B's 23:00 result if available at next wake, and criterion-level `FOREMAN_GOAL_RESULT`. Recalibrate packet size from MET/PARTIAL/BLOCKED evidence. Continue A001 until the live Transport/RFB/application path is coherent; do not schedule A002 merely because metadata chores are complete.