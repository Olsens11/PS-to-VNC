# Semantic product actions

## Status

    OWNER=INPUT
    CORE_PACKET=A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28
    LIVE_PACKET=A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29
    PRODUCT_MAPPING_SELECTED=NO
    LIVE_INPUT_RUNTIME_WIRING=YES_OPTIONAL_CALLER_CONFIGURED
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

## R29 live Input-runtime composition

R29 composes the accepted R28 resolver into `input_runtime` without selecting a
product mapping or executing a product effect.

Every runtime initializes with a valid zero-binding resolver. A caller may
replace that authority before worker start through
`pstvnc_input_runtime_set_product_action_bindings()`. The runtime retains only
the R28 resolver's immutable caller-owned binding pointer and owns all gesture
history inside that runtime instance. Worker start revalidates the binding set
and clears history before `CreateThread()`, so invalid or caller-mutated
authority cannot become live.

DESKTOP admission is supplied only through
`pstvnc_input_runtime_set_product_action_desktop_eligible()`. That live 0/1
fact is Application-owned; Input does not include or inspect UI/Application
state. R28 remains the sole owner of gesture-start provenance, context loss and
the no-mid-gesture-reacquisition rule.

When nonzero bindings exist, every trustworthy physical sample is offered to
R28, including stable polls with no press/release edge. Therefore the qualified
8/18/120 physical-poll thresholds are not redefined by the sparse
`CONTROLLER_STATE` event policy.

If one action resolves, Input queues exactly one
`PSTVNC_INPUT_EVENT_PRODUCT_ACTION` before controller-state or mouse work from
that same sample. Queue wait/signal/full failure or resolver contradiction stops
the worker path; later same-sample work does not continue after a resolved
action was lost.

The existing ordinary FIFO remains the only publication path. The existing
activity notification runs only after per-sample processing returns successfully,
so a wake observes already-published semantic queue work. R29 adds no urgent
mailbox or parallel product-action queue.

Hard physical boundaries also bound resolver history:

- physical connection loss resets the current R28 gesture before a replacement
  connection epoch;
- explicit libpad handoff clears resolver history before acknowledgement, and
  the returning first trustworthy sample still begins a new connection epoch;
- ordinary mouse-interpretation suspension does not transfer libpad ownership
  and therefore does not independently reset the product-action gesture owner.

Zero bindings remain the ordinary product default, so current controller/mouse
behavior is unchanged until a later packet selects a physical binding.

## Still-deferred composition

R29 still deliberately stops before:

- installing any selected physical MPEG CALIBRATION binding;
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
