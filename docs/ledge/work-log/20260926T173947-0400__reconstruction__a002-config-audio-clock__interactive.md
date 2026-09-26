# Ledge Reconstruction work log — R38 AUDIO session completion publication

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T17:39:47-04:00
COMPLETED_AT=2026-09-26T17:54:20-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e735b3298e1ee8a7b00671eaf2f62d6c32ce52a6
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Interactive Reconstruction Worker round received the baton at exact live
authority `e735b3298e1ee8a7b00671eaf2f62d6c32ce52a6` under Foreman State
revision 0080.

The only active packet was:

`A002-AUDIO-SESSION-COMPLETION-PUBLICATION-R38`.

R38 authorized one narrow AUDIO-owner observability seam: publish whether the
exact successfully started AUDIO worker has completed its session entry work
without blocking the ordinary coordinator. It explicitly did not authorize
Application AUDIO composition, Transport AUDIO activation, AUDIO start policy,
R36 profile consumption by Application, media-clock policy, Pi AUDIO work,
automatic MPEG recalibration, or hardware qualification.

The round consumed accepted R37 execution-binding authority, accepted R36 AUDIO
profile authority, R26 common-media-clock authority, the A002 semantic audit,
A006 orchestration/shutdown audit, current module-lifecycle policy, current
clean topology/dictionary policy, and the assigning Foreman state/log.

## Final pre-log source authority

Final source/test/documentation/dictionary authority before this immutable log:

`9834277d61b1f8dd6ed8c47923cb328e414ba08d`
— `test(audio): enforce R38 completion boundary`.

It is exactly eight commits ahead and zero behind the assigning Foreman head:

1. `55ff08604311df478507e1509566dddd41fccf7c`
   — `audio: add nonblocking R38 completion observation`
2. `177522956e4871ad781af19a134724433a7f506a`
   — `test(audio): prove R38 completion observation`
3. `b8e7e6c8fd24dd234929d6f558e1613bb1298e79`
   — `docs(audio): record R38 owner observation`
4. `e37c3bd70e435a0f9431dfd11820db84334c1adc`
   — `test(audio): fix R38 runtime fixture`
5. `033a4c70afd58306b8eb14e8e76a7805f58eceb0`
   — deterministic dictionary-reconciliation trigger
6. `f8650ae491227d70ac4588baa372a065e48cfcfe`
   — automation-generated dictionary reconciliation
7. `8442a759f7ec2980a8e490eff5df5b9d6e9a5d54`
   — `test(audio): add R38 completion source boundary`
8. `9834277d61b1f8dd6ed8c47923cb328e414ba08d`
   — `test(audio): enforce R38 completion boundary`

The exact changed surface is confined to:

