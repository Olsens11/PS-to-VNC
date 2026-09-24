DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T06:17:35-04:00
COMPLETED_AT=2026-09-24T06:34:06-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=003306d399c22f2a02625a867a512c225931a87a
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R20C Transport receiver completion fence

## Timestamp provenance

The exact `STARTED_AT` above was captured after read-only recovery of current
repository/Foreman authority and before the first R20C source mutation. No
earlier wall-clock value is invented for that read-only recovery interval.

## Objective and authority consumed

This interactive Reconstruction worker recovered current authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0053`, whose only
active Reconstruction packet was:

`A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C`

The assigning branch authority was:

`003306d399c22f2a02625a867a512c225931a87a`

R20C was activated after State-0052 validation exposed a latent A001 teardown
race. The sole Transport physical-I/O owner published
`runtime->receiver_done = 1` before completing terminal work, while
`pstvnc_transport_runtime_wait_receiver_done()` and
`pstvnc_transport_runtime_release()` treated that early flag as if it proved
the receiver would never again touch Transport-owned state.

The active packet required a truthful completion rendezvous after pending
outbound resolution and all enabled logical-owner terminal publication, while
preserving the early admission-closing fact, failed-shutdown convergence,
retryable pre-reclaim ownership, existing rider semantics and fixed Wire bytes.

Governing authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0006;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0053;
- the assigning Foreman immutable record
  `docs/ledge/work-log/20260924T001300-0400__foreman__a003-mpeg-generation__interactive.md`;
- `docs/development/module-lifecycle.md`;
- current `src/transport/runtime.*`;
- current Transport runtime host fixture;
- accepted A001 sole-physical-I/O ownership;
- accepted R13/R16A/R16B/R18/R20 neighboring Transport contracts.

State 0053 explicitly queued
`A003-APPLICATION-MPEG-RUN-START-R21`. R21 was not authorized and was not
executed by this shift.

The GitHub connector exposes repository/branch authority but not the external
developer Pi worktree. External staged/unstaged/untracked state and local
`scripts/resume-state.sh` execution remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim is made about
external checkout cleanliness.

## Result

R20C is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3`

The correction does not add another public lifecycle flag, epoch or generation.
It gives the already-existing receiver completion semaphore its intended
ownership meaning.

### Early terminal fact remains admission-closing only

The receiver still publishes:

`runtime->receiver_done = 1`

before terminal dispatch is complete.

That ordering remains useful and intentional: outbound submission, logical
reads/writes and activity paths can see a dying Wire Session early and refuse
new work. R20C does **not** move that flag to the end or weaken the race-closing
admission check.

The flag is now explicitly not reclaim authority.

### Final completion event is the no-touch ownership fence

The receiver continues its terminal sequence after the early flag:

1. fail/resolve any racing `outbound_pending` transaction;
2. wake an RFB writer blocked on provider credit;
3. publish terminal RFB activity;
4. publish terminal AUDIO activity when AUDIO is enabled;
5. publish terminal MPEG activity when MPEG is enabled;
6. only then signal `receiver_done_semaphore_id`;
7. execute `ExitThread()`.

R20C adds the private helper:

`pstvnc_transport_runtime_pass_receiver_completion_fence()`

It synchronizes on `receiver_done_semaphore_id`, then restores the binary
token. The receiver-done semaphore therefore acts as a latched completion
rendezvous rather than a one-consumer diagnostic pulse.

The restore allows `wait_receiver_done()`, `release()`, or a later retry to
observe the same already-completed no-touch edge without inventing a second
completion variable.

### wait_receiver_done() is truthful

`pstvnc_transport_runtime_wait_receiver_done()` no longer short-circuits merely
because the early `receiver_done` flag is visible.

For a started receiver, it must pass through the final completion rendezvous
before reporting success.

This means a caller blocked while the I/O owner is still resolving pending
outbound work or publishing logical terminal activity cannot falsely report
receiver completion.

### release() cannot reclaim from the early flag

`pstvnc_transport_runtime_release()` retains its prior nonblocking refusal
while the receiver has not even published early terminality.

Once early `receiver_done` becomes visible, release must pass the final
completion fence **before** it:

