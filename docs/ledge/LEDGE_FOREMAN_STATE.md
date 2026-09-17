# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0015
RECORDED_AT=2026-09-17T06:42:41-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0035
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0002
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the partial A003-P2A Reconstruction handoff, independently reviews the committed wire/control work, resolves the missing clean Pi production endpoint ownership, repairs the separate immutable-work-log governance incompatibility without rewriting history, and issues only the bounded P2A-completion packet. It does not implement P2A product behavior itself.

The live branch was re-read immediately before this state write at `f78cb9868107e5fe6f07e6e20358ffce442437a9`.

A003-P1 remains Foreman-accepted `MET`. A003-P2A remains incomplete. P2B/P2C/P2D and A004 remain unauthorized. Independent Validation and all physical qualification remain separate authorities.

## Current Foreman phase

`A003_P2A_PI_OWNER_COMPLETION_PACKET_ISSUED`

## P2A partial Reconstruction handoff consumed

WORK_ITEM_KEY=`a003-mpeg-generation`
WORKER=`interactive`
WORKER_LOG_COMMIT=`6ccd4339d709b0626a6ea237ba99d4c7fc6d03e7`
SUBSTANTIVE_SOURCE_COMMIT=`b609aec92f8f54d7d57ea81b38936bc45cd7d6d9`
WORKER_RESULT=`FOREMAN_GOAL_RESULT=PARTIAL`

Generated dictionary reconciliation following the substantive source commit:

- `46befa0157f8537c39251bf55b8f151470b583cf` — deterministic reconciliation trigger;
- `bcea2522997a1c00bee5ceafd616d8d0202aa5e1` — generated clean-definition reconciliation.

No product rollback is required. The partial handoff contains coherent bounded P2A source rather than a half-completed lifecycle implementation, and no P2B/P2C/P2D behavior is present.

## Foreman criterion-by-criterion P2A disposition

### P2A-1 `START_CONTROL_IDENTITY` — MET

Accepted from `b609aec...`.

- START is explicit kind `11`, control channel `0`, flags `0`.
- START is not represented as DATA/channel 4.
- classification uses frame identity rather than MPEG payload shape/length.

No correction is assigned for the accepted wire identity.

### P2A-2 `START_SEMANTICS` — PARTIAL

Accepted portion:

- exact 44-byte v1 wire payload remains eleven big-endian 32-bit words in the audited field order;
- codec enforces exact payload size and v1 representation.

Missing portion:

- exact active-session validation;
- nonzero generation;
- signed-int-range conversion for geometry;
- base width/height minimum and 16-pixel alignment;
- positive suppression dimensions;
- suppression containment of exact base rectangle;
- base and suppression bounds against active Pi desktop geometry;
- one immutable prepared generation;
- stale/repeated/conflicting START rejection without mutation.

These are Pi exact-generation/application semantics and do not belong in PS2 Transport or MPEG decoder ownership.

### P2A-3 `RETIRE_WIRE_IDENTITY` — MET

Accepted from `b609aec...` as a **wire/control** criterion only.

- RETIRE is explicit kind `10`, control channel `0`, flags `0`;
- exact payload size is 12 bytes;
- the codec preserves big-endian `(version, session_id, generation)` representation and v1 identity;
- no cleanup, acknowledgment, decoder retirement, or successor-generation lifecycle is claimed.

P2A-completion may add owner-correct RETIRE publication/semantic-request validation, but it must not reinterpret this wire criterion as P2C lifecycle authorization.

### P2A-4 `PURE_MPEG_DATA` — MET

Accepted from `b609aec...`.

The deterministic regression constructs START-shaped bytes in an exactly 44-byte `DATA/channel 4` frame and proves that frame remains MPEG media while START/RETIRE classifiers remain false. No correction is assigned.

### P2A-5 `SOLE_TRANSPORT_OWNER` — PARTIAL

Accepted portion:

- no new PS2 socket, receiver thread, or competing reader was introduced;
- PS2 outbound START uses the existing ordered physical Transport send path;
- PS2 receive ownership remains unchanged.

Missing portion:

- clean production Pi endpoint ownership and sole receive dispatch were previously undesignated;
- PS2 sole receiver still has no narrow inbound RETIRE-control publication seam and currently fails non-DATA frames rather than publishing exact-generation control upward.

Architecture overlay revision 0002 now resolves the Pi ownership ambiguity; the completion worker may implement the named seams but may not add another physical reader on either endpoint.

### P2A-6 `OWNERSHIP` — PARTIAL

Accepted portion:

- Transport wire code contains no prepared-generation or producer lifecycle state;
- MPEG decoder ownership is unchanged;
- Pi semantics were correctly not forced into PS2 `src/transport/runtime.c`.

