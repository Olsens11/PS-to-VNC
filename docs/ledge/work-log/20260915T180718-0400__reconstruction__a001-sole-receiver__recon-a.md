# Reconstruction Shift A — A001 sole receiver

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T18:07:18-04:00
COMPLETED_AT=2026-09-15T18:11:00-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=recon-a
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=b159471f62d9b67e3c349914d1b4df7a6e953b8b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Continue audit-ready A001 only, from reconstruction state revision 0004 and global state revision 0009, by advancing the one-physical-stream receive foundation without leaking logical RFB/media semantics into the physical framing owner. Consumed reconstruction contract revision 0002, work-log contract revision 0001, audit state revision 0007/A001 disposition, validation state revision 0004, clean architecture version 1, AGENTS.md, CONTRIBUTING.md, and forensic H1 authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

## Work performed

- Added direction-local expected receive sequence state to `pstvnc_transport_physical_stream_t`.
- Added the transport-internal sole physical frame receive contract.
- Implemented exact header/payload receipt, protocol decode, inbound sequence validation, caller-capacity/null validation, and sequence advancement only after a complete frame.
- Kept logical dispatch outside `physical_stream.*`; the next higher transport runtime remains responsible for channel-1 storage and activity semantics.
- Updated `src/transport/SYMBOLS.md` for the receive primitive and exact-receive helper.
- Advanced reconstruction state to revision 0005.

## Exact commits/files

- `4244ea0ddf1caa235b41d0e8eb124e33adf219bc` — `src/transport/physical_stream.h` receive contract/state.
- `9170f97a2ec327fec5a9bbe63bbd7476da7e6a7a` — `src/transport/physical_stream.c` ordered exact receive implementation.
- `3b532b191502b5deb7e0ffff1ff4000aa333e8c7` — `src/transport/SYMBOLS.md` receive symbol indexing.
- `f989eb33eddc9904243d85186ce8b7f3dbf76458` — `docs/ledge/LEDGE_RECONSTRUCTION_STATE.md` revision 0005.

## Checks and evidence

- GitHub branch authority was re-read before each mutation; no overlapping reconstruction advance appeared during this shift.
- Static comparison against forensic H1 sole-receiver ordering: the reconstructed primitive preserves exact-read loops, expected inbound sequence, full-payload completion before sequence advance, and physical-socket privacy.
- Diagnostic stages/counters were intentionally not adopted as synchronization authority.
- No GitHub status checks were available as machine-validation evidence.

PENDING_LOCAL: compile/build of the changed physical-stream unit; canonical `scripts/check.sh`; strict source-dictionary/topology checks; build integration; exact ELF/PT_LOAD identity; PS2DEV-dependent checks.

HARDWARE_PENDING: not yet promoted because A001 is still source-incomplete and machine validation has not established a DUT.

## Known-defect accounting / blockers

The historical receiver-dispatch shutdown race remains unresolved and visible. This shift did not silently fix it or claim quiescence; logical dispatch has not yet been reconstructed. V003 remains OPEN waiting for a coherent tranche. V004 remains OPEN: this shift updated the local transport dictionary for new symbols but did not prove definition completeness or generated portal integration.

No persistent blocker was found. External Pi-local dirty work remains unknown and outside this GitHub-native mutation surface; it was neither overwritten nor declared absent.

## State/contract revisions

Consumed: reconstruction contract 0002; work-log contract 0001; global state 0009; audit state 0007; reconstruction state 0004; validation state 0004.

Produced: reconstruction state 0005.

## Exact next pickup

Continue `a001-sole-receiver`. Add the smallest transport-owned logical RFB channel/storage mechanism and higher receiver runtime that is the sole caller of `pstvnc_transport_physical_stream_receive_frame()`. Accept audit-authorized channel-1 DATA into synchronized transport storage and expose committed producer activity without blind polling. Then add parser-consumption credit/residual distinction, outbound fragmentation through the existing serialized send path, and explicit receiver-dispatch quiescence before resource reclamation. Keep A002 queued and do not mark A001 VALIDATION_READY until build/tests, V004 completeness, exact ELF/PT_LOAD evidence, and required machine validation exist.
