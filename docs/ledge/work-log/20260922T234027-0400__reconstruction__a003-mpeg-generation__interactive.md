DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T23:40:27-04:00
COMPLETED_AT=2026-09-23T00:06:25-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=95d7b77e431f0f0765491da79e75591aae6e0300
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
HARDWARE_PENDING=YES

# Reconstruction shift — R18 PS2 Transport MPEG run boundary

## Objective and authority consumed

This interactive Reconstruction worker recovered live authority from
`ledge/h1-all-guns` and executed only Foreman State revision `0049` packet:

`A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`

The exact starting branch authority was:

`95d7b77e431f0f0765491da79e75591aae6e0300`

(`docs(work-log): accept R17 and hand off MPEG R18`).

The accepted R17 source authority consumed by this packet was
`cc7dc1237957bfd288addc8379caae47e83bc5a6`, with R17 immutable worker log
`8698890b4f839d5708e6cefc129c37608643e155`.

Authority reviewed included AGENTS/CONTRIBUTING, clean architecture/project
intent, Reconstruction Contract revision 0006, work-log contract revision 0007,
Foreman State revision 0049, A003 MPEG-generation audit authority,
`docs/development/module-lifecycle.md`, accepted PS2 Transport MPEG
runtime/channel/protocol source and fixtures, and the accepted R17 Pi producer
contract strictly as the other side of the existing START/RETIRE boundary.

The GitHub connector exposes repository/branch and Actions authority but not the
developer Pi worktree. Staged/unstaged/untracked state in that external worktree
remains `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`. No external
worktree cleanliness claim is made.

## Result

R18 is source-complete as a Reconstruction candidate for independent Foreman
review at exact pre-log source authority:

`2e0589fd9fe531c5dfb7130df301b6f0eadd09fd`

The PS2 Transport layer now owns the missing reusable MPEG run boundary around
its existing session-scoped channel-4 allocation without taking Application,
decoder, presentation, or product-generation ownership.

The maintained mechanism now provides:

- one explicit clean-idle MPEG run-open edge;
- one pre-START abort path that succeeds only while the opened boundary still
  has no START, queued bytes, pending credit, activity, producer-done,
  completion, retirement, or consumer transaction;
- channel-4 DATA admission only while a run boundary is open and neither
  retirement nor finalization owns the channel;
- START submission only from a still-clean opened boundary, preventing
  pre-START stale/early DATA from being promoted into a valid live run;
- exact RETIRE transaction correlation: Transport preserves the exact submitted
  RETIRE payload as correlation evidence only and accepts only its exact echoed
  completion; it does not allocate or interpret MPEG generation identity;
- atomic RETIRE-completion ordering in which DATA admission closes under the
  MPEG queue lock before the completion becomes observable to the higher owner;
- a retirement latch independent from the one-value completion slot, so taking
  completion never reopens DATA;
- exact residual queue discard that resets ring offsets and returns a byte count
  without misclassifying discarded bytes as decoder consumption;
- one finalization transaction that combines withheld consumed-byte credit with
  exact residual-discard credit and returns that sum once through the existing
  sole Transport physical sender;
- explicit finite-producer/run-state reset for safe reuse of the same
  session-scoped allocation;
- one protected MPEG consumer transaction spanning queue removal through
  consumed-byte credit bookkeeping/send, so finalization cannot race between
  consumption and credit return;
- finalization refusal while a protected activity waiter or active consumer
  transaction still owns the channel;
- a finalization-in-progress fence that refuses late MPEG read/status/activity
  wait/producer-done operations while Transport temporarily releases the queue
  lock to perform the existing synchronous outbound credit send;
- a final post-send invariant check that fails the Transport runtime rather than
  opening a successor boundary if any old-run queue, credit, waiter,
  producer-done, or consumer ownership reappears;