- `src/audio/session.c`
- `src/audio/session.h`
- `src/audio/ps2_runtime.c`
- `src/audio/SYMBOLS.md`
- `tests/unit/audio_session_test.c`
- `tests/unit/audio_ps2_runtime_test.c`
- `tests/unit/audio_ps2_runtime_host_stubs/kernel.h`
- `tests/unit/audio_ps2_runtime_source_test.py`
- `tests/unit/audio_session_completion_source_test.py`
- `tests/Makefile`
- `docs/development/module-lifecycle.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

No Application, Transport implementation, Configuration profile, media-clock,
playback/AUDSRV service, MPEG, RFB, Input/UI/Display, Pi, Wire/protocol, or H1
forensic source changed.

## R38 owner contract

The accepted AUDIO thread-operation contract now has one additional injected
operation:

`poll_completion(context, thread_id, completed)`.

The public AUDIO session owner exposes:

`pstvnc_audio_session_poll(session, state)`.

Its public state is deliberately only:

- `PSTVNC_AUDIO_SESSION_COMPLETION_PENDING`
- `PSTVNC_AUDIO_SESSION_COMPLETION_DONE`.

The operation is valid only for an initialized, created, successfully started,
not-destroyed exact worker with a valid thread ID. A clean unstarted session or
a retained create-success/start-failure prefix is rejected as INVALID rather
than being misreported as done.

If the thread owner cannot truthfully determine completion, the session returns
`PSTVNC_AUDIO_SESSION_THREAD_STATUS_FAILED`. It does not collapse a kernel
observation failure into PENDING.

The session poll never reads Transport AUDIO producer state, queue occupancy,
media-clock state, elapsed time, worker outcome, diagnostics, or Application
flags. It delegates only to the exact injected thread owner. Already-successful
join is a retained DONE fact.

## R38 PS2 completion mechanism

The R37 PS2 adapter now supplies the narrow nonblocking completion operation.

For an exact active started thread:

1. if `completion_observed` is already retained, return DONE immediately;
2. otherwise call `ReferSemaStatus` on the existing worker-completion
   semaphore;
3. a zero completion count returns PENDING immediately;
4. a positive count is consumed with nonblocking `PollSema`;
5. only successful consumption sets `completion_observed`;
6. later polls reuse that retained fact and do not consume the event again.

The poll path contains no `WaitSema`, `DelayThread`,
`ReferThreadStatus`, timeout, bounded retry, or elapsed-time success rule.

The existing join remains unchanged as the visibility/dormancy fence. If a
prior poll consumed completion, join skips the one-shot blocking completion
wait but still proves the exact EE worker reaches `THS_DORMANT` before
success. Outcome access still requires successful join. Resource release and
R37 retryable partial-start/destruction semantics remain later and unchanged.

## Deterministic focused evidence

The AUDIO session fixture proves:

- a started worker with no published completion returns PENDING immediately;
- repeated PENDING polls do not invoke join or time delay and do not become DONE
  by count;
- the public poll does not inspect Transport AUDIO status/activity while the
  worker remains pending;
- actual worker return becomes DONE before join;
- DONE does not make `pstvnc_audio_session_outcome()` legal before join;
- join after prior completion observation remains legal;
- a poll-owner failure returns THREAD_STATUS_FAILED rather than PENDING/DONE;
- clean unstarted and retained start-failure ownership are rejected;
- finite EMPTY completion becomes DONE;
- normal playback completion becomes DONE;
- representative playback failure becomes DONE;
- stop-request completion becomes DONE;
- Transport failure completion becomes DONE;
- clock failure completion becomes DONE.

The PS2 runtime fixture proves:

- repeated nonblocking polls before completion return unfinished with zero
  `WaitSema`, zero `PollSema`, and zero delay calls;
- semaphore-status failure before completion returns failure and retains no
  false completion;
- failed one-shot event consumption retains no false completion;
- successful event consumption records `completion_observed`;
- later completion polling returns done from the retained fact without touching
  the one-shot event again;
- a later kernel/status failure cannot erase retained completion;
- join after prior polling does not wait on the consumed completion semaphore;
- join still requires actual `THS_DORMANT`;
- existing R37 create/start/destroy/allocation/semaphore-release retry behavior
  remains covered.

The final source-boundary regression additionally proves that the session poll
contains no Transport AUDIO status/activity, media-clock, delay, worker-outcome
or worker-finished inference; the PS2 poll contains `ReferSemaStatus`,
`PollSema` and retained completion authority but no blocking wait, delay, or
thread-dormancy query; and no ordinary Application source calls the R38
session-poll API.

## Canonical machine evidence

Exact final-source GitHub Actions run:

`36274451271`

checked out exact head
`9834277d61b1f8dd6ed8c47923cb328e414ba08d`, attempt 1, and completed
SUCCESS.

Canonical job disposition:

- host-unit = SUCCESS
- project-check = SUCCESS
- dictionary-long = SUCCESS
- ps2-compile = SUCCESS
- ps2-link = SUCCESS
- dictionary-reconcile = correctly SKIPPED

Focused and neighboring regression output includes:

- `audio_session_test: PASS`
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`
- `audio_ps2_runtime_test: PASS`
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`
- `audio_playback_test: PASS`
- `audio_audsrv_service_test: PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`
- `media_clock_test: PASS`
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`
- `SOURCE_TOPOLOGY_CONTRACT=PASS`
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`
- `SOURCE_DICTIONARIES=PASS`
- `PS_TO_VNC_PROJECT_CHECK=PASS`
- `CLEAN_PS2_COMPILE_CHECK=PASS`
- `ISSUE7_LINKED_BUILD=PASS`
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

An earlier packet-local host run exposed one unused fixture declaration and the
expected pre-reconciliation strict-dictionary drift. The declaration was fixed
without product-source behavior change, deterministic dictionary reconciliation
changed only the AUDIO dictionary/portal, and the exact final-source run above
closes every canonical gate green.

## Exact linked identity and qualification boundary

Final R38 linked identity:

`ELF_PRISTINE_SHA256=3b8319a17aa57e50a75399b1eb4ac0c35e59168f2805a1d7d531bd68d92450c6`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=a5a048b8e96650bcce751c3899fb1491d7a41d5b7c2615e60e3cf4779f726313`

