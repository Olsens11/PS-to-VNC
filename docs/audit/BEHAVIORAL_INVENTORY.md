# PS-to-VNC Behavioral Inventory

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    INVENTORY_STRUCTURE=ACTIVE
    DETAILED_BEHAVIOR_AUDIT=IN_PROGRESS
    FIRST_TRANCHE=B01_B04_EVIDENCE_SUPPORTED

This is the top-level inventory of product behaviors and responsibilities that
must be understood before the clean architecture is derived.

Maturity meanings are defined in `docs/audit/README.md`:

- `SEEDED` — responsibility/evidence roots identified;
- `SOURCE_MAPPED` — implementation/state located;
- `EVIDENCE_SUPPORTED` — important behavior reconciled with historical tests or
  evidence;
- `REBUILD_READY` — sufficiently understood to design the clean replacement.

A maturity level describes understanding, not acceptance of the historical
implementation structure. B4A/current files remain behavioral/reference
authority, not a module blueprint.

## Inventory

| ID | Behavior family | Maturity | Evidence roots |
|---|---|---|---|
| B01 | Startup and application lifecycle | EVIDENCE_SUPPORTED | B4A/current startup source, successor startup evidence, legacy Test14 cross-layer evidence |
| B02 | PS2-to-Pi Ethernet and networking | EVIDENCE_SUPPORTED | B4A/current source, frozen PS2IP dependency, early VNC baseline, legacy Test9/Test10/Test12 history |
| B03 | RFB connection, handshake, negotiation, and session state | EVIDENCE_SUPPORTED | B4A/current RFB/recovery source, early VNC baseline, legacy Test12 recovery history |
| B04 | Framebuffer updates, rectangles, Raw/Hextile decode, and framebuffer validity | EVIDENCE_SUPPORTED | B4A/current decode path, legacy Test9/Test10 history, successor evidence snapshots |
| B05 | GS/video presentation and framebuffer-to-display transfer | SEEDED | B4A/current video code, `src/video/`, display evidence |
| B06 | Display modes, display transactions, geometry, and safe-area calibration | SEEDED | display-mode source, M4 display evidence, historical hardware qualification |
| B07 | Controller acquisition, pointer semantics, clicks, and logical actions | SEEDED | controller/input source, controller tests and diagnostics |
| B08 | Keyboard, on-screen keyboard, modifiers, and text interaction | SEEDED | OSK/input/UI source and interaction history |
| B09 | Menus, overlays, curtains, status presentation, and local UI flow | SEEDED | UI source, display-transition/UI evidence |
| B10 | Human-readable configuration, validation, persistence, and bindings | SEEDED | configuration source/modules, config tests and migration evidence |
| B11 | Manual refresh, recovery policy, reconnect mechanisms, and Pi management transactions | SEEDED | management/recovery source, rollback/recovery evidence |
| B12 | Diagnostics, runtime identity, telemetry, profiling, and reporting | SEEDED | `src/diagnostics/`, identity/profiling evidence |
| B13 | Pi-side VNC desktop, management/service, networking, and companion responsibilities | SEEDED | preserved Pi environment, service/config census, product intent |
| B14 | Build, deployment, test, evidence, and qualification mechanisms that constrain product development | SEEDED | `scripts/`, TestKit history, `evidence/`, development-system lessons |

## B01 — Startup and application lifecycle

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Bring the PS2 application from ELF entry to a usable remote-desktop session,
then converge explicit exit or fatal live failure to a known PS2 outcome.
Startup is an application orchestration responsibility; it coordinates platform,
network, configuration, display, RFB, input, and diagnostics without implying
that one future module should own those subsystems.

### Source-supported startup sequence

The qualified B4A/current path performs these steps in order:

1. allocate the maximum EE framebuffer backing store and initialize it;
2. initialize SIF RPC, reset/synchronize the IOP, initialize loadfile/IOP heap,
   and enable load-module-buffer support;
3. load embedded DEV9, NETMAN, and SMAP modules and initialize NetMan;
4. initialize visible debug output;
5. load/init/open libpad, force DualShock analog mode, and sample the explicit
   startup force-calibration chord;
6. configure PS2 IP `192.168.50.2/24`, gateway/Pi `192.168.50.1`, and initialize
   PS2IP;
