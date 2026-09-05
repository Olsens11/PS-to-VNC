#!/usr/bin/env python3
"""
Temporary Issue #39 source-dictionary curation helper.

This script is intentionally hosted on an automation-only Git branch and is
not part of the product branch.  It rewrites only the Description column of
source-dictionary rows whose Issue #39 semantics are already mechanically
inventoried, regenerates the aggregate portal, runs repository-native gates,
then commits and pushes only the three dictionary artifacts.

The prose below is grounded in the exact source comments/contracts at
reconstruct/issue39-keyboard-osk-local-ui commit
904483bc4547939e60d85b13bd7b5c7bc2208610.
"""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import os
import re
import subprocess
import sys


EXPECTED_BRANCH = "reconstruct/issue39-keyboard-osk-local-ui"
EXPECTED_HEAD = "904483bc4547939e60d85b13bd7b5c7bc2208610"
TEMP_REMOTE_BRANCH = "automation/issue39-dictionary-curation-904483b"

EXPECTED_BLOBS = {
    "src/SYMBOLS.md": "aff50569fa44bf2ca1ec40bcbb34b6cc2816aa42",
    "src/platform/SYMBOLS.md": "a45fa3195c792721cfdd689a4ab0ca2c8b43e453",
    "docs/reference/SOURCE_SYMBOL_DICTIONARIES.md":
        "2f922c5e50e441b4da0cf51d55252e977d1ce6d5",
}

DICTIONARIES = (
    Path("src/SYMBOLS.md"),
    Path("src/platform/SYMBOLS.md"),
)

PORTAL = Path("docs/reference/SOURCE_SYMBOL_DICTIONARIES.md")

ISSUE39_CONTEXTS = {
    "Issue #39: keyboard, OSK and local UI foreground model",
    "Issue #39: local overlay presentation",
}

HIGH_VALUE_KINDS = {
    "function",
    "function declaration",
    "macro",
    "enum",
    "enum value",
    "structure",
    "type",
}

IMPORTANT_VARIABLES = {
    "local_overlay_pixels",
    "osk_abc_rows",
    "osk_page_row_lengths",
    "osk_func_labels",
    "osk_func_keysyms",
    "osk_special_labels",
    "osk_font_glyphs",
    "desktop_texture_configured",
    "local_overlay_texture",
    "local_overlay_texture_configured",
    "local_overlay_width",
    "local_overlay_height",
}


def run(*args: str, capture: bool = False) -> str:
    result = subprocess.run(
        args,
        check=True,
        text=True,
        stdout=subprocess.PIPE if capture else None,
    )
    return result.stdout if capture else ""


def git(*args: str, capture: bool = False) -> str:
    return run("git", *args, capture=capture)


def parse_row(line: str) -> list[str]:
    cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
    if len(cells) != 7:
        raise RuntimeError(f"unexpected dictionary row shape: {line}")
    return cells


def render_row(cells: list[str]) -> str:
    return "| " + " | ".join(cells) + " |"


# ---------------------------------------------------------------------------
# Source-grounded semantic descriptions.
# ---------------------------------------------------------------------------

