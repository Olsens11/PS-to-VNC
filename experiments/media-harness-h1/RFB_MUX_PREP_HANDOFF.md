# H1 RFB mux preparation handoff

This short handoff exists for future conversations and development sessions.

Start with:

1. `AGENTS.md`
2. `docs/status.md`
3. `docs/PROJECT_INTENT.md`
4. `docs/CLEAN_ARCHITECTURE.md`
5. `experiments/media-harness-h1/README.md`
6. `experiments/media-harness-h1/RFB_MUX_INTEGRATION_PREP.md`
7. `experiments/media-harness-h1/RFB_MUX_PREP_CHECKLIST.md`
8. `experiments/media-harness-h1/MEDIA_OBJECT_COMPOSITION_NOTES.md`
9. `experiments/media-harness-h1/H1_TOOL_GUIDE.md`
10. `experiments/media-harness-h1/RFB_MUX_CP2J_HARDWARE_RESULT.md`
11. `experiments/media-harness-h1/RFB_MUX_PREP_STATUS.env`

## Current qualified boundary

CP2J is hardware-qualified for the narrow RFB-only milestone on the exact
candidate recorded in `RFB_MUX_CP2J_HARDWARE_RESULT.md`:

- one physical PSTV TCP stream;
- RFB on logical channel 1;
- configurable RFB queue/credit flow;
- Pi raw-RFB bridge;
- unchanged through-Issue-39 RFB parser through the mux I/O seam;
- authoritative 704x462 CPU framebuffer;
- initial framebuffer plus live incremental updates; and
- clean REQUEST -> BOUNDARY -> COMMIT -> COMPLETE finite shutdown.

The hardware run processed 17,125,191 server-direction RFB bytes and 107
completed incremental framebuffer updates with `integrity_pass=1`,
`transport_error=0`, and full quiesce/session validation.

Visible RFB presentation, input, OSK/local UI, RFB+MPEG, RFB+audio, and the
future shared compositor are **not** qualified by CP2J.

## Immediate engineering objective

> Add and independently qualify **visible RFB-only presentation** while audio
> and MPEG remain OFF. Reuse the through-Issue-39 presentation path or establish
> one clearly owned compositor boundary; do not introduce concurrent MPEG
> presentation in the same checkpoint.

After visible RFB-only presentation is qualified, reintroduce controller /
mouse / keyboard / OSK as a separate cumulative checkpoint. Only then proceed
toward the shared RFB-background + MPEG-media-object compositor.

Do not regress to separate physical RFB and media TCP streams.
Do not edit the clean RFB parser merely to accommodate mux framing; implement
`rfb_io.h` through the mux instead.
Do not grow queues merely because a test stalls; CP2J's 32768-byte RFB queue is
a qualified starting point, not a claim that it is the final optimum.
Do not broaden CP2J's hardware qualification beyond the exact layers exercised.
Do not start Issue #40 as part of this H1 experiment.

Canonical operator control is `h1_tool.py`; avoid returning to bespoke giant
command blocks for routine H1 parameter tests.