`PT_LOAD_BYTES=550804`.

Newest Foreman-accepted R37 identity was:

`ELF_PRISTINE_SHA256=f7270f3f738ab497dbe9e5163df24984056b6d5fdb0ca5286459d59fc32687a5`

`PT_LOAD_SHA256=776dd4923ca3302585bd31e630f7e9914a45cd2c4b5980c4766ad0dfdbf4b2b9`

`PT_LOAD_BYTES=550164`.

R38 therefore changes loadable bytes and creates new physical qualification
debt. No physical qualification transfers from R37 or earlier images.

Evidence classification:

- R38_SOURCE_COMPLETE=YES_WITHIN_PACKET
- R38_HOST_TESTED=PASS
- R38_PROJECT_CHECK=PASS
- R38_STRICT_DICTIONARIES=PASS
- R38_PS2_COMPILE=PASS
- R38_PS2_LINK=PASS
- R38_CURRENT_SOURCE_REPRODUCIBILITY=PASS
- R38_MACHINE_EVIDENCE=GITHUB_ACTIONS
- R38_PS2_PT_LOAD_CHANGED=YES
- R38_INDEPENDENT_VALIDATION=NOT_RUN
- R38_OPERATOR_OBSERVED=NO
- R38_HARDWARE_QUALIFIED=NO
- R38_HARDWARE_PENDING=YES

## Reconstruction Worker requirement disposition

These are Reconstruction Worker findings only; they are not Foreman
acceptance.

1. Public owner status is nonblocking — MET.
2. Only an exact successfully started worker is status-observable — MET.
3. Completion remains an AUDIO worker fact — MET.
4. R37 PS2 binding now supplies exact nonblocking completion observation — MET.
5. One-shot completion observation is retained across retries — MET.
6. Repeated active observation cannot manufacture completion — MET.
7. Kernel/status failure is a real owner error — MET.
8. Join remains the outcome-visibility and dormancy fence — MET.
9. Finite success/failure/stop/Transport/clock worker returns publish done —
   MET.
10. R37 partial-start/release semantics remain unchanged — MET.
11. No ordinary Application AUDIO policy/composition was added — MET.
12. Focused/regression/canonical machine evidence closes green — MET.

## Scope, limitations and next pickup

R38 does not open the Transport AUDIO rider, select AUDIO start policy, start
ordinary product AUDIO, consume the R36 profile from Application, aggregate
AUDIO failure into Application, change media-clock arm policy, change AUDSRV
playback ordering, modify Pi AUDIO behavior, add automatic MPEG recalibration,
or claim Validation/operator observation/hardware qualification.

There is no Reconstruction blocker at this stopping point. Hardware
qualification remains pending because PT_LOAD changed.

The Interactive Reconstruction Worker stops here. The Foreman must independently
inspect the eight-commit returned range, exact nonblocking completion semantics,
canonical evidence, changed linked identity, and this immutable closeout before
accepting or rejecting R38 and selecting any successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R38_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
