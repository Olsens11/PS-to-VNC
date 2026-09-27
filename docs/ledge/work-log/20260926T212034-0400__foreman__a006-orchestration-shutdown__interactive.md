# Ledge Foreman work log — review R42 and issue retained-abort correction

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T21:20:34-04:00
COMPLETED_AT=2026-09-26T21:26:10-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=0c8e5738a325ba56582730d8253b998e1c3239ec
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned R42 ordinary cross-platform AUDIO composition.

Live pickup authority was:

`0c8e5738a325ba56582730d8253b998e1c3239ec`
— `docs(work-log): record R42 ordinary audio composition`.

The round independently recovered:

- Foreman State revision 0084;
- the exact immutable R42 Reconstruction closeout;
- the exact forty-nine-commit returned source/test range;
- ordinary R42 Application and Pi product composition source;
- accepted R33/R35P MPEG retained-abort authority;
- accepted R41 AUDIO retained-abort authority;
- A002/A003/A006/A007 architecture/audit authority;
- exact source-head and closeout-head canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was performed.

## Returned R42 range

Assigning Foreman/log authority:

`b1153979b1e013110a16349e3b7051155431c1c6`.

Final pre-log R42 source authority:

`66d07aacb1ce76aa55d820ea14d4ef7bf999bbcf`.

Immutable Reconstruction closeout:

`0c8e5738a325ba56582730d8253b998e1c3239ec`.

Independent compare proves:

- 49 commits ahead;
- 0 behind.

Product-bearing changes are confined to:

- ordinary `src/app.c/.h` AUDIO composition;
- narrow `pi/audio_product_profile.py`;
- ordinary `pi/wire_runtime.py` AUDIO factory injection;
- Pi staging enrollment;
- focused integration tests.

The rest is documentation, test-boundary maintenance and dictionary
reconciliation.

Wire version, R36 selected numeric values, lower Transport/AUDIO/R26/MPEG/RFB/
Input/UI/Display mechanisms and H1 forensic source did not change.

## R42 behavior accepted in review

Independent source review confirms:

1. selected R36 AUDIO authority resolves before platform startup;
2. resident LIBSD/AUDSRV preparation occurs once after IOP readiness;
3. every Wire attempt opens exact AUDIO+MPEG Transport with AUDIO dormant;
4. every attempt constructs a fresh R41 coordinator from exact ticket/R26 time
   authority;
5. AUDIO starts only after the public successful protected-MPEG predicate and at
   most once per Wire Session;
6. R41 service remains nonblocking in the ordinary loop;
7. while R26 is unarmed, MPEG live service is withheld until R41 reports exact
   reservoir/producer-done readiness;
8. once R26 is armed, later MPEG frames are not startup-gated;
9. normal same-session MPEG retirement/reveal does not stop or restart AUDIO;
10. ordinary Pi composition injects one lazy exact-session R39 factory;
11. Pi cleanup uses 2.0 seconds only as terminate-to-kill escalation;
12. no AUDIO capture owner exists before first channel-2 CREDIT.

The source implementation uses one Wire-attempt-local
`audio_start_attempted` fact, so a later MPEG generation cannot send another
AUDIO activation after normal R35 retirement.

## Exact machine evidence

Final-source GitHub Actions run:

`36285048528`

checked out exact source head
`66d07aacb1ce76aa55d820ea14d4ef7bf999bbcf`,
attempt 1, and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

The immutable R42 closeout head
`0c8e5738a325ba56582730d8253b998e1c3239ec`
has exact GitHub Actions run `36285151794`, attempt 1, also SUCCESS across the
complete canonical gate set.

Candidate R42 linked identity:

`ELF_PRISTINE_SHA256=bfc6a2fb9b22db2a170b6687b8625c7b6b85bb99fad6b7accc7002e9dea72021`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=6375db546f08a60b2b8bf34aa6786e8f740f44213b64fedecfb4081f5fe9f065`

`PT_LOAD_BYTES=556180`.

This candidate is machine-green but is not Foreman-accepted and has no hardware
qualification authority.

## Foreman-discovered R42 defect

R42 criterion 9 requires one shared retained-session begin-abort edge.

The ordinary helper correctly satisfies that rule while MPEG/AUDIO abort service
returns OK-but-not-ready: one `pstvnc_transport_session_begin_abort()` call is
followed by an internal no-timeout loop over both local owners.

A different reachable prefix violates the contract:

1. media debt exists;
2. Input shutdown succeeds;
3. `pstvnc_transport_session_begin_abort()` succeeds;
4. a later MPEG or AUDIO local abort service returns an actual error;
5. `retire_attempt_owners()` returns failure;
6. `attempt_failed` jumps to `attempt_fatal`;
7. `attempt_fatal` invokes `retire_attempt_owners()` again;
8. no successful-begin fact survived the first helper invocation;
9. the second helper invocation can call Transport begin-abort again for the
   same already-retained session.

This is not hypothetical lower behavior. Accepted R33/R35P MPEG and R41 AUDIO
abort seams can return explicit stop/poll/join/outcome/release/runtime-release
errors while retaining exact ownership for a later cleanup attempt.

The returned R42 tests prove PENDING retry but do not exercise either a
post-begin MPEG-abort error or a post-begin AUDIO-abort error across the
`attempt_failed` -> `attempt_fatal` cleanup boundary.

R42 criterion disposition:

- C1-C8 = MET
- C9 = NOT_MET
- C10-C12 = MET

Therefore:

`R42_SOURCE_COMPLETE=NO`
`R42_FOREMAN_ACCEPTED=NO`.

The newest fully Foreman-accepted PS2 linked identity remains R41:

`ELF_PRISTINE_SHA256=306ccd622c1a1c00bdb0ff24208c64b7c1832c8caa50d96797e752ad9f9ad997`

`PT_LOAD_SHA256=dd7d5f7bca743d3a505c809c53f78474559105244d91b29f62c5d83ec2bb4dc0`

`PT_LOAD_BYTES=555284`.

## Foreman state write

Foreman State advanced from revision 0084 to revision 0085 in:

`ec36e90be446ca420fda72247412249c41e15bdb`
— `docs(ledge): reject R42 C9 and issue retained-abort correction`.

Revision 0085 preserves R42's machine-green evidence and the eleven met criteria,
but does not promote the R42 candidate identity.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A006-R42C-RETAINED-SESSION-SINGLE-BEGIN-ABORT-R43`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a006-orchestration-shutdown`
`WORKER_KEY=interactive`.

R43 is deliberately narrow. It must add one per-Wire-attempt Application fact
for successful retained-session begin-abort establishment and preserve it across
all later cleanup invocations for that same attempt.

After begin-abort has returned OK once:

- no later cleanup invocation may call begin-abort again;
- MPEG/AUDIO local abort cleanup may resume/retry using the retained storage;
- clock release and Transport close remain forbidden until every owned media
  path proves ready;
- replacement remains forbidden until final close.

If the initial begin-abort call itself fails, the new fact must remain false.
R16B's no-media one-shot abort path remains unchanged.

Focused deterministic evidence must cover both MPEG and AUDIO post-begin local
abort errors, proving total successful-session begin-abort call count remains
one across the later fatal cleanup invocation.

R43 does not reopen R42 AUDIO start/gating/Pi composition, does not change lower
Transport or media owners, and does not enter hardware qualification.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A006-R42C-RETAINED-SESSION-SINGLE-BEGIN-ABORT-R43`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A006-R42C-RETAINED-SESSION-SINGLE-BEGIN-ABORT-R43
