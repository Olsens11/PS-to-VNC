DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T21:35:55-04:00
COMPLETED_AT=2026-09-22T22:08:31-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=826d19dd08efe48db9731ed4713f30b15be0e4b8
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
HARDWARE_PENDING=YES

# Reconstruction shift — R17 Pi MPEG generation control and producer

## Objective and authority consumed

This interactive Reconstruction worker recovered live branch authority from
`ledge/h1-all-guns` and executed only Foreman State revision `0048` packet:

`A003-PI-MPEG-CONTROL-PRODUCER-R17`

The exact starting branch authority was:

`826d19dd08efe48db9731ed4713f30b15be0e4b8`

(`docs(work-log): accept R16B and hand off Pi MPEG R17`).

The accepted R16B source authority consumed by this packet was
`3310568e6c35ef59d77ee6075f1f9ea5a561476e`, with the R16B immutable worker
record `856a1065f5941c5149518ffe2a322bc81fe91642`.

Authority reviewed before and during implementation included AGENTS/CONTRIBUTING,
clean architecture/project intent, source naming/topology/module-lifecycle
guidance, Reconstruction Contract revision 0006, work-log contract revision
0007, Foreman State revision 0048, the A003 MPEG-generation audit, accepted PS2
Transport MPEG protocol/runtime source and tests, maintained Pi Wire/RFB source,
the accepted Configuration MPEG runtime profile, and historical H1 producer /
capture / suppression mechanisms strictly as forensic reference.

