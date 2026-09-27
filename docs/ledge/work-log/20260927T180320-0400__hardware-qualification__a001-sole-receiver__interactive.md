# Ledge hardware qualification work log — R45 focused wake discriminator HW2

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T18:03:20-04:00
COMPLETED_AT=2026-09-27T18:59:02-04:00
ROLE_KEY=hardware-qualification
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=24a87535e5a644969fe8e252a360ac9f3dec93f2
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Execute only Foreman packet `A001-R45-FOCUSED-WAKE-DISCRIMINATOR-HW2`.
This shift did not perform Reconstruction, Foreman acceptance, independent
Validation, or PS2VNC product qualification.

Authority consumed:
- branch `ledge/h1-all-guns` at `24a87535e5a644969fe8e252a360ac9f3dec93f2`;
- Foreman State revision 0089;
- R45 apparatus authority `6c27ce55123476bc139be7bf2d9bbe6f5202f2c3`;
- R45 Reconstruction closeout `c5d75441af7fb0695f76f1fd1d6462b6a9ee6300`;
- work-log contract revision 0007.

The packet forbade product-ELF execution, product-source mutation,
apparatus-source mutation during measurement, and treating any discriminator
outcome as product qualification. Remote branch authority was refreshed
immediately before this record and remained exactly `24a87535...`.

## Exact apparatus identities

