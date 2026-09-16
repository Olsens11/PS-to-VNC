# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0005
RECORDED_AT=2026-09-16T07:03:57-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0023
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman continuation consumes Foreman revision 0004 and verifies the branch remained at `e5188afe793fc70a64c3a2b2b75d3147be50ede2` before this write. It continues A001 integration/validation preparation without assigning filler Reconstruction work or beginning A002.

## Current foreman phase

`A001_FOREMAN_INTEGRATION_WITH_GOVERNANCE_CHECK_BLOCKER`

The A001 product-behavior packet remains MET. The two direct Transport fixtures remain canonically registered at source level, no new A001 product defect has been identified, and Reconstruction A/B plus the interactive Reconstruction B substitute remain on hold unless executable/independent evidence exposes a concrete reconstruction-owned defect.

## New integration finding — canonical work-log checker cannot currently pass

Foreman inspected the governing immutable work-log contract and `scripts/work-log-check.py` rather than continuing to label every unexecuted check merely `PENDING_LOCAL`.

The checker requires every Markdown shift record under `docs/ledge/work-log/` except `README.md` to use the canonical punctuation-free filename grammar:

`YYYYMMDDTHHMMSS+HHMM__role-key__work-item-key__worker-key.md`

or the corresponding negative UTC offset form. A filename that does not match this grammar fails immediately before record metadata is considered.

Current branch authority contains at least these two Validation-owned immutable records whose filenames do not match that grammar:

- `docs/ledge/work-log/2026-09-16T05-18-33-04-00__validation__v005-fatal-teardown__validation.md`
- `docs/ledge/work-log/2026-09-16T06-20-13-04-00__validation__a001-sole-receiver__validation.md`

Therefore `scripts/work-log-check.py`, and consequently `scripts/check.sh`, have a known deterministic failure condition in current committed source/history independent of whether an execution-capable checkout is available.

This is **not** an A001 product-behavior defect and not Reconstruction-owned work. It is a governance/history contradiction:

- the work-log contract says committed shift records are immutable and must not be edited, renamed, replaced, or deleted;
- the checker rejects the two already-committed malformed filenames;
- a later corrective immutable record alone cannot make those original filenames satisfy the current checker because the checker enumerates every Markdown record.

Foreman will not silently rename/delete Validation history and will not weaken the checker ad hoc. Resolution requires explicit governance/Validation authority for a history-compatible correction mechanism (for example, a narrowly governed exception/correction manifest or an explicitly authorized historical rename rule). Until that authority exists, a canonical full-project `scripts/check.sh=PASS` is impossible on the current history.

## A001 integration status

### Complete at source level

1. C1-C8 A001 behavior closure packet: MET by interactive Reconstruction B substitute.
2. Direct physical-stream/runtime fixtures: committed and independently statically reviewed without a new DUT defect.
3. Canonical unit registration: complete in `tests/Makefile` for `transport_physical_stream_test` and `transport_runtime_test`.
4. Transport topology/domain adoption: complete.
5. Clean linked/PS2 compile topology includes current Transport/RFB bridge source.
6. Transport dictionary: definition-level source inventory committed with `COVERAGE=COMPLETE`.
7. A001 CONFIG boundary: explicit caller-supplied Transport subprofile preserved; production producer remains A002-owned.

### Remaining Foreman integration

1. **V004 RFB dictionary — OPEN.** `src/rfb/SYMBOLS.md` still contains deleted `rfb_io.h` rows, obsolete `socket_fd` session/helper entries, stale socket-oriented descriptions, and lacks complete `src/rfb/bridge.*` inventory. Current RFB source consists of `bridge.c/.h`, `rfb.c/.h`, and `rfb_session.c/.h`; the old direct socket seam is absent.
2. **Generated dictionary portal — OPEN.** `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` still reports the stale RFB inventory and omits `src/transport`.
3. **Dictionary long/strict machine proof — PENDING_EXECUTION_SURFACE.** The canonical tool requires Universal Ctags and a repository checkout. No such successful execution is claimed on this connected surface.
4. **Canonical host unit execution — PENDING_EXECUTION_SURFACE.** Registration is complete; no canonical `make -C tests unit` PASS is claimed here.
5. **Project check — KNOWN_BLOCKED_BY_HISTORY_GOVERNANCE.** Even on an execution-capable checkout, current `work-log-check.py` will reject the two malformed immutable Validation filenames unless governance resolves the contradiction above. Other `scripts/check.sh` stages remain unexecuted here.
6. **PS2DEV link/reproducibility/exact ELF/PT_LOAD — PENDING_EXECUTION_SURFACE.**
7. **Hardware — HARDWARE_PENDING.**

## RFB dictionary reconciliation preflight

Foreman compared the stale dictionary against exact current RFB source authority and isolated the known migration delta rather than treating V004 as vague documentation debt.

Known stale definitions to remove include every entry for deleted `src/rfb/rfb_io.h` plus obsolete `socket_fd` definitions formerly owned by `read_exact`, `write_exact`, `read_bounded_text`, `discard_exact`, `pstvnc_rfb_session_start`, and the old session structure/prototype.

Current logical-bridge definitions that must be represented include the five `src/rfb/bridge.c` functions and their parameters/locals, the matching five `bridge.h` declarations plus prototype parameters and include guard, and the parser's current `quiesce_requested` local. Socket-oriented descriptions on surviving RFB session entries must be reconciled to logical-stream ownership.

The stale portal currently reports 339 RFB symbols. The exact known migration delta is consistent with a 345-entry current RFB inventory (339 minus 18 obsolete socket-seam/socket-parameter entries plus 24 current bridge/quiesce definitions), but **345 is a preflight expectation, not a canonical machine-proven count** until the regenerated dictionary passes the canonical definition discovery tool.

The current Transport dictionary is expected to contribute 422 entries to the regenerated portal from its committed definition-level inventory; this likewise remains subject to canonical portal/check execution.

## Validation disposition carried forward

- V003 remains formally OPEN / `WAIT_FOR_COHERENT_TRANCHE`; behavior is coherent by Reconstruction/Foreman evidence, but Validation owns promotion.
- V004 remains formally OPEN pending RFB dictionary/portal reconciliation and canonical dictionary evidence.
- V005 remains formally OPEN despite supported source shape and strengthened direct lifecycle fixtures; Validation owns executable disposition.
- A001 is not yet declared `VALIDATION_READY` because V004 and the canonical evidence/governance boundary remain unresolved.

## Reconstruction assignment status

RECONSTRUCTION_A_PACKET=NONE
RECONSTRUCTION_B_PACKET=NONE
INTERACTIVE_B_PACKET=NONE
REASON=NO_IDENTIFIED_A001_PRODUCT_BEHAVIOR_GAP;FOREMAN_INTEGRATION_AND_GOVERNANCE_RESOLUTION_REMAIN

Do not begin A002 and do not send Reconstruction workers back through C1-C8 as filler. If canonical execution later exposes a concrete product defect, issue a fresh narrowly scoped A001 packet against that evidence.

## Exact next Foreman pickup

Continue the RFB dictionary/portal reconciliation only through changes that can be reproduced from exact current source. Separately surface the malformed-Validation-log/checker contradiction to the owning governance/Validation lane without rewriting immutable history. Once V004 source integration is coherent and an execution-capable checkout is available, run the canonical unit/dictionary/build evidence. Do not represent `scripts/check.sh` as merely untried: its work-log stage is presently known blocked by committed filename/history governance until explicitly resolved.
