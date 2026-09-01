# PS-to-VNC Behavioral Inventory

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    INVENTORY_STRUCTURE=ACTIVE
    DETAILED_BEHAVIOR_AUDIT=IN_PROGRESS
    COMPLETED_TRANCHE=B01_B09_EVIDENCE_SUPPORTED

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
| B05 | GS/video presentation and framebuffer-to-display transfer | EVIDENCE_SUPPORTED | B4A/current presentation source, legacy Test14 interrupt campaign, M3/M4 display qualification |
| B06 | Display modes, display transactions, geometry, and safe-area calibration | EVIDENCE_SUPPORTED | `src/video/`, B4A/current display/calibration source, M3/M4 five-mode hardware evidence |
| B07 | Controller acquisition, pointer semantics, clicks, and logical actions | EVIDENCE_SUPPORTED | B4A/current input source, recovered Test11 foundation, Test13I, F8J2/B4A input-curtain authority |
| B08 | Keyboard, on-screen keyboard, modifiers, and text interaction | EVIDENCE_SUPPORTED | B4A/current OSK/key-event source, recovered Test11 keyboard/OSK foundation |
| B09 | Menus, overlays, curtains, status presentation, and local UI flow | EVIDENCE_SUPPORTED | B4A/current UI source, B06 transaction evidence, F8J2/B4A curtain/input ownership evidence |
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

## B05 — GS/video presentation and framebuffer-to-display transfer

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Turn a coherent logical framebuffer produced by B04 into visible PS2 GS output.
B05 owns PS2-specific display-backend mechanics, VRAM/presentation resources,
draw/finish/flip semantics, and the HIRES scanline/HSync machinery. It does not
own RFB parsing or the user-facing policy that chooses a display mode.

### Presentation backends

The historical implementation supports two distinct backend families:

- **STANDARD** — ordinary gsKit presentation for modes with a proven standard
  rendering contract;
- **HIRES** — gsKit HIRES/scanline-managed presentation for large/interlaced
  modes and the high-resolution mode progression.

The mode catalog associates each mode with an allowed/default backend, but mode
identity and backend identity are separate state. The presentation abstraction
provides common semantic operations such as create/destroy, prepare desktop,
draw, finish draw, flip, and memory-idle synchronization while mapping those
operations to backend-specific mechanisms.

For example, the qualified source maps finish-draw/flip differently for HIRES
and STANDARD rather than pretending both backends have identical GS mechanics.
That abstraction boundary is useful; the historical placement and global state
are not automatically the clean architecture.

### Framebuffer-to-presentation boundary

B04 owns decoded framebuffer validity. B05 consumes coherent GS-ready pixels and
prepares/publishes them through the active backend.

A display reconstruction obtains an authoritative complete framebuffer before
preparing and exposing a newly created GS presentation. Live incremental
updates can then update/publish only the affected presentation regions when the
backend supports it, but presentation-buffer coherency must be established
before normal incremental traffic resumes.

The clean design must keep these concepts distinct:

- RFB pixel/rectangle validity;
- EE framebuffer representation validity;
- GS/VRAM presentation-buffer validity;
- physical timing/scanout progress.

### Test14 HIRES HSync failure

Test14 provides hardware-grounded evidence for a real PS2-specific interrupt
contract in the HIRES HSync callback.

The observed vulnerable interval was:

    current INTC_GS callback active
        -> acknowledge/re-arm current HSINT early
        -> continue ordinary callback work
        -> another HSync reasserts CSR.HSINT
        -> callback has not yet retired
        -> terminal HIRES synchronization failure can occur

The campaign demonstrated that the screen/display path could park while the
controller thread remained responsive. It also demonstrated that the GS/EE
synchronization path could be recovered in place; reboot was not intrinsically
required to repair the subsystem.

### Reconstructed and tested interrupt discipline

The corrective discriminator was not a blind second ACK and was not prevention
of HSync events. The validated discipline is:

1. enter the `INTC_GS` HSync callback;
2. suppress HSync interrupt **delivery** while the callback owns the current
   transaction;
