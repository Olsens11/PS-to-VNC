# PS-to-VNC Durable Historical Lessons

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_4
    INPUT_BEHAVIOR_FAMILIES=B01_B14
    MATURITY=EVIDENCE_SUPPORTED
    ROLE=HISTORICAL_LESSONS_LEDGER

This document extracts the historical lessons that must survive the clean
reconstruction even when the old source structure does not.

Evidence classes use `docs/audit/README.md`:

- `HARDWARE_PROVEN`;
- `TEST_SUPPORTED`;
- `SOURCE_SUPPORTED`;
- `PRODUCT_REQUIREMENT`;
- `HISTORICAL_DESIGN`;
- `INFERENCE`;
- `OPEN_QUESTION`.

The intent is to preserve what was learned, including cases where the visible
symptom pointed at the wrong subsystem.

---

## L01 — A frozen screen is a symptom, not a root-cause classification

**Evidence:** `HARDWARE_PROVEN`

Legacy Test14 demonstrated that the HIRES/GS presentation path could stop making
visible progress while the controller/other application activity remained alive.
The failure was ultimately tied to GS HSync interrupt delivery/retirement rather
than to RFB parsing or the whole process dying.

**Durable rule:** classify progress by subsystem—network, RFB stream, framebuffer,
presentation, controller/threading—before choosing recovery or changing code.

**Primary roots:** legacy Test14 interrupt reconstruction/evidence; B05 audit.

---

## L02 — Test9 narrowed the framebuffer/incremental problem but did not prove a
framebuffer root cause

**Evidence:** `TEST_SUPPORTED`

The historical Test9 matrix established:

- 9A incremental network + partial CPU framebuffer + full GS -> FAIL;
- 9B full network + full CPU framebuffer + full GS -> PASS;
- 9C incremental network + partial CPU framebuffer + slowed + full GS -> FAIL;
- 9D was defined as incremental network + full CPU framebuffer + full GS, but the
  preserved audit does not claim a result that the evidence does not establish.

Full GS upload existed in both passing and failing variants, and merely slowing
the incremental path did not cure the failure.

**Durable rule:** a correlated feature in a failing matrix is not automatically
the cause. Preserve uncertainty until a discriminator isolates the layer.

**Primary roots:** legacy `docs/tests/TEST09_FRAMEBUFFER.md`; B02/B04 audit.

---

## L03 — Network receive corruption can masquerade as framebuffer/RFB logic
failure

**Evidence:** `HARDWARE_PROVEN` / `TEST_SUPPORTED`

Test10 captured malformed rectangle bytes already present in receive data under
the stock MTU1500 condition before later framebuffer mutation. A paired MTU1458
build survived the stress and Test10D restored ordinary live incremental
rendering.

**Durable rules:** external-looking protocol corruption can originate below the
parser; the decoder must fail closed; the exact qualified networking dependency
must not be silently replaced by a nominally similar SDK build.

The later PBUF/window-scale settings are part of the exact B4A dependency
authority, but Test10 alone does not prove each later setting was independently
necessary.

**Primary roots:** frozen qualified PS2IP archive/patch; legacy Test10; B02/B04.

---

## L04 — RFB framing must consume complete legal messages, even when the client
ignores their meaning

**Evidence:** `SOURCE_SUPPORTED` with historical incident note

The qualified parser handles FramebufferUpdate, SetColorMapEntries, Bell, and
ServerCutText framing rather than assuming every next server byte begins a
framebuffer update. Historical source commentary records that xterm selection
could provoke ServerCutText and make an earlier parser appear to freeze.

**Durable rule:** exact framing is more important than whether a message is
interesting. Unknown payload length is never guessed.

**Primary roots:** B04 audit and B4A framebuffer decoder.

---

## L05 — A replacement/reconstructed RFB session begins from authoritative full
state

**Evidence:** `TEST_SUPPORTED` / integrated `HARDWARE_PROVEN`

