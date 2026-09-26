# Ledge Foreman work log — accept R39 and issue deferred AUDIO activation fence

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T18:46:51-04:00
COMPLETED_AT=2026-09-26T18:54:15-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=863328cdfbf6caa6677f02608f6e759aafc32131
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R39 clean Pi AUDIO PCM producer/rider work.

Live pickup authority was:

`863328cdfbf6caa6677f02608f6e759aafc32131`
— `docs(work-log): record R39 Pi audio producer`.

The round independently recovered current Foreman State revision 0081, the exact
R39 immutable Reconstruction closeout, the nine-commit returned source/test
range, accepted R26/R36-R38 AUDIO/clock authority, clean PS2 Transport AUDIO
mechanism, current Pi WireServer/runtime ownership, A002/A003/A006/A007 audit
authority, and exact canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R39 range

Assigning Foreman/log authority:

`88d5d86717e516e4588579d05c7bea7af6f79e54`.

Final pre-log R39 source authority:

`66b44a2e68db99b511504e998e13def4a6acb7cf`.

Immutable Reconstruction closeout:

`863328cdfbf6caa6677f02608f6e759aafc32131`.

Independent compare proves:

- 9 commits ahead;
- 0 behind.

The changed range is confined to:

- clean Pi AUDIO producer/profile projection;
- Pi Wire protocol/server optional AUDIO rider seams;
- deterministic profile generation checks;
- focused Pi producer/Wire tests;
- topology/lifecycle documentation;
- Pi/source dictionaries.

`pi/wire_runtime.py`, all PS2 product source, Transport implementation,
Application, media-clock behavior, MPEG/RFB/Input/UI/Display product code,
Wire compatibility version and H1 forensic source remain unchanged.

## Independent R39 findings

R36 remains the sole selected numeric AUDIO profile authority. The generated Pi
projection is exactly:

- channel window 524288 bytes;
- PCM rate 48000 Hz;
- channels 2;
- bits/sample 16;
- frame alignment 4 bytes.

The new Pi owner is exact-session scoped and owns one default-sink monitor
capture process, one reader thread, one bounded spool, exact CREDIT state and a
local readiness socketpair. It never owns the PS2 socket or Wire sequence.

Default-sink discovery uses:

`wpctl inspect @DEFAULT_AUDIO_SINK@`

and exact selected capture uses:

`pw-record --target <node.name>.monitor --rate 48000 --format s16 --channels 2 -`.

AUDIO DATA emission is admitted only by exact channel-2 CREDIT. Every emitted
payload is nonempty, four-byte aligned and bounded by available complete PCM,
credit and the existing 8192-byte Wire payload maximum. WireServer remains the
sole physical send/global sequence owner.

AUDIO readiness is integrated into the existing select-based Wire owner rather
than a blind polling loop.

Unexpected capture EOF, malformed PCM tail, reader failure, discovery/launch
failure and credit overflow are terminal owner failures. Ordinary cleanup never
emits the zero-length channel-2 producer-done marker.

Capture/process retirement is proof-driven. Timeout may trigger terminate/kill
escalation but cannot itself establish successful cleanup.

Ordinary Pi product composition remains AUDIO-disabled because
`pi/wire_runtime.py` was not modified.

## Independent R39 criterion disposition

All twelve R39 requirements are independently accepted:

- A002-R39-C1=MET
- A002-R39-C2=MET
- A002-R39-C3=MET
- A002-R39-C4=MET
- A002-R39-C5=MET
- A002-R39-C6=MET
- A002-R39-C7=MET
- A002-R39-C8=MET
- A002-R39-C9=MET
- A002-R39-C10=MET
- A002-R39-C11=MET
- A002-R39-C12=MET

R39_SOURCE_COMPLETE=YES
R39_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36277134548`

checked out exact head
`66b44a2e68db99b511504e998e13def4a6acb7cf`,
attempt 1, and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed focused/canonical evidence includes:

- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`;
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `transport_audio_test: PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- `audio_session_test: PASS`;
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`;
- `audio_ps2_runtime_test: PASS`;
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`;
- current Pi Wire/RFB/MPEG suites PASS;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The immutable R39 closeout head
`863328cdfbf6caa6677f02608f6e759aafc32131`
has exact GitHub Actions run `36277245270`, attempt 1, also SUCCESS across the
complete canonical gate set.

R39 changes no PS2 loadable source. Exact linked identity remains:

`ELF_PRISTINE_SHA256=3b8319a17aa57e50a75399b1eb4ac0c35e59168f2805a1d7d531bd68d92450c6`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=a5a048b8e96650bcce751c3899fb1491d7a41d5b7c2615e60e3cf4779f726313`

