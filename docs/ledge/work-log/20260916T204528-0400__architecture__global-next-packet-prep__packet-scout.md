DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T20:45:28-04:00
COMPLETED_AT=2026-09-16T20:48:00-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=300444a3b99d9f198f33152233b4b33cbfbace2f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout — A004 first-physical-frame boundary

NON_AUTHORITATIVE_PENDING_FOREMAN

## Authority observed

Live branch `ledge/h1-all-guns` was refreshed at start and re-read immediately before this immutable write; HEAD remained `300444a3b99d9f198f33152233b4b33cbfbace2f` (`docs(diagnostics): record CI sentinel governance gate`).

Current Foreman state revision 0013 has accepted A003-P1 and issued A003-P2 exact-generation START/RETIRE to the interactive worker. Continuity revision 0035 likewise records A003-P2 as Reconstruction-ready with no immutable P2 completion handoff yet. Recent branch movement after the prior scout consists of support/documentation activity including Validation readiness follow-up and Diagnostics governance-gate reporting; no newer Foreman packet supersedes A003-P2.

The canonical repository red remains governance/work-log debt rather than an established A003 product-behavior defect. This scout does not repair that debt and does not reinterpret Validation authority.

## Evidence inspected

- current `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0013;
- current `docs/ledge/LEDGE_WORK_STATE.md` revision 0035;
- current Reconstruction/work-log policy authority as named by those states (revision 0005);
- recent branch commits through live HEAD `300444a3...`;
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001;
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` revision 0001;
- frozen H1 authority named by those documents at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, especially `experiments/media-harness-h1/h1_video_runtime.c`, `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`, and `experiments/media-harness-h1/mpeg_presentation_calibration/`.

## Active packet observed

A003-P2 remains the active authorized behavior packet: exact 44-byte START validation, exact RETIRE kind-10/channel-0 tuple, Pi generation emission-admission/lease/cleanup fence, PS2 admission-close/decoder-join/residual-credit fence, and ordered same-stream N -> N+1 safety. Presentation, common-clock first-frame arm, compositor and MPEG CALIBRATION remain explicit P2 non-goals.

No P2 completion handoff was observed in current state. Therefore every successor statement below is conditional on Foreman acceptance/integration of P2.

## Conditional next coherent responsibility

The narrow next behavior responsibility after a successful A003-P2 is an A004 **first-physical-frame ownership and protected RFB transition** packet, not the entire A004 tranche and not MPEG CALIBRATION editing UI.

Concrete lifecycle:

1. Consume an already accepted/prepared exact generation from A003 without treating START, producer admission, queue prefill, decode start, or upload as visual ownership.
2. Keep RFB presentation protected through the accept/prepare-to-first-frame gap at a complete-message/request-safe boundary while retaining authoritative RFB framebuffer truth.
3. Route the first valid MPEG surface through the single presentation/GS owner.
4. Only after the frame is physically presented, promote MPEG visual ownership for generation N and invoke the shared-media-clock first-real-presentation arm boundary.
5. While N owns presentation, suppress RFB repaint only inside N's accepted MPEG CALIBRATION outer/suppression footprint; do not mutate RFB framebuffer truth.
6. Preserve local overlay as the final layer and one physical GS synchronization/presentation path.
7. On retirement/failure, do not restore RFB merely because decode stops: restoration waits for A003 exact Pi retirement proof plus local MPEG owner/worker retirement, then creates exactly one fresh nonincremental/full RFB request obligation.

This packet creates the clean bridge from A003 generation safety to A004 presentation ownership without prematurely pulling in the full scheduler/drop or MPEG CALIBRATION editing transaction.

## Historical H1 call/process evidence

The A004 audit records the recovered H1 process as one shared compositor: remote desktop -> suppression/matte footprint -> MPEG base presentation -> inner matte -> local overlay -> one GS synchronization/presentation path -> physical flip. The historical `cumulative39` graphics wrapper was evidence for one cumulative owner, not a production requirement to preserve wrapper/source-inclusion machinery.

