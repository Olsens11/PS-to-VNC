# Ledge Reconstruction work log — R45 sole-owner wake discriminator

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T17:25:29-04:00
COMPLETED_AT=2026-09-27T17:40:51-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=fa546c6d955c9e2503c857c8306b22e144e52625
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and packet

This Reconstruction shift consumed Foreman State revision 0088 and executed only:

`A001-HW1-SOLE-OWNER-WAKE-DISCRIMINATOR-R45`.

The assigning Foreman State commit is:

`efc61abd1638b1e76526a05a6e70746ad306ab9b`
— `docs(ledge): reject R44 causal proof and issue wake discriminator`.

The assigning Foreman immutable log / branch authority at Reconstruction pickup
was:

`fa546c6d955c9e2503c857c8306b22e144e52625`
— `docs(work-log): reject R44 causal proof and hand off wake discriminator`.

The packet was based on:

- last independently validated / HW1-failed product source
  `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`;
- sealed HW1 evidence
  `bb646ed6c3733488876719cd4a4917831669da8d`;
- unaccepted R44 candidate
  `e005bf001a72302ba81e688e8616910077160764`;
- Foreman State revision 0088;
- immutable work-log contract document revision 0007.

The canonical log-format value remains `LOG_FORMAT_REVISION=0001`.

The exact initial read-only role/authority recovery timestamp was not captured
before the first mutation. To avoid inventing an earlier clock time,
`STARTED_AT` above is the exact first R45 repository-mutation timestamp,
17:25:29-04:00. Read-only packet recovery immediately preceded it.

Final pre-log source/test/apparatus authority:

`6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`
— `docs(r45): document apparatus ownership regression`.

The final pre-log range is exactly 19 commits ahead of the assigning Foreman
handoff and zero behind.

## R44 product conclusion removed

R44 was not Foreman-accepted. R45 therefore returned ordinary product Transport
scheduling to the pre-R44 semantics instead of treating zero-timeout polling as
an accepted fix.

Product behavior in `src/transport/runtime.c` again uses:

- `PSTVNC_TRANSPORT_IO_SELECT_TIMEOUT_US=1000`;
- one outbound-ready `PollSema` before the physical readiness call;
- no R44 second outbound poll inside the idle branch;
- the existing 1000-us cooperative EE idle yield after a bounded idle
  readiness return.

The only retained R44 mechanism change in this source is host-test fidelity:
`pstvnc_transport_runtime_take_outbound_ready()` uses `PollSema` in host
fixtures as well as EE builds. The accepted pre-HW1 EE product already used
`PollSema`, so this does not change loadable PS2 behavior.

The readiness timeout macro is wrapped in an `#ifndef` solely so the same host
fixture can compile a zero-timeout control executable. Its default remains
1000 us.

Product-restoration commits:

- `9cb990297147c173cfd173127508722927aac7e6`
  — `fix(transport): restore pre-R44 1000us readiness semantics`;
- `6d0c8fe63948ccb3ea3f6897a7b3a2269f7502da`
  — `test(transport): permit R45 timeout control build`.

No accepted product fix is claimed by R45.

## Honest deterministic baseline / control host evidence

The retained R44 concurrency fixture was corrected so it no longer declares
nonzero readiness invalid merely because the timeout is nonzero.

`tests/unit/transport_runtime_test.c` now has an explicit expected timeout
compile constant:

- default build: `R45_EXPECTED_READINESS_TIMEOUT_US=1000`;
- control build: `R45_EXPECTED_READINESS_TIMEOUT_US=0`.

Both use the same fake EE-like `PollSema` ordering.

The critical concurrency test still places a parser-side submitter after the
owner's first outbound-ready miss, consumes one complete 328-byte logical RFB
DATA item, publishes the exact 328-byte consumption CREDIT, queues no second
inbound frame, and waits for real owner completion.

For the default 1000-us build, the physical readiness stub returns the ordinary
bounded-idle result and the owner services the published work on its next pass.
This proves the old design is **not deterministically stuck merely because the
publication occurs after the first PollSema miss**.

For the zero-timeout control build, the same fixture also passes under the
control timeout override.

Both are enrolled in the ordinary host suite:

- `.build/transport_runtime_test` — baseline 1000-us semantics;
- `.build/transport_runtime_zero_timeout_test` — zero-timeout control.

Final host evidence contains two independent:

`transport_runtime_test: PASS`

results, one for each compile-time scheduling variant.

This resolves the R44 host-test defect: the host model does not reproduce an
indefinite baseline stall. Therefore hardware is required to decide whether the
real PS2/PS2SDK readiness operation violates the expected bounded-return fact.

## Retained R44 parser / credit evidence

The useful R44 evidence was preserved.

The exact sealed HW1 RFB response remains covered:

- total RFB payload = 328 bytes;
- one Raw rectangle;
- x=681;
- y=11;
- width=12;
- height=13;
- 312 Raw pixel bytes;
- negotiated framebuffer = 704x462.

The real clean RFB parser fixture still proves:

- the whole isolated response is accepted;
- all 328 bytes are consumed;
- the session returns to a true complete-message boundary with no later RFB
  byte;
- framebuffer dirty geometry is the exact 12x13 rectangle;
- a successor incremental FramebufferUpdateRequest is serializable;
- a second isolated 328-byte interval also completes and admits another
  successor request.

The Transport fixture still proves repeated exact consumption-credit return with
no accumulated pending credit.

Final canonical host results include:

- `rfb_async_framing_test: PASS`;
- `rfb_session_test: PASS`;
- `rfb_initial_frame_test: PASS`;
- `rfb_initial_coverage_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `transport_runtime_test: PASS` for both R45 variants.

R45 therefore changes neither RFB parser semantics nor selected 32768/8192/8192
flow values.

## Dedicated PS2 hardware discriminator

R45 adds a standalone diagnostic apparatus under:

`tests/hardware/r45-wake-discriminator/`.

It is not linked into the PS-to-VNC product and was not executed during
Reconstruction.

The PS2 source is:

`r45_wake_discriminator.c`.

It builds two variants from the same source:

1. `BASELINE_1000US`
   - `R45_ZERO_TIMEOUT_CONTROL=0`;
   - readiness timeout 1000 us;
   - no post-readiness second PollSema;
   - represents the pre-R44 scheduling shape.

2. `CONTROL_ZERO_TIMEOUT`
   - `R45_ZERO_TIMEOUT_CONTROL=1`;
   - readiness timeout 0;
   - second PollSema after the nonblocking readiness probe;
   - represents the unaccepted R44 scheduling control.

Both variants use:

- the exact pinned PS2SDK build environment;
- the qualified frozen PS2IP archive;
- the repository's ordinary PS2 network initialization seam;
- PS2 address 192.168.50.2 and Pi recorder address 192.168.50.1;
- dedicated diagnostic TCP port 5961;
- one physical socket owner only;
- one domain submitter that never touches the socket;
- real EE `PollSema`;
- real PS2 `select()`;
- 4096 isolated publication/completion cycles.

### Exact disputed interleaving

For every cycle:

1. the owner executes `PollSema(outbound_ready)`;
2. it proves a miss;
3. it signals the dedicated `owner_missed` rendezvous;
4. the submitter is one priority step above the product-shaped owner
   (submitter 62, owner 63), so publication is runnable immediately in the
   disputed gap;
5. the submitter publishes exactly cycle N and signals outbound-ready;
6. the owner enters the variant readiness strategy;
7. the Pi sends no application bytes;
8. only owner-side real socket serialization can signal outbound-done;
9. the submitter validates cycle/sequence/completion equality before entering
   N+1.

The submitter cannot become a second socket owner. It contains no send, receive,
select, socket or connect operation.

The owner is the only context that can serialize the completion record.

A source invariant regression proves:

`R45_WAKE_DISCRIMINATOR_SOURCE_TEST=PASS`.

It mechanically verifies:

- one `send()` site;
- one `select()` site;
- no `recv()` path;
- no socket primitive in the submitter;
- exact baseline/control compile constants;
- 4096-cycle requirement;
- deliberate owner/submission priority relationship.

## Completion truth

The discriminator never labels elapsed time, retry count or cooperative yield as
success.

Each cycle produces a fixed 24-byte completion record **only after the sole
owner's socket send succeeds**.

The record contains:

- magic `R45W`;
- version 1;
- exact variant code;
- completion kind;
- cycle;
- sequence;
- readiness timeout;
- deterministic checksum.

PASS requires all 4096 records in exact cycle/sequence order.

If the PS2 socket becomes readable during the experiment, the harness marks the
apparatus invalid instead of consuming peer bytes as a wake source.

## Silent Pi recorder

R45 adds:

`r45_wake_recorder.py`.

The recorder:

- accepts exactly one PS2 diagnostic TCP connection;
- receives/validates the fixed completion records;
- never calls application `send`, `sendall` or `sendto`;
- records exact completed cycle/sequence evidence;
- reports PASS only after every expected serialized record;
- may report STALL when its measurement timeout expires before the next record;
- never promotes timeout or elapsed time to completion authority;
- reports malformed order, premature EOF and record contract violations as
  `APPARATUS_INVALID`.

Its host regression proves baseline/control record validation, monotonic
sequence, STALL classification, malformed-order rejection and the no-send source
invariant:

`R45_WAKE_RECORDER_TEST=PASS`.

The build authority also records:

`R45_PEER_APPLICATION_BYTES_SENT=0`.

TCP acknowledgement behavior below the application layer is not treated as
peer application wake traffic.

## Pinned discriminator build and exact identities

R45 adds a dedicated PS2 Makefile and deterministic builder:

- `tests/hardware/r45-wake-discriminator/Makefile.ps2`;
- `tests/hardware/r45-wake-discriminator/build.sh`.

The builder uses exactly:

`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

