# Ledge Foreman work log — review R44 and issue sole-owner wake discriminator

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T17:08:23-04:00
COMPLETED_AT=2026-09-27T17:16:29-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3b15c3f6023ce2a06d466192d893d8c580135a14
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned R44 for the HW1 incremental-RFB product stall.

Live pickup authority was:

`3b15c3f6023ce2a06d466192d893d8c580135a14`
— `docs(work-log): record R44 HW1 live-progress correction`.

The round independently recovered:

- Foreman State revision 0087;
- exact R44 immutable Reconstruction closeout;
- exact eight-commit R44 source/test range;
- sealed LEDGE-FINAL-HW1 evidence;
- independently validated pre-HW1 source identity;
- current Transport sole-I/O/outbound rendezvous source;
- current physical readiness/select source;
- RFB parser/credit/application progress path;
- exact R44 source-head and closeout-head canonical Actions evidence;
- current PS2SDK select semantics relevant to the asserted root cause.

No product behavior was modified by this Foreman round. No hardware run was
performed.

## Returned R44 range

Assigning Foreman/log authority:

`deeec5a24e09f73eda19bb7703d5a8c280781f44`.

Final pre-log R44 source/test authority:

`e005bf001a72302ba81e688e8616910077160764`.

Immutable Reconstruction closeout:

`3b15c3f6023ce2a06d466192d893d8c580135a14`.

Independent compare proves:

- 8 commits ahead;
- 0 behind.

Changed paths are confined to:

- `src/transport/runtime.c`;
- focused Transport runtime/AUDIO/MPEG host fixtures;
- common Transport host kernel stub;
- focused asynchronous RFB framing fixture.

Only `src/transport/runtime.c` changes loadable product behavior.

## What R44 proved correctly

The exact sealed HW1 RFB message is now reproduced deterministically:

- total RFB payload 328 bytes;
- one Raw rectangle;
- x=681;
- y=11;
- width=12;
- height=13;
- 312 pixel bytes;
- 704x462 framebuffer;
- exact in-bounds dirty rectangle.

The clean RFB parser consumes that entire message without any later server byte,
returns to a true complete-message boundary and can serialize a successor
incremental request.

A second isolated interval proves parser/request state does not become stale.

Transport fixture evidence also proves that parser consumption of one isolated
328-byte logical frame returns exactly 328 bytes of RFB CREDIT under the selected
flush-on-empty policy, and repeated cycles do not accumulate pending credit.

The host Transport fixture now models EE `PollSema` consumption semantics
instead of relying on `outbound_pending + WaitSema`.

These are useful exclusions and should remain.

## R44 candidate behavior

Product commit:

`0c914d2b7136b18958a96f137ff3ec4722ca3723`
— `fix(transport): make HW1 outbound progress inbound-independent`.

It changes the physical readiness timeout from 1000 us to zero and adds a
second outbound `PollSema` after the nonblocking socket probe before the
existing 1000-us cooperative EE delay.

The design preserves:

- one physical recv/send owner;
- domain submission through the owner rendezvous;
- parser-consumption-gated credit;
- unchanged Wire protocol;
- unchanged RFB queue/credit profile;
- no reconnect/watchdog/timeout-success policy.

The 1000-us local delay remains cadence only.

## Independent causal review

R44's central causal claim is not proven.

The pre-R44 product did **not** enter an unbounded socket wait by construction.
It passed:

`PSTVNC_TRANSPORT_IO_SELECT_TIMEOUT_US=1000`

to `pstvnc_transport_physical_stream_wait_readable()`.

That physical helper forwards the timeout to `select()`.

Therefore, if the selected PS2 select implementation honors the supplied
timeout, an outbound item published after the owner's first `PollSema` miss is
delayed by at most one bounded readiness interval before the owner loops back and
checks outbound work again. Future inbound Wire traffic is not required by the
source algorithm in that case.

The new R44 critical host test does not falsify this bounded path.

Its fake readiness function does not emulate a select call that ignores or loses
the 1000-us timeout. Instead the fixture explicitly enables
`require_zero_readiness_timeout` and fails if the product passes any nonzero
timeout. Thus pre-R44 fails the regression because the test asserts the proposed
zero-timeout design, not because the modeled old interleaving demonstrably
stalls.

Without that assertion, the host readiness stub has a bounded/immediate idle
return and the published outbound item can be serviced on the next owner pass.

That distinction matters because R44 criterion 11 required the critical
interleaving to fail under the pre-correction mechanism, or otherwise establish
another exact source defect explaining HW1.