- a successful-reset path that clears activity sequence, START/RETIRE
  submission facts, exact RETIRE-correlation evidence, retirement/finalization
  latches, pending credit, residual queue state, offsets, and producer-done
  before a later explicit run-open can succeed.

No new timeout, sleep, watchdog, generation allocator, duplicate physical
sender, or diagnostics-only witness mechanism was added.

## Credit and ownership model

The accepted A003 design grants MPEG channel credit once per Wire Session and
returns it as bytes leave the bounded PS2 queue. R18 preserves that model.

Normal decoder reads can leave consumed-byte credit below the configured return
batch. A retiring run can also leave residual queued bytes that will never be
decoded. Finalization therefore snapshots both categories after the higher owner
has taken exact RETIRE completion and retired its consumer:

`credit_to_return = pending_consumed_credit + residual_discarded_bytes`

when the existing MPEG credit-return policy is enabled.

That amount leaves through the existing Transport outbound path and sole
physical-I/O owner exactly once. No successor run receives a new initial window
and no old-run bytes or credit debt are rebound as generation state.

The short-lived `mpeg_consumer_active` fact is synchronization ownership, not
diagnostic evidence: it remains asserted until a successful read's associated
credit bookkeeping/send is complete. This closes the race in which finalization
could otherwise reset `mpeg_credit_pending` after queue removal but before the
reader returned credit.

The stored expected RETIRE payload is likewise transaction-correlation evidence,
not generation ownership. Transport compares the exact decoded completion with
the exact RETIRE it submitted, while Application/Pi MPEG owners retain semantic
generation policy.

## Residual and finite-producer semantics

`pstvnc_transport_mpeg_channel_discard_all()` atomically reports the exact
residual count and resets queue offsets/count. It deliberately does not clear or
manufacture finite-producer completion.

`pstvnc_transport_mpeg_channel_reset_run_state()` is a separate operation that
refuses a non-empty queue and clears run-local producer-done/offset state only
after residual handling has completed.

This separation keeps:

- decoder-consumed bytes;
- finalizer-discarded residual bytes; and
- finite producer completion

as distinct facts.

## Deterministic R18 evidence

The final `transport_mpeg_test` suite is green and includes the prior MPEG
Transport/control coverage plus R18-specific cases proving:

1. DATA before explicit run-open protocol-fails the runtime;
2. clean run-open succeeds once and duplicate open fails closed;
3. clean pre-START abort returns to idle;
4. dirty pre-START state prevents both abort and START promotion;
5. ordinary channel-4 DATA remains bounded and credit-accounted while open;
6. exact RETIRE completion closes DATA admission before higher-owner
   observation;
7. taking completion does not clear the retirement latch or reopen admission;
8. post-completion DATA protocol-fails even after completion has been taken;
9. a RETIRE completion whose session/generation payload does not exactly match
   the submitted RETIRE protocol-fails without falsely closing the run;
10. wrapped residual ring state is discarded with the exact residual count and
    is not labeled consumption;
11. finalization returns pending-consumed plus residual-discard credit exactly
    once;
12. a second finalization attempt cannot return credit again;
13. old producer-done/activity/pending-credit/queue state is absent after
    successful finalization;
14. N finalization followed by N+1 open/read exposes only N+1 bytes/state;
15. a live protected activity waiter blocks finalization;
16. an active read/credit-return consumer transaction blocks finalization;
17. once finalization owns the channel, late read/status/activity-wait and
    producer-done publication are refused and cannot create successor-visible
    state.

The bridge fixture also proves the three new public run-boundary operations
preserve the existing opaque Transport access-ticket/stale-access fence.

Existing neighboring suites remain green, including RFB quiesce/provider
failure, AUDIO, MPEG decoder/worker/presentation, Q4, R16A, R16B, and the 12 R17
Pi producer tests.

## Reconstruction-owned corrections during the packet

### Bridge fixture linkage