The recovered ownership edge is physical presentation, not decode readiness. H1/A003 evidence qualifies the first real synchronized MPEG presentation as the common-epoch arm boundary. This means the future clean presentation owner needs an explicit result/event representing successful physical presentation; MPEG decoder availability alone is insufficient authority.

The RFB side is a presentation gate/request-policy consumer: freeze occurs at a safe complete-message/request boundary while the session and authoritative framebuffer remain alive. Restoration creates one full-refresh obligation, and coincident restoration obligations collapse rather than multiplying requests.

## Current clean ownership mapping

- A003 MPEG/decoder remains owner of decode/upload readiness and exact-generation lifecycle but must not self-promote presentation ownership.
- media/common-clock remains owner of the shared epoch state; A004 presentation owns the callsite that arms it on first physical MPEG presentation.
- RFB remains owner of protocol parsing, framebuffer truth and request mechanics; A004/application gives it freeze/full-refresh ownership policy through its bridge.
- the future presentation component should own the single GS composition/presentation path, resolved draw geometry, suppression/matte rendering and physical-present result.
- application should own the cross-domain transition sequence through existing/narrow component bridges; it should not absorb GS, RFB parser, decoder or Transport internals.
- Pi producer/Transport remains exact-generation capture/suppression installation/retirement authority from A003; A004 must consume that proof rather than invent a second generation lifecycle.

## CALIBRATION SEPARATION INVARIANT

DESKTOP_CALIBRATION and MPEG_CALIBRATION remain separate authorities.

DESKTOP_CALIBRATION: older safe-desktop/display-profile/screen-fit geometry for ordinary desktop/display presentation. No evidence inspected this wake authorizes using its geometry store as MPEG generation geometry.

MPEG_CALIBRATION: H1 generation-specific base rectangle, PS2-local inner matte, and outer/suppression footprint plus accept/prepare/first-frame lifecycle. The correct forensic lineage is `experiments/media-harness-h1/mpeg_presentation_calibration/` plus A004 audit authority.

For the proposed first A004 packet, MPEG_CALIBRATION geometry should be consumed as already committed generation facts if available; reconstructing the full MPEG_CALIBRATION foreground edit/review/cancel/accept UI is a natural later A004 packet. Any source proposal that shares a geometry store with DESKTOP_CALIBRATION solely because both manipulate rectangles is `NEEDS_FOREMAN_REVIEW` and should fail closed toward separation.

## Behaviors that must survive

- accept/preparation is distinct from first physical MPEG presentation;
- first physical valid MPEG presentation is both visual-ownership promotion and common-clock arm boundary;
- RFB remains protected through the entire accept-to-first-frame gap;
- suppression affects presentation only; remote framebuffer truth may continue to evolve;
- one GS owner/path composes desktop, suppression/mattes, MPEG and local overlay;
- restoration waits for exact Pi generation retirement plus local MPEG retirement;
- restoration schedules exactly one fresh full RFB request;
- MPEG_CALIBRATION base, inner matte and outer/suppression footprint retain distinct meanings;
- known cursor-sluggishness observation is not silently changed as part of structural reconstruction.

## Experiment scaffolding that should not survive

- cumulative wrapper/source-inclusion/symbol-renaming mechanics used to prove shared graphics ownership;
- scheduler comparison modes and stage-color/hold machinery;
- duplicate geometry CONFIG knobs when accepted MPEG_CALIBRATION generation geometry is authoritative;
- destination-specific adapter proliferation;
- any second GS presenter or second framebuffer truth.

## Dependency/order hazards

1. Promoting ownership or arming the clock on START/decode/upload would expose the accept-to-first-frame gap and violate A003/A004 clock authority.
2. Allowing RFB repaint inside the MPEG-owned footprint while generation N is visible creates competing presentation owners even if framebuffer protocol state is correct.
3. Restoring RFB before exact Pi retirement can repaint while generation-scoped capture/suppression remains active.
4. Joining a second graphics implementation instead of extending one presentation owner recreates the historical structural failure.
5. Treating MPEG_CALIBRATION outer footprint as the base rectangle would incorrectly change Pi capture geometry; treating inner matte as Pi state would also cross ownership.
6. Pulling full scheduler/drop into the first ownership packet risks obscuring the more fundamental first-frame/ownership proof; scheduler/drop is a natural follow-on or stretch only after the ownership seam is deterministic.

