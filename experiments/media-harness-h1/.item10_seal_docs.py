from pathlib import Path

CONTRACT = Path('experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md')
HISTORY = Path('experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md')

contract = CONTRACT.read_text()
history = HISTORY.read_text()
old_history = history


def replace_once(old: str, new: str) -> None:
    global contract
    count = contract.count(old)
    if count != 1:
        raise SystemExit(f'expected exactly one contract match, got {count}: {old[:120]!r}')
    contract = contract.replace(old, new, 1)

replace_once(
'''    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]\n    -> keep MPEG output local/bounded while public gate closed [item #10 OPEN]\n''',
'''    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]\n    -> open exact-generation emission admission for accepted N  [item #10 DONE]\n    -> send channel-4 MPEG only under N's emission lease         [item #10 DONE]\n''')

replace_once(
'''- the producer is attached to the existing H1 session as the video producer; no second PS2-facing socket, transport reader, or bypass writer is introduced;\n- producer output is locally bounded/backpressured and archived for evidence, but item #10's public MPEG CONFIG gate remains closed, so item #8 emits **zero PSTV MPEG DATA**;\n- only one MPEG producer generation may be active at a time; wrong-generation retirement is rejected;\n''',
'''- the producer is attached to the existing H1 session as the video producer; no second PS2-facing socket, transport reader, or bypass writer is introduced;\n- item #10 now opens the CP2P-only MPEG CONFIG/emission composition: START remains the sole producer authority, while each Pi->PS2 MPEG DATA send must acquire the exact active generation's emission lease and use the existing serialized PSTV writer;\n- the all-guns profile is registered only by the CP2P runner; generic H1 and CP2O retain their previous composition policy and cannot accidentally select the CP2P MPEG lifecycle;\n- only one MPEG producer generation may be active at a time; wrong-generation retirement is rejected;\n''')

replace_once(
'''Item #11B is now concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 remains responsible for actually opening the public MPEG CONFIG/emission gate and exercising this boundary with live MPEG DATA.\n''',
'''Item #11B is now concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 now uses this boundary for the public all-guns software composition: generation N is opened before START, each MPEG send is protected by N's exact emission lease, retirement makes the lease one-way closed, and no fresh generation can emit until #11B's old-epoch finalization is clean.\n''')

replace_once(
'''- while item #10 remains closed, the generation-owned producer has no PSTV MPEG DATA send path and cannot bypass the existing writer;\n- the item-#11B Pi emission fence is closed by default, exact-generation only, and RETIRE waits for an in-flight emission lease before producer stop/drain (`h1_cp2p_mpeg_producer_test.py`);\n''',
'''- item #10 opens MPEG only for the CP2P all-guns composition; CP2O retains its prior visible-RFB/PCM policy (`h1_config_cp2p_activation_gate_test.c`, CP2O gate regression);\n- START remains the sole producer authority; CONFIG enables capability but does not start FFmpeg; live channel-4 sends use the existing serialized writer only after an exact-generation emission lease is acquired (`h1_cp2p_item10_activation_test.py`);\n- the item-#11B Pi emission fence is closed by default, exact-generation only, and RETIRE waits for an in-flight emission lease before producer stop/drain (`h1_cp2p_mpeg_producer_test.py`);\n''')

replace_once(
'''- item #10 remains closed: the CP2P runner does not call `open_emission_exact()` and the public PS2 CP2P CONFIG gate still requires video OFF;\n- RFB and PCM ownership paths remain alive while MPEG is active;\n- only the existing PSTV writer emits PS2-facing frames.\n''',
'''- the CP2P-only all-guns profile accepts MPEG2_ES without broadening generic H1/CP2O policy;\n- finite-session MPEG accounting is derived from payloads actually sent on PSTV, including streaming picture/start-code observation rather than unsent producer archives;\n- generated CP2P runtime contains no obsolete standalone-GS `transfer_packet`/`draw_packet` first-frame prerequisite, and unexpected worker exit is sticky-failed while expected retirement cancellation remains valid;\n- RFB and PCM ownership paths remain alive while MPEG is active;\n- only the existing PSTV writer emits PS2-facing frames.\n''')

