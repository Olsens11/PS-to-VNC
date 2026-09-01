# B07-B09 Input, Keyboard, and Local UI Behavioral Audit

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    TRANCHE=B07_B08_B09
    MATURITY=EVIDENCE_SUPPORTED
    ROLE=DETAILED_BEHAVIOR_AUDIT

This document records the source-plus-historical-evidence audit for the input,
keyboard, and local-UI behavior families. It is subordinate to the top-level
inventory in `docs/audit/BEHAVIORAL_INVENTORY.md` and exists to keep the three
closely coupled historical concerns readable without treating the old
controller-thread implementation as the target architecture.

The reconstruction rule remains:

> preserve behavior and proven mechanisms, not accidental historical structure.

The historical implementation interleaves controller polling, pointer
semantics, keyboard shortcuts, hotkey arbitration, OSK state, menu ownership,
and display-transaction UI in one controller loop. The clean product should
separate those responsibilities while preserving the evidence-backed contracts
below.

---

## B07 — Controller acquisition, pointer semantics, clicks, and logical actions

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Convert PS2 controller state into deliberate product input actions while
maintaining explicit ownership of libpad and preventing stale physical/input
state from crossing local-UI, calibration, or hazardous display-transition
boundaries.

B07 owns controller acquisition/polling semantics, pointer/click/scroll action
generation, controller-derived hotkey arbitration, and the temporary libpad
handoff contract. It does not own RFB wire serialization, keyboard/OSK state,
or the local UI state machine itself.

### Ordinary controller owner

The qualified B4A/current controller path maintains state for:

- cursor x/y;
- raw physical pressed/edge history;
- last published remote pointer-button mask;
- D-pad precision/hold/acceleration state;
- fractional analog cursor motion;
- scroll direction/repeat state;
- runtime-hotkey observation, settling, hold, release, and latch state;
- local-UI input quarantine.

The historical controller thread polls at approximately 60 Hz. That exact
thread shape is not a clean-architecture requirement; the behavioral ownership
is.

### Explicit libpad ownership transfer

Display calibration and selected display-transition paths temporarily transfer
libpad ownership from the controller owner to the main/render owner.

The handoff is not merely a mutex around `padRead()`:

1. the main owner publishes `controller_pad_pause_requested`;
2. the controller owner reaches a boundary immediately before its next libpad
   access;
3. only there does it publish `controller_pad_pause_ack`;
4. while the acknowledgement is asserted, the controller owner performs no
   `padGetState()` or `padRead()` calls;
5. the temporary owner may now use libpad exclusively;
6. the request is withdrawn;
7. the controller acknowledgement drops;
8. ordinary polling resumes from reset derived-input state.

`ps2vnc_controller_pad_acquire()` waits for that acknowledgement with a bounded
failure path. `ps2vnc_controller_pad_release()` withdraws the request and waits
for acknowledgement withdrawal.

### State reset on ownership return

On return from a libpad handoff the controller owner deliberately forgets
pre-handoff derived state, including:

- prior raw edge state;
- prior remote pointer-button state;
- D-pad hold direction/timers/fractional motion;
- analog fractional motion;
- scroll repeat state;
- hotkey observation/settle/hold/release/latch state;
- local-UI quarantine state.

The durable invariant is:

> no physical or controller-derived state from before the handoff may become a
> new logical action after ownership returns.

This is stronger and more useful than preserving the exact historical set of
globals.

### Physical-release policy is context-dependent

Calibration and hazardous remote transition use different mechanisms to satisfy
the same no-leak invariant.

For calibration, `ps2vnc_controller_pad_wait_release()` keeps exclusive
main-thread ownership until every calibration button is physically released.
This prevents the X/Circle/D-pad state used to accept or cancel calibration from
becoming remote-desktop input immediately afterward.

The D17AL-F8J2-B4A hard remote transition intentionally does **not** wait for a
neutral analog stick before returning ownership. The hostile-stick benchmark
holds the analog stick during the transition. Safety instead comes from clearing
all pre-handoff edge/hold/analog/scroll state before ordinary polling resumes.

Therefore the clean contract is not “always wait for physical neutral.” It is
“never permit stale pre-handoff state to leak into the next context.”

