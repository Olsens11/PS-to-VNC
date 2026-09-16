# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0007
RECORDED_AT=2026-09-15T20:39:06-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0006
BASED_ON_AUDIT_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0011
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Current reconstruction phase

`A001_PUBLIC_BRIDGE_SOURCE_IN_PROGRESS`

## Authority and movement reconciled

- this interactive shift began from committed branch authority `151ab77c8b1afe2ce18db29fde2f5ba9f0d77100` after Reconstruction B recorded the public CONFIG/lifecycle/RFB-I/O design gate;
- forensic H1 authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0007` keeps A001 reconstruction-ready and the seeded audit complete;
- global state advanced during this shift to revision `0011`, which explicitly treats the bridge seam as a bounded reconstruction gate rather than justification for idle reconstruction capacity;
- validation's newer immutable 20:22 handoff independently reviewed reconstruction state revision `0006`, opened no new finding, and keeps V003/V004 OPEN;
- governing reconstruction contract revision `0002` and immutable work-log format revision `0001` remain current.

Branch and reconstruction-state authority were re-read between bounded writes. Validation and Continuity advanced documentation/global authority during this shift but did not mutate the reconstructed product source being edited here. Unknown external Pi-local dirty work remains outside this GitHub-native mutation surface and was neither overwritten nor declared absent.

## Progress

The bounded public-bridge/configuration design gate recorded by Reconstruction B has been materially narrowed without inventing CONFIG defaults:

- `src/transport/transport.h` now owns one stable `pstvnc_transport_session_config_t` value carrying the already-validated A001 queue, initial/batched-credit, credit-flush/return, receiver-thread stack/priority, and maximum-DATA-payload values required by the reconstructed runtime;
- `runtime.h` consumes that stable value directly and retains `pstvnc_transport_runtime_config_t` only as a transitional exact type alias, so there is no second configuration structure or default authority;
- `src/transport/bridge.h/.c` now provides Transport's single process-organized cross-component bridge body for application session lifecycle, logical-RFB delivery, and ordered RFB quiescence;
- successful session open transfers descriptor ownership explicitly by setting the caller's descriptor to `-1` after runtime adoption; failed initialization leaves caller ownership unchanged; failed receiver start retires the already-adopted Transport resources rather than creating ambiguous double-close ownership;
- session close refuses resource reclamation while a started receiver has not published completion;
- logical RFB read/poll/write and REQUEST/BOUNDARY/COMMIT/residual/COMPLETE operations cross the Transport bridge without exporting the physical descriptor or moving complete-RFB-message boundary policy into Transport;
- `src/rfb/bridge.h/.c` now provides RFB's side of the logical-stream adaptation with no socket parameter, translating Transport's result vocabulary into RFB's exact-read / readiness / exact-write contract;
- `src/transport/SYMBOLS.md` was migrated to canonical metadata and seven-column format and deliberately remains `COVERAGE=IN_PROGRESS` rather than fabricating V004 completion;
- `tests/unit/transport_bridge_test.c` now covers bridge socket-ownership transfer, failed-open ownership, live-receiver close refusal, receiver-completion release, logical RFB result mapping, and quiesce wrapper mapping with deterministic runtime stubs.

## Important incomplete integration boundary

The new bridge source is not yet the live clean-product I/O path.

Current `src/rfb/rfb_session.*` still carries the older socket-shaped RFB I/O contract, current `src/platform/ps2_network.c` still implements direct physical-socket RFB exact read/poll/write, and current `src/app.c` still establishes and passes the VNC socket through the pre-ledge session path. Those surfaces must be migrated coherently before the product can claim that RFB no longer possesses physical-socket-shaped authority.

Likewise, the retained `src/config/` surface currently supplies generic text parsing rather than a clean typed all-guns session configuration owner. This shift therefore defines the required validated Transport value but does not manufacture concrete queue/credit/thread/payload defaults in application code. Reconstruction must recover/adopt those values through explicit configuration authority before live lifecycle wiring.

The new RFB bridge files also make the previously complete `src/rfb/SYMBOLS.md` stale until their definitions are indexed. This is a maintenance consequence of new source, not V004 closure.

## Source-dictionary / topology status

V004 remains OPEN.

- `src/transport/SYMBOLS.md` now has canonical directory/generation/coverage metadata and a seven-column inventory, but it is explicitly `IN_PROGRESS` and has not passed definition-level strict discovery;
- `src/rfb/SYMBOLS.md` requires new entries for `bridge.h/.c` before it can truthfully return to complete current coverage;
- the generated product dictionary portal still requires regeneration/verification;
- `docs/development/source-topology.md` and the active directory allowlist in `scripts/continuity-check.sh` still predate the adopted `src/transport` ledge responsibility and require deliberate current-policy integration;
- no strict dictionary/topology PASS is claimed.

## Validation / evidence boundary

A001 remains **not VALIDATION_READY**.

PROVISIONAL by repository/source inspection only:

- explicit validated-value boundary into Transport without guessed defaults;
- one process-organized Transport bridge body;
- no physical descriptor in the new RFB bridge interface;
- unambiguous descriptor ownership transfer on the new Transport session-open path;
- receiver-completion guard before new bridge close can reclaim runtime resources;
- complete-RFB-message safe-boundary choice remains outside Transport;
- bridge host-test source meaningfully exercises ownership and result mapping.

PENDING_LOCAL / not claimed PASS:

- compile and execution of `transport_bridge_test` (the source exists but is not yet wired into `tests/Makefile`);
- execution of the already wired `transport_rfb_channel_test` and full host unit suite;
- PS2DEV compilation of `runtime.*`, `quiesce.c`, `bridge.*`, and changed channel source;
- canonical `scripts/check.sh`;
- strict definition-level dictionary/topology checks and generated portal synchronization;
- coherent application/RFB/platform migration onto the logical Transport bridge;
- clean-product topology/build integration;
- reproducible clean linked build, exact ELF identity, and PT_LOAD evidence.

HARDWARE_PENDING is not yet promoted because no coherent machine-validated reconstructed A001 DUT exists. Historical H1 hardware evidence remains forensic only.

## Known-defect accounting / lifecycle boundary

The historical H1 post-session receiver/mailbox poison remains unresolved and explicitly visible. This shift did not add a timeout, nonblocking-recv workaround, or other behavioral repair to the inherited defect.

The new bridge improves ownership/lifetime structure but does not by itself prove that the blocking sole receiver can always converge cleanly at finite-session teardown. Coherent close/error convergence remains an A001 obligation and requires executable/runtime evidence after the live application/RFB integration exists.

## Exact next pickup

Continue `a001-sole-receiver` from this state and the new bridge files. First finish the independent bridge evidence/metadata work: wire and run `transport_bridge_test` on an execution-capable surface, index `src/rfb/bridge.*`, complete the Transport definition inventory, and deliberately adopt `src/transport` into current topology/checker/portal authority rather than hand-editing generated compliance claims.

Then migrate the RFB session/app/platform call chain coherently from the socket-shaped RFB I/O seam onto `src/rfb/bridge.*` and `src/transport/bridge.*`. Supply `pstvnc_transport_session_config_t` only from explicit validated configuration authority; do not invent defaults. Preserve one physical PSTV owner, complete-RFB-message safe-boundary authority outside Transport, parser-consumption credit semantics, explicit receiver completion before reclamation, and the historical receive-poison defect boundary. Do not begin A002 while A001 remains incoherent.
