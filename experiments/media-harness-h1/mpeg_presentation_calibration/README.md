# MPEG presentation calibration

Experiment-local portable calibration core. This module owns calibration state and geometry only; it does not own pad polling, ordinary mouse/OSK routing, RFB parsing, MPEG decode, permanent composition, or PS2 GS primitives.

Accepted interaction preserved here:

- held START+SELECT enters calibration as a one-shot level chord;
- D-pad resizes around center in 16-pixel increments;
- R1 + D-pad moves one pixel per sample;
- R2 controls inner matte: Right grows all sides, Left shrinks all, Down grows vertical, Up shrinks vertical;
- L2 controls outer matte: Left grows all sides, Right shrinks all, Up grows vertical, Down shrinks vertical;
- Start resets; Triangle opens Controls; X opens Review; Circle cancels/backs out;
- Review requires the entering X to be released before a second X press can accept;
- post-exit release quarantine retains foreground ownership until all calibration controls are physically up.

## RFB / MPEG ownership contract

While calibration owns the foreground, the H1 adapter must keep the RFB transport/session healthy but stop new visual work only at an existing complete-message scheduling boundary: finish an in-progress server message, stop issuing new framebuffer-update requests, retain the last authoritative desktop image, suppress ordinary mouse/controller dispatch, and render calibration locally.

After accepted MPEG ownership begins, RFB may resume elsewhere. The rectangle returned by `pstvnc_mpeg_calibration_rfb_suppression_rect()` is the base MPEG region expanded by the outer matte and clipped to the physical canvas. From that rectangle's outer edge inward, RFB presentation is forbidden while MPEG presentation owns the region.

Calibration remains separate from the permanent compositor/presentation backend.