### B4A hard remote-transition curtain

The frozen B4A delta adds a wrapper around the previously proven B3C display
reconstruction. For a backdoor/remote display transition the sequence is:

    controller OFF
      -> discard queued controller RFB
      -> source-timing curtain ON
      -> source settle
      -> B3C display reconstruction
      -> destination-timing curtain ON
      -> destination settle
      -> present one complete destination desktop frame
      -> curtain OFF
      -> controller ON

The wrapper takes true libpad ownership before the transition and releases it
only after a coherent destination frame has been presented. Ordinary temporary
mode dwell remains interactive; the hard disconnect applies to the hazardous
one-way reconstruction boundary, not to the whole temporary-mode lease.

### Controller-derived RFB queue boundary

The later implementation uses a controller-to-main outbound RFB queue so the
main thread remains the sole owner of VNC socket I/O. B4A adds a main-thread-only
queue discard after the hard libpad pause acknowledgement.

That discard intentionally:

- removes pointer/key/scroll messages generated before the transition boundary;
- does not transmit that queued history into the reconstruction epoch;
- does not touch the RFB receive stream.

This exposes an important two-level ownership distinction for the clean design:

1. **physical input ownership** — whether new libpad reads/actions may occur;
2. **already-published logical input** — whether an event was already queued or
   transmitted before physical ownership changed.

A hard input curtain must reason about both.

### Desktop pointer and click semantics

The current qualified behavior includes:

- **D-pad pointer movement** — a fresh direction produces an immediate 2-pixel
  step, followed by delayed precision repeat and then smoothly accelerated
  fractional movement when held;
- **left analog pointer movement** — deadzone plus a deliberately shallow
  low/mid-range response and stronger final acceleration, with fractional Q8
  accumulation so slow near-center movement can take multiple polls per pixel;
- **Cross** — remote left pointer button;
- **Circle** — remote right pointer button;
- pointer coordinates are clamped to the current logical RFB desktop.

Historical Test11 developed and qualified the analog/deadzone/response family.
The exact current mappings come from the final B4A/current behavior rather than
blindly resurrecting every earlier Test11 mapping.

### Scroll semantics

Current behavior provides two wheel-control forms:

- **Triangle + D-pad** — digital vertical/horizontal wheel; vertical directions
  win if a diagonal combination is reported;
- **L3 + left analog** — analog wheel; while L3 owns the stick, cursor movement is
  frozen and fractional cursor state is cleared so an old partial movement
  cannot emerge when L3 is released.

Direction changes fire immediately; holding a direction repeats according to
the selected repeat/deflection policy.

Test13I explicitly records the Triangle+D-pad wheel addition in legacy history.
The complete current scroll contract remains final-source authority.

### Keyboard shortcuts originating from controller actions

While the desktop owns input, selected controller gestures produce logical
keyboard actions rather than pointer actions. Examples include the transient
L1+D-pad arrow-key chord and generic runtime bindings such as Tab/Enter.

The important rule from Test11F is that the arrow-key modifier is a physical
chord, not a latched mode. Nothing remains toggled after the chord is released.

### Runtime-hotkey arbitration

The later controller path deliberately treats multi-button hotkeys as gestures
with ownership, not as independent button edges.

Important semantics include:

- only buttons participating in configured runtime bindings enter the hotkey
  arbiter;
- the exact held mask must settle before an ambiguous binding can own the
  gesture;
- growing a chord cannot accidentally fire a previously armed subset-release
  action;
- shrinking a larger chord cannot manufacture a new subset shortcut;
- once a hotkey action fires, the session remains latched until all hotkey
  buttons are released;
- a desktop-only gesture that begins while a local menu/OSK owns input cannot
  become a desktop shortcut merely because the UI context changes before
  physical release;
- global actions remain subject to higher-level provisional-display safety
  gates.

The future binding representation belongs mainly to B10 configuration. B07 owns
the gesture/arbitration semantics used to execute those bindings.

### Historical evidence

Legacy commit `007379b6d4daae87251381b790a7641abab8725d` recovers
`docs/tests/TEST11_INPUT.md` as a **PROVEN HISTORICAL FOUNDATION** reconstructed
from Test11 commit history, preserved hardware-validated ELFs, and known-good
Test11L source.

The Test11 progression records:

- 11A right click;
- 11B L3 double-click experiment;
- 11C forced analog mode;
- 11D analog-speed reduction;
- 11E deadzone;
- 11F transient arrow-key chord;
- 11G OSK Shift behavior;
- 11H/11I/11J pointer-response refinements;
- 11K scrolling/acceleration family;
- 11L expanded keyboard/OSK behavior.

The recovered Test11 document explicitly warns that historical evidence is not
authority to revive obsolete mappings. This audit follows that rule.

Legacy commit `eaa5976171132dc619ccd96ecb9366b704cadc53` records the later
Test13I Triangle+D-pad wheel implementation.

For the F8J2/B4A transition envelope, the successor records B4A as the frozen
behavioral DUT and a known tested ELF. M0 explicitly states that B4A already
carries historical hardware-validation evidence and that its byte-identical M0
reproduction inherits that evidence. The successor does not import the
standalone B4A operator log, so this audit states the evidence precisely:

- B4A DUT behavior has inherited historical hardware-validation authority;
- the exact hard-controller-curtain mechanism is source/diff authority;
- B3A/B3B separately provide explicit hardware records for the preceding F8J2
  request-epoch and live forward/rollback paths.

### Invariants

- only one owner accesses libpad during an explicit handoff;
- acknowledgement is published at a boundary where the controller owner has
  stopped before its next libpad access;
- pre-handoff controller-derived state never becomes post-handoff input;
- physical-neutral waiting is used where required by the interaction, not as a
  universal rule;
- a hazardous remote reconstruction drops queued pre-boundary controller
  actions before proceeding;
- pointer coordinates always remain within the current logical desktop;
- chord growth/shrink/settle semantics prevent accidental subset actions;
- current product mappings come from final authority, not obsolete historical
  test mappings.

### Primary authority

- `working/b4a/ps2vnc_ui_tail.inc` — controller loop, pointer/scroll/hotkey
  semantics and handoff acknowledgement;
- `working/b4a/ps2vnc_calibration_tail.inc` — libpad acquire/release and physical
  release helper;
- `working/b4a/ps2vnc_runtime_core.inc` — controller-originated RFB queue and
  hard-boundary discard;
- `working/b4a/ps2vnc_display_core.inc` — B4A hard transition wrapper;
- `baseline/frozen-b4a/B3C-to-B4A.diff` and `IDENTITY.txt`;
- legacy `docs/tests/TEST11_INPUT.md` at the recovered Test11 history commit;
- legacy Test13I commit;
- `docs/M0_HARDWARE_RESOLUTION.md` and `docs/LEGACY_HANDOFF.md` for frozen B4A
  DUT/hardware-evidence authority.

### Clean-rebuild implication

Create an explicit controller/input owner that produces logical actions rather
than directly owning menus, keyboard rendering, display transitions, and socket
I/O. Preserve libpad handoff, stale-state invalidation, gesture arbitration, and
pointer/scroll response as first-class behavior. Model the boundary between
physical ownership and already-published logical input explicitly.

---

## B08 — Keyboard, on-screen keyboard, modifiers, and text interaction

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Produce correct RFB keyboard events and provide a controller-operated local
on-screen keyboard without allowing local OSK state to become ambiguous remote
modifier state.

B08 owns keyboard-event semantics and OSK navigation/modifier/page state. B07
may request logical key actions; B09 decides when the OSK owns the controller;
B03/RFB owns session transport serialization.

### RFB key-event contract

The qualified implementation sends ordinary RFB `KeyEvent` messages with
explicit down/up state and X11 keysyms.

A logical tap is down then up. Modified taps explicitly bracket the key with the
required modifier events. Keyboard traffic uses the same serialized outbound RFB
path as pointer traffic and framebuffer requests so byte-stream messages cannot
interleave.

Failure to enqueue/transmit a required key sequence is an input/session failure;
the clean implementation must not silently leave a modifier logically down.

### OSK pages and key domains

The current OSK exposes:

- an **ABC** printable-character page;
- a **FUNC** page containing real X11 non-printable keysyms;
- a permanent utility row:

      ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC

Printable ABC keys send the character shown by the OSK. Function-page and
utility keys use explicit X11 keysyms such as arrows, Home/End, Page Up/Down,
Insert/Delete, Escape, function keys, and related extended keys.

### Navigation and direct controller controls

While the OSK owns input:

- D-pad moves the local selection;
- Cross activates the selected key;
- Triangle toggles Shift;
- Square sends Backspace;
- Start sends Enter;
- R1 sends Tab;
- Circle or Select closes the keyboard and clears local modifiers.

Horizontal navigation wraps. Vertical movement preserves approximate horizontal
position when moving between rows with different key counts.

### One-shot modifier semantics

Shift, Ctrl, and Alt are local OSK latches, but all are deliberately one-shot:
after the next actual key is sent, every latched modifier returns to off.

The implementation distinguishes printable Shift-layer characters from true
modifier combinations:

- the ABC layer can send the exact displayed capital/symbol keysym;
- extended/utility keys can synthesize a real Shift-down/key/Shift-up sequence
  so combinations such as Shift+Tab retain actual modifier meaning;
- Ctrl/Alt are sent as real modifier events around the target key.

This preserves the historical TigerVNC lesson that blindly modeling printable
Shift as a persistent remote Shift state created undesirable CapsLock/modifier
reconciliation behavior.

### Opening and closing the OSK

Select opens the OSK from ordinary desktop input with predictable state:

- ABC page;
- known initial selection;
- Shift/Ctrl/Alt cleared.

If the remote pointer currently has a logical mouse button held, opening the OSK
first publishes a zero-button pointer event so the remote desktop cannot retain
a held mouse state while local keyboard input takes ownership.

Closing the OSK clears all one-shot modifiers.

### Local-only changes must wake presentation

OSK navigation changes only PS2-local pixels; it may not produce remote desktop
damage. The historical live loop can be blocked waiting for an incremental RFB
update, so local OSK changes explicitly wake the renderer.

The current implementation uses a one-pixel remote pointer jiggle out-and-back
to wake the blocked live loop. The pointer finishes exactly where it began.

The durable product requirement is **local UI state must be renderable promptly
without depending on unrelated remote framebuffer damage**. The pointer jiggle
is an implementation workaround and should not automatically become the clean
design.

### Historical evidence

Recovered Test11 is the primary historical input/keyboard foundation. Test11G
records Shift behavior; Test11L records the expanded two-page keyboard/OSK and
one-shot modifier model. The recovered document marks this family as proven
historical foundation while warning against restoring obsolete controller
mappings.

Current/frozen B4A source remains authority for the final mappings, page
contents, and interaction with later UI/hotkey behavior.

### Invariants

- remote key events have explicit down/up semantics;
- modifier down/up sequences cannot interleave with unrelated RFB messages;
- OSK Shift/Ctrl/Alt are one-shot and clear after a real key action;
- closing the OSK clears local modifiers;
- opening the OSK cannot leave a remote mouse button logically held;
- printable-layer semantics and true modifier combinations remain distinct;
- local-only OSK changes do not depend on remote framebuffer damage for visible
  presentation;
- historical mappings are evidence, not automatic current-product authority.

### Primary authority

- `working/b4a/ps2vnc_ui_core.inc` — OSK pages, navigation, modifiers, activation;
- `working/b4a/ps2vnc_ui_tail.inc` — OSK controller ownership/shortcuts;
- `working/b4a/ps2vnc_runtime_tail.inc` / runtime RFB send helpers — key-event
  serialization;
- legacy recovered `docs/tests/TEST11_INPUT.md`;
- frozen B4A/current behavior for final mappings.

### Clean-rebuild implication

Provide a small keyboard/RFB action layer and a separate local OSK state model.
Do not let the OSK become the owner of controller polling or RFB transport.
Represent modifier lifetime explicitly, and give local UI rendering a direct
wake/invalidation mechanism instead of relying on a remote-pointer side effect.

---

## B09 — Menus, overlays, curtains, status presentation, and local UI flow

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Own PS2-local interactive surfaces and determine which local context receives
controller actions at any moment. B09 includes menu/overlay visibility,
selection/focus, presentation priority, input quarantine, display confirmation
surfaces, transition curtains, and restored acknowledgements.

It does not own physical controller polling, RFB keyboard/pointer mechanics,
display reconstruction, or recovery mechanisms themselves.

### Foreground ownership and precedence

The qualified controller decision tree gives mutually exclusive local
foreground ownership in this order:

1. restored-display acknowledgement;
2. pending display confirmation / provisional remote transition;
3. System menu;
4. Display Settings;
5. ordinary desktop when the OSK is hidden;
6. OSK.

The exact historical `if/else` chain is not a required final implementation,
but the clean UI must make foreground input ownership equally unambiguous.

### Local-input quarantine

Buttons consumed by a local PS2VNC surface remain quarantined from the remote
desktop until those **physical buttons are released**.

Each poll removes only released bits from `local_ui_quarantine`. A held Circle
that closes a menu therefore cannot become a remote right click on the next poll
simply because the desktop branch has become active again.

This is a strong product invariant:

> context change does not reinterpret an already-held local UI button as a new
> remote-desktop action.

B07 libpad handoff resets derived state at hard ownership transitions; B09
quarantine handles ordinary foreground-context transitions while the controller
owner remains live.

### System overlay

The System surface contains:

- Refresh VNC Session;
- Display Settings;
- Exit to System Menu.

It also presents visible Refresh states such as ready, refreshing, and cooldown,
and hold countdowns for destructive/global hotkeys.

The System menu can act as a **global overlay over ordinary local UI**. When
opened by a global hotkey over Display Settings or the OSK, it does not
necessarily destroy the underlay; foreground ownership temporarily changes and
the preserved underlay can be revealed again.

However the global overlay must not punch through a provisional display
transaction or libpad calibration ownership boundary. Pending switch/calibration
requests are included in the safety gate so the overlay cannot win a race
between controller publication and main-thread transaction ownership.

Refresh/reconnect mechanics and cooldown policy are audited in B11. B09 owns the
visible/interactive surface and foreground ownership.

### Display Settings UI

Display Settings groups modes into TV/DTV, VGA 60, VGA Other, and Advanced.
Empty mode groups may disappear; Advanced remains available so a hidden/locked
mode policy can always be reversed.

Important current semantics:

- the active mode is visibly marked `CURRENT`;
- Square toggles lock policy for inactive modes;
- the active mode is not lockable from the UI;
- authoritative lock policy is refreshed when the menu opens;
- switching into inactive modes stays disabled while lock policy is unknown;
- X on the active mode requests safe-area calibration;
- X on an unlocked inactive mode requests a mode switch;
- X on a locked inactive mode is a hard no-op for switching;
- Circle returns to the preserved System underlay when Display Settings was
  entered from System;
- after display/calibration flow, selection returns to the active mode so its
  next X action is calibration.

The current footer makes the active-mode action explicit:

    X CALIBRATE <mode> SAFE AREA

This matches the product distinction established in B06: same-mode user action
means calibration, not reinitialization of the current timing.

### Display confirmation UI

A risky local display switch arms a modal confirmation surface with:

- **Go Back** as the default selection;
- **Keep Mode** as the alternate action;
- X to activate the selected action;
- Circle as immediate Go Back;
- visible auto-revert countdown.

B06 owns the 30-second transaction/deadline/rollback mechanics. B09 owns the
human foreground behavior.

A remote/backdoor display transaction uses the same input decision-tree
boundary but suppresses human Keep/Go Back ownership while the machine-owned
remote transaction is active. The controller may remain alive for global safety
machinery, but ordinary desktop pointer/key/scroll actions do not fall through
across the provisional transaction boundary.

### Transition curtain

D17AL-F8J2-B4A introduces a hard remote transition curtain around the hazardous
one-way reconstruction. The local display shows `PLEASE WAIT / CHANGING DISPLAY
MODES` in both source and destination timing, and a coherent destination desktop
frame is explicitly presented before the curtain is removed and controller
ownership returns.

B05/B06 own GS/display reconstruction. B07 owns true controller disconnect. B09
owns the product meaning of the curtain: do not expose an incomplete transition
as an interactive desktop.

### Restored acknowledgement

After a human-owned rollback, the UI can present an explicit “original display
mode restored” acknowledgement. Durable transaction state remains RESTORED until
the acknowledgement is successfully completed.