FUNCTIONS = {
    "publish_semantic_keyboard_tap":
        "Expands one resolved logical keyboard tap into a balanced native down/up sequence and publishes each event through the main-thread-owned RFB session.",
    "present_current_application_frame":
        "Composes the authoritative remote desktop with the current optional local overlay, presents the complete frame, and acknowledges only the exact local generation that reached the platform.",
    "neutralize_published_pointer_for_local_foreground":
        "Publishes a neutral click mask at the frozen cursor before local foreground ownership so a remotely held click cannot survive the transition.",
    "apply_osk_activation_result":
        "Converts one OSK activation result into local-generation dirtiness and, when produced, one semantic keyboard-tap publication.",
    "open_osk_foreground":
        "Establishes the desktop-to-OSK boundary by suspending mouse interpretation, neutralizing and rebasing remote pointer state, resetting OSK state, and entering local foreground.",
    "apply_local_controller_action":
        "Executes one already-routed semantic local action without interpreting physical controller vocabulary or taking controller-device ownership.",
    "service_controller_state":
        "Routes one trustworthy controller observation through local foreground policy, executes its semantic actions, and completes transition quarantine only after physical release is proven.",
    "resume_desktop_mouse_if_ready":
        "Resumes desktop mouse interpretation only after desktop ownership is restored, quarantine is clear, and the no-overlay local generation has actually been presented.",

    "input_runtime_map_native_buttons":
        "Translates native PS2SDK button observations into the stable project-owned active-high controller-bit vocabulary.",
    "input_runtime_publish_controller_state":
        "Publishes one trustworthy physical-controller observation as an atomic semantic controller-state event for application routing.",
    "input_runtime_enter_mouse_interpretation_suspension":
        "Establishes the worker-side local-foreground boundary by discarding pre-boundary queued input, clearing transient mouse response history, preserving durable interaction state, and recording physical-controller continuity.",
    "pstvnc_input_runtime_suspend_mouse_interpretation":
        "Requests mouse-interpretation suspension and waits for worker acknowledgement that the foreground boundary is established while ordinary libpad polling remains live.",
    "pstvnc_input_runtime_rebase_suspended_mouse_state":
        "Reconciles the acknowledged suspended mouse interpreter to main's last-successfully-published neutral pointer state while preserving persistent wheel mode unless physical continuity was lost.",
    "pstvnc_input_runtime_resume_mouse_interpretation":
        "Withdraws a successfully rebased suspension only after higher-level release quarantine is complete, applying a hard mouse reset first if physical continuity was lost during the suspension epoch.",

    "pstvnc_keyboard_build_tap_sequence":
        "Builds one balanced logical key tap by pressing Shift, Ctrl, and Alt in deterministic order, emitting target down/up, then releasing modifiers in reverse order; failure exposes zero valid events.",

    "local_controller_clear_result":
        "Clears one per-sample routing result so semantic actions from an earlier controller observation cannot leak into the next.",
    "local_controller_state_is_valid":
        "Checks that one project controller observation is internally consistent before any local foreground action is derived from it.",
    "local_controller_append_action":
        "Appends one semantic local action to the bounded per-sample routing result and fails instead of overflowing fixed capacity.",
    "pstvnc_local_controller_init":
        "Initializes local-controller routing with no transition-owning physical buttons retained for release quarantine.",
    "pstvnc_local_controller_route":
        "Converts project physical-controller facts plus foreground/quarantine state into ordered local semantic actions while retaining consumed held buttons for release proof and performing no application, RFB, mouse, or display side effects.",
    "pstvnc_local_controller_release_is_proven":
        "Reports true only after every locally consumed transition-owning physical button retained for quarantine has been released.",

    "local_ui_advance_generation":
        "Advances visible local-UI generation and asserts dirty presentation authority for the newly changed local state.",
    "pstvnc_local_ui_init":
        "Initializes ordinary desktop foreground ownership with no transition quarantine and no outstanding local-only presentation work.",
    "pstvnc_local_ui_open_osk":
        "Transitions from DESKTOP to OSK foreground, records the return owner, asserts input quarantine, and advances dirty local generation atomically.",
    "pstvnc_local_ui_close_osk":
        "Returns from OSK to its recorded underlay owner while asserting fresh quarantine and dirty generation so neither the close gesture nor disappearing overlay leaks across ownership.",
    "pstvnc_local_ui_input_is_quarantined":
        "Reports whether semantic controller actions are currently blocked by a foreground-transition quarantine.",
    "pstvnc_local_ui_complete_input_quarantine":
        "Clears an active foreground-transition quarantine only after upstream input policy has proven the transition-owning gesture cannot leak into the current foreground.",
    "pstvnc_local_ui_mark_local_change":
        "Records a visible PS2-local UI mutation by advancing generation and asserting dirty presentation state without drawing or presenting pixels.",
    "pstvnc_local_ui_generation":
        "Returns the exact current local-UI generation used to correlate composition with presentation acknowledgement.",
    "pstvnc_local_ui_needs_present":
        "Reports whether the current local generation still has outstanding platform presentation work.",
    "pstvnc_local_ui_mark_presented":
        "Acknowledges only the exact current local generation after successful presentation; stale acknowledgements are rejected and leave dirty asserted.",

    "clear_presentation":
        "Clears one local-presentation descriptor so desktop-only or failed composition cannot retain stale overlay metadata.",
    "pstvnc_local_ui_prepare_presentation":
        "Builds the visual description for one exact local generation: DESKTOP yields no overlay, while OSK renders caller-owned 600x178 pixels at the adopted bottom-centered placement without acknowledging presentation.",

    "pstvnc_mouse_reset_transient_history":
        "Clears transient movement, fractional, direction, and repeat history while preserving persistent wheel mode plus durable cursor and click state across a continuous-controller foreground pause.",

    "osk_shifted_ascii":
        "Maps one adopted ABC-page printable character to the exact shifted printable character used by the restored historical OSK.",
    "osk_func_keysym":
        "Returns the native X11 keysym assigned to one valid FUNC-page cell in the adopted historical key inventory.",
    "osk_real_modifier_mask":
        "Builds the real semantic modifier mask represented by current OSK one-shot state, including Shift only when the selected action requires a real Shift bracket.",
    "osk_activation_clear":
        "Clears one OSK activation result so failed or local-state-only activations cannot expose stale keyboard work.",
    "osk_consume_modifiers_after_key":
        "Consumes one-shot Shift, Ctrl, and Alt after an actual key action and records a visible local-state change when any modifier was cleared.",
    "pstvnc_osk_reset_for_open":
        "Restores the adopted historical opening state: ABC page, row 1, column 0, with Shift, Ctrl, and Alt clear.",
    "pstvnc_osk_clear_modifiers":
        "Clears only OSK-owned one-shot Shift, Ctrl, and Alt state while leaving page and selection unchanged.",
    "pstvnc_osk_toggle_shift_modifier":
        "Toggles OSK-owned one-shot Shift without allowing the physical controller shortcut to enter the OSK behavior vocabulary.",
    "pstvnc_osk_activate_direct_key":
        "Produces one fixed logical shortcut key using current one-shot modifiers, emits real modifier brackets as required, and consumes all one-shot modifiers after the key action.",
    "pstvnc_osk_row_length":
        "Returns the adopted key count for one valid OSK page and row, or zero for invalid coordinates.",
    "pstvnc_osk_move_horizontal":
        "Moves selection within the current row using the adopted historical left/right wrapping rule.",
    "pstvnc_osk_move_vertical":
        "Moves selection between wrapped rows using the adopted historical proportional column rule new_col = old_col * new_row_length / old_row_length.",
    "pstvnc_osk_display_char":
        "Returns the exact printable ABC-page character currently displayed at one cell after Shift-layer interpretation.",
    "pstvnc_osk_key_label":
        "Returns the adopted historical textual label for one FUNC-page or permanent-utility cell; printable ABC cells remain character-driven.",
    "pstvnc_osk_activate_selected":
        "Activates the current OSK selection: page and modifier keys mutate local state, actual keys produce one semantic keyboard tap, printable ABC Shift selects a shifted keysym rather than real Shift, and every actual key consumes one-shot modifiers.",

    "pack_opaque_surface_color":
        "Packs five-bit red, green, and blue channels into the project-owned opaque 16-bit local-presentation pixel contract.",
    "fill_surface_rectangle":
        "Fills one bounded rectangle of the fixed OSK surface with a project presentation pixel.",
    "find_font_glyph":
        "Looks up one supported character in the renderer-owned built-in OSK glyph table.",
    "draw_surface_character":
        "Rasterizes one built-in glyph into caller-owned OSK surface pixels at the requested position and scale.",
    "draw_centered_surface_text":
        "Centers and draws one textual key label inside a supplied rectangle using the renderer-owned OSK font.",
    "utility_key_is_active":
        "Reports whether a utility-row page selector or one-shot modifier should render in its active visual state from the current OSK model.",
    "pstvnc_osk_render_surface":
        "Renders the complete current 600x178 OSK appearance into caller-owned project presentation pixels without controller, RFB, foreground, GS, or platform ownership.",

    "write_be32":
        "Writes one 32-bit value in big-endian RFB wire order.",
    "pstvnc_rfb_build_key_event":
        "Serializes one exact eight-byte RFB 3.x KeyEvent from explicit down/up state and a caller-resolved 32-bit X11 keysym without interpreting keyboard meaning.",
    "pstvnc_rfb_session_send_key_event":
        "Requires a READY main-thread-owned RFB session, serializes one native KeyEvent, and sends it exactly while leaving keysym, modifier, OSK, and controller meaning with the caller.",

    "configure_desktop_texture":
        "Initializes the fixed desktop gsKit texture descriptor and allocates its persistent GS VRAM backing.",
    "local_overlay_is_valid":
        "Validates optional local-overlay storage, geometry, pixel count, and placement within the logical desktop before any GS presentation work.",
    "configure_local_overlay_texture":
        "Configures or reconfigures the reusable gsKit local-overlay texture and its VRAM backing for the current overlay dimensions.",
    "pstvnc_ps2_graphics_present":
        "Presents one complete coherent logical desktop plus an optional generic caller-owned local overlay, mapping both project surfaces into private gsKit and VRAM mechanisms before synchronized execution and flip.",
}

