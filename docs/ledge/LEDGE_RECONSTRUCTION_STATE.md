# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0005
RECORDED_AT=2026-09-15T18:10:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0004
BASED_ON_AUDIT_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0009
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Authority inspected

- scheduled Reconstruction A branch authority at start: `b159471f62d9b67e3c349914d1b4df7a6e953b8b`;
- branch immediately before this state write: `3b532b191502b5deb7e0ffff1ff4000aa333e8c7`;
- forensic H1 source authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0007` preserves A001-A006 as reconstruction-ready and closes the seeded audit;
- global state revision `0009` keeps A001 as the active dependency foundation;
- validation state revision `0004` keeps V003 and V004 open while A001 remains incomplete;
- governing reconstruction contract revision `0002` and immutable work-log contract revision `0001` were consumed.

This worker is GitHub-native. Unknown external Pi-local dirty work is outside this mutation surface and was neither overwritten nor declared absent.

## Current reconstruction phase

`A001_PHYSICAL_STREAM_RECEIVE_IN_PROGRESS`

## Progress

A001 now has direction-local sequence state in `src/transport/physical_stream.*` and a sole physical receive primitive alongside the existing serialized send path. `pstvnc_transport_physical_stream_receive_frame()` performs exact header receipt, protocol decode, expected inbound-sequence enforcement, payload-capacity/null validation, exact payload receipt, and advances the expected sequence only after the complete frame is present.

The physical descriptor remains private to Transport. This increment deliberately stops at physical framing: it does not introduce callbacks or cross-component dispatch from the physical-stream unit. Logical channel dispatch/storage belongs in the higher transport runtime so the physical owner does not acquire RFB/media semantics.

`src/transport/SYMBOLS.md` was updated for the receive state, public internal receive primitive, and file-local exact-receive helper. V004 is therefore improved for this increment but remains OPEN because full dictionary/portal completeness has not been machine-checked or resolved.

## Behavioral parity / defect treatment

The H1 direction-local receive invariant is preserved: exactly one physical socket owner reads complete PSTV frames; inbound sequence starts at 1 and advances only after a complete frame; malformed/oversized/out-of-order/incomplete frames fail rather than becoming receive authority. Diagnostic stages/counters were not carried into product synchronization.

The historical receiver-dispatch shutdown race remains explicitly unresolved. This increment does not claim dispatch quiescence because dispatch itself is not yet reconstructed. No known defect was silently corrected.

## Validation status

A001 remains **not VALIDATION_READY**. Logical channel dispatch, transport-owned logical RFB storage/activity/credit/residual behavior, outbound RFB fragmentation, explicit receiver-dispatch quiescence, RFB bridge adaptation, topology/build/test integration, host tests, and V004 portal/completeness closure remain outstanding.

PENDING_LOCAL: compile/build of the physical-stream receive increment, canonical `scripts/check.sh`, strict source-dictionary/topology checks, clean-product build integration, exact ELF/PT_LOAD identity, and PS2DEV-dependent checks. No unavailable local check is claimed PASS.

HARDWARE_PENDING: none yet because A001 has not reached machine-validation completion; historical H1 hardware evidence remains forensic only.

## Commits in this shift

- `4244ea0ddf1caa235b41d0e8eb124e33adf219bc` — define sole physical receive contract and direction-local receive sequence state;
- `9170f97a2ec327fec5a9bbe63bbd7476da7e6a7a` — implement exact ordered physical frame receive;
- `3b532b191502b5deb7e0ffff1ff4000aa333e8c7` — index receive symbols in the transport dictionary.

## Work remaining

Continue A001 only. Build the higher transport runtime that is the sole caller of the physical receive primitive, dispatch channel 1 DATA into transport-owned logical RFB storage, provide producer activity rendezvous, and keep physical framing ignorant of RFB semantics. Then implement parser-consumption-based credit, terminal residual distinction, outbound RFB fragmentation through the serialized send path, and explicit receiver-dispatch quiescence before resource reclamation. Follow with RFB bridge adaptation, topology/build/test integration, V004 completeness closure, canonical checks, and exact build/PT_LOAD evidence.

## Exact next pickup

Start from `physical_stream.*`, `transport.h`, and the A001 H1 logical-RFB evidence. Introduce the smallest transport-owned logical RFB channel/storage mechanism and receiver runtime that consumes `pstvnc_transport_physical_stream_receive_frame()`, accepts only the audit-authorized channel-1 DATA shape, enqueues complete payloads under transport synchronization, and exposes activity without blind polling. Keep credit/residual accounting explicit and do not reclaim receiver-touched resources until a first-class quiescence mechanism proves dispatch completion. Do not begin A002 media transport while A001 remains incoherent.
