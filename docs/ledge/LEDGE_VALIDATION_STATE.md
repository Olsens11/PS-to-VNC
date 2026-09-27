# Ledge Validation — Lane State

DOCUMENT=LEDGE_VALIDATION_STATE
STATE_REVISION=0007
RECORDED_AT=2026-09-27T05:56:12-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0006
BASED_ON_FOREMAN_STATE_REVISION=0086
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0007
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This state owns validation continuity only. It does not supersede global, audit, reconstruction, Foreman, architecture-overlay, Wire-decision, or governance authority.

## Current validation phase

`A001_A006_COMPLETE_CURRENT_LEDGE_PASS_MACHINE_SOURCE_HARDWARE_PENDING`

## Independent disposition

Validation independently refreshed and reviewed the complete current ledge source tranche rather than adopting the Foreman's completion conclusion. The stale A001-era Validation snapshot is superseded by this complete-current-source disposition.

VALIDATION_COMPLETE_CURRENT_LEDGE_MACHINE_SOURCE=PASS
VALIDATION_A001_MACHINE_SOURCE=PASS
VALIDATION_A002_MACHINE_SOURCE=PASS
VALIDATION_A003_MACHINE_SOURCE=PASS
VALIDATION_A004_MACHINE_SOURCE=PASS
VALIDATION_A005_MACHINE_SOURCE=PASS
VALIDATION_A006_MACHINE_SOURCE=PASS
RECONSTRUCTION_OWNED_SOURCE_DEFECTS_OPEN=NONE
HARDWARE_STATUS=HARDWARE_PENDING

The final reconstructed product-source authority is `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`. Validation began from branch ledger HEAD `095e44b1da60fab13a22bd0601818911a72f57e6`; comparison proves the three commits after the final product-source authority modify only Foreman/Reconstruction documentation and immutable logs, not product source.

## A001 — Transport / RFB

PASS_MACHINE_SOURCE.

Independent source review confirms one Transport-owned physical Wire session and sole receiver, with RFB using logical Transport-owned read/write/quiesce surfaces rather than a physical descriptor. RFB credit is returned from actual logical consumption, residual bytes remain separately owned, activity publication is explicit, and quiescence preserves accepted-write drain and commit/complete ordering.

Transport terminality closes outbound admission before receiver reclamation, drains already-admitted outbound submitters, distinguishes early terminal publication from the final receiver no-touch completion fence, and permits storage release only from the irreversible PROVEN completion outcome. Descriptor/ticket authority is cleared only after runtime release. Fatal convergence remains Transport-owned.

## A002 — Configuration / AUDIO / media clock

PASS_MACHINE_SOURCE.

Selected runtime profiles are Configuration-owned typed projections with provenance from checked generated/canonical values; malformed, duplicate, unknown, missing, or otherwise invalid profile content is rejected rather than repaired with invented defaults. Product-action configuration likewise has explicit zero/no-binding fallback rather than a hidden physical default.

AUDIO owns its session worker/playback lifecycle, waits for real AUDSRV capacity before submission, reports consumption only after successful playback submission, and requires owner-proven completion/join before local reclamation. AUDSRV remains resident rather than being quit per Wire session. The common media clock is a distinct value/lifecycle owner and is armed by the first physically presented MPEG frame, not by decode or queue availability.

## A003 — Wire / provider / MPEG generation

PASS_MACHINE_SOURCE.

Pi Wire owns Q4 establishment, private session identity, all physical recv/send sequencing, and sequential session replacement. RFB provider attachment is lazy, provider-local, and reports one typed terminal cause through the sole Wire sender; provider failure does not create a second physical owner. Replacement Wire sessions construct fresh attachment/generation state.

MPEG START is the first producer edge, is exact-session/exact-generation fenced, and does not arise from CREDIT alone. Retirement closes emission admission, waits for in-flight emission ownership, requires actual producer retirement, and cannot convert timeout/failure into completion. AUDIO/MPEG/RFB ordinary product composition injects mechanisms without moving Wire ownership or inventing session identity.

## A004 — Presentation / MPEG calibration

PASS_MACHINE_SOURCE.

Presentation remains the single owner of physical MPEG/RFB visual ownership state. DESKTOP CALIBRATION and MPEG CALIBRATION are separate lineages and are not used as substitutes for one another. MPEG base geometry, inner matte, and suppression rectangle remain distinct validated values.