replace_once(
'''| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, keep output locally bounded while #10 is closed, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |\n| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean; item #10 may open emission only through this fence |\n''',
'''| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, expose output only through the item-#10 leased scheduler, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |\n| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; item-#10 scheduler integration in `h1_mux_server_cp2p_start_receiver.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, leased MPEG DATA sends, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean |\n| CP2P all-guns activation | PS2 `h1_config_cp2p_activation_gate.c`, CP2P main/makefile; Pi CP2P-only profile/scheduler in `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_item10_activation_test.py`; `CP2P_ITEM10_ALL_GUNS_ACTIVATION.md` | preserve CP2P-only MPEG capability, START-owned producer activation, exact-generation leased channel-4 sends, truthful sent-stream accounting, and unchanged CP2O policy |\n''')

replace_once(
'''| verification | items #5/#6/#7/#8/#11A/#11B host contracts; clean #11B run `34713445142` (host `103606261825`, PS2 `103606261723`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, Pi emission lease/fence, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, and closed item-#10 public gate in fresh-install self-check plus project regressions |\n''',
'''| verification | items #5/#6/#7/#8/#11A/#11B/#10 host contracts; clean #10 run `34718230541` (host `103619197119`, PS2 `103619197057`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, Pi emission lease/fence, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, CP2P-only all-guns CONFIG, leased MPEG scheduling, truthful wire accounting, generated-runtime first-frame guard regression, and CP2O preservation in fresh-install self-check plus project regressions |\n''')

replace_once(
'''- generation-owned START-driven Pi MPEG producer: `experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`\n- producer lifecycle + exact-generation emission-fence contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`\n''',
'''- generation-owned START-driven Pi MPEG producer: `experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`\n- producer lifecycle + exact-generation emission-fence contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`\n- CP2P-only all-guns Pi scheduler/profile + finite-session accounting: `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`\n- CP2P all-guns activation host contract: `experiments/media-harness-h1/h1_cp2p_item10_activation_test.py`\n- PS2 CP2P-only CONFIG gate: `experiments/media-harness-h1/h1_config_cp2p_activation_gate.c`\n- item-#10 implementation/proof note: `experiments/media-harness-h1/CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`\n''')

replace_once(
'''- item #8 START-driven MPEG production is DONE at the pre-public-gate software boundary: suppression and exact capture preparation precede one generation-owned FFmpeg launch, output remains locally bounded/backpressured, and no PSTV MPEG DATA is emitted while item #10 remains closed;\n''',
'''- item #8 START-driven MPEG production remains the producer-lifecycle authority: suppression and exact capture preparation precede one generation-owned FFmpeg launch; item #10 now exposes that producer's output through the exact-generation leased MPEG scheduler rather than creating a second activation path;\n''')

replace_once(
'''- item #11B live cross-machine MPEG generation boundary is DONE at the pre-public-gate software boundary: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;\n- item #10 remains CLOSED by design and is now the next implementation tranche: it must open the public CP2P MPEG CONFIG gate and route live MPEG DATA only through item #11B's exact-generation emission lease and existing serialized writer;\n''',
'''- item #11B live cross-machine MPEG generation boundary is DONE: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;\n- item #10 exact CP2P CONFIG/all-guns activation is DONE at the committed-source software-proof boundary: CP2P alone accepts MPEG2_ES, START remains the sole producer authority, every live channel-4 send is protected by the exact-generation emission lease and existing serialized writer, retirement cannot reopen a closing generation, sent-stream MPEG accounting is truthful, CP2O behavior remains unchanged, and generated CP2P first-frame presentation no longer depends on obsolete standalone-GS packets;\n''')

