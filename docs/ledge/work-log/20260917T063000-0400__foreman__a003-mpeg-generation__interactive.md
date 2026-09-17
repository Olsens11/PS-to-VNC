# Foreman shift — A003 P2A ownership recovery and completion packet

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-17T06:30:00-04:00
COMPLETED_AT=2026-09-17T06:45:22-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=08d0b2e07264c1da85c304fda1a1292358ca57a3
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Consume the interactive A003-P2A `PARTIAL` Reconstruction handoff, independently disposition P2A-1 through P2A-8, preserve correct wire/control work, resolve the previously undesignated clean production Pi PSTV/session/control ownership, repair the unrelated frozen-work-log governance red without rewriting immutable history, obtain settled canonical evidence, and issue only the bounded P2A-completion packet.

Governing authority consumed included Reconstruction contract revision 0005, A003 audit revision 0001, Foreman state revision 0014, current clean Transport/source-topology authority, current Pi companion/desktop authority, frozen H1 START/RETIRE and Pi desktop metadata evidence, and the immutable Reconstruction handoff `20260916T221349-0400__reconstruction__a003-mpeg-generation__interactive.md` at `6ccd4339d709b0626a6ea237ba99d4c7fc6d03e7`.

At the exact shift start the connector first surfaced branch authority `08d0b2e07264c1da85c304fda1a1292358ca57a3`. A subsequent live refresh no longer resolved that transient authority and returned `6ccd4339d709b0626a6ea237ba99d4c7fc6d03e7` as current branch authority. No write was based on the transient observation: live branch authority was re-read before every substantive write, and all committed work below descends from the then-current live branch.

No product rollback was required. The prior recovery result remained valid: failed P2 attempts did not leave a committed half-P2 lifecycle implementation.

## Reconstruction handoff consumed

Worker handoff:

- immutable handoff commit `6ccd4339d709b0626a6ea237ba99d4c7fc6d03e7`;
- substantive source commit `b609aec92f8f54d7d57ea81b38936bc45cd7d6d9` — `reconstruct(a003): add exact MPEG generation wire control`;
- deterministic dictionary trigger `46befa0157f8537c39251bf55b8f151470b583cf`;
- generated dictionary reconciliation `bcea2522997a1c00bee5ceafd616d8d0202aa5e1`;
- worker result `FOREMAN_GOAL_RESULT=PARTIAL`.

Direct source/test inspection confirmed that `b609aec...` changes only shared Transport framing/control codecs, the existing PS2 ordered outbound START seam, and behavior-specific protocol tests. `src/transport/runtime.c` remains untouched. No prepared-generation state, Pi producer lifecycle, decoder-retirement lifecycle, second socket/receiver, MPEG generation tag, or payload-sniffing classifier was introduced.

## Foreman P2A disposition

P2A-1 `START_CONTROL_IDENTITY=MET`.

Accepted without rework. START is explicit kind 11 / control channel 0 / flags 0 and is not DATA/channel 4 or payload-sniffed.

P2A-2 `START_SEMANTICS=PARTIAL`.

Accepted: exact 44-byte v1 eleven-word big-endian wire representation and exact version/length codec. Missing: owner-correct Pi active-session/nonzero-generation/signed-range/geometry/bounds validation, stale/repeat/conflict rules, and one immutable prepared generation.

P2A-3 `RETIRE_WIRE_IDENTITY=MET`.

Accepted as wire/control identity only: kind 10 / control channel 0 / flags 0 with exact 12-byte big-endian `(version, session_id, generation)` representation. No producer cleanup, completion ACK, decoder retirement, or successor lifecycle is implied.

P2A-4 `PURE_MPEG_DATA=MET`.

Accepted. Deterministic regression proves START-shaped exactly 44-byte DATA/channel-4 bytes remain MPEG media and do not classify as START or RETIRE.

P2A-5 `SOLE_TRANSPORT_OWNER=PARTIAL`.

Accepted PS2 portion: existing physical stream/send sequencing and sole PS2 receiver are preserved with no new socket or reader. Missing: designated/implemented clean Pi sole PSTV session/receive owner plus narrow PS2 inbound RETIRE control publication.

P2A-6 `OWNERSHIP=PARTIAL`.

Accepted: wire/framing code contains no generation business state, MPEG decoder ownership is unchanged, and Pi START receive semantics were correctly not placed in PS2 `src/transport/runtime.c`. Missing: production Pi PSTV Session / exact-generation coordinator implementation and public PS2 control seams above private physical-stream mechanism.

P2A-7 `NO_GENERATION_TAGGING=MET`.

Accepted. MPEG DATA format is unchanged; no per-packet generation field, magic 44-byte discriminator, or payload sniffing exists.

P2A-8 `BOUNDED_SCOPE=MET`.

Accepted. No P2B/P2C/P2D, presentation/common-clock policy, A004 reconstruction, DESKTOP CALIBRATION change, or hardware claim is present.

No accepted `b609aec...` wire/control work is assigned for redo absent a concrete defect.

## Clean production Pi ownership resolved

Updated `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` to revision 0002:

- commit `66a0c95a1b00e7183ce0c8bb57dc6456bfff2224` — `docs(architecture): designate clean Pi PSTV ownership`.

The architecture now designates `pi/pstv/` as the clean production Pi PSTV runtime component namespace.