## Behavior-specific evidence likely required

Host-testable/injected:

- START/prepared/decode/upload do not promote ownership or arm the clock;
- exactly one first-physical-present success promotes N and arms the common epoch;
- failed/non-presented surfaces do not promote;
- RFB freeze/full-refresh policy obeys complete-message/request-safe boundaries;
- suppression rectangle equals the exact generation's MPEG_CALIBRATION outer footprint while base and inner remain semantically distinct;
- authoritative RFB framebuffer may update while its owned footprint is presentation-suppressed;
- duplicate restoration causes one full-refresh obligation;
- retirement/failure cannot restore before both exact Pi retirement proof and local MPEG retirement;
- composition plan/order has one presentation owner and local overlay last;
- DESKTOP_CALIBRATION state is neither read nor mutated as MPEG generation authority.

PS2-build/hardware-only or hardware-essential:

- actual GS physical-present boundary and one-owner linkage;
- visual base/inner/outer matte correctness;
- no RFB repaint through the MPEG-owned footprint on real hardware;
- local overlay layering;
- first-frame A/V common-epoch behavior;
- repeated accept/present/retire generations and endurance.

## Natural packet split and stretch

Recommended A004-P1: first-physical-frame promotion, protected RFB transition/restoration, generation-specific suppression consumption, one presentation/GS owner seam, and common-clock arm callsite.

Recommended later A004-P2: MPEG_CALIBRATION foreground edit/review/cancel/accept transaction, held-X release/re-press protection, release quarantine, geometry bounds/alignment and accepted-edge propagation. Keep DESKTOP_CALIBRATION separate.

Recommended later A004-P3 or stretch after P1: qualified absolute common-clock scheduler and narrow late-frame/drop policy. Do not preserve comparison modes or stage-marker machinery.

A meaningful P1 stretch, only after ownership criteria are complete, is deterministic render-plan/composition-order proof through the presentation seam; do not stretch into full MPEG_CALIBRATION UI or scheduler tuning.

## Open Foreman decisions

1. Exact clean component/file name for the single presentation owner is not yet authoritative; choose from current architecture/dictionary constraints rather than copying `h1_video_runtime.c` monolithically.
2. Decide whether the first A004 packet consumes an already reconstructed immutable MPEG_CALIBRATION generation-geometry value or must include the smallest geometry value type/bridge necessary to express base/inner/outer distinctly. Do not source it from DESKTOP_CALIBRATION.
3. Decide the narrow RFB bridge representation for freeze/full-refresh ownership policy so application/presentation does not learn parser internals.
4. Decide whether absolute scheduler/drop belongs in a separate packet (recommended) or as bounded stretch after first-frame ownership proof.
5. Any proposed shared state between DESKTOP_CALIBRATION and MPEG_CALIBRATION is `NEEDS_FOREMAN_REVIEW`; no such relationship was established by the authority inspected here.

## Exact Foreman consumption point

Consume this dossier only after an immutable A003-P2 Reconstruction handoff exists and the Foreman has independently reviewed P2-1 through P2-8 against current authority. If P2 is accepted/integrated, use this archaeology immediately before issuing the first A004 worker-targeted packet. If P2 changes generation readiness/retirement seams, re-map the A004 bridge assumptions rather than forcing this conditional plan.

PENDING_LOCAL=A003-P2 worker handoff and Foreman acceptance/integration; current repository work-log governance reconciliation; future A004 canonical integration/evidence
HARDWARE_PENDING=A001 physical PS2; A002 physical audio/common-clock; A003 physical MPEG/video; future A004 presentation/visual/timing qualification