VARIABLES = {
    "local_overlay_pixels":
        "Owns the fixed OSK-sized application staging surface for PS2-local overlay pixels without becoming authoritative remote framebuffer state.",
    "osk_abc_rows":
        "Stores the exact adopted ABC-page printable character inventory by historical row.",
    "osk_page_row_lengths":
        "Stores the exact adopted key count for every row on each supported OSK page.",
    "osk_func_labels":
        "Stores the adopted textual labels for FUNC-page cells that are label-driven.",
    "osk_func_keysyms":
        "Stores the native X11 keysyms assigned to the adopted FUNC-page cells.",
    "osk_special_labels":
        "Stores the adopted textual labels for the permanent historical utility row.",
    "osk_font_glyphs":
        "Stores the renderer-owned built-in glyph table used to reproduce adopted OSK labels.",
    "desktop_texture_configured":
        "Records whether the fixed desktop texture already owns its persistent GS VRAM allocation.",
    "local_overlay_texture":
        "Owns the reusable gsKit texture descriptor used only for the optional generic local overlay.",
    "local_overlay_texture_configured":
        "Records whether the reusable local-overlay texture currently owns valid GS VRAM backing.",
    "local_overlay_width":
        "Records the width associated with the current reusable local-overlay texture allocation.",
    "local_overlay_height":
        "Records the height associated with the current reusable local-overlay texture allocation.",
}

STRUCTURES = {
    "pstvnc_controller_state":
        "Carries one trustworthy physical-controller observation: complete active-high down state, same-epoch pressed and released edges, and a physical connection-epoch-start marker.",
    "pstvnc_keyboard_tap":
        "Carries one already-resolved logical keyboard intent: a target native X11 keysym plus only the real modifiers that must bracket it.",
    "pstvnc_keyboard_key_event":
        "Carries one native pre-RFB keyboard fact consisting of explicit down/up state and a complete 32-bit X11 keysym.",
    "pstvnc_keyboard_sequence":
        "Carries one complete bounded balanced keyboard action and the authoritative count of initialized native key events.",
    "pstvnc_local_controller_result":
        "Carries the ordered bounded semantic local actions produced from one physical-controller observation.",
    "pstvnc_local_controller":
        "Retains only locally consumed physical buttons still awaiting release proof across a foreground transition quarantine.",
    "pstvnc_local_ui":
        "Carries all main-owned local-UI authority: current and return foregrounds, transition quarantine, visible generation, presented generation, and dirty state.",
    "pstvnc_local_ui_presentation":
        "Describes one exact local-UI generation for platform presentation, including optional caller-owned overlay pixels, dimensions, logical placement, and generation identity.",
    "pstvnc_osk":
        "Carries complete OSK-owned interaction state: page, row and column selection, and one-shot Shift, Ctrl, and Alt; visibility and foreground ownership live in local_ui.",
    "pstvnc_osk_activation":
        "Reports one OSK activation as a visible local-state change and/or one complete semantic keyboard-tap intent.",
    "pstvnc_osk_font_glyph":
        "Pairs one supported character with the renderer-owned bitmap rows used to draw its built-in OSK glyph.",
    "pstvnc_ps2_graphics_overlay":
        "Describes one generic caller-owned 16-bit local overlay surface plus logical dimensions and placement while keeping product meaning outside PS2 graphics.",
}