- inspects live rider-wait ownership;
- calls `ReferThreadStatus()`;
- calls `TerminateThread()`;
- calls `DeleteThread()`;
- deletes Transport semaphores;
- frees RFB/AUDIO/MPEG queue storage;
- frees the receiver stack allocation;
- releases the physical stream.

If the post-fence kernel status still reports the already-no-touch receiver
RUNNING, the existing forced-DORMANT fallback remains legal. It now occurs only
after the final completion event proves the receiver has no remaining
Transport-owned resource access.

A pre-reclaim `ReferThreadStatus()` failure remains retryable and preserves all
session resources for a later release attempt.

## Deterministic host synchronization proof

R20C does not use sleeps, polling delays, test timeouts, or rerun luck as a
success condition.

The host fixture gained explicit condition-variable rendezvous around existing
host-only stubs.

### Pending outbound + terminal rider proof

`test_receiver_completion_event_is_real_no_touch_fence()` deterministically:

1. creates one RFB+AUDIO+MPEG Transport runtime;
2. arms real AUDIO and MPEG activity waiters;
3. blocks a real RFB writer on zero provider credit;
4. holds the sole I/O owner inside the host readiness pass;
5. starts a direct Transport submitter and holds its outbound-ready publication
   after `outbound_pending` is already visible;
6. queues an invalid inbound frame so the owner must take the terminal path;
7. releases the readiness barrier while holding the final receiver-completion
   signal;
8. proves early `receiver_done` is visible while final completion is not;
9. proves the pending outbound slot is resolved before final completion;
10. proves the RFB credit waiter and AUDIO/MPEG activity waiters all return and
    clear their protected wait ownership before final completion;
11. starts a real `wait_receiver_done()` caller and proves it remains blocked
    despite early `receiver_done`;
12. releases the final completion barrier and proves the wait then succeeds;
13. performs safe release afterward.

The readiness barrier was added because the first version of this **test
apparatus** could deadlock itself: with `outbound_pending` visible while the
outbound-ready token was intentionally withheld, the host receiver could loop
to the outbound path and wait for the same deliberately blocked token before it
processed the terminal inbound frame. The corrected fixture holds the owner
inside one readiness pass first, making the intended pending-outbound/terminal
ordering deterministic without changing product source.

### Direct release proof

`test_release_waits_for_receiver_completion_before_reclaim()`
deterministically holds the receiver at the final completion publication after
early terminality is visible, then starts `release()` concurrently.

Before the completion barrier is released, the fixture proves:

- no `ReferThreadStatus()`;
- no `TerminateThread()`;
- no `DeleteThread()`;
- no physical-stream release;
- queue storage remains owned;
- receiver stack allocation remains owned.

After the completion barrier is released, the fixture forces the first
post-fence thread-status observation to RUNNING and proves:

`receiver completion signal < TerminateThread < DeleteThread < physical release`.

### Existing failure/retry proofs retained

The existing fatal-stop fixture remains green and proves:

- failed physical `shutdown_io` reports failure but `stop_requested` remains
  visible to the bounded readiness loop;
- receiver completion still converges without timeout-as-success;
- a first `ReferThreadStatus()` failure leaves the runtime initialized and
  preserves queue storage, receiver stack and physical ownership;
- retry succeeds after the real completion fence;
- fresh runtime reuse resets terminal/quiesce state.

R20C strengthens that fresh-runtime proof with explicit assertions that the
reused runtime has:

- `receiver_thread_started == 0`;
- `receiver_thread_id == -1`;
- a newly created receiver completion semaphore;
- zero count in that completion semaphore;
- `receiver_done == 0`.

No old completion token or receiver-thread authority can therefore be inherited
by the fresh runtime.

## Repeated synchronization evidence

The reconciled R20C product/test authority at
`44eb58a45f81ed8cef0cf1a3b51ff925e9f0844d` was executed unchanged through
GitHub Actions run `35987476377` three times.

Attempts 1, 2 and 3 each completed the host-unit job successfully. Attempt 3
also completed all canonical workflow gates successfully.