Startup/recovery/display reconstruction negotiate a known pixel/encoding state,
obtain a complete authoritative framebuffer, establish presentation coherency,
and only then return to normal live incremental operation.

**Durable rule:** do not resume incremental updates on top of uncertain local
representation state merely because a TCP connection exists.

**Primary roots:** Test12 recovery; B03/B04/B06; M3/M4 display qualification.

---

## L06 — One EE thread owns VNC socket operations

**Evidence:** `SOURCE_SUPPORTED` with historical test lineage

Test26+ established the rule that after the controller thread starts, only the
main/render thread calls `send()`/`recv()` on the VNC TCP socket. The controller
historically queued complete RFB input messages; `recv_exact()` moved to
nonblocking refill so the main thread could drain them during `EAGAIN`.

**Durable rule:** preserve one PS2IP socket owner. In the clean design, improve
the queue semantics rather than restoring multi-thread socket ownership.

**Clean adaptation:** controller publishes typed input events; application routes;
RFB owns wire serialization.

**Primary roots:** B4A `ps2vnc_runtime_core.inc`; B03/B07/source map.

---

## L07 — Benign local work and hard recovery must have different receive
semantics

**Evidence:** `SOURCE_SUPPORTED` / integrated historical behavior

The final `recv_exact()` permits benign local yield only when the current exact
read has consumed zero bytes and the caller is at a complete server-message
boundary. Refresh/rollback/exit are higher-priority and may abort even after a
partial read; that stream is then suspect and replaced.

**Durable rule:** responsiveness does not require guessing a protocol boundary.
Safe local work can yield at known boundaries; hard recovery may abandon the
stream only by accepting that it must be replaced.

**Primary roots:** B11 audit; B4A runtime receive loop.

---

## L08 — Explicit failures may recover automatically; unexplained silence is a
separate policy decision

**Evidence:** `TEST_SUPPORTED` + `PRODUCT_REQUIREMENT`

Test12 established recovery for explicit carrier/socket/RFB failure, including
proactive physical-link detection when a socket might otherwise remain idle.
During active freeze debugging, unexplained silent stalls deliberately remain
failed until manual intervention rather than being erased by a generic watchdog.

**Durable rule:** reconnect mechanism and reconnect policy are separate. Do not
turn every lack of progress into automatic recovery merely because replacement
works.

**Primary roots:** legacy Test12; B03/B11; current project requirement.

---

## L09 — Manual Refresh is a controlled recovery transaction, not a repeated
button-triggered reconnect

**Evidence:** `SOURCE_SUPPORTED` / integrated authority

Manual Refresh shares the full RFB replacement mechanism but has explicit
admission rules and a three-second post-success cooldown. It may intentionally
interrupt a blocking/partial receive, which makes the old stream unusable.

**Durable rule:** user-triggered recovery needs admission, visible lifecycle, and
anti-spam state, and must never pretend the interrupted stream remained aligned.

**Primary roots:** B11 audit / final B4A source.

---

## L10 — PS2SDK `ExitHandler()` and application exit are completely different
concepts

**Evidence:** `HARDWARE_PROVEN` for interrupt-return mechanism;
`SOURCE_SUPPORTED` for app lifecycle

The Test14 HIRES fix depends on the established PS2 interrupt-handler retirement
shape, including `ExitHandler()` immediately before return. Application exit is
the separate `LoadExecPS2("rom0:OSDSYS", ...)` lifecycle path.

**Durable rule:** never conflate the identically suggestive names when reasoning
about shutdown versus interrupt correctness.

**Primary roots:** Test14; B01/B05.

---

## L11 — HIRES HSync correctness is about interrupt delivery across the whole
callback lifetime

**Evidence:** `HARDWARE_PROVEN`

