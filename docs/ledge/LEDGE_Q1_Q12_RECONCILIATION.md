# Ledge Q1-Q12 Post-Hardware Reconciliation

DOCUMENT=LEDGE_Q1_Q12_RECONCILIATION
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-20T15:54:52-04:00
SOURCE_COMMIT=SELF
LEDGE_START_HEAD=cf0e298ea1e23af34010f779f83b05be957ffe32
HARDWARE_PROOF_HEAD=acea15284131abf8b107352c9a6c944d1e3dd469
EXPLORATORY_RESUME_HEAD=71a14dcfb749bc7e297ee0881bf361343038d6d4
WIRE_RUNTIME_DECISIONS_REVISION=0011
ARCHITECTURE_OVERLAY_REVISION=0004
TEMPORAL_CLASS=RECONCILIATION_AUTHORITY
TEMPORAL_SEMANTICS=GOVERNING_UNTIL_SUPERSEDED

## Purpose

This is the canonical baton from the bounded Wire/Q1-Q12 hardware-proof detour
back to normal ledge reconstruction. Repository history shows
`wire/q1-q12-hardware-proof` descends directly from the frozen ledge head and
contains the bounded decision/proof work. The divergent
`wire/q1-q12-hardware-proof-resume` branch is preserved as exploratory history
only and is not architecture authority.

Q1-Q12 are sufficiently settled to resume source reconstruction. Remaining
hardware qualification is tracked separately and is not a blanket source blocker.

## Current architecture

1. RFB provider is Pi-local/provider-neutral relative to Wire and does not own
   the physical product connection.
2. The mature Pi product has a product-owned Wire server. Wire owns physical
   connection/session, sole receive, framing/mux, ordered send and Transport
   flow control. Healthy Wire may remain completely idle.
3. Logical riders attach through Transport-owned Wire Channel Relays. Payload is
   domain-opaque; bounded capacity/credit/readiness is Transport bookkeeping.
4. Connection is provisional until Wire compatibility establishment succeeds.
   Pi assigns authoritative session identity. Final clean product establishment
   remains implementation/qualification work.
5. Persistent Pi configuration owns desktop calibration/display geometry; the
   runtime owner publishes one immutable active geometry snapshot.
6. MPEG activation is an Application/domain lifecycle transaction. MPEG region
   is durable domain meaning. MPEG generation is run-fencing, not the
   user-facing product definition.
7. MPEG stop closes new production first; accepted work may drain; RFB
   restoration may begin underneath while valid retiring MPEG remains visible;
   final retirement reveals already-refreshing RFB.
8. Failure containment starts at the smallest owner that can prove a safe state
   and escalates only as necessary.
9. A dead Wire Session never resumes. Reconnect creates a new session;
   session-scoped runtime dies; durable state survives by its actual owner; MPEG
   is not silently resumed.
10. Wire publishes INACTIVE / ACTIVE(session_id). ACTIVE is communication
    availability, not universal rider readiness.
11. Each module owns readiness/start/stop/failure lifecycle. Rider failure does
    not automatically mean Wire failure.
12. Cross-Wire module Transport authority is session-bound. Old authority cannot
    become replacement-session authority. Each module owns complete local
    retirement before replacement startup/resource reuse.

Wire Session identity and MPEG generation identity remain distinct validity
dimensions.

## Q12 temporal correction

The history at `5c3c53cd...` temporarily interpreted Q12 as requiring a
generic Transport admitted-call drain. Candidate implementation commits
`70aafb4a...` and `f088080f...` explored that idea.

Commit `143c77b74cdbc69e12df5224c96e0f37ff2fc1bf` explicitly rejected it and
is governing. The split is:

    Wire Transport -> communication validity across Wire Sessions
    module         -> complete retirement of its own workers/callbacks/queues/resources

Do not restore the generic drain. The divergent resume-branch commits
`d553f357...` and `71a14dcf...` are not architecture authority.

## Hardware-supported facts and limits

- Proof 1A: provisional establishment, authoritative session identity and a
  completely idle rider-free session worked.
- Proof 1B: incompatible version establishment was rejected and did not become
  ACTIVE.
- Proof 2: the same PS2 process survived Wire loss and established a different
  session identity; the dead session was not resumed.
- Proof 3: representative RFB Channel Relay mechanics, bounded credit and
  backpressure worked; Wire progressed while RFB was stalled; credit was not
  returned before domain consumption.
- Proof 4: split physical-I/O execution stalled in the tested transaction while
  the single physical-I/O owner completed it. This does **not** prove
  `ps2ip` is generally non-thread-safe.
- Proof 4C: RFB and PCM behaved as independent riders; intentional PCM failure
  remained local; Wire and RFB continued.