The GitHub connector exposes repository/branch and Actions authority but not the
developer Pi worktree. Staged/unstaged/untracked state in that external worktree
and `scripts/resume-state.sh` remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`. No external-worktree
cleanliness claim is made.

## Result

R17 is source-complete as a Reconstruction candidate for independent Foreman
review at exact pre-log source authority:

`cc7dc1237957bfd288addc8379caae47e83bc5a6`

The maintained Pi now has an exact MPEG generation-control/producer mechanism
that:

- mirrors the accepted PS2 START kind 11 and RETIRE kind 10 control ABI exactly,
  using control channel 0, zero flags, generation-control version 1, 44/12-byte
  big-endian payloads, and MPEG DATA/CREDIT on logical channel 4;
- validates active Q4 session identity, nonzero monotonically fresh generation,
  current injected desktop bounds, 16-pixel base alignment and exact
  suppression containment before a generation becomes LIVE;
- owns only one PREPARING/LIVE/RETIRING generation at a time and fails closed on
  stale, mismatched, duplicate, launch-failed or retirement-unproven authority;
- derives capture from the START base rectangle and prepares the exact
  generation-specific outer suppression footprint separately; inner matte stays
  PS2-local;
- owns one generation-scoped FFmpeg process, one reader thread and a bounded
  Configuration-projected stdout buffer, using backpressure instead of
  unbounded accumulation;
- exposes producer bytes only to the generation owner; neither producer nor
  generation code reads/writes the PS2-facing physical socket or allocates PSTV
  sequence numbers;
- accepts MPEG credit only through channel 4 and lets `WireConnectionOwner`
  remain the sole physical receive/send/global-sequence owner for MPEG DATA and
  RETIRE completion;
- closes new generation-N emission admission before retirement, waits for every
  already-admitted local physical-send lease to finish, then requires real
  producer process/thread and suppression-footprint retirement before allowing
  RETIRE completion;
- treats a deadline as failure detection only; timeout cannot manufacture
  successful retirement;
- retains a monotonic stale-generation fence while rebuilding producer,
  capture-plan, suppression footprint, local buffer and emission authority for
  N+1.

No final PS2 Application MPEG activation, decoder/presentation orchestration,
calibration acceptance, first-frame ownership, AUDIO activation, CONFIG-on-Wire,
heartbeat, direct-RFB fallback, or final all-guns composition was added.

## Configuration authority projection

R17 needed already-selected MPEG producer facts on the Pi without duplicating
laboratory literals. The existing A003-R7 values were therefore represented in
one machine-readable owner:

`src/config/mpeg_runtime_profile.json`

`scripts/generate-mpeg-runtime-profile.py` deterministically projects those
unchanged values to:

- `src/config/mpeg_runtime_profile_generated.h`;
- `pi/mpeg_runtime_profile_generated.py`.

The C runtime now consumes the generated constants with the same prior selected
values. The narrow Pi wrapper exposes only the accepted channel window, bounded
producer buffer capacity, maximum capture dimensions and frame rate needed by
R17.

R17 deliberately does not copy H1 bitrate, GOP, VBV or other experimental
encoding knobs into maintained product authority. The default FFmpeg mechanism
uses exact START geometry plus the Configuration-selected frame rate; hardware
qualification remains separate.

## Credit-authority clarification

Accepted PS2 Transport grants initial MPEG channel credit once per Wire Session
and returns channel-4 credit as queued bytes are consumed or discarded.
Generation START/RETIRE does not establish a second independent credit namespace.

R17 therefore keeps one bounded **Wire-session** MPEG credit balance. It does
not create or rebind generation-N credit identity. Generation-local producer
bytes, producer object, buffer, suppression state and emission leases are fully
retired and recreated for N+1.

The deterministic repeated-generation test intentionally leaves old generation
bytes buffered, retires N, then proves N+1 emits its new producer's bytes rather
than N's remainder while the still-valid Wire-session credit balance remains
bounded. The accepted A003 audit separately requires PS2 finalization to discard
residual old-generation receive bytes and return owed channel credit after exact
RETIRE completion.

This distinction is recorded explicitly for Foreman review of R17-C9 rather
than silently treating session-level credit as generation-level state.

## Commits

The complete starting-authority-to-pre-log range is 15 commits ahead / 0 behind:

1. `b72f58d00068ed63effd97bfaa38ee8035962867` —
   `config: project MPEG runtime profile to Pi`
2. `3d5f7d6d7ca6c90a1840ab71bd51536690880d67` —
   `protocol: mirror exact MPEG generation control`
3. `ba4d0893bbcc01efca1bd062ba2a6f3d41296850` —
   `config: expose MPEG channel window to Pi`
4. `ea5afa67ee9f9ce2df936338abe4b7747be0cf62` —
   `pi: add exact MPEG generation owner`
5. `1429416bbbc4bd7d5903da8ce2321bf8feecf749` —
   `pi: integrate MPEG rider with sole Wire owner`
6. `d4723940d1e67931f22a3d61ee6869c497101f27` —
   `test(pi): prove exact MPEG generation lifecycle`
7. `aabf33a06edf987369f3ba26e0acef367e30f073` —
   `test: enroll R17 Pi MPEG lifecycle suite`
8. `946a885557b3ab000b7785e2b7cf668ec5e9a8e7` —
   `test(pi): preserve R13 quiesce owner expression`
9. `b1fe02cd84b37e3b8b011e137eda99ddbc740fa3` —
   `docs(pi): record R17 MPEG generation boundary`
10. `dcfd475cdf7da9625845679681e58349b474e857` —
    `test(pi): observe R17 completion state publication`
11. `8cc27293fb37fe417c1c4d0e3f1573b9b0a01c8a` —
    `test(pi): strengthen R17 freshness and buffer proof`
12. `1f2a231986b290d0f0d8228f766b1aecb872b935` —
    `tooling(symbols): run deterministic dictionary reconciliation`
13. `bd7fa8cd578e4a1d69f9d50b0bcd69a61489d9db` —
    `docs(symbols): reconcile current clean definitions`
14. `48e1118a9cfd67f1c9dff8caf93adb54607ee786` —
    `docs(symbols): describe R17 Pi ownership`
15. `cc7dc1237957bfd288addc8379caae47e83bc5a6` —
    `docs: index R17 MPEG generation contract`

Changed paths are limited to:

- `docs/INDEX.md`;
- `docs/development/README.md`;
- `docs/development/mpeg-generation-control.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `pi/mpeg_generation.py`;
- `pi/mpeg_runtime_profile.py`;
- `pi/mpeg_runtime_profile_generated.py`;
- `pi/wire_protocol.py`;
- `pi/wire_server.py`;
- `scripts/check.sh`;
- `scripts/generate-mpeg-runtime-profile.py`;
- `scripts/pi/install-wire-runtime.sh`;
- `src/config/mpeg_runtime_profile.c`;
- `src/config/mpeg_runtime_profile.json`;
- `src/config/mpeg_runtime_profile_generated.h`;
- `tests/Makefile`;
- `tests/unit/pi_mpeg_generation_test.py`.

No PS2 Application, MPEG decoder/worker/backend, Display/Presentation, Input/UI,
RFB product source, AUDIO product source, Transport product source, R13/R14
semantics, or final all-guns orchestration source changed.

## Deterministic R17 evidence

