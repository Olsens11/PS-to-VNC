# Ledge Foreman work log — accept R45 apparatus and authorize focused hardware discriminator

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T17:50:07-04:00
COMPLETED_AT=2026-09-27T17:54:26-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c5d75441af7fb0695f76f1fd1d6462b6a9ee6300
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned R45 sole-owner wake-discriminator apparatus.

Live pickup authority was:

`c5d75441af7fb0695f76f1fd1d6462b6a9ee6300`
— `docs(work-log): record R45 wake discriminator`.

The round independently recovered:

- Foreman State revision 0088;
- exact R45 immutable Reconstruction closeout;
- exact nineteen-commit R45 source/test/apparatus range;
- sealed LEDGE-FINAL-HW1 evidence;
- independently validated pre-HW1 product loadable identity;
- R44 causal-gap disposition;
- ordinary Transport receiver priority/profile authority;
- the baseline/control PS2 discriminator source/build;
- silent Pi recorder source and host tests;
- exact source-head and log-head GitHub Actions evidence.

No PS2 hardware was executed by the Foreman. No product behavior was changed by
the Foreman.

## Returned R45 range

Assigning Foreman/log authority:

`fa546c6d955c9e2503c857c8306b22e144e52625`.

Final pre-log R45 source/apparatus authority:

`6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`.

Immutable Reconstruction closeout:

`c5d75441af7fb0695f76f1fd1d6462b6a9ee6300`.

Independent compare proves:

- 19 commits ahead;
- 0 behind.

Authorized changes are confined to:

- restoration/test-selectability in `src/transport/runtime.c`;
- focused Transport host fixture changes;
- host/build enrollment;
- standalone R45 hardware discriminator and recorder apparatus.

No ordinary Pi product source, RFB profile values, Wire version, Application
policy, RFB parser behavior, media owner, Input/UI behavior,
Configuration/Management source or H1 forensic source changed.

## Product behavior restoration

R45 correctly removes the unaccepted R44 source conclusion.

Ordinary product runtime again uses:

- `PSTVNC_TRANSPORT_IO_SELECT_TIMEOUT_US=1000`;
- one outbound-ready PollSema before readiness;
- no R44 post-probe second PollSema;
- the existing 1000-us local EE cooperative idle yield.

The timeout macro is overridable only so the same host fixture can compile the
zero-timeout control path.

The retained all-build `PollSema` implementation changes host-test fidelity.
EE product behavior already used PollSema before R44.

Canonical product PT_LOAD is restored exactly to the independently validated
identity that failed HW1:

`PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`

`PT_LOAD_BYTES=556180`.

Current reproducible whole-ELF identity is:

`ELF_PRISTINE_SHA256=15e3c92f433208d914239754709717ab6608f9c4e233039c2f5abf4b7f80864f`.

The whole ELF differs because non-loadable debug metadata reflects changed
source/preprocessor layout; no PT_LOAD product-byte change remains.

## Honest host discriminator baseline

R45 fixes R44's causal test defect.

The same EE-like fake-PollSema interleaving is now tested twice:

- default baseline expecting 1000-us readiness;
- zero-timeout control expecting 0-us readiness.

In both host executions:

1. owner has missed outbound-ready;
2. parser-side consumer publishes exact 328-byte CREDIT;
3. no second inbound frame is queued;
4. the readiness stub returns its bounded idle result;
5. the owner later consumes the actual outbound semaphore and serializes CREDIT.

Both variants PASS.

This proves that the source-level check-after-miss ordering by itself does not
deterministically create an indefinite baseline stall when the readiness call
returns as bounded. It correctly leaves exact-platform select behavior as the
remaining discriminator question.

The useful R44 parser/credit exclusions remain green, including the exact sealed
328-byte / 12x13 Raw update and repeated isolated intervals.

## Independent hardware-apparatus review

The R45 PS2 discriminator is acceptable for focused hardware execution.

It uses:

- the exact pinned PS2SDK toolchain;
- the qualified frozen PS2IP archive;
- the repository's PS2 network initialization seam;
- receiver/owner priority 63, matching the selected product profile;
- one sole socket owner;
- one higher-priority submitter at 62;
- real EE PollSema;
- real PS2 select();
- one TCP connection to Pi port 5961;
- 4096 isolated cycles.

The submitter contains no socket, connect, send, receive or select operation.
Only the owner calls select and send.

For each cycle, the owner first misses outbound-ready and signals the exact
disputed boundary. The submitter becomes runnable, publishes the cycle through
the owner semaphore and blocks for real owner completion.

Baseline variant:

- `BASELINE_1000US`;
- 1000-us readiness;
- no post-probe second PollSema.

Control variant:

- `CONTROL_ZERO_TIMEOUT`;
- zero-timeout readiness;
- post-probe second PollSema.

Aside from the required variant identity/record fields, that is the intended
behavioral discriminator.

The Pi recorder is application-silent. It contains no send/sendall/sendto call.
TCP ACK behavior below the application layer is not treated as inbound
application wake traffic.