- Proof 4I: the single-I/O Transport needs an actual cooperative EE scheduling
  opportunity during idle polling in the tested workload. The tested 1000 us is
  not ABI or a declared final mechanism/optimum. The productized source was
  physically requalified.
- Proof 5/Q12: stale Session-A outbound authority did not enter B, stale A
  inbound access did not consume B payload, and fresh B authority remained
  usable. `Q12_TRANSPORT_SESSION_VALIDITY_HARDWARE_PROVEN=YES`.

`ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN`.

The unexecuted MPEG Proof 6 apparatus does not establish product MPEG lifecycle
or hardware qualification.

## Native Pi RFB authority

Current classified product-facing path:

    existing LightDM/Xorg desktop :0
        -> socket-activated X0tigervnc
        -> systemd-owned 192.168.50.1:5900 PS2-facing RFB endpoint

`127.0.0.1:5903` is Windows/operator development tooling, not product
architecture. The staged Openbox/lxpanel `:1` candidate remains historical
evaluation evidence and is not the current target.

Broader reproducible installation/lifecycle qualification of this native route
remains global work and does not block unrelated clean source reconstruction.

## Product-shaped hardware-branch disposition

| Commit | Disposition | Reconciliation decision |
| --- | --- | --- |
| `325cf83322...` centralize physical Wire I/O | DEFER_TO_PACKET | Product behavior; reuse in the next Reconstruction packet. |
| `03b09eb5a9...` runtime fixture alignment | DEFER_TO_PACKET | Paired deterministic fixture for the single-I/O owner. |
| `d226b59ba5...` cooperative idle yield | DEFER_TO_PACKET | Product behavior with bounded hardware support; preserve invariant, not 1000-us ABI. |
| `f81c18067c...` productized P4I evidence | REAPPLY_SEMANTICS | Evidence is authority by hardware-branch commit/result; not a product source commit. |
| `7b32e37c29...` module lifecycle/session validity | REAPPLY_SEMANTICS | Final lifecycle document is adopted after later Q12 correction. |
| `571f24b277...` lifecycle indexing | ADOPT_EXACT | Documentation/index intent is adopted. |
| `8f4841968f...` session-bound rider access | DEFER_TO_PACKET | Product behavior; use final post-143c semantics. |
| `e5b42f80e3...` RFB fixture adaptation | DEFER_TO_PACKET | Pair with session-bound access integration. |
| `2bbe74e7cc...` audio fixture adaptation | DEFER_TO_PACKET | Pair with current readiness/access behavior. |
| `d31202b547...` symbol reconciliation | DEFER_TO_PACKET | Regenerate/reconcile after product source lands. |
| `c69c578a86...` obsolete RFB symbol cleanup | DEFER_TO_PACKET | Integrate with final product source/dictionary state. |
| `70aafb4a26...` generic admitted-call drain | DO_NOT_ADOPT | Explicitly rejected by later architecture. |
| `f088080f90...` drain follow-up | DO_NOT_ADOPT | Belongs only to rejected candidate. |
| `143c77b74c...` module-boundary retirement | REAPPLY_SEMANTICS | Governing Q12 split; source revert unnecessary because ledge never adopted the drain. |
| `acea152841...` unrun MPEG Proof 6 apparatus | DO_NOT_ADOPT | Future test material only; not product/hardware authority. |

Foreman does not directly land the behavior-bearing source rows because the
governing Reconstruction contract assigns product behavior to Reconstruction.
The next packet consumes them rather than rebuilding already-proven work.

## Audit/current-authority supersession

Historical A003/A004 audits and immutable work logs remain unchanged. Where they
require complete MPEG retirement before RFB restoration starts, current Q7
supersedes that ordering. Where the old A003 manual packet requires
rediscovering Wire/Pi ownership or treats exact-generation preparation as the
product definition, current Q1-Q12 supersedes it.

The canonical current architecture is the combination of
`CLEAN_ARCHITECTURE.md`, `LEDGE_ARCHITECTURE_OVERLAY.md` revision 0004,
`LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011 and
`docs/development/module-lifecycle.md`.

## Remaining explicit qualification debt

Still pending where applicable:

- final clean product Q4 establishment;
- exact product Wire service/runtime;
- exact reconstructed module lifecycle;
- MPEG one-run lifecycle;
- repeated MPEG generations;
- stale generation N versus N+1;
- Wire loss during MPEG;
- Q7 overlapped RFB restoration;
- presentation-visible handoff;
- full product/all-guns endurance;
- exact product ELF qualification;
- reproducible lifecycle/installation qualification of the native Pi RFB path.

These are validation gates, not a blanket hold on clean source reconstruction.
