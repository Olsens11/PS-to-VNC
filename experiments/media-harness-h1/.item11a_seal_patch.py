#!/usr/bin/env python3
"""One-shot documentation promotion for sealed CP2P item #11A."""

from pathlib import Path

contract_path = Path("experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md")
text = contract_path.read_text()


def replace_once(old: str, new: str) -> None:
    global text
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"expected exactly one contract match, got {count}: {old[:120]!r}")
    text = text.replace(old, new, 1)


replace_once(
    """### Retirement wire status

At the time this document was created, the PS2-side semantic retirement behavior is settled, but a final Pi-facing retirement wire encoding has **not yet been established in the current source tree**. Do not infer retirement from silence, timeout, or a guessed START variant.

Before the all-guns candidate is sealed, the Pi implementation must have an explicit generation-safe way to stop/drain the old producer and remove its suppression when the PS2 retires that generation. Once that exact wire/control seam is implemented, update this section with the concrete encoding and source authority.

For the first RFB-only -> MPEG START path, no prior MPEG generation exists and therefore no retirement message is required before START.""",
    """### Exact-generation retirement control (item #11A)

The Pi-facing retirement seam is now concrete and host/PS2-build proven. It uses one H1 experiment-local PSTV frame kind on **control channel 0**, not MPEG DATA channel 4:

- frame kind: `PSTVNC_H1_FRAME_MPEG_RETIRE = 10`;
- flags: `0`;
- payload length: exactly 12 bytes;
- payload: three unsigned big-endian 32-bit words: `version=1`, `session_id`, `generation`;
- request direction: PS2 -> Pi;
- completion direction: Pi -> PS2 using the identical payload;
- direction plus each side's strict one-pending-generation state distinguish request from completion.

The PS2 transport publishes the exact pending generation **before** sending the request so an immediate Pi completion cannot race the receiver thread. The sole receiver accepts completion only when version, active session id, nonzero generation, and exact pending generation all match. The application coordinator polls that receiver-owned completion asynchronously; it never blocks the sole transport receive owner.

The ordering is deliberately **Pi first** during recalibration. While generation N remains the authoritative PS2 MPEG owner, the coordinator sends RETIRE(N) and waits. The Pi completion is not sent until exact Pi runtime cleanup succeeds. Only after the PS2 observes completion does the existing local worker/presentation retirement run and create the fresh full-RFB restoration obligation. Therefore a failed or missing Pi retirement cannot leave the PS2 locally retired while the Pi still suppresses RFB, and no fresh full-RFB restoration can begin before Pi cleanup is proven complete.

At the current pre-producer boundary the Pi exact cleanup transaction validates one matching prepared START, capture plan, and suppression generation; rejects any unexpected live legacy producer; removes exact-generation suppression; releases prepared START/capture state and evidence; then sends completion. The prepared-generation high-water is retained, so the retired generation remains stale and cannot be reused.

Item #8 must extend this exact same transaction rather than inventing a second retirement protocol: replace the current dormant-producer guard with exact producer stop/drain for generation N, prove that stop/drain is complete **before** suppression removal, then continue the already-proven cleanup and completion ordering.

For the first RFB-only -> MPEG START path, no prior MPEG generation exists and therefore no retirement message is required before START.""",
)

replace_once(
    "- retirement stops/drains the exact active generation once the retirement wire seam is defined;",
    "- exact RETIRE request/completion rejects wrong session/generation and does not acknowledge until exact Pi state is released (`h1_cp2p_retirement_control_test.py`);\n- PS2 local MPEG ownership remains intact while Pi retirement is pending, and full-RFB restoration begins only after exact Pi completion (`h1_cp2p_session_coordinator_test.c`);\n- once item #8 makes the producer live, retirement stops/drains that exact producer before suppression removal and completion;",
)

replace_once(
    "| retirement control | wire/control seam still to be made concrete | exact-generation stop/drain/remove-suppression/release-capture path with completion semantics before fresh full-RFB restoration |",
    "| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10/control-channel exact request/completion, one pending generation, Pi-cleanup-before-completion, and Pi-completion-before-local-retire/full-RFB ordering; item #8 extends cleanup with exact producer stop/drain before suppression removal |",
)

replace_once(
    "| verification | items #5/#6/#7 host tests; proof runs `34702828032`, `34709167702`, final atomic proof `34710050027` | include START, same-reader, dynamic suppression, exact capture, and compound rollback contracts in fresh-install self-check plus project regressions |",
    "| verification | items #5/#6/#7 host tests plus item #11A exact-retirement tests; proof runs `34702828032`, `34709167702`, `34710050027`, item #11A run `34710972431` | include START, same-reader, dynamic suppression, exact capture, compound rollback, exact retirement request/completion, and Pi-completion-before-local-retire contracts in fresh-install self-check plus project regressions |",
)

replace_once(
    "- compound START preparation rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`\n- Pi START implementation/proof note:",
    "- compound START preparation rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`\n- Pi exact-generation retirement wire/control codec: `experiments/media-harness-h1/h1_cp2p_retirement_control.py`\n- Pi exact-generation retirement contract: `experiments/media-harness-h1/h1_cp2p_retirement_control_test.py`\n- PS2 exact retirement transport owner: `experiments/media-harness-h1/h1_transport_runtime.[ch]`\n- PS2 retirement/recalibration coordinator ordering: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`\n- Pi START implementation/proof note:",
)

replace_once(
    "- item #8 producer activation remains deliberately dormant;\n- item #11A exact-generation Pi retirement control remains the next architectural prerequisite before item #8; local `retire_suppression_exact()` is only a primitive, not the completed cross-machine control seam;\n- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;",
    "- item #11A exact-generation Pi retirement control is DONE at the pre-producer software-proof boundary: kind-10 control request/completion is exact-session/exact-generation matched, Pi prepared/capture/suppression state is released before completion, PS2 local MPEG ownership remains intact while completion is pending, and fresh full-RFB restoration is created only after exact Pi completion;\n- item #8 producer activation remains deliberately dormant and is now the next implementation tranche; when item #8 makes FFmpeg live, it must extend item #11A cleanup with exact generation stop/drain before suppression removal/completion rather than changing the wire or ordering;\n- item #11B remains responsible for the fully live cross-machine lifecycle, including stale old-generation MPEG queue exclusion/drain after producer activation;\n- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;",
)

replace_once(
    "- item #6/#7 final host proof authority is branch `experiment/h1-cp2p-pi-suppression-geometry`, source head `81d150988e5bc0435c580465b3a826429aa7c139`, GitHub Actions run `34710050027` / job `103597045614`;\n- physical all-guns hardware qualification has not yet occurred.",
    "- item #6/#7 final host proof authority is branch `experiment/h1-cp2p-pi-suppression-geometry`, source head `81d150988e5bc0435c580465b3a826429aa7c139`, GitHub Actions run `34710050027` / job `103597045614`;\n- item #11A proof authority is branch `experiment/h1-cp2p-pi-retirement-control`, source head `d8d731ce9f66fc14ea7bfc2777215085b69975ae`, GitHub Actions run `34710972431`, host job `103599586082`, PS2-build job `103599586147`, unqualified artifact `10302704205`;\n- item #11A proof ELF SHA256 is `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`, ELF bytes `3232156`, PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`, PT_LOAD bytes `510612`;\n- physical all-guns hardware qualification has not yet occurred.",
)

contract_path.write_text(text.rstrip() + "\n")
print("ITEM11A_CONTRACT_PROMOTION=PASS")