The focused `pi_mpeg_generation_test.py` suite has 12 passing tests on the
final product/test source shape. It proves:

- byte-exact START/RETIRE payload/frame symmetry with the accepted PS2 ABI;
- malformed version/length/wrong-envelope rejection;
- active-session mismatch, zero generation, stale/repeated generation, base
  alignment, desktop bounds and suppression-containment rejection;
- capture base geometry distinct from the outer suppression geometry;
- one prepared/live generation and no replacement while live;
- producer launch failure fails closed and removes prepared suppression;
- bounded local producer buffering with an intentionally over-capacity fake
  stdout source;
- bounded channel-4 credit and exact-generation emission leases;
- RETIRE closes admission while an in-flight lease is still outstanding and
  waits for that lease before producer/suppression retirement;
- stuck/unproven retirement and explicit producer-retirement failure never
  become RETIRE completion;
- N retirement followed by a distinct N+1 producer/suppression generation,
  including proof that old buffered N bytes are not emitted by N+1;
- physical DATA and RETIRE-completion serialization through one
  `WireConnectionOwner` global sequence;
- runtime staging includes the maintained R17 imports while the default
  `wire_runtime.py` does not activate final MPEG composition.

The existing RFB/Wire tests remain green, including the R13 quiesce source
boundary proving no second Wire sender or timeout polling, RFB relay
credit/fragmentation, fresh-session RFB authority, Q4 establishment, and R16A
provider-terminal representation.

The current PS2 Transport MPEG test is green on the final exact-head rerun,
preserving accepted START/RETIRE representation, channel-4 queue/credit and
retirement-completion behavior.

## Intermediate CI observations and corrections

The first integrated host run found that the generalized Wire rider loop had
rewritten the exact source expression used by R13's repository-boundary guard:
`quiesce_wake = attachment.quiesce_wake_reader`. The R13 mechanism itself had
not changed. Commit `946a8855...` restored that canonical owner expression
verbatim inside the generalized loop rather than weakening the existing guard.

An early R17 integration test received the exact RETIRE-completion bytes and
then immediately inspected local controller state before the Wire-owner thread
could execute the adjacent completion-confirmation statement. Commit
`dcfd475c...` changed only the test observation to wait for explicit owner
publication of IDLE after the exact completion frame had already been proved.
No product delay or timeout-as-retirement proof was added.

One subsequent host run reproduced the already-recorded nondeterministic
`transport_runtime_test.c` observation at assertions 862/864 in unchanged PS2
Transport source. No Transport change was made.

On exact final pre-log source `cc7dc123...`, canonical run `35808953320`
attempt 1 reached another unchanged PS2 host-fixture scheduling observation in
`transport_mpeg_test.c` around asynchronous RETIRE/DATA publication. The
failed host job was rerun on the identical source without modification.
Attempt 2 passed `transport_mpeg_test`, the complete host suite and all 12 R17
focused tests. The overall workflow then completed SUCCESS.

Project-check also exposed one direct R17 documentation integration omission:
the new development note was missing from exhaustive `docs/INDEX.md`. Commit
`cc7dc123...` added that one index entry; project-check is green afterward.

## Acceptance-criterion worker dispositions

These are Reconstruction worker evidence dispositions for independent Foreman
review, not Foreman acceptance.

- `A003-R17-C1 PI_MPEG_CONTROL_CODEC_EXACTLY_MATCHES_ACCEPTED_PS2` — MET.
  Exact kind/channel/flags/version/44-byte START/12-byte RETIRE golden bytes and
  malformed cases are deterministic-test green.
- `A003-R17-C2 START_VALIDATES_ACTIVE_SESSION_GENERATION_AND_GEOMETRY` — MET.
  START requires the active session, nonzero fresh generation, current desktop
  containment, 16-pixel base alignment and suppression containment.
- `A003-R17-C3 ONE_EXACT_GENERATION_OWNER_STALE_CONTROL_REJECTED` — MET.
  The explicit state machine admits one generation and a monotonic high-water
  mark rejects stale/repeated authority.
- `A003-R17-C4 CAPTURE_AND_SUPPRESSION_PREPARED_FOR_EXACT_GENERATION` — MET.
  Exact base capture and distinct outer suppression footprint are prepared
  before LIVE/emission admission; failed producer launch removes preparation.
  R17 deliberately does not bind the footprint into RFB policy because final
  cross-domain Application composition remains excluded.