The first integrated bridge build exposed that
`tests/unit/transport_bridge_test.c` intentionally links `bridge.c` against
fixture-local fake runtime functions rather than the real runtime. The three new
bridge calls therefore initially lacked fake definitions. Commit
`f3c421d7e072f1528d352a741b73437ddc1c47fc` added only the corresponding
fixture seams and result mapping.

### Finalization lock release

Review of the first finalization implementation found a short-circuit semaphore
release expression that could skip the MPEG queue release if the control release
failed. Commit `cb5b7b89fb2c26b7cbabfba9ebdda28dc7f9b6a8` made both release
paths explicit.

### Late-consumer / credit-return race

After the first reconciled source had already achieved a green canonical run,
closing review found a real R18-owned race: finalization releases the MPEG queue
lock while the existing sole physical sender returns final credit. Without an
additional ownership fence, a new activity waiter or producer-done publication
could enter during that interval. Separately, an MPEG read removed bytes under
the queue lock but returned their consumed-byte credit after releasing it,
allowing finalization to race between those two halves of the transaction.

This was not accepted as completed work.

Commit `1406100933666b8a19f800ec7f0d90548b03d06b` corrected the model by:

- keeping a successful MPEG read Transport-visible until its credit transaction
  completes;
- refusing finalization while that consumer transaction or an activity waiter
  is live;
- blocking late MPEG consumer/activity/producer-done operations while
  finalization is in progress;
- requiring an invariant-clean state again after the external credit send
  before publishing reusable idle;
- correlating RETIRE completion with the exact submitted RETIRE payload.

Commit `0bce79ccf60dc7693b1950ebae2b4460b2ccb8b7` added deterministic regression
coverage for those exact edges. Final machine evidence below is from the
corrected authority, not the earlier green checkpoint.

## Known host-fixture timing observations

Two intermediate workflows reproduced the already-recorded nondeterministic
RFB host-fixture publication race at unchanged
`tests/unit/transport_runtime_test.c:862`:

- behavior-head workflow `35815838197` attempt 1 first missed the asynchronous
  RFB publication and cascaded into later quiesce assertions; identical-head
  attempt 2 passed the complete host suite including `transport_mpeg_test`;
- dictionary-trigger workflow `35816149336` reproduced only the line-862
  observation.

No RFB source or RFB fixture behavior was changed for R18. The final exact R18
authority workflow passed `transport_runtime_test: PASS` on attempt 1.

## Dictionary reconciliation

R18 used the repository-owned deterministic reconciliation workflow rather than
hand-authoring mechanical dictionary rows.

The first R18 reconciliation added 45 current Transport definitions and removed
none. Closing-review concurrency state then added additional real definitions;
the final reconciliation at commit
`68b05acb399b3b19c280c1c64b5e5d984b4a018d` reported:

`DICTIONARY_RECONCILED=src/transport before=1343 removed=0 added=6`
`DICTIONARY_RECONCILIATION_REMOVED=0`
`DICTIONARY_RECONCILIATION_ADDED=6`
`SOURCE_DICTIONARIES=PASS`

One earlier no-content trigger commit
`81e67b3e5b234695cd25ba79f6ec4124b8cee968` used a noncanonical reconciliation
message and therefore correctly did not invoke the message-gated bot. It changed
no repository tree content. The canonical trigger was then used.

## Commits

The complete starting-authority-to-pre-log range is 17 commits ahead / 0 behind:

1. `d5abfc1bcad0b2920cbb0d6ab0b876b03a482104` —
   `transport: add MPEG residual discard primitive`
2. `39de081552426bcbe0bd2aee53c9962d01e2b87e` —
   `transport: add exact MPEG run boundary`
3. `cb5b7b89fb2c26b7cbabfba9ebdda28dc7f9b6a8` —
   `transport: keep MPEG finalization lock release explicit`
4. `662895a9e27ab6b2fba8a3cd9086ea52fff9a2d8` —
   `transport: expose MPEG run boundary through bridge`
