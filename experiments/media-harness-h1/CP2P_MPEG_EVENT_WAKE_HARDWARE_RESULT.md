# CP2P MPEG event-wake hardware result

## Status

**ACTIVE-RUNTIME HARDWARE QUALIFIED. FULL SESSION LIFECYCLE NOT YET QUALIFIED.**

On 2026-09-13 the PS2 sustained concurrent interactive RFB, PCM audio, and
MPEG-2 video for the full 600-second all-guns confirmation workload after the
normal MPEG empty-queue wait was changed from timer-backed polling to
receiver-driven `SleepThread()` / `WakeupThread()` synchronization.

This is a major H1 experimental checkpoint, not the final project milestone.

## Exact authority

Branch:

`experiment/h1-cp2p-mpeg-data-wakeup-fix`

Source head:

`d85e896093f9f22be5fb9ff8feb464c01b7cd0a9`

PS2 ELF:

`mass:/0/mpegevent.elf`

ELF SHA-256:

`c0181f19e0819b7e701b01c56d746d09f4684d810c6fe0684c8acf34e2e4f461`

ELF size:

`3259400` bytes

Confirmation evidence:

`/home/ps2/ps2vnc-evidence/h1-cp2p-mpeg-eventwake-confirm-600s-20260913T223333Z`

## Confirmation profile

The 600-second confirmation retained the diagnostic stress split:

- RFB queue capacity: 524288
- RFB initial credit: 32768
- RFB credit batch: 8192
- PCM queue / initial credit: 524288 / 524288
- MPEG queue / initial credit: 524288 / 524288

RFB retained its existing timer-backed polling path and therefore remained a
control. The normal MPEG empty-queue path contained no timer alarm.

## Why event wake was tried

Earlier hardware diagnostics independently caught:

- RFB stopped at its ordinary exact-read `DelayThread()` retry while receiver,
  PCM, MPEG and Pi transport activity continued;
- MPEG stopped at its ordinary empty-queue `DelayThread()` retry while receiver,
  PCM and RFB continued; and
- a later MPEG wake/sleep experiment that still used `SetTimerAlarm()` stopped
  after successful alarm setup but before the timer callback appeared.

These results did not prove a universal PS2SDK timer defect, but they justified
removing timer-backed polling from the MPEG consumer hot path.

The receiver-driven candidate publishes the sole MPEG consumer's EE thread id
while queue ownership prevents a lost DATA race. The receiver wakes that waiter
after committing MPEG DATA. The EE pending-wakeup count covers wake-before-
sleep ordering.

## 600-second result

At the duration boundary:

- MPEG sent: 113876965 bytes
- MPEG enqueued: 113876965 bytes
- MPEG consumed: 113876965 bytes
- MPEG reads: 64049
- PCM consumed: 114614272 bytes
- PCM played: 114610176 bytes
- send attempts / completions: 68518 / 68518
- send failures: 0

Interactive RFB, PCM, MPEG presentation, receiver progress and Pi send progress
remained alive for the complete active workload.

The previous bounded MPEG queue-fill / dead-consumer signature did not recur.

## Finite shutdown remains open

After the 600-second workload, RFB clean quiesce completed through:

`REQUEST -> BOUNDARY -> COMMIT -> COMPLETE`

The Pi then retired the MPEG producer and sent ordinary `MEDIA_END`.

No PS2 `SESSION_RESULT` returned before the Pi timeout.

Therefore:

- 600-second concurrent active runtime: PASS
- RFB finite quiesce: PASS
- Pi MEDIA_END transmission: PASS
- PS2 SESSION_RESULT completion: OPEN / FAIL
- automatic successive H1 sessions: NOT YET QUALIFIED

This lifecycle defect is the immediate next debugging target because it blocks
reliable successive H1 test execution.

## Source-forensics correction

Do not interpret:

`frames_received = receiver_loop_count + 1`

as evidence that the final frame itself was stuck in dispatch.

CONFIG is counted in `frames_received` before the asynchronous receiver loop
begins, so the +1 relationship is expected.

`h1_accept_end()` sets `end_received` before waking the MPEG waiter and setting
`receiver_done`. The existing evidence does not yet prove exactly where the
post-MEDIA_END lifecycle stopped.

## Remaining pre-ledge qualification

Before this experiment is considered ready to turn back toward the clean
post-Issue-39 main project, the remaining gate includes:

1. clean MEDIA_END -> SESSION_RESULT -> repeat-session lifecycle;
2. removal/qualification of the remaining RFB timer-backed polling;
3. end-to-end MPEG calibration through the real UI;
4. matte/RFB-suppression alignment across calibration changes;
5. stress across small, large, awkward and edge-positioned MPEG geometries;
6. repeated stop/recalibrate/relaunch generations;
7. cursor rendered above MPEG;
8. correct OSK/local-UI composition above MPEG; and
9. non-default-geometry all-guns endurance.

Noticeable A/V latency is recorded for a later optimization pass unless it
becomes a functional usability blocker.

## Scope discipline

This H1 campaign remains an experiment launched from the post-Issue-39 /
pre-Issue-40 project frontier. It does not itself start Issue #40.

When the experiment reaches its stable ledge, proven features and architectural
knowledge are intended to return to the clean main-project line selectively.
Experimental branches, diagnostics and worktrees remain evidence until that
handoff; they should be archived to verified cold storage before later pruning.
