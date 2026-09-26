# Ledge Foreman work log — accept R38 and issue clean Pi AUDIO producer

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T18:22:05-04:00
COMPLETED_AT=2026-09-26T18:26:31-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=bd538e663ff71295e8ca68cbe49ff329b1bbffaf
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Timestamp note

The exact STARTED_AT above is the first truthful America/New_York timestamp
captured after the user's mid-round status interruption. Preliminary live
repository recovery and R38 inspection occurred earlier in the same Foreman
turn; no fabricated earlier timestamp is asserted.

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R38 AUDIO session-completion publication.

Live pickup authority was:

`bd538e663ff71295e8ca68cbe49ff329b1bbffaf`
— `docs(work-log): record R38 audio completion publication`.

The round independently recovered:

- Foreman State revision 0080;
- the exact immutable R38 Reconstruction closeout;
- the exact eight-commit R38 returned range;
- accepted R36/R37 AUDIO profile/execution authority;
- A002/A006/A007 audit authority;
- current clean PS2 Transport AUDIO mechanism;
- current clean Pi Wire protocol/server/runtime composition;
- forensic H1 PipeWire PCM producer authority;
- exact source-head and log-head canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R38 range

Assigning Foreman/log authority:

`e735b3298e1ee8a7b00671eaf2f62d6c32ce52a6`.

Final pre-log R38 source authority:

`9834277d61b1f8dd6ed8c47923cb328e414ba08d`.

Immutable Reconstruction closeout:

`bd538e663ff71295e8ca68cbe49ff329b1bbffaf`.

Independent compare proves:

- 8 commits ahead;
- 0 behind.

Changed source is confined to:

- `src/audio/session.c/.h`;
- `src/audio/ps2_runtime.c`;
- focused AUDIO session/runtime host tests and boundary checks;
- lifecycle documentation and AUDIO/source dictionaries.

No Application, Transport, Configuration profile values, media-clock behavior,
playback/AUDSRV service semantics, MPEG/RFB/Input/UI/Display, Pi product source
or Wire protocol behavior changed.

## Independent R38 findings

The public owner seam:

`pstvnc_audio_session_poll(session, state)`

is valid only for an exact initialized, created, successfully started,
not-destroyed worker.

It exposes only:

- `PSTVNC_AUDIO_SESSION_COMPLETION_PENDING`;
- `PSTVNC_AUDIO_SESSION_COMPLETION_DONE`.

The session poll does not inspect:

- Transport AUDIO status/activity;
- queue bytes or producer-done;
- media-clock state;
- delay/elapsed time;
- worker outcome;
- Application state.

Injected owner observation failure returns
`PSTVNC_AUDIO_SESSION_THREAD_STATUS_FAILED`; uncertainty is not silently
converted to PENDING.

The accepted R37 PS2 adapter now provides exact nonblocking
`poll_completion`.

Before completion it uses `ReferSemaStatus` and returns immediately.
A positive retained completion count is consumed only through nonblocking
`PollSema`; successful consumption records `completion_observed` before
returning DONE.

Once retained, later status calls and join do not attempt to consume/wait for the
same one-shot event again.

The poll path contains no:

- `WaitSema`;
- `DelayThread`;
- `ReferThreadStatus`;
- timeout;
- bounded retry;
- poll-count completion rule.

DONE is not reclamation proof. Existing join remains the worker-write visibility
and exact `THS_DORMANT` fence, and outcome/release remain unavailable until
that join succeeds.

R37 partial-start, allocation ownership, DeleteThread retry and runtime-release
semantics remain unchanged.

## Independent R38 criterion disposition

All twelve R38 requirements are independently accepted:

- A002-R38-C1=MET
- A002-R38-C2=MET
- A002-R38-C3=MET
- A002-R38-C4=MET
- A002-R38-C5=MET
- A002-R38-C6=MET
- A002-R38-C7=MET
- A002-R38-C8=MET
- A002-R38-C9=MET
- A002-R38-C10=MET
- A002-R38-C11=MET
- A002-R38-C12=MET

R38_SOURCE_COMPLETE=YES
R38_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36274451271`

independently confirms:

- exact head `9834277d61b1f8dd6ed8c47923cb328e414ba08d`;
- branch `ledge/h1-all-guns`;
- attempt 1;
- conclusion SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed evidence includes:

- `audio_session_test: PASS`;
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`;
- `audio_ps2_runtime_test: PASS`;
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log-head run:

`36274547884`

at `bd538e663ff71295e8ca68cbe49ff329b1bbffaf`, attempt 1, also completed
SUCCESS with host-unit, project-check, dictionary-long, ps2-compile and ps2-link
all successful.

Exact accepted linked identity:

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
- INDEPENDENT_VALIDATION=NOT_RUN
- OPERATOR_OBSERVED=NO
- HARDWARE_QUALIFIED=NO
- HARDWARE_PENDING=YES

This becomes the newest fully Foreman-accepted source/build identity.

## Next dependency review

R38 closes the PS2-side nonblocking completion gap, but the full AUDIO product is
still not ready for Application composition.

Independent clean-Pi inspection proves:

- `pi/wire_runtime.py` composes selected RFB and MPEG factories only;
- `pi/wire_protocol.py` has no channel-2 AUDIO constant/helpers;
- `pi/wire_server.py` has no AUDIO owner/factory, does not dispatch channel-2
  CREDIT and cannot emit channel-2 DATA;
- no clean Pi module discovers the default PipeWire sink monitor or launches the
  qualified PCM capture.

A007 classifies the Pi media mux/producer family as A001/A002/A003/A006 product
responsibility, so the missing clean Pi AUDIO owner is a real reconstruction
dependency.

The PS2 lower side already supports the exact companion semantics:

- channel 2 is AUDIO;
- session startup emits initial AUDIO credit when enabled;
- consumed bytes return exact credit;
- nonzero channel-2 DATA enters the bounded AUDIO queue;
- zero-length channel-2 DATA is the one-shot producer-done marker.

Therefore PS2 Application must not enable AUDIO before a clean Pi producer exists
to obey the same one-socket sequence/credit owner.

## Foreman state write

Foreman State advanced from revision 0080 to revision 0081.

Primary state packet commit:

`497c62f1e46e5880e3359a50cc63ea2dab8b662f`
— `docs(ledge): accept R38 and issue Pi audio producer`.

A same-round timestamp-only correction then produced final state authority:

`3f6e6ea3e93bfcc47b8d2ae35bb6dab09e6ceda3`
— `docs(ledge): correct R38 state timestamp`.

Revision 0081:

- accepts R38 at exact source/log authority;
- records the R38 linked identity as newest accepted build authority;
- identifies the missing clean Pi AUDIO producer/rider dependency;
- publishes exactly one R39 Pi AUDIO producer packet;
- keeps ordinary Pi AUDIO activation and PS2 Application AUDIO composition
  deferred to the later cross-platform composition packet;
- preserves Wire/product compatibility version;
- keeps automatic MPEG recalibration optional/deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A002-PI-AUDIO-PCM-PRODUCER-R39`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a002-config-audio-clock`
`WORKER_KEY=interactive`.

R39 must reconstruct only the clean Pi AUDIO owner and optional WireServer rider
seam.

Its core obligations are:

1. channel-2 AUDIO protocol helpers use existing Wire framing/version;
2. PCM DATA is exact 4-byte S16-stereo frame aligned;
3. Pi selected values are generated from R36 JSON, not copied;
4. default sink monitor discovery uses `wpctl inspect @DEFAULT_AUDIO_SINK@`;
5. `pw-record` uses selected 48 kHz/s16/stereo capture;
6. one exact session owner owns capture/credit/bounded producer state;
7. PS2 CREDIT is sole DATA admission authority;
8. WireServer remains the only physical socket/send-sequence owner;
9. producer readiness integrates into the existing event loop without blind
   polling sleeps;
10. unexpected capture EOF/error fails closed and never fabricates producer-done;
11. cleanup proves actual producer process/thread dormancy;
12. ordinary `pi/wire_runtime.py` does not enable AUDIO yet.

If implementation uses a local spool, it may not exceed the selected 524288-byte
channel window. No independent host AUDIO tuning knob is authorized.

R39 should not alter PS2 loadable source. Canonical linked evidence should
therefore remain byte-identical to accepted R38; any unexpected PT_LOAD change is
a packet blocker requiring explanation.

After R39 acceptance, the expected next dependency is one A006 ordinary
cross-platform AUDIO composition packet that simultaneously activates the
selected Pi AUDIO factory and PS2 Transport/AUDIO session under an explicit
session/start/failure/teardown policy.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A002-PI-AUDIO-PCM-PRODUCER-R39`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A002-PI-AUDIO-PCM-PRODUCER-R39
