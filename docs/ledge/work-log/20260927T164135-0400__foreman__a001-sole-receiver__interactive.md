# Ledge Foreman work log — consume Validation/HW1 and reopen A001 live progress

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T16:41:35-04:00
COMPLETED_AT=2026-09-27T16:49:06-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=bb646ed6c3733488876719cd4a4917831669da8d
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after complete-source independent
Validation and the first final exact-DUT hardware qualification campaign had both
advanced repository authority.

Live pickup authority was:

`bb646ed6c3733488876719cd4a4917831669da8d`
— `evidence(hardware): record LEDGE-FINAL-HW1 product stall`.

The round independently recovered:

- Foreman State revision 0086;
- Validation state revision 0007 and findings revision 0006;
- Validation ledger commit
  `81d5915eb0d57d769a66f19e4f180b8eef0a2b79`;
- validated final product source
  `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`;
- exact HW1 hardware work log and sealed evidence directory;
- current RFB parser/Application receive path;
- current Transport logical RFB queue/credit path;
- sole physical-I/O thread and outbound submission rendezvous;
- A001 audit and current Wire/architecture ownership authority.

No product source was modified by this Foreman round. No physical hardware action
was performed by the Foreman.

## Validation consumed

Independent Validation closed the complete accumulated A001-A006 machine/source
tranche:

`VALIDATION_COMPLETE_CURRENT_LEDGE_MACHINE_SOURCE=PASS`.

Its exact validated source identity is:

`ELF_PRISTINE_SHA256=993655ccffc430347281291c1fa917e3b22ce77f17407a99edbc920b71a65a2a`

`PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`

`PT_LOAD_BYTES=556180`.

Validation explicitly left physical qualification pending. Its PASS is not
silently withdrawn: it remains valid for the deterministic/source obligations it
reviewed.

## HW1 evidence consumed

The subsequent exact-DUT campaign:

`LEDGE-FINAL-HW1`

correctly bound and deployed the final reconstructed product.

Initial hardware behavior passed:

- exact runtime identity observed;
- NET_READY observed;
- GS_READY observed;
- DESKTOP_READY observed;
- INPUT_READY observed;
- no FATAL stage;
- final product Wire endpoint established;
- native internal X0tigervnc provider activated;
- initial PS2 desktop matched the independent 5903 operator view.

The campaign then failed and stopped at the first product defect.

Exact sealed failure boundary:

- PS2 final application frame:
  DATA/RFB sequence 132 carrying a 10-byte incremental
  FramebufferUpdateRequest for 704x462;
- Pi final application frame:
  DATA/RFB sequence 130 carrying one exact 328-byte FramebufferUpdate;
- update rectangle:
  x=681, y=11, width=12, height=13, Raw;
- Raw bytes: 312;
- rectangle: in bounds;
- RFB payload: exact;
- PS2 TCP ACKed final Pi bytes;
- no subsequent PS2 PSTV CREDIT;
- no subsequent framebuffer request.

At that boundary:

- Wire service remained active with zero restarts;
- internal provider remained active with zero restarts;
- Wire TCP remained ESTABLISHED;
- independent 5903 desktop remained live/current;
- no FATAL diagnostic was emitted.

The correct hardware classification is therefore preserved:

`HARDWARE_QUALIFICATION=FAIL_PRODUCT_DEFECT`.

The separate provider-stager STATIC/enabled guard defect is apparatus debt and
did not block the live runtime.

## Foreman source-level classification

The failure is routed to A001's PS2-side RFB/Transport live-progress boundary.

Current source gives an important candidate mechanism but not yet a proven root
cause.

RFB exact consumption returns credit synchronously through
`pstvnc_transport_runtime_submit_frame()`.

That submission:

1. registers an outbound submitter;
2. publishes one pending work item;
3. signals `outbound_ready_semaphore_id`;
4. blocks on `outbound_done_semaphore_id`.

The sole physical-I/O thread:

1. nonblockingly checks the outbound-ready semaphore;
2. if none is visible, enters socket readability wait;
3. only after that wait returns does it check outbound readiness again.

The outbound semaphore itself does not wake the socket readiness wait.

Therefore the following interleaving is real in current source:

- I/O owner checks outbound readiness and sees none;
- owner enters readiness wait;
- Application/RFB consumption publishes CREDIT or request work;
- submitter blocks waiting for owner completion;
- no later inbound Wire frame exists to wake the physical receive side.

Current implementation relies on its bounded select/idle cycle to guarantee
return to the outbound poll. HW1's exact isolated-update failure makes this
boundary the primary mechanism to prove or falsify.

The Foreman does **not** declare that `select()` timeout behavior is the root
cause. The first missing local progress fact could still be proven elsewhere:
logical commit, parser consumption, presentation/accounting, or request
serialization.

## Foreman state write

Foreman State advanced from revision 0086 to revision 0087 in:

`03113716658d0c24cb4f0b66cab38a1815e72a97`
— `docs(ledge): route HW1 RFB stall to Reconstruction`.

Revision 0087:

- consumes Validation PASS_MACHINE_SOURCE;
- consumes exact HW1 failure authority;
- preserves prior A002-A006 acceptance;
- reopens seeded Reconstruction only for the newly demonstrated A001 product
  defect;
- records HW1 as failed/pending, not qualified;
- publishes exactly one bounded A001 correction packet;
- forbids watchdog/reconnect/timeout-success substitution.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A001-HW1-RFB-INCREMENTAL-LIVE-PROGRESS-R44`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a001-sole-receiver`
`WORKER_KEY=interactive`.

R44 must first determine the earliest missing PS2 progress fact from the HW1
boundary and then correct only that owner.

The primary concurrency proof must cover an outbound submission occurring after
the sole I/O owner has checked outbound work and before/during its wait state,
with no future inbound frame available as an accidental wake.

Any accepted correction must preserve:

- one physical Wire I/O owner;
- credit only for parser-consumed bytes;
- existing Wire bytes/version;
- current selected RFB window/credit values;
- complete-message RFB parser ownership;
- R16B/provider-failure semantics;
- R20C/R20D/R20E terminal/drain/reclaim semantics;
- current MPEG/AUDIO lifecycle behavior.

It may not add:

- a second physical sender;
- direct RFB socket ownership;
- automatic reconnect;
- freeze watchdog/restart;
- timeout/poll-count success;
- protocol or tuning changes merely to hide the failure.

If deterministic source/test work cannot distinguish the first missing local
progress fact, Reconstruction must return BLOCKED with the exact additional PS2
witness needed rather than guessing.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A001-HW1-RFB-INCREMENTAL-LIVE-PROGRESS-R44`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A001-HW1-RFB-INCREMENTAL-LIVE-PROGRESS-R44