The final strengthened source
`4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3`, which differs from
`44eb58a...` only by explicit fresh-runtime completion/thread assertions in the
host fixture, then passed the complete canonical workflow independently in run
`35987788623` attempt 1.

The repeated proof is synchronization-driven; no source, delay, timeout or retry
condition was changed between the three `44eb58a...` host executions.

## Superseded host-fixture observation

The first barrier-fixture commit
`cb977b9dc8df2bf54751a4f3a977a1246401f467` started workflow run
`35986763935`.

Its initial host fixture contained the apparatus self-deadlock described above.
At closeout that superseded workflow remained in-progress at its canonical host
step. It is not product evidence and no product defect is inferred from that
stalled run.

The fixture-only correction landed in
`54a694fcf124777d5a99c3fdec991f57067a49df`. Exact workflow
`35987046272` then passed host-unit, project-check, PS2 compile and PS2 link/
reproducibility. Its only red gate was the expected strict dictionary drift for
the newly introduced private product helper.

Product source did not change between `cb977b9...` and `54a694f...`.

## Documentation and dictionaries

`docs/development/module-lifecycle.md` now records the R20C distinction:

- early receiver terminality closes admission;
- final receiver completion is the no-touch reclaim fence;
- wait/release synchronize through the final event;
- the binary completion token is retained for later observers/retries;
- timeout or favorable scheduling is never completion proof.

The canonical dictionary reconciler added exactly the missing
`pstvnc_transport_runtime_pass_receiver_completion_fence` function/parameter
definitions plus the generated portal update. No product source changed during
dictionary reconciliation.

## Commits

The complete pre-log R20C range is eight commits ahead / zero behind the
assigning Foreman authority:

1. `db95ac2f7f77ce87f4cc763042e224e40e3b6c6d` —
   `transport: make receiver completion fence reclaim-authoritative`
2. `cb977b9dc8df2bf54751a4f3a977a1246401f467` —
   `test(transport): prove R20C receiver completion fence`
3. `54a694fcf124777d5a99c3fdec991f57067a49df` —
   `test(transport): make R20C outbound race deterministic`
4. `819d8687a7006797aa74b11dcb455c066ea1fd19` —
   `docs(transport): record R20C receiver completion fence`
5. `d09329bcf59eb289792552de49367fbf44b194a9` —
   `tooling(symbols): run deterministic dictionary reconciliation`
6. `92e2ed69b4b64fdc9a6ae8d991cebd7945b1456e` —
   `docs(symbols): reconcile current clean definitions`
7. `44eb58a45f81ed8cef0cf1a3b51ff925e9f0844d` —
   `test: verify final R20C receiver completion authority`
8. `4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3` —
   `test(transport): prove fresh R20C completion authority`

## Changed paths

The complete assigning-Foreman-to-pre-log diff is confined to the authorized
R20C Transport lifecycle/test/documentation/dictionary surfaces:

- `src/transport/runtime.c`
- `tests/unit/transport_runtime_test.c`
- `docs/development/module-lifecycle.md`
- `src/transport/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

Notably unchanged:

- `src/transport/runtime.h`;
- Transport protocol/frame codecs;
- Transport bridge public MPEG semantics;
- `src/app.c`;
- R21 Application support;
- `src/app_mpeg_frame.*`;
- Pi product source;
- RFB parser/session/flow-policy product source;
- MPEG decoder/worker/backend;
- Display/Presentation;
- calibration/Input/UI;
- AUDIO product behavior;
- Configuration product source.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A001-R20C-C1 EARLY_TERMINAL_STATE_IS_NOT_RECLAIM_COMPLETION_AUTHORITY` — MET.
- `A001-R20C-C2 WAIT_RECEIVER_DONE_SYNCHRONIZES_AFTER_ALL_TERMINAL_OWNER_WORK` — MET.
- `A001-R20C-C3 RELEASE_CANNOT_TERMINATE_OR_DELETE_BEFORE_COMPLETION_FENCE` — MET.
- `A001-R20C-C4 PENDING_OUTBOUND_AND_LOGICAL_TERMINAL_WAKES_PRECEDE_RECLAIM` — MET.
- `A001-R20C-C5 FAILED_SOCKET_SHUTDOWN_STILL_CONVERGES_WITHOUT_TIMEOUT_SUCCESS` — MET.
- `A001-R20C-C6 PRE_RECLAIM_KERNEL_STATUS_FAILURE_PRESERVES_RETRYABLE_OWNERSHIP` — MET.
- `A001-R20C-C7 STACK_SEMAPHORES_QUEUES_AND_STREAM_RECLAIM_ONLY_AFTER_NO_TOUCH_PROOF` — MET.
- `A001-R20C-C8 FRESH_RUNTIME_REUSE_CANNOT_INHERIT_COMPLETION_AUTHORITY` — MET.
- `A001-R20C-C9 DETERMINISTIC_BARRIER_PROVES_PRE_COMPLETION_RACE_IS_CLOSED` — MET.
- `A001-R20C-C10 RFB_AUDIO_MPEG_AND_SINGLE_IO_OWNER_CONTRACTS_UNCHANGED` — MET.
- `A001-R20C-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP` — MET.
- `A001-R20C-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3`