TYPES = {
    "pstvnc_controller_state_t":
        "Provides the public value type for one trustworthy platform-neutral physical-controller observation.",
    "pstvnc_keyboard_tap_t":
        "Provides the public value type for one compact already-resolved logical keyboard-tap intent.",
    "pstvnc_keyboard_key_event_t":
        "Provides the public value type for one explicit native keyboard down/up fact before RFB encoding.",
    "pstvnc_keyboard_sequence_t":
        "Provides the public value type for one complete bounded balanced logical keyboard action.",
    "pstvnc_local_controller_action_t":
        "Provides the semantic local-controller action type consumed by application/main after physical routing.",
    "pstvnc_local_controller_result_t":
        "Provides the bounded ordered result type emitted by routing one physical-controller observation.",
    "pstvnc_local_controller_t":
        "Provides the retained local-controller quarantine state used to prove consumed-button release.",
    "pstvnc_local_ui_foreground_t":
        "Provides the semantic foreground-owner type distinguishing remote desktop interaction from PS2-local OSK ownership.",
    "pstvnc_local_ui_t":
        "Provides the main-owned local foreground, quarantine, generation, and presentation-authority value type.",
    "pstvnc_local_ui_presentation_t":
        "Provides the platform-neutral visual-description type for one exact local-UI generation.",
    "pstvnc_osk_page_t":
        "Provides the OSK page-selection type for the adopted ABC and FUNC pages.",
    "pstvnc_osk_utility_key_t":
        "Provides stable typed indices for the eleven permanent historical utility-row keys.",
    "pstvnc_osk_t":
        "Provides the pure OSK interaction-state type independent of visibility, controller acquisition, RFB, and platform presentation.",
    "pstvnc_osk_activation_t":
        "Provides the typed result of activating one OSK selection or direct logical key.",
    "pstvnc_osk_font_glyph_t":
        "Provides the renderer-private built-in OSK font-glyph value type.",
    "pstvnc_ps2_graphics_overlay_t":
        "Provides the generic project-overlay descriptor accepted by the narrow PS2 graphics seam.",
}

ENUMS = {
    "pstvnc_local_controller_action":
        "Enumerates semantic local-UI actions emitted by controller routing independently of physical button vocabulary.",
    "pstvnc_local_ui_foreground":
        "Enumerates semantic-input ownership between ordinary remote desktop interaction and the PS2-local OSK.",
    "pstvnc_osk_page":
        "Enumerates the adopted ABC and FUNC keyboard pages plus the non-selectable page-count bounds sentinel.",
    "pstvnc_osk_utility_key":
        "Assigns stable indices to the historical permanent utility row: page selectors, one-shot modifiers, and fixed utility keys.",
}

ENUM_VALUES = {
    "PSTVNC_INPUT_EVENT_CONTROLLER_STATE":
        "Identifies one atomic trustworthy physical-controller observation at the input-to-application boundary.",
    "PSTVNC_INPUT_EVENT_KEYBOARD_TAP":
        "Identifies one already-resolved logical keyboard-tap intent awaiting application-owned publication.",

    "PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK":
        "Requests transition from desktop foreground into OSK foreground.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT":
        "Requests one historical wrapped OSK selection move to the left.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT":
        "Requests one historical wrapped OSK selection move to the right.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP":
        "Requests one historical proportional OSK selection move upward.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN":
        "Requests one historical proportional OSK selection move downward.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT":
        "Requests a toggle of OSK-owned one-shot Shift.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED":
        "Requests activation of the currently selected OSK key.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE":
        "Requests the fixed OSK direct-key Backspace action.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER":
        "Requests the fixed OSK direct-key Enter action.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_TAB":
        "Requests the fixed OSK direct-key Tab action.",
    "PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK":
        "Requests OSK close and return to the recorded underlying foreground owner.",

    "PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP":
        "Assigns semantic input ownership to ordinary remote-desktop interaction.",
    "PSTVNC_LOCAL_UI_FOREGROUND_OSK":
        "Assigns semantic input ownership to the PS2-local on-screen keyboard.",

    "PSTVNC_OSK_PAGE_ABC":
        "Selects the adopted printable ABC keyboard page.",
    "PSTVNC_OSK_PAGE_FUNC":
        "Selects the adopted function-key keyboard page.",
    "PSTVNC_OSK_PAGE_COUNT":
        "Counts supported OSK pages and serves only as a bounds sentinel, not a selectable page.",

    "PSTVNC_OSK_UTILITY_ABC":
        "Names the permanent utility-row key that switches to the ABC page.",
    "PSTVNC_OSK_UTILITY_FUNC":
        "Names the permanent utility-row key that switches to the FUNC page.",
    "PSTVNC_OSK_UTILITY_SHIFT":
        "Names the permanent utility-row key that toggles one-shot Shift.",
    "PSTVNC_OSK_UTILITY_CTRL":
        "Names the permanent utility-row key that toggles one-shot Ctrl.",
    "PSTVNC_OSK_UTILITY_ALT":
        "Names the permanent utility-row key that toggles one-shot Alt.",
    "PSTVNC_OSK_UTILITY_SPACE":
        "Names the permanent utility-row Space key.",
    "PSTVNC_OSK_UTILITY_TAB":
        "Names the permanent utility-row Tab key.",
    "PSTVNC_OSK_UTILITY_BACKSPACE":
        "Names the permanent utility-row Backspace key.",
    "PSTVNC_OSK_UTILITY_DELETE":
        "Names the permanent utility-row Delete key.",
    "PSTVNC_OSK_UTILITY_ENTER":
        "Names the permanent utility-row Enter key.",
    "PSTVNC_OSK_UTILITY_ESCAPE":
        "Names the permanent utility-row Escape key.",
}

