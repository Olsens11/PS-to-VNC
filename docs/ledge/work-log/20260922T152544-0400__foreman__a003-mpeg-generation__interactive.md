DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T15:25:44-04:00
COMPLETED_AT=2026-09-22T15:47:02-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a307de050c015a46548647b45ac9f145b8ba8e71
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
HARDWARE_PENDING=YES
GOAL_STATUS=COMPLETED

# Foreman acceptance — R16A; R16B recovery activated

## Objective and incoming authority

Receive the immutable Reconstruction baton for
`A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A`, independently verify the
worker's source/evidence against current repository authority, reconcile the
new compatibility semantics with older temporal architecture records, decide
R16A criterion-by-criterion, and publish the next bounded Reconstruction packet
without crossing the Foreman/Reconstruction ownership boundary.

The pickup branch authority was:

`a307de050c015a46548647b45ac9f145b8ba8e71`

with immutable Reconstruction closeout:

`docs/ledge/work-log/20260922T094431-0400__reconstruction__a003-mpeg-generation__interactive.md`

The closeout's parent/final pre-log R16A source authority was:

`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`

The exact Reconstruction range after the prior immutable Foreman handoff
`594f2fdcebbf0f23e724b203532aeb2a14882f36` contained 34 pre-log commits plus
one immutable Reconstruction log commit. No commit existed after the closeout
when Foreman recovered the baton.

Authority consumed included the current branch/history, `AGENTS.md`,
`CONTRIBUTING.md`, project status/README/intent, clean architecture,
development README/naming/topology/lifecycle authority, Reconstruction Contract
revision 0006, work-log contract revision 0007, Foreman State 0046, Wire
Runtime Decisions revision 0011, Architecture Overlay revision 0007, Q1-Q12
reconciliation, relevant A001/A006 audits, the R16A provider-failure contract,
source-symbol dictionaries, the complete R16A diff, current product source,
dedicated tests and exact-head GitHub Actions evidence.

No local/Pi checkout was exposed to this Foreman environment. Therefore this
round makes no claim about external staged/unstaged/untracked worktree state and
did not run local `scripts/resume-state.sh` or `scripts/check.sh`. Repository
review and machine evidence in this record are GitHub authority only.

## Independent R16A source review

Foreman independently confirmed the worker's concrete compatibility/failure
representation rather than accepting the log's conclusions by assertion.

Accepted representation:

- fixed PSTV header/framing version remains `1`;
- Q4 product-establishment compatibility advances symmetrically to `2`;
- typed provider-terminal report is frame kind `ERROR=7`, RFB channel `1`,
  flags `0`, exact payload length `4`;
- payload is one big-endian `uint32`: CONNECT=`1`, READ/EOF=`2`, WRITE=`3`;
- wrong channel, flags, payload length or unknown reason is invalid;
- zero-length RFB DATA retains only the accepted R13 finite-quiesce meaning;
- incompatible product-v1/product-v2 peers reject during Q4 before ACTIVE.

Independent implementation findings:

1. `pi/rfb_attachment.py` latches the first specific CONNECT/READ/WRITE provider
   cause, makes the attachment terminal, clears provider-bound capacity and
   retires provider/private-wake resources. It adds no retry/rebind loop.
2. `pi/wire_server.py` remains the sole Pi physical receive/send/global-sequence
   owner. The attachment publishes a local terminal fact; the Wire owner alone
   serializes the ERROR report.
3. Successful report transmission does not erase or revive the original Pi
   provider failure.
4. `pi/wire_protocol.py` and PS2 `src/transport/protocol.*` implement the same
   exact frame/reason contract while fixed framing version stays 1.
5. PS2 `src/transport/physical_stream.c` sends product compatibility 2 in the
   actual Q4 HELLO path; this is not merely a test-only constant.
6. `src/transport/runtime.c` receives the typed provider-terminal fact into
   RFB-local state, clears provider-bound credit, wakes blocked RFB activity and
   does not set generic physical `runtime->failed` solely because the provider
   died.
7. Already ordered old RFB DATA may drain according to its existing sequence;
   no new provider-bound credit/ticket is minted after terminal reporting and
   stale old authority is not rebound to a replacement instance.
8. `src/rfb/bridge.*` and `src/rfb/rfb_session.*` expose/refine CONNECT, READ and
   WRITE provider terminality as an RFB-local result without acquiring
   Application retry/restart policy.
9. The dedicated PS2 waiter fixture proves a genuinely blocked reader and a
   genuinely blocked credit-waiting writer both terminate on the typed
   provider-local fact while the physical Wire runtime remains live.
10. The complete packet diff stays inside authorized Pi RFB/Wire mechanism,
    PS2 Transport/RFB mechanism, directly affected tests/workflow/dictionaries
    and continuity documentation. `src/app.*` recovery behavior and AUDIO/MPEG
    product source were not changed.

## R16A acceptance disposition