Missing portion:

- production Pi PSTV Session and Pi Exact-Generation Coordinator behavior are not yet implemented;
- the PS2 application-facing START/RETIRE control seam is not yet complete: current START send is transport-internal and inbound RETIRE publication is absent.

Architecture overlay revision 0002 now gives these responsibilities named clean owners.

### P2A-7 `NO_GENERATION_TAGGING` — MET

Accepted from `b609aec...`. No generation field was added to MPEG DATA and no magic 44-byte media discriminator exists.

### P2A-8 `BOUNDED_SCOPE` — MET

Accepted from `b609aec...`. No Pi producer emission fence, PS2 decoder retirement/join/drain/credit fence, successor-generation reopen, presentation/deadline policy, A004 reconstruction, DESKTOP CALIBRATION change, or hardware claim was introduced.

## Clean production Pi ownership decision

Architecture authority is now `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0002, commit `66a0c95a1b00e7183ce0c8bb57dc6456bfff2224`.

### Pi PSTV Session

The production Pi counterpart of historical H1 `H1Session.reader()` is the **Pi PSTV Session** component in the newly designated clean runtime namespace:

`pi/pstv/`

It owns:

- one accepted PS2-facing PSTV socket/session;
- the Pi endpoint's sole PSTV receive chain and receive sequence;
- framing/version/length and explicit frame-kind/channel/flags dispatch;
- ordered physical sends and send sequence;
- Transport-level publication of explicit control messages;
- mechanism-level connection/session teardown.

It does not own exact-generation business state, MPEG producer lifecycle, presentation/calibration policy, or DESKTOP CALIBRATION.

### Pi Exact-Generation Coordinator

A separate internal owner inside `pi/pstv/` owns:

- active session identity;
- highest accepted generation / stale-generation authority;
- the optional one-and-only immutable prepared START value;
- START semantic decode/validation after envelope classification;
- wrong-session/zero/stale/repeated/conflicting rejection;
- accepted MPEG CALIBRATION / MPEG presentation base and suppression geometry as immutable generation inputs;
- publication of an accepted prepared generation to later lifecycle work.

The Pi PSTV Session publishes a typed control identity (`START` or `RETIRE`) plus exact payload and immutable session context to this coordinator. Transport must not infer generation semantics from MPEG media bytes.

### Active Pi desktop geometry authority

The coordinator receives immutable active desktop bounds at PSTV-session/coordinator creation from the Pi companion's active **PS2-facing desktop/session owner** — the runtime/configuration authority that creates or adopts the active Xtigervnc desktop.

Historical H1 `desktop_width` / `desktop_height` are evidence of this Pi-local session-metadata ownership. `704x462` is only the current qualified/candidate desktop value, not a permanent product constant. START does not define its own bounds authority.

This value is not PS2 DESKTOP CALIBRATION. START's base rectangle and suppression footprint are MPEG CALIBRATION / MPEG presentation geometry consumed here only as already-qualified generation-preparation inputs. P2A does not begin A004 MPEG calibration reconstruction.

### Pi MPEG producer

The Pi MPEG producer remains a later P2B responsibility. P2A completion must not launch it, mutate capture/suppression state, open emission admission, or implement retirement cleanup.

## PS2 control-seam decision

The accepted `b609aec...` START wire/send helper may be preserved. P2A completion must make it available through the normal Transport bridge/session API so application/exact-generation coordination does not reach into `physical_stream` internals.

Because RETIRE completion will later arrive Pi -> PS2 on the same PSTV connection, P2A completion must also add only the narrow Transport-owned inbound control-publication seam required to accept/classify kind-10 control on the existing sole receiver and publish its decoded wire identity upward.

Transport must not decide whether that RETIRE actually retires a generation. No decoder stop/join, residual drain, credit restoration, cleanup acknowledgment, or successor reopen belongs in this packet.

## Separate governance repair — settled

The prior canonical red was not a P2A defect. Two already-frozen otherwise-canonical records wrote `LOG_FORMAT_REVISION=0005`:

- `docs/ledge/work-log/20260916T160445-0400__integration__global-dictionary-prep__dictionary.md`;
- `docs/ledge/work-log/20260916T162043-0400__validation__a003-mpeg-generation__validation.md`.

Neither immutable record was edited.

Work-log contract revision 0006, commit `d1afc0f9c32b7c53c49d9a72c08b256b3c7b3ca1`, adds only two exact path-specific format-revision compatibility entries. They are not broad legacy grandfather exceptions; every other canonical filename/metadata/status/time check still applies.

Checker commit `f78cb9868107e5fe6f07e6e20358ffce442437a9` implements the same exact-path-only rule.

Canonical workflow run `35211799038` on `f78cb986...` is settled green:

- `host-unit=SUCCESS`;
- `ps2-compile=SUCCESS`;
- `ps2-link=SUCCESS`, including current-source reproducibility;
- `dictionary-long=SUCCESS`;
- `project-check=SUCCESS`;
- project-check reports `WORK_LOG_CHECK=PASS records=118 grandfathered=9 format_compat=2` and `PS_TO_VNC_PROJECT_CHECK=PASS`.

This closes the known governance/checker red only. It is not independent Validation PASS and is not hardware evidence.

## Fresh bounded Reconstruction packet — A003-P2A-COMPLETION

WORK_ITEM_KEY=`a003-mpeg-generation`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`f78cb9868107e5fe6f07e6e20358ffce442437a9`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001`
ASSIGNING_ARCHITECTURE_OVERLAY=`LEDGE_ARCHITECTURE_OVERLAY.md:0002`
ASSIGNING_FOREMAN_STATE=`0015`

