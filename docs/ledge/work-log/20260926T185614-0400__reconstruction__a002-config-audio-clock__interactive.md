# Ledge Reconstruction work log — R40 deferred AUDIO initial-credit activation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T18:56:14-04:00
COMPLETED_AT=2026-09-26T19:16:38-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ce02a1b06708aa6054c1ca493f75df467d33cf00
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and packet

This round recovered Foreman State revision 0082 at
`ce02a1b06708aa6054c1ca493f75df467d33cf00` and executed only the active
`A002-DEFERRED-AUDIO-CREDIT-ACTIVATION-R40` packet owned by Reconstruction /
`interactive`.

R40 was bounded to the lower AUDIO activation fence. It did not authorize or
perform ordinary Application AUDIO composition, ordinary
`pi/wire_runtime.py` AUDIO factory injection, AUDIO start policy, selected
R36 value changes, Wire/version changes, or changes to AUDIO playback/AUDSRV,
R37 thread runtime, R26 media clock, MPEG/RFB product owners, Input/UI/Display,
or H1 forensic source.

Final pre-log source authority is:

`add629833183f655533825e69286b95300817936`
— `test(audio): verify reconciled R40 source`.

The final source range is 18 commits ahead and zero behind the assigning
Foreman head.

## Implemented PS2 Transport activation fence

Transport now distinguishes exact AUDIO admission state:

- `PSTVNC_TRANSPORT_AUDIO_DORMANT`
- `PSTVNC_TRANSPORT_AUDIO_ACTIVATING`
- `PSTVNC_TRANSPORT_AUDIO_ACTIVE`.

A rider-capable Transport session still allocates and validates AUDIO queue,
semaphores, selected queue capacity, selected initial credit, credit batching
and receiver resources during open, but AUDIO begins DORMANT.

Receiver startup no longer emits channel-2 initial CREDIT. Existing RFB and
MPEG startup-credit behavior remains in the same owner path.

The new public seam is:

`pstvnc_transport_audio_activate(const pstvnc_transport_access_t *)`.

The bridge first applies the existing exact-ticket/session-lineage fence and
then delegates to the runtime owner.

Runtime activation is one-way and fail-closed:

1. validate initialized/enabled/nonterminal runtime;
2. lock the AUDIO queue owner;
3. require exact DORMANT state and nonzero configured initial credit;
4. publish ACTIVATING before any physical initial-CREDIT submission;
5. release the AUDIO lock;
6. submit the stored selected initial credit through the existing sole-owner
   synchronous outbound rendezvous;
7. after successful submission, reacquire the AUDIO lock and publish ACTIVE.

A duplicate call, stale ticket, disabled AUDIO runtime, terminal session, or
non-DORMANT state cannot emit a second initial CREDIT.

A failed physical initial-CREDIT submission marks Transport failed and leaves
the state ACTIVATING. It is never recycled to DORMANT and never retried as a
fresh activation.

## Race and consumer fences

Inbound AUDIO DATA and zero-length producer-done remain protocol-invalid while
DORMANT.

ACTIVATING is deliberately admissible on the receive side. Because ACTIVATING
is published before the sole-I/O initial CREDIT is submitted, a peer that
responds immediately to that CREDIT can send DATA before the caller has resumed
and observed ACTIVE without racing a stale DORMANT rejection.

AUDIO read, status, activity snapshot and activity wait APIs require exact
ACTIVE state. Before activation they reject immediately; they do not arm a
waiter or block indefinitely.

Dormant request-stop / receiver completion / release remains clean and needs no
producer-done marker or fabricated completion because no AUDIO producer
authority has been admitted.

## Pi lazy owner activation

R39's AUDIO PCM owner implementation remains unchanged.

`pi/wire_server.py` now retains an injected AUDIO factory as dormant
session-scoped mechanism authority rather than invoking it after Q4.

Q4 acceptance alone creates no AUDIO owner.

On the first valid channel-2 CREDIT:

1. decode the amount once;
2. require the active Wire Session identity;
3. call the configured factory exactly once with that session ID;
4. attach the created owner to that exact session;
5. apply the same decoded CREDIT amount exactly once.

Later channel-2 CREDIT reuses the already-attached owner and does not recreate
capture ownership.

Factory failure, exact-session attachment failure, or first-credit application
failure makes the Wire Session protocol-failed. A created-but-unattached owner
is closed immediately; an attached owner is closed through ordinary Wire
session finish.

Closing a Wire Session before any AUDIO CREDIT never invokes the factory.

The lazy factory is part of the existing Wire readiness loop even before an
owner exists; after first CREDIT the loop refreshes the newly attached owner and
uses its normal local readiness descriptor. WireServer remains the sole
PS2-facing Pi writer and global sequence allocator.

`pi/wire_runtime.py` remains unchanged and still supplies no AUDIO factory.
R40 therefore does not activate AUDIO in the ordinary product.