7. wait for a valid Ethernet link;
8. retrieve general configuration from the Pi; failure is deliberately
   non-fatal;
9. preflight/prepare any durable display transaction and apply exactly one
   authoritative startup display profile;
10. initialize optional diagnostics; diagnostic failure is non-fatal;
11. reset RFB receive/transport diagnostic state;
12. establish a reconciled RFB session and obtain startup framebuffer state;
13. enter the live desktop loop;
14. explicit application exit or fatal live-loop return converges on
   `LoadExecPS2("rom0:OSDSYS", ...)`.

If `LoadExecPS2()` unexpectedly returns, the historical implementation enters a
visible stopped state instead of continuing through an unknown lifecycle.

### Failure semantics

The historical source distinguishes failure classes rather than treating all
initialization failures identically:

- framebuffer allocation returns from `main()`;
- early controller/module and PS2IP-init failures stop visibly and require reset;
- startup Ethernet-link failure returns to OSDSYS;
- unsafe display-transaction state, failed authoritative-profile application,
  or failed VNC startup returns to OSDSYS;
- configuration retrieval and diagnostics initialization are non-fatal.

These exact choices are historical behavior to understand; the clean design may
refine user-facing policy while preserving the proven dependency ordering and
safe outcomes.

### Exit semantics and terminology

The application-level exit publication gate sets the semantic
`EXIT_TO_SYSTEM_MENU` action and shared shutdown request, subject to overlay and
provisional-display safety rules. Long-running controller/recovery/management
paths observe that request rather than beginning incompatible work.

Do **not** confuse this with PS2SDK `ExitHandler()`. In this project,
`ExitHandler()` is the EE interrupt-handler return workaround involved in the
HIRES GS HSync investigation. It is primarily a B05 platform/presentation
mechanism. Application exit is the B01 OSDSYS/`LoadExecPS2` lifecycle path.

### Historical evidence and durable lessons

Successor startup evidence preserves the same OSDSYS convergence path rather
than only a source-level intention. Test14 adds a cross-layer lesson: a visible
desktop/HIRES freeze did not necessarily mean the whole application was dead;
other threads remained alive while GS HSync synchronization stopped. The later
interrupt-discipline fix belongs to B05, but B01 must preserve the broader
lesson that lifecycle decisions should be based on subsystem state rather than
the single symptom "screen frozen."

### State/ownership implications

The historical implementation exposes lifecycle through shared globals
(`exit_requested`, controller state, display-transaction state, network/RFB
state). The clean architecture should instead make application lifecycle and
shutdown intent explicit while leaving subsystem state with its actual owner.

### Invariants

- ordinary RFB startup does not begin before the network foundation is usable;
- durable display authority is resolved before ordinary RFB operation;
- optional diagnostics cannot become a product-start prerequisite;
- explicit exit must not silently cut across an unsafe provisional display
  transaction;
- fatal startup/live outcomes converge to a known visible state;
- `ExitHandler()` and application exit remain semantically separate concepts.

### Primary authority

- `working/b4a/ps2vnc_display_tail.inc` — `main()` lifecycle orchestration;
- `working/b4a/ps2vnc_runtime_tail.inc` — explicit exit publication;
- `working/b4a/ps2vnc_calibration_core.inc` — startup display reconciliation;
- `baseline/frozen-b4a/ps2ip.c` — frozen B4A reference;
- successor M3/M4 startup evidence;
- legacy Test14 history only for the cross-layer freeze-classification lesson.

### Clean-rebuild implication

Provide one readable application orchestrator with narrow calls for platform
setup, network readiness, configuration, display authority, RFB startup, live
operation, and shutdown. Do not reproduce the old global ownership graph merely
because it carried the qualified behavior.

## B02 — PS2-to-Pi Ethernet and networking

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Provide PS2 Ethernet/IP/link and generic TCP foundations used by higher-level
PS-to-VNC protocols. B02 ends below RFB framing and below Pi-management request
semantics.

### Proven topology and platform path

The long-lived private-network convention is:

    Pi:   192.168.50.1
    PS2:  192.168.50.2/24
    VNC:  TCP 5900
    management/config: TCP 5959