### Objective

Complete only the missing owner-correct P2A semantics and dispatch around the accepted `b609aec...` wire/control implementation. Establish the clean Pi PSTV Session plus Pi Exact-Generation Coordinator under `pi/pstv/`, finish the narrow PS2 application-facing START/RETIRE Transport seams, and make P2A-2/P2A-5/P2A-6 independently reviewable as `MET` without crossing into producer or decoder-retirement lifecycle.

### Preserve without rework unless a concrete defect is found

Preserve the accepted `b609aec...` behavior:

- START kind 11 / control channel 0 / flags 0;
- exact 44-byte START v1 wire codec;
- RETIRE kind 10 / control channel 0 / flags 0;
- exact 12-byte RETIRE wire codec;
- explicit START/RETIRE/MPEG-DATA classifiers;
- ordinary 44-byte DATA/channel-4 payload remains pure MPEG media;
- no generation tags or payload sniffing;
- one existing PS2 physical stream and sole receive owner.

### Required deliverables

1. **Clean Pi runtime component.** Create the behavior-bearing production Pi PSTV component under `pi/pstv/` as authorized by architecture overlay rev0002. Keep one Pi PSTV Session owner for the one accepted socket/session, framing/sequence, ordered receive/send, and explicit envelope dispatch. Do not copy the H1 experiment into production wholesale.

2. **Typed Pi control publication seam.** The Pi PSTV Session must classify explicit START/RETIRE envelopes and publish a typed control identity plus exact payload/session context to the Pi Exact-Generation Coordinator. It must not sniff DATA/channel-4 payloads and must not own prepared-generation business state.

3. **Pi exact-generation START semantics.** The coordinator must validate START v1 against:
   - exact active session id;
   - nonzero generation;
   - every geometry word fitting the implementation's signed-int range before signed use;
   - base width and height each at least 16 and 16-pixel aligned;
   - positive suppression width/height;
   - suppression rectangle fully containing the exact base rectangle;
   - exact base rectangle inside immutable active Pi desktop bounds;
   - suppression rectangle inside immutable active Pi desktop bounds;
   - overflow-safe right/bottom arithmetic.

4. **Immutable prepared-generation state.** Successful START stores exactly one immutable prepared value containing session id, generation, base rectangle, and suppression rectangle. While one value is prepared, repeated, stale, newer-conflicting, wrong-session, malformed, or invalid START must fail without mutating it. Preserve monotonic stale-generation authority so an accepted generation cannot later be replaced by itself or an older generation. Do not activate a producer.

5. **Active Pi desktop bounds injection.** Supply desktop width/height to the coordinator as immutable session construction/context from the PS2-facing Pi desktop/session owner. Do not hardcode 704x462 as an architectural maximum, derive bounds from START, or involve PS2 DESKTOP CALIBRATION. Tests may inject deterministic bounds.

6. **Pi RETIRE publication only.** Route exact RETIRE control through the Pi sole session owner to the coordinator and validate its wire/session/generation request identity sufficiently to publish a typed valid request. Do not stop/clean a producer, send completion ACK, clear/reopen a generation, or implement the P2B/P2D fence.

7. **PS2 public START seam.** Preserve the accepted physical-stream START send path but expose it through the normal Transport bridge/session boundary so application/exact-generation coordination never reaches into private `physical_stream` ownership.

8. **PS2 inbound RETIRE publication seam.** Extend only the existing sole PS2 Transport receiver/control bridge as needed to accept exact RETIRE control envelope/wire identity and publish it upward through a narrow application-facing seam. Do not perform decoder stop/join, queue drain, credit restoration, generation retirement, or successor reopen.

