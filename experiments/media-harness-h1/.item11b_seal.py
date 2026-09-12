#!/usr/bin/env python3
"""One-shot seal for H1 CP2P item #11B."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CONTRACT = ROOT / "experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md"
HISTORY = ROOT / "experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md"


def rep(text, old, new, label):
    n = text.count(old)
    if n != 1:
        raise SystemExit(f"{label}: expected one stale block, found {n}")
    return text.replace(old, new, 1)

contract = CONTRACT.read_text()
contract = rep(contract,
    "Item #11B remains responsible for the fully live data-plane generation boundary once MPEG bytes are allowed onto PSTV: stale generation-N bytes must not remain consumable by a fresh N+1 worker, and the one-reader/one-writer architecture must remain intact.",
    "Item #11B is now concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 remains responsible for actually opening the public MPEG CONFIG/emission gate and exercising this boundary with live MPEG DATA.",
    "item 11B retirement paragraph")
contract = rep(contract,
    "- while item #10 remains closed, the generation-owned producer has no PSTV MPEG DATA send path and cannot bypass the existing writer;\n- RFB and PCM ownership paths remain alive while MPEG is active;",
    "- while item #10 remains closed, the generation-owned producer has no PSTV MPEG DATA send path and cannot bypass the existing writer;\n- the item-#11B Pi emission fence is closed by default, exact-generation only, and RETIRE waits for an in-flight emission lease before producer stop/drain (`h1_cp2p_mpeg_producer_test.py`);\n- the shared PS2 queue discard primitive empties wrapped residual data without reallocating or lowering queue high-water telemetry (`transport_queue_generation_boundary_test.c`);\n- the PS2 coordinator opens one exact MPEG transport generation before START, aborts it on START-send failure, latches the ordered RETIRE completion, stops the old worker before residual queue discard/final-credit return, and cannot open N+1 until the old epoch is finalized (`h1_cp2p_session_coordinator_test.c`);\n- item #10 remains closed: the CP2P runner does not call `open_emission_exact()` and the public PS2 CP2P CONFIG gate still requires video OFF;\n- RFB and PCM ownership paths remain alive while MPEG is active;",
    "reconstruction tests 11B")
contract = rep(contract,
    "| MPEG mux scheduling | implementation still to be made concrete | channel-4 queue/credit/scheduling support without second socket |",
    "| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean; item #10 may open emission only through this fence |",
    "promotion MPEG scheduling row")
contract = rep(contract,
    "| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`, `h1_cp2p_mpeg_producer.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10/control-channel exact request/completion, one pending generation, exact producer stop/drain before suppression removal, Pi-cleanup-before-completion, and Pi-completion-before-local-retire/full-RFB ordering; #11B adds stale live-data exclusion across generations |",
    "| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`, `h1_cp2p_mpeg_producer.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10 exact retirement, Pi producer/emission quiescence before completion, completion as the ordered receive-side MPEG epoch fence, old-worker stop before residual queue discard/final-credit return, and only then local presentation retirement/full-RFB restoration |",
    "promotion retirement row")
contract = rep(contract,
    "| verification | items #5/#6/#7/#8 host tests plus item #11A exact-retirement tests; proof runs `34702828032`, `34709167702`, `34710050027`, `34710972431`, item #8 clean run `34712568840` | include START, same-reader, dynamic suppression, exact capture, generation-owned producer start/stop/drain, fail-closed post-launch rollback, exact retirement request/completion, closed-public-gate/no-bypass-writer, and Pi-completion-before-local-retire contracts in fresh-install self-check plus project regressions |",
    "| verification | items #5/#6/#7/#8/#11A/#11B host contracts; clean #11B run `34713445142` (host `103606261825`, PS2 `103606261723`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, Pi emission lease/fence, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, and closed item-#10 public gate in fresh-install self-check plus project regressions |",
    "promotion verification row")
contract = rep(contract,
    "- producer lifecycle contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`\n- compound START/producer rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`",
    "- producer lifecycle + exact-generation emission-fence contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`\n- compound START/producer rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`\n- live MPEG epoch-boundary design authority: `experiments/media-harness-h1/CP2P_LIVE_GENERATION_BOUNDARY.md`\n- shared residual-queue discard authority: `experiments/audio-transport/common/transport_queue.[ch]` and `transport_queue_generation_boundary_test.c`",
    "source map 11B")
contract = rep(contract,
    "- item #11B is the next implementation tranche and remains responsible for the fully live cross-machine data-plane generation boundary, especially excluding/draining stale old-generation MPEG bytes so generation N data can never feed N+1;\n- item #10 remains CLOSED and remains the later authority that will expose MPEG DATA through public CP2P CONFIG only after #11B is safe;",
    "- item #11B live cross-machine MPEG generation boundary is DONE at the pre-public-gate software boundary: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;\n- item #10 remains CLOSED by design and is now the next implementation tranche: it must open the public CP2P MPEG CONFIG gate and route live MPEG DATA only through item #11B's exact-generation emission lease and existing serialized writer;",
    "current boundary 11B status")
contract = rep(contract,
    "- item #8 clean proof is GitHub Actions run `34712568840`, host job `103603873398`, PS2 regression job `103603873498`;\n- the pinned PS2 regression retained the #11A binary identity exactly: ELF SHA256 `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`, ELF bytes `3232156`, PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`, PT_LOAD bytes `510612`;",
    "- item #8 clean proof is GitHub Actions run `34712568840`, host job `103603873398`, PS2 regression job `103603873498`;\n- item #11B product source authority is branch `experiment/h1-cp2p-live-generation-boundary`, commit `14d2b5fc68e11756356af397cfe3766e03c89008`; clean committed-source verification used wrapper head `bb56f36b4b0071e87819b338b4efa98a1e00740d`, whose only product-independent delta is the one-line CI trigger comment;\n- item #11B clean proof is GitHub Actions run `34713445142`, host job `103606261825`, PS2 build job `103606261723`;\n- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;",
    "current boundary proof identity")
CONTRACT.write_text(contract)

old = HISTORY.read_text()
heading = "## 42. Item #11B — ordered PSTV epoch fence closes the stale-generation gap"
if heading in old:
    raise SystemExit("history section 42 already exists")
entry = r'''

---

# PHASE R — SEPTEMBER 12 LIVE MPEG GENERATION BOUNDARY

## 42. Item #11B — ordered PSTV epoch fence closes the stale-generation gap

After item #8 was sealed at `12c88738f6c7ce4fc70ac62c6dd4908dd0b03c8a`, work moved to the last data-plane safety prerequisite before public MPEG activation on branch `experiment/h1-cp2p-live-generation-boundary`.

The unresolved risk was specific: the PS2 MPEG ring is session-scoped, so once MPEG DATA is live a fresh generation N+1 must never consume residual generation-N bytes. The architecture already supplied a smaller exact boundary than tagging every MPEG frame: PSTV is one ordered TCP stream with one Pi writer and one PS2 receiver.

Item #11B therefore treats exact RETIRE completion as an **epoch wire fence**. Before Pi completion for N, `H1Cp2pMpegProducer` closes new generation-N emission admission and waits for every already-issued send lease. Producer stop/drain and the existing Pi cleanup then finish before completion is written. When the sole PS2 receiver observes that completion, every earlier N DATA frame has already arrived in stream order; it immediately closes channel-4 generation admission and leaves the retirement transaction latched. The coordinator stops the N worker, then transport finalization discards the finite residual N bytes from the existing ring under its semaphore, returns withheld credit, and clears the latch. N+1 may open only when old generation, retirement, queue, and pending-credit state are clean.

That is the minimal architecture: **one queue, one socket, one receiver, one writer, and an ordered epoch boundary**. No per-packet generation tag, second MPEG queue, alternate socket, receive owner, or send owner was introduced.

The Pi producer gained a closed-by-default exact-generation emission lease (`open_emission_exact`, `begin_emission_exact`, `finish_emission_exact`). Item #10 is the future caller that may open it. The PS2 transport gained `pstvnc_h1_transport_mpeg_generation_open`, `pstvnc_h1_transport_mpeg_generation_abort`, latched retirement polling, `pstvnc_h1_transport_mpeg_retire_finalize`, and the O(1) `pstvnc_transport_queue_discard_all` primitive while preserving queue high-water telemetry.

The first proof attempt exposed a proof-helper defect rather than a product defect: an overly broad idempotency marker saw the newly defined coordinator retirement wrapper and incorrectly skipped its call-site replacement, so the strict host build rejected the wrapper as unused. The helper was corrected to exact contextual markers, and two other broad markers were tightened before rerun; no product invariant was weakened.

The corrected proof produced product-source commit `14d2b5fc68e11756356af397cfe3766e03c89008` (`h1: add exact live MPEG generation boundary`). Since Actions-bot product commits do not recursively trigger another workflow, product-independent wrapper `bb56f36b4b0071e87819b338b4efa98a1e00740d` adds only a one-line workflow trigger comment for committed-source verification.

Clean run `34713445142` completed SUCCESS: host-contracts `103606261825` and ps2-build `103606261723`. It proved the queue boundary, all prior Pi contracts, the exact emission fence including an in-flight lease, the full calibration/session suite including coordinator epoch ordering, `H1_CP2P_ITEM11B_PUBLIC_GATE=CLOSED`, a clean committed-source tree, and the pinned linked PS2 generation-boundary symbols.

Exact #11B PS2 proof identity under pinned `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`;
- ELF bytes `3238440`;
- PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`;
- PT_LOAD bytes `512020`;
- `H1_CP2P_ITEM11B_PS2_LINK=PASS` and `TESTKIT_PT_LOAD_FINGERPRINT=PASS`.

**Checklist transition:** #11B stale/live MPEG generation boundary is **OPEN/PARTIAL -> DONE at the pre-public-gate software-proof boundary**. #10 remains deliberately CLOSED and is now next: open the all-guns CP2P MPEG CONFIG path and send live channel-4 MPEG only through #11B's emission lease and the existing serialized PSTV writer. #12 remains the immutable/hardware-candidate milestone.

No live all-guns MPEG transport, deployment, physical PS2 behavior, or hardware stability is claimed here. Preferred remaining order is **#10 -> #12**.
'''
new = old + entry
if not new.startswith(old) or new.count(heading) != 1:
    raise SystemExit("captain's-log append-only invariant failed")
HISTORY.write_text(new)

for rel in (
    ".github/workflows/h1-cp2p-item11b-generation-boundary.yml",
    "experiments/media-harness-h1/.item11b_patch.py",
    "experiments/media-harness-h1/.item11b_patch_fix.py",
    "experiments/media-harness-h1/.item11b_seal.py",
):
    p = ROOT / rel
    if not p.exists():
        raise SystemExit(f"expected temporary file missing: {rel}")
    p.unlink()
print("H1_CP2P_ITEM11B_SEAL=PASS")