## Deterministic proof and regression repair

Focused Transport proof now covers:

- AUDIO-capable receiver startup sends no initial channel-2 CREDIT;
- RFB startup credit remains unchanged;
- explicit activation emits the configured initial AUDIO credit exactly once;
- duplicate activation is rejected with no second send;
- preactivation read/status/activity APIs reject;
- DORMANT nonzero DATA fails the session;
- DORMANT zero-length producer-done fails the session;
- clean DORMANT stop/close/release succeeds without fabricated producer
  completion;
- ACTIVATING admits immediate inbound AUDIO DATA;
- failed initial-CREDIT physical send is terminal and retains ACTIVATING;
- exact-ticket bridge activation succeeds only for the current session and
  rejects stale/disabled/terminal access.

A focused source-boundary test additionally proves the source ordering
ACTIVATING publication -> initial-CREDIT submission -> ACTIVE publication,
confirms receiver startup contains no AUDIO credit send, confirms the bridge
uses the exact access fence, confirms the Pi first-credit factory edge, and
confirms ordinary `pi/wire_runtime.py` remains AUDIO-disabled.

Two existing cross-domain host fixtures required lifecycle adaptation rather
than product rollback:

- the Transport receiver-completion fixture now explicitly activates AUDIO
  before arming an AUDIO activity waiter;
- the combined RFB/AUDIO/MPEG Transport fixture now explicitly activates AUDIO
  before queuing channel-2 DATA.

Those fixtures had previously assumed AUDIO-enabled implied AUDIO-active. The
first pre-fix CI runs exposed those assumptions as blocking waits; after the
explicit lifecycle updates the canonical host suite is green.

Pi proof covers:

- zero factory calls after Q4 alone;
- close-before-credit creates zero AUDIO owners;
- first CREDIT creates one exact-session owner and applies the original amount
  once;
- later CREDIT reuses that owner;
- a later Wire Session receives a fresh owner;
- RFB/AUDIO/MPEG continue sharing one WireServer send sequence;
- factory, wrong-session attachment, first-credit application, and retirement
  failures are terminal and clean up truthfully;
- ordinary product runtime still injects no AUDIO factory.

## Commits and changed files

R40 source/test commits, oldest first:

- `5c34d19e98e1e76eea2ade0c7faa879955466c82`
  — define R40 AUDIO activation state.
- `2170fd774ac924793c9bc0adc78964fe1c0cdea0`
  — defer initial AUDIO credit until activation.
- `103a2d0f863a7b1f7e139689cb3b9ffd9336e7e8`
  — expose ticket-scoped activation seam.
- `16c7d7de199f536b2235d0dd942d7694b2c75d44`
  — bind activation to current ticket.
- `e0945875383398814f7295491da33ed0ed2271b1`
  — make Pi AUDIO factory lazy on first credit.
- `b98f325f8218a061a2c0f2602c9b0cca32379cab`
  — prove deferred Transport AUDIO activation.
- `b87257bff1fdba3d2cdbcc44d36f0615620c22a8`
  — prove exact-ticket activation.
- `f7d619c0fbb0dfe842b864317bd36f6011899b35`
  — retain lazy factory in Wire readiness loop.
- `717eaaca71e9fcd85221f9dd1907be5def43c511`
  — prove lazy Pi factory activation/failure/reuse.
- `6a3d5966cd9471f0048e1d98295519e3f7d08229`
  — adapt completion waiter fixture to explicit activation.
- `5705d4be8f6321589dc7866362df98c767e40ac2`
  — add source-ordering proof.
- `88d9f3b653bd721e45bd5c0caaf8f25c458c82d5`
  — enroll source proof.
- `982e6189162c006d3bc286095dc656d8782bd54e`
  — adapt combined MPEG/AUDIO fixture.
- `ae39bc16ac508f1640d23c4a978a976a417e9b77`
  — clarify deferred Transport ownership.
- `1c40def5e4471b7d95dcfb703e42f0afcebd47a3`
  — document dormant startup boundary in Transport source.
- `1497634c86656e09e923df3c8d144b4b1b9456c1`
  — deterministic dictionary reconciliation trigger.
- `099d69c2904676315ba540a5f04b5e7eccac4fb5`
  — automation-generated current clean dictionary reconciliation.
- `add629833183f655533825e69286b95300817936`
  — exact reconciled-source verification head.

Changed paths are confined to:

- `src/transport/runtime.c`
- `src/transport/runtime.h`
- `src/transport/bridge.c`
- `src/transport/bridge.h`
- `pi/wire_server.py`
- focused Transport/Pi tests and `tests/Makefile`
- `src/transport/SYMBOLS.md`
- `pi/SYMBOLS.md`
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `src/app*`, `pi/wire_runtime.py`, R36 profile, AUDIO session/playback,
R37 PS2 AUDIO runtime, R26 clock, MPEG/RFB product owner, Input/UI/Display,
systemd activation, or H1 source changed.