and frozen PS2IP:

`b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

Each variant is built twice at the same path and must compare byte-for-byte
equal before being accepted.

Final baseline discriminator identity:

`R45_BASELINE_ELF_SHA256=ce5b2eda81e2c2c65ff0d0c66f152d2fc0c4621956ef3f6c58aa789ab0fe0976`

`R45_BASELINE_PT_LOAD_SHA256=97a1b02c89c8d72a8c836b5012cbc203a6337914577778245e6f2b547365ef6a`

`R45_BASELINE_PT_LOAD_BYTES=316040`.

Final control discriminator identity:

`R45_CONTROL_ELF_SHA256=8bf21f52f87b9c965fe3dc1dbf75ac59051a2e239915cc62b10cac183d91ebe5`

`R45_CONTROL_PT_LOAD_SHA256=a5fe291785c692c8aabca88e8840e13e7fb343f8210a083d48276ba08f2a315c`

`R45_CONTROL_PT_LOAD_BYTES=316168`.

Machine build facts:

- `R45_BASELINE_REPRODUCIBLE=YES`;
- `R45_CONTROL_REPRODUCIBLE=YES`;
- `R45_VARIANTS_DISTINCT=YES`;
- `R45_HARDWARE_EXECUTED=NO`;
- `R45_DISCRIMINATOR_BUILD=PASS`.

The final CI job preserves both unexecuted ELF artifacts plus
`BUILD-AUTHORITY.env`.

Two intermediate apparatus-build failures were corrected before closeout:

1. variant copies were initially built under different directory names, and
   PS2SDK DWARF embedded those paths, causing a false whole-ELF reproducibility
   mismatch;
2. a shell helper initially printed a literal backslash-n, so its line counter
   misreported a valid PT_LOAD field as absent;
3. an unnecessary assertion required equal baseline/control PT_LOAD byte
   counts, although the authorized control strategy contains an extra branch
   and may legitimately change code size.

All three were apparatus/build-fixture defects only. No product change followed
the initial baseline restoration.

## Canonical product loadable identity restored

The Foreman packet required canonical product loadable bytes to return to the
validated/HW1-failed identity before any later discriminator hardware run.

Final exact pre-log product build evidence:

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`

`PT_LOAD_BYTES=556180`.

Those are byte-for-byte identical to the independently validated product that
failed HW1.

The final whole-ELF hash is:

`ELF_PRISTINE_SHA256=15e3c92f433208d914239754709717ab6608f9c4e233039c2f5abf4b7f80864f`.

It differs from the prior validated whole-ELF hash because retained host-test
source/preprocessor/comment layout affects non-loadable debug metadata under the
PS2SDK `-gdwarf-2 -gz` build. The loadable PT_LOAD identity is exact, which is
the R45 product-byte requirement.

Current-source reproducibility independently rebuilt that product twice and
returned the same ELF and PT_LOAD identities.

## Final canonical machine evidence

Exact pre-log head:

`6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`.

GitHub Actions:

`run=36352489627`, run number 1071, attempt 1, exact head
`6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`, completed SUCCESS.

Jobs:

- host-unit = SUCCESS;
- project-check = SUCCESS;
- dictionary-long = SUCCESS;
- ps2-compile = SUCCESS;
- ps2-link = SUCCESS;
- r45-discriminator-build = SUCCESS;
- dictionary-reconcile = correctly SKIPPED.

Selected exact-head evidence:

- `transport_runtime_test: PASS` — baseline build;
- `transport_runtime_test: PASS` — zero-timeout control build;
- `rfb_async_framing_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `transport_rfb_provider_failure_test: PASS`;
- `transport_audio_test: PASS`;
- `transport_mpeg_test: PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `APP_AUDIO_PRODUCT_TEST=PASS`;
- `APP_AUDIO_PRODUCT_SOURCE_TEST=PASS`;
- `APP_AUDIO_COMPOSITION_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`;
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`;
- `PI_AUDIO_PRODUCT_COMPOSITION_TEST=PASS`;
- `R45_WAKE_RECORDER_TEST=PASS`;
- `R45_WAKE_DISCRIMINATOR_SOURCE_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`;
- `R45_DISCRIMINATOR_BUILD=PASS`.

Final product symbol counts remain unchanged:

- `src:1132`;
- `pi:922`;
- total product symbols: `7349`.

No product dictionary reconciliation was required.

## Changed paths

Relative to the assigning R45 branch head, the final pre-log changes are
confined to the authorized Reconstruction surface:

- `src/transport/runtime.c`;
- `tests/unit/transport_runtime_test.c`;
- `tests/Makefile`;
- `.github/workflows/ledge-reconstruction.yml`;
- `tests/hardware/r45-wake-discriminator/Makefile.ps2`;
- `tests/hardware/r45-wake-discriminator/build.sh`;
- `tests/hardware/r45-wake-discriminator/r45_wake_discriminator.c`;
- `tests/hardware/r45-wake-discriminator/r45_wake_discriminator_source_test.py`;
- `tests/hardware/r45-wake-discriminator/r45_wake_recorder.py`;
- `tests/hardware/r45-wake-discriminator/r45_wake_recorder_test.py`;
- `tests/hardware/r45-wake-discriminator/README.md`.

No ordinary Pi product source changed.

No RFB profile, Wire protocol/version, Application recovery policy, RFB parser
behavior, AUDIO/MPEG/Presentation/media-clock owner, Input/UI semantic,
Configuration/Management source or H1 forensic evidence changed.

## Worker criterion disposition

Worker findings only. Foreman acceptance and any later hardware classification
remain separate.

1. Unaccepted R44 zero-timeout product conclusion removed; default product
   restored to pre-R44 1000-us readiness semantics — MET.
2. Honest deterministic 1000-us host baseline with real fake-PollSema ordering
   returns bounded idle and services post-miss publication without a second
   inbound frame — MET.
3. Standalone PS2 discriminator built on the exact pinned PS2SDK/PS2IP
   networking stack and sole-owner primitives — MET.
4. Exact disputed owner-miss -> publisher -> readiness interleaving represented,
   with publisher made runnable in the gap and no inbound application wake —
   MET.
5. Pi peer accepts/records completion and has no application send primitive —
   MET.
6. Control variant is the same apparatus with zero-timeout/second-poll
   scheduling control and a distinct exact identity — MET.
7. PASS authority is actual sole-owner serialized sequence only; elapsed time
   may classify STALL but never completion — MET.
8. 4096 repeated cycles carry exact cycle/sequence/completion accounting —
   MET.
9. One physical socket owner is mechanically enforced; submitter has no direct
   socket access and no reconnect/watchdog path exists — MET.
10. R44 exact parser/credit/repeated-isolated-update evidence remains enrolled
    and green — MET.
11. Canonical product PT_LOAD is restored exactly to
    `3bf4319f... / 556180` — MET.
12. Host/project/strict dictionaries/pinned PS2 compile/link/current-source
    reproducibility plus both discriminator compile/reproducibility/identity
    checks are green — MET.

R45_SOURCE_COMPLETE=YES_WITHIN_PACKET
R45_PRODUCT_FIX_ACCEPTED=NO
R45_PRODUCT_DEFAULT_READINESS_US=1000
R45_HOST_BASELINE_PROGRESS=PASS
R45_HOST_CONTROL_PROGRESS=PASS
R45_HARDWARE_DISCRIMINATOR_READY=YES
R45_BASELINE_REPRODUCIBLE=YES
R45_CONTROL_REPRODUCIBLE=YES
R45_VARIANTS_DISTINCT=YES
R45_PEER_APPLICATION_BYTES_SENT=0
R45_PRODUCT_PT_LOAD_RESTORED=YES
R45_HOST_TESTED=PASS
R45_PROJECT_CHECK=PASS
R45_STRICT_DICTIONARIES=PASS
R45_PS2_COMPILE=PASS
R45_PS2_LINK=PASS
R45_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R45_HARDWARE_EXECUTED=NO
R45_HARDWARE_QUALIFICATION=NONE

## Hardware boundary and next classification

Reconstruction stops before hardware exactly as required.

No baseline discriminator ELF was launched.

No control discriminator ELF was launched.

No product ELF was launched.

Therefore this shift does **not** classify:

- `BASELINE_STALLS_CONTROL_PASSES`;
- `BASELINE_PASSES_CONTROL_PASSES`;
- `BASELINE_STALLS_CONTROL_STALLS`;
- `APPARATUS_INVALID`.

Those outcomes remain for a later explicitly authorized hardware role.

The existing HW1 product defect remains open until that discriminator evidence is
reviewed and, if authorized, executed.

The separate provider-stager STATIC/enabled apparatus defect remains outside
R45 product scope.

BLOCKERS=NONE

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently review:

- restoration of the canonical product 1000-us scheduling semantics;
- exact loadable identity restoration;
- the honest baseline/control host evidence;
- the one-owner PS2 hardware discriminator;
- the silent Pi recorder;
- exact baseline/control build identities;
- the complete canonical run;
- the explicit no-hardware boundary.

If accepted, the next activity is Foreman-routed focused R45 hardware
discrimination. Reconstruction does not authorize or perform that run.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R45_REVIEW_AND_HARDWARE_DISCRIMINATOR_ROUTING
