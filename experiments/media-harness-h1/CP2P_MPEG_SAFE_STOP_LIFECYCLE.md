# CP2P MPEG Safe-Stop Lifecycle

## Status

Hardware-proven on 2026-09-13 using:

- source: `bde3151651972d99920a12104e0d2db16dd2e9ac`
- ELF SHA-256:
  `0f2d6b495c96bd33014a36b61b7f64fb553ad6c663013e83f46bffadb424ec78`
- profile: `CP2P_ALL_GUNS`
- diagnostic stop delay: 2 seconds after `MPEG_OWNED`

Observed result:

- MPEG stopped and its rectangle was reclaimed;
- PCM continued without teardown shudder;
- full-screen RFB redrew and continued normally;
- receiver, PCM, RFB, and heartbeat telemetry continued after MPEG retirement;
- the decoder release path reached its final post-release witness.

## Proven lifecycle rule

An asynchronous CP2P lifecycle stop must **not** be converted into a false
end-of-stream return from inside libmpeg's data callback while `MPEG_Picture()`
is active.

The qualified ordering is:

1. request local MPEG stop while the Pi producer remains live;
2. continue supplying ordinary MPEG data to the active `MPEG_Picture()` call;
3. observe the stop only after `MPEG_Picture()` returns to project code;
4. destroy decoder ownership and restore local IPU/DMAC state;
5. observe the MPEG worker as finished;
6. retire the exact Pi producer generation;
7. wait for the exact retirement ACK;
8. join/delete the already-finished worker, clear MPEG pixels, and finalize the
   old MPEG queue;
9. return presentation ownership to `RFB_ONLY` and perform the required full RFB
   refresh.

This rule also applies to ordinary local worker shutdown: a stop flag is a
request to terminate at a decoder-call boundary, not permission for the data
callback to synthesize lifecycle EOF.

## Why

The failed path let `stop_requested` interrupt the transport read used by
libmpeg's data callback. Returning zero from that callback while an active
picture decode needed data caused a global PS2 loss of forward progress.

A diagnostic that parked the worker immediately before that callback returned
zero kept PCM and RFB healthy. Moving stop observation out of the callback and
to the completed-picture boundary made the complete MPEG teardown and return to
RFB+PCM succeed.

## Provenance

This implementation is an independently written project lifecycle policy.
Mature PS2 implementations were used to identify the behavioral pattern:
control/abort is handled at decoder ownership boundaries rather than by copying
their source or control-flow expression.

No SMS player lifecycle implementation, Sony `sceMpeg` implementation, or
commercial-game source is copied here.

The repository's pre-existing vendored SMS libmpeg dependency remains governed
by its existing license and provenance; this safe-stop policy creates no new
source-copy dependency on the player code used for behavioral comparison.

## Separate unresolved issue

This result fixes the teardown/global-freeze defect. It does **not** by itself
resolve the independently observed spontaneous active-session MPEG read stall.
That remains a separate work item and must not be conflated with safe shutdown.
