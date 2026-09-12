# CP2P Pi START Receive + Validation — Item #5

## Scope

This checkpoint closes scoreboard item **#5 — Pi START receive + validation** at the pre-MPEG-production software boundary.

The Pi now recognizes the PS2's already-defined CP2P MPEG START control message on the existing physical PSTV session, validates it against the current session and active desktop, and retains one immutable prepared generation for later activation work.

This checkpoint intentionally does **not** activate MPEG production.

## Transport ownership

The transport architecture remains unchanged:

- one physical PSTV TCP connection;
- one Pi-side `H1Session.reader()` call chain owns PS2->Pi frame reception;
- the existing RFB adapter continues to consume only logical channel 1 on that same call chain;
- the CP2P START receiver wraps the already-active receive owner after RFB attachment and consumes only PSTV `DATA` on logical MPEG2 channel 4;
- no second PS2-facing socket, receive thread, frame-sequence owner, or transport reader is introduced.

Frames that are not CP2P START continue through the pre-existing dispatch path unchanged.

## Wire authority

The Pi consumes the fixed PS2 START representation already established by `mpeg_presentation_calibration/h1_mpeg_start_wire.{h,c}`:

- PSTV frame kind: `DATA`;
- logical channel: MPEG2 / channel 4;
- flags: zero;
- payload length: 44 bytes;
- eleven unsigned 32-bit words, big-endian:
  1. version;
  2. session id;
  3. generation;
  4. base/draw x;
  5. base/draw y;
  6. base/draw width;
  7. base/draw height;
  8. suppression x;
  9. suppression y;
  10. suppression width;
  11. suppression height.

Wire version is currently `1`.

## Validation performed on the Pi

A START request is rejected unless all of the following are true:

- payload size is exactly 44 bytes;
- version is exactly 1;
- session id exactly matches the active H1 CONFIG/session id;
- generation is nonzero;
- every geometry word fits the signed-int range used by the PS2 representation;
- base width and height are each at least 16 pixels and 16-pixel aligned;
- suppression width and height are positive;
- the suppression rectangle fully contains the exact base rectangle;
- the exact base rectangle is inside the active Pi desktop;
- the suppression rectangle is inside the active Pi desktop.

The active desktop authority remains the existing Pi-only profile metadata (`desktop_width`, `desktop_height`). No new transport CONFIG field or synthetic profile name is added.

## Prepared-generation state

Successful validation creates one frozen Pi-side prepared request containing:

- exact session id;
- exact generation;
- exact base rectangle;
- exact suppression rectangle.

Only one request may remain prepared at a time. A second START is rejected until the exact prepared generation is released by later activation/lifecycle work. Once a generation has been prepared, that generation or any older generation is stale and cannot later replace it.

This is intentionally a small seam rather than a producer lifecycle implementation. The exact-generation release operation exists so later work can hand the prepared request to the producer/suppression stage without mutating generation identity.

## Evidence

When a START is accepted, the runner writes:

`mpeg_start_prepared.json`

inside the normal H1 evidence directory and prints `H1_CP2P_MPEG_START_PREPARED=...` with the same immutable fields.

## Explicit non-goals of item #5

This checkpoint does not:

- install RFB source suppression;
- change RFB update geometry;
- mutate X11 capture coordinates;
- launch or wake an MPEG producer;
- send MPEG DATA to the PS2;
- add or run the PS2 concurrent MPEG worker;
- open the final all-guns CONFIG gate;
- claim hardware qualification.

Those remain later scoreboard items. The immediate next planned item is **#9 — integrate the concurrent PS2 MPEG worker**, so the receiving side can be made genuinely ready before START is allowed to cause MPEG production.
