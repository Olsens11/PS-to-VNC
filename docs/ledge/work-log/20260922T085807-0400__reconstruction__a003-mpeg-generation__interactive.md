# Reconstruction shift — R16 provider-failure visibility boundary

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T08:58:07-04:00
COMPLETED_AT=2026-09-22T09:12:47-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=BLOCKED
STARTING_BRANCH_COMMIT=84e88377b55228dcf6fa0af8601550c05639ea6f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Execution classification

EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
HARDWARE_PENDING=YES
GOAL_STATUS=BLOCKED

## Objective and authority consumed

This shift consumed Foreman state revision `0045` and active packet
`A003-RFB-FAILURE-STOP-RESTART-POLICY-R16`, based on accepted R15 source
`377ab24f066a71a813aecccd4c92ccb096d94c27` and live starting branch authority
`84e88377b55228dcf6fa0af8601550c05639ea6f`.

The packet objective was to reconstruct the smallest explicit ordinary-RFB
failure/stop/restart policy that makes accepted R13 provider connect/read/write
failure visible to Product/Application, keeps RFB-provider failure distinct from
physical Wire/Transport failure, retires the failed authority completely before
recovery, and permits restoration only through fresh session-scoped authority.

The packet also froze the existing Wire/Q4 representation and explicitly required
a `BLOCKED` return if exact provider-failure visibility would require a new or
ambiguous Wire representation. It forbade inventing a new frame kind, flag,
Wire version, control channel, marker meaning, or repurposing the accepted R13
zero-length DATA quiesce representation.

Governing authority read during the shift included:

- `AGENTS.md`
- `CONTRIBUTING.md`
- `docs/status.md`
- `docs/README.md`
- `docs/PROJECT_INTENT.md`
- `docs/CLEAN_ARCHITECTURE.md`
- `docs/development/README.md`
- `docs/development/module-lifecycle.md`
- `docs/development/source-naming-and-symbols.md`
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision `0006`
- `docs/ledge/work-log/README.md` revision `0007`
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision `0045`
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md`
- `docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md`
- the preceding R15 Reconstruction closeout and current Foreman handoff history.

## Exact work performed

No product, test, runtime, service, configuration, dictionary, build-manifest, or
Foreman-state source was changed.

The shift traced the accepted R13/R15 provider-failure path end-to-end and tested
all existing candidate representations against the R16 frozen-protocol and
failure-domain constraints.

### Pi R13 attachment failure behavior

`pi/rfb_attachment.py` already has terminal local `FAILED` state. Provider connect
failure and terminal provider read/write failure converge through its local
failure path, which clears pending credit and retires the connecting/provider,
relay, and private wake resources. The dead attachment is not rebound or retried.

That is sufficient for R13-local containment, but the state is only Pi-local. It
is not itself an existing Wire-visible Application failure fact.

### Pi Wire owner behavior

`pi/wire_server.py` remains the sole Pi physical Wire owner. Its accepted
RFB-channel output vocabulary is the existing finite-flow/quiesce traffic:
initial CREDIT plus zero-length DATA REQUEST/COMMIT markers and ordinary RFB DATA.
It does not translate `RfbAttachment.FAILED` into any existing cross-Wire provider
failure indication.

Killing or closing the physical Wire socket to surface this condition was rejected
because the active packet explicitly keeps provider failure RFB-local and requires
provider failure to remain distinguishable from physical Wire/Transport failure.
The R13 attachment is not allowed to acquire Wire termination/rebind ownership.

### Existing Pi Wire vocabulary

`pi/wire_protocol.py` defines the currently maintained product Pi frame kinds
HELLO (`1`), DATA (`3`), CREDIT (`4`), ACCEPT (`12`), and NOT_ACCEPTED (`13`).
It contains no provider-error frame codec, payload, classification, or RFB-local
failure representation.

The accepted zero-length RFB DATA representation cannot be reused: current R13
semantics reserve it for finite quiesce REQUEST/COMMIT progression, and R16
criterion C5 freezes that meaning.

### PS2 Transport `ERROR=7` archaeology

`src/transport/protocol.h` contains enum member
`PSTVNC_TRANSPORT_FRAME_ERROR = 7`, but repository archaeology found no accepted
clean-generation semantic contract that assigns this frame an RFB-provider-failure
meaning, payload, channel rule, or codec.

The enum reservation originates in clean-source framing commit
`259be4a074bd6c84f9822f0ad06551b991055011` (`reconstruct wire framing base`).
The early protocol implementation beginning at
`5aaf4b07702c4f85ae5cfc66e9d144889fc215fc` provides generic header framing and
specific established codecs, but no `ERROR=7` provider-failure semantics.
Repository searches found no later accepted implementation establishing such a
meaning.

Current `src/transport/runtime.c` does not dispatch `ERROR=7` as an RFB-local
provider-failure fact. An unrecognized inbound frame is a Transport runtime
failure. Therefore merely emitting numeric kind `7` from the Pi would not reuse a
defined provider-failure contract: it would either retain the present generic
Transport failure collapse or require assigning new Wire semantics so Application
could distinguish provider failure from physical Wire/Transport failure.

That new semantic assignment is exactly the new/ambiguous Wire representation the
R16 packet forbids this worker from inventing.

### Other existing candidates

No existing accepted frame/channel/flag/marker was found that simultaneously:

1. carries provider connect/read/write terminality from Pi to PS2 Application;
2. preserves the normal zero-length RFB quiesce markers unchanged;
3. leaves the physical Wire session alive rather than converting the condition
   into Transport failure; and
4. gives Application an unambiguous RFB-provider-failure fact without defining a
   new Wire meaning.

Accordingly there is no authorized source edit that can satisfy C1-C3 together
with C5, C10, and the frozen-protocol boundary.

## Blocker / stop-rule disposition

`STATUS=BLOCKED` is the packet-prescribed result, not a missing-environment or
user-action blocker.

Exact blocker:

- provider failure is observable only inside the Pi R13 attachment today;
- the current accepted Pi Wire vocabulary has no provider-failure representation;
- normal zero-length RFB DATA markers are already owned by quiesce and are frozen;
- closing Wire would collapse the RFB-provider fault into Wire/Transport failure;
- the dormant PS2 `ERROR=7` enum has no accepted RFB-provider-failure contract and
  current runtime does not expose it as such;
- assigning `ERROR=7` (or any other existing numeric/marker shape) a new
  provider-failure meaning would create the new/ambiguous Wire representation
  explicitly forbidden by R16.

The R16 stop rule therefore applies before product-source mutation. No speculative
protocol extension was made.

## Acceptance criteria disposition

- C1 provider connect failure visible to Application: `BLOCKED` at frozen Wire
  representation boundary.
- C2 provider EOF/read failure visible to Application: `BLOCKED` at the same
  boundary.
- C3 provider write failure visible to Application: `BLOCKED` at the same
  boundary.
- C4 provider failure does not acquire Wire ownership: preserved by making no
  Wire-termination workaround.
- C5 normal R13 quiesce unchanged: preserved.
- C6 explicit Application failure convergence: cannot be implemented until an
  authorized unambiguous provider-failure fact can cross the Pi/PS2 boundary.
- C7 complete stop before restart: existing local R13 retirement evidence was
  inspected, but end-to-end Application-owned recovery cannot be completed while
  C1-C3 are blocked.
- C8 fresh session-scoped authority restoration: existing R13/R15 fresh attachment
  model remains unchanged; R16 recovery policy not added.
- C9 no stale attachment/access-ticket/credit/wake reuse: existing R13 retirement
  model remains unchanged; no retry/rebind was introduced.
- C10 physical Wire failure remains distinct: preserved by rejecting Wire close
  and generic Transport-failure collapse as provider-failure signaling.
- C11 no protocol/media/timeout/retune creep: met by stopping before mutation.
- C12 stable tests/docs/dictionary/build boundary: no behavior source changed;
  this immutable log is the only shift write.

## Files and commits changed

Product/source changes: `NONE`.

This shift creates exactly one immutable Reconstruction record:

`docs/ledge/work-log/20260922T085807-0400__reconstruction__a003-mpeg-generation__interactive.md`

Its commit is `SELF` for purposes of this immutable entry.

## Checks and evidence

Evidence was repository/source archaeology and exact current-source inspection.
No product-source patch was produced, so no new behavior test result is claimed.
The post-log canonical GitHub Actions result necessarily occurs after this
immutable record is committed and therefore is not backfilled into this log.

Previously accepted R14 runtime-profile values and R15 ordinary activation source
were not changed or retuned. No new Wire frame, channel, flag, marker meaning,
Wire version, timeout/backoff, AUDIO/MPEG behavior, provider retry, second physical
owner, or direct-RFB bypass was introduced.

`PENDING_LOCAL=NO` for the repository-level blocker determination.
`HARDWARE_PENDING=YES` remains unchanged: no physical PS2/Pi qualification was
performed or inferred.

## Known-defect / evidence accounting

This shift does not classify the absence of an authorized provider-failure Wire
representation as permission to improvise one. The missing semantic is returned
to the Foreman as a packet-definition/architecture authority blocker.

The dormant `ERROR=7` numeric enum alone is insufficient evidence of an accepted
RFB-provider-failure contract. Treating it otherwise would erase the distinction
between historical vocabulary reservation and reconstructed behavior authority.

No claim is made about live Pi service state, Pi-local worktree cleanliness,
physical hardware behavior, or performance qualification.

## State/contract revisions

Consumed:

- `LEDGE_FOREMAN_STATE` revision `0045`
- `LEDGE_RECONSTRUCTION_CONTRACT` revision `0006`
- `LEDGE_WORK_LOG_CONTRACT` revision `0007`

Produced state revisions: `NONE`.
Foreman state was not edited by this Reconstruction worker.

## Next pickup

Foreman must independently review the blocker and decide whether to revise R16
with an already-authoritative failure representation, authorize a protocol/semantic
change in a later bounded packet, or choose another architecture-preserving path.
This worker does not select that policy.
