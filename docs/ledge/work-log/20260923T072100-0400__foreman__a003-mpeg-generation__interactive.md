DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-23T07:21:00-04:00
COMPLETED_AT=2026-09-23T07:28:30-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=1ded79fdfb5f15eb4266a3e997cd445a26c126a2
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R19 RFB flow composition; R20 private MPEG session binding activated

## Objective and recovered authority

Receive the Reconstruction baton after `A004-RFB-FLOW-APPLICATION-COMPOSITION-R19`,
recover live branch authority independently, verify the exact worker range and
machine/build evidence, decide source acceptance criterion-by-criterion, then
publish only the smallest remaining prerequisite before MPEG Application
generation orchestration.

Live pickup authority was:

`1ded79fdfb5f15eb4266a3e997cd445a26c126a2`

with immutable Reconstruction record:

`docs/ledge/work-log/20260923T042621-0400__reconstruction__a003-mpeg-generation__interactive.md`

The worker's final pre-log source authority was:

`cd09bc263e5835c75d5a8fea256cd71127e7821b`

and exactly six Reconstruction commits followed assigning Foreman authority
`3ab9d8bbd57bac9cf078c92dae772474f358d198` before the immutable R19 log.
No commit followed that log before this Foreman round began.

Current repository authority read included AGENTS/CONTRIBUTING, current status,
project intent, clean architecture, development naming/topology/module lifecycle
rules, Reconstruction/work-log contracts, Foreman State 0050, Wire Runtime
Decisions revision 0011, Architecture Overlay revision 0007, Q1-Q12
reconciliation, A001/A003/A004/A005/A006 audit authority, R19 Application/P2
source and tests, and the accepted MPEG/Presentation/worker/Transport public
seams relevant to next-dependency selection.

No external Pi/local checkout was exposed to this Foreman seat. External staged,
unstaged and untracked worktree state remains unasserted; no local
`scripts/resume-state.sh` or `scripts/check.sh` execution is claimed.

## Exact R19 source review

R19 remained inside its authorized Application/test/build/check/documentation/
dictionary surface. The accepted `src/rfb/flow_policy.c/.h` mechanism itself
was unchanged. No Pi product, Transport product, RFB parser/session, framebuffer
product, AUDIO, MPEG, media-clock, Display/Presentation, calibration/local-
controller product, protocol or Configuration product source changed.

Independent review accepts all twelve packet criteria:

A004-R19-C1 FRESH_RFB_ATTEMPT_OWNS_FRESH_THAWED_FLOW_POLICY — MET
A004-R19-C2 LIVE_REQUESTS_ARE_SELECTED_ONLY_BY_P2_POLICY — MET
A004-R19-C3 REQUEST_ACCOUNTING_ADVANCES_ONLY_AFTER_SUCCESSFUL_SEND — MET
A004-R19-C4 COMPLETED_UPDATE_CLEARS_EXACT_OUTSTANDING_REQUEST_BEFORE_NEXT — MET
A004-R19-C5 REMOTE_VISUAL_PUBLICATION_IS_GATED_BY_P2_POLICY — MET
A004-R19-C6 ORDINARY_THAWED_INCREMENTAL_BEHAVIOR_IS_PRESERVED — MET
A004-R19-C7 IDLE_OR_HOLD_DOES_NOT_FABRICATE_REQUEST_OR_COMPLETION — MET
A004-R19-C8 PROVIDER_REPLACEMENT_CANNOT_INHERIT_FLOW_STATE — MET
A004-R19-C9 P2_FULL_REFRESH_DEBT_SEMANTICS_REMAIN_SINGLE_AUTHORITY — MET
A004-R19-C10 RFB_TRANSPORT_FRAMEBUFFER_PRESENTATION_OWNERSHIP_UNCHANGED — MET
A004-R19-C11 NO_MPEG_CALIBRATION_AUDIO_PI_OR_FINAL_COMPOSITION_SCOPE_CREEP — MET
A004-R19-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

Key independently verified behavior:

- each successful RFB attempt initializes fresh thawed P2 state only after the
  authoritative startup full frame;
- `service_rfb_flow_request()` asks P2 for HOLD/INCREMENTAL/FULL and contains no
  second request-debt state machine;
- HOLD is a successful no-send/no-accounting result;
- INCREMENTAL/FULL map only to the existing RFB request serialization API;
- P2 records a request only after successful RFB serialization;
- an UPDATE completion clears exactly one outstanding P2 obligation before
  framebuffer validity, publication or successor request scheduling;
