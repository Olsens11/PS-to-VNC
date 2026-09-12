#!/usr/bin/env python3
"""One-shot seal for H1 CP2P item #8.

Promotes only current-state authorities, appends one captain's-log entry, and
removes temporary item-#8 proof/staging machinery. Historical entries are never
rewritten.
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CONTRACT = ROOT / "experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md"
HISTORY = ROOT / "experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected exactly one stale block, found {count}")
    return text.replace(old, new, 1)


contract = CONTRACT.read_text()

contract = replace_once(
    contract,
    "- FFmpeg is now the concrete MPEG capture/encode executable assumed by the prepared CP2P capture plan. Item #7 prepares its exact x11grab command but does not launch it; item #8 remains responsible for producer activation. The comprehensive bootstrap must eventually map the `ffmpeg` capability to the supported Pi OS package/install authority before the live producer is promoted.",
    "- FFmpeg is now a live CP2P runtime dependency. Item #8 launches the exact START-derived x11grab command for one immutable generation after suppression and capture preparation succeed. The comprehensive bootstrap must eventually map the `ffmpeg` capability to the supported Pi OS package/install authority before this experiment is promoted.",
    "ffmpeg dependency",
)

contract = replace_once(
    contract,
    """    -> establish immutable prepared-generation state           [item #5 DONE]\n    -> install same-generation RFB suppression as pending      [item #6 DONE]\n    -> prepare exact START-base X,Y,W,H capture command        [item #7 preparation DONE]\n    -> start/activate MPEG producer for that generation        [item #8 OPEN]\n    -> only then permit MPEG DATA for that generation onto PSTV channel 4""",
    """    -> establish immutable prepared-generation state           [item #5 DONE]\n    -> install same-generation RFB suppression as pending      [item #6 DONE]\n    -> prepare exact START-base X,Y,W,H capture command        [item #7 DONE]\n    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]\n    -> keep MPEG output local/bounded while public gate closed [item #10 OPEN]""",
    "START order",
)

contract = replace_once(
    contract,
    "The first three Pi preparation phases are one fail-closed transaction. If suppression or capture-plan preparation/evidence fails, usable state for that generation is rolled back together while the generation high-water remains stale. No failed compound preparation may later become producer-eligible.",
    "The START preparation/launch path is one fail-closed transaction. If suppression, capture-plan preparation/evidence, or producer launch fails, usable state for that generation is unwound only as far as quiescence can be proven while the generation high-water remains stale. Once a producer has launched, producer stop/drain is the first rollback step; if that exact producer cannot be proven quiescent, suppression and prepared-generation state are deliberately retained rather than exposing a potentially live MPEG source. No failed compound preparation may later become producer-eligible.",
    "compound START transaction",
)

