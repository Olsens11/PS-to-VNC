# Ledge Reconstruction — Append-only Lane Log

DOCUMENT=LEDGE_RECONSTRUCTION_LOG
LOG_FORMAT_REVISION=0001
TEMPORAL_CLASS=APPEND_ONLY_EVENT_LOG

Entries describe what was true and done at their recorded time. Later entries may supersede current-state conclusions but must not rewrite historical entries.

## R001 — 2026-09-15T10:18:00-04:00 — A001 architecture gate

Starting authority:

- branch `ledge/h1-all-guns` at `9c0ff04209b31a186a4995a49b8631adca9de957`;
- audit state revision `0001` / semantic audit revision `0002`;
- global state revision `0002`;
- primary architecture `docs/CLEAN_ARCHITECTURE.md`, version `1`, status `REBUILD_READY`.

Work performed:

- consumed all A001 rows explicitly marked `RECONSTRUCTION_READY`;
- inspected the existing clean `src/` component surface and current `src/rfb` surface;
- compared A001's required one-physical-PSTV/sole-receiver ownership against the primary architecture's concurrency and dependency rules;
- identified a direct authority conflict: A001 requires a new shared transport owner while architecture version 1 still assigns VNC socket operations to the main/application thread and socket facilities to RFB;
- created reconstruction-lane continuity state and this append-only log;
- prepared an exact source reconstruction order and validation queue behind the architecture gate.

Source changes: reconstructed product source none; forensic H1 source none; audit/simplification records none; validation records none; reconstruction-owned docs only.

Decision: do not manufacture a new transport authority while declared architecture contradicts that ownership. Preserve A001 ready semantics and block only source migration until governance reconciliation is explicit.

Exact next pickup: re-read architecture and lane states; implement A001 only after explicit reconciliation.

## R002 — 2026-09-15T11:24:00-04:00 — A001 shared-transport interface begins

Starting authority:

- branch `ledge/h1-all-guns` at `5d376fe67eecfdad8f96ee1553177f82d753a31d`;
- global state revision `0003`;
- audit state revision `0002`;
- `LEDGE_ARCHITECTURE_OVERLAY` revision `0001`, which explicitly authorizes the A001 shared-transport owner;
- validation V001 still OPEN pending actual source reconstruction/validation.

Work performed:

- confirmed the former architecture gate is resolved prospectively by governance;
- created `src/transport/transport.h` as the explicit shared-transport ownership interface;
- created `src/transport/SYMBOLS.md` as the new domain's local dictionary;
- removed physical socket authority from the RFB-facing byte-stream API shape;
- made application-requested session open/quiesce/close explicit while documenting that receiver-dispatch quiescence, not diagnostic counters, is synchronization authority;
- advanced reconstruction lane state to revision `0002`.

Commits produced during the tranche:

- `8733754fac4b7ed8a66c241e64f5eb248294f544` — transport interface;
- `37f2a626e62aafb4cf4626091ef9421b205768f5` — transport local symbol dictionary;
- `0d2b250966d85bf9437d3db5862b46ea02356bfe` — reconstruction state revision `0002`.

Evidence/validation:

- source interface was checked against A001 audit semantics and the governing overlay;
- no physical framing/receiver/channel implementation exists yet, so behavioral tests and build qualification were not claimed;
- the interface is not wired into the build, therefore this shift does not claim a changed DUT/PT_LOAD;
- the new-directory topology policy was inspected and its remaining obligations are explicitly recorded rather than silently bypassed.

Simplification/decision:

The H1 symbol-renaming/fake-socket adapter is not reproduced. The clean boundary names logical RFB stream operations directly and withholds the physical socket descriptor from RFB consumers. The counter-equality/delay shutdown fence is likewise not encoded into the interface; quiescence is a semantic operation whose implementation must prove the same ordering invariant explicitly.

Incomplete work:

- physical PSTV framing/sequence validation;
- sole receiver and logical-channel dispatch;
- logical RFB storage/credit/residual behavior and outbound fragmentation;
- serialized physical send;
- explicit receiver-dispatch state implementation;
- RFB bridge adaptation;
- new-domain topology/continuity/build/dictionary-portal integration;
- host tests, canonical checks, build identity, PT_LOAD comparison, and PS2 qualification.

Risk/discovery:

The topology policy intentionally makes a new clean domain a multi-surface change. The interface files are therefore an IN_PROGRESS reconstruction boundary, not a completed architecture stage. Future workers must not mistake file presence for build adoption or validation.

Ending authority:

Reconstruction state revision `0002` is the current lane handoff. A001 remains IN_PROGRESS and not VALIDATION_READY. A002 is ready but intentionally queued behind a coherent A001 transport foundation.

Exact next pickup:

Continue A001 by reconstructing the proven H1 physical mux/sole-receiver/logical-RFB/quiescence mechanism into the transport owner, then satisfy the new-domain topology/build/test/dictionary obligations and run canonical checks before handing any tranche to validation.

## R003 — 2026-09-15T16:09:50-04:00 — Reconstruction Shift B begins A001 implementation pickup

STARTED_AT=2026-09-15T16:09:50-04:00
WORKER=RECONSTRUCTION_SHIFT_B
STARTING_BRANCH_AUTHORITY=a2b4dbf27dc9897a49480edcfceddce38b51e8b0
STARTING_RECONSTRUCTION_STATE_REVISION=0002
STARTING_GLOBAL_STATE_REVISION=0007
STARTING_AUDIT_STATE_REVISION=0006
STARTING_VALIDATION_STATE_REVISION=0002

Shift intent: continue only A001 from the shared reconstruction authority. Inspect the proven H1 physical mux, sole receiver, logical-RFB queue/credit/fragmentation, serialized-send and quiescence mechanisms and reconstruct the smallest coherent transport-owned body. Unknown external Pi-local dirty state remains outside this GitHub-native worker's mutation surface and is neither overwritten nor declared absent.

Status at start: IN_PROGRESS; completion details will be appended in a later reconstruction log event after repository authority is rechecked.

## R004 — 2026-09-15T16:13:55-04:00 — Reconstruction Shift B completes framing subtranche

COMPLETED_AT=2026-09-15T16:13:55-04:00
ELAPSED_SHIFT_DURATION=00:04:05
WORKER=RECONSTRUCTION_SHIFT_B
ENDING_RECONSTRUCTION_STATE_REVISION=0003
ENDING_BRANCH_AUTHORITY_BEFORE_LOG_WRITE=9da9ba1ac499a47300dd041f93c834d38fe269a7
SELF_PAUSED=NO

Work completed:

- inspected A001 semantic authority and the proven H1 `h1_transport_runtime.*` plus portable `transport_protocol.*` framing implementation;
- reconstructed `src/transport/protocol.h` and `src/transport/protocol.c` as a transport-owned, backend-independent PSTV v1 framing layer;
- preserved magic/version, fixed 16-byte header, 8192-byte payload ceiling, direction-local sequence field representation, stable channel identities and big-endian wire encoding;
- kept socket, dispatch, queue, RFB, media and threading policy out of the framing layer;
- updated `src/transport/SYMBOLS.md` for the new clean-generation symbols;
- advanced shared reconstruction state to revision `0003`.

Commits/files:

- `4d92cac4860259ed6956858eaedd16ac3e60dc09` — `docs/ledge/LEDGE_RECONSTRUCTION_LOG.md` STARTED_AT event;
- `259be4a074bd6c84f9822f0ad06551b991055011` — `src/transport/protocol.h`;
- `5aaf4b07702c4f85ae5cfc66e9d144889fc215fc` — `src/transport/protocol.c`;
- `cf4f10a45ca33300314f5cfc66e9d144889fc215fc` — `src/transport/SYMBOLS.md`;
- `9da9ba1ac499a47300dd041f93c834d38fe269a7` — `docs/ledge/LEDGE_RECONSTRUCTION_STATE.md` revision 0003.

Checks/evidence:

- repository authority and reconstruction state were re-read before every GitHub write: PASS;
- no competing reconstruction advancement or overlapping committed tranche appeared during this shift: PASS;
- semantic comparison against A001 and proven portable H1 framing source: PASS by source inspection;
- build/compile/host execution: PENDING_LOCAL because this GitHub-native worker has no repository execution/toolchain surface;
- canonical `scripts/check.sh`, strict source-dictionary/topology gates, clean-product build and exact ELF/PT_LOAD identity: PENDING_LOCAL;
- PS2 hardware qualification: not yet eligible; A001 is not machine-validation complete and no hardware PASS is claimed.

Behavioral parity / known defects:

No known defect was silently fixed. Diagnostic stages/counters and H1's bounded-delay/counter shutdown mechanism were not copied into framing because they are not framing semantics. The receiver-dispatch race-prevention invariant remains an explicit obligation for the upcoming runtime/quiescence body.

Work remaining:

A001 remains IN_PROGRESS and not VALIDATION_READY. Physical socket/session ownership, serialized framed send, sole receiver, sequence-state enforcement, logical RFB queue/activity/credit/residual behavior, outbound fragmentation, explicit dispatch/quiescence state, RFB bridge adaptation, topology/build integration, host tests, canonical checks, exact build/PT_LOAD identity and later hardware qualification remain incomplete.

Exact next pickup:

Build on `src/transport/protocol.*` and `transport.h`: implement the smallest transport runtime body that adopts one physical socket and owns serialized framed sends, then add the sole receive/sequence-validation path and logical RFB dispatch. Keep the physical descriptor private to transport, preserve explicit dispatch quiescence before resource reclamation, and do not start A002 transport-facing media work until this A001 foundation is coherent.

