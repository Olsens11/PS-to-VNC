# CP2P Item #10 — Public All-Guns Activation

Status: implementation/proof contract. This is **not** a live-hardware qualification seal.

## Exact public composition

The CP2P target now accepts one exact public H1 composition:

- visible RFB mode 2;
- MPEG2 elementary-stream video enabled;
- canonical PCM either enabled or OFF;
- the existing independent RFB/AUDIO/MPEG queue and credit fields;
- one physical PSTV TCP connection and one PS2 receive owner.

The CP2O target remains unchanged: visible RFB + optional PCM with MPEG OFF.

## Activation ownership

CONFIG authorizes and allocates capability only. It does not start MPEG.

The live path remains:

`CONFIG ACK -> calibration accept -> generation N worker arm -> START(N) ->`
`Pi suppression/capture/producer prepare -> exact-generation emission lease -> DATA`

The Pi scheduler opens the #11B emission fence at most once for a generation and
holds `begin_emission_exact()/finish_emission_exact()` around every MPEG
`_send_from()` call. Producer retirement latches a one-way retiring state before
closing admission, so a scheduler race cannot reopen the generation.

## Session accounting

MEDIA_END reports only bytes actually written as MPEG DATA. Per-generation FFmpeg
archives remain evidence, but unsent producer tail discarded at retirement or
session end is not counted as wire data.

The PS2 SESSION_RESULT now carries the real most-recent generation decoder result
instead of the pre-#10 zero placeholder. Transport integrity remains based on
exact MPEG bytes/frames/sequence/CRC received. MPEG bytes consumed may be lower
than bytes enqueued because exact retirement and final session teardown may
intentionally leave/discard residual queued generation bytes.

## Proof boundary

Host/compile proof must establish:

- the CP2O gate still rejects MPEG with visible RFB;
- the CP2P gate accepts visible RFB + MPEG + optional PCM and delegates ordinary
  MPEG/PCM constraints to the unchanged inner validator;
- no MPEG DATA send occurs without the exact generation emission lease;
- a retiring generation cannot be reopened;
- prior START/suppression/capture/producer/retirement contracts remain green;
- the pinned PS2 target links with the CP2P gate and real worker result path.

Live PS2/Pi all-guns qualification remains item #12.