old_section8 = """## 8. Producer startup state\n\nFor the all-guns CP2P session:\n\n- RFB may be active from session startup;\n- PCM audio may be active from session startup;\n- MPEG capture/encoding remains dormant after items #5/#6/#7 preparation;\n- START supplies the live capture rectangle; startup CONFIG geometry is not a substitute for the user's accepted calibration;\n- `h1_cp2p_capture_geometry.py` now prepares the exact FFmpeg x11grab command from that immutable START rectangle;\n- item #8 alone will consume that prepared command and launch the producer;\n- producer lifecycle must be generation-scoped;\n- only one MPEG generation is active at a time for the current milestone.\n\nThe current experimental runner `h1_mux_server_cp2p_start_receiver.py` now owns the ordered pre-producer transaction while preserving the same sole-reader call chain. It records suppression/capture evidence with the producer explicitly marked dormant. The permanent comprehensive Pi runtime/bootstrap must absorb these proven semantics rather than install the experiment as an unrelated second daemon.\n\nNo new Pi service/socket/thread/package was introduced by suppression itself. The capture plan makes the `ffmpeg` executable dependency concrete, but item #8 has not yet launched or qualified it as a live producer.\n"""
new_section8 = """## 8. Producer startup state\n\nItem #8 is now concrete at the software/pre-public-gate boundary:\n\n- RFB may be active from session startup;\n- PCM audio may be active from session startup;\n- START supplies the live capture rectangle; startup CONFIG geometry is not a substitute for the user's accepted calibration;\n- `h1_cp2p_capture_geometry.py` prepares the exact FFmpeg x11grab command from the immutable START base rectangle;\n- only after same-generation suppression and exact capture preparation succeed does `h1_cp2p_mpeg_producer.py` launch one generation-owned FFmpeg process;\n- the producer is attached to the existing H1 session as the video producer; no second PS2-facing socket, transport reader, or bypass writer is introduced;\n- producer output is locally bounded/backpressured and archived for evidence, but item #10's public MPEG CONFIG gate remains closed, so item #8 emits **zero PSTV MPEG DATA**;\n- only one MPEG producer generation may be active at a time; wrong-generation retirement is rejected;\n- exact retirement first requests producer stop, drains/discards every locally buffered unsent byte, and proves the subprocess/reader quiescent before suppression may be removed;\n- if graceful stop cannot prove quiescence, termination is attempted; if quiescence still cannot be proven, retirement fails closed and generation ownership/suppression are retained rather than acknowledged.\n\nThe current experimental runner `h1_mux_server_cp2p_start_receiver.py` owns the ordered START -> suppression -> exact capture -> producer transaction while preserving the same sole-reader call chain. `h1_cp2p_mpeg_producer_test.py`, the retirement-control contract, and the compound START rollback contract jointly prove the producer lifecycle and its fail-closed boundaries. The permanent comprehensive Pi runtime/bootstrap must absorb these semantics rather than install the experiment as an unrelated second daemon.\n\nFFmpeg is therefore now a real runtime capability required by the experiment, not merely a planned command. The distro-specific installation/package authority still belongs to the future comprehensive Pi bootstrap and is deliberately not guessed here.\n"""
contract = replace_once(contract, old_section8, new_section8, "section 8")

contract = replace_once(
    contract,
    "At the current pre-producer boundary the Pi exact cleanup transaction validates one matching prepared START, capture plan, and suppression generation; rejects any unexpected live legacy producer; removes exact-generation suppression; releases prepared START/capture state and evidence; then sends completion. The prepared-generation high-water is retained, so the retired generation remains stale and cannot be reused.\n\nItem #8 must extend this exact same transaction rather than inventing a second retirement protocol: replace the current dormant-producer guard with exact producer stop/drain for generation N, prove that stop/drain is complete **before** suppression removal, then continue the already-proven cleanup and completion ordering.",
    "Item #8 now extends this exact transaction with the real generation-owned producer rather than inventing a second retirement protocol. The Pi validates one matching prepared START, capture plan, suppression generation, and producer generation; exact producer stop/drain/discard is the first destructive step; only after the producer subprocess and reader are proven quiescent may exact-generation suppression be removed, prepared START/capture state and evidence be released, and completion be sent. An unexpected live legacy producer without the generation owner still blocks acknowledgement. If producer quiescence cannot be proven, cleanup stops fail-closed before suppression removal and no completion is sent. The prepared-generation high-water is retained, so the retired generation remains stale and cannot be reused.\n\nItem #11B remains responsible for the fully live data-plane generation boundary once MPEG bytes are allowed onto PSTV: stale generation-N bytes must not remain consumable by a fresh N+1 worker, and the one-reader/one-writer architecture must remain intact.",
    "retirement extension",
)