5. `f3c421d7e072f1528d352a741b73437ddc1c47fc` —
   `test(transport): map MPEG run boundary through bridge`
6. `bfaa4a1f8bd23ecc25fd828bf0571e3b90c704df` —
   `test(transport): prove R18 MPEG run finalization`
7. `bfa81bab93e03073371c47ace28ad474f7cbadbd` —
   `docs(transport): record R18 MPEG run boundary`
8. `81e67b3e5b234695cd25ba79f6ec4124b8cee968` —
   `tooling(symbols): reconcile R18 Transport definitions`
9. `fd8a19b77476e942e74f6aa750680e419e46fb78` —
   `tooling(symbols): run deterministic dictionary reconciliation`
10. `1800ba9abd4f7eaa9d92a76fea22ac32728edbbe` —
    `docs(symbols): reconcile current clean definitions`
11. `795df1cc6827d8c708b9f4124126495215967919` —
    `test: verify reconciled R18 Transport authority`
12. `1406100933666b8a19f800ec7f0d90548b03d06b` —
    `transport: fence MPEG finalization against late consumers`
13. `0bce79ccf60dc7693b1950ebae2b4460b2ccb8b7` —
    `test(transport): prove R18 late-consumer and exact-retire fences`
14. `f80a5d8dafe9f57ed4df13cd021a9f063685221d` —
    `docs(transport): record R18 finalization concurrency fence`
15. `d08dca3d2f24c55aa8abfd9bc1693d42e411a6e0` —
    `tooling(symbols): run deterministic dictionary reconciliation`
16. `68b05acb399b3b19c280c1c64b5e5d984b4a018d` —
    `docs(symbols): reconcile current clean definitions`
17. `2e0589fd9fe531c5dfb7130df301b6f0eadd09fd` —
    `test: verify final R18 Transport authority`

The final changed-path set is limited to:

- `docs/development/module-lifecycle.md`;
- `docs/development/mpeg-generation-control.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `src/transport/SYMBOLS.md`;
- `src/transport/bridge.c`;
- `src/transport/bridge.h`;
- `src/transport/mpeg_channel.c`;
- `src/transport/mpeg_channel.h`;
- `src/transport/runtime.c`;
- `src/transport/runtime.h`;
- `tests/unit/transport_bridge_test.c`;
- `tests/unit/transport_mpeg_test.c`.

No `src/app.c`, Pi product source, MPEG decoder/worker/backend,
Display/Presentation, calibration/Input/UI, RFB product source, AUDIO product
source, or protocol framing source changed.

## Acceptance-criterion worker dispositions

These are Reconstruction worker evidence dispositions for independent Foreman
review, not Foreman acceptance.

- `A003-R18-C1 MPEG_RUN_BOUNDARY_OPENS_ONLY_FROM_CLEAN_IDLE_TRANSPORT` — MET.
  Run-open requires empty queue, zero pending credit/activity, no waiter,
  producer-done, completion, expected RETIRE, consumer transaction, retirement,
  START/RETIRE submission, or finalization state.
- `A003-R18-C2 PRE_START_ABORT_RESTORES_PROVEN_CLEAN_BOUNDARY` — MET.
  Clean pre-START abort resets the channel and closes admission; dirty
  pre-START DATA makes both abort and START promotion fail closed.
- `A003-R18-C3 MPEG_DATA_ADMISSION_REQUIRES_OPEN_NONRETIRING_RUN` — MET.
  Non-empty channel-4 DATA is rejected unless the explicit boundary is open and
  retirement/finalization do not own it.
- `A003-R18-C4 RETIRE_COMPLETION_ATOMICALLY_CLOSES_DATA_ADMISSION` — MET.
  Only the exact echoed submitted RETIRE completion closes admission under the
  queue lock before the higher-owner completion slot is published.
- `A003-R18-C5 COMPLETION_TAKE_DOES_NOT_REOPEN_CHANNEL` — MET.
  Taking the one-value completion clears only that slot; the independent
  retirement latch remains until successful finalization.
- `A003-R18-C6 RESIDUAL_QUEUE_DISCARD_IS_ATOMIC_AND_NOT_CONSUMPTION` — MET.
  The ring discard primitive returns exact wrapped residual count and resets
  offsets/count separately from producer state and decoder consumption.
- `A003-R18-C7 RESIDUAL_PLUS_PENDING_CREDIT_RETURNED_EXACTLY_ONCE` — MET.
  Finalization combines withheld consumed-byte credit with residual-discard
  credit and returns the exact sum once through the existing Transport sender;
  repeat finalize cannot send again.
- `A003-R18-C8 FINALIZATION_RESETS_OLD_RUN_TRANSPORT_STATE` — MET.
  Successful finalization clears queue/offset, producer-done, pending credit,
  activity sequence, completion/expected-RETIRE facts, START/RETIRE facts,
  consumer state and retirement/finalization latches.
- `A003-R18-C9 LIVE_WAITER_OR_UNPROVEN_CONSUMER_BLOCKS_FINALIZATION` — MET.
  Deterministic tests prove both protected waiter and active read/credit-return
  transaction block finalization; finalization-in-progress also refuses late
  consumer/activity/producer-done operations.
- `A003-R18-C10 FRESH_SUCCESSOR_RUN_CANNOT_OBSERVE_OLD_RUN_STATE` — MET.
  N finalization -> N+1 open/read proves only new bytes/state and clean
  producer/activity/credit authority.
- `A003-R18-C11 RFB_AUDIO_WIRE_R17_AND_MEDIA_OWNER_CONTRACTS_UNCHANGED` — MET.
  Changed paths stay inside authorized Transport/tests/docs/dictionaries;
  canonical neighboring suites including RFB/AUDIO/R17 are green and no
  Application/media implementation source changed.
- `A003-R18-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact final pre-log source has green canonical host, project, strict
  dictionary, PS2 compile/link and current-source reproducibility evidence.