Exact-head workflow:

- workflow: `Ledge reconstruction checks`
- run: `35987788623`
- attempt: `1`
- conclusion: `success`
- head SHA: `4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3`

Job results:

- `host-unit` — SUCCESS
  - `transport protocol tests passed`
  - `transport bridge tests passed`
  - `transport_runtime_test: PASS`
  - `transport_mpeg_test: PASS`
  - `RFB_FLOW_POLICY_TEST=PASS`
  - Pi R17 generation fixture: 12 tests, OK
  - `app R15/R16B/R19 tests: PASS`
  - `transport_rfb_provider_failure_test: PASS`
- `project-check` — SUCCESS
  - `WORK_LOG_CHECK=PASS`
  - `SOURCE_DICTIONARIES=PASS`
  - `PS_TO_VNC_PROJECT_CHECK=PASS`
- `dictionary-long` — SUCCESS
  - `SOURCE_DICTIONARY_CHECK_MODE=LONG`
  - `SOURCE_DICTIONARIES=PASS`
- `ps2-compile` — SUCCESS
- `ps2-link` — SUCCESS
  - `ISSUE7_LINKED_BUILD=PASS`
  - `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`
- `dictionary-reconcile` — SKIPPED as expected on this non-trigger exact head.

Final linked identity:

- `ELF_PRISTINE_SHA256=28965b2409074a034e491d316f1474c214d2fb2fe6167f2c366b7c25fa706f91`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
- `PT_LOAD_BYTES=491540`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

Accepted R20 had:

`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
`PT_LOAD_BYTES=491540`

R20C therefore changes loadable bytes while preserving byte count. The new
identity is reproducible repository evidence only and creates newer hardware
debt; it does not inherit physical qualification.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R20C_INDEPENDENT_VALIDATION=NOT_RUN`
- `R20C_OPERATOR_OBSERVED=NO`
- `R20C_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine/host evidence proves source lifecycle behavior and reproducibility only.

## State/contract accounting

Consumed:

- Foreman State revision `0053`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted R20 authority;
- accepted A001 sole-I/O-owner lifecycle;
- accepted neighboring RFB/AUDIO/MPEG Transport contracts.

Produced:

- no Foreman state revision;
- no Reconstruction-contract revision;
- no Wire/protocol version revision;
- no R21 source;
- no packet selection or Foreman acceptance.

## Findings / blockers

R20C has no remaining known source blocker within the authorized packet.

The Transport receiver now has two intentionally different facts without a new
public state machine:

- early `receiver_done`: terminal/admission closure;
- final receiver-done rendezvous: no-touch/reclaim completion.

The previously queued R21 remains unexecuted and requires independent Foreman
review/acceptance of R20C before it can become Reconstruction authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R20C_REVIEW_ACCEPTANCE_AND_R21_DEPENDENCY_DECISION`

The Reconstruction worker stops here. The Foreman must independently recover
current repository authority, review this correction/evidence/log, decide R20C
acceptance, clear or retain the architecture blocker, and decide whether the
queued R21 packet may become active. Reconstruction must not self-accept R20C or
begin R21 from this record.