3. acknowledge the current HSINT at the ordinary source-ACK location;
4. perform normal scanline/pass accounting;
5. preserve ordinary semaphore signaling;
6. do not add a blind second HSINT acknowledgement near return;
7. restore HSync delivery only after ordinary callback work is complete;
8. keep restoration immediately adjacent to interrupt retirement;
9. execute the required PS2SDK `ExitHandler()` sequence;
10. return.

Cg6 is the key mechanism discriminator: CSR.HSINT reasserted while HSync
delivery remained masked, and healthy synchronization continued afterward.
Therefore masking changes delivery timing; it does not suppress the physical
HSync event/source assertion.

### Source/destination interrupt state distinction

The audit must preserve four distinct concepts:

- GS CSR interrupt source condition/acknowledgement;
- GS IMR source-delivery permission;
- EE I_STAT destination request state;
- EE I_MASK destination permission.

Test14 rejected simpler explanations involving a forgotten ACK, an occupied
I_STAT request, a disabled I_MASK bit, or CSR.HSINT first becoming stranded only
after `ExitHandler()`.

### `ExitHandler()` scope

PS2SDK/homebrew practice requires `ExitHandler()` immediately before returning
from an EE interrupt handler; the workaround shape includes `sync; ei`.
Test14 preserved and qualified that requirement but did not recover a complete
Sony cycle-by-cycle explanation of the ROM-kernel defect.

This is a B05 platform mechanism, **not** B01 application exit.

### Tested constants versus generalized rule

The exact corrected PS2VNC state used:

    GS_IMR 0x7f00 = HSync delivery masked
    GS_IMR 0x7b00 = HSync delivery enabled

Those constants describe the tested state. A clean/generalized presentation
implementation must preserve unrelated GS interrupt permissions rather than
blindly overwrite the entire mask register if other interrupt users coexist.

### Hardware evidence

- Test14 Cg5 remained healthy for 40 minutes of operator-observed use.
- Test14 Cg6 sealed the source-reassertion-while-masked mechanism and continued
  healthy synchronization for hundreds of frames afterward.
- The production closeout subsequently recorded 127,159 frames over more than
  16 hours with no recurrence of the Test14 720p failure, including more than
  five hours of controlled high-change workload.
- Successor M3/M4 hardware qualification repeatedly exercised the resulting
  presentation mechanisms across the five principal display modes.

### Invariants

- visible-frame failure must not be assumed to mean the entire process or RFB
  session is dead;
- B05 consumes coherent framebuffer state; it does not silently repair protocol
  corruption;
- HIRES callback source acknowledgement and interrupt retirement form one
  delivery transaction, not unrelated bit operations;
- HSync delivery remains suppressed during the active HIRES callback work;
- delivery is restored before `ExitHandler()`/return;
- no blind second HSINT ACK is introduced without new evidence;
- generalized mask handling preserves unrelated interrupt permissions;
- presentation backend state is separate from mode-selection policy.

### Primary authority

- `working/b4a/ps2vnc_gsHires.c` — qualified HSync callback;
- `working/b4a/ps2vnc_calibration_tail.inc` — backend presentation operations;
- `working/b4a/ps2vnc_display_core.inc` — presentation reconstruction boundary;
- legacy `docs/test14/TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md` and
  associated Cg evidence;
- successor M3/M4 display hardware evidence.

### Clean-rebuild implication

Create an explicit presentation responsibility whose public contract is about
creating a backend, preparing/publishing coherent pixels, drawing/finishing,
flipping, and waiting for presentation memory. Keep PS2-specific Standard and
HIRES mechanics behind that boundary. Preserve the Test14 interrupt discipline
as a qualified hardware invariant, while expressing mask changes in a way that
can coexist safely with future GS interrupt users.

## B06 — Display modes, transactions, geometry, and safe-area calibration

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Own the product meaning of display configuration: available physical modes,
backend choice/admission, remote desktop geometry, presented desktop rectangle,
safe-area calibration, persistence, risky-transition confirmation, rollback,
and startup reconciliation.

B06 coordinates B03 RFB resizing and B05 presentation reconstruction but should
not own their internal transport or GS mechanisms.

### Distinct geometry concepts

The mode catalog explicitly establishes that these are separate concepts:

1. **physical timing/raster geometry** — the GS output timing/raster;
2. **GS drawing geometry** — the dimensions used by the selected presentation
   backend;
3. **RFB logical desktop geometry** — what TigerVNC is asked to render/send;
4. **presented desktop rectangle** — x/y/width/height placing the logical
   desktop within the physical raster;
5. **safe-area calibration** — per-mode safe width/height and offsets used to
   derive the guaranteed-visible presentation;
6. **startup mode** — durable mode identity selected for future boots.

Do not collapse these into one width/height pair. Historical 480i is an obvious
example: physical NTSC 480i timing, 704x232 GS FRAME drawing geometry, and a
704x464 RFB desktop are intentionally different.

Likewise, a high-resolution physical output may remain at its native raster
while TigerVNC is reduced to a calibrated guaranteed-visible logical desktop.

### Mode catalog and backend policy

`src/video/mode.c` is the normalized catalog for named modes and captures
physical GS timing, raster dimensions, Standard/HIRES geometry, backend support,
and menu grouping.

The catalog contains more exploratory/survey modes than the five-mode hardware
regression. Catalog presence is therefore not equivalent to hardware
qualification. The repeatedly qualified principal matrix is:

    480i, 480p-hires, 576i, 720p, 1080i

480p Standard is also a historically proven contract used as reference for
480p-hires geometry, but the audit keeps exact qualification claims tied to the
specific evidence set rather than promoting every catalog entry automatically.

### Display switch transaction

A mode switch is not a register write. The qualified reconstruction path:

1. resolves/validates the target mode and presented rectangle before touching
   the current display;
2. serializes RFB ownership and flushes queued controller-originated RFB data;
3. destroys the old presentation and creates the target backend/mode;
4. re-advertises ExtendedDesktopSize capability;
5. derives the target logical RFB desktop from the presented geometry;
6. requests/confirms the TigerVNC desktop resize when needed;
7. publishes the new logical/presented geometry and locks it;
8. requests a complete Raw authoritative framebuffer for that target geometry;
9. receives it under the strict full-frame contract;
10. prepares the desktop/OSK in the new presentation backend;
11. restores normal live RFB encoding;
12. explicitly presents a coherent destination frame before normal interaction
    resumes.

The remote/backdoor transition wrapper additionally takes controller/libpad
ownership, discards stale queued input, shows a transition curtain in source and
destination timing, and inserts settle periods around the hazardous
reconstruction. Those exact UI timings are historical behavior; the durable
invariant is that input and visible exposure do not race an incomplete display
transaction.

### Active-mode selection means calibration

The user-facing Display Settings UI distinguishes active and inactive modes:

- X on the **active** mode opens runtime safe-area calibration;
- X on an inactive, admitted/unlocked mode requests a mode switch;
- a locked inactive mode is a hard no-op for switching;
- the active mode is not lockable from the UI.

The main thread independently verifies that a calibration request still names
the current active mode. A stale calibration request is never reinterpreted as
a switch. After calibration, focus returns to the active row so X again means
`Calibrate <mode> Safe Area`.

The low-level switch owner also treats a same-mode switch request as a no-op;
the intended user-facing path to same-mode work is calibration, not
reinitializing the active timing.

### Safe-area calibration and persistence

Calibration is per mode and stores a safe rectangle/offset concept rather than
blindly replacing physical raster dimensions. The persisted representation uses
safe width/height/x/y values; presentation geometry is reconstructed from those
values relative to the active mode's raster.

Accepted calibration is valid for the current session even if persistence to
the Pi fails. Persistence failure therefore should be visible/reportable but
must not retroactively invalidate the pixels the user just accepted in the live
session.

Startup mode persistence is a separate operation represented by the named
`startup_mode = <mode>` value. A mode that is explicitly kept after a risky
transition becomes durable startup authority only after the Pi-side commit is
acknowledged.

### Confirmation and rollback

A risky local display transition arms a real wall-clock confirmation deadline:

    DISPLAY_CONFIRM_TIMEOUT_SECONDS = 30

The default selection is **Go Back**. The deadline begins before the hazardous
switch completes. While reconstruction is incomplete the confirmation UI may be
hidden, but timeout can already publish rollback.

If the main thread is still trapped inside the risky synchronous reconstruction
when the deadline expires, the controller thread does **not** manipulate GS or
RFB directly. The hard-stall fail-safe returns through the established OSDSYS
process-replacement path, leaving the durable transaction provisional so startup
reconciliation restores the previously confirmed authority.

If the candidate is responsive:

- **Keep** does not reconstruct the already-running candidate; it asks the Pi to
  promote the complete provisional profile to confirmed/durable state;
- commit failure returns ownership to the confirmation UI instead of silently
  accepting an unpersisted candidate;
- **Go Back**, Circle, timeout, or candidate failure reconstructs the complete
  saved known-good profile.

### Complete profile rollback

Rollback restores more than the mode name. The saved known-good authority
includes at least:

- video mode;
- presentation backend;
- RFB logical desktop geometry;
- presented output x/y/width/height;
- safe-area width/height/offset state and bypass policy.

If transport may be dirty, rollback first obtains a fresh synchronized RFB
session. If the old stream is known to be at a clean message boundary, it may be
reused. The previous display is reconstructed, safe-area state is restored as an
independent component, and the complete resulting profile is verified before
rollback is treated as successful.

### Durable transaction state and startup reconciliation

The historical durable display transaction has explicit states:

    NONE
    PROVISIONAL
    COMMITTING
    RESTORING
    RESTORED

The Pi-side state exists so a reboot/process replacement during a hazardous
transition does not leave startup guessing whether candidate B or confirmed A
owns authority.

Startup preflight selects one complete authoritative profile and can advance an
interrupted provisional/committing state into RESTORING. RFB handshake may
temporarily accept the server's actual stale geometry so the system can converge
TigerVNC back to the authoritative profile; temporary transport acceptance does
not change display authority.

RESTORED/acknowledgement is also explicit. A best-effort local rollback is not
silently called globally successful if the management/durable state transition
failed.

### Hardware qualification

The successor normalization campaign repeatedly exercised display transitions
across the principal five-mode matrix. Final M4I-FINAL-HW1 recorded:

- machine result: PASS 5/5;
- physical result: FULL PASS;
- operator result: FULL PASS;
- sequence: 480i, 480p-hires, 576i, 720p, 1080i;
- final persisted startup mode: 480p.

This is direct hardware evidence that the integrated display-mode,
transaction/geometry, presentation, and persistence mechanisms survived that
matrix. It does not imply every exploratory VGA/alternate-refresh catalog entry
was independently qualified.

### Invariants

- mode identity, backend, physical raster, RFB logical geometry, presented
  rectangle, safe area, and startup persistence remain distinguishable state;
- selecting the already-active mode enters calibration rather than reinitializing
  the timing;
- an unavailable/locked target cannot partially mutate the current display;
- a candidate is not durable merely because it is visible;
- confirmation timeout is wall-clock safety state, not tied to render-loop
  progress;
- rollback restores and verifies a complete known-good profile;
- suspect RFB transport is replaced before rollback reconstruction;
- input cannot leak across a hazardous display reconstruction boundary;
- startup never guesses authority from whichever geometry TigerVNC happens to
  report after an interrupted transaction;
- hardware qualification claims remain mode/evidence-specific.

### Primary authority

- `src/video/mode.c` / `src/video/mode.h` — normalized mode catalog/semantics;
- `working/b4a/ps2vnc_display_core.inc` — switch and rollback transaction;
- `working/b4a/ps2vnc_display_tail.inc` — main-thread transaction owner;
- `working/b4a/ps2vnc_ui_core.inc` / `ps2vnc_ui_tail.inc` — mode/calibration UI
  semantics and confirmation deadline handling;
- `working/b4a/ps2vnc_calibration_core.inc` / `ps2vnc_calibration_tail.inc` —
  safe-area persistence, startup authority, geometry, and presentation helpers;
- M3/M4 five-mode hardware evidence, especially M4I-FINAL-HW1.

### Clean-rebuild implication

Model a display profile explicitly rather than spreading its components across
globals. Separate the mode catalog, safe-area calibration, durable startup
choice, and risky-transition state machine from the B05 backend implementation
and B03 RFB transport. The application should coordinate a display transaction
through narrow operations and be able to state, at every point, which complete
profile is authoritative and whether it is provisional, confirmed, or being
restored.

