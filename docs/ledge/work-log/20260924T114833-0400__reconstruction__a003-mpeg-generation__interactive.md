DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T11:48:33-04:00
COMPLETED_AT=2026-09-24T12:08:56-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3032424be8296b5329a6876e947ff87653ea2e20
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R20D Transport outbound-submitter drain fence

## Objective and authority consumed

This interactive Reconstruction worker recovered current authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0054`, whose only
active Reconstruction packet was:

`A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D`

The assigning branch authority was:

`3032424be8296b5329a6876e947ff87653ea2e20`

R20D was activated after independent Foreman review found that R20C's final
receiver-completion event still did not prove every pre-terminal outbound
submitter had finished touching `outbound_work` and the outbound
slot/ready/done rendezvous.

R20D therefore had to extend the accepted R20C no-touch fence across the whole
synchronous submit transaction, including callers already queued behind the
single outbound slot, without weakening sole physical-I/O ownership or starting
queued R21 Application MPEG work.

Governing authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0006;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0054;
- the assigning Foreman immutable record
  `docs/ledge/work-log/20260924T075400-0400__foreman__a003-mpeg-generation__interactive.md`;
- accepted R20 private MPEG identity authority;
- the R20C candidate source/log preserved by State 0054;
- current `src/transport/runtime.*`, host Transport lifecycle fixture and
  directly affected lifecycle documentation.

State 0054 explicitly kept
`A003-APPLICATION-MPEG-RUN-START-R21` dependency-queued. R21 was not
authorized and was not executed.

The GitHub connector exposes repository/branch authority but not the external
developer Pi worktree. External staged/unstaged/untracked state and local
`scripts/resume-state.sh` execution remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim is made about
external checkout cleanliness.

## Result

R20D is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`c719a46003e04ac625cb92f4b455c94fe7281020`

The correction extends R20C's receiver completion meaning with one explicit
private outbound-submitter lifetime fence.

### Atomic pre-terminal admission

A submitter now registers before touching any outbound slot/ready/done
semaphore. Registration and receiver terminal admission closure share one tiny
EE interrupt-disabled critical section using the PS2SDK `DIntr()`/`EIntr()`
contract.

No blocking operation, semaphore wait, physical I/O or domain work occurs while
interrupts are disabled.

A caller therefore has exactly two possibilities:

1. it registers before terminality and becomes part of the drain set before it
   can touch the outbound rendezvous; or
2. it observes receiver terminality/failure/stop and returns without entering
   outbound semaphore ownership.

There is no check-then-register window in which a post-terminal caller can join
the drain set.

### Whole submit transaction is drain-owned

`outbound_submitter_count` covers the submitter from successful registration
through its final outbound-rendezvous touch.

For an active sender this includes:

- waiting for/acquiring the one outbound slot;
- publishing work and outbound-ready;
- waiting for outbound-done;
- consuming `outbound_work.result`;
- releasing the outbound slot;
- only then unregistering.

For a caller already queued behind the occupied slot, registration happens
before the slot wait. Such a caller therefore remains in the drain even though
it has not yet acquired the slot. Once the active caller releases the slot, the
queued caller acquires it, observes terminality, releases it without publishing
new work, then unregisters.

Signaling `outbound_done_semaphore_id` resolves the current item but is
deliberately not submitter-completion authority.

### Receiver terminal path

The sole physical-I/O owner now atomically publishes early
`receiver_done = 1` through the same admission boundary used by registration,
then preserves R20C's existing terminal ordering:

1. fail/resolve the currently pending outbound item;
2. wake the blocked RFB outbound-credit owner;
3. publish terminal RFB activity;
4. publish terminal AUDIO activity when enabled;
5. publish terminal MPEG activity when enabled;
6. drain every submitter admitted before terminality;
7. only after the drain is empty, publish the final R20C receiver-completion
   semaphore;
8. exit the I/O owner thread.

The private `outbound_submitter_drain_semaphore_id` is used only when the
receiver observes one or more registered submitters. The last registered
submitter signals it after its final outbound-semaphore touch.

If the last submitter leaves before the receiver arms the drain wait, the
receiver atomically observes a zero count and does not wait. If it leaves after
the wait is armed, that last departure signals the event. No timeout or
diagnostic counter is used as completion authority.

### Release/reclaim meaning

`pstvnc_transport_runtime_release()` remains a consumer of the final R20C
receiver-completion proof. It does not add a scheduling workaround.

Because the receiver cannot publish completion until the R20D drain is empty,
release cannot delete:

- outbound-done;
- outbound-ready;
- outbound-slot;
- outbound-submitter-drain;

or reclaim the receiver thread/stack/queues/physical stream while any
pre-terminal submitter can still touch those resources.

R20C's retryable pre-reclaim `ReferThreadStatus()` behavior remains intact.

### Fresh runtime reuse

Successful release clears the runtime object and identifier state as before.

The fresh-runtime lifecycle proof now explicitly verifies:

- receiver completion token starts empty;
- outbound submitter drain token starts empty;
- `outbound_submitter_count == 0`;
- `outbound_submitter_drain_waiting == 0`;
- receiver thread identity/start state is clean;
- no old stop/failure/quiesce authority is inherited.

## Deterministic active-and-queued race proof

R20D adds a synchronization-driven host fixture with no sleeps or correctness
timeouts.

`test_outbound_submitter_drain_fences_active_and_queued_callers()`
deterministically proves:

1. the sole I/O owner is held inside one readiness pass;
2. one direct Transport submitter registers, acquires the outbound slot,
   publishes one pending item and blocks on outbound-done;
3. a second direct submitter registers before terminality and is proven queued
   behind the occupied outbound slot;
4. the active caller is later held after the terminal outbound-done wake but
   before its final outbound-slot release;
5. receiver terminality is driven by an invalid inbound frame;
6. early `receiver_done` is visible, pending work is resolved, but final
   receiver completion remains unpublished;
7. `outbound_submitter_count == 2` and the queued slot waiter remains live
   while the active caller is held after its done wake;
8. a real concurrent `release()` caller blocks on final receiver completion
   and performs no `ReferThreadStatus`, thread deletion or physical release;
9. all outbound rendezvous semaphores remain allocated while drain is held;
10. releasing the active caller's final slot touch permits the queued caller to
    acquire the slot, observe terminality, release it and unregister;
11. only the second/final unregister may reduce the count to zero and release
    the receiver's drain wait;
12. receiver completion appears only after count zero and no queued slot waiter;
13. a caller beginning after early terminality returns without registering,
    waiting on the slot or changing the outbound-ready token;
14. post-fence ordering remains
    `receiver completion < TerminateThread < DeleteThread < physical release`.

The retained R20C fixture continues proving pending outbound resolution, blocked
RFB writer wake and AUDIO/MPEG activity waiter terminal publication before the
final receiver completion event.

## Superseded fixture deadlock and correction

The first R20D product commit
`0540eb8342262bd8004c713d205d7725481db827` started Actions run
`36023646380`.

The first active+queued test commit
`02aa3bd3c69baaee8d8626a373fa55bf4de0b1df` started run
`36023935743`.

Both superseded runs remained in-progress in the canonical host-unit step at
closeout. The reason was identified deterministically in the host fixture, not
in product synchronization:

the older R20C proof intentionally blocked the submitter before
`outbound_ready` publication and then waited for receiver completion. Under
R20D that caller is correctly registered in the drain set, so final completion
cannot occur until the test releases the caller. The old fixture therefore
created a circular test-only wait that contradicted the stronger R20D contract.

Commit `575117b8beed6373accb030b1dcfd8246b1393e8` corrected the retained
R20C proof by relying on its already-existing receiver-readiness barrier. The
submitter may publish ready and block normally on outbound-done while the I/O
owner remains inside that same readiness pass. Terminal inbound processing then
resolves and drains it before completion exactly as the product contract now
requires.

Commit `67a1719e1a005cf34a62968a4b7bf6e35a8807b9` removed the now-unused
host outbound-ready barrier helpers.

No product source changed in either fixture correction.

Run `36024670474` at `67a1719...` then proved the corrected shape:
host-unit, project-check, pinned PS2 compile and PS2 link/reproducibility were
all SUCCESS. Its only red gate was the expected strict dictionary drift before
reconciliation.

The superseded stalled runs are not product evidence and no product defect is
inferred from their host apparatus deadlock.

## Repeated exact-head synchronization evidence

Final reconciled source authority:

`c719a46003e04ac625cb92f4b455c94fe7281020`

GitHub Actions run:

`36025000101`

The exact same SHA was exercised with no source, delay, timeout or fixture
changes across three attempts.

Attempt 1:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/reproducibility — SUCCESS.

Attempt 2:

- host-unit — SUCCESS;
- all retained canonical non-host jobs remained SUCCESS on the same source
  authority.

Attempt 3:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/reproducibility — SUCCESS.

Attempt-3 host evidence includes:

- `transport protocol tests passed`;
- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi R17 MPEG generation fixture — 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`;
- `transport_rfb_provider_failure_test: PASS`.

