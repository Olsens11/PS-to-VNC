# Pi MPEG generation control and producer lifecycle

## Purpose

This note records the maintained R17 Raspberry Pi boundary that consumes the
already-accepted PS2 MPEG START/RETIRE control representation.

The central rule is:

> **The generation owner owns MPEG behavior; WireConnectionOwner owns physical
> Wire I/O and sequence.**

R17 does not create another socket reader/sender and does not move global PSTV
sequence ownership out of the Wire server.

## Exact control representation

The maintained Pi codec mirrors `src/transport/protocol.*`:

- START: frame kind 11, control channel 0, flags 0, 44-byte payload;
- RETIRE: frame kind 10, control channel 0, flags 0, 12-byte payload;
- generation-control payload version 1;
- all fields are big-endian uint32;
- MPEG DATA: ordinary DATA kind 3 on logical channel 4, non-empty and no larger
  than the accepted PSTV payload maximum;
- MPEG CREDIT: ordinary CREDIT kind 4 on logical channel 4 with one nonzero
  big-endian uint32 amount.

This extension changes neither fixed PSTV header version 1 nor Q4 product
compatibility version 2.

## Exact generation authority

One `MpegGenerationController` belongs to one active Wire Session. A START is
valid only when its session identity matches, generation is nonzero and newer
than every generation previously admitted by that controller, and its geometry
is valid against current injected desktop dimensions and Configuration limits.

The START base rectangle is the capture rectangle. The START outer rectangle is
the generation-specific suppression footprint. They are intentionally distinct:
inner matte remains a PS2 rendering concern.

Only one generation may be PREPARING, LIVE, RETIRING, or awaiting RETIRE
completion. A dead generation is never rebound.

## Configuration projection

`src/config/mpeg_runtime_profile.json` is the machine-readable form of the
already-selected A003 R7 MPEG profile. The C runtime consumes generated constants
with the same prior values, while the Pi projection carries only facts needed by
R17: channel window, producer buffer bound, maximum capture dimensions and frame
rate.

`scripts/generate-mpeg-runtime-profile.py --check` fails closed when either
projection is stale.

R17 does not import H1 bitrate, GOP, VBV, timeout, or other laboratory tuning as
new product authority.

## Producer and backpressure

`BufferedMpegProducer` owns one FFmpeg subprocess and one reader thread. Its
stdout buffer cannot exceed the projected Configuration capacity. A full buffer
blocks that local reader rather than accumulating unbounded bytes.

The producer exposes bytes to the generation controller only. It cannot access
the PS2 socket and cannot allocate Wire sequence.

Channel-4 credit is session-scoped, matching PS2 Transport's one initial grant
plus credit returns. The controller consumes that authority only when it admits
one physical-send lease. RFB credit remains separate.

## Retirement proof

RETIRE has this ordered meaning:

1. exact session/generation match is required;
2. state moves LIVE -> RETIRING, closing new emission admission;
3. every already-admitted emission lease leaves the sole physical-send call;
4. the producer process and reader thread are proven retired;
5. the exact suppression preparation is retired;
6. only then may the Wire owner serialize RETIRE completion;
7. after successful completion serialization, local generation state returns to
   IDLE while the monotonic stale-generation fence remains.

A deadline may reveal a stuck lease/process/thread. Expiration is failure, never
proof of retirement. Failed producer ownership cannot be converted into a
successful EOF/completion.

## Suppression boundary

R17 owns and validates the exact generation-specific suppression footprint, but
does not modify the maintained RFB attachment/relay implementation. The default
Pi composition supplies no MPEG factory, so the mechanism is dormant until the
later final Application composition binds the prepared footprint into its
cross-domain presentation/RFB policy.

This keeps R17 inside its authorized Pi MPEG mechanism scope and prevents a
hidden RFB redesign.

## Evidence boundary

Host tests prove codec symmetry, malformed rejection, geometry/session/generation
validation, bounded credit, producer launch/retirement failure, in-flight lease
drain, no timeout-as-success, fresh successor authority, and sole-Wire
serialization. Canonical project/dictionary/PS2 build checks remain required.

These are source/machine facts, not physical PS2/Pi qualification.
