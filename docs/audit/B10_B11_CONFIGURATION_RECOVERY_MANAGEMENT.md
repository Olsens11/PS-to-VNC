# B10-B11 Configuration, Recovery, and Management Behavioral Audit

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    TRANCHE=B10_B11
    MATURITY=EVIDENCE_SUPPORTED
    ROLE=DETAILED_BEHAVIOR_AUDIT

This document records the source-plus-historical-evidence audit for human-readable
configuration/persistence and recovery/management behavior. It is subordinate
to `docs/audit/BEHAVIORAL_INVENTORY.md`.

The reconstruction rule remains:

> preserve behavior and proven mechanisms, not accidental historical structure.

The historical implementation places configuration parsing, HTTP management,
recovery, display-policy persistence, and durable display transactions close
together. The clean product should separate those responsibilities according to
their semantics and reliability requirements.

---

## B10 — Human-readable configuration, validation, persistence, and bindings

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Provide a deliberately human-readable configuration surface that can be
inspected, backed up, edited manually, and eventually edited through the local
UI, while ensuring untrusted or partially edited text cannot directly corrupt
live runtime state.

B10 owns configuration syntax/validation, user-visible desired settings, and
ordinary persistence semantics. It does not own crash-safe display-transaction
authority, which is a stricter management responsibility coordinated with B06
and B11.

### Historical configuration transport

The PS2 retrieves configuration from the Pi over a TCP 5959 management
connection using HTTP/1.0:

    GET /ps2vnc.conf

This connection is independent of the RFB socket. The PS2 deliberately does not
know the Pi filesystem path of the backing file. HTTP response header/body sizes
are bounded, only status 200 is accepted, and the received body remains
untrusted until parsing/validation completes.

Configuration retrieval failure is non-fatal during ordinary startup. The
historical code falls back to calibration/default behavior rather than making a
missing management service or malformed user file an unconditional boot
failure.

### Human-readable grammar

The established configuration format is intentionally data, never executable
code. It permits:

- blank lines;
- `#` comments;
- named sections;
- whitespace around keys/values;
- unknown/future sections;
- unknown/future keys inside known sections.

Unknown content is intentionally opaque to an older build so a newer
configuration can remain usable after rollback to an older PS2VNC build.

Recognized settings are strict. The parser rejects, where applicable:

- embedded NUL bytes;
- oversized documents/lines;
- malformed recognized section syntax;
- duplicate recognized sections or keys where ownership would become
  ambiguous;
- empty recognized values;
- extra `=` junk in recognized values;
- non-decimal/trailing-junk integers;
- signedness violations;
- integer overflow;
- incomplete current display presets;
- geometrically impossible safe-area values.

The normalized integer parser performs its own decimal magnitude preflight
because the observed PS2 libc `strtol()` behavior is not trusted as the sole
overflow detector.

### Atomic parse/publication rule

Parsing is side-effect free with respect to live display state. A recognized
preset is accumulated in temporary state and is published only after the whole
recognized preset passes syntax, integer, completeness, and geometry
validation.

A partly valid document must not produce partly updated live state.

Applying display configuration passes through the same geometry acceptance gate
used by runtime calibration rather than assigning `screen_fit_*` fields
individually.

### Startup mode and calibration are separate settings

The global `[display]` section can contain:

    startup_mode = <symbolic mode name>

Startup-mode selection is parsed independently from per-mode calibration. A
missing setting leaves the compiled/default mode in force; an invalid setting
is reported and also falls back rather than making startup impossible.

Per-mode calibration is stored in:

    [display.<mode>]
    safe_width  = ...
    safe_height = ...
    safe_x      = ...
    safe_y      = ...

The historical `[display]` safe-area section remains only as a backward-
compatible 1080i calibration slot. If a mode-specific keyed preset exists but
is invalid, that invalid keyed preset remains authoritative; the parser does
not hide it by silently falling back to legacy data.

The persisted safe rectangle is exactly the selected safe desktop. Calibration
border decoration is visualization only and does not alter the persisted
geometry.

### Ordinary persistence operations

The historical PS2 uses dedicated management writes for distinct setting
classes:

- calibration save posts the explicit mode plus exact safe rectangle;
- startup-mode save posts the symbolic startup mode separately;
- display lock/unlock policy posts the explicit target mode plus boolean state;
- hide-locked-modes posts a separate boolean UI policy.

All require a valid HTTP 200 acknowledgement for the write operation to report
success.

A calibration accepted in the current session remains valid in that session if
its ordinary persistence write fails. The persistence failure is reportable but
does not retroactively invalidate accepted live geometry.

