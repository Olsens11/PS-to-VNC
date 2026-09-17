DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T22:13:49-04:00
COMPLETED_AT=2026-09-17T06:07:48-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=69555c5311b641958acd91538502f16f5140576a
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# A003 MPEG Generation — P2A exact-generation control plane

## Objective and authority

This shift consumed the governing A003 P2A packet from `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0014, the A003 semantic audit, the reconstruction/work-log contracts, current clean Transport authority, and the frozen H1 START/RETIRE evidence. The packet authorized only exact-generation control-plane representation and prepared-generation semantics around the accepted P1 MPEG path.

Endpoint ownership was resolved against the historical evidence before implementation: PS2 `src/transport/runtime.c` remains the sole Pi-to-PS2 receiver and is not the owner of PS2-to-Pi START receive semantics. Historical Pi authority places PS2-to-Pi frame reception in `H1Session.reader()` and prepared-generation validation/state on the Pi side. The current clean tree retains that Pi implementation only under `experiments/media-harness-h1`; the integration inventory says its equivalent becomes production when Pi product architecture is adopted. No designated clean Pi production PSTV/session/control owner exists at this authority. Therefore this shift did not invent a production Pi path and did not move Pi semantics into the PS2 runtime.

## Work performed

Substantive implementation commit:

- `b609aec92f8f54d7d57ea81b38936bc45cd7d6d9` — `reconstruct(a003): add exact MPEG generation wire control`

Changed source/test files:

- `src/transport/protocol.h`
- `src/transport/protocol.c`
- `src/transport/physical_stream.h`
- `tests/unit/transport_protocol_test.c`

The implementation adds the explicit shared control identities and codecs required by the unblocked portion of P2A:

- START frame kind `11`, control channel `0`, flags `0`;
- exact 44-byte START v1 codec as eleven big-endian 32-bit words in order: `version`, `session_id`, `generation`, base/draw `x/y/width/height`, suppression `x/y/width/height`;
- RETIRE frame kind `10`, control channel `0`, flags `0`;
- exact 12-byte big-endian RETIRE codec for `(version, session_id, generation)`;
- explicit header classifiers separating MPEG DATA, START, and RETIRE without inspecting media payload contents;
- a PS2 outbound-only START seam through the existing `pstvnc_transport_physical_stream_send_frame()` path, preserving the existing physical stream, send lock, and ordered sequence space;
- deterministic host coverage proving a legitimate 44-byte `DATA/channel 4` frame is MPEG media and is not START.

`src/transport/runtime.c` was intentionally untouched. No START receive semantics or Pi prepared-generation state were placed in the PS2 sole receiver.

## Generated dictionary reconciliation

The first canonical run correctly exposed stale generated source dictionaries after the new protocol symbols were added. The repository's own deterministic reconciliation path was used rather than hand-editing generated inventories:

- `46befa0157f8537c39251bf55b8f151470b583cf` — `tooling(symbols): run deterministic dictionary reconciliation`
- `bcea2522997a1c00bee5ceafd616d8d0202aa5e1` — automation-produced `docs(symbols): reconcile current clean definitions`

The reconciliation job ran the repository generator, regenerated the source-dictionary portal, ran `python3 scripts/source-dictionary.py check --long --require-complete --strict`, and ran `git diff --check` successfully before committing the generated result.

## P2A acceptance result

### P2A-1 `START_CONTROL_IDENTITY` — MET

Criterion: START is an explicit named control frame (kind 11/channel 0/flags 0), not DATA/channel 4 and not payload-sniffed.

Evidence: the shared protocol namespace defines START as kind 11; the outbound START seam always uses control channel 0 and flags 0; START classification uses explicit header identity only. No DATA/channel-4 START representation was introduced.

### P2A-2 `START_SEMANTICS` — PARTIAL

Criterion: the proven 44-byte v1 fields and exact session/generation/MPEG-presentation-geometry validation are preserved, with one immutable prepared generation and stale/repeat rejection.

Evidence completed: the exact 44-byte, eleven-big-endian-word START wire fields/order and v1 version validation are reconstructed in shared protocol code.

Blocked remainder: full START semantic validation, exact active-session/nonzero-generation rules, signed-range conversion, 16-pixel base alignment/minimums, positive suppression sizing, containment, active-desktop bounds checks, stale/repeated/conflicting rejection, and retention of exactly one immutable prepared generation are Pi-side exact-generation/application responsibilities. The current clean tree has no designated production Pi PSTV receive/control owner in which to reconstruct that state. Implementing those semantics under PS2 `src/transport/runtime.c` or inventing a new Pi production location would violate endpoint ownership and the packet's architectural boundary.

### P2A-3 `RETIRE_WIRE_IDENTITY` — MET

Criterion: RETIRE remains kind 10/channel 0/flags 0 with exact 12-byte `>3I` identity validation; no cleanup/ack lifecycle is claimed yet.

Evidence: shared protocol code implements kind 10, exact control envelope identity, exact 12-byte big-endian `(version, session_id, generation)` encode/decode, exact payload-length classification, and version validation. No RETIRE lifecycle was implemented.

### P2A-4 `PURE_MPEG_DATA` — MET

Criterion: ordinary MPEG remains DATA/channel 4 opaque media, and a legitimate 44-byte MPEG payload is demonstrably not START.

Evidence: deterministic protocol tests construct a valid 44-byte DATA/channel-4 MPEG frame and prove it is classified as MPEG media while START classification remains false. START recognition depends on header identity, never payload bytes or the magic length 44.

### P2A-5 `SOLE_TRANSPORT_OWNER` — PARTIAL

Criterion: one physical PSTV connection and one physical receive owner remain authoritative; START/RETIRE dispatch adds no competing reader/socket.

Evidence completed: no new socket, receiver thread, or competing reader was introduced. The PS2 outbound START seam reuses the existing physical PSTV stream and serialized send path. PS2 receive ownership remains unchanged.

Blocked remainder: the current clean source does not designate a production Pi endpoint PSTV/session/control owner, so production Pi-side START/RETIRE receive dispatch could not be added or proven without inventing architecture. Historical `H1Session.reader()` remains experiment/harness authority only.

### P2A-6 `OWNERSHIP` — PARTIAL

Criterion: Transport owns envelope/dispatch only; exact-generation coordination owns START/RETIRE semantic validation/prepared state; MPEG decoder ownership remains unchanged.

Evidence completed: shared Transport code owns only wire identity/codecs and ordered outbound framing; no generation business state was added to Transport or MPEG. MPEG decoder ownership is unchanged, and PS2 runtime was not made the receiver of PS2-to-Pi control traffic.

Blocked remainder: a clean production Pi exact-generation/application coordination owner has not yet been designated, so its receive-dispatch and prepared-generation state cannot be reconstructed at an owner-correct production path.

### P2A-7 `NO_GENERATION_TAGGING` — MET

Criterion: no per-MPEG-DATA generation tag, magic 44-byte discriminator, or equivalent media sniffing appears.

Evidence: no MPEG DATA payload format was changed, no generation field was added to MPEG packets, and the 44-byte regression test proves explicit envelope identity controls classification.

### P2A-8 `BOUNDED_SCOPE` — MET

Criterion: no Pi emission fence, PS2 retirement/join/drain/credit fence, successor transaction, presentation/deadline policy, A004 reconstruction, or hardware claim is introduced.

Evidence: the shift changes only shared wire representation/classification, the PS2 outbound START seam, behavior-specific protocol tests, and generated symbol dictionaries. It does not activate a Pi MPEG producer, implement producer retirement or RETIRE completion, add PS2 decoder join/drain/credit fencing, reopen a successor generation, claim complete `N -> RETIRE(N) -> N+1`, change presentation/common-clock policy, reconstruct A004/DESKTOP CALIBRATION, or claim hardware qualification.

FOREMAN_GOAL_RESULT=PARTIAL

## Checks and evidence

For substantive commit `b609aec92f8f54d7d57ea81b38936bc45cd7d6d9`, GitHub Actions run `35203230222` produced:

- canonical host unit tests: PASS;
- clean PS2 compile with pinned toolchain: PASS;
- current-source PS2 linked build and reproducibility check: PASS;
- strict dictionary and project-check jobs: FAIL only because the newly added source symbols had not yet been reconciled into generated dictionaries.

The follow-up deterministic reconciliation workflow on trigger commit `46befa0157f8537c39251bf55b8f151470b583cf` completed its dictionary-reconcile job successfully and produced `bcea2522997a1c00bee5ceafd616d8d0202aa5e1`. That job's pre-commit sequence included the complete strict source-dictionary audit and `git diff --check`, both successful.

The immutable log commit necessarily occurs after the evidence summarized above. Any branch CI triggered by this log write is subsequent verification evidence and must not cause this immutable record to be rewritten.

PENDING_LOCAL=Foreman must designate the clean production Pi PSTV/session/control receive owner and exact-generation/application coordination path before the blocked Pi-side P2A semantics can be reconstructed owner-correctly.
HARDWARE_PENDING=A003 physical MPEG/video qualification remains outside P2A; no hardware qualification is claimed.

## Blocker / packet-definition gap

The exact unresolved architecture question is: which clean production Pi-side component owns the one PSTV session/reader and narrow exact-generation control dispatch that replaces historical `experiments/media-harness-h1` `H1Session.reader()`? Until governing authority designates that production owner/path, START semantic validation and immutable prepared-generation state cannot be placed without guessing architecture.

This is a packet-definition/clean-owner gap, not evidence that START belongs in the PS2 receive runtime.

## Next pickup

Foreman should consume this P2A handoff as PARTIAL, preserve the accepted wire/control work, and resolve/designate the clean Pi production receive/control owner before authorizing completion of the blocked P2A-2/P2A-5/P2A-6 Pi-side work. Do not advance into P2B, P2C, P2D, presentation/common-clock policy, A004, or hardware qualification merely to bypass this ownership gap.
