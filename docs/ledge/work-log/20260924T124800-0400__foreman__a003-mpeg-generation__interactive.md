DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T12:48:00-04:00
COMPLETED_AT=2026-09-24T13:10:38-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=5357e7ed422bb402f187a2887ad0e67bcc710e4a
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — cumulative Transport reclaim fence accepted; R21 reactivated

## Objective and recovered authority

Receive the Reconstruction baton after
`A001-TRANSPORT-DRAIN-FAIL-CLOSED-R20E`, recover live repository authority
independently, review the exact source/test/log evidence, decide whether R20E
closes the cumulative R20C/R20D/R20E A001 reclaim correction, and if so decide
whether the previously queued R21 Application MPEG run-start packet may become
active again.

Live pickup authority was the immutable R20E Reconstruction record:

`5357e7ed422bb402f187a2887ad0e67bcc710e4a`

with final pre-log source:

`316ad217bef229c9ca0134b9b922a213cb5af247`

and assigning Foreman authority:

`fcdc8789bddca31c0ee2792177aaa4843a38e299`.

Exactly six pre-log R20E commits followed the assigning authority; the
immutable Reconstruction log was the seventh commit and no later commit existed
at Foreman pickup.

Current authority review included AGENTS/CONTRIBUTING/current status/project
intent/clean architecture, development naming/topology/module-lifecycle rules,
Reconstruction/work-log contracts, Foreman State 0055, Architecture Overlay,
Wire Runtime Decisions, Q1-Q12 reconciliation, A001/A003/A004/A005/A006 audit
authority, exact R20E runtime/tests/docs, accepted R20 Transport authority, the
intermediate R20C/R20D corrections, and the original State-0052 R21 packet.

No external Pi/local checkout was exposed to this Foreman seat. External
staged/unstaged/untracked state and local resume/check execution remain
unasserted.

## R20E independent acceptance

R20E stayed completely within its authorized Transport runtime/test/docs/
dictionary surface. It did not modify Application, R21 source, Pi product
source, protocol/frame codecs, Transport bridge R20 MPEG semantic API, RFB
product policy, AUDIO policy, MPEG worker/backend, Display/Presentation,
calibration/Input/UI or Configuration product source.

Independent Foreman criterion dispositions:

A001-R20E-C1 RECLAIM_SUCCESS_REQUIRES_EXPLICIT_SUCCESSFUL_DRAIN_PROOF — MET
A001-R20E-C2 DRAIN_WAIT_FAILURE_CANNOT_PUBLISH_SUCCESSFUL_COMPLETION — MET
A001-R20E-C3 NONZERO_COUNT_AFTER_DRAIN_WAKE_CANNOT_PUBLISH_SUCCESS — MET
A001-R20E-C4 WAIT_RECEIVER_DONE_NEVER_SUCCEEDS_FROM_UNPROVEN_DRAIN — MET
A001-R20E-C5 RELEASE_PRESERVES_ALL_OWNERSHIP_ON_UNPROVEN_DRAIN — MET
A001-R20E-C6 NORMAL_R20D_ACTIVE_AND_QUEUED_DRAIN_BEHAVIOR_UNCHANGED — MET
A001-R20E-C7 SUCCESSFUL_COMPLETION_REMAINS_LATCHED_AND_REOBSERVABLE — MET
A001-R20E-C8 FAILED_PATH_CANNOT_BECOME_FRESH_SESSION_AUTHORITY — MET
A001-R20E-C9 R20C_FAILED_SHUTDOWN_AND_POST_PROOF_RETRY_CONTRACTS_UNCHANGED — MET
A001-R20E-C10 NO_TIMEOUT_SLEEP_OR_DIAGNOSTIC_SUCCESS_SUBSTITUTE — MET
A001-R20E-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP — MET
A001-R20E-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

All twelve criteria are MET.

## Exact source findings

R20E introduces one Transport-private receiver-completion outcome:

- `PENDING` before terminal outcome publication;
- `PROVEN` only after R20C terminal-owner work and a successful R20D submitter
  drain prove the reclaim boundary;
- irreversible `FAILED` whenever drain proof is absent or completion
  rendezvous observation itself becomes contradictory.

The receiver-completion semaphore is therefore no longer synonymous with
success. It is a terminal-outcome rendezvous. A failed runtime may signal that
event so waiters can return, but only `PROVEN` can pass
`pstvnc_transport_runtime_pass_receiver_completion_fence()`.

Drain `WaitSema()` failure now clears the private drain-waiting fact and returns
failure. A drain wake followed by protected nonzero submitter count also
returns failure. The receiver maps either result to irreversible `FAILED`
rather than reclaim proof.

`pstvnc_transport_runtime_wait_receiver_done()` observes an already-latched
`FAILED` as immediate failure. Otherwise it consumes/restores the binary
terminal-outcome event and succeeds only when the private outcome is exactly
`PROVEN`. An impossible non-PROVEN event or rendezvous error converts the
runtime to irreversible `FAILED`.

`pstvnc_transport_runtime_release()` still passes that fence before inspecting
waiters, kernel thread status, queues, semaphores, stack, or physical stream.
Thus a failed/unproven drain leaves all Transport ownership unreclaimed.

Later submitter count zero cannot upgrade `FAILED`; repeated wait/release
observations remain failure. Fresh runtime reuse occurs only after genuinely
successful release and returns completion outcome to `PENDING` through fresh
runtime initialization.

R20D's normal registration/count/drain design remains unchanged: active and
queued pre-terminal submitters are counted through their final outbound
rendezvous touch, and post-terminal callers never enter outbound ownership.

## Deterministic failure-path proof

The focused host fixture now proves both failure cases required by State 0055.

