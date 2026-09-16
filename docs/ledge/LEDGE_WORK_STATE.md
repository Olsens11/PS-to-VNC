# Ledge Reconstruction — Current Work State

DOCUMENT=LEDGE_WORK_STATE
STATE_REVISION=0035
RECORDED_AT=2026-09-16T19:33:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0034
SUPERSEDES_STATE_REVISION=0034
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

## Authority synthesized

- Branch `ledge/h1-all-guns`; pre-write authority re-read as `6d4603c2a92d1676f6915a178b78d06ed12a2e67`.
- Governing Reconstruction contract revision 0005 and immutable work-log contract revision 0005 remain current.
- Audit state revision 0007 remains seeded-audit authority; A003 audit revision 0001 remains current active reconstruction authority. A004 audit revision 0001 plus `LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` govern future calibration packetization.
- Reconstruction state revision 0007 is historical A001-era state and remains stale relative to current A003 work.
- Foreman state revision 0013 consumed A003-P1, independently accepted M1-M8 as `MET`, recorded canonical P1 integration evidence, and issued the fresh bounded A003-P2 exact-generation START/RETIRE packet targeted to `interactive` with objective P2-1 through P2-8 criteria.
- Validation's newest A003 record independently found no P1 product-source defect and product-specific machine evidence PASS, but leaves repository readiness `PARTIAL` because canonical project-check is red only on two immutable support logs carrying unsupported `LOG_FORMAT_REVISION=0005`.
- Diagnostics confirms host-unit, PS2 compile, PS2 link/reproducibility, and dictionary-long green; the remaining red is classified as governance/work-log documentation debt, not an A003 product defect.

Unknown external/Pi-local dirty work remains outside connector visibility and is neither overwritten nor declared absent.

## Current phase and pipeline

`A001_MACHINE_SOURCE_PASS_HARDWARE_PENDING_A002_MACHINE_SOURCE_PASS_HARDWARE_PENDING_A003_P1_VALIDATION_PARTIAL_A003_P2_RECONSTRUCTION_READY`

- `AUDIT`: no active mutation; A003 audit revision 0001 supplies current dispositions.
- `RECONSTRUCTION_READY`: A003-P2 is the current bounded behavior packet. A004-A006 remain dependency-queued.
- `RECONSTRUCTING`: no immutable A003-P2 completion handoff exists yet; P2 is authorized but not reported complete.
- `FOREMAN_PLANNED/INTEGRATING`: Foreman 0013 is current. P1 acceptance/integration is complete at Foreman authority; P2 packet is issued. Foreman/governance also owns reconciliation of the two frozen unsupported-format logs/checker policy without rewriting immutable history.
- `VALIDATION_READY`: A003-P1 product-specific evidence is green but repository readiness remains `PARTIAL` until canonical project-check is settled green. A003-P2 is not Validation-ready.
- `PASS`: A001 and A002 machine/source PASS only.
- `BLOCKED`: no A003 product-behavior blocker is established. Current canonical red is governance/checker debt.
- `HARDWARE_PENDING`: A001 physical PS2; A002 physical audio/common-clock; A003 physical MPEG/video. No physical result is inferred.

## Goal governance and cadence health

A003-P1 governance closed correctly: worker `FOREMAN_GOAL_RESULT=MET`, followed by independent Foreman acceptance. A003-P2 now has a fresh bounded packet with explicit exact START/RETIRE representation, Pi emission-admission/lease/cleanup fencing, PS2 admission/decoder-join/residual-credit fencing, ordered same-stream restart safety, finite error semantics, deterministic host coverage, and explicit non-goals. No P2 worker result exists yet, so Continuity does not manufacture `FOREMAN_GOAL_RESULT`.

The governing cadence remains B -> Foreman -> A, one seat every 30 minutes and each seat every 90 minutes when scheduled seats are active. Current authority continues interactive substitution; no cadence drift is inferred from absent scheduled A/B wakes under that substitution. A scheduled A/B wake still requires a packet targeted to that worker; the current P2 packet targets `interactive`.

## Calibration separation invariant

Future A004 work must preserve two separate historical authorities:

- **DESKTOP CALIBRATION**: older safe-desktop/display-profile/screen-fit geometry for ordinary desktop/display presentation.
- **MPEG CALIBRATION**: H1 MPEG-presentation generation-specific base rectangle, inner matte, outer/suppression footprint, accepted-generation geometry, and accept-to-first-physical-frame lifecycle ownership.

The Foreman A004 documentation-only wake created and indexed the explicit separation invariant without implementing A004 behavior or altering current A003 packet authority. Generic `calibration` wording is insufficient where lineage/geometry/ownership could be confused. Future A004 packets, worker logs, validation, dictionaries, and state must name DESKTOP CALIBRATION versus MPEG CALIBRATION explicitly and must not silently derive one from the other.

## Reconciliation since revision 0034

1. Foreman revision 0013 consumed the A003-P1 worker handoff, accepted P1 M1-M8 as `MET`, and recorded settled product-specific canonical evidence.
2. Foreman issued A003-P2 exact-generation START/RETIRE lifecycle criteria P2-1 through P2-8. No immutable P2 Reconstruction completion handoff exists yet.
3. Validation independently reviewed settled P1 and found no product-source defect; product-specific machine evidence is PASS, while repository readiness remains `PARTIAL` solely because project-check rejects two frozen revision-0005 support logs not covered by current grandfather policy.
4. Diagnostics independently confirms the current red classification as governance/work-log documentation debt with product-specific executable gates green.
5. A004 calibration-separation authority was added and indexed. This is packetization/documentation authority only, not A004 product reconstruction and not a successor to active A003-P2.
6. A001/A002 machine/source PASS and all physical hardware obligations remain unchanged.

## Contradictions / stale state

- Reconstruction state 0007 remains historical A001-era state.
- Validation state 0006 remains a shared A001 snapshot; newer immutable logs carry current A002/A003 dispositions.
- The two frozen support logs using unsupported `LOG_FORMAT_REVISION=0005` conflict with work-log contract revision 0005's current nine-path grandfather set. Do not rewrite them; Foreman/governance must explicitly reconcile policy/checker authority.
- A003-P1 positive product evidence does not equal full repository Validation PASS while canonical project-check remains red.
- The A004 documentation-only Foreman wake must not be misread as an A004 Reconstruction packet or as superseding A003-P2.
- DESKTOP CALIBRATION and MPEG CALIBRATION are distinct; ambiguous generic calibration language that could merge their geometry/state/ownership is a governance contradiction to flag, not silently reconcile.

## Exact next safe actions

### Audit
Remain idle unless P2 exposes genuinely unexplained H1 responsibility requiring audit ownership.

### Foreman
Reconcile the two frozen unsupported-format work logs through explicit governing policy/checker authority without mutating immutable records; obtain settled canonical project-check PASS. Keep A003-P2 current against branch movement. Preserve the A004 calibration-separation invariant when later packetizing A004.

### Reconstruction A/B / interactive Reconstruction
Only the worker targeted by current Foreman authority may execute A003-P2. Implement P2-1 through P2-8; emit exactly one immutable completion handoff with `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED`. Do not substitute governance/checker chores or A004 calibration work.

### Validation
After governance reconciliation, independently close or retain the A003-P1 repository-readiness gate. Do not judge A003-P2 until its worker handoff and Foreman integration exist. Preserve DESKTOP CALIBRATION versus MPEG CALIBRATION separation in any future A004 review.

### Continuity
Consume P2 worker/Foreman/Validation movement and governance-checker reconciliation. Preserve worker result, Foreman acceptance/integration, Validation disposition, and physical qualification as separate authorities.