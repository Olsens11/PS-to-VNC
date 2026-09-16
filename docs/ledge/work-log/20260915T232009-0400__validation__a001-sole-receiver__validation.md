# Validation shift — A001 sole receiver / live-path lifecycle

ROLE_KEY=validation
WORKER_KEY=validation
WORK_ITEM_KEY=a001-sole-receiver
STARTED_AT=2026-09-15T23:20:09-04:00
COMPLETED_AT=2026-09-15T23:22:01-04:00
STATUS=BLOCKED_ON_PRODUCT_BEHAVIOR
STARTING_AUTHORITY=9790ae22f6db4d0aee20f822c9fd6c51d1c7d129
PRE_LOG_AUTHORITY=f2a9362f678b7a83e538270e46cf2f5667d6734c
VALIDATION_STATE_REVISION=0005
VALIDATION_FINDINGS_REVISION=0004
RECONSTRUCTION_STATE_REVISION_INSPECTED=0007
FOREMAN_STATE_REVISION_INSPECTED=0002
AUDIT_STATE_REVISION_INSPECTED=0007
GLOBAL_STATE_REVISION_INSPECTED=0011

## Scope reviewed

Reviewed the newly committed A001 RFB-session logical-bridge migration and application Transport lifecycle adoption, including `src/app.c`, `src/app.h`, `src/transport/bridge.h`, the application ownership fixture, recent branch movement, Foreman packet authority, reconstruction state, and existing validation findings. Repository committed state was treated as authority; unknown Pi-local dirty work was neither overwritten nor declared absent.

## Exact checks and results

- branch/HEAD authority before review: PASS; starting HEAD `9790ae22f6db4d0aee20f822c9fd6c51d1c7d129`;
- live descriptor ownership: PROVISIONAL PASS by source inspection; application passes the connected descriptor once to `pstvnc_transport_session_open()` and Transport consumes caller ownership on adoption;
- RFB physical-socket exclusion: PROVISIONAL PASS by source inspection; application starts RFB without supplying the physical descriptor and current session seam is logical-bridge based;
- configuration provenance boundary: PROVISIONAL PASS; `pstvnc_app_run_with_transport_config()` requires caller-supplied config and `pstvnc_app_run()` fails closed rather than guessing values;
- receiver-before-reclaim ordering: structurally present, but fatal convergence FAILS validation because the cleanup path waits for receiver completion without first causing a healthy receiver to terminate;
- application ownership test source: PRESENT; no independent executable PASS was available on this GitHub-only surface;
- canonical host/static checks, PS2DEV build, reproducibility and ELF/PT_LOAD identity: PENDING_LOCAL this shift;
- hardware qualification: HARDWARE_PENDING and not claimed.

## Finding opened

V005 HIGH/OPEN — `src/app.c` post-adoption fatal cleanup calls `pstvnc_transport_session_wait_receiver_done()` before close, while the Transport bridge exposes no application-requested stop/abort operation. Server-driven finite RFB quiescence is not an application-local fatal convergence mechanism. An application-local failure with an otherwise healthy receiver can therefore wait on a completion condition it did not cause. This blocks `VALIDATION_READY` until Reconstruction provides authority-consistent deterministic convergence without a guessed timeout, unsafe physical close, second receiver, or hidden receive-poison workaround.

OWNER_HANDOFF=RECONSTRUCTION_A_OR_B
FOREMAN_CHORE_HANDOFF=V004_DICTIONARY_TOPOLOGY_PORTAL_AND_CANONICAL_EVIDENCE_REMAINS_FOREMAN_OWNED

## Self-correction

The first validation-findings write in this shift accidentally compressed the historical V003/V004 evidence while adding V005. Because the findings register is append-preserving authority, validation immediately restored the complete prior V001-V004 text in the next commit while retaining V005. No product source was touched.

## Current disposition

A001 is not `VALIDATION_READY`. V003 OPEN, V004 OPEN, V005 HIGH/OPEN. The live ownership migration is materially advanced, but deterministic fatal-session convergence is a product-behavior blocker. Machine evidence remains PENDING_LOCAL; PS2 hardware evidence remains HARDWARE_PENDING.

## Exact next pickup

Inspect the next Reconstruction disposition of V005 first. Require behavior-specific proof that a post-adoption application-local failure reaches receiver completion/reclamation deterministically while preserving sole physical receive ownership, safe finite-RFB quiescence semantics, and the inherited H1 receive-poison boundary. Then independently evaluate the application lifecycle fixture execution and Foreman-prepared V004/canonical build/reproducibility evidence.
