DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T20:46:00-04:00
COMPLETED_AT=2026-09-22T21:06:46-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=353f51c248235a9d6c1c1e5d7078de21dc6d1a74
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
HARDWARE_PENDING=YES

# Reconstruction shift — R16B RFB failure stop/restart policy

## Objective and authority consumed

This interactive Reconstruction shift recovered live repository authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0047`, whose active
bounded packet was:

`A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`

The starting branch authority was
`353f51c248235a9d6c1c1e5d7078de21dc6d1a74`
(`docs(work-log): accept R16A and hand off R16B`).

The accepted lower-layer source authority consumed by R16B was R16A source
`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`, with accepted semantics from
`docs/ledge/LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT.md`.

Authority reviewed before implementation included AGENTS/CONTRIBUTING, current
project/status and clean-architecture documents, development naming/topology/
lifecycle guidance, the ledge reconstruction contract, current Foreman State,
architecture overlay, Wire decisions, Q1-Q12 reconciliation, R16A contract,
relevant A001/A005/A006 audits, and the newest Foreman/Reconstruction immutable
records.

The GitHub connector exposes repository/branch authority but not the developer
Pi worktree. Therefore staged/unstaged/untracked local-worktree state and
`scripts/resume-state.sh` remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim of an external
clean worktree is made. Repository writes were made directly against refreshed
GitHub branch authority.

## Result

R16B is source-complete as a Reconstruction candidate for Foreman review at
final pre-log source authority:

`3310568e6c35ef59d77ee6075f1f9ea5a561476e`

Application now owns the explicit policy response to the accepted R16A typed
provider-terminal causes:

- CONNECT;
- READ/EOF;
- WRITE.

Those causes remain typed RFB-local facts. They are not rewritten into generic
physical-Wire failure.

When Application observes one of those provider causes it leaves the failed
attempt's ordinary RFB/input admission path immediately. It then requires
owner-proven input-runtime shutdown and Transport abort/receiver/session
retirement before another attempt is admitted. Failure to prove either stop
boundary fails closed and does not reconnect.

Successful retirement does not restart an old component in place. Application
returns through the ordinary startup path and obtains/reinitializes fresh:

- network connection / Q4 Wire Session authority;
- Transport runtime/access authority;
- Pi session-scoped RFB attachment;
- RFB parser/session state;
- framebuffer validity state;
- input runtime/publication state.

There is no retry-delay, backoff, timeout, sleep, or elapsed-time success
condition in the recovery policy. The replacement must independently reach the
ordinary healthy desktop/input startup boundaries.

Graphics/diagnostics and the already-established platform prerequisites remain
resident Application ownership across an ordinary RFB-attempt replacement; the
remote/session-scoped owners listed above are rebuilt.

## Commits

The complete source/test/docs commit range after the starting authority and
before this immutable log is:

1. `855e76c596eb7da0f6a64ff146a9c86c78550039` —
   `app: add typed RFB provider recovery policy`
2. `8a24e4952f0124be175277506a5af631c35ce81f` —
   `test(app): script typed provider failure attempts`
3. `165e460ad3a854ec4239dd23689df8a6f6c78246` —
   `test(app): prove R16B fresh-attempt recovery`
4. `bfda7f0f4bc0e0fe962ebe849019ed03f85053bb` —
   `test(pi): prove fresh RFB attachment per replacement session`
5. `0fb93ece595100305cd4897a882fcea58f8e709c` —
   `docs(rfb): record R16B application recovery lifecycle`
6. `bd13d7fa065b8f0e3fc7d506b8bbc4b2ab606df5` —
   `docs(symbols): describe R16B application recovery`
7. `ca404665a0b1200de20f7dff3ed74aa3f4c41d3b` —
   `test(pi): observe R13 request-state confirmation`
8. `3310568e6c35ef59d77ee6075f1f9ea5a561476e` —
   `test(pi): observe R13 commit-state confirmation`

The final startup-to-pre-log comparison is 8 commits ahead / 0 behind.

Changed paths are limited to:

- `src/app.c`
- `src/SYMBOLS.md`
- `tests/unit/app_test.c`
- `tests/unit/app_test_legacy.inc`
- `tests/unit/pi_rfb_attachment_test.py`
- `tests/unit/pi_rfb_attachment_test_legacy.py`
- `docs/development/rfb-provider-failure.md`

No Transport/RFB product lower-layer source, Wire protocol source, R13
representation, R14 selected values, AUDIO source, MPEG source, CONFIG-on-Wire,
heartbeat/liveness, direct-RFB fallback, or final all-guns orchestration was
changed.

## Deterministic behavior evidence

Application host fixtures now prove:

- CONNECT, READ, and WRITE provider causes each reach the Application-owned
  recovery policy;
- a provider READ failure closes ordinary admission before input shutdown,
  Transport abort, and fresh connection;
- no input queue pop, key publication, pointer publication, or RFB update
  request is admitted after the observed failed receive and before teardown;
- WRITE failure likewise closes admission before teardown/reconnect;
- provider CONNECT failure before input startup still retires the containing
  Transport attempt before replacement;
- repeated READ then WRITE recovery uses three separately connected
  descriptors, three fresh RFB/framebuffer/input initializations, and neutral
  center pointer publication for each attempt;
- graphics initialization remains resident and occurs once across those
  replacement attempts;
- generic RFB I/O failure remains fatal and does not enter provider recovery;
- failed input shutdown blocks replacement;
- failed Transport abort blocks replacement.

Pi fixture evidence proves two consecutive Wire sessions receive distinct Q4
session identities and allocate distinct RFB attachment objects rather than
rebinding the dead attachment.

Existing canonical lower-layer tests continue to prove:

- stale Transport access tickets return CLOSED across reconnect;
- replacement access receives a distinct opaque ticket;
- repeated Wire sessions do not resume old rider authority;
- session-B RFB relay queue/credit state starts fresh;
- R16A provider ERROR representation remains distinct from DATA/CREDIT and does
  not falsely convert provider-local failure into physical Wire failure;
- R13 REQUEST/BOUNDARY/COMMIT/COMPLETE behavior remains intact.

## Intermediate CI scheduling defects and corrections

The first final-shape exact-head CI attempt at
`bd13d7fa065b8f0e3fc7d506b8bbc4b2ab606df5`, workflow run
`35804659533`, showed the newly-added replacement-attachment test passing but
exposed an unrelated pre-existing host scheduling race in
`test_request_boundary_drains_and_retires_before_commit_complete`.

The test had already received the serialized R13 REQUEST frame, then immediately
asserted the sender thread's adjacent local state publication and sometimes
observed `REQUEST_PENDING` before `WAIT_BOUNDARY`.

Commit `ca404665a0b1200de20f7dff3ed74aa3f4c41d3b` corrected only that test
observation by waiting for the explicit owned `WAIT_BOUNDARY` state. It did
not add a product delay, timeout-as-proof, or protocol change.

The next exact-head run `35804833915` progressed through REQUEST and exposed
the same pre-existing adjacent publication race after receiving COMMIT:
`COMMIT_PENDING` could still be observed for one scheduling slice before
`WAIT_COMPLETE`.

Commit `3310568e6c35ef59d77ee6075f1f9ea5a561476e` made the analogous
test-only correction: preserve the COMMIT byte/sequence/payload assertions, then
observe the explicit owned `WAIT_COMPLETE` state rather than racing it.

No R13 product source or lifecycle representation changed in either correction.
The final exact-head host suite is green.

Two earlier attempted GitHub write-orchestration paths failed before any branch
mutation: one connector branch-URL parse rejection and one atomic-tree
precondition that noticed the legacy fixture intentionally lacked a trailing
newline. Neither produced a repository commit or discarded repository work.

## Acceptance-criterion worker dispositions

These are Reconstruction evidence dispositions for Foreman review, not
self-acceptance.

- `A003-R16B-C1 TYPED_PROVIDER_FAILURE_REACHES_APPLICATION_POLICY` — MET.
  Application explicitly branches on PROVIDER_CONNECT, PROVIDER_READ, and
  PROVIDER_WRITE; deterministic host tests exercise all three.
- `A003-R16B-C2 FAILED_ATTEMPT_ADMISSION_CLOSES_BEFORE_TEARDOWN` — MET.
  Event-order fixtures prove no post-failure input/RFB publication enters the
  failed attempt before shutdown begins.
- `A003-R16B-C3 INPUT_RFB_TRANSPORT_COMPLETE_STOP_PROVEN` — MET.
  Recovery requires successful input-runtime shutdown and successful
  Transport-owned abort/receiver/session retirement; either failed proof blocks
  replacement. RFB attempt authority is abandoned and freshly initialized.
- `A003-R16B-C4 DEAD_ATTEMPT_AUTHORITY_NEVER_REBOUND` — MET.
  Replacement uses the ordinary fresh startup path; stale Transport access and
  Pi attachment tests prove dead authority is not rebound.
- `A003-R16B-C5 RESTORATION_USES_FRESH_NETWORK_Q4_TRANSPORT_AND_ATTACHMENT_AUTHORITY`
  — MET. Repeated-attempt fixtures show fresh network descriptors; canonical
  Transport tests show fresh access authority; Pi fixtures show distinct Q4
  session IDs and distinct attachment objects.
- `A003-R16B-C6 RFB_FRAMEBUFFER_INPUT_STATE_FRESH_ON_RESTART` — MET.
  Repeated recovery reconstructs RFB session, framebuffer initialization/
  initial frame, input runtime, and neutral published pointer state on every
  attempt.
- `A003-R16B-C7 STALE_TICKET_CREDIT_SEQUENCE_PROVIDER_WAKE_AND_INPUT_STATE_CONTAINED`
  — MET. Canonical Transport/Pi stale-access, fresh relay-state, and
  fresh-attachment tests remain green; repeated Application attempts rebuild
  input/publication authority.
- `A003-R16B-C8 PHYSICAL_WIRE_FAILURE_REMAINS_DISTINCT_FROM_PROVIDER_FAILURE`
  — MET. Application recovers only the three provider-local typed causes;
  generic RFB I/O remains fatal. Accepted R16A distinction tests remain green.
- `A003-R16B-C9 NO_SUCCESS_BY_DELAY_OR_IMPLICIT_COMPONENT_RESTART` — MET.
  Recovery uses owner-state completion results and fresh ordinary startup; no
  recovery delay/backoff or in-place restart was added.
- `A003-R16B-C10 R16A_R13_R14_LOWER_LAYER_CONTRACTS_UNCHANGED` — MET.
  No relevant lower-layer product source or R14 values changed; exact R16A
  waiter proof and R13 suite are green. The two R13 fixture edits only remove
  host-scheduling races by observing explicit owner state.
- `A003-R16B-C11 NO_MEDIA_PROTOCOL_HEARTBEAT_OR_RETUNE_SCOPE_CREEP` — MET.
  Final changed-path review contains no AUDIO/MPEG/Wire-protocol/heartbeat/
  flow-retune/final-orchestration product changes.
- `A003-R16B-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact-head canonical CI is green for host unit, project check, strict
  dictionaries, pinned PS2 compile, PS2 link, and reproducibility.

