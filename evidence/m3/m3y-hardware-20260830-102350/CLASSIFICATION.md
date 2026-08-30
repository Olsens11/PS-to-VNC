# M3Y hardware classification

## DUT

M3X eleven-object calibration split.

## Attempt 1

Result: FAIL.

The first five-mode hardware run passed 480i, 480p-hires, 576i,
and 720p. During the final 1080i -> 480p restore, the visible output
became almost entirely green except near the bottom.

Machine control reached RESTORING but never reached RESTORED, and the
harness timed out.

This failure remains part of the permanent evidence. It is not erased
or reclassified as a pass.

## Persistent control aftermath

The failed display-control record remained durably stored as
RESTORING / txid=1 / mode=1080i / baseline=480p.

That stale control record survived:

- restart of ps2vnc-management.service;
- reboot of the Raspberry Pi;
- reboot of the PlayStation 2.

The stale record prevented a new independent display-control test from
starting until the exact abandoned display-control state file was
archived and explicitly cleared.

This persistence problem is classified as an aftermath/recovery-design
defect, not as an additional M3X hardware failure.

## Attempt 2

After full Pi and PS2 reboot, archival and explicit removal of the
abandoned display-control record, the exact same M3X ELF was tested
again with the exact same five-mode harness parameters.

Machine result: 5/5 PASS.

Physical result: FULL PASS.

The final 1080i -> 480p restore passed visibly and mechanically.

## Classification

The available evidence does not establish a deterministic M3X
regression.

Attempt 1 is classified as an intermittent display-transition
liveness failure observed during 1080i -> 480p restoration.

Its exact root cause remains unresolved.

M3X is accepted as hardware-qualified for continued source migration,
with the intermittent failure retained as an explicit qualification
caveat.

## Deferred recovery requirement

A PS2 runtime may occasionally hang or freeze under extraordinary
conditions. A fresh PS2 session must not inherit an abandoned
display-control transaction from the previous session.

Future recovery/control-plane work should introduce session-generation
or equivalent stale-state reconciliation so an old RESTORING lease
cannot poison a newly booted PS2VNC session.

This recovery improvement is deliberately deferred and does not block
the current source migration.
