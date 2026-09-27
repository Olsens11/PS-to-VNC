# Ledge Validation — Findings

DOCUMENT=LEDGE_VALIDATION_FINDINGS
DOCUMENT_REVISION=0006
RECORDED_AT=2026-09-27T05:56:12-04:00
TEMPORAL_CLASS=APPEND_ONLY_FINDING_REGISTER

Findings are validation-owned. They describe evidence and required disposition; they do not redesign product behavior or rewrite audit/reconstruction history.

## V001 — A001 source validation blocked by primary architecture contradiction

SEVERITY=GATE
STATUS=RESOLVED
AFFECTED_TRANCHE=A001
RESOLVED_BY=docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md:0001

The architecture prerequisite remains resolved by overlay revision 0001. This resolution is not itself behavioral or hardware qualification.

## V002 — Reconstruction lane respected source/authority boundary

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001_GATE_HANDLING

The prior gate-handling disposition remains PASS and is not changed by this validation pass.

## V003 — A001 interface/topology tranche completion

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001
RESOLVED_AT_SOURCE=20b2e21b7718d987892bb71b498d609a5db0ec5d

Resolution evidence:

- the coherent current source owns one physical PSTV connection and sole receiver in Transport;
- RFB consumes the logical bridge rather than a physical socket descriptor;
- application transfers descriptor ownership to Transport and routes fatal post-adoption convergence through Transport;
- canonical host tests, project checks, strict dictionaries, PS2 compile, and current linked reproducibility all pass in workflow run 35091578944 at exact source authority `20b2e21b7718d987892bb71b498d609a5db0ec5d`;
- the docs-only handoff HEAD `0cb250b330e962de673dd712d7885905eb6e8128` also has successful canonical workflow run 35091890556.

Validation disposition: `PASS_MACHINE_SOURCE`. Physical PS2 qualification remains separate.

## V004 — Transport symbol dictionary completeness

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001
RESOLVED_AT_SOURCE=20b2e21b7718d987892bb71b498d609a5db0ec5d

Resolution evidence:

- `src/transport/SYMBOLS.md` and `src/rfb/SYMBOLS.md` are current complete dictionaries;
- the generated source-dictionary portal includes the Transport/RFB ownership surfaces;
- topology/continuity integration recognizes the Transport domain;
- `python3 scripts/source-dictionary.py check --long --require-complete --strict` passes canonically in run 35091578944;
- `scripts/check.sh` passes in that run and again at docs-only current handoff HEAD in run 35091890556.

Validation disposition: `PASS` for the source/dictionary/topology/portal gate.

## V005 — Application fatal teardown convergence

SEVERITY=HIGH
STATUS=PASS
AFFECTED_TRANCHE=A001
RESOLVED_AT_SOURCE=20b2e21b7718d987892bb71b498d609a5db0ec5d

Resolution evidence:

- `pstvnc_transport_session_abort()` is now an explicit application-local fatal-convergence operation distinct from finite RFB quiescence;
- Transport publishes stop intent, interrupts its privately owned blocking physical I/O, waits for sole-receiver completion, and only then releases receiver-visible state and the adopted descriptor;
- `src/app.c` calls this operation on every fatal path after successful Transport adoption and never directly closes the adopted descriptor;
- direct Transport runtime host fixture coverage exercises blocking receive interruption, receiver-done ordering, release ordering, retryable lifecycle failure, and repeatable ownership without timeout masking;
- the canonical host suite including `transport_physical_stream_test` and `transport_runtime_test` passes in run 35091578944 and again on docs-only current handoff HEAD in run 35091890556.

Validation disposition: `PASS_MACHINE_SOURCE`. This resolves the reconstruction-owned deadlock finding. It does not infer PS2 physical success.

## Current A001 finding summary

V001 RESOLVED; V002 PASS; V003 PASS; V004 PASS; V005 PASS.

No open Validation finding currently blocks A001 machine/source acceptance. The exact remaining boundary is physical PS2 qualification of the changed PT_LOAD/DUT, which remains `HARDWARE_PENDING` until operator-backed evidence exists.

## V006 — Complete-current A001-A006 machine/source closure

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001_A006
VALIDATED_PRODUCT_SOURCE=e2727cb31e214d11e094f1f45b0b9d8ab01d84e9
VALIDATED_LEDGER_HEAD=095e44b1da60fab13a22bd0601818911a72f57e6

Independent Validation reviewed the complete accumulated A001-A006 reconstructed source rather than only the final R43 change or the Foreman's disposition.

Resolution evidence:

- A001 preserves one Transport physical owner/receiver, logical RFB ownership, credit/residual/activity/quiescence, outbound-admission drain, proven receiver completion, descriptor release ordering, and Transport-owned fatal convergence;
- A002 preserves Configuration provenance/typed projections without invented defaults, AUDIO owner-completion/join reclamation, resident AUDSRV policy, and common-clock ownership;
- A003 preserves Q4/session fencing, one Pi Wire owner/global sequence, provider-local RFB attachment/reporting/recovery, exact MPEG generation/run ownership, real retirement proof, and fresh-session product composition;
- A004 preserves one Presentation owner, strict DESKTOP CALIBRATION versus MPEG CALIBRATION separation, validated base/inner/suppression geometry, first-physical-frame promotion, and current Q7 restoration-under-retiring-MPEG ordering before final reveal;
- A005 preserves input-worker semantic-only ownership, Application product-action execution, configurable binding publication, physical-continuity/quarantine rules, cooperative dormancy before reclamation, and bounded Management/Configuration snapshot behavior;
- A006 preserves Application cross-domain retirement, real MPEG/AUDIO abort readiness before clock/Transport reclamation, R42 media ordering, R43 one successful retained-session begin-abort per attempt across cleanup re-entry, failed-begin fail-closed behavior, and fresh replacement-attempt state;
- canonical current-head Actions run `36287568567` is SUCCESS for host-unit, project-check, strict dictionary-long, pinned PS2 compile, linked build and current-source ELF/PT_LOAD reproducibility;
- source comparison from `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9` to `095e44b1da60fab13a22bd0601818911a72f57e6` contains only Foreman/Reconstruction documentation and immutable logs.

Validation disposition: `PASS_MACHINE_SOURCE`.

No reconstruction-owned A001-A006 source defect remains open from this pass. Physical qualification of the exact final linked identity remains `HARDWARE_PENDING`; V006 does not claim or imply a PS2 hardware PASS.

## Current complete-source finding summary

V001 RESOLVED; V002 PASS; V003 PASS; V004 PASS; V005 PASS; V006 PASS.

OPEN_RECONSTRUCTION_OWNED_A001_A006_VALIDATION_FINDINGS=NONE
COMPLETE_CURRENT_LEDGE_MACHINE_SOURCE=PASS
COMPLETE_CURRENT_LEDGE_HARDWARE=HARDWARE_PENDING

The next independent gate is final ledge hardware qualification bound to the exact final ELF/PT_LOAD identity.
