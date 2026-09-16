# Ledge Validation — Findings

DOCUMENT=LEDGE_VALIDATION_FINDINGS
DOCUMENT_REVISION=0005
RECORDED_AT=2026-09-16T07:53:00-04:00
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