## R005 — 2026-09-15T16:43:27-04:00 — Interactive reconstruction shift begins A001 physical-send pickup

STARTED_AT=2026-09-15T16:43:27-04:00
WORKER=INTERACTIVE_RECONSTRUCTION_SHIFT
STARTING_BRANCH_AUTHORITY=f056ad828f4d520d9bca0eae350fe16240bdf82b
STARTING_RECONSTRUCTION_STATE_REVISION=0003
STARTING_GLOBAL_STATE_REVISION=0008
STARTING_AUDIT_STATE_REVISION=0006
STARTING_VALIDATION_STATE_REVISION=0003

Shift intent: use the short safe gap before Reconstruction Shift A to advance only the smallest coherent next A001 unit: transport-owned adopted physical-socket state plus serialized framed send, based on the proven H1 send ordering. Do not begin sole receive/dispatch, logical-RFB queueing, A002 media work, or any cross-component bridge work in this short shift. The physical descriptor remains private to transport. Build/toolchain checks unavailable to this GitHub-native interactive surface remain PENDING_LOCAL rather than implied PASS.

Status at start: IN_PROGRESS; completion or safe-stop details will be appended after authority is rechecked.

## R006 — 2026-09-15T16:47:47-04:00 — Interactive reconstruction shift completes A001 physical-send subtranche

COMPLETED_AT=2026-09-15T16:47:47-04:00
ELAPSED_SHIFT_DURATION=00:04:20
WORKER=INTERACTIVE_RECONSTRUCTION_SHIFT
ENDING_RECONSTRUCTION_STATE_REVISION=0004
ENDING_BRANCH_AUTHORITY_BEFORE_LOG_WRITE=b4cf384c04645c6a7a732a01b87186d72350f372
SELF_PAUSED=NO

Work completed:

- re-read current branch/reconstruction authority throughout the shift and found no competing reconstruction advancement;
- inspected the proven H1 serialized physical-send ordering and reconstructed it as transport-internal `physical_stream.h/.c`;
- established explicit ownership of one adopted physical socket, one send semaphore and outbound sequence state;
- preserved exact-send behavior for header and payload, one lock across the complete frame transaction, initial outbound sequence 1, and sequence advancement only after complete frame send;
- kept the physical descriptor private to transport and excluded receive/dispatch, logical-channel queues, RFB parsing, media policy and application lifecycle from this unit;
- made release explicitly dependent on higher-level receiver/dispatch quiescence once receive exists rather than reproducing H1 diagnostic-counter synchronization;
- updated `src/transport/SYMBOLS.md` and advanced reconstruction state to revision 0004.

Commits/files:

- `ef37c18cf35fe58f1b696c14d82dcb8248405392` — STARTED_AT log event;
- `e7d2196198391d3d17443fb8c7103f4278f46ab7` — `src/transport/physical_stream.h`;
- `6de2203a6af44b8488c3241125a59a61947bd0a0` — `src/transport/physical_stream.c`;
- `ed6d929b24f1636603905926f93bc81051cb3e4c` — `src/transport/SYMBOLS.md`;
- `b4cf384c04645c6a7a732a01b87186d72350f372` — `LEDGE_RECONSTRUCTION_STATE` revision 0004.

Checks/evidence:

- H1 source-ordering comparison: PASS by source inspection;
- ownership/boundary review: PASS by source inspection; no physical descriptor escapes transport;
- symbol-dictionary accounting for introduced project-defined symbols: PASS by repository inspection;
- compile/build/host execution of new physical-stream unit: PENDING_LOCAL;
- canonical `scripts/check.sh`, strict topology/dictionary gates, build integration and exact ELF/PT_LOAD identity: PENDING_LOCAL;
- hardware qualification: not yet eligible and not claimed.

Behavioral parity / known defects:

No known defect was silently fixed. The send-side H1 ordering was retained without carrying diagnostic stages/counters into correctness. The receiver-dispatch shutdown race remains explicitly outstanding; this subtranche does not claim to solve it.

Work remaining:

A001 remains IN_PROGRESS and not VALIDATION_READY. Sole physical receive, inbound sequence enforcement, logical dispatch, RFB queue/activity/credit/residual behavior, outbound RFB fragmentation, explicit receiver-dispatch quiescence, RFB bridge adaptation, build/topology/test integration, canonical checks and exact build/PT_LOAD evidence remain.

Exact next pickup:

Start from `physical_stream.*`, `protocol.*` and `transport.h`. Implement the smallest sole-receiver path that reads complete PSTV frames, validates the expected inbound sequence and dispatches channel 1 into transport-owned logical RFB storage. Preserve descriptor privacy and make receiver-dispatch quiescence explicit before any resource-reclamation path; do not begin A002 media transport while A001 remains incoherent.