DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T07:54:00-04:00
COMPLETED_AT=2026-09-24T08:02:14-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3fac8afb2d403f2b96171dfadc3386c8c57da347
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman review — R20C not accepted; R20D outbound submitter drain activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C`, recover live repository
authority independently, inspect the exact returned source/test range and
machine evidence, decide all packet criteria, then either clear the Transport
blocker and reactivate queued R21 or publish the smallest corrective packet.

Live pickup authority was the immutable R20C Reconstruction record:

`3fac8afb2d403f2b96171dfadc3386c8c57da347`

with final pre-log source:

`4194ed70ef8f5758d3c958e1d9bf182a8e2dd4b3`

and assigning Foreman authority:

`003306d399c22f2a02625a867a512c225931a87a`.

Exactly eight pre-log Reconstruction commits followed the assigning authority;
the immutable log was the ninth commit and no later commit existed at pickup.

Current authority read included AGENTS/CONTRIBUTING/current status/project
intent/clean architecture, development naming/topology/module-lifecycle rules,
Reconstruction and work-log contracts, Foreman State 0053, Architecture
Overlay, Wire Runtime Decisions, Q1-Q12 reconciliation, A001/A003/A004/A005/
A006 audits, R20C source/tests, the prior Foreman correction log, and queued
R21 authority from State 0052.

No external Pi/local checkout was exposed to this Foreman seat. No staged,
unstaged, untracked, `scripts/resume-state.sh`, or local `scripts/check.sh`
claim is made.

## What R20C correctly repaired

Foreman confirms the returned source closes the originally discovered
receiver-thread reclaim race:

- early `runtime->receiver_done` still closes admission before terminal
  publication completes;
- receiver terminal processing then resolves the currently pending outbound
  item, wakes RFB outbound-credit ownership, publishes terminal RFB activity,
  and publishes enabled AUDIO/MPEG terminal activity;
- only after those I/O-owner operations does it signal
  `receiver_done_semaphore_id`;
- `pstvnc_transport_runtime_wait_receiver_done()` now synchronizes through that
  semaphore and restores its binary token for later observers;
- `pstvnc_transport_runtime_release()` cannot inspect/force/delete the receiver
  thread or reclaim queues/semaphores/stack/stream before passing that final
  receiver fence;
- failed socket shutdown convergence, retryable pre-reclaim kernel status
  failure, and fresh runtime reset remain covered.

This is a material and correct improvement over the State-0052 source.

## Foreman-discovered remaining ownership race

R20C does not make receiver completion a complete Transport-resource no-touch
fence because an outbound **submitting caller** can still touch Transport-owned
rendezvous state after the receiver publishes final completion.

`pstvnc_transport_runtime_submit_frame()` holds or depends on the outbound
rendezvous from its `WaitSema(outbound_slot_semaphore_id)` through pending-item
publication, outbound-done wait, result observation, and final
`SignalSema(outbound_slot_semaphore_id)`.

Terminal receiver processing calls
`pstvnc_transport_runtime_fail_pending_outbound()`, which writes the failed
result, clears `outbound_pending`, and signals `outbound_done_semaphore_id`.
That wakes the submitting caller but does not prove it has finished reading the
result and releasing the outbound slot.

The receiver then continues directly to logical terminal publication and may
signal `receiver_done_semaphore_id` before the submitter runs again.

Therefore this product interleaving remains valid:

1. submitter A owns the outbound slot and waits on outbound-done;
2. receiver publishes early terminality and signals A's outbound-done;
3. receiver completes logical terminal work and signals final receiver
   completion before A resumes;
4. release passes that completion fence and deletes outbound-done/ready/slot
   semaphores;
5. A resumes and attempts its remaining outbound-work/slot-semaphore return
   path against reclaimed resources.

A second pre-terminal caller B can already be blocked on the outbound slot after
passing the initial admission check. R20C likewise carries no proof that B has
left that semaphore wait before completion/reclaim.

## Why the returned test does not close that race

`test_receiver_completion_event_is_real_no_touch_fence()` deliberately blocks
the receiver at the host completion-signal barrier, then explicitly performs:

`pthread_join(outbound_submit_thread, NULL)`

before it releases the receiver completion barrier.

That test choreography guarantees the submitter has finished its final
outbound-slot touch before completion is allowed to publish. Product source has
no corresponding submitter-drain synchronization. The test therefore proves a
stronger schedule than the implementation guarantees and cannot serve as
acceptance evidence for the missing ownership fence.

Green repeated host runs do not change that source fact.

## R20C criterion disposition

A001-R20C-C1 EARLY_TERMINAL_STATE_IS_NOT_RECLAIM_COMPLETION_AUTHORITY — MET
A001-R20C-C2 WAIT_RECEIVER_DONE_SYNCHRONIZES_AFTER_ALL_TERMINAL_OWNER_WORK — MET_FOR_RECEIVER_OWNER_ONLY
A001-R20C-C3 RELEASE_CANNOT_TERMINATE_OR_DELETE_BEFORE_COMPLETION_FENCE — MET_FOR_RECEIVER_OWNER_ONLY
A001-R20C-C4 PENDING_OUTBOUND_AND_LOGICAL_TERMINAL_WAKES_PRECEDE_RECLAIM — NOT_MET_AS_COMPLETE_TRANSACTION_DRAIN
A001-R20C-C5 FAILED_SOCKET_SHUTDOWN_STILL_CONVERGES_WITHOUT_TIMEOUT_SUCCESS — MET
A001-R20C-C6 PRE_RECLAIM_KERNEL_STATUS_FAILURE_PRESERVES_RETRYABLE_OWNERSHIP — MET
A001-R20C-C7 STACK_SEMAPHORES_QUEUES_AND_STREAM_RECLAIM_ONLY_AFTER_NO_TOUCH_PROOF — NOT_MET
A001-R20C-C8 FRESH_RUNTIME_REUSE_CANNOT_INHERIT_COMPLETION_AUTHORITY — NOT_PROVEN_WHILE_SUBMITTER_CAN_OUTLIVE_RELEASE
A001-R20C-C9 DETERMINISTIC_BARRIER_PROVES_PRE_COMPLETION_RACE_IS_CLOSED — NOT_MET_FOR_OUTBOUND_SUBMITTER
A001-R20C-C10 RFB_AUDIO_MPEG_AND_SINGLE_IO_OWNER_CONTRACTS_UNCHANGED — MET
A001-R20C-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP — MET
A001-R20C-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

All twelve criteria were required. R20C is therefore not Foreman-accepted.

## R20C machine evidence retained without semantic promotion

Final pre-log source run `35987788623` attempt 1 completed SUCCESS.
Immutable-log-head run `35988159385` attempt 1 completed SUCCESS.

Those heads passed canonical host, project check, complete strict dictionaries,
pinned PS2 compile, PS2 link and current-source reproducibility.

R20C candidate linked identity:

`ELF_PRISTINE_SHA256=28965b2409074a034e491d316f1474c214d2fb2fe6167f2c366b7c25fa706f91`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
`PT_LOAD_BYTES=491540`

Evidence classification:

R20C_SOURCE_COMPLETE_WORKER_CANDIDATE=YES
R20C_FOREMAN_ACCEPTED=NO
HOST_TESTED=PASS_BUT_INSUFFICIENT_FOR_UNCOVERED_INTERLEAVING
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO

The last Foreman-accepted loadable product remains R20. The R20C identity is
current branch candidate hardware debt only.

## Corrective dependency decision

R21 still requires trustworthy Transport unwind on every pre-START failure and
cannot be reactivated while Transport may delete an outbound semaphore beneath
a pre-terminal caller.

Foreman preserves the valid R20C receiver correction and activates a narrower
continuation rather than reverting it:

`A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D`

R20D must extend final receiver completion so it becomes reclaim-authoritative
only after both the sole I/O owner and every outbound submitter admitted before
terminality are unable to touch Transport-owned outbound state/semaphores.

The proof must cover both an active pending submitter and another caller already
queued on the outbound slot. A host-only join, sleep, scheduling-priority
assumption, race-luck rerun, or generic timeout is not product synchronization.

## Foreman-owned publication

Published Foreman State revision 0054 at:

`6e883c2568093563b3901423ec5f06b8309dfa34`

with message:

`docs(foreman): require outbound submitter drain before R21`

State 0054:

- does not accept R20C;
- narrows `ARCHITECTURE_BLOCKER` to
  `TRANSPORT_OUTBOUND_SUBMITTER_DRAIN_FENCE`;
- leaves `A003-APPLICATION-MPEG-RUN-START-R21` queued;
- activates only `A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D`;
- authorizes only the minimum Transport runtime/test/docs/dictionary surface;
- requires deterministic active-and-queued submitter barriers;
- keeps Application, Pi, MPEG worker/backend, Presentation/calibration, RFB
  product semantics, AUDIO product policy, Configuration and protocol bytes
  out of scope.

## State-0054 machine evidence

Exact State-0054 Actions run `35996428577` attempt 1 completed SUCCESS across:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

The documentation-only State publication retained the exact R20C candidate
identity:

`ELF_PRISTINE_SHA256=28965b2409074a034e491d316f1474c214d2fb2fe6167f2c366b7c25fa706f91`
`PT_LOAD_SHA256=840eed441700a074719db0ac2353d5b0c381ce38b05cafba75d224c897c85ecd`
`PT_LOAD_BYTES=491540`

That green State run does not override the source-level review finding.

## Next pickup

Reconstruction must recover current branch authority and execute only:

`A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D`

from Foreman State revision 0054.

Preserve the useful R20C receiver completion fence. Add a real product drain for
all pre-terminal outbound submitters so final completion cannot publish while
an active or queued submitter can still touch outbound rendezvous resources.

Do not execute queued R21, do not add Application MPEG run-start source, and do
not broaden into Pi, Presentation, calibration, MPEG worker/backend, AUDIO
policy, RFB semantics, Configuration or protocol changes.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