MACROS = {
    "PSTVNC_CONTROLLER_H":
        "Prevents repeated inclusion of the platform-neutral physical-controller facts interface.",
    "PSTVNC_CONTROLLER_BUTTON_MASK":
        "Combines every stable project-owned physical button bit accepted in a trustworthy controller observation.",

    "PSTVNC_KEYBOARD_H":
        "Prevents repeated inclusion of the pure semantic keyboard sequencing interface.",
    "PSTVNC_KEYBOARD_MODIFIER_SHIFT":
        "Marks a real Shift modifier that must bracket the target key in a semantic keyboard tap.",
    "PSTVNC_KEYBOARD_MODIFIER_CTRL":
        "Marks a real Ctrl modifier that must bracket the target key in a semantic keyboard tap.",
    "PSTVNC_KEYBOARD_MODIFIER_ALT":
        "Marks a real Alt modifier that must bracket the target key in a semantic keyboard tap.",
    "PSTVNC_KEYBOARD_MODIFIER_MASK":
        "Combines every real modifier bit accepted by the balanced keyboard sequence builder.",
    "PSTVNC_KEYBOARD_KEYSYM_BACKSPACE":
        "Names the native X11 Backspace keysym used by fixed OSK direct-key actions.",
    "PSTVNC_KEYBOARD_KEYSYM_TAB":
        "Names the native X11 Tab keysym used by fixed OSK direct-key actions.",
    "PSTVNC_KEYBOARD_KEYSYM_ENTER":
        "Names the native X11 Enter keysym used by fixed OSK direct-key actions.",
    "PSTVNC_KEYBOARD_KEYSYM_SHIFT_L":
        "Names the native X11 left-Shift keysym used when expanding a real Shift modifier.",
    "PSTVNC_KEYBOARD_KEYSYM_CONTROL_L":
        "Names the native X11 left-Control keysym used when expanding a real Ctrl modifier.",
    "PSTVNC_KEYBOARD_KEYSYM_ALT_L":
        "Names the native X11 left-Alt keysym used when expanding a real Alt modifier.",
    "PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS":
        "Provides capacity for the maximal balanced tap: three modifier downs, target down/up, and three reverse-order modifier releases.",

    "PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS":
        "Combines the physical buttons local UI routing may consume so their held state can be retained until release quarantine is proven.",
    "PSTVNC_LOCAL_CONTROLLER_H":
        "Prevents repeated inclusion of the platform-neutral controller-to-local-action routing interface.",
    "PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS":
        "Bounds the ordered semantic action list that one physical-controller observation may produce.",

    "PSTVNC_LOCAL_UI_H":
        "Prevents repeated inclusion of the main-owned local foreground, transition-quarantine, and generation interface.",
    "PSTVNC_LOCAL_UI_BOTTOM_INSET":
        "Preserves the adopted six-logical-pixel gap between the OSK surface and the bottom edge of the 704x462 logical desktop.",
    "PSTVNC_LOCAL_UI_PRESENTATION_H":
        "Prevents repeated inclusion of the platform-neutral local-presentation description interface.",

    "PSTVNC_OSK_H":
        "Prevents repeated inclusion of the platform-neutral on-screen-keyboard behavior interface.",
    "PSTVNC_OSK_WIDTH":
        "Preserves the adopted historical OSK surface width of 600 logical pixels.",
    "PSTVNC_OSK_HEIGHT":
        "Preserves the adopted historical OSK surface height of 178 logical pixels.",
    "PSTVNC_OSK_ROWS":
        "Preserves the adopted five-row OSK model, including the permanent utility row.",
    "PSTVNC_OSK_UTILITY_ROW":
        "Names row index 4 as the permanent historical utility-key row.",
    "PSTVNC_OSK_UTILITY_KEY_COUNT":
        "Preserves the eleven permanent historical utility keys from ABC through Escape.",

    "PSTVNC_OSK_RENDER_H":
        "Prevents repeated inclusion of the platform-neutral historical-OSK renderer interface.",
    "PSTVNC_OSK_SURFACE_PIXEL_COUNT":
        "Computes the exact caller-owned pixel capacity required for one complete 600x178 OSK surface.",

    "PSTVNC_RFB_KEY_EVENT_SIZE":
        "Names the exact eight-byte size of an RFB 3.x client-to-server KeyEvent.",
}

BUTTON_LABELS = {
    "SELECT": "Select",
    "L3": "L3",
    "R3": "R3",
    "START": "Start",
    "UP": "D-pad Up",
    "RIGHT": "D-pad Right",
    "DOWN": "D-pad Down",
    "LEFT": "D-pad Left",
    "L2": "L2",
    "R2": "R2",
    "L1": "L1",
    "R1": "R1",
    "TRIANGLE": "Triangle",
    "CIRCLE": "Circle",
    "CROSS": "Cross",
    "SQUARE": "Square",
}

for suffix, label in BUTTON_LABELS.items():
    MACROS[f"PSTVNC_CONTROLLER_BUTTON_{suffix}"] = (
        f"Assigns {label} a stable project-owned active-high physical button bit after native PS2SDK-to-project translation."
    )


# Public semantic fields are worth curating too.  Locals and prototype
# parameters remain mechanically descriptive because elaborating hundreds of
# obvious plumbing rows would reduce rather than improve the dictionary's
# signal-to-noise ratio.
FIELDS = {
    ("pstvnc_controller_state", "buttons_down"):
        "Stores the complete current active-high project button state for this trustworthy physical sample.",
    ("pstvnc_controller_state", "buttons_pressed"):
        "Stores press edges relative to the immediately preceding trustworthy sample in the same physical connection epoch.",
    ("pstvnc_controller_state", "buttons_released"):
        "Stores release edges relative to the immediately preceding trustworthy sample in the same physical connection epoch.",
    ("pstvnc_controller_state", "connection_epoch_started"):
        "Marks the first trustworthy sample after startup, reconnect, or hard physical-history invalidation without manufacturing press edges.",

    ("pstvnc_input_event_payload", "controller_state"):
        "Carries one platform-neutral physical-controller fact value without embedding OSK or product-action meaning.",
    ("pstvnc_input_event_payload", "keyboard_tap"):
        "Carries one already-resolved logical keyboard-tap intent as a distinct semantic payload family.",

    ("pstvnc_input_runtime", "mouse_interpretation_suspend_requested"):
        "Carries application/main's request for the worker to establish and retain the mouse-interpretation suspension boundary.",
    ("pstvnc_input_runtime", "mouse_interpretation_suspended"):
        "Carries the worker acknowledgement proving mouse interpretation is frozen while libpad polling remains live.",
    ("pstvnc_input_runtime", "suspended_mouse_state_rebased"):
        "Records that application/main reconciled the frozen mouse interpreter to exact last-successfully-published remote pointer state.",
    ("pstvnc_input_runtime", "suspended_physical_continuity_lost"):
        "Records worker-observed physical-controller continuity loss during the acknowledged suspension epoch so a hard mouse reset occurs before resume.",

    ("pstvnc_keyboard_tap", "keysym"):
        "Stores the already-resolved native 32-bit X11 keysym targeted by this logical tap.",
    ("pstvnc_keyboard_tap", "modifiers"):
        "Stores only the real Shift, Ctrl, and Alt semantic modifier bits that must bracket the target key.",
    ("pstvnc_keyboard_key_event", "down"):
        "Stores explicit native keyboard state: nonzero for key down and zero for key up.",
    ("pstvnc_keyboard_key_event", "keysym"):
        "Stores the complete native 32-bit X11 keysym for this pre-RFB keyboard fact.",
    ("pstvnc_keyboard_sequence", "events"):
        "Stores the bounded explicit down/up facts comprising one complete balanced logical keyboard action.",
    ("pstvnc_keyboard_sequence", "event_count"):
        "Records the authoritative number of initialized key facts in the bounded sequence; zero carries no publication authority.",

    ("pstvnc_local_controller_result", "actions"):
        "Stores the ordered bounded semantic local actions produced from one physical-controller observation.",
    ("pstvnc_local_controller_result", "action_count"):
        "Records the authoritative number of valid ordered actions in the current routing result.",
    ("pstvnc_local_controller", "owned_buttons_awaiting_release"):
        "Retains locally consumed physical buttons still held so the transition gesture cannot leak into the next foreground.",

    ("pstvnc_local_ui", "foreground"):
        "Records the current semantic-input owner: ordinary remote desktop or PS2-local OSK.",
    ("pstvnc_local_ui", "return_foreground"):
        "Records the foreground owner to restore when the current local foreground closes rather than relying on an implicit global assumption.",
    ("pstvnc_local_ui", "input_quarantined"):
        "States that semantic controller actions remain blocked until upstream release proof completes the current foreground transition.",
    ("pstvnc_local_ui", "generation"):
        "Identifies the exact current visible local-state generation and advances for every visible local mutation.",
    ("pstvnc_local_ui", "presented_generation"):
        "Records the exact local generation last acknowledged after successful platform presentation.",
    ("pstvnc_local_ui", "dirty"):
        "Carries local presentation authority indicating that the current generation still requires successful presentation.",

    ("pstvnc_local_ui_presentation", "overlay_visible"):
        "States whether this exact local generation includes a PS2-local overlay rather than desktop-only presentation.",
    ("pstvnc_local_ui_presentation", "overlay_pixels"):
        "Points to caller-owned project presentation pixels for the local overlay without changing remote framebuffer authority.",
    ("pstvnc_local_ui_presentation", "overlay_pixel_count"):
        "Records the exact caller-owned local-overlay pixel count described by this presentation.",
    ("pstvnc_local_ui_presentation", "overlay_width"):
        "Records the logical width of the local overlay surface described for platform presentation.",
    ("pstvnc_local_ui_presentation", "overlay_height"):
        "Records the logical height of the local overlay surface described for platform presentation.",
    ("pstvnc_local_ui_presentation", "overlay_x"):
        "Records the logical desktop X coordinate of the local overlay's adopted placement.",
    ("pstvnc_local_ui_presentation", "overlay_y"):
        "Records the logical desktop Y coordinate of the local overlay's adopted placement.",
    ("pstvnc_local_ui_presentation", "generation"):
        "Identifies the exact local-UI generation represented by this visual description for later acknowledgement.",

    ("pstvnc_osk", "page"):
        "Records the current adopted ABC or FUNC OSK page independently of foreground visibility.",
    ("pstvnc_osk", "row"):
        "Records the currently selected historical OSK row.",
    ("pstvnc_osk", "col"):
        "Records the currently selected column within the current historical OSK row.",
    ("pstvnc_osk", "shift"):
        "Stores OSK-owned one-shot Shift state, whose printable ABC behavior is distinct from real remote Shift.",
    ("pstvnc_osk", "ctrl"):
        "Stores OSK-owned one-shot Ctrl state for the next actual key action.",
    ("pstvnc_osk", "alt"):
        "Stores OSK-owned one-shot Alt state for the next actual key action.",
    ("pstvnc_osk_activation", "local_state_changed"):
        "Reports whether this activation visibly changed OSK-local state and therefore requires a new local dirty generation.",
    ("pstvnc_osk_activation", "produced_keyboard_tap"):
        "Reports whether this activation produced a complete semantic keyboard-tap intent for application publication.",
    ("pstvnc_osk_activation", "keyboard_tap"):
        "Carries the complete resolved semantic keyboard tap when produced_keyboard_tap is asserted.",

    ("pstvnc_osk_font_glyph", "character"):
        "Identifies the supported character represented by this renderer-owned built-in OSK font glyph.",
    ("pstvnc_osk_font_glyph", "rows"):
        "Stores the renderer-owned bitmap rows used to rasterize the built-in glyph.",

    ("pstvnc_ps2_graphics_overlay", "pixels"):
        "Points to caller-owned opaque 16-bit project presentation pixels for the optional generic local overlay.",
    ("pstvnc_ps2_graphics_overlay", "pixel_count"):
        "Records the exact available pixel count used to validate the generic local overlay surface.",
    ("pstvnc_ps2_graphics_overlay", "width"):
        "Records the logical width of the generic local overlay surface.",
    ("pstvnc_ps2_graphics_overlay", "height"):
        "Records the logical height of the generic local overlay surface.",
    ("pstvnc_ps2_graphics_overlay", "x"):
        "Records the logical desktop X coordinate at which the platform presents the generic local overlay.",
    ("pstvnc_ps2_graphics_overlay", "y"):
        "Records the logical desktop Y coordinate at which the platform presents the generic local overlay.",
}