## B07 — Controller and pointer behavior

**Maturity:** `EVIDENCE_SUPPORTED`

Detailed audit: `docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md`.

### Responsibility boundary

B07 owns the physical controller/libpad owner, pointer/click/scroll response,
controller-derived hotkey gestures, explicit libpad handoff, and publication of
logical input actions. It does not own the local-menu state machine, OSK model,
display reconstruction, or RFB socket itself.

### Core behavior

The current authority preserves precise D-pad mouse motion with delayed repeat
and later acceleration, shaped fractional analog motion, Cross/Circle remote
pointer buttons, Triangle+D-pad and L3+analog scrolling, transient keyboard
chords, and generic multi-button hotkey arbitration.

The gesture arbiter prevents growing/shrinking chords from accidentally firing
subset actions and prevents desktop-only gestures begun under a local UI from
becoming desktop shortcuts merely because the foreground changes before
release.

### libpad ownership and stale-state invalidation

`controller_pad_pause_requested` / `controller_pad_pause_ack` implement a real
ownership boundary: acknowledgement is published immediately before the
controller's next libpad access, and no `padGetState()`/`padRead()` occurs while
ownership is held elsewhere.

On return, pre-handoff edge/button, D-pad, analog, scroll, hotkey, and quarantine
state is cleared. Calibration may additionally require physical release before
returning ownership. The B4A hostile-stick remote transition deliberately does
not require neutral analog release; instead it relies on derived-state
invalidation.

The invariant is therefore **no pre-handoff state leaks into the next input
context**, not “every handoff must wait for physical neutral.”

### Hard remote transition

Frozen B4A wraps B3C reconstruction with:

    controller OFF
      -> discard queued controller RFB
      -> transition curtain / settle
      -> display reconstruction
      -> destination curtain / settle
      -> coherent destination frame
      -> controller ON

The main-thread queue discard removes controller actions generated before the
pause acknowledgement without touching the RFB receive stream. This makes the
distinction between physical-input ownership and already-published logical
input explicit.

### Evidence

Recovered legacy commit `007379b6d4daae87251381b790a7641abab8725d`
marks Test11 input/keyboard history as a `PROVEN HISTORICAL FOUNDATION` built
from preserved hardware-validated ELFs and known-good Test11L source. Test13I
commit `eaa5976171132dc619ccd96ecb9366b704cadc53` records the later
Triangle+D-pad wheel addition.

Final mappings remain B4A/current authority; the Test11 document explicitly
warns against reviving obsolete test mappings.

B4A is preserved as a known tested historical DUT, and M0 states that its
byte-identical reproduction inherits existing B4A hardware-validation evidence.
The exact hard-curtain mechanism is source/diff authority; the successor does
not contain a standalone imported B4A operator log, so this audit does not
invent a more granular artifact.

### Clean-rebuild implication

Build a controller owner that emits logical actions through narrow interfaces.
Preserve libpad handoff, derived-state invalidation, gesture arbitration, and
pointer/scroll response, while modeling already-queued logical input separately
from physical controller ownership.

## B08 — Keyboard and on-screen keyboard

**Maturity:** `EVIDENCE_SUPPORTED`

Detailed audit: `docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md`.

### Responsibility boundary

B08 owns RFB keyboard-event semantics and the local OSK model. B07 may request a
logical key action; B09 determines when the OSK owns foreground input; B03 owns
transport/session serialization.

### Key-event and modifier semantics

The qualified implementation sends explicit RFB KeyEvent down/up sequences with
X11 keysyms. Modified taps explicitly bracket the target key with modifier
down/up events through the shared serialized RFB output path.

The OSK provides ABC and FUNC pages plus a permanent utility row:

    ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC

Shift/Ctrl/Alt are deliberately one-shot. After the next real key action all
latched modifiers clear. Printable Shift-layer characters and true extended
modifier combinations remain distinct so the historical TigerVNC
CapsLock/modifier-reconciliation problem is not reintroduced.

### Local ownership behavior