## Exact machine/build evidence

Final pre-log source authority:

`2e0589fd9fe531c5dfb7130df301b6f0eadd09fd`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `35816881762`
run attempt `1`
conclusion `success`

Final job classes:

- `host-unit` — SUCCESS, including:
  - `transport bridge tests passed`;
  - `transport_runtime_test: PASS`;
  - `transport_mpeg_test: PASS`;
  - `transport_rfb_provider_failure_test: PASS`;
  - existing R17 Pi MPEG suite: 12 PASS.
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS;
- `ps2-link` — SUCCESS;
- linked current-source reproducibility — PASS;
- dictionary reconciliation — correctly SKIPPED because final dictionaries
  were already reconciled.

Current linked identity:

`ELF_PRISTINE_SHA256=4d0bc02f6bc89f138acb53ddb7751fd9e91a300b9e7cbfd9bc7301920f08d1c4`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=e244769ef21dadd8a09e6fe65ec4b2201e0acd9beb52ae06180ab3d4f3a3a232`
`PT_LOAD_BYTES=490516`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`
`ISSUE7_LINKED_BUILD=PASS`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

The PT_LOAD identity changes from accepted R17 because R18 deliberately changes
loadable PS2 Transport runtime code. This is build/source identity only; it is
not physical qualification.

## Evidence classification and limitations

`SOURCE_COMPLETE=YES_RECONSTRUCTION_CANDIDATE`
`HOST_TESTED=YES`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`MACHINE_EVIDENCE=GITHUB_ACTIONS_RUN_35816881762_ATTEMPT_1`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`

No hardware/user-assisted action was required or performed by R18.

## Next pickup

Reconstruction stops here. The Foreman owns independent review, R18 criterion
acceptance or rework disposition, state/integration publication, and selection
of any next bounded packet.

`NEXT_PICKUP=FOREMAN_REVIEW_OF_A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`