The **Pi PSTV Session** owns one accepted PS2-facing PSTV socket/session, the Pi endpoint's sole receive chain/sequence, framing/version/length checks, ordered sends, explicit frame-kind/channel/flags dispatch, Transport-level control publication, and mechanism-level connection/session teardown.

A separate internal **Pi Exact-Generation Coordinator** owns active session identity, highest accepted generation/stale authority, one optional immutable prepared START value, START semantic validation, and accepted exact-generation state. The Pi PSTV Session publishes typed START/RETIRE control identity plus exact payload and immutable session context to that coordinator. Transport does not infer generation semantics from MPEG media.

Active Pi desktop width/height are immutable Pi-local session context supplied to the coordinator by the active PS2-facing Pi desktop/session owner that creates/adopts the Xtigervnc desktop. Historical H1 `desktop_width` / `desktop_height` support this ownership. `704x462` remains a current qualified/candidate session value, not a permanent product maximum.

This desktop-bounds authority is not PS2 **DESKTOP CALIBRATION**. START's base/suppression geometry is **MPEG CALIBRATION / MPEG presentation geometry** consumed only as already-qualified generation-preparation input. This shift did not begin A004.

The Pi MPEG producer remains deferred to P2B and is not authorized by this state.

The architecture also pins the PS2 control boundary: inbound RETIRE control must later be accepted/classified by the existing sole PS2 Transport receiver and published upward through a narrow seam; P2A may add that publication mechanism only, not decoder/join/drain/credit/successor lifecycle.

## Separate governance repair

The canonical red at the P2A partial handoff was not a Reconstruction/product defect. Two already-frozen otherwise-canonical work logs contained `LOG_FORMAT_REVISION=0005` while canonical log format remains 0001:

- `20260916T160445-0400__integration__global-dictionary-prep__dictionary.md`;
- `20260916T162043-0400__validation__a003-mpeg-generation__validation.md`.

Neither immutable record was edited.

Work-log policy advanced to revision 0006:

- commit `d1afc0f9c32b7c53c49d9a72c08b256b3c7b3ca1` — `docs(work-log): preserve two frozen format revisions`.

Revision 0006 preserves those two exact paths as **format-revision-only** compatibility cases. The existing nine broad legacy grandfather records remain unchanged. All ordinary canonical filename, required-metadata, status, timestamp, self-pause, and role/work/worker checks continue to apply; new records still require `LOG_FORMAT_REVISION=0001`.

Checker implementation:

- commit `f78cb9868107e5fe6f07e6e20358ffce442437a9` — `tooling(work-log): admit exact frozen revision exceptions`.

The checker uses exact path matching only and does not accept revision 0005 generally.

## Canonical evidence

At worker handoff commit `6ccd4339...`, workflow run `35208923263` had:

- host-unit `SUCCESS`;
- PS2 compile `SUCCESS`;
- PS2 link/current-source reproducibility `SUCCESS`;
- dictionary-long `SUCCESS`;
- project-check `FAILURE` only at the pre-repair work-log compatibility gate.

After the narrow governance repair, workflow run `35211799038` at `f78cb986...` settled fully green:

- host-unit `SUCCESS`;
- PS2 compile `SUCCESS`;
- PS2 link/current-source reproducibility `SUCCESS`;
- dictionary-long `SUCCESS`;
- project-check `SUCCESS`.

Its project-check reports:

`WORK_LOG_CHECK=PASS records=118 grandfathered=9 format_compat=2`

and:

`PS_TO_VNC_PROJECT_CHECK=PASS`.

Foreman state revision 0015 was then committed:

- `ae803eeb18cb683cd89d8e973990a2bcd05f07c2` — `docs(foreman): issue A003 P2A Pi completion packet`.

Its own canonical workflow run `35212006015` also settled `SUCCESS`, with host-unit, PS2 compile, PS2 link/reproducibility, dictionary-long, and project-check all green.

These are machine/repository evidence only. They do not constitute independent Validation PASS or physical qualification.

## P2A-completion packet issued

Foreman state revision 0015 issues only `A003-P2A-COMPLETION` to `interactive`.

The worker must preserve accepted `b609aec...` wire/control behavior and complete only:

- clean production `pi/pstv/` Pi PSTV Session behavior;
- typed Pi START/RETIRE publication to the Pi Exact-Generation Coordinator;
- full audited START semantic validation;
- one immutable prepared generation and stale/repeat/conflict behavior;
- immutable active Pi desktop-bounds injection from the Pi desktop/session owner;
- Pi RETIRE publication/semantic-request validation without cleanup/ACK lifecycle;
- application-facing PS2 START send seam above private physical-stream ownership;
- narrow PS2 inbound RETIRE publication from the existing sole receiver without P2C lifecycle;
- deterministic host proof of those boundaries and continued P1/wire regressions.

P2B, P2C, P2D, A004, producer activation/emission/cleanup, decoder stop/join/drain/credit restoration, successor-generation reopening, presentation/common-clock arm, DESKTOP CALIBRATION changes, and hardware qualification remain explicitly unauthorized.

PENDING_LOCAL=A003-P2A-completion Reconstruction handoff; subsequent Foreman integration and independent Validation
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification

## Exact next pickup

Interactive Reconstruction should read live branch authority and Foreman state revision 0015, then execute only `A003-P2A-COMPLETION`. It must report P2A-C1 through P2A-C8 criterion-by-criterion and emit exactly one immutable Reconstruction log with `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED`. There is no P2B stretch target.