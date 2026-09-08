# H1 RFB channel-1 credit policy decision packet

Status: **DECIDED — CANDIDATE A SELECTED BY OPERATOR 2026-09-08**

This record resolves the evidence and choices around the only policy ambiguity
left after CP2D. David explicitly selected the fully configurable policy: RFB is
not exempt from H1's per-channel credit controls and must retain enough runtime
knobs to support thorough hardware optimization without rebuilding the ELF.

## Invariants already decided

The following are not part of this decision:

- one PS2-facing PSTV TCP connection only;
- RFB uses logical channel 1;
- the H1 receiver thread remains the sole physical `recv()` owner;
- outbound RFB bytes use H1's existing serialized send owner;
- through-Issue-39 RFB parser/session semantics remain unchanged;
- RFB queue storage is independent from AUDIO and MPEG;
- first controlled RFB queue capacity starts at 32768 bytes, matching the
  through-Issue-39 direct-RFB receive-prefetch precedent;
- queue exhaustion is an observable failure, not permission to silently grow
  the queue;
- first hardware milestone is RFB-only, with AUDIO and MPEG disabled;
- Issue #40 is outside this experiment.

## Existing H1 credit precedent

AUDIO and MPEG already use receiver credit with four independent profile
choices per enabled channel:

1. initial credit bytes;
2. credit-return enabled/disabled;
3. returned-credit batch size;
4. flush pending credit when the logical queue becomes empty.

The qualified P11-compatible H1 profiles currently grant initial credit equal
to the full queue capacity. MPEG uses an 8192-byte return batch and flushes on
empty. PCM uses a 4096-byte return batch and flushes on empty. Both return
credit as bytes are consumed rather than merely as DATA frames arrive.

## Selected policy — explicit RFB CONFIG knobs

Add RFB counterparts to the existing H1 credit vocabulary:

    rfb_queue_capacity
    rfb_credit_batch_bytes
    rfb_credit_flush_on_empty
    rfb_credit_return_enabled
    rfb_initial_credit_bytes

These are session/profile parameters, not hidden RFB constants. They must be
visible through `h1_profiles.py`, the CONFIG wire payload, `h1_tool.py knobs`,
raw `--set`, and `sweep --vary` just like the existing AUDIO/MPEG controls.

First RFB-only profile values:

    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1

The 32768-byte starting queue is evidence-based from the qualified direct-RFB
prefetch path. The 8192-byte batch is a controlled first hypothesis because it
matches current `max_data_payload`, is one quarter of the starting queue, and
matches the MPEG return batch. Flush-on-empty avoids stranding credit after a
small interactive update. None of these defaults are claims of optimal or
required hardware values; the point of making them CONFIG knobs is to test that
rather than assume it.

## Validation contract

When RFB is OFF:

- `rfb_queue_capacity` and `rfb_initial_credit_bytes` must be zero;
- no RFB queue allocation, semaphore, DATA dispatch, CREDIT, or parser activity
  may occur.

When RFB is ON:

- queue capacity must be at least `max_data_payload` and representable by the
  allocator/runtime;
- initial credit may not exceed queue capacity;
- if credit return is enabled, batch bytes must be nonzero and no larger than
  queue capacity;
- `rfb_credit_flush_on_empty` and `rfb_credit_return_enabled` are boolean;
- no arbitrary upper queue ceiling should be added beyond representability and
  allocation success; failed allocation is evidence;
- queue exhaustion remains an explicit failure/telemetry event, never an
  implicit request to enlarge the queue.

## Why this was selected

The H1 build is a laboratory control surface. RFB must not become a special
case with hidden transport policy while AUDIO and MPEG remain independently
tunable. Making the RFB queue and credit policy explicit lets later hardware
sweeps distinguish transport scheduling, queue pressure, parser consumption,
and presentation behavior without another source/ELF change for every trial.

This also preserves the operator-tool goal established earlier in the session:
routine testing should be declarative and reproducible rather than a mountain
of bespoke commands.

## Next implementation checkpoint

1. Expand H1 CONFIG/profile vocabulary with the five selected RFB fields.
2. Preserve RFB-OFF inertness and keep `rfb_mode=ON` rejected until the expanded
   wire/profile/runtime plumbing is build-verified.
3. Make dormant RFB resource allocation consume `rfb_queue_capacity` rather
   than a compile-time-only capacity.
4. Wire channel-1 DATA dispatch, consumed-byte credit batching/flush, and
   outbound channel-1 DATA through the existing H1 owners.
5. Activate the mux-backed RFB I/O adapter and Pi VNC bridge.
6. Only then advertise RFB capability and accept `rfb_mode=ON` for the first
   RFB-only hardware milestone.

The first hardware test remains RFB only: AUDIO OFF, MPEG OFF. Hybrid graphics
presentation is still deferred until RFB-over-mux transport is independently
qualified.
