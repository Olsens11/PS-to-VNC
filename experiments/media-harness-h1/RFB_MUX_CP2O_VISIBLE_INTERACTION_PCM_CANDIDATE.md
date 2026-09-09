# CP2O — visible RFB + real Issue-39 interaction + PCM candidate

## Purpose

CP2O is the next narrow H1 hardware-proof checkpoint after CP2N.

It keeps CP2N's hardware-qualified visible RFB, mouse, keyboard, OSK and local-UI
composition unchanged and activates the existing H1 PCM/AUDSRV implementation on
logical channel 2 of the same physical PSTV TCP connection. MPEG remains OFF.

This is deliberately **not** a new audio implementation and **not** an
optimization checkpoint. The cumulative CP2N build already links the canonical
H1 audio runtime. CP2O changes only the minimum activation/coordinator and Pi
producer behavior required to exercise it concurrently with CP2N interaction.

## Inherited authority

- current hardware authority before this checkpoint: CP2N
- RFB mode: visible mode 2
- RFB queue: 32768 bytes
- RFB initial credit: 32768 bytes
- RFB credit batch: 8192 bytes
- RFB flush-on-empty: enabled
- one physical PSTV connection
- real through-Issue-39 controller/input/keyboard/OSK/local-UI modules
- existing clean finite RFB quiesce: REQUEST -> BOUNDARY -> COMMIT -> COMPLETE

## PCM baseline

The first mixed run uses the established H1 PCM path and the newer combined-media
operator baseline:

- PCM: 48000 Hz, signed 16-bit, stereo
- audio queue: 524288 bytes
- audio initial credit: 524288 bytes
- audio credit batch: 4096 bytes
- audio flush-on-empty: enabled
- audio start mode: target prefill
- audio start target: 458752 bytes
- audio chunk: 4096 bytes
- audio idle delay: 1000 us
- audio worker priority: 8
- audio worker stack: 16384 bytes
- receiver priority: 63
- audio volume: 100
- MPEG/video mode: OFF

These are test starting values, not permanent product constants. After mixed
hardware stability is established, h1_tool sweeps may vary them deliberately.

## Pi source condition for HW1

The operator will keep a YouTube video playing in a minimized browser window.
This provides real continuous desktop audio while keeping RFB visual activity
comparatively low. The browser must remain minimized throughout the baseline
mixed session.

A later, separate stress session may maximize the same video. That is intentionally
not part of HW1 because maximizing changes the RFB workload and should remain a
single controlled variable.

## Resident ELF behavior

The CP2O ELF remains resident and multi-session. It accepts:

1. visible-RFB-only CP2N-equivalent sessions; and
2. visible-RFB + PCM sessions with MPEG OFF.

After a normal session ends, the ELF returns to its connection retry loop and
should reconnect automatically when the next h1_tool listener appears. Relaunch
is only required after a console reset or actual ELF exit.

## Canonical operator surface

All sessions are driven through `experiments/media-harness-h1/h1_tool.py`.
The CP2O runner is:

`experiments/media-harness-h1/h1_mux_server_cumulative39_rfb_pcm_bridge.py`

The first hardware sequence is intentionally two sessions against one unchanged
ELF:

### A. CP2N regression control

- audio OFF
- MPEG OFF
- visible RFB mode 2
- short finite session
- confirm live desktop and representative interaction still work
- clean finite quiesce and result validation required

### B. CP2O mixed baseline

- same resident ELF
- PCM ON with the baseline above
- MPEG OFF
- minimized YouTube continues playing
- confirm uninterrupted audible audio, live RFB desktop, mouse and OSK/keyboard
  interaction throughout the finite session
- clean finite quiesce, exact PCM accounting/CRC, integrity pass and zero
  transport error required

## Qualification rule

CP2O remains hardware-unqualified until both machine evidence and operator
observation pass on the exact pinned candidate ELF/PT_LOAD identity.

Cursor sluggishness already observed in CP2N is a performance note and does not
by itself fail CP2O. New freezes, audio stalls/shudder, lost interaction,
malformed quiesce, integrity failure, transport error, or incorrect PCM accounting
do fail the candidate.

Do not optimize queue sizes, priorities, credit policy or RFB presentation during
HW1. Preserve any failure as evidence first. Optimization/profile sweeps begin
only after a stable mixed baseline exists.
