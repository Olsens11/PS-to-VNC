DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T18:47:05-04:00
COMPLETED_AT=2026-09-24T18:56:02-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=00015df0ef2c3a249e0d707b1f8218e3c43f95aa
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R22 MPEG live service accepted; R23 retirement drain activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-LIVE-SERVICE-R22`, independently recover the live
`ledge/h1-all-guns` authority, inspect the actual worker commit range, source,
tests, dictionaries, machine evidence and immutable closeout, disposition every
R22 criterion, then publish exactly one dependency-correct next Reconstruction
packet.

Live pickup authority was:

`00015df0ef2c3a249e0d707b1f8218e3c43f95aa`

This was independently compared with the live branch and was identical at
Foreman pickup.

The governing current authorities consumed included:

- Foreman State revision 0057;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Architecture Overlay revision 0007;
- Wire Runtime Decisions revision 0011;
- the accepted R21 Application MPEG run-start authority;
- accepted P7 frame-consumer and P3 Presentation ownership;
- accepted R18 Transport MPEG run-boundary semantics;
- accepted R20 private-session START/RETIRE projection;
- accepted R20E Transport receiver/submission drain fences;
- accepted R4 worker and R5 PS2 worker-runtime lifecycle contracts.

The newest preceding Foreman record was
`docs/ledge/work-log/20260924T145500-0400__foreman__a003-mpeg-generation__interactive.md`.

The Reconstruction handoff consumed was
`docs/ledge/work-log/20260924T164702-0400__reconstruction__a003-mpeg-generation__interactive.md`.

## Exact Reconstruction range recovered

The worker began at:

`07b9dc4ec91c2947b7da62164bda6f214ab5b78e`

and the exact pre-log R22 range was four commits:

1. `b8fab72ad354b14d6f56f35f59baa0f4afea73b6` —
   `app: compose R22 MPEG live frame service`;
2. `dea41fa46e0a22ee395336af22d1ee1084bf262e` —
   `tooling(symbols): run deterministic dictionary reconciliation`;
3. `ed8887bd0b6199ab633a5154b3445e8516042529` —
   `docs(symbols): reconcile current clean definitions`;
4. `345e726effee8f01d1be71938e8f09efde030c0b` —
   `test: verify final R22 MPEG live service authority`.

The final source authority was therefore:

`345e726effee8f01d1be71938e8f09efde030c0b`

The next and only following commit was the immutable Reconstruction closeout:

`00015df0ef2c3a249e0d707b1f8218e3c43f95aa`.

Independent compare showed four commits ahead and zero behind from the assigning
Foreman authority to final source, then exactly one log-only commit to pickup.
No concurrent branch movement or write collision occurred in that range.

The behavior-bearing diff changed only:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `docs/development/mpeg-generation-control.md`.

Deterministic dictionary reconciliation additionally changed:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The worker did not modify ordinary `src/app.c`, P7 implementation, Display /
Presentation implementation, MPEG worker/backend/runtime, Transport, RFB flow
policy, Configuration, Pi product source, AUDIO, Input/UI/calibration or
protocol bytes.

## Independent R22 source acceptance

R22 adds exactly one truthful Application live state,
`PSTVNC_APP_MPEG_RUN_MPEG_OWNED`, and one trigger-agnostic live-service
operation, `pstvnc_app_mpeg_run_service()`.

The Foreman independently verified that the service delegates frame work only
to the already-accepted P7 consumer for `run->current_generation`. It does not
duplicate P7 frame claim, scheduler, compositor, media-clock, worker or
Presentation mechanisms.

Before first synchronized presentation, benign P7 IDLE with an unfinished worker
leaves Application in `STARTED_WAIT_FIRST_FRAME`. Application promotes to
`MPEG_OWNED` only when the P7 result proves the exact first PRESENTED ordinal,
synchronized compositor effects and first-frame promotion and when P3
independently reports exact-generation MPEG_OWNED.

After promotion, P7 IDLE, WAIT, PRESENTED and DROPPED remain ordinary live
outcomes. WAIT preserves the exact outstanding claim, ordinal and deadline.

Unexpected worker completion, negative P7 results, faulted P7 status,
generation mismatch, impossible P3 state, or returned/status contradictions
fault the Application run and require outer session teardown.

The R22 live failure path does not clear P7, abort P3, stop/join/release R4,
release R5, send RETIRE, finalize R18, thaw P2 or reveal RFB. Lower-owner
evidence remains available for the later lifecycle owner.

### Criterion dispositions

A003-R22-C1 RUN_OWNER_HAS_ONE_EXPLICIT_MPEG_OWNED_LIVE_STATE — MET
A003-R22-C2 LIVE_SERVICE_USES_ONLY_EXISTING_EXACT_GENERATION_P7_CONSUMER — MET
A003-R22-C3 PRE_FIRST_IDLE_PRESERVES_WAIT_FIRST_FRAME_WITHOUT_SIDE_EFFECT — MET
A003-R22-C4 FIRST_PRESENTED_FRAME_EXACTLY_PROMOTES_RUN_AFTER_P3_CONFIRMATION — MET
A003-R22-C5 MEDIA_CLOCK_AND_PHYSICAL_FIRST_SYNC_OWNERSHIP_REMAINS_P7_DISPLAY — MET
A003-R22-C6 MPEG_OWNED_IDLE_WAIT_PRESENTED_DROPPED_PRESERVE_LIVE_AUTHORITY — MET
A003-R22-C7 UNEXPECTED_WORKER_FINISH_IS_FAILURE_NOT_NORMAL_RETIREMENT — MET
A003-R22-C8 P7_OR_STATE_CONTRADICTION_FAULTS_WITHOUT_ERASING_OWNER_EVIDENCE — MET
A003-R22-C9 NO_RETIREMENT_STOP_FINALIZE_THAW_OR_REVEAL_SCOPE — MET
A003-R22-C10 RUN_STATUS_REMAINS_EXACT_GENERATION_AND_STATE_AUTHORITY — MET
A003-R22-C11 NO_ORDINARY_APP_PI_INPUT_AUDIO_OR_PROTOCOL_SCOPE_CREEP — MET
A003-R22-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

R22_DISPOSITION=FOREMAN_ACCEPTED

## Exact R22 machine/build evidence

At final source authority
`345e726effee8f01d1be71938e8f09efde030c0b`, GitHub Actions run
`36058496825`, attempt 1, completed successfully.

Independently observed jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-reconcile — SKIPPED as expected on the final non-trigger commit.

Observed host evidence included:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed project/build evidence included:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

- `ELF_PRISTINE_SHA256=71412c76f5f748913b1f8247e6e1c18c5e4a4ffe7e8a89da98f65c6f343ab5d1`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`;
- `PT_LOAD_BYTES=496404`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R22
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