A003-R16A-C1_CONNECT_FAILURE_CROSSES_AS_TYPED_RFB_FACT=MET
A003-R16A-C2_EOF_READ_FAILURE_CROSSES_AS_TYPED_RFB_FACT=MET
A003-R16A-C3_WRITE_FAILURE_CROSSES_AS_TYPED_RFB_FACT=MET
A003-R16A-C4_FIRST_SPECIFIC_PROVIDER_FAILURE_PRESERVED=MET
A003-R16A-C5_WIRE_SESSION_REMAINS_DISTINCT_AND_NOT_FALSELY_FAILED=MET
A003-R16A-C6_SOLE_WIRE_SEND_RECEIVE_AND_SEQUENCE_OWNERSHIP_PRESERVED=MET
A003-R16A-C7_NORMAL_R13_QUIESCE_BYTES_AND_MEANING_UNCHANGED=MET
A003-R16A-C8_COMPATIBILITY_VERSION_CONTRACT_EXPLICIT_AND_SYMMETRIC=MET
A003-R16A-C9_PS2_RFB_WAIT_TERMINATES_WITH_RFB_LOCAL_FAILURE=MET
A003-R16A-C10_LATE_CREDITED_RFB_BYTES_CONTAINED_NO_REBIND=MET
A003-R16A-C11_NO_APPLICATION_RESTART_PROTOCOL_MEDIA_TIMEOUT_OR_RETUNE_SCOPE_CREEP=MET
A003-R16A-C12_HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN=MET

R16A is therefore accepted as source/build authority. It is not accepted as
hardware-qualified behavior.

## Exact machine evidence

At exact final pre-log source authority
`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`, GitHub Actions run
`35772175812` completed SUCCESS with:

- `host-unit` — SUCCESS;
- dedicated `Prove R16A PS2 RFB wait termination` — SUCCESS;
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS;
- `ps2-link` / current-source reproducibility — SUCCESS.

At exact incoming immutable Reconstruction closeout head
`a307de050c015a46548647b45ac9f145b8ba8e71`, GitHub Actions run
`35773248375` also completed SUCCESS across the same canonical gates and the
same dedicated R16A waiter proof.

Current linked-build identity at that accepted source/log authority:

ISSUE7_LINKED_BUILD=PASS
LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS
ELF_PRISTINE_SHA256=06a1d2858dd195458bab81d8e88842d4fca4d3e3aa38d412c8b303fc9f8b4de2
PT_LOAD_SEGMENTS=1
PT_LOAD_SHA256=db94f95160ec28491546d53231ecf9235fcacbae1680875ac7f16bcd1b77bff8
PT_LOAD_BYTES=486932
PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

Those linked bytes are reproducible machine identity only. They do not inherit
hardware qualification from earlier H1/checkpoint or RFB-only binaries.

## Temporal architecture reconciliation

Wire Runtime Decisions revision 0011 and Architecture Overlay revision 0007
remain governing for physical ownership, rider-local failure, Q4 establishment,
R13 quiesce and Application recovery ownership. Their packet-time statements
that then-accepted source did not change protocol/establishment bytes are
historical/as-of statements, not a perpetual ban on a later explicitly
authorized compatibility change.

The accepted R16A contract plus the later Foreman State 0047 are now the narrow
newer authority for ERROR=7 provider-terminal semantics and Q4 product
compatibility version 2. This does not alter fixed framing version 1, sole Wire
ownership, R13 quiesce bytes, R14 selected profile or Application policy
ownership.

## Foreman publication

Published Foreman State revision 0047 in commit:

`ee0fd512175036a6e931c05701867a3b2069b9ec`

Commit message:

`docs(foreman): accept R16A and activate R16B recovery`

State 0047 records R16A as `COMPLETED_FOREMAN_ACCEPTED`, clears the missing
provider-failure-representation architecture blocker, preserves original R16 as
historically `BLOCKED_FOREMAN_CONFIRMED`, and activates the bounded successor:

`A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B`

R16B owns the remaining Application recovery/lifecycle work: explicit typed
provider-failure policy, immediate failed-attempt admission closure, complete
input/RFB/Transport-session retirement, fresh network/Q4/attachment/RFB/input
authority on restoration, stale-attempt containment, and deterministic repeated
recovery evidence.

R16B is forbidden from redesigning the accepted R16A protocol/compatibility
contract, changing R13 quiesce bytes, duplicating/retuning the R14 profile,
activating media/heartbeat/CONFIG/direct-RFB fallback, or performing final
all-guns orchestration.

Exact State-0047 GitHub Actions run `35775792747` completed SUCCESS with:

- `host-unit` — SUCCESS;
- dedicated R16A waiter proof — SUCCESS;
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS;
- `ps2-link` / current-source reproducibility — SUCCESS;
- `dictionary-reconcile` — correctly SKIPPED because committed dictionaries
  were already reconciled.

## Evidence classification

R16A_SOURCE_COMPLETE=YES_FOREMAN_ACCEPTED
R16A_HOST_TESTED=YES
R16A_PROJECT_CHECK=PASS
R16A_STRICT_DICTIONARIES=PASS
R16A_PS2_COMPILE=PASS
R16A_PS2_LINK=PASS
R16A_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R16A_MACHINE_EVIDENCE=GITHUB_ACTIONS_ONLY
R16A_INDEPENDENT_VALIDATION=NOT_RUN
R16A_OPERATOR_OBSERVED=NO
R16A_HARDWARE_QUALIFIED=NO
CURRENT_SOURCE_HARDWARE_QUALIFIED=NO
R16B_PACKET_PUBLISHED=YES

## Next pickup

Reconstruction must recover current repository authority and execute only
`A003-RFB-FAILURE-STOP-RESTART-POLICY-R16B` from Foreman State 0047.

It must make the typed R16A provider-terminal cause reach explicit Application
policy, close admission for the failed attempt, prove complete owner retirement,
and restore ordinary RFB only through fresh network/Q4/Transport/attachment/RFB/
framebuffer/input authority. It must preserve the accepted lower-layer protocol,
quiesce and profile contracts, keep provider failure truthful and distinct from
physical Wire failure, avoid success-by-delay, and stop with exactly one
immutable Reconstruction work log before returning the baton.