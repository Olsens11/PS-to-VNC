DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T19:47:12-04:00
COMPLETED_AT=2026-09-16T19:51:10-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c1a07a5144018041167fc52665c7b58e769e400a
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout — A003-P2 successor preflight

NON_AUTHORITATIVE_PENDING_FOREMAN

## Authority observed

Live branch `ledge/h1-all-guns` began and was re-read immediately before this immutable write at `c1a07a5144018041167fc52665c7b58e769e400a`.

Current authority consumed:

- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0005;
- `docs/ledge/work-log/README.md` revision 0005;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0013;
- `docs/ledge/LEDGE_WORK_STATE.md` revision 0035;
- current Validation/Diagnostics disposition as synthesized by Continuity revision 0035;
- semantic audit state/current A003 authority through Foreman 0013;
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001;
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` revision 0001;
- frozen H1 authority commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, including `experiments/media-harness-h1/mpeg_presentation_calibration/README.md` and the A004 audit's indexed H1 presentation evidence.

Active packet observed: A003-P2 exact-generation START/RETIRE, targeted by Foreman revision 0013 to the interactive Reconstruction worker. Continuity revision 0035 reports no immutable P2 completion handoff yet. Therefore everything below is conditional successor archaeology only.

## Conditional exact next coherent responsibility

If the Foreman accepts/integrates A003-P2, the next dependency-ordered behavior is an A004 **MPEG first-presentation ownership / protected-transition packet**, not general calibration reconstruction and not DESKTOP CALIBRATION.

Natural packet boundary:

1. consume already-accepted exact-generation MPEG geometry/state from the A003/Pi lifecycle without inventing a second generation authority;
2. keep RFB presentation protected through generation preparation/decode/upload;
3. promote MPEG visual ownership only on the first valid physically presented MPEG frame;
4. arm the shared media epoch at that same first-real-presentation boundary;
5. enforce the exact generation's MPEG CALIBRATION outer/suppression footprint in presentation while retaining RFB framebuffer truth;
6. preserve one physical GS presentation/composition owner/path;
7. on MPEG retirement/failure, do not restore RFB presentation until exact Pi retirement plus local MPEG-owner/worker retirement are proven, then create exactly one fresh full-RFB request obligation.

This is the smallest coherent successor because A003-P2 establishes generation lifetime/fencing, while A004 owns the first physical presentation boundary and the resulting visual-ownership transition. Pulling MPEG CALIBRATION editing UI into this same packet would enlarge the packet across a separate foreground/edit-state responsibility; pulling only clock arm without visual ownership would split one historically coupled first-physical-frame fact into two authorities.

## Concrete H1/A004 call-chain semantics

Frozen H1/A004 evidence establishes this process chain:

`MPEG CALIBRATION accept -> immutable generation geometry -> exact generation preparation / Pi capture+suppression installation -> MPEG emission/decode/upload readiness -> physical synchronized MPEG presentation -> first valid physically presented frame -> MPEG visual-owner promotion + common media epoch arm -> generation-N suppression/composition ownership`.

Critically, none of CONFIG, MPEG CALIBRATION acceptance, START send, producer admission, queue prefill, decoder start, or upload is the ownership/clock boundary.

For RFB, the corresponding chain is:

`presentation owner requires protection -> finish current complete RFB message/request boundary -> stop new framebuffer-update requests while frozen -> retain authoritative desktop image -> local/presentation composition may overlay it -> first MPEG physical presentation changes owned footprint rather than framebuffer truth -> exact retirement proof + local MPEG retirement -> restore ordinary RFB presentation -> schedule exactly one nonincremental/full update obligation`.

Frozen `experiments/media-harness-h1/mpeg_presentation_calibration/README.md` independently confirms that the MPEG CALIBRATION core owns state/geometry only, not pad polling, ordinary mouse/OSK routing, RFB parsing, MPEG decode, permanent composition, or GS primitives. It defines the RFB suppression rectangle as MPEG base expanded by the MPEG CALIBRATION outer matte and clipped to the physical canvas, while keeping permanent composition separate.

## Calibration terminology / ownership findings

**MPEG_CALIBRATION:** A004 base rectangle = exact Pi capture rectangle plus PS2 MPEG presentation region for the generation.

**MPEG_CALIBRATION:** inner matte = PS2-local presentation-only inset state. It must not be sent to Pi or collapsed into capture geometry.

**MPEG_CALIBRATION:** outer matte / suppression footprint = generation-specific MPEG visual-ownership perimeter used to exclude RFB presentation. It does not change Pi base capture geometry.

**MPEG_CALIBRATION:** acceptance commits immutable generation geometry but does not itself claim MPEG visual ownership.

**DESKTOP_CALIBRATION:** older safe-desktop/display-profile/screen-fit geometry remains separate authority. No evidence inspected in this wake authorizes using B4A `ps2vnc_calibration_*` / screen-fit state as MPEG base/inner/outer storage or as the MPEG ownership lifecycle.

No mixed authority was found in the governing A004 invariant. Any future implementation proposal that reuses a generic desktop screen-fit rectangle as MPEG generation geometry is `NEEDS_FOREMAN_REVIEW` rather than a simplification this scout can approve.

## Current clean owners / expected receiving boundaries

Conditional mapping after A003-P2:

- MPEG component: decode/upload readiness only; must not self-promote merely because a decoded surface exists.
- presentation component: one GS composition path, physical presentation event, resolved MPEG draw geometry, matte/suppression rendering, presentation ownership state, absolute deadline/drop policy when later included, and common-clock arm callsite.
- RFB component/bridge: protocol parsing and authoritative framebuffer truth; receives freeze/full-refresh scheduling policy without learning MPEG internals.
- application: cross-domain transition sequence through existing component bridges; should not duplicate presentation or generation state.
- Pi producer/Transport generation owner: exact generation capture/suppression installation and retirement proof from A003-P2; A004 must consume that proof rather than create a second producer lifecycle.
- MPEG CALIBRATION component: edit/commit state and accepted base/inner/outer geometry; permanent compositor remains separate.
- DESKTOP CALIBRATION: remains unrelated ordinary desktop/display geometry authority unless later explicit evidence proves a narrow bridge.

## Behaviors to preserve

- first valid physically presented MPEG frame is both MPEG ownership promotion and common-clock arm boundary;
- RFB remains protected during accept/prepare/decode/upload gap;
- authoritative RFB pixels may continue changing while presentation excludes the MPEG-owned footprint;
- one GS presentation owner/path composes desktop -> suppression/matte footprint -> MPEG base -> inner matte -> local overlay -> sync/flip;
- retirement restoration is ordered after exact Pi suppression/capture retirement and local MPEG owner/worker retirement;
- restoration creates one collapsed full-refresh obligation, not repeated refresh spam;
- A003 generation/Transport accounting and EOF/cancel/retire distinctions survive unchanged.

## Experiment scaffolding not to preserve

- H1 scheduler comparison modes;
- stage colors, stage-marker holds, broad timing counters/witness surfaces as correctness mechanisms;
- independent H1 CONFIG draw/encode rectangle knobs once accepted generation geometry exists;
- source-inclusion/symbol-renaming compatibility wrappers used to combine historical graphics implementations;
- duplicate destination-specific adapters/bridges where one process-organized clean bridge can express the final ownership transition.

## Known defects / cautions not to silently change

- accept-to-first-frame must not be collapsed; premature promotion can expose stale RFB/MPEG overlap and arm the shared clock early;
- RFB must not restore before exact Pi retirement proof;
- duplicate graphics owners are structurally invalid;
- historical cursor sluggishness is a performance observation, not an A004 correctness defect to repair opportunistically;
- held-X review/accept and release-quarantine behavior belongs to MPEG CALIBRATION foreground work and must survive when that later packet is reconstructed, but need not be pulled into this first-presentation packet.

## Dependency/order hazards

1. A003-P2 generation identity must be stable before A004 can bind ownership/suppression to exact N.
2. Physical-present success must be observable at the single presentation owner; decode/upload completion is insufficient.
3. Clock arm and ownership promotion should consume the same first-physical-frame fact, or split authorities can disagree about when N became real.
4. RFB freeze must occur only at a complete-message/request scheduling boundary; protocol/session liveness must not be confused with presentation freeze.
5. Suppression must affect drawing, not corrupt/drop authoritative RFB framebuffer truth.
6. Retirement restoration must consume A003-P2 exact retirement proof and local worker retirement, not merely a timeout or generation-state flag.
7. Do not introduce a second GS presenter while reconstructing MPEG draw ownership.

## Behavior-specific evidence likely required

Host-testable/injected seams:

- START/accept/prepared state does not promote owner or arm clock;
- decode-ready/upload-ready does not promote owner or arm clock;
- first valid physical-present callback/event promotes exactly once and arms epoch exactly once;
- stale/wrong-generation physical-present event cannot promote current ownership;
- RFB suppression rect uses MPEG CALIBRATION base + outer footprint, clipped correctly, and inner matte remains local-only;
- authoritative RFB framebuffer update may proceed while suppressed presentation excludes owned pixels;
- safe-boundary RFB freeze and one-shot full-refresh collapse;
- retirement/failure does not restore RFB before both Pi exact-retirement proof and local MPEG retirement;
- repeated/coincident restoration causes one full refresh;
- render-plan/layer-order fixture proves one presentation owner and expected ordering;
- static/link/dependency checks prove no second graphics owner and no DESKTOP_CALIBRATION -> MPEG_CALIBRATION authority substitution.

PS2-build/hardware-only or hardware-essential:

- canonical PS2 compile/link/PT_LOAD identity for hardware-facing presentation code;
- physical first-frame event really corresponds to displayed GS output rather than queued/uploaded work;
- visible base/inner/outer matte correctness and clipping;
- no RFB repaint inside MPEG-owned footprint while MPEG is physically presented;
- local overlay final-layer behavior;
- common-clock A/V timing at first presentation;
- repeated generation retirement/restoration and endurance.

## Natural split and stretch

Recommended first A004 packet: **first-physical-frame ownership transition + RFB protected transition/restoration + one-GS-owner composition seam**, consuming already-committed MPEG CALIBRATION generation geometry but not reconstructing the full edit UI.

Meaningful stretch only after those invariants are complete: add deterministic absolute common-clock video deadline classification and narrow late-frame drop policy, still without MPEG CALIBRATION editing UI or DESKTOP CALIBRATION changes.

Likely later A004 packet: MPEG CALIBRATION foreground/edit/review/cancel/accept behavior, including held-X release/re-press, release quarantine, base/inner/outer editing/alignment/bounds and exact accepted-edge semantics. This packet must explicitly carry the calibration-separation invariant.

## Open Foreman questions

1. Whether the first A004 packet should consume an already-existing clean MPEG CALIBRATION accepted-geometry value type, or first reconstruct the smallest neutral committed-geometry type needed by presentation. Do not reuse DESKTOP CALIBRATION storage by convenience.
2. Which current clean presentation/graphics file should own the physical-present success event after A003-P2 source lands; current authority has not yet reconstructed A004 product source, so the Foreman must inspect live clean graphics ownership before naming a file.
3. Whether absolute scheduler/drop policy belongs in the initial packet or its stretch. Archaeology favors stretch because first-frame ownership/restoration is already a coherent substantial tranche.
4. Exact bridge seam by which application tells RFB to freeze/restore without teaching RFB MPEG internals; preserve one process-organized bridge and complete-message boundary semantics.

## Exact Foreman consumption point

Consume this dossier only after an immutable A003-P2 worker handoff exists and the Foreman independently accepts/integrates P2-1 through P2-8. Re-read live A003/P2 source and A004 audit before packet issuance. If P2 changes generation/retirement seams, adapt this ownership packet to those actual public seams rather than preserving this provisional mapping by force.

PENDING_LOCAL=A003-P2 completion/Foreman acceptance; exact current clean presentation file ownership after P2; canonical machine evidence for any future A004 implementation
HARDWARE_PENDING=A001 physical PS2; A002 physical audio/common-clock; A003 physical MPEG/video; future A004 physical presentation/calibration qualification