def desired_description(cells: list[str]) -> str | None:
    name, kind, file_name, owner, scope, description, context = cells

    if kind in {"function", "function declaration"}:
        return FUNCTIONS.get(name)
    if kind == "variable":
        return VARIABLES.get(name)
    if kind == "structure":
        return STRUCTURES.get(name)
    if kind == "type":
        return TYPES.get(name)
    if kind == "enum":
        return ENUMS.get(name)
    if kind == "enum value":
        return ENUM_VALUES.get(name)
    if kind == "macro":
        return MACROS.get(name)
    if kind == "field":
        return FIELDS.get((owner, name))
    return None


def target_row(cells: list[str]) -> bool:
    name, kind, file_name, owner, scope, description, context = cells

    if name == "pstvnc_ps2_graphics_present" and kind in {
        "function",
        "function declaration",
    }:
        return True

    if context not in ISSUE39_CONTEXTS:
        return False

    if kind in HIGH_VALUE_KINDS or kind == "field":
        return True

    return name in IMPORTANT_VARIABLES


def assert_authority() -> None:
    branch = git("branch", "--show-current", capture=True).strip()
    head = git("rev-parse", "HEAD", capture=True).strip()

    print(f"BRANCH={branch}")
    print(f"HEAD={head}")

    if branch != EXPECTED_BRANCH:
        raise RuntimeError(f"wrong branch: {branch}")
    if head != EXPECTED_HEAD:
        raise RuntimeError(f"unexpected HEAD: {head}")
    if git("status", "--porcelain=v1", "--untracked-files=all", capture=True):
        raise RuntimeError("worktree must be completely clean before curation")

    for path, expected_blob in EXPECTED_BLOBS.items():
        actual_blob = git("hash-object", path, capture=True).strip()
        print(f"BLOB={path}:{actual_blob}")
        if actual_blob != expected_blob:
            raise RuntimeError(
                f"dictionary authority drift for {path}: "
                f"expected {expected_blob}, got {actual_blob}"
            )


def curate() -> tuple[int, Counter[str]]:
    changed: list[tuple[str, str, str]] = []
    kind_counts: Counter[str] = Counter()

    for dictionary in DICTIONARIES:
        lines = dictionary.read_text(encoding="utf-8").splitlines()
        output: list[str] = []

        for line in lines:
            if not line.startswith("| ") or line.startswith("| Name |"):
                output.append(line)
                continue

            cells = parse_row(line)
            if not target_row(cells):
                output.append(line)
                continue

            replacement = desired_description(cells)
            if replacement is None:
                raise RuntimeError(
                    "missing explicit semantic description for target row: "
                    f"{cells[2]}:{cells[0]}:{cells[1]}:{cells[3]}"
                )
            if "|" in replacement or "\n" in replacement:
                raise RuntimeError(
                    f"invalid Markdown description for {cells[2]}:{cells[0]}"
                )
            if replacement == cells[5]:
                raise RuntimeError(
                    "target row unexpectedly already has curated description: "
                    f"{cells[2]}:{cells[0]}:{cells[1]}"
                )

            cells[5] = replacement
            output.append(render_row(cells))
            changed.append((cells[2], cells[0], cells[1]))
            kind_counts[cells[1]] += 1

        dictionary.write_text("\n".join(output) + "\n", encoding="utf-8")

    # The earlier census proved 211 non-field semantic landmarks.  This pass
    # deliberately adds 50 public semantic fields and the two pre-existing
    # graphics-present rows whose contract expanded to optional local overlay.
    expected_changed = 263
    if len(changed) != expected_changed:
        raise RuntimeError(
            f"expected {expected_changed} curated rows, changed {len(changed)}"
        )

    # Re-parse and prove every intended row now equals the explicit mapping.
    verified = 0
    for dictionary in DICTIONARIES:
        for line in dictionary.read_text(encoding="utf-8").splitlines():
            if not line.startswith("| ") or line.startswith("| Name |"):
                continue
            cells = parse_row(line)
            if not target_row(cells):
                continue
            expected = desired_description(cells)
            if expected is None or cells[5] != expected:
                raise RuntimeError(
                    "post-curation semantic mismatch: "
                    f"{cells[2]}:{cells[0]}:{cells[1]}"
                )
            verified += 1

    if verified != expected_changed:
        raise RuntimeError(
            f"expected {expected_changed} verified curated rows, found {verified}"
        )

    return len(changed), kind_counts


