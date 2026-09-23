DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T21:13:00-04:00
COMPLETED_AT=2026-09-22T21:25:04-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=856a1065f5941c5149518ffe2a322bc81fe91642
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R16B provider recovery; R17 Pi MPEG control/producer activated

## Objective and recovered authority

Receive the Reconstruction baton after `A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`,
independently recover the actual live branch and immutable worker record, review
the complete worker range against current ownership/lifecycle authority and
packet criteria, then publish the smallest next dependency if R16B is accepted.

Live pickup authority was:

`856a1065f5941c5149518ffe2a322bc81fe91642`

with immutable worker record:

`docs/ledge/work-log/20260922T204600-0400__reconstruction__a003-mpeg-generation__interactive.md`

The worker's final pre-log source authority was:

`3310568e6c35ef59d77ee6075f1f9ea5a561476e`

and exactly eight Reconstruction commits followed the assigning Foreman log
`353f51c248235a9d6c1c1e5d7078de21dc6d1a74` before the immutable closeout.

Authority consumed included current Foreman State 0047, Reconstruction Contract
revision 0006, work-log contract revision 0007, Wire Runtime Decisions revision
0011, Architecture Overlay revision 0007, Q1-Q12 reconciliation, A001/A003/A005/
A006 semantic audits, clean architecture/project intent, module lifecycle,
source topology/naming guidance, current Application/Input/RFB/Transport/Pi
source, exact changed-file history and canonical Actions evidence.

No Pi/local checkout was exposed to this Foreman seat. Staged, unstaged and
untracked external-worktree state therefore remains unasserted, and no local
`scripts/resume-state.sh` or `scripts/check.sh` result is claimed.

## Exact R16B range reviewed

The eight pre-log commits were:

- `855e76c596eb7da0f6a64ff146a9c86c78550039` — Application typed provider recovery policy;
- `8a24e4952f0124be175277506a5af631c35ce81f` — typed-attempt test scripting;
- `165e460ad3a854ec4239dd23689df8a6f6c78246` — deterministic fresh-attempt recovery tests;
- `bfda7f0f4bc0e0fe962ebe849019ed03f85053bb` — fresh Pi attachment-per-session test;
- `0fb93ece595100305cd4897a882fcea58f8e709c` — R16B lifecycle documentation;
- `bd13d7fa065b8f0e3fc7d506b8bbc4b2ab606df5` — Application symbol dictionary update;
- `ca404665a0b1200de20f7dff3ed74aa3f4c41d3b` — explicit R13 REQUEST-state test observation;
- `3310568e6c35ef59d77ee6075f1f9ea5a561476e` — explicit R13 COMMIT-state test observation.

Materially changed paths were confined to:

- `src/app.c`;
- `src/SYMBOLS.md`;
- `tests/unit/app_test.c`;
- `tests/unit/app_test_legacy.inc`;
- `tests/unit/pi_rfb_attachment_test.py`;
- `tests/unit/pi_rfb_attachment_test_legacy.py`;
- `docs/development/rfb-provider-failure.md`.

No Transport/RFB lower-layer product source, protocol bytes, R13/R14 product
semantics, AUDIO/MPEG/CONFIG product source, heartbeat or final all-guns
orchestration entered the packet.

## Independent R16B acceptance

All twelve packet criteria are Foreman-accepted.

- Typed CONNECT, READ/EOF and WRITE provider-terminal causes reach an explicit
  Application recovery-policy branch.
- After Application observes one of those causes, the old attempt performs no
  further ordinary provider-bound input/RFB admission.
- Input shutdown is fail-closed and proves controller-worker dormancy before
  reusable input ownership is released.
- Transport abort must prove the sole receiver/session retired before the next
  connection is admitted.
- Failure of either stop proof blocks replacement and converges fatal rather
  than forcing ownership reuse.
- The next attempt obtains a fresh descriptor and repeats ordinary Q4/Transport
  establishment; old Transport authority cannot become the new session.
- RFB parser/session, framebuffer validity, input runtime and published pointer
  authority are initialized afresh on every admitted attempt.
- Pi evidence proves replacement Wire Sessions allocate distinct R13 attachment
  objects and distinct session identities; no failed attachment is rebound.
- Generic physical/RFB I/O failure remains distinct from typed provider failure
  and does not enter this bounded provider-recovery loop.
- No recovery delay, backoff, timeout-as-success or in-place component restart
  was introduced.
