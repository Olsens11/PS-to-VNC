# Ledge Reconstruction Shift A — A001 RFB session migration

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T22:30:00-04:00
COMPLETED_AT=2026-09-15T22:31:15-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=recon-a
STATUS=BLOCKED
FOREMAN_GOAL_RESULT=BLOCKED
STARTING_BRANCH_COMMIT=49cf02950dfe3aae0dbc91de8774311fcf0a8af0
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
RECONSTRUCTION_CONTRACT_REVISION=0005
RECONSTRUCTION_STATE_REVISION=0007
FOREMAN_STATE_REVISION=0002

## Authority checked

The shift read the governing reconstruction contract first, current Foreman packet, reconstruction state, work-log contract, current branch authority, and the live `src/rfb/rfb_session.h/.c` plus current RFB session fixture. Branch authority remained `49cf02950dfe3aae0dbc91de8774311fcf0a8af0` immediately before this immutable write. The Foreman packet remains semantically applicable: `rfb_session` still owns `socket_fd`, includes `rfb_io.h`, and calls `pstvnc_rfb_io_*` directly.

## Foreman acceptance criteria

- A1 — BLOCKED/UNMET. Current `rfb_session` still exposes `socket_fd` in state/start and reaches bytes through `pstvnc_rfb_io_*`.
- A2 — PRESERVED CURRENTLY. Inspection confirmed handshake, ServerInit/name handling, Raw update parser, full-frame coverage proof, update requests, pointer/key publication, and fail-closed behavior remain present. No source mutation was made that could erase them.
- A3 — UNMET. Current live session service has an explicit idle-safe pre-message boundary but does not yet call the reconstructed quiesce bridge there or after complete ignored/server-update messages.
- A4 — UNMET. Existing `tests/unit/rfb_session_test.c` still stubs the old socket-shaped `pstvnc_rfb_io_*` seam; no migrated logical-stream session fixture was safely committed.
- A5 — MET BY NON-CHANGE. No Transport CONFIG defaults, physical receive ownership, generic timeout, or receive-poison workaround was introduced.

## Blocker

The available GitHub mutation surface for this wake exposes whole-file create/update operations but no patch/edit primitive. The assigned behavior goal requires a coherent multi-hundred-line migration of `src/rfb/rfb_session.c`, its public header, and behavior fixture. The current source was retrieved in bounded chunks because the connector truncates the complete file response. Reconstructing and replacing the entire translation unit from manually reassembled truncated connector output would create an unacceptable source-corruption risk and would violate the goal's requirement to preserve all existing parser/handshake semantics. No partial header/API change was committed because that would knowingly leave the branch uncompilable and make the following B packet less safe.

This is an execution-tool boundary, not a repository design blocker. The required migration remains well-defined from current authority.

## Work completed

Read and traced the exact remaining socket-shaped dependencies. Confirmed the safe quiesce insertion rule can be satisfied without parser redesign: readiness/quiesce may be checked before consuming the next message type; once a message begins, exact reads must finish that message before any quiesce completion. Confirmed the existing test fixture is coupled to `pstvnc_rfb_io_*` and therefore must migrate with the session seam.

No reconstructed DUT source changed. Reconstruction state was not advanced.

## Pending evidence

PENDING_LOCAL: migrated host fixture compile/execution, canonical checks, PS2DEV compile, linked ELF/PT_LOAD/reproducibility evidence.
HARDWARE_PENDING: all physical A001 qualification after a coherent machine-validated DUT exists.

## Exact next pickup

On a surface that can safely patch or materialize/edit the complete files, migrate `rfb_session.h/.c` atomically enough to remove `socket_fd` and `rfb_io` authority, route exact read/poll/write through `pstvnc_rfb_bridge_*`, integrate REQUEST handling only at complete-message/idle-safe parser boundaries, and migrate the behavior fixture to logical bridge stubs including exact-I/O failure and safe-boundary quiesce cases. Do not invent Transport configuration or touch app/platform lifecycle until this packet is coherent.