### Display policy cache

Display lock/hide policy is parsed from the same human-readable configuration
surface but is cached separately from calibration. Missing lock/hide keys mean
false. The complete new policy cache is published only after the whole relevant
document parses successfully.

The active display mode cannot be locked through the historical local policy
write path.

### Human configuration versus durable transaction authority

A central audit conclusion is that these are not one abstraction:

1. human-editable desired configuration;
2. validated live runtime state;
3. crash-safe cross-machine transactional authority.

Risky display changes use dedicated durable begin/commit/restore/restored/ack
management transitions carrying complete profiles. They must not be reduced to
"rewrite the config file and hope". B06 owns the display-transaction product
semantics; B11 owns the management/reconciliation mechanism boundary.

### Controller/hotkey binding gap

The final historical runtime already has a useful semantic binding model:

- button mask;
- logical action;
- trigger kind (`SETTLE`, `RELEASE`, `HOLD`);
- context (`DESKTOP`, `GLOBAL`).

However the final B4A binding table is compiled into the program. The audit has
not found evidence that B4A completed a human-readable controller-binding
configuration contract.

Therefore the clean reconstruction must not pretend this feature already
exists. It should preserve the typed logical-action/trigger/context model while
making user-adjustable bindings explicit in the human-readable configuration
and editable through a future local UI. The config remains the deliberate
manual/backup/advanced interface rather than hidden UI-only state.

### Historical evidence

M4F normalized the reusable config text helpers into `src/config/text.c/.h`
without changing product logic. Host unit tests passed, and the resulting
PT_LOAD was byte-for-byte identical to the already hardware-qualified M4A
PT_LOAD, so hardware qualification was transferred on an explicit byte-identity
basis rather than inferred from source similarity.

The mode/calibration/persistence behavior is additionally exercised as part of
the B06 successor display qualification, including the final five-mode
hardware regression and persisted startup-mode checks.

### Invariants

- configuration is data, never executable code;
- untrusted text never mutates live state while it is being parsed;
- recognized settings fail closed on malformed/duplicate/overflowing data;
- unknown future configuration remains forward-compatible where ownership is
  unambiguous;
- a current display preset is accepted atomically, not field by field;
- startup-mode identity and per-mode safe-area calibration remain separate;
- ordinary user configuration and crash-safe transaction authority remain
  separate concepts;
- persistence failure does not rewrite the meaning of already accepted live
  calibration;
- user-adjustable controller/hotkey bindings must eventually exist in the
  human-readable configuration rather than only compiled tables.

### Primary authority

- `src/config/text.c` / `src/config/text.h`;
- `working/b4a/ps2vnc_management_core.inc`;
- `working/b4a/ps2vnc_management_tail.inc`;
- `working/b4a/ps2vnc_calibration_core.inc`;
- `working/b4a/ps2vnc_runtime_core.inc` — historical typed hotkey model;
- `evidence/m4/m4f-config-text-normalization/`;
- `evidence/m4/m4f-source-authority/SOURCE-AUTHORITY.env`;
- B06 display qualification for integrated persistence behavior.

### Clean-rebuild implication

Create a small explicit configuration model with a tolerant document reader and
strict typed validators. Parse into temporary/typed state, validate complete
settings, then publish. Treat persistence adapters separately from parsing.
Represent controller bindings with stable semantic action/trigger/context
values in the config, with UI as a front end to the same data rather than a
second hidden authority.

---

## B11 — Manual refresh, recovery policy, reconnect mechanisms, and Pi management transactions

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Detect explicit connectivity/session failures, execute deliberate session
replacement, support user-requested manual Refresh without obscuring silent
failures during debugging, coordinate Pi management operations, and reconcile
cross-machine state where an operation must survive interruption.

B11 must keep five concepts distinguishable:

1. failure observation;
2. recovery policy;
3. recovery mechanism;
4. user-visible recovery state;
5. durable/cross-machine reconciliation.

### Test12 recovery foundation

Test12 is the historical direct recovery foundation. It established that
recovery is a two-sided PS2/Pi system.

PS2 responsibilities proven there include:

- proactive physical Ethernet-carrier detection;
- detection of VNC/RFB connection failure;
- visible recovery status;
- bounded reconnect attempts;
- complete live-session restoration when connectivity returns;
- explicit terminal failure rather than an indefinitely silent frozen desktop.

The historical constants were ten retry attempts over approximately ten
seconds.

Pi responsibilities proven there include:

- keeping the dedicated PS2 Ethernet administratively ready even without
  carrier;
- supervising the VNC/X/Openbox/panel stack with systemd;
- checking actual endpoint/display usability rather than merely process
  existence;
- restarting VNC when the service appears active but its listener/X display is
  unusable;
- explicit status/start/stop/restart operator commands.

This establishes an important recovery rule: process existence is not equivalent
to service health.

### RFB replacement mechanism

The qualified recovery path does not attempt to continue a dead/suspect stream.
It:

1. clears published RFB-connected state;
2. resets buffered receive/outbound transport state;
3. closes the old socket;
4. retries only while Ethernet is usable and the application is not exiting;
5. creates a fresh TCP/RFB session;
6. re-negotiates pixel format and Raw encoding;
7. requests/receives a complete authoritative framebuffer;
8. re-probes ExtendedDesktopSize capability;
9. if a restarted TigerVNC reports bootstrap geometry, explicitly converges it
   back to the saved authoritative desktop geometry;
10. publishes the complete replacement framebuffer through the active
    presentation backend;
11. restores normal live encoding;
12. only then republishes the session as connected.

This is mechanism. Policy determines why the replacement was requested.

### Automatic recovery policy for explicit failures

Test12 supports automatic entry into reconnect for explicit failure conditions,
including:

- socket EOF/error;
- failed update request/receive path;
- proactive Ethernet carrier loss.

The live loop also distinguishes display-transaction ownership. If an RFB
failure occurs while a risky display candidate remains unconfirmed, the failure
belongs to that display transaction and triggers rollback of the candidate
rather than generic recovery into an unapproved display profile. Once KEEP has
been explicitly chosen, an unrelated later RFB failure no longer rejects the
selected display mode.

### Silent-stall policy

An unexplained silent stall is deliberately different.

After the controller/main-thread nonblocking receive path is active,
`recv_exact()` may remain in `EAGAIN/EWOULDBLOCK` while waiting for legitimate
remote damage. The final historical source does **not** turn elapsed silent wait
time into a generic automatic reconnect watchdog.

The receive loop exits only for explicit causes such as:

- application exit;
- display rollback request;
- explicit manual Refresh request;
- socket EOF/non-EAGAIN error;
- physical carrier loss;
- the specific remote-display safety lease used by controlled temporary display
  transactions.

Therefore the debugging policy remains evidence-compatible: silent stalls stay
failed/observable until the user explicitly intervenes. Future automatic stall
recovery is a separate product-policy decision and must not be smuggled in as a
transport timeout.

### Manual Refresh semantics

Manual Refresh is a semantic runtime action that reuses the already-proven
live-session replacement transaction.

It has one admission gate. A request is rejected when:

- the system overlay/action context is unsafe;
- Refresh is already `REFRESHING` or in `COOLDOWN`;
- another runtime action is pending.

On acceptance it immediately owns the semantic request and publishes visible
`REFRESHING` status.

If Refresh arrives while the main thread is blocked inside `recv_exact()`, the
receive helper deliberately returns failure even if part of the old RFB
structure has already been consumed. The old stream is therefore treated as
suspect and replaced; the code does not guess a new protocol boundary.

After successful replacement:

- status becomes visibly complete;
- Refresh enters a three-second cooldown;
- repeated button presses/chords cannot retrigger recovery during that period;
- expiry returns the state to READY and clears transient status.

This is runtime anti-spam policy, not merely a menu-animation convention.

### Recovery UI and input relationship

Recovery status is rendered locally using the PS2 presentation path, so a dead
VNC stream does not prevent the user from seeing Ethernet/VNC reconnect status.
The controller thread can continue polling during ordinary recovery, while
`rfb_connected`/queue-reset rules prevent stale remote input from being carried
into the replacement session.

The System menu remains foreground through manual replacement and communicates
refreshing/completed/cooldown state explicitly.

### Display rollback transport boundary

Display rollback uses a stricter transport helper when the candidate stream may
have been interrupted mid-message. It discards the suspect socket, performs a
fresh synchronized handshake under a temporarily widened geometry envelope,
and stops before normal GS publication so B06 can reconstruct the saved
complete known-good display profile.

A known-clean stream may be reused by the higher-level rollback path; a stream
that may be desynchronized is never guessed back into alignment.

### Management transport versus management policy

The historical TCP 5959 HTTP client is a transport mechanism used by several
very different semantic clients:

- read-only human configuration retrieval;
- ordinary configuration/policy writes;
- durable display begin/commit/restore/restored/ack transitions;
- later development/hardware-test remote display-control polling/results.

Sharing a port/protocol does not make these operations one reliability class.