The historical startup path loads DEV9/NETMAN/SMAP, initializes NetMan, calls
PS2IP with the static address/netmask/gateway, waits for link-up, and only then
allows higher-level clients to proceed. RFB and management use separate TCP
connections even though they share the same PS2IP stack and Pi address.

The early-VNC historical harvest independently establishes that this private
network, RFB 3.8, Security None, and Raw framebuffer reception existed before
the later stability campaign.

### Test9/Test10 network lesson

Test9 deliberately varied network/update behavior, CPU framebuffer behavior,
and timing while keeping full GS upload in the matrix:

- 9A: incremental network + partial CPU framebuffer + full GS -> FAIL;
- 9B: full network + full CPU framebuffer rewrite + full GS -> PASS;
- 9C: incremental network + partial CPU framebuffer + slowed path + full GS ->
  FAIL;
- 9D was defined as incremental network + full CPU framebuffer + full GS, but
  the preserved Test9 document does not establish a final result and the audit
  does not invent one.

Therefore full GS upload alone could not explain the failures, and merely
slowing the incremental path did not cure them. Test9 narrowed the investigation
but did not prove root cause.

Test10 supplied the stronger discriminator. Under stock MTU1500, malformed RFB
rectangle bytes were already present in captured receive data before later CPU
framebuffer mutation. A paired MTU1458 build survived the same stress, and
Test10D restored normal live incremental rendering with MTU1458.

Durable conclusion: the apparent incremental/framebuffer freeze crossed the
network/receive boundary; incremental rendering itself was not proven
inherently unsafe.

### Qualified PS2IP dependency

Frozen B4A does not link an arbitrary SDK PS2IP archive. Its exact qualified
archive is preserved as `baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a`.
The preserved patch includes MTU1458 plus larger PBUF/window configuration and
TCP receive-window scaling.

Evidence-strength distinction matters:

- MTU1458 has direct Test10 paired behavioral evidence;
- the later PBUF/window-scale settings are part of the exact B4A dependency
  authority, but Test10 alone does not prove each of those settings was
  independently necessary.

The clean rebuild must preserve/requalify known network constraints instead of
silently substituting a nominally similar PS2IP build.

### Link loss and recovery boundary

NetMan provides explicit Ethernet carrier state. Test12 demonstrated that
physical carrier loss may leave a socket path otherwise waiting indefinitely,
so proactive link-state detection is required to turn an explicit link failure
into a recoverable event.

B02 reports mechanism/state: initialized, link ready/lost, socket/connect
failure. B03/B11 decide session replacement and product-facing recovery policy.

### Invariants

- higher-level TCP consumers do not assume link readiness before B02 reports it;
- RFB and management traffic remain distinct consumers;
- network/library identity that changes proven behavior requires qualification;
- a link/socket symptom must not be automatically mislabeled as RFB decode or GS
  failure;
- the audit distinguishes proven MTU behavior from later dependency settings
  that are authoritative but not independently isolated.

### Primary authority

- B4A/current network startup and socket code;
- `baseline/frozen-b4a/ps2ip-mtu1458-wscale128.patch`;
- `docs/M0_BUILD_PROVENANCE.md`;
- legacy `docs/tests/EARLY_VNC_FOUNDATION.md`;
- legacy `docs/tests/TEST09_FRAMEBUFFER.md`;
- legacy `docs/tests/TEST10_NETWORK.md`;
- legacy `docs/tests/TEST12_RECOVERY.md`.

### Clean-rebuild implication

Create a small PS2 network/platform boundary for initialization and link
readiness. Keep RFB, management protocol semantics, display rollback, and UI
policy outside it. Pin or deliberately rebuild/requalify the PS2IP dependency
rather than treating SDK networking as an interchangeable black box.

## B03 — RFB connection, handshake, negotiation, and session state

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Own the mechanics/state of one RFB session: TCP connection, protocol/security
negotiation, ServerInit acceptance, pixel/encoding setup, update requests,
transport synchronization, and complete session replacement.

### Handshake contract

The qualified client:

1. connects to the Pi on TCP 5900;
2. negotiates RFB 3.8 using `RFB 003.008`;
3. reads offered security types and requires SecurityType None (`1`);
4. validates successful SecurityResult;
5. sends shared ClientInit;
6. reads ServerInit;
7. validates nonzero desktop geometry against a caller-owned acceptance
   envelope that can never exceed allocated backing-store capacity;