def main() -> int:
    print("===== ISSUE39 GITHUB-HOSTED DICTIONARY CURATION =====")
    print()
    print("===== 1. EXACT AUTHORITY =====")
    assert_authority()
    print("CURATION_AUTHORITY=PASS")

    print()
    print("===== 2. APPLY SEMANTIC DESCRIPTION CURATION =====")
    changed, kind_counts = curate()
    print(f"CURATED_DESCRIPTION_ROWS={changed}")
    for kind in sorted(kind_counts):
        print(f"CURATED_KIND_COUNT={kind}:{kind_counts[kind]}")

    print()
    print("===== 3. REGENERATE AGGREGATE PORTAL =====")
    run(
        "python3",
        "scripts/source-dictionary.py",
        "portal",
        "--output",
        str(PORTAL),
    )
    print("SOURCE_DICTIONARY_PORTAL=REGENERATED")

    print()
    print("===== 4. STRICT SOURCE-DICTIONARY CHECK =====")
    run("python3", "scripts/source-dictionary.py", "check", "--strict")
    print("STRICT_SOURCE_DICTIONARY_CHECK=PASS")

    print()
    print("===== 5. CANONICAL PROJECT CHECK =====")
    run("./scripts/check.sh")
    print("CANONICAL_PROJECT_CHECK=PASS")

    print()
    print("===== 6. DIFF SCOPE / HYGIENE =====")
    git("diff", "--check")

    changed_paths = git("diff", "--name-only", capture=True).splitlines()
    expected_paths = {
        "src/SYMBOLS.md",
        "src/platform/SYMBOLS.md",
        "docs/reference/SOURCE_SYMBOL_DICTIONARIES.md",
    }
    if set(changed_paths) != expected_paths:
        raise RuntimeError(
            "curation changed unexpected paths: " + repr(changed_paths)
        )

    print("CURATION_PATH_SCOPE=PASS")
    git("diff", "--stat")

    print()
    print("===== 7. REPRESENTATIVE CURATED ROWS =====")
    samples = {
        "pstvnc_keyboard_build_tap_sequence",
        "pstvnc_local_controller_route",
        "pstvnc_local_ui_open_osk",
        "pstvnc_local_ui_mark_presented",
        "pstvnc_mouse_reset_transient_history",
        "pstvnc_input_runtime_rebase_suspended_mouse_state",
        "pstvnc_osk_activate_selected",
        "pstvnc_osk_render_surface",
        "pstvnc_rfb_build_key_event",
        "pstvnc_rfb_session_send_key_event",
        "pstvnc_ps2_graphics_present",
    }
    for dictionary in DICTIONARIES:
        for line in dictionary.read_text(encoding="utf-8").splitlines():
            if not line.startswith("| ") or line.startswith("| Name |"):
                continue
            cells = parse_row(line)
            if cells[0] in samples:
                print(f"{dictionary}:{line}")

    print()
    print("===== 8. COMMIT ONLY CURATED DICTIONARIES =====")
    git(
        "add",
        "--",
        "src/SYMBOLS.md",
        "src/platform/SYMBOLS.md",
        "docs/reference/SOURCE_SYMBOL_DICTIONARIES.md",
    )
    git("diff", "--cached", "--check")

    staged_paths = git("diff", "--cached", "--name-only", capture=True).splitlines()
    if set(staged_paths) != expected_paths:
        raise RuntimeError(
            "unexpected staged dictionary path set: " + repr(staged_paths)
        )

    git("commit", "-m", "docs: curate Issue 39 source dictionaries")
    commit_sha = git("rev-parse", "HEAD", capture=True).strip()
    print(f"CURATION_COMMIT={commit_sha}")

    print()
    print("===== 9. PUSH TARGET BRANCH =====")
    git("push", "origin", EXPECTED_BRANCH)
    remote_head = git(
        "ls-remote",
        "origin",
        f"refs/heads/{EXPECTED_BRANCH}",
        capture=True,
    ).split()[0]
    print(f"LOCAL_HEAD={commit_sha}")
    print(f"REMOTE_HEAD={remote_head}")
    if remote_head != commit_sha:
        raise RuntimeError("remote head does not match curation commit")
    print("REMOTE_HEAD_VERIFIED=PASS")

    if git("status", "--porcelain=v1", "--untracked-files=all", capture=True):
        raise RuntimeError("worktree is not clean after curation commit")
    print("WORKTREE_CLEAN=PASS")

    print()
    print("===== 10. RETIRE TEMPORARY AUTOMATION BRANCH =====")
    cleanup = subprocess.run(
        ["git", "push", "origin", "--delete", TEMP_REMOTE_BRANCH],
        text=True,
    )
    if cleanup.returncode == 0:
        print("TEMP_AUTOMATION_BRANCH_DELETED=PASS")
    else:
        print("TEMP_AUTOMATION_BRANCH_DELETED=WARNING")

    print()
    print("===== FINAL =====")
    print(f"CURATION_COMMIT={commit_sha}")
    print("CURATED_DESCRIPTION_ROWS=263")
    print("STRICT_SOURCE_DICTIONARY_CHECK=PASS")
    print("SOURCE_DICTIONARIES=PASS")
    print("CANONICAL_PROJECT_CHECK=PASS")
    print("REMOTE_HEAD_VERIFIED=PASS")
    print("WORKTREE_CLEAN=PASS")
    print("RUNTIME_SOURCE_MUTATION=NO")
    print("BUILD_PERFORMED=NO")
    print("DEPLOY_PERFORMED=NO")
    print("ISSUE39_CLOSED=NO")
    print("NEXT=FINAL_ISSUE39_WHOLE_DIFF_REPRODUCIBLE_BUILD_AND_CLOSEOUT_REVIEW")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print()
        print("===== CURATION FAILED =====")
        print(f"ERROR={exc}")
        print("COMMIT_CREATED=NO_OR_REQUIRES_INSPECTION")
        print("PUSH_PERFORMED=NO_OR_REQUIRES_INSPECTION")
        raise
