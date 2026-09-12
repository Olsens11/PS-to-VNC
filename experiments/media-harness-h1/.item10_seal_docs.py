#!/usr/bin/env python3
"""One-shot exact documentation promotion for sealed CP2P item #10."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CONTRACT = ROOT / "experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md"
HISTORY = ROOT / "experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


contract = CONTRACT.read_text()

contract = replace_once(
    contract,
    "    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]\n"
    "    -> keep MPEG output local/bounded while public gate closed [item #10 OPEN]\n",
    "    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]\n"
    "    -> open that generation's exact emission fence             [item #10 DONE]\n"
    "    -> send channel-4 MPEG only under an exact emission lease  [item #10 DONE]\n",
    "START processing item10 state",
)

contract = replace_once(
    contract,
    "Item #8 is now concrete at the software/pre-public-gate boundary:\n",
    "Item #8 remains the START-owned producer authority. Item #10 now exposes that already-proven producer through the public CP2P MPEG gate without changing who may start it:\n",
    "producer startup section status",
)

contract = replace_once(
    contract,
    "- producer output is locally bounded/backpressured and archived for evidence, but item #10's public MPEG CONFIG gate remains closed, so item #8 emits **zero PSTV MPEG DATA**;\n",
    "- producer output remains locally bounded/backpressured and archived for evidence; item #10 now permits the existing CP2P scheduler to emit that exact generation's bytes only through `open_emission_exact` / `begin_emission_exact` / `finish_emission_exact` and the existing serialized PSTV writer;\n",
    "producer live emission promotion",
)

contract = replace_once(
    contract,
    "Item #11B is now concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 remains responsible for actually opening the public MPEG CONFIG/emission gate and exercising this boundary with live MPEG DATA.\n",
    "Item #11B is concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 now exercises this boundary with live MPEG DATA: CONFIG enables the capability, START remains the sole producer/generation authority, and every MPEG send holds the exact-generation emission lease.\n",
    "retirement item10 promotion",
)

contract = replace_once(
    contract,
    "- while item #10 remains closed, the generation-owned producer has no PSTV MPEG DATA send path and cannot bypass the existing writer;\n",
    "- item #10 live MPEG DATA is emitted only through the existing serialized PSTV writer while holding the exact generation's emission lease; CONFIG alone cannot start a producer and wrong/stale generations cannot send;\n",
    "reconstruction test item10 send path",
)

contract = replace_once(
    contract,
    "- item #10 remains closed: the CP2P runner does not call `open_emission_exact()` and the public PS2 CP2P CONFIG gate still requires video OFF;\n",
    "- item #10 is open only for the CP2P composition: the CP2P CONFIG gate requires visible RFB + MPEG2_ES + optional PCM, CP2O still requires MPEG OFF, START remains producer authority, and the generated CP2P decoder contains no obsolete standalone-GS transfer/draw-packet prerequisite (`h1_cp2p_item10_activation_test.py`);\n",
    "reconstruction test public gate",
)

contract = replace_once(
    contract,
    "| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, keep output locally bounded while #10 is closed, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |\n",
    "| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, expose output only through item #10's exact-generation emission lease, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |\n",
    "promotion inventory MPEG producer",
)

contract = replace_once(
    contract,
    "| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean; item #10 may open emission only through this fence |\n",
    "| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; item-#10 scheduler in `h1_mux_server_cp2p_start_receiver.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, lease-wrapped live channel-4 sends, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean |\n",
    "promotion inventory live scheduler",
)

contract = replace_once(
    contract,
    "| verification | items #5/#6/#7/#8/#11A/#11B host contracts; clean #11B run `34713445142` (host `103606261825`, PS2 `103606261723`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, Pi emission lease/fence, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, and closed item-#10 public gate in fresh-install self-check plus project regressions |\n",
    "| verification | items #5/#6/#7/#8/#11A/#11B/#10 host contracts; clean #10 run `34718230541` (host `103619197119`, PS2 `103619197057`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, live exact-generation MPEG lease/send accounting, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, CP2O MPEG-OFF regression, generated-runtime first-frame/cancellation checks, and unexpected-worker-failure latching in fresh-install self-check plus project regressions |\n",
    "promotion inventory verification",
)

contract = replace_once(
    contract,
    "- Pi exact-generation retirement contract: `experiments/media-harness-h1/h1_cp2p_retirement_control_test.py`\n",
    "- Pi exact-generation retirement contract: `experiments/media-harness-h1/h1_cp2p_retirement_control_test.py`\n"
    "- CP2P all-guns CONFIG activation gate: `experiments/media-harness-h1/h1_config_cp2p_activation_gate.c` and `h1_config_cp2p_activation_gate_test.c`\n"
    "- CP2P live all-guns scheduler/accounting authority: `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`\n"
    "- CP2P item-#10 activation/runtime contract: `experiments/media-harness-h1/h1_cp2p_item10_activation_test.py` and `CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`\n",
    "source map item10",
)

contract = replace_once(
    contract,
    "- item #8 START-driven MPEG production is DONE at the pre-public-gate software boundary: suppression and exact capture preparation precede one generation-owned FFmpeg launch, output remains locally bounded/backpressured, and no PSTV MPEG DATA is emitted while item #10 remains closed;\n",
    "- item #8 START-driven MPEG production remains DONE: suppression and exact capture preparation precede one generation-owned FFmpeg launch; item #10 now exposes that same START-owned producer through the exact-generation lease rather than introducing a CONFIG-owned producer path;\n",
    "current boundary item8",
)

contract = replace_once(
    contract,
    "- item #11B live cross-machine MPEG generation boundary is DONE at the pre-public-gate software boundary: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;\n"
    "- item #10 remains CLOSED by design and is now the next implementation tranche: it must open the public CP2P MPEG CONFIG gate and route live MPEG DATA only through item #11B's exact-generation emission lease and existing serialized writer;\n",
    "- item #11B live cross-machine MPEG generation boundary remains DONE and is now exercised by the item-#10 live send path: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;\n"
    "- item #10 exact CP2P all-guns CONFIG/live MPEG activation is DONE at the software-proof boundary: CP2P alone accepts visible RFB + MPEG2_ES + optional PCM; CP2O remains MPEG-OFF; CONFIG enables capability but START remains producer/generation authority; every live MPEG send uses the exact-generation emission lease and existing serialized writer; actual wire counters/CRC/start-code evidence are reported; explicit retirement cancellation is clean; and unexpected decoder-worker exit is sticky/fatal;\n",
    "current boundary item10",
)

contract = replace_once(
    contract,
    "- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;\n- physical all-guns hardware qualification has not yet occurred.\n",
    "- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;\n"
    "- item #10 clean official product source is `experiment/h1-cp2p-all-guns-activation` commit `6c24fddbf4618d2e0bf69eba909be640dcff005f`, a clean re-parent of the host/PS2-proven product tree; clean verification run `34718230541` used wrapper `a09fd93b02b4604370370d21626712b28c183ca8` whose only product-independent delta is its proof workflow;\n"
    "- item #10 clean proof jobs are host `103619197119` and pinned PS2 build `103619197057`; the unqualified candidate artifact is `h1-cp2p-item10-all-guns-unqualified-elf`, artifact ID `10305431287`;\n"
    "- item #10 pinned candidate identity is ELF SHA256 `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`, ELF bytes `3238996`, PT_LOAD SHA256 `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`, PT_LOAD bytes `512276`;\n"
    "- physical all-guns hardware qualification has not yet occurred; item #12 is the remaining qualification/seal milestone.\n",
    "current boundary proof identities",
)

CONTRACT.write_text(contract.rstrip() + "\n")

old_history = HISTORY.read_text()
if "## 43. Item #10" in old_history:
    raise SystemExit("history already contains item #10 section 43")

entry = r'''

---

# PHASE S — SEPTEMBER 12 CP2P ALL-GUNS SOFTWARE ACTIVATION

## 43. Item #10 — public CP2P MPEG gate opens; clean software candidate is ready for hardware qualification

After item #11B sealed the live generation boundary, item #10 opened the final public software activation gate on a CP2P-only continuation. The architectural rule remained unchanged: **CONFIG enables capability; START owns generation and producer activation**. No CONFIG-driven producer path was reintroduced.

The PS2 side now has a CP2P-only activation wrapper, `h1_config_cp2p_activation_gate.c`. For the all-guns CP2P composition it accepts visible RFB plus `MPEG2_ES` with optional canonical PCM, while delegating MPEG queue, credit, decoder, scheduler, geometry-capacity, thread, and ordinary H1 constraints to the existing generic validator. The CP2O composition still selects its original visible-RFB/PCM gate and therefore continues to require MPEG OFF. The CP2P main likewise now requires `MPEG2_ES` rather than the prior item-#9 dormant mode.

The Pi runner keeps START as the sole producer authority. A valid START still establishes immutable generation state, pending suppression, exact START-base capture geometry, and the exact-generation FFmpeg producer. Item #10 adds only the live scheduling privilege: `open_emission_exact(generation)` opens the #11B fence for that exact active generation, and each channel-4 send must acquire `begin_emission_exact(generation)` and release `finish_emission_exact(generation)` around the pre-existing serialized `_send_from()` writer. Retirement sets a one-way `retiring` latch before closing emission, so the same generation cannot reopen its fence while RETIRE is waiting for an in-flight send.

Finite-session accounting was made truthful for this live path. MPEG byte/frame/sequence/CRC totals come from the bytes actually written to PSTV, not from every byte FFmpeg produced. MPEG start-code counts are accumulated from the actual sent-byte stream, including boundaries that span DATA frames. This matters because producer tail bytes may be deliberately discarded unsent, and #11B may legitimately discard residual old-generation bytes already enqueued on the PS2 during recalibration. Result validation therefore preserves exact wire integrity while allowing consumed MPEG bytes to be less than enqueued bytes only through the already-proven generation-retirement boundary.

A first complete compile/link proof was not treated as sufficient. During pre-hardware source review, the generated CP2P decoder was found to retain a standalone-GS first-picture prerequisite requiring `transfer_packet` and `draw_packet` to be non-NULL. CP2P deliberately removed those packet owners when it moved MPEG presentation into the shared compositor, so a real first decoded picture would have failed despite a green link. The generator was hardened to remove and forbid those obsolete checks. Expected exact-generation cancellation during prefill/decode is now a clean successful exit, while the generation worker carries a sticky `failure_latched` bit for any unexpected decoder/runtime exit. Final session validation rejects an unexpected worker failure even if an earlier frame had already been displayed.

The first apply proof also exposed a proof-harness-only link omission: the host activation-gate test initially omitted `transport_protocol.c`, leaving `pstvnc_transport_read_be32` unresolved. No product commit was made from that failed run. After the harness was corrected, the all-guns implementation passed; later hardening and a clean committed-source proof were then required before seal.

The hardened product tree was first materialized as development commit `28d86b1ca68b8582a77683e5f4306fe15e3c468e`. To keep temporary staging/CI ancestry out of the official line, that exact tree (`1817b10e12c43f62eb6e73872602ebf1a6a0bbba`) was re-parented directly onto sealed #11B, producing official item-#10 source commit:

`6c24fddbf4618d2e0bf69eba909be640dcff005f`

`h1: open and harden CP2P item10 all-guns activation gate`

The official #10 product commit is exactly one commit ahead of #11B and contains only durable activation/runtime/test/documentation files; no temporary workflow, staging helper, symbol dump, or identity file is present.

Clean committed-source verification used proof-wrapper commit `a09fd93b02b4604370370d21626712b28c183ca8`, whose only delta over the proven product tree is `.github/workflows/h1-cp2p-item10-clean-proof.yml`. GitHub Actions run `34718230541` completed SUCCESS:

- host-contracts job `103619197119` — SUCCESS;
- pinned PS2 build job `103619197057` — SUCCESS;
- CP2O visible-RFB/PCM source/object and mux regressions — PASS;
- CP2P all-guns CONFIG gate contract — PASS;
- all prior START/suppression/capture/producer/retirement/generation-boundary host contracts — PASS;
- calibration/session host suite — PASS;
- generated CP2P runtime contains no obsolete standalone transfer/draw-packet first-frame guard — PASS;
- exact cancellation and sticky unexpected-worker-failure checks — PASS;
- `H1_CP2P_ITEM10_PUBLIC_MPEG_GATE=LIVE` — PASS;
- `H1_CP2P_MPEG_WORKER=GENERATION_BOUND_START_ACTIVATED` — PASS;
- `H1_CP2P_ITEM10_CLEAN_PS2=PASS`;
- no tracked source mutation after either host or PS2 proof job.

Pinned toolchain authority remained:

`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Exact item-#10 unqualified all-guns candidate identity:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`;
- ELF bytes `3238996`;
- PT_LOAD segments `1`;
- PT_LOAD SHA256 `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`;
- PT_LOAD bytes `512276`;
- artifact `h1-cp2p-item10-all-guns-unqualified-elf`;
- artifact ID `10305431287`;
- artifact ZIP SHA256 `15f06494487e0f4aeb124601ec24b70d0d425add201ab36b77891baf787b1974`.

**Checklist transition:** item #10 — exact CP2P CONFIG/live MPEG gate — **OPEN -> DONE at the software-proof boundary**. Items #1-#11B are now mechanically implemented/proven at their intended software boundaries. The remaining milestone is **#12**: freeze the complete PS2/Pi/config/toolchain/deployment identities and perform actual all-guns hardware qualification, including visible RFB + PCM + calibrated MPEG, first-frame ownership, high-change RFB outside suppression, exact capture geometry, retirement/recalibration N -> full RFB -> N+1, stale-generation exclusion, and continued input/keyboard/OSK/local-UI operation.

No live physical PS2 all-guns behavior or hardware stability is claimed by item #10. The artifact remains explicitly **unqualified** until #12 hardware evidence exists.
'''

new_history = old_history.rstrip() + entry + "\n"
if not new_history.startswith(old_history.rstrip()):
    raise SystemExit("history prefix preservation failed")
if new_history.count("## 43. Item #10") != 1:
    raise SystemExit("history item #10 section count mismatch")
HISTORY.write_text(new_history)

print("H1_CP2P_ITEM10_DOC_SEAL=APPLIED")