contract = replace_once(
    contract,
    "- the producer remains dormant during #5/#6/#7 preparation;",
    "- the producer remains dormant through #5/#6/#7 preparation and launches only at item #8's final same-generation activation step;",
    "reconstruction producer dormant test",
)
contract = replace_once(
    contract,
    "- once item #8 makes the producer live, retirement stops/drains that exact producer before suppression removal and completion;",
    "- item #8 exact retirement stops/drains/discards that exact producer's unsent local bytes and proves process/reader quiescence before suppression removal and completion (`h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_retirement_control_test.py`);\n- failed post-launch rollback that cannot prove producer quiescence retains suppression/prepared state and fails closed (`h1_cp2p_start_preparation_transaction_test.py`);\n- while item #10 remains closed, the generation-owned producer has no PSTV MPEG DATA send path and cannot bypass the existing writer;",
    "reconstruction retirement test",
)

contract = replace_once(
    contract,
    "| compound START preparation | `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_start_preparation_transaction_test.py` | preserve ordered START -> suppression -> exact capture preparation and fail-closed rollback before producer activation |",
    "| compound START / producer activation | `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_start_preparation_transaction_test.py` | preserve ordered START -> suppression -> exact capture -> generation-owned producer launch and the fail-closed rollback barrier that retains suppression if producer quiescence cannot be proven |",
    "promotion compound START row",
)
contract = replace_once(
    contract,
    "| MPEG producer | item #8 still open | launch the already-prepared exact capture command only after #11A retirement control exists; no independent geometry authority |",
    "| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, keep output locally bounded while #10 is closed, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |",
    "promotion producer row",
)
contract = replace_once(
    contract,
    "| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10/control-channel exact request/completion, one pending generation, Pi-cleanup-before-completion, and Pi-completion-before-local-retire/full-RFB ordering; item #8 extends cleanup with exact producer stop/drain before suppression removal |",
    "| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`, `h1_cp2p_mpeg_producer.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10/control-channel exact request/completion, one pending generation, exact producer stop/drain before suppression removal, Pi-cleanup-before-completion, and Pi-completion-before-local-retire/full-RFB ordering; #11B adds stale live-data exclusion across generations |",
    "promotion retirement row",
)
contract = replace_once(
    contract,
    "| verification | items #5/#6/#7 host tests plus item #11A exact-retirement tests; proof runs `34702828032`, `34709167702`, `34710050027`, item #11A run `34710972431` | include START, same-reader, dynamic suppression, exact capture, compound rollback, exact retirement request/completion, and Pi-completion-before-local-retire contracts in fresh-install self-check plus project regressions |",
    "| verification | items #5/#6/#7/#8 host tests plus item #11A exact-retirement tests; proof runs `34702828032`, `34709167702`, `34710050027`, `34710972431`, item #8 clean run `34712568840` | include START, same-reader, dynamic suppression, exact capture, generation-owned producer start/stop/drain, fail-closed post-launch rollback, exact retirement request/completion, closed-public-gate/no-bypass-writer, and Pi-completion-before-local-retire contracts in fresh-install self-check plus project regressions |",
    "promotion verification row",
)

contract = replace_once(
    contract,
    "- exact capture geometry contract: `experiments/media-harness-h1/h1_cp2p_capture_geometry_test.py`\n- compound START preparation rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`",
    "- exact capture geometry contract: `experiments/media-harness-h1/h1_cp2p_capture_geometry_test.py`\n- generation-owned START-driven Pi MPEG producer: `experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`\n- producer lifecycle contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`\n- compound START/producer rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`",
    "source map producer",
)