Remote/machine-owned rollback instead attempts automatic durable ACK because no
human confirmation surface initiated that flow. If automatic durable ACK fails,
the system exposes the human restored-ack surface rather than hiding unresolved
durability.

This preserves the broader rule that UI absence must never imply successful
state convergence when durable authority is unresolved.

### Local UI rendering wake

Menus/OSK/status changes can occur with no remote framebuffer damage. Historical
code uses a pointer-jiggle wake to force the main renderer out of a blocking
incremental RFB wait.

As in B08, the clean requirement is local UI invalidation/wake independent of
remote framebuffer changes. The cursor jiggle is not a required architecture.

### Invariants

- exactly one local/desktop context owns a controller action at a time;
- a button consumed locally stays quarantined until physical release;
- global ordinary overlays cannot cross provisional display/libpad ownership
  boundaries;
- opening an overlay need not destroy a valid underlay;
- active display selection means calibration, not same-mode switch;
- locked/unavailable display targets cannot partially begin a switch;
- the transition curtain prevents exposure/input against an incomplete
  hazardous reconstruction;
- remote machine-owned transition flow cannot accidentally enable human
  Keep/Go Back actions;
- unresolved durable rollback state is never hidden behind a false-success UI;
- local UI can repaint without depending on unrelated remote desktop damage.

### Primary authority

- `working/b4a/ps2vnc_ui_tail.inc` — controller-context precedence,
  quarantine, menu interactions, confirmation ownership;
- `working/b4a/ps2vnc_ui_core.inc` — menu/confirmation/restored-ack rendering;
- `working/b4a/ps2vnc_runtime_tail.inc` — overlay safety gates and refresh/exit
  publication boundaries;
- `working/b4a/ps2vnc_display_core.inc` — B4A transition curtain and
  remote/local transaction distinction;
- B06 display-transaction evidence and frozen B4A identity/diff.

### Clean-rebuild implication

Create an explicit local-UI state/foreground model that consumes logical input
actions from B07/B08 rather than owning controller polling. Preserve underlay,
quarantine, modal ownership, transition-curtain, and durable-ack semantics.
Give local UI a first-class render invalidation/wake path so its correctness is
not coupled to artificial remote pointer traffic.

---

## Cross-domain conclusions from B07-B09

The three families share historical machinery but should not be one clean
module.

A useful target separation is:

- **B07 input/controller** — physical controller owner, pointer/scroll response,
  gesture/hotkey recognition, libpad handoff, logical actions;
- **B08 keyboard/OSK** — remote keyboard event semantics and local keyboard state;
- **B09 local UI** — foreground ownership, menus/modals/curtains, quarantine,
  selection/focus, local presentation state.

They meet through narrow action/state interfaces rather than shared access to a
controller-thread global namespace.

The most important shared reconstruction rule is:

> Input correctness is an ownership problem across time, not only a mapping from
> a button bit to an action.

A clean design must know who owns physical input now, what logical actions have
already been published, which held buttons remain quarantined, and whether a
context/transaction boundary invalidates prior derived state.

## Evidence-strength notes

- Recovered Test11 provides explicit **PROVEN HISTORICAL FOUNDATION** for the
  pointer/keyboard progression and known-good Test11L state.
- Final B4A/current source is authoritative for current mappings and later
  arbitration/UI behavior.
- Test13I supplies later implementation history for Triangle+D-pad scrolling.
- Frozen B4A is a known tested historical DUT, and M0 documents inherited
  hardware-validation authority for its byte-identical reproduction.
- The exact B4A hard-curtain mechanism is preserved source/diff authority. A
  standalone B4A operator log was not imported into the successor repository,
  so this document does not invent a more granular hardware artifact than the
  project currently exposes.

## Next audit boundary

B07, B08, and B09 are ready to be marked `EVIDENCE_SUPPORTED` in the top-level
inventory.

The next grouped semantic audit should cover B10/B11:

- B10 human-readable configuration, validation, persistence, and bindings;
- B11 manual refresh, explicit-error recovery, reconnect policy, and Pi
  management transactions.

Those domains share management/configuration endpoints but must preserve the
policy-versus-mechanism distinction already established by B02/B03/B06/B09.