`test_outbound_drain_wait_failure_is_permanently_nonreclaiming()` injects one
failure into the exact private drain `WaitSema()` while a registered active
submitter remains live. It proves `FAILED`, repeated completion-wait failure,
repeated release failure, no kernel thread inspection/termination/deletion, no
physical release, no queue/stack reclaim, and no outbound/drain semaphore
deletion. It then allows the submitter to leave and proves later count zero
cannot promote the failed runtime.

`test_outbound_drain_false_wake_is_permanently_nonreclaiming()` injects an
impossible drain token while the submitter count remains nonzero. The receiver
consumes that token, rechecks protected count, latches `FAILED`, preserves all
resources, and remains permanently non-reclaiming even after the submitter later
leaves truthfully.

The retained R20C/R20D normal active+queued race proof and failed-shutdown/
post-proof kernel-status retry proofs remain green.

## Cumulative A001 reclaim-fence acceptance

R20C and R20D remain historically correct as intermediate candidate records:
each was not accepted at its own head because later Foreman review found one
additional ownership gap.

At exact R20E source authority, however, the cumulative mechanism is complete:

1. early `receiver_done` atomically closes new outbound admission;
2. R20C preserves the sole physical-I/O owner through pending outbound
   resolution and all logical RFB/AUDIO/MPEG terminal publication;
3. R20D drains every outbound submitter admitted before terminality through
   its final outbound semaphore touch;
4. R20E requires explicit successful drain proof before reclaim and makes every
   unproven drain irreversibly non-reclaiming.

Foreman therefore accepts the cumulative R20C/R20D/R20E Transport reclaim fence
only at R20E source authority:

`316ad217bef229c9ca0134b9b922a213cb5af247`

## Exact R20E machine/build evidence

Final source workflow run `36030820124` completed SUCCESS on attempt 1 and was
repeated on identical source; final observed attempt 2 also completed SUCCESS.

Immutable-log-head workflow run `36031179032` attempt 1 completed SUCCESS.

Both exact final authorities passed:

- host-unit;
- project-check;
- complete strict source dictionaries;
- pinned PS2 compile;
- pinned PS2 link/current-source reproducibility.

Immutable-log-head host evidence includes:

- `transport protocol tests passed`;
- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi R17 MPEG generation fixture — 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`;
- `transport_rfb_provider_failure_test: PASS`.

Accepted cumulative correction linked identity:

`ELF_PRISTINE_SHA256=c838f0afda456c8026f8ea30afed1aad68eb7b0480142d0a3d956ed0e636274c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c280849d1310b0a530daa77da736f22b0eae175a0adbdc43a0cb856c3c560adc`
`PT_LOAD_BYTES=492308`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R20E
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

R20E changes loadable bytes relative to accepted R20 and therefore becomes the
current exact hardware-debt identity. Repository reproducibility does not
physically qualify it.

## R21 dependency decision

State 0052 originally selected
`A003-APPLICATION-MPEG-RUN-START-R21` as the smallest next dependency after R20.
Exact State-0052 validation exposed the A001 reclaim race before Reconstruction
was allowed to execute R21, so States 0053-0055 correctly kept R21 queued while
R20C/R20D/R20E repaired Transport.

No intervening correction changed the R21 Application semantics or component
contracts. The cumulative Transport blocker is now independently accepted.

Foreman therefore reactivates the original trigger-agnostic R21 packet without
scope expansion.

R21 owns only the Application run-start support transaction: session-local
nonzero monotonic generation allocation, exact accepted geometry mapping,
preexisting P2 protection requirement, R18 run-open, fresh R5/R3/R4 execution
owners, P3 WAIT_FIRST_FRAME, P7 frame consumer, and R20 START as the final
irreversible startup action.

R21 still does not modify ordinary `app.c` to invoke that transaction, choose a
controller/keyboard/UI calibration/start gesture, activate the Pi MPEG product
runtime, service frames, thaw/reveal RFB, perform retirement/finalization, or
activate AUDIO.

## Foreman-owned publication

Published Foreman State revision 0056 at:

`ff0bdf1adcf28aa810a03ff1bfb9e4177d86d7dd`

with message:

`docs(foreman): accept Transport reclaim fence and reactivate R21`

State 0056:

- accepts the cumulative R20C/R20D/R20E reclaim correction at R20E authority;
- clears `ARCHITECTURE_BLOCKER`;
- marks the receiver-completion, outbound-submitter-drain and drain-failure
  fence as Foreman accepted;
- reactivates `A003-APPLICATION-MPEG-RUN-START-R21` using the original State-
  0052 objective, required behavior, criteria, deterministic evidence and
  authorized source surface;
- adds only the new accepted R20E Transport dependency to the packet metadata;
- keeps Application retirement and ordinary MPEG product activation deferred.

## State-0056 exact-head evidence

Exact State-0056 workflow run `36032362015` attempt 1 completed SUCCESS across:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

The documentation-only State publication retained exact accepted R20E identity:

`ELF_PRISTINE_SHA256=c838f0afda456c8026f8ea30afed1aad68eb7b0480142d0a3d956ed0e636274c`
`PT_LOAD_SHA256=c280849d1310b0a530daa77da736f22b0eae175a0adbdc43a0cb856c3c560adc`
`PT_LOAD_BYTES=492308`

## Next pickup

Reconstruction must recover current branch authority and execute only:

`A003-APPLICATION-MPEG-RUN-START-R21`

from Foreman State revision 0056.

Preserve its trigger-agnostic scope exactly. Implement the Application-owned
run-start support transaction and deterministic failure-unwind evidence, but do
not wire it into ordinary `app.c`, select a product trigger, perform downstream
MPEG retirement, activate Pi MPEG production, or broaden component internals.

If direct R21 composition exposes a concrete integration-contract defect in an
existing component, stop and return BLOCKED rather than expanding packet scope.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
