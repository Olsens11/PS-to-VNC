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

Immediate engineering objective:

> Activate the already-reserved RFB logical channel inside H1's existing single
> PSTV TCP connection, first as an RFB-only hardware milestone. Do not combine
> graphics owners or attempt automatic MPEG overlay detection until the
> transport adapter itself is independently qualified.

Do not regress to separate physical RFB and media TCP streams.
Do not edit the clean RFB parser merely to accommodate mux framing; implement
`rfb_io.h` through the mux instead.
Do not grow queues merely because a test stalls; use the existing 32768-byte
RFB receive-buffer precedent as the first controlled reference and instrument
before changing capacity.
Do not claim changed PT_LOAD qualification without hardware evidence.
Do not start Issue #40 as part of this H1 experiment.

Canonical operator control is `h1_tool.py`; avoid returning to bespoke giant
command blocks for routine H1 parameter tests.