## Exact machine evidence

Final pre-log source authority:

`3310568e6c35ef59d77ee6075f1f9ea5a561476e`

Exact-head GitHub Actions workflow:

`Ledge reconstruction checks`
run `35804891365`
conclusion `success`

Job results:

- `host-unit` — SUCCESS, including `app R15/R16B tests: PASS`, all 13 Pi
  RFB-attachment tests, all 20 Pi Wire-server tests, and dedicated
  `transport_rfb_provider_failure_test: PASS`;
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS with the pinned PS2 toolchain image;
- `ps2-link` — SUCCESS;
- linked-current-source reproducibility — PASS;
- identity-ready linked ELF artifact upload — SUCCESS.

Final linked identity:

`ELF_PRISTINE_SHA256=9b98aaa5b5239eec42545d111e7d32cedb418273fe4092f3deb5d8bd6690c40c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Accepted R16A authority had:

`ELF_PRISTINE_SHA256=06a1d2858dd195458bab81d8e88842d4fca4d3e3aa38d412c8b303fc9f8b4de2`
`PT_LOAD_SHA256=db94f95160ec28491546d53231ecf9235fcacbae1680875ac7f16bcd1b77bff8`
`PT_LOAD_BYTES=486932`

Therefore the R16B linked bytes changed and no historical hardware qualification
is transferred.

## Evidence classification and limitations

`SOURCE_COMPLETE=YES_RECONSTRUCTION_CANDIDATE`
`HOST_TESTED=YES`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35804891365`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`

Machine/build evidence does not qualify hardware. No PS2/Pi physical runtime
qualification was performed in this Reconstruction packet.

## Next pickup

Reconstruction stops here. Foreman owns independent inspection, acceptance or
rework disposition, integration/state publication, and any next bounded packet.

`NEXT_PICKUP=FOREMAN_REVIEW_OF_A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`