old_boundary = """## 16. Current implementation boundary\n\nAt this document's current revision:\n\n- PS2 accepted -> fresh generation -> worker arm -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;\n- START+SELECT remains explicitly only a replaceable test trigger;\n- Pi START receive/validation and immutable prepared-generation state are implemented and host-proven by item #5 without adding a second socket, reader, receive thread, producer, or package dependency;\n- item #6 generation-scoped Pi RFB suppression is concrete and host-proven: pending suppression activates on the first new update request, Raw updates are clipped around the exact suppression footprint, and the transaction budget derives from the current desktop dimensions rather than a hard-coded resolution;\n- item #7 exact capture preparation is concrete and host-proven: START base X/Y/W/H becomes the prepared FFmpeg x11grab source while current desktop dimensions are validation bounds only;\n- compound START preparation is atomic at the current software boundary: failed post-suppression capture setup removes usable suppression/prepared/capture state and evidence while preserving stale generation high-water;\n- item #6 is DONE at the current host/software boundary; item #7 remains PARTIAL only because the prepared exact capture plan is not yet consumed by a live producer;\n- item #11A exact-generation Pi retirement control is DONE at the pre-producer software-proof boundary: kind-10 control request/completion is exact-session/exact-generation matched, Pi prepared/capture/suppression state is released before completion, PS2 local MPEG ownership remains intact while completion is pending, and fresh full-RFB restoration is created only after exact Pi completion;\n- item #8 producer activation remains deliberately dormant and is now the next implementation tranche; when item #8 makes FFmpeg live, it must extend item #11A cleanup with exact generation stop/drain before suppression removal/completion rather than changing the wire or ordering;\n- item #11B remains responsible for the fully live cross-machine lifecycle, including stale old-generation MPEG queue exclusion/drain after producer activation;\n- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;\n- item #6/#7 final host proof authority is branch `experiment/h1-cp2p-pi-suppression-geometry`, source head `81d150988e5bc0435c580465b3a826429aa7c139`, GitHub Actions run `34710050027` / job `103597045614`;\n- item #11A proof authority is branch `experiment/h1-cp2p-pi-retirement-control`, source head `d8d731ce9f66fc14ea7bfc2777215085b69975ae`, GitHub Actions run `34710972431`, host job `103599586082`, PS2-build job `103599586147`, unqualified artifact `10302704205`;\n- item #11A proof ELF SHA256 is `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`, ELF bytes `3232156`, PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`, PT_LOAD bytes `510612`;\n- physical all-guns hardware qualification has not yet occurred.\n\nUpdate this file whenever a Pi-side mechanism, dependency, path, service requirement, or configuration step becomes concrete. Preserve the experiment history separately rather than rewriting historical entries."""
new_boundary = """## 16. Current implementation boundary\n\nAt this document's current revision:\n\n- PS2 accepted -> fresh generation -> worker arm -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;\n- START+SELECT remains explicitly only a replaceable test trigger;\n- Pi START receive/validation and immutable prepared-generation state are implemented and host-proven by item #5 without adding a second PS2-facing socket, reader, or receive thread;\n- item #6 generation-scoped Pi RFB suppression is concrete and host-proven: pending suppression activates on the first new update request, Raw updates are clipped around the exact suppression footprint, and the transaction budget derives from the current desktop dimensions rather than a hard-coded resolution;\n- item #7 exact calibrated geometry is DONE at the current software boundary: START base X/Y/W/H becomes the exact FFmpeg x11grab source and item #8 consumes that same immutable plan in a live generation-owned producer; current desktop dimensions remain validation bounds only;\n- item #8 START-driven MPEG production is DONE at the pre-public-gate software boundary: suppression and exact capture preparation precede one generation-owned FFmpeg launch, output remains locally bounded/backpressured, and no PSTV MPEG DATA is emitted while item #10 remains closed;\n- item #8 extends item #11A retirement exactly as intended: exact producer stop/drain/discard and subprocess/reader quiescence precede suppression removal and Pi completion; a producer that cannot be proven quiescent leaves suppression/prepared state retained fail-closed and cannot be acknowledged;\n- item #11A exact-generation Pi retirement control is therefore DONE with the live local producer present: kind-10 request/completion remains exact-session/exact-generation matched, PS2 local MPEG ownership remains intact while completion is pending, and fresh full-RFB restoration is created only after exact Pi completion;\n- item #11B is the next implementation tranche and remains responsible for the fully live cross-machine data-plane generation boundary, especially excluding/draining stale old-generation MPEG bytes so generation N data can never feed N+1;\n- item #10 remains CLOSED and remains the later authority that will expose MPEG DATA through public CP2P CONFIG only after #11B is safe;\n- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;\n- item #8 product source authority is branch `experiment/h1-cp2p-start-driven-producer`, commit `0347e2f1a0295a83aa7051fc9b963b7f3ece85a2`; clean committed-source verification used wrapper head `51e1141f252edec2a7c1b19f445d941033e6fa19`, whose only product-independent delta is the one-line CI trigger comment;\n- item #8 clean proof is GitHub Actions run `34712568840`, host job `103603873398`, PS2 regression job `103603873498`;\n- the pinned PS2 regression retained the #11A binary identity exactly: ELF SHA256 `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`, ELF bytes `3232156`, PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`, PT_LOAD bytes `510612`;\n- physical all-guns hardware qualification has not yet occurred.\n\nUpdate this file whenever a Pi-side mechanism, dependency, path, service requirement, or configuration step becomes concrete. Preserve the experiment history separately rather than rewriting historical entries."""
contract = replace_once(contract, old_boundary, new_boundary, "section 16")

CONTRACT.write_text(contract)

old_history = HISTORY.read_text()
heading = "## 41. Item #8 — START-driven Pi MPEG producer closes behind the public gate"
if heading in old_history:
    raise SystemExit("history section 41 already exists")

entry = r'''

---

# PHASE Q — SEPTEMBER 12 START-DRIVEN PI MPEG PRODUCER

## 41. Item #8 — START-driven Pi MPEG producer closes behind the public gate

Work continued from sealed item-#11A head `d2283b44cd9c240b26b9bda2f045a7ca1452af06` on branch `experiment/h1-cp2p-start-driven-producer`. The objective was deliberately narrower than all-guns activation: make a valid immutable START wake a real exact-region Pi encoder owned by that generation, while keeping item #10's public MPEG CONFIG gate closed so no MPEG bytes could yet enter PSTV.

The implementation first landed as `f9ad2f991713256890409fdfb55399bff4eaabd9` (`h1: add START-driven generation MPEG producer`) and was then hardened at `0347e2f1a0295a83aa7051fc9b963b7f3ece85a2` (`h1: harden START-driven producer rollback ordering`). The latter is the item-#8 product-source authority.

`h1_cp2p_mpeg_producer.py` now owns one exact generation. It launches only the already-prepared FFmpeg/x11grab command derived from the accepted START base X/Y/W/H after that same generation's RFB suppression and capture plan are established. It does not reconstruct geometry independently. The producer is attached to the existing H1 session and uses the existing bounded producer-buffer behavior; it creates no second PS2-facing transport, reader, frame-sequence owner, or bypass writer.

The public all-guns gate remains intentionally closed. Item #8 therefore proves a real local encoder process and lifecycle, **not** MPEG transport activation: generated MPEG output remains locally bounded/backpressured and `h1_cp2p_mpeg_producer.py` has no PSTV channel-4 send path. `H1_CP2P_ITEM8_PUBLIC_GATE=CLOSED` is part of the proof boundary. Item #10 remains the later authority that will expose MPEG DATA only after the live generation-transition boundary is safe.

The important lifecycle addition is exact producer retirement. For generation N, Pi cleanup now orders:

`stop/drain/discard exact producer N -> prove subprocess + reader quiescent -> remove suppression N -> release prepared START/capture/evidence N -> send exact RETIRE completion N`.

Every locally buffered unsent MPEG byte is drained/discarded before suppression can be removed. Wrong-generation retirement is rejected. If graceful stop cannot quiesce the producer, termination is attempted; if quiescence still cannot be proven, retirement fails closed. A review during implementation found and corrected a subtler rollback edge: a failed post-launch START transaction originally could have continued removing suppression after producer stop/drain failed. The hardened path now stops rollback at that boundary and deliberately retains suppression plus prepared-generation state when producer dormancy cannot be proven.

That ordering preserves item #11A's Pi-first cross-machine rule. The PS2 still retains generation N as authoritative while waiting for Pi completion; the Pi cannot acknowledge until the exact local producer and suppression/capture state are safely retired; only then may PS2 local retirement create the fresh full-RFB restoration obligation.

Host coverage now includes:

- `h1_cp2p_mpeg_producer_test.py`: exact START-plan launch, no PSTV MPEG emission, exact retirement and unsent-byte drain, wrong-generation refusal, and stuck-producer fail-closed behavior;
- `h1_cp2p_retirement_control_test.py`: a live generation-owned producer retires before suppression removal and completion, while an unowned live legacy producer blocks acknowledgement;
- `h1_cp2p_start_preparation_transaction_test.py`: post-launch rollback failure that cannot prove producer quiescence retains suppression/prepared state fail-closed;
- the full prior #5/#6/#7/#11A Pi contract suite;
- the complete calibration/session host suite.

A first green proof run, `34711889280`, applied the final safety hardening in the runner worktree and then committed it as `0347e2f1...`. Because the project distinguishes tested working-tree state from immutable committed-source proof, a second clean verification was required. CI-only head `51e1141f252edec2a7c1b19f445d941033e6fa19` differs from the product source only by a one-line workflow trigger comment. GitHub Actions run `34712568840` checked out that committed tree; all item-#8 staging helpers reported `ALREADY_APPLIED`, the working tree remained product-clean, and both jobs passed:

- host-contracts `103603873398` — SUCCESS;
- ps2-regression `103603873498` — SUCCESS.

The pinned PS2 regression used `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11` and retained the exact item-#11A PS2 binary identity, as expected for this Pi-only product tranche:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`;
- ELF bytes `3232156`;
- PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`;
- PT_LOAD bytes `510612`;
- `H1_CP2P_ITEM8_PS2_REGRESSION=PASS`;
- `TESTKIT_PT_LOAD_FINGERPRINT=PASS`.

This changes the checklist at the software boundary:

- **#7 exact calibrated geometry end-to-end: PARTIAL -> DONE**, because the immutable START base rectangle is now consumed by the real generation-owned encoder process rather than ending at a prepared command;
- **#8 START-driven MPEG producer: OPEN -> DONE behind the public gate**;
- **#11A exact Pi retirement control remains DONE and is now proven with a live local producer**;
- **#11B remains OPEN/PARTIAL and is next**: before item #10 permits real MPEG DATA, the cross-machine data plane must ensure stale generation-N bytes in the session-scoped MPEG transport path cannot be consumed by generation N+1;
- **#10 remains OPEN/CLOSED by design**;
- **#12 remains OPEN**.

No deployment, physical PS2 behavior, concurrent live MPEG transport, or all-guns hardware stability is claimed by item #8. The preferred remaining order is **#11B -> #10 -> #12**.
'''

new_history = old_history + entry
if not new_history.startswith(old_history):
    raise SystemExit("captain's log prefix invariant failed")
if new_history.count(heading) != 1:
    raise SystemExit("history section 41 cardinality failed")
HISTORY.write_text(new_history)

# Temporary proof/seal machinery must not survive the seal commit.
for rel in (
    ".github/workflows/h1-cp2p-item8-producer.yml",
    "experiments/media-harness-h1/.item8_patch.py",
    "experiments/media-harness-h1/.item8_patch_fix.py",
    "experiments/media-harness-h1/.item8_safety_patch.py",
    "experiments/media-harness-h1/.item8_seal.py",
):
    path = ROOT / rel
    if not path.exists():
        raise SystemExit(f"expected temporary file missing before cleanup: {rel}")
    path.unlink()

print("H1_CP2P_ITEM8_SEAL=PASS")