## Exact final evidence

Canonical exact-source GitHub Actions run:

`36278839400`

checked out exact head
`add629833183f655533825e69286b95300817936`, attempt 1, and completed
SUCCESS.

Canonical disposition:

- host-unit = SUCCESS
- project-check = SUCCESS
- dictionary-long = SUCCESS
- ps2-compile = SUCCESS
- ps2-link = SUCCESS
- dictionary-reconcile = correctly SKIPPED.

Selected exact-head evidence includes:

- `transport bridge tests passed`
- `transport_runtime_test: PASS`
- `transport_audio_test: PASS`
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`
- `transport_mpeg_test: PASS`
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`
- `media_clock_test: PASS`
- `audio_playback_test: PASS`
- `audio_audsrv_service_test: PASS`
- `audio_session_test: PASS`
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`
- `audio_ps2_runtime_test: PASS`
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`
- current RFB/MPEG/Application regressions = PASS
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`
- `SOURCE_TOPOLOGY_CONTRACT=PASS`
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`
- `SOURCE_DICTIONARIES=PASS`
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`
- `WORK_LOG_CHECK=PASS`
- `PS_TO_VNC_PROJECT_CHECK=PASS`
- `ISSUE7_LINKED_BUILD=PASS`
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## Linked identity and hardware boundary

Accepted R39 PT_LOAD identity:

`PT_LOAD_SHA256=a5a048b8e96650bcce751c3899fb1491d7a41d5b7c2615e60e3cf4779f726313`

`PT_LOAD_BYTES=550804`.

R40 exact final-source identity:

`ELF_PRISTINE_SHA256=b7fc2805a2ac4351466594376df6c3a59ef9c9f3acccea0f3f952ecedebac963`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=7216b06d319ce6cb3e81085427ef7696bf36a0361c46c05a1cd7fe8bc1baebff`

`PT_LOAD_BYTES=551316`.

Therefore:

- R40_PS2_PT_LOAD_CHANGED=YES
- R40_HARDWARE_PENDING=YES
- R40_OPERATOR_OBSERVED=NO
- R40_HARDWARE_QUALIFIED=NO
- R40_INDEPENDENT_VALIDATION=NOT_RUN.

No physical behavior claim is made by this Reconstruction Worker.

## Worker requirement disposition

These are Worker findings, not Foreman acceptance.

1. AUDIO-capable Transport open remains allocated but DORMANT with no startup
   channel-2 initial CREDIT — MET.
2. Existing RFB/MPEG startup behavior remains independent — MET.
3. Exact-ticket public AUDIO activation seam publishes stored initial credit
   through the sole outbound owner — MET.
4. Activation is one-way; duplicate/stale/disabled/terminal/contradictory calls
   cannot send a second initial CREDIT — MET.
5. Inbound AUDIO DATA/producer-done while DORMANT fails closed — MET.
6. ACTIVATING is published before CREDIT; immediate post-credit DATA is admitted;
   send failure is terminal and ACTIVATING is not recycled — MET.
7. AUDIO read/status/activity APIs require ACTIVE and reject before activation —
   MET.
8. Dormant abort/close/release reclaims cleanly without fabricated producer
   completion — MET.
9. Pi factory is not called by Q4; first valid channel-2 CREDIT creates one
   current-session owner — MET.
10. The first decoded CREDIT is applied once; factory/attachment/application
    failure fails the session and closes created ownership truthfully — MET.
11. No capture exists before first CREDIT; later CREDIT reuses the same owner —
    MET.
12. Ordinary product AUDIO composition remains absent; selected R36 values and
    Wire version are unchanged; canonical gates are green — MET.

Evidence classification:

- R40_SOURCE_COMPLETE=YES_WITHIN_PACKET
- R40_HOST_TESTED=PASS
- R40_PROJECT_CHECK=PASS
- R40_STRICT_DICTIONARIES=PASS
- R40_PS2_COMPILE=PASS
- R40_PS2_LINK=PASS
- R40_CURRENT_SOURCE_REPRODUCIBILITY=PASS
- R40_MACHINE_EVIDENCE=GITHUB_ACTIONS
- R40_INDEPENDENT_VALIDATION=NOT_RUN
- R40_OPERATOR_OBSERVED=NO
- R40_HARDWARE_QUALIFIED=NO
- R40_HARDWARE_PENDING=YES

## Next pickup

There is no Reconstruction blocker at this stopping point.

The Interactive Reconstruction Worker stops here. The Foreman must independently
inspect the R40 source range, exact activation/race ownership, lazy Pi factory
edge, canonical evidence, changed PT_LOAD identity, and this immutable closeout
before accepting or rejecting R40 and selecting any successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R40_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