- `A003-R17-C5 PRODUCER_LIFETIME_AND_BUFFERING_BOUNDED_FAIL_CLOSED` — MET.
  One generation-scoped process/thread and Configuration-bounded condition
  buffer are owned explicitly; launch, reader, premature EOF and retirement
  failures remain failure.
- `A003-R17-C6 CHANNEL4_CREDIT_AND_SOLE_WIRE_SERIALIZATION_PRESERVED` — MET.
  MPEG CREDIT classification is channel 4 only, the controller enforces the
  selected session window, and all physical DATA/completion sends and global
  sequence allocation remain in `WireConnectionOwner`. Existing RFB credit
  tests remain independently green.
- `A003-R17-C7 RETIRE_CLOSES_ADMISSION_AND_DRAINS_INFLIGHT_SEND_LEASES` — MET.
  LIVE becomes RETIRING before waiting; no new lease is admitted and the
  deterministic race test proves retirement blocks until the admitted lease is
  finished.
- `A003-R17-C8 RETIRE_COMPLETION_ONLY_AFTER_REAL_PI_RETIREMENT` — MET.
  Completion becomes serializable only after producer process/thread and exact
  suppression preparation retire. Timeout/failure emits no successful local
  completion state.
- `A003-R17-C9 REPEATED_GENERATION_USES_FRESH_AUTHORITY_NO_STALE_REBIND` — MET
  with the explicit session-credit clarification above. N+1 receives a distinct
  producer/capture plan/suppression generation/buffer and cannot emit N's
  buffered remainder or reuse N's emission leases. The one surviving credit
  counter is Wire-session capacity authority rather than dead-generation
  identity; no generation-local credit namespace is rebound.
- `A003-R17-C10 RFB_Q4_R16A_R13_R14_AND_PS2_MPEG_CONTRACTS_UNCHANGED` — MET.
  No lower PS2 Transport/RFB product source or RFB product mechanism changed;
  canonical existing suites are green. The R13 source-expression repair
  preserved, rather than weakened, its ownership guard.
- `A003-R17-C11 NO_APPLICATION_PRESENTATION_AUDIO_HEARTBEAT_OR_FINAL_SCOPE_CREEP`
  — MET. Final changed-path review contains none of those product surfaces and
  the default Pi composition intentionally supplies no MPEG generation factory.
- `A003-R17-C12 HOST_PROJECT_DICTIONARY_AND_BUILD_EVIDENCE_GREEN` — MET.
  Exact final pre-log source has green canonical host, project, strict
  dictionary, pinned PS2 compile/link and current-source reproducibility
  evidence.

## Exact machine/build evidence

Final pre-log source authority:

`cc7dc1237957bfd288addc8379caae47e83bc5a6`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `35808953320`
final run attempt `2`
conclusion `success`

Final job classes:

- `host-unit` — SUCCESS after one identical-head retry of the unchanged
  Transport MPEG fixture; includes `transport_mpeg_test: PASS` and all 12
  `pi_mpeg_generation_test.py` tests PASS;
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS;
- `ps2-link` — SUCCESS;
- linked current-source reproducibility — PASS;
- dictionary reconciliation — correctly SKIPPED because dictionaries were
  already reconciled.

Current linked identity:

`ELF_PRISTINE_SHA256=615407bd6ce43722f6ead9ced75985d85bec18fa24e3e0f3a841c4af21894dfc`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`
`ISSUE7_LINKED_BUILD=PASS`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

The whole ELF hash differs from R16B, but the PT_LOAD fingerprint and byte count
are exactly unchanged from accepted R16B source. R16B itself remains physically
unqualified, so no new physical qualification is claimed or inherited here.
R17's new Pi producer/generation source likewise has no physical Pi/PS2 runtime
qualification in this packet.

## Evidence classification and limitations

`SOURCE_COMPLETE=YES_RECONSTRUCTION_CANDIDATE`
`HOST_TESTED=YES`
`FOCUSED_R17_TESTS=12_PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35808953320_ATTEMPT_2`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`

Repository/CI evidence does not qualify the reconstructed Pi producer on real
hardware and does not qualify R16A/R16B recovery behavior. No hardware or
user-assisted local action was required or performed by R17.

## Next pickup

Reconstruction stops here. The Foreman owns independent review, criterion
acceptance or rework disposition, state/integration publication, and selection
of any next bounded packet.

`NEXT_PICKUP=FOREMAN_REVIEW_OF_A003-PI-MPEG-CONTROL-PRODUCER-R17`