This repetition is evidence of the synchronization contract, not a
retry-until-green acceptance rule: attempt 1 was already completely green
before attempts 2 and 3 were requested.

## Documentation and dictionaries

`docs/development/module-lifecycle.md` now records that final Transport
receiver completion covers both:

- R20C sole-I/O-owner terminal no-touch work; and
- R20D complete drain of every outbound submitter admitted before terminality.

`src/transport/SYMBOLS.md` ownership prose names the R20D drain fence.

The canonical deterministic dictionary reconciler at
`de9ef8e58f0c81f5139d2acc96cd2f743b7caa31` produced
`1922fd27e17125ce7ef2855032d885ee53e06ea4`, changing only:

- `src/transport/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The content-identical checkpoint
`c719a46003e04ac625cb92f4b455c94fe7281020` binds final machine evidence to
that reconciled source tree.

## Commits

The complete pre-log R20D range is eight commits ahead / zero behind the
assigning Foreman authority:

1. `0540eb8342262bd8004c713d205d7725481db827` —
   `transport: drain preterminal outbound submitters`
2. `02aa3bd3c69baaee8d8626a373fa55bf4de0b1df` —
   `test(transport): prove R20D outbound submitter drain`
3. `575117b8beed6373accb030b1dcfd8246b1393e8` —
   `test(transport): let R20C proof honor R20D drain`
4. `67a1719e1a005cf34a62968a4b7bf6e35a8807b9` —
   `test(transport): retire superseded R20C ready barrier`
5. `576bd4165012354b2244514d0ec3a0a41dfe112d` —
   `docs(transport): record R20D submitter drain fence`
6. `de9ef8e58f0c81f5139d2acc96cd2f743b7caa31` —
   `tooling(symbols): run deterministic dictionary reconciliation`
7. `1922fd27e17125ce7ef2855032d885ee53e06ea4` —
   `docs(symbols): reconcile current clean definitions`
8. `c719a46003e04ac625cb92f4b455c94fe7281020` —
   `test: verify final R20D outbound drain authority`

## Changed paths

The complete assigning-Foreman-to-pre-log diff is confined to the authorized
R20D lifecycle/test/documentation/dictionary surface:

- `src/transport/runtime.c`
- `src/transport/runtime.h`
- `tests/unit/transport_runtime_test.c`
- `tests/unit/transport_host_stubs/kernel.h`
- `docs/development/module-lifecycle.md`
- `src/transport/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