The current Q7 authority supersedes the older immutable audit wording that said MPEG must retire completely before RFB restoration begins. Current source follows Q7: after RETIRE serialization it releases RFB suppression/freeze so the full-refresh restoration can begin underneath still-visible retiring MPEG; module-owned MPEG retirement then completes; only after fresh RFB presentation evidence is recorded does Presentation seal retirement and physically reveal RFB. First physical MPEG presentation, not decode/upload alone, promotes MPEG ownership and arms the common clock.

## A005 — Interaction / input / management snapshot

PASS_MACHINE_SOURCE.

The input worker owns libpad polling and semantic event publication but does not execute product actions, write RFB, or mutate presentation state. Application/main owns semantic product-action execution. Local foreground transitions preserve physical polling while quarantining/rebasing mouse interpretation, and worker shutdown refuses resource reclamation unless cooperative thread dormancy is actually observed.

Desired product-action bindings are acquired once through the bounded Management read surface, parsed by Configuration, and installed as an immutable-by-convention Application snapshot. Retrieval/parse failure yields explicit zero bindings. No unearned persistence/editor/reload policy is inferred by this Validation pass.

## A006 — Application orchestration / shutdown

PASS_MACHINE_SOURCE.

Application owns cross-domain sequencing. For media-bearing abnormal teardown it first proves Input retirement, establishes Transport begin-abort before allowing MPEG/AUDIO cleanup to reclaim Transport-backed ownership, services MPEG and AUDIO until each owner reports real abort readiness, releases the media-clock binding only afterward, and closes retained Transport storage last.

R43's attempt-local retained-session fact is published only after successful `pstvnc_transport_session_begin_abort()`. Cleanup re-entry after MPEG or AUDIO errors and pending states reuses that already-established retained authority instead of invoking begin-abort again. Failed begin-abort does not fabricate retained authority. A replacement attempt initializes the fact false and therefore establishes its own fresh abort boundary. The no-media path retains the one-shot Transport abort operation.

## Cross-family disposition

No unit-local green result was treated as sufficient by itself. Source and contract were reviewed across:

- Wire/Transport terminality versus RFB provider and parser ownership;
- MPEG generation retirement versus Presentation restoration/reveal;
- AUDIO first-presentation readiness versus common-clock arming;
- Input dormancy versus Application media teardown;
- retained Transport storage versus MPEG/AUDIO cleanup;
- provider replacement versus fresh attempt/session state.

No unresolved ownership/order contradiction was found in the reconstructed A001-A006 source.

## Exact machine evidence consumed

Canonical GitHub Actions run `36287568567` is SUCCESS at exact source-equivalent ledger HEAD `095e44b1da60fab13a22bd0601818911a72f57e6`.

Verified successful jobs/evidence include:

- host-unit, including Application R42/R43 orchestration, Transport runtime, RFB provider/attachment, MPEG generation/product, AUDIO producer/product, calibration and activation coverage;
- `scripts/check.sh` / project-check;
- source topology and generated-profile checks;
- `python3 scripts/source-dictionary.py check --long --require-complete --strict`;
- pinned clean PS2 compile;
- clean linked PS2 build and repeated current-source ELF/PT_LOAD reproducibility.

Exact linked identity:

- `ELF_PRISTINE_SHA256=993655ccffc430347281291c1fa917e3b22ce77f17407a99edbc920b71a65a2a`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`;
- `PT_LOAD_BYTES=556180`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The current-head linked artifact is preserved as Actions artifact ID `10921200508` (legacy artifact name `ledge-a001-unqualified-clean-elf`); the legacy name does not narrow the source content of the exact current-head build.

## Hardware boundary

`HARDWARE_PENDING`.

This Validation pass is machine/source qualification only. It does not declare the exact ELF/PT_LOAD physically qualified on a PS2. Operator-backed hardware evidence must be bound to the exact identity above under the repository's hardware-qualification procedure. No physical success was inferred from CI, host tests, compile/link success, source review, or historical DUT evidence.

## Exact next pickup

The complete current ledge A001-A006 machine/source tranche is independently closed. The next conversation is final ledge hardware qualification against the exact linked identity above. Do not begin optional future features and do not describe the tranche as physically qualified until that separate hardware evidence exists.