For example, remote display-control polling is explicitly best-effort
development control: an absent management service must not damage VNC. It also
waits rather than stealing ownership from an active local display transaction.

Durable display transaction writes are different: their acknowledgement is part
of global safety/reconciliation, and failure cannot be silently treated as
success.

### Remote temporary-display lease

The later test-control path can request a temporary mode with a transaction ID
and lease. The PS2 refuses invalid/same-mode targets, does not steal local
transaction ownership, reports state/results, and forces rollback when the
specific lease expires or an explicit restore arrives.

This lease is a controlled display-test safety mechanism. It is not authority to
invent a general silent-RFB-stall watchdog.

### Durable cross-machine reconciliation

The B06 display transaction demonstrates the strongest B11 management rule:
local success and global success are not always the same thing.

A rollback may reconstruct the old local profile successfully while the Pi-side
durable transition fails. That outcome is not reported as a clean global
success. Begin/commit/restore/restored/ack transitions exist specifically so
startup and later operations can determine which complete profile owns
authority after interruption.

### Evidence-strength distinction

- Test12 gives direct historical authority for PS2 reconnect behavior and the
  supervised Pi runtime.
- B06 successor hardware evidence qualifies the integrated durable display
  transaction/rollback/persistence behavior across the principal display-mode
  matrix.
- B4A/final source precisely documents the later manual Refresh/cooldown and
  development remote-control mechanisms.
- The audit does not claim that every later individual management helper has a
  separately isolated hardware test merely because it exists in the final
  tested B4A image.

### Invariants

- explicit failure recovery and unexplained silent-stall policy remain separate;
- suspect RFB byte streams are replaced rather than guessed back into sync;
- manual Refresh may deliberately interrupt a silent/partial receive and then
  treats the old stream as suspect;
- manual Refresh has one admission gate, visible state, and anti-spam cooldown;
- automatic recovery remains appropriate for explicit carrier/socket/RFB
  failures already supported by evidence;
- an unconfirmed display candidate owns its failures and rolls back rather than
  being legitimized by generic recovery;
- local recovery success is not silently equated with durable cross-machine
  success;
- management clients with different reliability semantics remain distinguishable
  even when they share TCP 5959/HTTP;
- development/test control remains best-effort and cannot become an accidental
  runtime dependency;
- Pi service health is judged by usable endpoints/dependencies, not process
  existence alone.

### Primary authority

- legacy `docs/tests/TEST12_RECOVERY.md`;
- `working/b4a/ps2vnc_management_core.inc` — reconnect/manual Refresh/config
  management mechanisms;
- `working/b4a/ps2vnc_runtime_core.inc` / `ps2vnc_runtime_tail.inc` — receive
  interruption, carrier detection, Refresh admission/cooldown;
- `working/b4a/ps2vnc_display_tail.inc` — live failure policy/transaction
  ownership;
- `working/b4a/ps2vnc_display_core.inc` — durable rollback and development
  remote-control lease/orchestration;
- B06 display-transaction evidence and final M3/M4 hardware qualification.

### Clean-rebuild implication

Provide explicit layers for recovery policy, RFB replacement mechanics, local
recovery presentation, and Pi management clients. Keep a generic bounded HTTP
transport small and boring, but give each semantic client its own contract.
Manual Refresh should be a first-class action with visible lifecycle/cooldown.
Do not add a silent-stall watchdog during reconstruction merely because a
reconnect mechanism exists. Pi runtime health/restart behavior should integrate
with the reproducible companion-service architecture rather than recreate a
parallel supervisor.

---

## Cross-domain conclusions from B10/B11

1. **Human configuration is not durable transaction state.** A text file may
   express desired persistent settings; safety-critical interrupted operations
   need explicit transactional authority.
2. **Parsing is not publication.** Untrusted text becomes typed live state only
   after complete validation.
3. **Recovery policy is not recovery mechanism.** The same session-replacement
   primitive can service explicit socket failure or manual Refresh without
   implying an automatic silent-stall watchdog.
4. **Management transport is not management semantics.** Configuration,
   durable transactions, and development control may share HTTP/TCP 5959 while
   retaining different failure policies.
5. **Local success is not always global success.** Cross-machine durable state
   must be reconciled explicitly.
6. **Config/UI duality should be intentional.** Future binding/config UI should
   edit the same human-readable model rather than create hidden parallel state.

## Rebuild-readiness note

B10 and B11 are now evidence-supported enough to inform the clean architecture.
Their historical file boundaries are not proposed module boundaries. Final
`REBUILD_READY` promotion should occur only after the remaining behavior
families and cross-domain state ownership have been reconciled into one clean
architecture pass.
