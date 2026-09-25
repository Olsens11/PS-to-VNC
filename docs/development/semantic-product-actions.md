# Semantic product actions

## Status

    OWNER=INPUT
    PACKET=A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28
    PRODUCT_MAPPING_SELECTED=NO
    LIVE_INPUT_RUNTIME_WIRING=NO
    APPLICATION_ROUTING=DEFERRED

R28 reconstructs the semantic boundary needed for future product actions without
choosing a physical controller gesture or executing the action.

## Ownership split

Input owns the translation from a trustworthy physical controller sample plus a
caller-supplied binding set into one semantic product-action value.

The binding value contains exactly:

- a project-owned physical controller button mask;
- a semantic product action;
- a trigger kind: SETTLE, RELEASE, or HOLD;
- a context: DESKTOP or GLOBAL.

R28's first semantic action is
`PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION`. That name is product meaning only.
The resolver does not enter calibration, freeze RFB, mutate UI, start MPEG,
touch Transport, or arm the media clock.

A later Configuration/composition owner may supply the physical binding. R28
contains no built-in binding table and therefore no compiled physical mapping
for MPEG CALIBRATION.

## Reconstructed arbitration

The resolver preserves the qualified B4A arbitration policy while discarding
B4A's compiled binding table:

- ordinary exact-mask settle threshold: 8 stable polls;
- extendable SETTLE threshold: 18 stable polls when the exact SETTLE mask is a
  strict subset of an eligible HOLD binding;
- HOLD completion threshold: 120 hold polls after ordinary settle admission;
- any exact relevant-mask change resets pending settle/hold progress;
- RELEASE is armed only when arriving at its exact mask through gesture growth;
- growing beyond an armed RELEASE cancels it;
- shrinking from a larger gesture cannot manufacture a release shortcut;
- after an action fires, the gesture remains latched until all relevant binding
  buttons are released;
- a DESKTOP gesture must start while desktop context is eligible and must remain
  desktop-eligible until resolution; context cannot be acquired or reacquired
  in the middle of one physical gesture;
- a physical controller connection-epoch boundary resets pending arbitration.

Only buttons present in the supplied binding universe participate in this
resolver. Unrelated physical buttons remain available to their existing Input/UI
owners.

## Event boundary

`PSTVNC_INPUT_EVENT_PRODUCT_ACTION` carries one validated
`pstvnc_product_action_t` through the existing ordinary Input FIFO.

The queue preserves the same bounded FIFO, copy, wraparound, overflow, and
discard semantics used by existing controller/mouse/keyboard events. NONE and
unknown product-action values are rejected at queue admission.

R28 does not create an urgent-control mailbox. No requirement in this packet
earns a separate concurrency path.

## Historical MPEG entry disposition

The H1
`h1_mpeg_calibration_entry_hold.*` adapter used held START+SELECT for
750 milliseconds to enter MPEG calibration. A005 explicitly classified that as
temporary apparatus and discarded it as a product binding.

R28 therefore reconstructs neither that chord nor that 750 ms adapter. The
semantic action exists independently so a later packet can choose a product
binding without changing the MPEG calibration mechanism.

The frozen B4A runtime is used only for the reusable arbitration behavior
(settle/release/hold/context/latch), not as authority for current physical
bindings.

## Deferred composition

R28 deliberately stops before:

- installing a selected binding;
- wiring the resolver into `input_runtime`;
- routing `MPEG_CALIBRATION` in Application;
- entering P9 or P10;
- starting or servicing an MPEG run;
- adding Configuration parsing/persistence or a binding editor;
- adding UI labels or controller-help text.

Those require later explicit product/composition authority.

Context: `docs/ledge/LEDGE_AUDIT_A005_INTERACTION_INPUT.md`;
`docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md`;
`docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md`;
`docs/ledge/LEDGE_FOREMAN_STATE.md`.