Current upstream PS2SDK select source also forwards the caller's timeval to the
socket backend and contains the newer iop-fd range correction for the separate
historical select bug where the wrong descriptor range could block forever.
R44's own record states the pinned build postdates that correction.

None of this proves that the exact PS2 runtime can never violate or wedge its
1000-us select call. It means such a failure is the missing empirical fact and
cannot be assumed from the semaphore race alone.

## R44 criterion disposition

Independent Foreman disposition:

- C1 = MET;
- C2 = NOT_MET — the first missing HW1 PS2 progress fact is not established;
- C3 = NOT_MET — the old 1000-us bounded return path was not falsified;
- C4 = MET;
- C5 = MET;
- C6 = MET;
- C7 = MET;
- C8 = MET;
- C9 = MET;
- C10 = MET as composed deterministic parser/Transport/Application evidence;
- C11 = NOT_MET — the fixture enforces zero timeout rather than reproducing an
  old-path stall;
- C12 = MET.

Therefore:

`R44_SOURCE_COMPLETE=NO`

`R44_FOREMAN_ACCEPTED=NO`.

This is not a rejection of the zero-timeout design as impossible or undesirable.
It is a rejection of claiming that current evidence has shown it to be the
HW1 cause-resolving product correction.

## Exact machine evidence

R44 exact source-head run:

`36350306508`, attempt 1, exact head
`e005bf001a72302ba81e688e8616910077160764`.

All canonical jobs succeeded:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed focused results include:

- `transport_runtime_test: PASS`;
- `transport_physical_stream_test: PASS`;
- `transport_rfb_channel_test: PASS`;
- `transport_rfb_provider_failure_test: PASS`;
- `rfb_async_framing_test: PASS`;
- `rfb_session_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- AUDIO/MPEG/Application neighboring suites PASS;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact source-head candidate identity:

`ELF_PRISTINE_SHA256=d025df4cbe8c92e15931ab1ee30fe2f869fb2d61862441e5ad93cf4e0f20feb0`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=9c4a8d25ca9ec1798e84f716f16b711bcc86328449082e054ce1027a69016966`

`PT_LOAD_BYTES=556180`.

Exact immutable-log-head run:

`36350490291`, attempt 1, exact head
`3b15c3f6023ce2a06d466192d893d8c580135a14`,
also passed the complete canonical job set.

Machine-green status does not promote the unproven causal conclusion.

## Foreman state write

Foreman State advanced from revision 0087 to revision 0088 in:

`efc61abd1638b1e76526a05a6e70746ad306ab9b`
— `docs(ledge): reject R44 causal proof and issue wake discriminator`.

Revision 0088:

- preserves HW1 as FAIL_PRODUCT_DEFECT;
- records R44 as machine-green but not Foreman-accepted;
- preserves the exact R44 candidate identity without qualification transfer;
- keeps the validated/HW1-failed identity as the last independently validated
  product source;
- publishes exactly one bounded A001 evidence packet;
- authorizes no product hardware retest yet.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A001-HW1-SOLE-OWNER-WAKE-DISCRIMINATOR-R45`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a001-sole-receiver`
`WORKER_KEY=interactive`.

R45 must remove the unsupported causal assumption rather than stack another
behavioral guess on top of it.

It must restore ordinary product Transport readiness to the pre-R44 1000-us
semantics, preserve the useful R44 parser/credit evidence, and create a dedicated
PS2 hardware discriminator with two otherwise-identical variants:

1. baseline — 1000-us readiness;
2. control — zero-timeout/nonblocking readiness.

The harness must reproduce the disputed ordering:

- sole owner misses outbound-ready;
- owner enters the readiness phase;
- another EE thread publishes one outbound item;
- peer supplies **no inbound application traffic**;
- completion is proven only by the sole owner actually serializing the outbound
  sequence.

It must support repeated isolated cycles and record exact cycle/sequence/result
counts.

Reconstruction builds and machine-validates the apparatus only. It must not run
the PS2 experiment.

The later hardware classification is deliberately finite:

- BASELINE_STALLS_CONTROL_PASSES;
- BASELINE_PASSES_CONTROL_PASSES;
- BASELINE_STALLS_CONTROL_STALLS;
- APPARATUS_INVALID.

Only the first category would materially support returning to the R44
zero-timeout candidate as the product correction. None of the categories is
itself product hardware qualification.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A001-HW1-SOLE-OWNER-WAKE-DISCRIMINATOR-R45`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A001-HW1-SOLE-OWNER-WAKE-DISCRIMINATOR-R45
