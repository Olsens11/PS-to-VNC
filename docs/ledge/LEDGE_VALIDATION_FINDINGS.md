# Ledge Validation — Findings

DOCUMENT=LEDGE_VALIDATION_FINDINGS
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-15T11:28:00-04:00
TEMPORAL_CLASS=APPEND_ONLY_FINDING_REGISTER

Findings are validation-owned. They describe evidence and required disposition; they do not redesign product behavior or rewrite audit/reconstruction history.

## V001 — A001 source validation blocked by primary architecture contradiction

SEVERITY=GATE
STATUS=RESOLVED
AFFECTED_TRANCHE=A001
AFFECTED_RECONSTRUCTION_STATE=0001
RESOLVED_BY=docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md:0001

Evidence at opening:

- semantic audit revision 0002 marked A001 transport/RFB responsibilities `RECONSTRUCTION_READY` and required one physical PSTV socket, one sole receiver, logical-channel dispatch, serialized send, and an explicit logical RFB byte-stream boundary;
- reconstruction state revision 0001 correctly identified that no product-source reconstruction could begin without reconciling that ownership against the primary architecture;
- `docs/CLEAN_ARCHITECTURE.md` version 1 remained `PRIMARY_ARCHITECTURE_AUTHORITY=YES` and assigned all RFB writes/requests and receive orchestration to the main/application thread while describing RFB as depending directly on socket/PS2IP facilities;
- that reconstruction shift changed documentation only; there was therefore no A001 reconstructed source tranche to test or mark PASS.

Resolution evidence:

Governance revision `LEDGE_ARCHITECTURE_OVERLAY` 0001 explicitly supersedes the conflicting architecture-version-1 socket ownership for `ledge/h1-all-guns`, establishes one PSTV transport owner and sole receiver, makes RFB consume a logical byte stream, preserves application policy ownership, and carries forward the complete A001 validation obligations. Reconstruction state 0002 consumed that authority and began the transport interface without recreating a competing physical socket.

Validation disposition:

`RESOLVED` for the architecture prerequisite only. This does not validate A001 behavior or hardware.

## V002 — Reconstruction lane respected source/authority boundary

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001_GATE_HANDLING

Evidence:

The reconstruction shift did not modify forensic H1 source, reconstructed product source, audit records, or validation records. Its two reconstruction-owned continuity documents explicitly preserve the semantic requirements while refusing to create a competing transport architecture.

Validation disposition:

`PASS` for gate handling and lane ownership only. This is not behavioral validation of A001 and is not hardware qualification.

## V003 — A001 interface/topology tranche is intentionally incomplete

SEVERITY=INFO
STATUS=OPEN
AFFECTED_TRANCHE=A001
AFFECTED_RECONSTRUCTION_STATE=0002

Evidence:

- `src/transport/transport.h` establishes the approved logical RFB stream and application-requested/transport-owned lifecycle boundary without exposing physical socket operations to RFB;
- `src/transport/SYMBOLS.md` accounts for the currently introduced public transport symbols;
- no transport implementation body, sole receiver, physical framing/sequence handling, logical RFB queue/credit/fragmentation implementation, serialized physical send, or explicit dispatch-quiescence mechanism exists in the tranche yet;
- reconstruction state 0002 explicitly records that the new directory is not wired into the build and that topology/continuity/build/dictionary/test obligations remain incomplete;
- reconstruction therefore correctly leaves A001 `IN_PROGRESS` and not `VALIDATION_READY`.

Validation disposition:

`WAIT_FOR_COHERENT_TRANCHE`. The interface shape is provisionally consistent with architecture overlay 0001 and the A001 audit boundary, and its current public symbols are locally documented. No behavioral, build, reproducibility, PT_LOAD, or hardware PASS is available yet.

Required resolution:

Reconstruction must finish the coherent A001 implementation and new-domain integration obligations, add the queued host tests, run canonical checks/builds, and hand the tranche to validation with exact source/build identity. Validation must then independently execute the A001 obligation set before changing this finding to PASS/HARDWARE_PENDING or opening defects.