Test14 showed the vulnerable pattern: acknowledge/re-arm the current HSINT early,
continue ordinary callback work, allow a new HSync source assertion/delivery
before the current handler retires, then encounter terminal synchronization
failure.

The validated correction suppresses HSync delivery while the callback owns the
transaction, performs the ordinary source ACK and work/semaphore behavior,
restores delivery immediately before interrupt retirement, then executes the
required `ExitHandler()` sequence. A blind second ACK was not the solution.

Cg6 demonstrated that CSR.HSINT could reassert while delivery remained masked
without causing the prior failure, separating physical source assertion from
delivery permission.

**Durable rule:** preserve source condition, source-delivery permission, EE
request state, and EE permission as distinct concepts; preserve unrelated mask
bits when generalized.

**Primary roots:** Test14 Cg5/Cg6 and B05.

---

## L12 — Display geometry is several independent coordinate/state concepts

**Evidence:** `SOURCE_SUPPORTED` / integrated `HARDWARE_PROVEN`

Physical timing/raster, GS drawing geometry, RFB logical desktop geometry,
presented output rectangle, safe-area calibration, and persisted startup mode
are not one width/height pair.

Examples such as 480i and the high-resolution modes prove that collapsing these
concepts produces incorrect behavior.

**Durable rule:** pass/validate complete display-profile values and pure geometry
results; do not rebuild a profile by reading unrelated component globals.

**Primary roots:** `src/video/mode.c`, B06, M3/M4 five-mode qualification.

---

## L13 — A display-mode change is a cross-machine transaction, not a GS register
write

**Evidence:** integrated `HARDWARE_PROVEN`

A safe transition includes candidate/profile validation, durable Pi transaction
state, input ownership, transition curtain, presentation reconstruction, RFB
resize/reconciliation, complete framebuffer acquisition, coherent destination
presentation, confirmation/lease policy, and complete-profile commit or
rollback.

The confirmation deadline is real wall-clock safety state. If a hazardous
synchronous reconstruction traps the main thread beyond the deadline, the
controller-side fail-safe may return through the OSDSYS process-replacement path
while durable provisional state ensures startup restores the previous authority.

**Durable rule:** candidate visibility is not confirmation; rollback restores the
whole known-good profile; local success is not equivalent to durable
cross-machine reconciliation.

**Primary roots:** B06 and M3/M4 display transaction qualification.

---

## L14 — Selecting the active display mode means calibration, not same-mode
reinitialization

**Evidence:** `PRODUCT_REQUIREMENT` + `SOURCE_SUPPORTED`

The final user contract is:

- X on active mode -> `Calibrate <mode> Safe Area`;
- X on admitted inactive mode -> switch;
- locked inactive mode -> no switch;
- low-level same-mode switch remains a no-op.

After a switch/calibration flow, focus remains on the active mode's calibration
action.

**Durable rule:** same-mode user intent and hardware reinitialization are
semantically different operations.

**Primary roots:** B06/B09 and current product requirement.

---

## L15 — Controller correctness is ownership across time, not only button maps

**Evidence:** `TEST_SUPPORTED` / source-supported final behavior

Recovered Test11 provides a proven historical pointer/keyboard/OSK foundation;
F8J2/B4A adds the stronger libpad handoff and hard-transition rules.

The critical handoff invariant is that the controller publishes acknowledgement
immediately before its next libpad access and performs no further pad reads while
ownership is held elsewhere. On return, stale button/edge/analog/scroll/hotkey
state is invalidated. Some contexts require physical release; others safely use
state invalidation.

**Durable rule:** no pre-handoff physical/derived state may leak into the new
input context. Physical controller ownership and already-published logical input
are separate concerns.

**Primary roots:** Test11, F8J2/B4A, B07.

---

## L16 — Local UI must quarantine consumed held controls and repaint locally

**Evidence:** `SOURCE_SUPPORTED` / `PRODUCT_REQUIREMENT`