PASS requires all 4096 actual owner-serialized completion records in exact
cycle/sequence order.

Recorder idle timeout may classify STALL but is never completion authority.

A later recorder close after STALL may make the PS2 socket readable; hardware
evidence must preserve ordering so that post-measurement close is not
misidentified as the pre-timeout wake mechanism.

Exact discriminator identities:

BASELINE:

`ELF_SHA256=ce5b2eda81e2c2c65ff0d0c66f152d2fc0c4621956ef3f6c58aa789ab0fe0976`

`PT_LOAD_SHA256=97a1b02c89c8d72a8c836b5012cbc203a6337914577778245e6f2b547365ef6a`

`PT_LOAD_BYTES=316040`.

CONTROL:

`ELF_SHA256=8bf21f52f87b9c965fe3dc1dbf75ac59051a2e239915cc62b10cac183d91ebe5`

`PT_LOAD_SHA256=a5fe291785c692c8aabca88e8840e13e7fb343f8210a083d48276ba08f2a315c`

`PT_LOAD_BYTES=316168`.

Both variants are reproducible and distinct.

## R45 criterion disposition

All twelve R45 requirements are independently accepted:

- A001-R45-C1=MET
- A001-R45-C2=MET
- A001-R45-C3=MET
- A001-R45-C4=MET
- A001-R45-C5=MET
- A001-R45-C6=MET
- A001-R45-C7=MET
- A001-R45-C8=MET
- A001-R45-C9=MET
- A001-R45-C10=MET
- A001-R45-C11=MET
- A001-R45-C12=MET

R45_SOURCE_COMPLETE=YES_WITHIN_PACKET
R45_FOREMAN_ACCEPTED=YES_AS_DIAGNOSTIC_APPARATUS
R45_PRODUCT_FIX_ACCEPTED=NO

## Exact machine evidence

Exact pre-log source/apparatus GitHub Actions run:

`36352489627`, attempt 1, exact head
`6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- r45-discriminator-build;
- dictionary-reconcile correctly SKIPPED.

Observed evidence includes:

- baseline `transport_runtime_test: PASS`;
- control `transport_runtime_test: PASS`;
- `rfb_async_framing_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `transport_rfb_provider_failure_test: PASS`;
- neighboring AUDIO/MPEG/Application suites PASS;
- `R45_WAKE_RECORDER_TEST=PASS`;
- `R45_WAKE_DISCRIMINATOR_SOURCE_TEST=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`;
- `R45_DISCRIMINATOR_BUILD=PASS`.

Exact immutable-log-head run:

`36352699409`, attempt 1, exact head
`c5d75441af7fb0695f76f1fd1d6462b6a9ee6300`,
also completed SUCCESS.

## Foreman state write

Foreman State advanced from revision 0088 to revision 0089 in:

`de12c0ed89a604d35560b5412fa188662f6d7d52`
— `docs(ledge): accept R45 apparatus and authorize focused hardware`.

Revision 0089:

- accepts R45 only as diagnostic apparatus/causal-discriminator authority;
- explicitly does not accept a product fix;
- preserves HW1 as FAIL_PRODUCT_DEFECT;
- preserves the validated product PT_LOAD identity;
- publishes no active Reconstruction packet;
- authorizes exactly one focused hardware discriminator packet;
- forbids product ELF execution and product-source mutation during that packet.

No Foreman-owned product behavior was written.

## Active hardware packet

Exactly one hardware packet is active:

`PACKET_ID=A001-R45-FOCUSED-WAKE-DISCRIMINATOR-HW2`
`PACKET_OWNER=HARDWARE_QUALIFICATION`
`ROLE_KEY=hardware-qualification`
`WORK_ITEM_KEY=a001-sole-receiver`.

The hardware role must:

1. independently rebuild and verify both exact discriminator identities;
2. create sealed `LEDGE-R45-WAKE-HW2` evidence;
3. run baseline first with a silent fresh recorder;
4. preserve the complete baseline result;
5. reset/re-establish the PS2 harness environment;
6. run control with a fresh silent recorder;
7. never launch the PS2VNC product ELF in this packet;
8. never mutate product or apparatus source to rescue an outcome;
9. classify exactly one of:
   - BASELINE_STALLS_CONTROL_PASSES;
   - BASELINE_PASSES_CONTROL_PASSES;
   - BASELINE_STALLS_CONTROL_STALLS;
   - APPARATUS_INVALID;
10. return the baton to Foreman.

Any baseline-PASS/control-STALL combination is APPARATUS_INVALID because it is
outside the authorized causal classification.

No discriminator outcome itself is product hardware qualification.

## Next pickup

The focused hardware-qualification role must execute only:

`A001-R45-FOCUSED-WAKE-DISCRIMINATOR-HW2`.

It must emit exactly one immutable hardware-qualification work log and return
the baton.

NEXT_PICKUP=A001-R45-FOCUSED-WAKE-DISCRIMINATOR-HW2