Notably unchanged:

- `src/app.c`;
- all R21 Application source;
- `src/app_mpeg_frame.*`;
- Pi product source;
- Transport protocol/frame codecs;
- Transport bridge R20 MPEG semantic API;
- RFB parser/session/flow-policy product source;
- AUDIO product policy;
- MPEG decoder/worker/backend;
- Display/Presentation;
- calibration/Input/UI;
- Configuration product source.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A001-R20D-C1 PRETERMINAL_SUBMITTER_LIFETIME_IS_EXPLICITLY_DRAINED` — MET.
- `A001-R20D-C2 TERMINALITY_PREVENTS_NEW_OUTBOUND_RENDEZVOUS_ADMISSION` — MET.
- `A001-R20D-C3 OUTBOUND_DONE_SIGNAL_IS_NOT_MISTAKEN_FOR_SUBMITTER_COMPLETION` — MET.
- `A001-R20D-C4 ACTIVE_SUBMITTER_EXITS_BEFORE_RECEIVER_COMPLETION` — MET.
- `A001-R20D-C5 QUEUED_OUTBOUND_SLOT_WAITERS_CANNOT_OUTLIVE_COMPLETION` — MET.
- `A001-R20D-C6 RELEASE_CANNOT_DELETE_OUTBOUND_SEMAPHORES_BEFORE_DRAIN` — MET.
- `A001-R20D-C7 R20C_RECEIVER_NO_TOUCH_AND_LOGICAL_TERMINAL_ORDERING_PRESERVED` — MET.
- `A001-R20D-C8 OUTBOUND_SERIALIZATION_BACKPRESSURE_AND_SOLE_SENDER_UNCHANGED` — MET.
- `A001-R20D-C9 FAILED_SHUTDOWN_RETRY_AND_FRESH_RUNTIME_REUSE_REMAIN_SAFE` — MET.
- `A001-R20D-C10 DETERMINISTIC_BARRIERS_PROVE_ACTIVE_AND_QUEUED_RACES_CLOSED` — MET.
- `A001-R20D-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP` — MET.
- `A001-R20D-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`c719a46003e04ac625cb92f4b455c94fe7281020`