- IDLE clears no request debt and sends no duplicate request;
- remote framebuffer parsing/truth may advance while visual publication is
  denied by the P2 gate;
- a provider replacement rebuilds P2 from zero with the rest of attempt-local
  authority;
- failed post-send P2 accounting fails the attempt closed rather than silently
  repairing local state.

R16B recovery ordering remains intact: failed-attempt admission closes, Input
must prove shutdown, Transport must retire the old Wire/runtime, and only then
may a replacement attempt create fresh RFB/P2 authority.

## R19 machine/build evidence

Exact final source run `35852093499` attempt 1 completed SUCCESS with:

- host-unit — PASS, including `RFB_FLOW_POLICY_TEST=PASS` and
  `app R15/R16B/R19 tests: PASS`;
- dedicated R16A PS2 RFB wait-termination proof — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link — PASS;
- current-source reproducibility — PASS.

Exact immutable-log-head run `35852383542` attempt 2 completed SUCCESS with the
same canonical evidence classes.

Exact R19 linked identity on both final source and immutable-log heads:

`ELF_PRISTINE_SHA256=308688f933c72de19934eae3d8048e9476de91e688d2098db0e98d283ca0f4b0`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`
`PT_LOAD_BYTES=491156`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

This PT_LOAD changed from accepted R18 and therefore creates a newer exact
hardware-debt identity. No physical qualification is inferred.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R19
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
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency review

Foreman did not authorize direct MPEG Application orchestration after R19.

The exact generation-control path still has an ownership mismatch:

- Pi Q4 establishment is the sole allocator of a nonzero Wire `session_id`;
- PS2 Transport receives that identity and stores it as
  `pstvnc_transport_bridge_private_session_id`;
- Transport documentation explicitly says the Pi-assigned session identity is
  private to Transport and dies with the physical Wire Session;
- yet the public MPEG bridge currently requires callers to pass full
  `pstvnc_mpeg_start_payload_t` / `pstvnc_mpeg_retire_payload_t` wire structs,
  including protocol version and `session_id`.

A future Application generation owner cannot satisfy that API without either
exposing Transport-private Q4 identity or manufacturing/duplicating it. Both
would violate current dependency authority. The same leak would make
stale-session correlation depend on caller discipline rather than Transport.

R20 therefore closes only this seam. Above Transport, callers will express
run-owned generation/geometry meaning. At the bridge, Transport stamps the
current private Q4 session identity and accepted generation-control wire
version, then keeps full wire completion correlation below the bridge.

No public Q4 session-id getter is authorized. Fixed START/RETIRE wire bytes are
not being redesigned.

## Foreman-owned publication

Published Foreman State revision 0051 at:

`84e0434f1e20f18a38d74d20b4d20116124eaedd`

with message:

`docs(foreman): accept R19 and bind MPEG control identity`

State 0051 activates:

`A003-MPEG-PRIVATE-SESSION-BINDING-R20`

R20 is restricted to owner-correct Transport MPEG generation-control value
representation, private session/version stamping, internal exact completion
correlation, stale-session fencing, tests, dictionaries and directly required
build/documentation integration.

R20 explicitly excludes Application MPEG activation, generation allocation,
calibration/product triggers, RFB freeze/thaw calls, worker/decoder/backend
activation, Display/Presentation composition, Pi product changes, AUDIO,
heartbeat, CONFIG-on-Wire and final all-guns orchestration.

## State-0051 machine evidence

Exact State-0051 Actions run `35854624948` attempt 1 completed SUCCESS.

- host-unit — PASS;
- dedicated R16A PS2 RFB wait termination — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

The documentation-only State publication retained exact R19 linked identity:

`ELF_PRISTINE_SHA256=308688f933c72de19934eae3d8048e9476de91e688d2098db0e98d283ca0f4b0`
`PT_LOAD_SHA256=47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`
`PT_LOAD_BYTES=491156`

## Next pickup

Reconstruction should recover current branch authority and execute only
`A003-MPEG-PRIVATE-SESSION-BINDING-R20` from Foreman State 0051.

Do not begin Application generation allocation/orchestration, calibration UI or
trigger work, RFB freeze/thaw composition, MPEG worker/Presentation activation,
Pi changes, AUDIO, heartbeat or unrelated cleanup.

Preserve exact 44-byte START v1 and 12-byte RETIRE v1 wire representation.
Transport should solve the ownership mismatch by stamping its private current
Q4 identity below the bridge, not by publishing that identity upward.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