8. when geometry is locked, requires exact geometry agreement;
9. consumes the entire desktop-name field even when retaining only a bounded
   printable prefix;
10. returns a socket only at a synchronized post-ServerInit boundary.

Ordinary callers use the normal output-size envelope. Startup reconciliation or
display rollback may explicitly widen the temporary acceptance envelope up to
the allocated maximum. Widening transport acceptance never grants display
policy authority.

### Session establishment

A fresh historical session begins from a deliberately conservative known state:

- negotiate the PS2/GS-compatible 16-bit true-color pixel format;
- select Raw encoding first;
- issue a non-incremental framebuffer request;
- consume startup/recovery framebuffer state;
- probe ExtendedDesktopSize separately;
- select normal live encoding only after the replacement session is coherent.

`FramebufferUpdateRequest` semantics are explicit: flag `0` requests the full
requested region; flag `1` requests only damage since the prior request.
Queued outbound controller/RFB traffic is flushed before the main-thread update
request so independent client writes do not interleave on the stream.

### Recovery/session replacement

Test12 and the final source support replacement rather than reuse of a suspect
RFB stream:

- clear published RFB-connected state;
- close the old socket;
- reset receive/parser transport state;
- establish a new TCP/RFB session;
- re-negotiate pixel format/Raw and capability state;
- obtain authoritative framebuffer state;
- converge server geometry to higher-level display authority when necessary;
- publish the replacement session only when it is ready.

Display rollback is stricter: if an operation may have interrupted a blocking
receive mid-message, that old byte stream is discarded. The client does not
attempt to guess the next protocol boundary.

### Explicit failure versus silent-stall policy

Test12 proves visible automatic recovery for explicit link/socket/RFB failure,
including proactive Ethernet-carrier detection. That does **not** imply that an
unexplained silent stall should always be auto-healed. During debugging, silent
freeze/stall recovery is deliberately manual so the failure remains observable.
The mechanism for replacing an RFB session is B03; the decision of when to
invoke it is B11 policy.

### Failure behavior

Handshake/session establishment fails closed on socket/connect failure,
incomplete fields, server rejection, missing SecurityType None, failed security
result, invalid/out-of-envelope geometry, locked-geometry mismatch, or
incomplete consumption of variable-length fields. A failed/suspect socket is
closed rather than recycled.

### State/ownership implications

A clean RFB session object should make explicit at least:

- socket/session validity;
- negotiated/current desktop geometry and geometry-lock state;
- receive-buffer/parser position;
- outstanding framebuffer-request state;
- outbound-message serialization;
- negotiated encoding/capability state;
- publication of session readiness to input/UI consumers.

### Invariants

- never reuse a stream that may be desynchronized;
- never widen geometry acceptance implicitly;
- transport acceptance does not change display authority;
- a replacement session starts from known synchronized protocol state;
- independent socket writes cannot interleave;
- RFB reconnect mechanics do not decide silent-stall recovery policy.

### Primary authority

- `working/b4a/ps2vnc_runtime_tail.inc` — handshake/link/socket mechanisms;
- `working/b4a/ps2vnc_management_core.inc` — initial/recovery/replacement flows;
- `working/b4a/ps2vnc_framebuffer_core.inc` — framebuffer request mechanism;
- frozen B4A reference;
- legacy early-VNC foundation and Test12 recovery history.

### Clean-rebuild implication

Represent RFB as an explicit session/transport responsibility with ordinary
operations for connect/handshake, negotiate, request, receive, replace, and
close. Display/UI/recovery code should coordinate those operations without
owning parser internals.

## B04 — Framebuffer update processing

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Consume synchronized RFB server messages, parse FramebufferUpdate framing,
decode supported rectangles, maintain coherent logical/GS-ready framebuffer
state, and report changed regions. B04 ends before GS/DMA presentation policy.

### Server-message framing

The decoder must not assume the next server byte always begins a
FramebufferUpdate. The qualified source handles:

- type 0 — FramebufferUpdate;
- type 1 — SetColorMapEntries, fully consumed/ignored under true color;
- type 2 — Bell, ignored with no payload;
- type 3 — ServerCutText, whose declared payload is fully consumed/ignored.

The source records the historical incident that xterm selection could cause
TigerVNC to send ServerCutText; earlier code treated the next byte as if it were
always a framebuffer message and the symptom looked like a VNC/video freeze.
No separately sealed legacy test for that incident has yet been located, so the
audit classifies the rule itself as `SOURCE_SUPPORTED` and the incident note as
historical source commentary rather than upgrading it to independent
`HARDWARE_PROVEN` evidence.

Unknown server-message types fail closed because their payload length cannot be
safely guessed.

### FramebufferUpdate framing and validation

For each update the qualified path:

1. reads padding/rectangle count exactly;
2. allows zero rectangles for normal incremental operation but can require a
   stronger complete-frame contract when a caller asks for it;
3. reads every 12-byte rectangle header exactly;
4. parses x/y/width/height/encoding;
5. consumes ExtendedDesktopSize as metadata rather than pixels and validates it
   against pending-resize/current geometry state;
6. accepts Raw and Hextile for pixel-bearing rectangles;
7. rejects unsupported encoding values;
8. validates every pixel rectangle is inside the logical framebuffer;
9. records the union/bounds of changed rectangles;
10. consumes the exact rectangle payload.

### Raw behavior

Pixels arrive in the requested GS-compatible B5:G5:R5 wire ordering. Each Raw
row requires exactly `width * 2` bytes. Short pixel data is fatal to the stream.
The local GS A1 bit is added when producing GS-ready pixels.

The current qualified source may alias RFB and GS storage; that is an
optimization/history detail, not a required future architecture.

Some recovery callers use a stronger `require_full` contract that validates a
complete authoritative Raw frame. Ordinary initial non-incremental requests and
recovery callers are not identical, so the clean design should express the
required frame-validity level explicitly rather than infer it solely from the
RFB incremental flag.

### Hextile behavior

Hextile is decoded in 16x16-or-smaller tiles. The decoder carries protocol
background/foreground state and supports Raw tiles and subrectangles while
validating:

- only defined Hextile flag bits;
- required background/foreground state;
- illegal ForegroundSpecified/SubrectsColored combinations;
- subrectangle bounds inside the tile;
- exact receipt of all pixel and geometry fields.

Malformed Hextile terminates the update rather than publishing partially trusted
state.

### Framebuffer validity

The exploratory implementation developed direct-write/presentation paths that
can make a conventional linear framebuffer stale. The durable requirement is
not that exact optimization; it is that representation validity is explicit.
The qualified source refuses combinations where an incremental update would
silently merge new data with a stale/incompatible representation.

### Test9/Test10 lesson

Test9 associated failures with the incremental/partial-update path but could not
isolate whether network receive or CPU framebuffer handling was responsible;
full GS upload occurred in both pass and fail cases, and slowing the incremental
path did not cure the failure.

Test10 then captured malformed rectangle bytes already in receive data under the
stock network condition, before later framebuffer mutation, while MTU1458
survived the paired stress and restored live incremental rendering. The durable
lesson is layered: malformed protocol/framebuffer symptoms may originate below
the decoder, and a decoder must fail closed rather than convert corrupted
framing into a later apparent freeze.

### Cross-layer freeze lesson

Test14 separately showed that a visible high-resolution display freeze can
originate in GS interrupt/presentation behavior while other application threads
remain alive. Therefore B04 must report decoder/stream validity accurately but
must not claim ownership of every visible "video freeze." The detailed GS
interrupt contract belongs to B05.

### Invariants

- consume a message exactly or stop using that stream;
- unknown payload length is never guessed;
- rectangles modify only pixels inside the logical framebuffer;
- incomplete/malformed data cannot contaminate the next message boundary;
- callers may require stronger framebuffer completeness than ordinary live
  incremental updates;
- representation validity/staleness is explicit when multiple write paths
  exist;
- RFB decoding and GS presentation remain separate responsibilities.

### Primary authority

- `working/b4a/ps2vnc_framebuffer_core.inc` — message/rectangle/Raw/Hextile
  decode;