Canonical workflow:

- workflow: `Ledge reconstruction checks`
- run: `36025000101`
- final observed attempt: `3`
- final conclusion: `success`
- head SHA: `c719a46003e04ac625cb92f4b455c94fe7281020`

Attempt-3 job results:

- `host-unit` — SUCCESS
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

- `ELF_PRISTINE_SHA256=58d00a302171a8d1e37f1bb911a443912f00d3a5a04f68a3737f31f5124106b1`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=df81e351eb67b77560a124279e7cb664843462f89581bb57aa7fc9993125d8d9`
- `PT_LOAD_BYTES=492052`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R20C candidate authority had:

`PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
`PT_LOAD_BYTES=491540`

R20D therefore changes loadable bytes and byte count. This exact identity is
reproducible repository evidence only and creates newer hardware debt; it does
not inherit physical qualification.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R20D_INDEPENDENT_VALIDATION=NOT_RUN`
- `R20D_OPERATOR_OBSERVED=NO`
- `R20D_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine/host evidence proves lifecycle behavior and reproducibility only.

## State/contract accounting

Consumed:

- Foreman State revision `0054`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted R20 authority;
- preserved R20C candidate correction;
- accepted A001 sole-I/O-owner and neighboring RFB/AUDIO/MPEG Transport
  contracts.

Produced:

- no Foreman state revision;
- no Reconstruction-contract revision;
- no Wire/protocol version revision;
- no R21 source;
- no packet selection or Foreman acceptance.

## Findings / blockers

R20D has no remaining known source blocker within the authorized packet.

The resulting final receiver-completion meaning is now intentionally cumulative:

- early receiver terminality closes new outbound admission;
- R20C completes all sole-I/O-owner terminal/wakeup work;
- R20D drains every outbound submitter admitted before that terminal edge;
- only then does the latched receiver-completion token authorize reclaim.

R21 remains unexecuted and dependency-queued. It requires independent Foreman
review/acceptance of this R20D correction before it can become Reconstruction
authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R20D_REVIEW_ACCEPTANCE_AND_R21_DEPENDENCY_DECISION`

The Reconstruction worker stops here. The Foreman must independently recover
current repository authority, inspect the R20D source/evidence/log, decide
acceptance, update Foreman-owned state as appropriate, and decide whether queued
R21 may become active. Reconstruction must not self-accept R20D or begin R21
from this record.