While the OSK is active, D-pad navigates, Cross activates, Triangle toggles
Shift, Square sends Backspace, Start sends Enter, R1 sends Tab, and
Circle/Select closes while clearing modifiers.

Opening the OSK resets it to predictable page/selection/modifier state and
releases any remotely held pointer button first.

Local OSK navigation changes only PS2-local pixels. Historical code wakes a
blocked incremental render loop with a one-pixel pointer jiggle; the durable
requirement is a first-class local UI wake/invalidation path, not that specific
hack.

### Evidence and rebuild implication

Recovered Test11, especially 11G and 11L, provides the historical keyboard/OSK
foundation. Final B4A/current source supplies current mappings and interaction
with later hotkey/UI behavior.

The clean design should separate a small keyboard/RFB action layer from the OSK
state model and from controller polling. Modifier lifetime and failure cleanup
must be explicit.

## B09 — Local UI

**Maturity:** `EVIDENCE_SUPPORTED`

Detailed audit: `docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md`.

### Responsibility boundary and precedence

B09 owns local foreground/input context, menus, overlays, modal confirmation,
transition curtains, restored acknowledgement, selection/focus, quarantine, and
local presentation state.

The current decision-tree precedence is:

1. restored-display acknowledgement;
2. display confirmation / provisional remote transition;
3. System menu;
4. Display Settings;
5. ordinary desktop when OSK is hidden;
6. OSK.

The final architecture need not retain the historical `if/else` structure, but
foreground ownership must remain equally unambiguous.

### Local-input quarantine

Buttons consumed by local UI remain quarantined until **physical release**.
Closing a menu while Circle/X is still held therefore cannot reinterpret that
same held state as a remote desktop click on the next poll.

This ordinary context-transition mechanism complements B07's stronger libpad
handoff/reset boundary.

### System and display UI

The System surface exposes Refresh VNC Session, Display Settings, and Exit to
System Menu, including visible refresh/countdown state. A global System overlay
may temporarily cover an ordinary OSK/Display Settings underlay without
destroying it, but it cannot punch through a provisional display transaction or
calibration/libpad boundary.

Display Settings preserves the active-mode/calibration contract from B06:

- active mode visibly marked CURRENT;
- X active => `CALIBRATE <mode> SAFE AREA`;
- X admitted inactive => switch;
- X locked inactive => hard no-op;
- active mode cannot be locked from the UI;
- inactive switching stays disabled until authoritative lock policy is loaded;
- Circle can return to a preserved System underlay.

### Confirmation, curtain, and restored acknowledgement

Risky local mode changes default to Go Back and expose Keep/Go Back plus the
wall-clock auto-revert countdown owned by B06. Remote machine-owned provisional
transactions use the same foreground input boundary but suppress human
Keep/Go Back control while active.

B4A's transition curtain prevents an incomplete hazardous reconstruction from
being exposed as an interactive desktop. A coherent destination frame is
presented before controller ownership returns.

Human rollback can publish an explicit restored acknowledgement whose durable
RESTORED state remains outstanding until ACK completes. Remote rollback may
ACK automatically; if durable automatic ACK fails, the human acknowledgement is
shown instead of hiding unresolved state.

### Clean-rebuild implication

Create an explicit local UI/foreground model consuming logical actions from B07
and B08. Preserve quarantine, underlay, modal ownership, transition-curtain,
and durable-ack semantics. Give local UI a direct repaint/wake mechanism rather
than coupling correctness to remote framebuffer damage.

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
- physical timing, logical desktop, presentation geometry, safe-area calibration,
  and persisted startup mode are different concepts;
- a risky display change is a complete transaction, not a mode-register write;
- controller correctness is an ownership problem across time, not only a
  button-to-action mapping;
- physical input ownership and already-published logical input are distinct;
- locally consumed held buttons remain quarantined until release;
- local UI rendering must not depend on unrelated remote framebuffer damage;
- hardware-facing conclusions ultimately require physical PS2 qualification.

## Next audit action

Audit B10 and B11 together. Configuration/persistence/bindings and
recovery/management share Pi management endpoints and durable state, but the
audit must preserve the already established policy-versus-mechanism,
local-versus-Pi authority, and explicit-failure-versus-silent-stall distinctions.