9. **Deterministic host tests.** Cover at minimum:
   - exact valid START preparing one immutable Pi generation;
   - wrong session, zero generation, signed-range failure, base minimum/alignment failure, suppression-size/containment failure, desktop-bounds failure, and overflow-safe boundary cases;
   - repeated/stale/conflicting START leaves accepted prepared state byte/value-identical;
   - Pi session dispatch recognizes START/RETIRE only from explicit control identity;
   - START-shaped exactly-44-byte DATA/channel-4 remains ordinary MPEG media;
   - Pi RETIRE malformed/version/session/generation mismatch rejection at the semantic request boundary without lifecycle effects;
   - one Pi receive owner / no second socket or reader introduced;
   - PS2 START uses the public Transport seam while retaining the existing ordered physical sender;
   - PS2 RETIRE is published by the sole existing receiver without P2C behavior;
   - accepted P1 MPEG Transport/decoder tests continue to pass.

10. **Source/topology discipline.** Treat `pi/pstv/` as the architecture-authorized new clean Pi runtime component. Add behavior-specific source/tests and clear local documentation/synopses appropriate to the language. Foreman retains canonical topology/file-map/checker/test-registration/generated-portal integration after the worker handoff where those are non-behavioral chores.

### Acceptance criteria

P2A-C1 `START_SEMANTICS_MET` — P2A-2 becomes `MET`: valid START produces the exact immutable prepared generation; every audited invalid/stale/repeated/conflicting case fails without mutation.

P2A-C2 `PI_SOLE_SESSION_OWNER` — one production Pi PSTV Session owns one socket/session/receive sequence and all START/RETIRE receive dispatch; no second reader/socket appears.

P2A-C3 `PI_OWNERSHIP_SPLIT` — Transport/session owns framing/dispatch only; Pi Exact-Generation Coordinator owns semantic validation/prepared state; Pi MPEG producer remains absent/deferred.

P2A-C4 `ACTIVE_DESKTOP_AUTHORITY` — bounds come from injected immutable Pi active desktop/session context, not START, DESKTOP CALIBRATION, or a hardcoded permanent 704x462 limit.

P2A-C5 `PS2_CONTROL_SEAMS` — application-facing START send and inbound RETIRE publication both preserve existing sole Transport physical ownership without adding P2C lifecycle.

P2A-C6 `PRESERVE_ACCEPTED_WIRE_WORK` — P2A-1/P2A-3/P2A-4/P2A-7 remain `MET`, including the 44-byte MPEG-media regression and no generation tagging/sniffing.

P2A-C7 `BOUNDED_SCOPE` — P2A-8 remains `MET`: no Pi producer preparation/emission/retirement fence, PS2 decoder stop/join/drain/credit fence, successor-generation transaction, presentation/common-clock arm, A004 reconstruction, DESKTOP CALIBRATION change, or hardware claim.

P2A-C8 `HANDOFF` — emit exactly one canonical immutable Reconstruction work log with criterion-by-criterion evidence and `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED` for the P2A-completion packet.

### Blocker / fallback rule

The Pi production owner/path is no longer an architectural blocker: overlay rev0002 designates `pi/pstv/`, the Pi PSTV Session, and Pi Exact-Generation Coordinator.

If implementation uncovers a concrete contradiction between that boundary and newer repository authority, stop at the exact contradiction and report `PARTIAL`/`BLOCKED`; do not invent another Pi production owner. If a narrow reusable Transport publication primitive is required on PS2, implement only that mechanism and keep exact-generation policy above Transport.

There is no P2B stretch target. Finishing P2A early means stop with a complete handoff.

## Deferred successor packets — not authorized

- P2B: Pi producer preparation/emission-admission/lease/cleanup fence.
- P2C: PS2 exact retirement completion, decoder stop/join, residual discard and credit restoration.
- P2D: complete ordered `N -> RETIRE(N) -> N+1` transaction.
- A004: presentation/compositor/MPEG CALIBRATION lifecycle beyond the already-accepted geometry contract.

## Next Foreman pickup

Consume only the P2A-completion handoff. Reinspect P2A-1 through P2A-8 on the integrated tree, with special attention to the Pi single-reader boundary, immutable prepared-generation state, active desktop bounds authority, public PS2 control seams, and absence of P2B/P2C behavior. Perform canonical Pi topology/test/file-map integration as Foreman-owned chores only after behavior source is coherent.

PENDING_LOCAL=A003-P2A-completion worker handoff and Foreman integration; independent Validation after coherent P2A acceptance
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification