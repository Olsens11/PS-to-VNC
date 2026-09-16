# Ledge Validation Shift — V005 fatal teardown

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T00:20:29-04:00
COMPLETED_AT=2026-09-16T00:28:00-04:00
ROLE_KEY=validation
WORK_ITEM_KEY=v005-fatal-teardown
WORKER_KEY=validation
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a83470607bd3fd03e37bc5d5617f59d037dce75b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority

Re-review A001 with V005 as the primary validation target after Reconstruction's fatal-convergence work. Committed `ledge/h1-all-guns` authority was used; unknown Pi-local dirty work was not touched or declared absent. Validation consumed the current validation finding/state, work-log contract, current branch history, and the reconstructed Transport/application source now present at the starting HEAD.

## Checks and result

Static source review of `src/transport/bridge.c` and `src/app.c` confirms the V005 missing-cause defect has been reconstructed: `pstvnc_transport_session_abort()` now requests Transport-owned stop, waits for sole-receiver completion, and only then releases runtime resources. The application post-adoption fatal path calls this abort operation and never directly closes the adopted descriptor. Pre-adoption failure retains caller close ownership. Server-driven finite-RFB quiescence remains a distinct logical-RFB process rather than being repurposed as application fatal abort.

The current `tests/unit/app_test.c` has also been migrated to model Transport abort and descriptor adoption. This shift inspected test source but did not independently execute the fixture on a host, so executable evidence remains PENDING_LOCAL.

Architecture disposition: provisional PASS for V005 source shape. One physical Transport owner and sole receiver are preserved; no guessed timeout, second receiver, or application direct close was introduced. Receiver-completion-before-reclaim is explicit.

## Findings

V005 source defect: RESOLUTION_SUPPORTED_BY_STATIC_REVIEW. The shared finding register still says OPEN at this point-in-time because this shift intentionally avoided racing a concurrent reconstruction/documentation stream with a shared-file rewrite; the next validation pickup should revision-chain the validation finding/state once branch movement is coherent and executable evidence is available. This immutable record is the point-in-time validation evidence for the reconstructed source.

V003 remains OPEN pending coherent A001 completion/evidence. V004 remains OPEN and is Foreman-owned dictionary/topology/portal/checker/build-evidence work under the current role split. No new finding opened.

A001 is not marked VALIDATION_READY or PASS by this shift. Concrete production Transport CONFIG authority remains unresolved in `pstvnc_app_run()`, which intentionally fails closed; canonical host/static checks, clean PS2DEV build, reproducibility and exact ELF/PT_LOAD evidence were not independently executed here.

PENDING_LOCAL=host fixture execution; canonical scripts/check.sh; clean PS2DEV build; reproducibility/PT_LOAD evidence; Foreman integration evidence
HARDWARE_PENDING=YES

## Owner handoff

V005 product-source implementation appears reconstructed and awaits independent executable confirmation / findings-register closure by Validation. V004 remains Foreman chore work. Any new product-behavior defect returns to Reconstruction A/B.

## Revisions

Consumed validation findings revision 0004 and validation state revision 0005. Work-log contract revision 0001. No validation state/findings revision produced in this shift because concurrent branch work was active and no shared-file race was taken.

## Exact next pickup

Re-read newest Reconstruction/Foreman logs and branch authority. Independently execute or inspect committed evidence for Transport fatal-convergence and application lifecycle fixtures. If results support the static disposition, revision-chain V005 to RESOLVED and validation state accordingly. Then review V004 Foreman dictionary/topology/build integration and remaining A001 CONFIG authority before any VALIDATION_READY decision.