A button consumed by a menu remains quarantined until physical release so closing
an overlay cannot reinterpret the same held button as a remote desktop click.
The historical implementation sometimes used a tiny remote pointer movement to
wake a blocked render loop after local OSK changes.

**Durable rule:** keep physical-release quarantine; reject remote framebuffer
damage as the primary local-UI wake mechanism. Local UI needs its own dirty/wake
path.

**Primary roots:** B08/B09.

---

## L17 — One-shot OSK modifiers are deliberate behavior

**Evidence:** `TEST_SUPPORTED` / `SOURCE_SUPPORTED`

Shift/Ctrl/Alt in the final OSK are one-shot modifiers. Printable Shift-layer
characters and true remote modifier combinations are deliberately distinguished
to avoid historical TigerVNC modifier/CapsLock reconciliation surprises.

**Durable rule:** modifier lifetime is explicit product state, not incidental key
serialization behavior.

**Primary roots:** recovered Test11 and B08.

---

## L18 — Human configuration, accepted live state, and crash-safe transaction
authority are three different layers

**Evidence:** `SOURCE_SUPPORTED` / integrated hardware evidence

The text config is deliberately human-editable and forward compatible while
recognized values are strictly validated. Parsing does not directly mutate live
display state. An accepted calibration may remain valid for the current session
if ordinary persistence fails; this is different from a risky display commit,
where durable Pi acknowledgement is part of safety authority.

**Durable rule:** desired text, validated runtime state, and durable transaction
authority must not collapse into one config object/file merely because they are
all persisted somehow.

**Primary roots:** M4F config normalization, B10/B06.

---

## L19 — Runtime identity is part of the evidence chain, and visible success does
not excuse ambiguous identity

**Evidence:** `HARDWARE_PROVEN`

The first M4I hardware attempt visibly produced a 480p desktop and DBG traffic,
but runtime identity was truncated to an effective 106-byte payload. The run was
correctly declared invalid for qualification. The defect was classified as
source/runtime serialization, not a parser/TestKit problem.

Deterministic bounded byte serialization removed the printf-family dependency,
produced reproducible builds, and was requalified. M4I-FINAL-HW1 then observed one
exact identity packet and sealed a full machine/physical/operator pass.

**Durable rule:** fail closed on ambiguous DUT identity. Never weaken evidence
validation because the product appears to work.

**Primary roots:** M4I identity classification/repair, `src/diagnostics/identity.c`,
M4I-FINAL-HW1.

---

## L20 — Diagnostics should observe state without becoming the state owner

**Evidence:** `SOURCE_SUPPORTED` / `HARDWARE_PROVEN` for M4I facility

M4I separated runtime identity, diagnostic transport/stages, and structured
DBG/PRF/GEOM serialization. Diagnostics can be optional for ordinary startup
while a specific qualification manifest may require exact diagnostic traffic.

**Durable rule:** owners provide snapshots/reports. Diagnostic usefulness does
not justify reaching through every subsystem's writable internals in the clean
design.

**Primary roots:** `src/diagnostics/*`, M4I final hardware authority, B12.

---

## L21 — Service health means endpoint usability, not merely a live process

**Evidence:** `TEST_SUPPORTED` / historical runtime authority

The Pi health supervisor checks the private interface/address, VNC pacing,
listener on 5900, actual X display usability, and desktop children. It restarts
VNC when the service can appear active without the listener/display actually
being usable.

**Durable rule:** product health checks should verify the service contract the
PS2 consumes.

Management remains independent of VNC desktop availability.

**Primary roots:** Test12/Pi runtime installer, B13.

---

## L22 — Historical VNC pacing is explicit state, not invisible folklore

**Evidence:** `SOURCE_SUPPORTED` / integrated historical runtime

The final Pi runtime applies a narrowly scoped traffic-control rule to VNC TCP
5900 traffic toward the PS2 and refuses to replace an unknown foreign root
qdisc.

