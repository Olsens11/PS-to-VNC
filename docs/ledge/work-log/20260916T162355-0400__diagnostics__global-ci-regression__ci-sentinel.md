DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T16:23:55-04:00
COMPLETED_AT=2026-09-16T16:26:40-04:00
ROLE_KEY=diagnostics
WORK_ITEM_KEY=global-ci-regression
WORKER_KEY=ci-sentinel
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=90412d08a43c99fbe5de941632fffb9a8857002b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# CI / Regression Sentinel

## Authority consumed

- Branch: `ledge/h1-all-guns`.
- Starting and final pre-write branch authority: `90412d08a43c99fbe5de941632fffb9a8857002b` (`docs(ledge): log A003 validation wait`).
- Governing work-log contract: revision `0005`.
- Current Foreman state observed: revision `0012`, A003-P1 MPEG Transport/decoder core packet issued.
- Repository authority was re-read immediately before this immutable-log create.

## Workflow evidence

Newest settled-tree workflow inspected: run `35146056349`, head `90412d08a43c99fbe5de941632fffb9a8857002b`, conclusion `failure`.

Exact job conclusions:

- `host-unit` job `104962318790`: FAILURE. Actual linker evidence in canonical `make -C tests unit`: `transport_runtime_test` links `runtime.c`, `quiesce.c`, `audio_channel.c`, `rfb_channel.c`, and `protocol.c` but omits the newly required `mpeg_channel.c`; linker therefore reports undefined `pstvnc_transport_mpeg_channel_*` references. Classification: `MECHANICAL_INTEGRATION_TOOLING`.
- `ps2-link` job `104962318972`: FAILURE. In the current A003-P1 moving tree this is consistent with the same not-yet-integrated MPEG implementation/build graph; no product-behavior conclusion is drawn from it. Classification: `MECHANICAL_INTEGRATION_TOOLING` pending Foreman integration.
- `project-check` job `104962318997`: FAILURE. Actual checker evidence: `SOURCE_TOPOLOGY_DIRECTORY_SET_MISMATCH`; expected clean directory set omits `src/mpeg` while actual source contains `src/mpeg`. Classification: `GENERATED_DICTIONARY_TOPOLOGY` / source-topology integration debt.
- `ps2-compile` job `104962319078`: SUCCESS. The current source remains compilable under the pinned PS2 compile inventory.
- `dictionary-long` job `104962319195`: FAILURE. This is an unreconciled A003-P1 dictionary/integration state on the same moving tree; route to source-side Reconstruction completion and later Foreman generated reconciliation, not as an established product defect. Classification: `GENERATED_DICTIONARY_TOPOLOGY`.
- `dictionary-reconcile`: SKIPPED because prerequisite jobs were red.

No later relevant green settled-tree run existed at the final pre-write authority, so run `35146056349` is current rather than stale.

## Regression scan

Lightweight scan against the current A003-P1 authority found no evidence in this CI pass that a second physical receiver/socket owner was introduced or that RFB/AUDIO/MPEG acquired direct competing physical socket ownership. The host linker failure itself shows MPEG calls are being made from the existing Transport runtime rather than proving a second receiver path.

No CI evidence established endpoint drift back to direct VNC 5900, a hardcoded replacement for caller/profile Transport configuration, finite-RFB quiescence converted to fatal abort, generic timeout-as-success, per-session `audsrv_quit()`, or silent repair of a known inherited defect.

The A003-specific protected boundary remains: MPEG must remain an independent logical path under the sole Transport receive owner; local decoder stop must not synthesize false EOF; finite producer exhaustion must remain distinct from cancellation; decoder-call ownership must return before decoder-visible reclamation. No current CI failure contradicts those invariants.

## Routing

ROUTE_TO_RECONSTRUCTION=Finish the already-active A003-P1 source-owned MPEG dictionary/topology obligations and any behavior-specific test source required by the packet; do not treat current red CI as proof of a behavior defect.

ROUTE_TO_FOREMAN=After the Reconstruction handoff, register `mpeg_channel.c`/MPEG tests/build linkage and perform generated dictionary portal/canonical integration evidence as Foreman-owned mechanical work.

ROUTE_TO_VALIDATION=NONE from this sentinel pass; independent Validation should wait for coherent Foreman-integrated A003 authority.

PRODUCT_BEHAVIOR_OR_TEST_DEFECT=NOT_ESTABLISHED
INFRASTRUCTURE_TRANSIENT=NO_EVIDENCE
UNKNOWN_NEEDS_FOREMAN=NO

PENDING_LOCAL=canonical settled-tree rerun after A003-P1 source handoff and Foreman mechanical integration
HARDWARE_PENDING=unchanged; this pass makes no physical PS2 qualification claim

## Next pickup

Inspect the first workflow run after the A003-P1 Reconstruction immutable handoff and Foreman integration. Confirm that `transport_runtime_test` links MPEG channel ownership, `src/mpeg` is present in source topology, strict dictionary reconciliation is clean, and PS2 link/reproducibility returns green before escalating any residual failure as product behavior.
