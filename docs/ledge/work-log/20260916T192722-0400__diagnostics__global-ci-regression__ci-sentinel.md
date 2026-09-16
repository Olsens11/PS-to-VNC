DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T19:27:22-04:00
COMPLETED_AT=2026-09-16T19:31:40-04:00
ROLE_KEY=diagnostics
WORK_ITEM_KEY=global-ci-regression
WORKER_KEY=ci-sentinel
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e913a250b1aab1682924d1ff14036d8f95f87f9e
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# CI / Regression Sentinel

## Authority consumed

- Governing work-log contract: `docs/ledge/work-log/README.md`, revision 0005.
- Current Foreman state: `LEDGE_FOREMAN_STATE` revision 0013.
- Starting and pre-write branch authority: `e913a250b1aab1682924d1ff14036d8f95f87f9e`.
- Latest branch commits include the A004 documentation-only calibration-separation invariant commits through `e913a250...`; no A004 product implementation was observed in this pass.

## CI evidence

Newest completed workflow inspected: run `35162265330`, head `e913a250b1aab1682924d1ff14036d8f95f87f9e`, overall `failure`.

Exact job conclusions:

- `project-check`: FAILURE; failing step `Run canonical project check`.
- `host-unit`: SUCCESS.
- `ps2-compile`: SUCCESS.
- `ps2-link`: SUCCESS, including current-source build/reproducibility steps.
- `dictionary-long`: SUCCESS.
- `dictionary-reconcile`: SKIPPED because the prerequisite project-check failed.

The GitHub connector did not expose the job log body endpoint in this pass, so failure classification is reconciled against the current Foreman state and governing work-log contract rather than guessed from the check name. Foreman revision 0013 records the settled canonical project-check reaching `WORK_LOG_CHECK=FAIL` on two already-frozen support logs carrying unsupported `LOG_FORMAT_REVISION=0005`: `20260916T160445-0400__integration__global-dictionary-prep__dictionary.md` and `20260916T162043-0400__validation__a003-mpeg-generation__validation.md`. Current work-log contract revision 0005 still requires new records to use `LOG_FORMAT_REVISION=0001` and does not list those two records among its nine exact grandfather exceptions.

FAILURE_CLASSIFICATION=GOVERNANCE_WORK_LOG_DOCS

No product-behavior or test defect is established by this run. Product-specific executable gates are green. Route the current red condition to Foreman/governance reconciliation, not Reconstruction. Do not rewrite either immutable historical log; contract/checker authority must resolve the frozen-record incompatibility explicitly.

## Lightweight protected-invariant scan

RESULT=NO_CONFIRMED_REGRESSION_SIGNAL

Evidence basis: host-unit, PS2 compile, PS2 link/reproducibility, and strict dictionary audit all remain green on the current HEAD. Current Foreman source review continues to record one physical ordered Transport stream/sole receive owner, MPEG as logical channel 4 behind that owner, explicit caller-owned bounds, finite exhaustion distinct from stop, and worker/resource retirement ordering. No current evidence shows RFB/AUDIO/MPEG reacquiring a direct physical socket, a second receiver path, endpoint drift to legacy 5900, generic timeout-as-success, or per-session `audsrv_quit()` regression.

A004 calibration separation was newly elevated into repository authority by documentation commits immediately before this pass. No A004 product-source implementation appeared in the inspected newest commits, so there is no source/test evidence of desktop calibration being substituted for MPEG calibration or their geometry/state ownership being collapsed. Preserve this as an active protected invariant once A004 source work begins.

Known inherited defects remain governed by current audit/Foreman authority; this pass found no evidence of an unauthorized silent rewrite.

## Routing

- Reconstruction: NONE from current CI evidence.
- Foreman/governance: reconcile the two frozen unsupported-format work logs through explicit current policy/checker authority; do not mutate immutable records.
- Ignore as stale/transient: NONE for run `35162265330`; it is the newest completed run on the starting/current pre-write HEAD.

## Next pickup

Inspect the first settled workflow after governance reconciliation or any newer source commit. If A003-P2 product source lands, extend the invariant scan to exact START/RETIRE same-stream fencing, exact-generation admission/lease/cleanup ordering, decoder join/residual-credit retirement, and mismatch/cancel/error separation. If A004 implementation appears, explicitly scan that desktop-calibration and MPEG-calibration authorities remain separate.

PENDING_LOCAL=exact raw project-check job-log text was unavailable through the connector endpoint in this pass; classification is supported by current Foreman state plus the governing work-log contract and green product-specific jobs.
HARDWARE_PENDING=unchanged; no physical qualification claim is made here.
