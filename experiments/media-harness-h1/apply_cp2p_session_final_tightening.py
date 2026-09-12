#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[2]
h1 = root / 'experiments/media-harness-h1'
cal = h1 / 'mpeg_presentation_calibration'

# Make the temporary controller chord visibly an input adapter, not a lifecycle
# contract. A future UI button should replace only this request source.
interaction = h1 / 'h1_interaction_coordinator.c'
text = interaction.read_text()
old = '''#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD \\
    (PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT)'''
new = '''/*
 * Temporary hardware-test input adapter only. The session/MPEG lifecycle is
 * deliberately trigger-agnostic so a later local-UI button can replace this
 * chord without changing retirement, RFB restoration, START, or ownership.
 */
#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD \\
    (PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT)'''
if old not in text:
    raise SystemExit('temporary chord comment anchor missing')
text = text.replace(old, new, 1)

# CP2O does not consume CP2P's accepted-event snapshot. A later accepted event
# therefore replaces the snapshot instead of turning an otherwise-valid CP2O
# calibration into an application error.
old = '''    if (coordinator->accepted_calibration_pending)\n        return 0;\n\n    committed_region ='''
new = '''    /*\n     * CP2P consumes this snapshot from the same service cadence. Other H1\n     * compositions may have no consumer; in that case retain the latest accepted\n     * settings without changing their calibration behavior.\n     */\n    committed_region ='''
if old not in text:
    raise SystemExit('accepted snapshot guard anchor missing')
interaction.write_text(text.replace(old, new, 1))

# A diagnostic counter must never create a post-send failure. Check it before
# prepare_start()/START so every wire-visible generation remains locally owned.
session = h1 / 'h1_cp2p_session_coordinator.c'
text = session.read_text()
old = '''    memset(&contract, 0, sizeof(contract));\n    if (!pstvnc_h1_mpeg_start_handoff_prepare_start('''
new = '''    if (coordinator->start_messages_sent == UINT32_MAX)\n        return 0;\n\n    memset(&contract, 0, sizeof(contract));\n    if (!pstvnc_h1_mpeg_start_handoff_prepare_start('''
if old not in text:
    raise SystemExit('pre-prepare START anchor missing')
text = text.replace(old, new, 1)
old = '''\n    if (coordinator->start_messages_sent == UINT32_MAX) {\n        (void)pstvnc_h1_mpeg_start_handoff_abort_start(\n            &coordinator->mpeg_handoff,\n            contract.generation);\n        return 0;\n    }\n\n    coordinator->current_start_contract = contract;'''
new = '''\n    coordinator->current_start_contract = contract;'''
if old not in text:
    raise SystemExit('post-send counter guard anchor missing')
session.write_text(text.replace(old, new, 1))

# Lock the pre-send rule into the host contract.
test = cal / 'h1_cp2p_session_coordinator_test.c'
text = test.read_text()
anchor = '''    assert(!coordinator.current_start_contract_valid);\n\n    send_success = 1;\n    region = sample_region(128, 80);'''
insert = '''    assert(!coordinator.current_start_contract_valid);\n\n    /* Saturated diagnostics cannot put an unaccounted START on the wire. */\n    send_success = 1;\n    coordinator.start_messages_sent = UINT32_MAX;\n    region = sample_region(112, 72);\n    coordinator.interaction.accepted_calibration_region = region;\n    coordinator.interaction.accepted_calibration_pending = 1;\n    assert(!pstvnc_h1_cp2p_session_coordinator_service(\n        &coordinator, &rfb_session));\n    assert(send_count == 2u);\n    assert(pstvnc_h1_mpeg_presentation_owner_state(\n        &coordinator.mpeg_handoff.owner) ==\n        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);\n    coordinator.start_messages_sent = 2u;\n\n    region = sample_region(128, 80);'''
if anchor not in text:
    raise SystemExit('session overflow test anchor missing')
test.write_text(text.replace(anchor, insert, 1))

# Append the source-readback correction chronologically; do not rewrite Phase K.
history = h1 / 'CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md'
with history.open('a') as handle:
    handle.write('''\n\n### 34. September 12 — post-proof coordinator readback tightens boundaries\n\nAfter the first green session-coordinator proof, source readback found two small boundary conditions. CP2O does not consume the CP2P accepted-event snapshot, so a later accepted calibration now replaces an unconsumed snapshot instead of causing an interaction error. Separately, the START diagnostic counter guard was moved before `prepare_start()`/wire send so a bookkeeping limit can never create a wire-visible START followed by local abort. The session host contract now explicitly checks that saturated accounting sends no START and arms no generation.\n\nThe START+SELECT entry chord is also recorded here as a temporary hardware-test shortcut, not part of the settled lifecycle contract. The session coordinator remains trigger-agnostic. The intended later product path is a local-UI button feeding the same interaction request boundary; that change should not alter MPEG retirement, one-full-RFB restoration, immutable generation creation, START serialization, or first-frame ownership.\n''')

# One-shot proof scaffolding must not survive the final tree.
for relative in [
    '.github/workflows/cp2p-session-final-check.yml',
    'experiments/media-harness-h1/apply_cp2p_session_final_tightening.py',
]:
    path = root / relative
    if path.exists():
        path.unlink()