The R22 PT_LOAD differs from accepted R21. This exact R22 identity is
repository-reproducible evidence only and does not inherit physical
qualification.

## Next dependency decision

Foreman dependency review does not jump from live service directly to ordinary
product activation and does not combine all retirement/restoration stages into
one oversized transaction.

The accepted lower-owner contracts establish two distinct downstream boundaries:

1. exact MPEG producer/decoder/Transport safe-stop and reclaim; then
2. RFB underlay restoration, P2 thaw/FULL-refresh accounting and synchronized
   no-MPEG reveal.

The first boundary is now dependency-ready because R22 provides coherent
Application live authority and P7 already supports service while P3 is RETIRING.

The next bounded Reconstruction tranche is therefore:

`A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23`.

R23 must, for one exact healthy MPEG_OWNED generation:

- move P3 to RETIRING before producer close;
- invoke one exact R20 RETIRE;
- continue nonblocking P7 drain service while already-admitted data drains;
- treat RETIRE-completion WOULD_BLOCK as pending;
- accept only exact-generation RETIRE completion;
- publish the R18 finite producer-done fence only after exact completion;
- require natural R4 completion after that fence, no clean-path stop request;
- prove no P7 borrow remains;
- join and require exact-generation COMPLETED outcome;
- retire P7/R4/R5 execution ownership in dependency order;
- finalize R18 last;
- stop in an explicit restoration-pending Application state with P3 still
  RETIRING and P2 still frozen.

R23 explicitly may not seal/reveal P3, thaw P2, service FULL-refresh debt,
activate ordinary `src/app.c`, activate the Pi product MPEG factory, or change
accepted lower-owner mechanisms. Those are downstream work.

## Foreman-owned publication

Published Foreman State revision 0058 in:

`4b5e7c9ed87676870e3edb916ec8a129bb9c8a82`

with message:

`docs(foreman): accept R22 and activate MPEG retirement drain`.

The state changes only the Foreman planning/current-state document. No product
source, dictionaries, topology, tooling or build manifests required Foreman
repair because R22's canonical machine gates were already coherent.

Immediately after publication, branch authority was refreshed and remained
exactly at `4b5e7c9ed87676870e3edb916ec8a129bb9c8a82`.

No commit status or PR-triggered workflow record was exposed for that
documentation-only state commit at log close. Therefore no new state-head CI
result is claimed. This limitation does not weaken the exact R22 source-head
machine evidence above and no hardware or Validation claim is inferred.

## Current hardware debt

Current accepted exact loadable authority is R22:

`PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`
`PT_LOAD_BYTES=496404`

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

Any R23 loadable-byte change creates a newer exact identity that must be
recorded as build evidence and remains hardware-pending unless separately
qualified.

## Exact next pickup

NEXT_PACKET=A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R23_FROM_FOREMAN_STATE_0058

The Reconstruction worker must independently recover current repository
authority, consume Foreman State revision 0058, execute only R23, create exactly
one immutable Reconstruction log, stop, and return the baton.

RFB thaw/FULL-refresh/restoration, synchronized reveal, ordinary MPEG product
activation, AUDIO and Input/UI/calibration remain deferred.
