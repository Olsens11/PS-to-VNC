# O3 P6B Absolute-Clock Telemetry Hardware Result

## Scope

P6B is instrumentation-only relative to P6. It preserves the exact P6 absolute 30000/1001-fps scheduler and adds one post-playback UDP telemetry datagram containing clock counters.

## DUT identity

- Branch: `experiment/media-stream-exp3`
- P6B apparatus commit: `0972062020ebd9c10eac01e87926221e85fcb264`
- Generated source blob: `908b4304d816035c6a6f2d5dda19a6714451342d`
- ELF SHA256: `523ab812971641e397ac4e207df9a349380193fdc2b57c6ce87a65819428c5f2`
- ELF bytes: `2469968`
- Rolling deployment: `/mass/0/PS2VNC-exp3-p6b-clock-telemetry.ELF`

## Exact replay source

- Archive: `build/evidence/exp3-local-x11-3m-control-20260907-062342/local-stream.m2v`
- Bytes: `7732149`
- SHA256: `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`
- CRC32: `ce1e9d03`
- Coded picture starts: `600`
- Source cadence: `30000/1001 fps`
- Queue capacity: `32768`

## Hardware observation

The video completed, but remained visibly non-natural: motion was still somewhat bursty, alternating between faster and slower-looking periods. P6B looked materially like P6, as expected for instrumentation-only behavior.

## Clock telemetry

Post-playback UDP telemetry returned:

- `frame_period_ticks=4920115`
- `vsync_waits=604`
- `deadline_misses=289`
- `max_deadline_late_ticks=196645428`
- `deadline_misses / 598 returned pictures = 48.33%`
- `max_deadline_late_ticks / frame_period_ticks = 39.967649 frame periods`

The P6 scheduler epoch is established immediately before picture 1 presentation, after the qualification-screen holds. Therefore these misses are playback-time misses, not an artifact of the pre-video holds.

## Interpretation

P6 does not drop late pictures. When a picture reaches the scheduler after its absolute deadline, the scheduler records lateness and returns immediately; the stale picture is still drawn. Once the path has accumulated lateness, successive decoded pictures can therefore be drawn rapidly while the absolute clock catches up. This behavior matches the observed fast/slow bursting.

The next isolated experiment should preserve full MPEG decoding while skipping only GS presentation of pictures that are already at least one full source frame period late. A one-frame threshold avoids treating minor VBlank quantization as an obsolete frame. Decode dropping is prohibited because MPEG reference-picture state must continue to advance.

## Harvest apparatus note

The first P6B harvest script stopped after the clock telemetry because `SERVER_LOG` was not restored into its shell environment. That occurred after the clock datagram had already been captured and does not invalidate the telemetry above. No additional transport-integrity claim is made here from that truncated harvest.