`PT_LOAD_BYTES=550804`.

Evidence classification:

- HOST_TESTED=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARIES=PASS
- PS2_COMPILE=PASS
- PS2_LINK=PASS
- CURRENT_SOURCE_REPRODUCIBILITY=PASS
- MACHINE_EVIDENCE=GITHUB_ACTIONS
- R39_PS2_PT_LOAD_CHANGED=NO
- INDEPENDENT_VALIDATION=NOT_RUN
- OPERATOR_OBSERVED=NO
- HARDWARE_QUALIFIED=NO
- INHERITED_HARDWARE_PENDING=YES

## Newly exposed activation-boundary dependency

R39 is not the final missing dependency before ordinary AUDIO composition.

Accepted A003/R26 authority keeps the common media epoch unarmed until the first
real MPEG presentation. The current product has manually triggered MPEG
activation, so that boundary may occur arbitrarily long after Wire admission.

Current Transport AUDIO behavior sends the configured AUDIO initial credit
automatically during receiver-thread startup.

R36 selects that initial AUDIO credit as 524288 bytes.

If ordinary Pi composition enabled the accepted R39 factory without another
lower fence, the startup CREDIT would immediately create the PipeWire capture
owner and begin filling the AUDIO queue before any MPEG epoch exists. The queue
would preserve old desktop PCM and later present those samples against a newly
armed media epoch.

That is not a Configuration problem and must not be hidden in Application by
discarding arbitrary old bytes.

The correct lower mechanism is an explicit one-shot AUDIO activation edge:

- Transport allocates dormant AUDIO resources at session open;
- no initial channel-2 CREDIT leaves at receiver startup;
- an exact ticket-scoped activation operation publishes the already-selected
  initial credit once;
- inbound AUDIO is forbidden while dormant;
- Pi AUDIO factory creation is lazy on that first CREDIT.

This preserves the selected R36 value and existing Wire bytes while allowing
later A006 policy to choose the exact AUDIO start moment.

## Foreman state write

Foreman State advanced from revision 0081 to revision 0082 in:

`1ac4e6e6f5946ac4f088181dd79d31add797a903`
— `docs(ledge): accept R39 and issue audio activation fence`.

Revision 0082:

- accepts R39 at exact source/log authority;
- preserves R38 PS2 loadable identity because R39 is Pi-only;
- publishes exactly one deferred AUDIO-credit activation prerequisite;
- keeps Application AUDIO composition and ordinary Pi AUDIO factory activation
  deferred;
- preserves R36 selected values and Wire version;
- keeps optional MPEG auto-recalibration deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A002-DEFERRED-AUDIO-CREDIT-ACTIVATION-R40`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a002-config-audio-clock`
`WORKER_KEY=interactive`.

R40 must:

1. stop automatic AUDIO initial-credit emission at Transport receiver startup;
2. preserve RFB/MPEG startup credits unchanged;
3. add one exact ticket-scoped AUDIO activation seam that sends the stored
   selected initial credit once through the sole outbound owner;
4. publish ACTIVATING before physical CREDIT submission so immediate Pi DATA
   cannot race dormant receiver admission;
5. reject channel-2 DATA/producer-done before activation;
6. reject pre-activation AUDIO consumer use;
7. allow clean abort/close of dormant AUDIO resources;
8. make Pi AUDIO factory construction lazy on first valid channel-2 CREDIT;
9. apply that first CREDIT exactly once after exact-session owner creation;
10. fail closed and retire any created owner on lazy-create/attach/credit error;
11. create no Pi capture owner for sessions that never activate AUDIO;
12. leave ordinary Pi/Application AUDIO composition absent.

R40 must not add a Wire control frame or couple activation automatically to
MPEG. Final start policy remains A006 Application work.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A002-DEFERRED-AUDIO-CREDIT-ACTIVATION-R40`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A002-DEFERRED-AUDIO-CREDIT-ACTIVATION-R40