- `working/b4a/ps2vnc_display_tail.inc` — live incremental request loop;
- `working/b4a/ps2vnc_runtime_tail.inc` — synchronized transport helpers;
- frozen B4A reference and successor source snapshots;
- legacy Test9/Test10 history;
- Test14 only for the cross-layer freeze-classification lesson.

### Clean-rebuild implication

Build a protocol decoder around explicit session input, framebuffer/update
validity, and dirty-region output. Raw and Hextile can be internally separate if
that improves readability, but parser correctness and representation validity
must be first-class. A legal asynchronous message, short read, malformed
rectangle, or stale framebuffer may never be silently transformed into the next
apparent freeze.

## B05 — GS/video presentation

**Maturity:** `SEEDED`

Audit pending.

The audit must separate the logical framebuffer produced by RFB decoding from
PS2-specific GS/DMA/presentation mechanisms. Test14's reconstructed GS HSync
interrupt discipline and the PS2SDK `ExitHandler()` workaround are priority
evidence for this domain.

## B06 — Display modes and calibration

**Maturity:** `SEEDED`

Audit pending.

This domain includes mode selection, mode persistence, transactions,
confirmation/rollback behavior, logical versus presented geometry, safe-area
calibration, and hardware-specific invariants.

## B07 — Controller and pointer behavior

**Maturity:** `SEEDED`

Audit pending.

This domain includes controller hardware acquisition, logical button actions,
pointer movement, click semantics, pause/ack behavior, chords, and configurable
bindings.

## B08 — Keyboard and on-screen keyboard

**Maturity:** `SEEDED`

Audit pending.

This domain includes keyboard event generation, OSK navigation, modifier state,
and interaction between local UI controls and remote keyboard input.

## B09 — Local UI

**Maturity:** `SEEDED`

Audit pending.

This domain includes menus, overlays, transition curtains, current-state
presentation, selection/focus behavior, and any UI state that exists
independently of the remote desktop.

## B10 — Configuration and persistence

**Maturity:** `SEEDED`

Audit pending.

The audit must identify the human-readable configuration contract, defaults,
validation, persistence boundaries, display-mode/calibration persistence,
controller bindings, and ownership of parsed versus live state.

## B11 — Recovery and management

**Maturity:** `SEEDED`

Audit pending.

The audit must distinguish:

- mechanism from policy;
- explicit socket/link error recovery from silent-stall policy;
- manual refresh from automatic recovery;
- PS2-local state from Pi-management transactions;
- display-mode rollback/recovery from RFB-session recovery.

## B12 — Diagnostics and identity

**Maturity:** `SEEDED`

Audit pending.

This domain includes debug stages, low-overhead telemetry, profiling,
deterministic identity/report serialization, build/runtime provenance, and
evidence-facing state.

## B13 — Raspberry Pi companion

**Maturity:** `SEEDED`

Audit pending.

This domain must identify which Pi responsibilities are ordinary operating
system responsibilities and which genuinely belong to PS-to-VNC.

Areas include VNC desktop/service lifecycle, networking, management endpoints,
recovery, Samba/file access where product-significant, and future installer
state.

## B14 — Product-significant development infrastructure

**Maturity:** `SEEDED`

Audit pending.

Build, deployment, TestKit, identity, evidence capture, and hardware
qualification are not runtime product features, but some of their constraints
are part of the product's engineering definition and must survive the clean
reconstruction.

## Cross-domain rules already established

These are inputs to the audit, not conclusions about final module layout:

- evidence outranks intuition;
- historical source remains reference authority;
- migration-era module boundaries are not automatically retained;
- RFB protocol and PS2 graphics presentation are different responsibilities;
- policy and mechanism should be distinguishable;
- important mutable state requires explicit ownership;
- explicit link/socket failure recovery is not the same policy question as an
  unexplained silent stall;
- silent-freeze recovery policy must not be invented merely to make testing
  convenient;
- PS2SDK `ExitHandler()` and application exit-to-OSDSYS are different concepts;
- hardware-facing conclusions ultimately require physical PS2 qualification.

## Next audit action

Audit B05 and B06 together because GS/video presentation, high-resolution
interrupt behavior, display modes, geometry, calibration, and display
transactions are tightly coupled in the historical evidence while still being
distinct responsibilities.