A clean replay-safe rebuild used pinned image
`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`
and frozen PS2IP SHA256
`b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

Baseline `BASELINE_1000US`:
- ELF SHA256 `ce5b2eda81e2c2c65ff0d0c66f152d2fc0c4621956ef3f6c58aa789ab0fe0976`;
- ELF bytes `2212280`;
- PT_LOAD SHA256 `97a1b02c89c8d72a8c836b5012cbc203a6337914577778245e6f2b547365ef6a`;
- PT_LOAD bytes `316040`;
- readiness timeout `1000 us`.

Control `CONTROL_ZERO_TIMEOUT`:
- ELF SHA256 `8bf21f52f87b9c965fe3dc1dbf75ac59051a2e239915cc62b10cac183d91ebe5`;
- ELF bytes `2212624`;
- PT_LOAD SHA256 `a5fe291785c692c8aabca88e8840e13e7fb343f8210a083d48276ba08f2a315c`;
- PT_LOAD bytes `316168`;
- readiness timeout `0 us`.

Both variants reproduced the exact Foreman-accepted identities before hardware
launch.

## Deployment and isolation

PS2Net/ps2ftpd returned exact banner `220 ps2ftpd ready.`.
Generic TestKit deployment/readback used dedicated diagnostic targets:

- baseline rolling: `/mass/0/R45-WAKE-BASELINE-1000US.ELF`;
- baseline archival: `/mass/0/PS2VNC-R45-WAKE-HW2-BASELINE-ce5b2eda.ELF`;
- control rolling: `/mass/0/R45-WAKE-CONTROL-ZERO.ELF`;
- control archival: `/mass/0/PS2VNC-R45-WAKE-HW2-CONTROL-8bf21f52.ELF`.

Both rolling and archival readbacks matched their exact local ELF hashes.
`/mass/0/PS2VNC.ELF` was not used or overwritten.

The peer listened only on TCP `192.168.50.1:5961`. Before each run there was
no stale R45 recorder, listener, or 5961 session.

## Baseline result

The operator launched exactly `mass:/0/R45-WAKE-BASELINE-1000US.ELF`.

Result:
- PASS;
- completed cycles `4096/4096`;
- last sequence `4096`;
- serialized bytes `98304`;
- completion authority `ACTUAL_SERIALIZED_OUTBOUND_SEQUENCE`;
- recorder elapsed `4.568638747092336` seconds.

Every record was independently checked as exact cycle/sequence `1..4096`,
variant `BASELINE_1000US`, timeout field `1000`.

Baseline pcap:
- SHA256 `8ab77b144883e72bd7289994c62f4530366476b5a83170957ff788a8a4dbab3c`;
- 6348 packets captured / 6348 received by filter / 0 dropped;
- PS2→Pi TCP payload bytes on 5961: `98304`;
- Pi→PS2 TCP application payload bytes on 5961: `0`.

Thus the real PS2 1000-us baseline completed the disputed sole-owner
publication/serialization interleaving without future inbound peer application
data.

## Reset / control separation

After baseline evidence was sealed, the operator reset the PS2, returned to
wLaunchELF, restarted PS2Net/ps2ftpd, and reported FTP running again.
The Pi then proved the FTP banner again, no stale R45 recorder, no 5961
listener/session, and the exact control ELF still present. A fresh recorder and
fresh capture were then armed.

## Control result

The operator launched exactly `mass:/0/R45-WAKE-CONTROL-ZERO.ELF`.

Result:
- PASS;
- completed cycles `4096/4096`;
- last sequence `4096`;
- serialized bytes `98304`;
- completion authority `ACTUAL_SERIALIZED_OUTBOUND_SEQUENCE`;
- recorder elapsed `0.29622861300595105` seconds.

Every record was independently checked as exact cycle/sequence `1..4096`,
variant `CONTROL_ZERO_TIMEOUT`, timeout field `0`.

Control pcap:
- SHA256 `7fe35aa47f579a5eb84a1d03195e171e2b542bde2fc5c7d2e1b46e48929fde4d`;
- 118 packets captured / 118 received by filter / 0 dropped;
- PS2→Pi TCP payload bytes on 5961: `98304`;
- Pi→PS2 TCP application payload bytes on 5961: `0`.

## Authorized pair classification

`CLASSIFICATION=BASELINE_PASSES_CONTROL_PASSES`.

Per Foreman State revision 0089:
`R44_WAKE_RACE_THEORY_NOT_SUPPORTED_BY_FOCUSED_HARDWARE`.

The real PS2 1000-us baseline completed all 4096 owner-serialized records, and
the zero-timeout control also completed. This focused experiment therefore does
not support the R44 readiness/wake theory as the explanation for sealed HW1.
It does not identify HW1 root cause or choose a permanent Transport design.

## Product qualification remains unchanged

- `HW1_PRODUCT_DEFECT_OPEN=YES`;
- `PRODUCT_FIX_ACCEPTED=NO`;
- `PRODUCT_HARDWARE_QUALIFICATION=FAIL_PRODUCT_DEFECT`;
- `PRODUCT_HARDWARE_RETEST_AUTHORIZED_BY_THIS_RESULT=NO`;
- `QUALIFY_NATIVE_PI_DESKTOP_RFB_PATH_REPRODUCIBILITY=PARTIAL_NOT_SATISFIED`.

The independently validated/HW1-failed product PT_LOAD remains
`3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`
with `PT_LOAD_BYTES=556180`. No PS2VNC product ELF was launched during HW2.

## Preserved evidence

Tracked package:
`evidence/ledge/ledge-r45-wake-hw2-hardware-discriminator/`.

It contains both exact discriminator ELFs, build/PT_LOAD authority,
deployment/readback evidence, independent recorder JSONL streams and summaries,
both pcaps and tcpdump logs, per-run result files/checksums, operator
observations, pair classification, pair-level `HARDWARE-AUTHORITY.env`, and
package-wide `SHA256SUMS.txt`.

Both pcaps have Git attribute `text: unset`, no content filter, and identical
filtered/no-filter Git hashes.

A pre-hardware connector interruption left the evidence directory partially
created once. It was recovered without deleting evidence; no measurement had
started, and a clean replay-safe rebuild was performed before deployment.
A later shell-heredoc quoting error mangled only the first uncommitted draft of
this log; the draft was replaced before commit. Neither event changed source,
apparatus source, or hardware evidence.

## Repository checks

Final pre-commit checks:

- `python3 scripts/work-log-check.py`:
  `WORK_LOG_CHECK=PASS records=264 grandfathered=9 format_compat=2 stamp_compat=1`;
- `git diff --check`: PASS;
- `./scripts/check.sh`: `PS_TO_VNC_PROJECT_CHECK=PASS`;
- canonical dictionary scan reported the existing advisory
  `SOURCE_DICTIONARIES=ATTENTION`, `ATTENTION_COUNT=62`, with
  `CHECK_CONTINUES=YES`.

No product source, apparatus source, Pi product source, systemd unit, build
script, test source, dictionary, or Foreman state was modified by this role.

## Exact next pickup

Return the baton to the Foreman.

Foreman should consume `BASELINE_PASSES_CONTROL_PASSES`, keep the sealed HW1
product defect open, and packetize deeper PS2 product-progress tracing rather
than accepting the R44 zero-timeout scheduling candidate on this evidence.

NEXT_PICKUP=FOREMAN_DEEPER_PS2_PROGRESS_TRACING
