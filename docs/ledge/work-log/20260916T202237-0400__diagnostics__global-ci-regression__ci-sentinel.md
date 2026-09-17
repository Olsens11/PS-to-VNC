DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T20:22:37-04:00
COMPLETED_AT=2026-09-16T20:28:12-04:00
ROLE_KEY=diagnostics
WORK_ITEM_KEY=global-ci-regression
WORKER_KEY=ci-sentinel
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e6404cf041b48fded5cd41d4d797c05e130fdc0d
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# CI / Regression Sentinel — current-head governance gate

## Authority consumed

Refreshed `ledge/h1-all-guns` at `e6404cf041b48fded5cd41d4d797c05e130fdc0d`; re-read work-log contract revision 0005 and Foreman state revision 0013. The newest Validation log on this same source authority preserves A003-P1 product-machine evidence as healthy, leaves full repository readiness pending only on the work-log governance gate, and does not judge active A003-P2 Reconstruction.

## Workflow evidence

Newest settled current-HEAD workflow: run `35166264194`, source `e6404cf041b48fded5cd41d4d797c05e130fdc0d`, conclusion `failure`.

Exact jobs inspected:
- `host-unit=SUCCESS` (job 105027938497; canonical host unit tests passed).
- `ps2-compile=SUCCESS` (job 105027938330; pinned PS2 compilation passed).
- `ps2-link=SUCCESS` (job 105027938462; current ledge link plus current-source reproducibility passed).
- `dictionary-long=SUCCESS` (job 105027938506; complete strict source-dictionary audit passed).
- `project-check=FAILURE` (job 105027938521; only `Run canonical project check` failed).
- `dictionary-reconcile=SKIPPED` (job 105027939216).

The current Foreman/Validation authority identifies the canonical project-check failure as `WORK_LOG_CHECK=FAIL` on exactly two already-frozen support records that carry unsupported `LOG_FORMAT_REVISION=0005` while governing revision 0005 requires new records to use `LOG_FORMAT_REVISION=0001`:
- `docs/ledge/work-log/20260916T160445-0400__integration__global-dictionary-prep__dictionary.md`;
- `docs/ledge/work-log/20260916T162043-0400__validation__a003-mpeg-generation__validation.md`.
Direct reads confirm both frozen records contain `LOG_FORMAT_REVISION=0005`.

FAILURE_CLASSIFICATION=GOVERNANCE_WORK_LOG_DOCS
ROUTE=FOREMAN_GOVERNANCE
RECONSTRUCTION_CORRECTION=NO
STALE_OR_TRANSIENT=NO; this is the newest settled current-HEAD run and the governance failure remains reproducible in current authority.

## Lightweight protected-invariant scan

No new product-source commit landed between the prior coherent Sentinel/Foreman evidence and this wake; current HEAD is a Validation immutable-log commit. Product-specific host, compile, link/reproducibility, and strict dictionary jobs are green. Current Foreman authority continues to bind A003-P2 to the existing sole ordered Transport receiver, forbids per-MPEG-DATA generation tags/auxiliary physical connections, requires exact-generation join/resource fencing, and keeps cancel/abort distinct from finite EOF and RETIRE success. No current evidence establishes regression of the sole physical PSTV/Transport receive owner, direct RFB/AUDIO/MPEG socket ownership, caller-owned Transport configuration, RFB finite-quiescence semantics, worker-before-reclaim ordering, timeout-as-success behavior, AUDIO `audsrv_quit()` ownership, or known-defect accounting.

A004 separation check: no new A004 product source was introduced in the inspected delta. Current Validation authority explicitly preserves DESKTOP CALIBRATION as the older safe-desktop/display-profile/screen-fit system and MPEG CALIBRATION as the later exact-generation MPEG presentation geometry/lifecycle system. No substitution/collapse signal is present.

INVARIANT_SCAN_RESULT=NO_CONFIRMED_REGRESSION_SIGNAL

## Exact next pickup

Re-read live HEAD and newest workflow. Foreman/governance must reconcile the two immutable unsupported-format records through explicit policy/checker authority without rewriting them, then obtain a settled canonical `project-check=SUCCESS`. Continue to treat active A003-P2 source movement as intermediate until a coherent Reconstruction handoff is consumed by the Foreman; if later CI turns red on product jobs, inspect the exact failing step/log before classifying it.