- R16A provider-terminal representation, Q4 compatibility, R13 finite quiesce
  and R14 values remain unchanged.
- No media/final-orchestration scope creep occurred.

Acceptance results:

A003-R16B-C1 TYPED_PROVIDER_FAILURE_REACHES_APPLICATION_POLICY — MET
A003-R16B-C2 FAILED_ATTEMPT_ADMISSION_CLOSES_BEFORE_TEARDOWN — MET
A003-R16B-C3 COMPLETE_OWNER_STOP_PROVEN_BEFORE_REPLACEMENT — MET
A003-R16B-C4 DEAD_SESSION_AND_ATTACHMENT_NEVER_REBOUND — MET
A003-R16B-C5 REPLACEMENT_USES_FRESH_NETWORK_Q4_TRANSPORT_AUTHORITY — MET
A003-R16B-C6 FRESH_RFB_FRAMEBUFFER_AND_INPUT_AUTHORITY — MET
A003-R16B-C7 STALE_OLD_ATTEMPT_STATE_CONTAINED — MET
A003-R16B-C8 PROVIDER_FAILURE_REMAINS_DISTINCT_FROM_WIRE_FAILURE — MET
A003-R16B-C9 NO_DELAY_TIMEOUT_OR_IMPLICIT_RESTART_POLICY — MET
A003-R16B-C10 R16A_R13_R14_LOWER_LAYER_CONTRACTS_UNCHANGED — MET
A003-R16B-C11 NO_MEDIA_HEARTBEAT_RETUNE_OR_FINAL_SCOPE_CREEP — MET
A003-R16B-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

## Machine/build evidence

Exact final R16B source-head run `35804891365` completed SUCCESS with:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link — PASS;
- current-source linked reproducibility — PASS.

Exact immutable Reconstruction-log-head run `35805083590` also completed SUCCESS
with the same canonical evidence classes.

Independent PS2-link log inspection confirmed:

`ELF_PRISTINE_SHA256=9b98aaa5b5239eec42545d111e7d32cedb418273fe4092f3deb5d8bd6690c40c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R16B
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

## Foreman-owned publication

Published `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0048 in:

`dade25f6c971584dacc7a039bc10961156654958`

Commit message:

`docs(foreman): accept R16B and activate Pi MPEG producer`

State 0048 marks R16B Foreman-accepted and activates:

`A003-PI-MPEG-CONTROL-PRODUCER-R17`

R17 is bounded to the missing maintained Pi-side MPEG generation-control/
producer mechanism. It must mirror the already-accepted PS2 START/RETIRE
representation, validate exact session/generation/geometry, preserve one
generation owner, use bounded producer buffering and channel-4 credit, serialize
all physical sends through the existing Wire owner, close admission before
retirement, wait out in-flight send leases, and emit exact RETIRE completion
only after real Pi capture/suppression/producer retirement.

Final PS2 Application MPEG activation/presentation/decoder orchestration remains
dependency-queued after R17.

## State-publication CI observation

Actions run `35806089974` on exact State-0048 commit first reproduced the known
timing-sensitive `transport_runtime_test` observation at assertions 862/864
(RFB payload availability/activity publication) while the commit changed only
Foreman documentation and the identical R16B source had already passed.

The failed `host-unit` job was rerun once on the identical commit. Attempt 2
passed the complete canonical host suite and the dedicated R16A provider-wait
proof. The run then completed SUCCESS; project-check, dictionary-long,
ps2-compile and reproducible ps2-link also passed. This records a nondeterministic
host-fixture/evidence observation, not a product-source change or hidden repair.

## Hardware/evidence boundary

The new R16B PT_LOAD identity is not hardware-qualified. Historical H1/RFB
hardware evidence is mechanism provenance and does not transfer to these bytes.
R17 will likewise begin as source/host/machine evidence unless separately
qualified on hardware.

## Next pickup

Reconstruction should recover current branch authority and execute only
`A003-PI-MPEG-CONTROL-PRODUCER-R17` from Foreman State 0048.

It must not begin final Application MPEG orchestration, Presentation ownership/
first-frame policy, AUDIO activation, heartbeat, CONFIG-on-Wire expansion, RFB
redesign or unrelated cleanup. If a required Pi producer/profile/desktop seam
cannot be supplied without duplicating or inventing authority, return a truthful
`BLOCKED` record naming that exact missing authority rather than importing H1
experiment scaffolding.

At the end of the R17 shift emit exactly one immutable Reconstruction work-log
record and stop.
