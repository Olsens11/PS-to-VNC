# PS2 media-clock product binding

## Status

    OWNER_SPLIT=CONFIGURATION_PLUS_MEDIA_PLUS_PS2_PLATFORM
    PACKET=A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26
    MEDIA_CLOCK_MECHANISM_OWNER=src/media/clock.*
    SELECTED_PROFILE_OWNER=src/config/media_clock_profile.*
    PS2_BINDING_OWNER=src/platform/ps2_media_clock.*

This note records the current product boundary that makes the already-accepted
A002 common media clock composable on PlayStation 2 without duplicating the
clock itself.

## Ownership

`src/media/clock.*` remains the sole owner of the session epoch, one-shot arm,
signed offset conversion, saturating deadline arithmetic, and wait policy. R26
does not move or duplicate any of those responsibilities.

`src/config/media_clock_profile.*` owns only the selected immutable product
values:

- epoch lead: 0 microseconds;
- audio presentation offset: 0 microseconds;
- video presentation offset: 0 microseconds.

The selected function returns a copy, so caller mutation cannot alter
Configuration-owned authority. These values are internal selected product
profile facts, not new PSTV CONFIG fields and not a resurrection of H1's
laboratory configuration vocabulary.

`src/platform/ps2_media_clock.*` owns only PS2-specific mechanism adaptation:

- one session-owned EE semaphore used as the media clock's lock/unlock context;
- direct `GetTimerSystemTime()` observation;
- exact `kBUSCLK` tick-rate publication;
- exact requested-microsecond delegation to `DelayThread()`.

The adapter contains no epoch, armed flag, deadline, offset, media generation,
AUDIO, or Application policy.

## Session lifetime

A binding object has a strict `NEW -> ACTIVE -> RETIRED` lifetime. Successful
initialization creates one semaphore with initial/max counts 1/1. Failed
initialization publishes no usable synchronization or time observers.

Release revokes the binding locally before kernel deletion is attempted. Thus a
stale observer copy from Session A cannot regain authority even if
`DeleteSema()` itself reports failure. A released binding is terminal; Session
B is represented by a fresh binding object and a freshly created semaphore.

This is deliberately different from a reusable process-global lock identity.
The Platform object carries no clock state that could migrate from one session
to another.

## Recovered PS2 timing authority

Frozen H1 commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845` records the qualified/all-guns
timer mechanism in
`experiments/media-harness-h1/h1_media_clock.c`:

- current time from `GetTimerSystemTime()`;
- conversion/tick domain based on `kBUSCLK`;
- bounded thread yielding via `DelayThread()`.

R26 retains only those PS2 mechanism facts. H1's volatile/EE_SYNCL epoch
representation is not copied; the clean A002 media owner instead uses its
existing injected synchronization contract.

## Composition boundary

R27 now consumes the accepted R26 product boundary in ordinary Application
session startup.

Before platform startup, product entry resolves
`pstvnc_config_media_clock_profile_selected()`. After one MPEG-capable
Transport session has been admitted, each physical attempt creates a fresh
`pstvnc_ps2_media_clock_binding_t`, obtains its
`pstvnc_media_clock_sync_t` and exact `kBUSCLK` tick rate, and initializes
one clean `pstvnc_media_clock_t` from the selected profile.

That composition remains dormant. R27 does not request time-ops/current-tick
service because no media deadline is active yet, and it never calls
`pstvnc_media_clock_arm()`. The accepted first synchronized MPEG presentation
remains the future arm boundary.

Provider-local replacement retires the exact session binding before Transport
retirement and before successor connection admission. Release failure blocks
replacement. A successor attempt constructs fresh binding and clean clock
authority rather than resetting/reusing the old session object.

R26 itself remains a mechanism/profile packet; R27 is the first ordinary caller.
Neither packet chooses a controller gesture, starts MPEG/AUDIO, or changes
Wire/protocol bytes.

## Qualification

R26 adds linked PS2 Config/Platform code, so its PS2 loadable identity must be
treated as new hardware-pending authority until separately qualified. Host
tests and repository reproducibility prove source/build behavior only; they do
not transfer physical qualification from earlier PT_LOAD bytes.

Context: `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md`;
`docs/ledge/LEDGE_FOREMAN_STATE.md`.