**Durable rule:** preserve the fact that pacing existed and was part of the
historical runtime, but re-evaluate/requalify the exact rate/settings on the clean
Pi. Do not silently clone or silently omit the tweak.

**Primary roots:** `pi/runtime/vnc-pacing.sh`, B13.

---

## L23 — Apparatus failures and DUT failures are different experimental outcomes

**Evidence:** `TEST_SUPPORTED` / project development policy

The test campaign repeatedly encountered logging, ownership, timing, and identity
apparatus issues. The development system therefore treats source/product, build,
deployment, apparatus, evidence collection, and operator procedure as separate
failure classes.

**Durable rule:** classify failure before replaying a run. Apparatus changes are
experimental variables and must be recorded.

**Primary roots:** TestKit/evidence history, `docs/development/testing.md`, B14.

---

## L24 — A hardware claim is about an exact DUT and exact experiment

**Evidence:** `HARDWARE_PROVEN` / project qualification authority

Current hardware evidence can bind source authority, source/Makefile hash,
whole-ELF SHA, PT_LOAD SHA/bytes, runtime identity, deployment target/readback,
observer hashes, capture limits, workload, and physical operator requirements.

A changed PT_LOAD requires hardware qualification unless an explicit recorded
policy grants a valid qualification transfer. Raw evidence is byte-preserved and
sealed rather than normalized as ordinary source text.

**Durable rule:** reproducibility and provenance are part of product engineering,
not paperwork added after testing.

**Primary roots:** M4I manifest/evidence, TestKit bridge, development testing
policy, B14.

---

## L25 — The exploratory development machine is not the product

**Evidence:** `HISTORICAL_DESIGN` reconciled into `PRODUCT_REQUIREMENT`

The old Pi accumulated build tools, runtime services, caches, game media, Docker,
and development history. The actual product-specific companion runtime is much
smaller: private Ethernet, dedicated VNC desktop, management/persistent state,
health/recovery, and operator controls, built from standard OS mechanisms.

**Durable rule:** reconstruct the companion from a supported ordinary OS through
a tracked installer/dependency ledger. Adopt packages/configurations because the
product needs them, not because the development Pi happened to contain them.

**Primary roots:** preservation census, historical runtime installer, B13,
`docs/PROJECT_INTENT.md`.

---

## L26 — Historical module boundaries are not architectural lessons

**Evidence:** `HISTORICAL_DESIGN`

M2/M3/M4 mechanical extraction successfully preserved behavior and exposed
coupling, but `core`, `tail`, `.inc`, detached-service, and directional-header
boundaries mostly describe how an 18,938-line monolith was separated under
qualification constraints.

**Durable rule:** keep those artifacts as navigation/reference evidence; do not
mistake successful extraction boundaries for the clean design.

**Primary roots:** successor migration history, source-responsibility map,
project intent.

---

## Open questions deliberately not promoted into lessons

The audit does not invent answers where evidence is insufficient. Examples:

- the preserved Test9 record does not establish a final 9D result;
- Test10 directly demonstrates MTU1458 as the paired discriminator but does not
  independently prove every later PS2IP tuning constant was necessary;
- source commentary records the historical ServerCutText incident, but a
  separately sealed hardware artifact for that exact incident has not been
  located;
- Test14 proves the required interrupt discipline but does not supply a complete
  Sony-level cycle-by-cycle explanation of the underlying ROM/kernel behavior;
- the historical 60 Mbit/s VNC pacing value is not promoted into a universal
  clean-platform requirement without re-evaluation.

These boundaries are part of the lesson: preserving uncertainty is preferable to
turning a plausible story into project authority.

---

## Completion

The historical-test mining requested by GitHub Issue #4 is now durable in this
ledger and the detailed B01-B14 audits.

New historical evidence can refine an entry, but the clean architecture no
longer depends on recovering these lessons from conversation memory or from the
18,938-line reference implementation.
