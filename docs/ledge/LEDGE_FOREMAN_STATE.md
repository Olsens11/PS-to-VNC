# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0021
RECORDED_AT=2026-09-20T15:54:52-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0020
SUPERSEDES_FOREMAN_STATE_REVISION=0020
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0021 consumes the bounded Wire/Q1-Q12 detour and resumes ledge
reconstruction. The former manual A003 packet and its generic owner-discovery
gate are superseded. A004 is no longer held by unresolved Wire ownership, but it
remains dependency-queued behind the current bounded A003/Transport packet.

## Current Foreman phase

`A003_WIRE_SESSION_FOUNDATION_RECONSTRUCTION_ACTIVE__Q1_Q12_RECONCILED__A004_DEPENDENCY_QUEUED`

ARCHITECTURE_BLOCKER=NONE
SOURCE_RECONSTRUCTION_RESUMED=YES
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Governing reconciliation authority

Read before product work:

- `docs/ledge/LEDGE_Q1_Q12_RECONCILIATION.md` revision 0001;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0004;
- `docs/development/module-lifecycle.md`;
- `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` revision 0004.

## Active bounded Reconstruction packet

PACKET_ID=`A003-WIRE-SESSION-FOUNDATION-R1`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a003-mpeg-generation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`NORMAL_RECONSTRUCTION`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
WORKTREE_PREFLIGHT_REQUIRED=YES
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`
REFERENCE_HARDWARE_BRANCH=`wire/q1-q12-hardware-proof@acea15284131abf8b107352c9a6c944d1e3dd469`

### Objective

Land the smallest coherent clean-product Wire/session foundation already learned
and partly productized during the hardware detour, without importing proof
harnesses or the rejected generic Transport drain.

### Required source reuse / implementation scope

Inspect current source first. Where still compatible, prefer exact reuse of the
final hardware-branch product behavior from:

- `325cf8332212042bc78170fc8d5776a442985006` — one Transport-owned
  physical framed-I/O execution context;
- `03b09eb5a917cfca43001ea3322432a91c3c7473` — aligned runtime fixture;
- `d226b59ba54ef2e3f114906564289dada132b361` — cooperative idle
  scheduling opportunity on PS2;
- `8f4841968f47f20eadcb18ed166df54909a30e48` — opaque,
  Wire-lifetime-bound Transport access for RFB/audio/MPEG;
- `e5b42f80e36e4c77e1a2f4eccfc55c1ca4457a3d` and
  `2bbe74e7cce0eb914d9f874bb4270afa12b7cd96` — fixture adaptations.

Reconcile source dictionaries with `d31202b547...` and `c69c578a86...` only
after final source shape is known.

Do **not** import `70aafb4a...`, `f088080f...`, the divergent resume branch,
or proof harnesses as product code.

### Required invariants

1. Exactly one Transport-owned physical framed-I/O execution context advances
   physical Wire I/O. Riders never receive/send on the physical socket directly.
2. The PS2 idle path provides a cooperative EE scheduling opportunity sufficient
   for network progress. Preserve 1000 us only as the hardware-tested baseline,
   not ABI or declared final optimum.
3. Each ordinary cross-Wire module instance acquires opaque Transport authority
   valid only for the active Wire Session under which it was created.
4. Stale Session-A access remains terminal after B establishes; it may not send
   through B or consume B inbound state. Modules do not need numeric session IDs.
5. RFB, audio and MPEG carry/use their own acquired Transport access through
   their public owner boundaries.
6. Wire loss/terminal Transport access is a stop condition for the affected
   module instance. Each affected module completely retires old local
   workers/callbacks/queues/resources before replacement startup or reuse.
7. Do not solve module retirement with a project-wide Transport admitted-call
   drain. Add module-specific fencing only if source proves it necessary.
8. Preserve Q8/Q11 rider failure containment: one rider failure does not
   automatically fail Wire or unrelated riders.

### Acceptance criteria

- `R1-C1 SINGLE_PHYSICAL_IO_OWNER`: final source has one Transport physical
  framed-I/O owner and no rider physical-socket bypass.
- `R1-C2 COOPERATIVE_IDLE_PROGRESS`: the PS2 idle path preserves the
  hardware-supported scheduling invariant without making the tested delay ABI.
- `R1-C3 SESSION_BOUND_ACCESS`: opaque access is acquired per module instance
  and stale A access is rejected after session replacement.
- `R1-C4 RIDER_INTEGRATION`: RFB/audio/MPEG product paths and deterministic
  fixtures use the access-bound public seam.
- `R1-C5 MODULE_RETIREMENT`: affected module lifecycle evidence proves
  complete old-instance retirement before replacement/resource reuse, or the
  smallest module-local repair is landed where necessary.
- `R1-C6 NO_REJECTED_DRAIN`: no generic Transport admitted-call drain or
  resume-branch Q12 candidate is present.
- `R1-C7 DETERMINISTIC_CHECKS`: focused host tests, canonical host suite,
  `scripts/check.sh`, dictionary checks and available PS2 compile/link gates
  pass from the worker's normal execution seat.
- `R1-C8 EVIDENCE_BOUNDARY`: report SOURCE/HOST/PS2 compile/link separately;
  make no new HARDWARE_PROVEN claim without a physical run.

### Explicit non-goals

This packet does not require final clean product Q4 establishment, Pi Wire-server
service packaging/reproducibility, MPEG activation/retirement state machine,
repeated MPEG generations, Q7 overlapped restoration implementation, A004
product work, or a physical PS2 run.

### Worker return

Return exact source/test/docs files changed, whether each referenced
hardware-line commit was adopted exactly or semantically reapplied, criterion
dispositions, executed commands/results, any dirty/local work found during
preflight, and the exact next blocker/baton point. Emit exactly one immutable
Reconstruction shift log using the existing `a003-mpeg-generation` work-item
key.

## A004 disposition

A004 is no longer held on unresolved Wire ownership. Its historical audit remains
valid except where current Q7 supersedes restoration ordering. It remains
dependency-queued until this Transport/session foundation returns.

## Remaining hardware qualification debt

HARDWARE_PENDING=final product Q4 establishment; exact product Wire service/runtime; reconstructed module lifecycle; MPEG one-run/repeated generations/stale N-vs-N+1/Wire-loss; Q7 overlapped RFB restoration; presentation-visible handoff; full all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A003-WIRE-SESSION-FOUNDATION-R1` worker baton. If coherent,
integrate non-behavioral dictionary/topology evidence as needed and issue the
next bounded packet. Do not reopen Q1-Q12 merely because physical qualification
remains pending.
