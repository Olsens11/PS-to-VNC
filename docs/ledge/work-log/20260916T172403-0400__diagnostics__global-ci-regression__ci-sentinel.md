DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T17:24:03-04:00
COMPLETED_AT=2026-09-16T17:27:00-04:00
ROLE_KEY=diagnostics
WORK_ITEM_KEY=global-ci-regression
WORKER_KEY=ci-sentinel
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3519df7be8a7d9fb72c7e0ea1005a2ff8364e4ca
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# CI / Regression Sentinel

## Authority consumed

- Governing immutable-work-log contract: `docs/ledge/work-log/README.md`, revision 0005.
- Starting and pre-write branch authority: `3519df7be8a7d9fb72c7e0ea1005a2ff8364e4ca` (`docs(ledge): record A003 validation overlap pause`).
- Immediate substantive predecessor: `0ec6223d45a80871c8fdb0b1ccd77328779c15cd` (`a003: close MPEG decoder source ownership`).
- Current branch movement was re-read before this log write; no newer commit superseded the diagnosis.

## Workflow evidence

Newest settled-tree workflow inspected: GitHub Actions run `35151913192`, head `3519df7be8a7d9fb72c7e0ea1005a2ff8364e4ca`, conclusion `failure`.

Exact job conclusions:
- `ps2-compile` job `104982033178`: SUCCESS.
- `dictionary-long` job `104982033482`: SUCCESS.
- `project-check` job `104982033493`: FAILURE.
- `host-unit` job `104982033538`: FAILURE.
- `ps2-link` job `104982033583`: FAILURE; reproducibility and identity steps skipped after link failure.
- `dictionary-reconcile` job `104982034715`: SKIPPED because prerequisite jobs were not all successful.

Failure evidence:
- `host-unit` fails while linking `transport_runtime_test`: `src/transport/runtime.c` now references the MPEG channel implementation, but the canonical test link command includes `audio_channel.c`, `rfb_channel.c`, `protocol.c`, etc. and omits `src/transport/mpeg_channel.c`. The linker therefore reports undefined references including `pstvnc_transport_mpeg_channel_commit_data`, `_initialize`, `_read_available`, `_available`, `_producer_done`, and `_mark_producer_done`.
- `ps2-link` compiles the current source set successfully through `transport_runtime.o`, then the final ELF link omits a `transport_mpeg_channel.o` object and fails on the same MPEG channel symbols. This is not a compiler rejection of the new source.
- `project-check` reaches CLEAN PRODUCT SOURCE TOPOLOGY and fails with `ERROR=SOURCE_DICTIONARY_PORTAL_OUT_OF_DATE`, explicitly requiring regeneration of `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.
- Complete strict `dictionary-long` succeeds on the same head, narrowing the project-check failure to generated portal reconciliation rather than missing source definitions.

## Classification and routing

- Host-unit MPEG undefined references: `MECHANICAL_INTEGRATION_TOOLING`.
- PS2-link MPEG undefined references: `MECHANICAL_INTEGRATION_TOOLING`.
- Source dictionary portal stale: `GENERATED_DICTIONARY_TOPOLOGY`.
- No current failure is classified `PRODUCT_BEHAVIOR_OR_TEST_DEFECT` from this CI evidence.
- Route the mechanical link/build/test registration and generated portal reconciliation to the Foreman/integration boundary after the active A003-P1 Reconstruction handoff is coherent. Do not route a product correction to Reconstruction from these failures alone.

## Lightweight architecture regression scan

No CI/source-change signal inspected in this pass establishes a protected-invariant regression. In particular, the failures are consistent with `runtime.c` consuming the new logical MPEG channel while canonical build/test graphs have not yet adopted its implementation object; they do not indicate a second physical PSTV receiver or direct MPEG socket owner. Existing RFB/Transport bridge and physical-stream fixtures reached PASS before host-unit stopped at the missing MPEG object. No evidence in this pass establishes endpoint regression to direct VNC 5900, timeout-as-success behavior, per-session `audsrv_quit()`, or silent repair of an inherited defect.

This is a lightweight sentinel scan, not independent Validation; absence of a signal is not a Validation PASS.

## Current diagnosis

The newest settled-tree red run remains an A003-P1 integration boundary: source ownership has advanced through the MPEG decoder/Transport seam, while canonical host/PS2 link graphs and the generated dictionary portal have not yet caught up. The fact that `ps2-compile` and strict `dictionary-long` both pass materially narrows the red state away from source compilation and source-dictionary-definition defects.

PENDING_LOCAL=NO additional local execution required for this sentinel classification; canonical CI supplied the evidence.
HARDWARE_PENDING=UNCHANGED; this pass makes no physical PS2 qualification claim.

## Next pickup

Inspect the first workflow run after Foreman integration of `src/transport/mpeg_channel.c` into canonical host/PS2 linkage and regeneration of the source-dictionary portal. If that settled-tree run is green, record a meaningful NOOP unless a protected-invariant scan finds a new signal. If red, fetch the exact failed job logs and classify the new failure rather than carrying this diagnosis forward by assumption.