replace_once(
'''- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;\n- physical all-guns hardware qualification has not yet occurred.\n''',
'''- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;\n- item #10 official product source is branch `experiment/h1-cp2p-all-guns-activation`, commit `6c24fddbf4618d2e0bf69eba909be640dcff005f`, built as a clean one-commit continuation from the sealed #11B line; development product tree `28d86b1ca68b8582a77683e5f4306fe15e3c468e` was reparented without changing its durable tree;\n- item #10 clean committed-source proof is GitHub Actions run `34718230541`, host job `103619197119`, PS2 build job `103619197057`; proof wrapper `a09fd93b02b4604370370d21626712b28c183ca8` differs from the product tree only by the clean-proof workflow;\n- item #10 pinned all-guns software-candidate identity is ELF SHA256 `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`, ELF bytes `3238996`, PT_LOAD SHA256 `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`, PT_LOAD bytes `512276`; unqualified artifact `h1-cp2p-item10-all-guns-unqualified-elf` is artifact ID `10305431287`;\n- physical all-guns hardware qualification has not yet occurred; item #12 is now the remaining qualification milestone.\n''')

entry = r'''

---

# PHASE S — SEPTEMBER 12 ALL-GUNS SOFTWARE ACTIVATION

## 43. Item #10 — CP2P all-guns CONFIG and leased MPEG DATA path close at the committed-source software boundary

With #11B sealed, the last software activation milestone moved onto `experiment/h1-cp2p-all-guns-activation`. The requirement remained narrow: expose the already-built RFB + PCM + generation-owned MPEG mechanism without changing the architecture that made the earlier milestones safe.

The final official item-#10 product commit is:

`6c24fddbf4618d2e0bf69eba909be640dcff005f`

`h1: activate CP2P all-guns MPEG path`

It is a one-commit continuation directly from sealed #11B head `1449bfadad653c420fe3498a8c3673afcf593be7`. Development and hardening produced equivalent durable product tree `28d86b1ca68b8582a77683e5f4306fe15e3c468e`; that tree was reparented onto the clean official line so temporary staging/proof ancestry is not part of the project continuation.

### CP2P-only CONFIG activation

The generic H1 validator already understood MPEG2_ES. The prior block was composition policy: CP2O deliberately allowed visible RFB plus optional PCM with video OFF, and the CP2P main retained an explicit video-OFF restriction.

Item #10 does not weaken the generic validator or CP2O. It adds the CP2P-specific gate `h1_config_cp2p_activation_gate.c` and selects it only in the CP2P makefile. The CP2O gate remains its previous visible-RFB/PCM policy and is rebuilt/regression-tested separately.

The Pi follows the same rule. The all-guns profile is registered only by the CP2P runner rather than globally. CONFIG therefore enables the CP2P MPEG capability but does **not** start the encoder. The immutable START transaction created in earlier items remains the only authority that installs suppression, prepares exact capture geometry, launches the exact-generation FFmpeg producer, arms the PS2 worker, and selects generation ownership.

### Live MPEG scheduling uses the #11B fence

The generation-owned producer remains bounded and is not a PSTV writer. Item #10 adds scheduling at the existing sole serialized writer. Generation N must first be the exact prepared/active generation, then its #11B emission fence is opened. Each Pi->PS2 channel-4 MPEG DATA send acquires an exact-generation lease and releases it after the existing writer returns.

Retirement is one-way for the retiring generation. Once RETIRE starts, N cannot reopen its emission fence. RETIRE waits for any in-flight lease, then the already-proven #8/#11A/#11B chain stops/drains the producer, removes suppression/prepared state, emits the ordered completion fence, stops the old PS2 worker, discards residual old-generation ring data, returns withheld credit, and only then permits a fresh generation.

No new MPEG socket, queue, writer, PS2 receive owner, or per-packet generation header was added.

### Finite-session accounting and failure visibility

Opening MPEG DATA exposed two validation details that were deliberately closed before hardware qualification.

First, finite-session MPEG result accounting is based on bytes and start codes actually sent through PSTV, not on the producer archive. Producer output that never acquired a send lease can be discarded during retirement and must not be reported as wire traffic. The CP2P scheduler therefore maintains streaming MPEG picture/start-code accounting on the payloads actually emitted by the serialized writer.

Second, a source audit found that the generated compositor-aware CP2P video runtime still inherited a standalone-GS first-picture guard requiring `transfer_packet` and `draw_packet`. Those packet owners are intentionally removed in CP2P because presentation belongs to the shared compositor, so the guard would have rejected the first decoded picture on real hardware even though compile/link proof was green. The generator was hardened to remove that obsolete prerequisite, and the clean PS2 proof explicitly checks the generated source does not contain it.

Worker failure semantics were hardened at the same time. Expected generation retirement/cancellation remains valid, but an MPEG worker that exits on its own before stop is requested latches `failure_latched`; the session cannot mask a decoder/runtime failure merely because one or more frames were previously displayed.

### Clean committed-source proof

Development hardening product source was `28d86b1ca68b8582a77683e5f4306fe15e3c468e`. CI-only proof wrapper `a09fd93b02b4604370370d21626712b28c183ca8` adds only the clean-proof workflow. GitHub Actions run `34718230541` checked out that committed source and completed SUCCESS with no tracked source mutation:

- host-contracts job `103619197119` — SUCCESS;
- pinned PS2-build job `103619197057` — SUCCESS;
- pinned image `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`.

The host job reran the item-#10 CP2P gate/scheduler contract plus the prior START, RFB bridge/suppression, capture geometry, compound rollback, retirement, producer, generation-boundary, and calibration/session suites. The PS2 job rebuilt CP2O regression and the hardened CP2P target, generated the compositor-aware runtime, asserted the obsolete transfer/draw-packet guard was absent, linked the generation worker and transport epoch machinery, and left the working tree unchanged.

Important emitted markers include:

- `H1_CP2O_ITEM10_FINAL_REGRESSION=PASS`;
- `H1_CP2P_APPLICATION_LINK=PASS`;
- `H1_CP2P_SESSION_COORDINATOR=LIVE`;
- `H1_CP2P_ACCEPT_TO_START=LIVE`;
- `H1_CP2P_GRAPHICS_OWNER=SHARED_COMPOSITOR`;
- `H1_CP2P_MPEG_WORKER=GENERATION_BOUND_START_ACTIVATED`;
- `H1_CP2P_ITEM10_PUBLIC_MPEG_GATE=LIVE`;
- `H1_CP2P_MPEG_CANCELLABLE_READ=LIVE`;
- `H1_CP2P_ITEM10_CLEAN_PS2=PASS`;
- `TESTKIT_PT_LOAD_FINGERPRINT=PASS`.

Exact all-guns software-candidate identity:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`;
- ELF bytes `3238996`;
- PT_LOAD SHA256 `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`;
- PT_LOAD bytes `512276`;
- unqualified artifact `h1-cp2p-item10-all-guns-unqualified-elf`, artifact ID `10305431287`;
- artifact ZIP SHA256 `15f06494487e0f4aeb124601ec24b70d0d425add201ab36b77891baf787b1974`.

### Checklist transition

Item **#10 — Exact CP2P CONFIG/all-guns activation — OPEN -> DONE at the committed-source software-proof boundary**.

At this point the software checklist through #11B is closed: accepted calibration, exact START, Pi validation/suppression/capture, START-owned FFmpeg, generation-bound PS2 worker, Pi-first retirement, ordered stale-byte epoch boundary, and the public leased MPEG DATA path are all implemented and software-proven together.

Item **#12 remains OPEN**. The artifact above is deliberately named *unqualified*. No claim is made yet that the Pi has been deployed with this exact runtime, that this exact ELF has run on the physical PS2, that simultaneous high-change RFB + PCM + MPEG is stable, or that recalibration/retirement works on hardware. Those provenance/deployment/physical observations are the next milestone.

Preferred remaining order is therefore simply:

`#12 — freeze/deploy exact candidate -> physical all-guns qualification -> seal immutable hardware evidence`.
'''

if '# PHASE S — SEPTEMBER 12 ALL-GUNS SOFTWARE ACTIVATION' in history:
    raise SystemExit('history already contains item10 seal entry')
history = history.rstrip() + entry + '\n'

CONTRACT.write_text(contract.rstrip() + '\n')
HISTORY.write_text(history)

if not history.startswith(old_history.rstrip()):
    raise SystemExit('history prefix changed')
if history.count('# PHASE S — SEPTEMBER 12 ALL-GUNS SOFTWARE ACTIVATION') != 1:
    raise SystemExit('history seal count mismatch')
print('H1_CP2P_ITEM10_DOC_SEAL_PREP=PASS')
