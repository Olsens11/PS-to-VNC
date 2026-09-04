# PS-to-VNC TestKit Operator Guide

This is the canonical operator-facing guide to `scripts/testkit/`.

Use this file to answer:

- what TestKit tools exist;
- which tool owns a particular development operation;
- whether a tool is generic, Issue-specific, regression-only, forensic, or
  retained historical-state machinery;
- whether it can contact the PS2 or mutate project authority;
- how the tool is normally invoked;
- which self-test proves the mechanism before operational use.

For implementation-level symbol ownership and function/constant descriptions,
see [`SYMBOLS.md`](SYMBOLS.md).

For project-wide policy about when to use saved tooling rather than bespoke
commands, see [`../../docs/development/tooling.md`](../../docs/development/tooling.md).

## Core rule

Use the narrowest current tool that owns the job.

Do not recreate a routine TestKit procedure in an ad-hoc shell packet when a
canonical tool already owns that operation.

Do not use a historical or Issue-specific tool merely because its filename or
behavior resembles the current task.

A tool failure is development-infrastructure evidence. It is not evidence that
the DUT itself failed.

## Lifecycle labels

The catalog below uses these labels:

- **CURRENT / GENERIC** — reusable current-development mechanism.
- **CURRENT / SUPPORT** — current low-level mechanism normally consumed by a
  higher-level current tool.
- **CURRENT / REGRESSION** — non-hardware self-test or compatibility proof.
- **ISSUE #7 / OPERATOR** — current tool whose policy belongs specifically to
  Issue #7 qualification.
- **ISSUE #7 / INTERNAL** — helper owned by the Issue #7 apparatus; normally
  not invoked manually.
- **ISSUE #7 / REGRESSION** — Issue #7-specific non-hardware regression.
- **FORENSIC / REFERENCE** — analysis or compatibility mechanism retained to
  prove a historical contract; not a normal development operation.
- **M4 STATE / RETAINED** — preserved M4 authority-state mechanism. It is not
  part of ordinary clean-reconstruction work and should be used only when the
  explicit task is to inspect, validate, or transition preserved M4 authority.

## Routine workflow map

A normal current hardware-development flow is conceptually:

    build exact DUT
        |
        v
    inspect PT_LOAD when required
        |
        v
    prepare/stamp hardware ELF when runtime identity is required
        |
        v
    create any qualification-specific manifest/evidence plan
        |
        v
    arm qualification-specific observers when the test requires them
        |
        v
    deploy exact ELF with deploy-elf.py
        |
        v
    operator launches/runs DUT on the PS2
        |
        v
    stop/seal observers
        |
        v
    evaluate machine evidence separately from physical/operator judgment

Not every experiment requires every stage.

The important ownership boundary is:

- **deployment mechanics are generic**;
- **qualification policy is not**.

`deploy-elf.py` does not decide whether an ELF is qualified, whether observers
must be armed, what branch should be active, or what a successful physical test
means.

## Generic deployment

Canonical implementation:

    scripts/testkit/deploy-elf.py

Normal live use from the canonical tooling worktree against another DUT
worktree:

    python3 scripts/testkit/deploy-elf.py \
        --repo <DUT_WORKTREE> \
        --elf <ELF_PATH_RELATIVE_TO_DUT> \
        --test-id <TEST_ID> \
        --expected-sha256 <ELF_SHA256> \
        --expected-bytes <ELF_BYTES> \
        --evidence <EVIDENCE_PATH_RELATIVE_TO_DUT> \
        --operator-authorized

Preflight without FTP contact:

    python3 scripts/testkit/deploy-elf.py \
        --repo <DUT_WORKTREE> \
        --elf <ELF_PATH_RELATIVE_TO_DUT> \
        --test-id <TEST_ID> \
        --expected-sha256 <ELF_SHA256> \
        --expected-bytes <ELF_BYTES> \
        --evidence <EVIDENCE_PATH_RELATIVE_TO_DUT> \
        --dry-run

Normal deployment always writes the same bytes to:

1. a deterministic unique archival filename under `/mass/0`; and
2. `/mass/0/PS2VNC.ELF`, the stable launch target.

Both copies are downloaded after upload and must match the local ELF SHA256 and
byte count.

Live FTP requires `--operator-authorized`.

`--dry-run` performs no FTP contact.

Deployment is not hardware execution and does not itself qualify the ELF.

Run:

    python3 scripts/testkit/deploy-elf.py --help

for the complete current command-line interface.

Regression:

    ./scripts/testkit/deploy-elf-self-test.sh

The regression uses fake FTP only and must not contact the real PS2.

## Hardware ELF identity preparation

Preferred high-level preparation:

    ./scripts/testkit/prepare-hardware-elf.sh \
        PRISTINE_ELF \
        TEST_ID \
        OUTPUT_ELF

The output path must not already exist.

The tool copies the pristine ELF, stamps its embedded runtime identity, verifies
that identity, checks deterministic reproduction, and records pristine/stamped
PT_LOAD identities.

Capture its stdout when the preparation record is needed by a later
qualification step:

    ./scripts/testkit/prepare-hardware-elf.sh \
        build/example/Pristine.ELF \
        EXAMPLE-HW1 \
        build/example/Example-HW1.ELF \
        > build/example/preparation.env

Lower-level identity tools exist for direct inspection and regression work, but
routine hardware preparation should normally use `prepare-hardware-elf.sh`.

## PT_LOAD fingerprinting

Use:

    ./scripts/testkit/pt-load-fingerprint.sh ELF

to report the concatenated PT_LOAD SHA256 and byte count.

Optionally preserve the concatenated load bytes:

    ./scripts/testkit/pt-load-fingerprint.sh ELF OUTPUT_BIN

A changed PT_LOAD remains a hardware-relevant change unless current repository
authority explicitly says otherwise.

## Issue #7 qualification flow

Issue #7 owns its qualification policy separately from generic deployment.

Typical sequence:

1. Build the canonical Issue #7 clean linked ELF.
2. Prepare a stamped hardware ELF with `prepare-hardware-elf.sh`.
3. Create the Issue #7 DUT manifest:

       python3 scripts/testkit/issue7-dut-manifest.py \
           PREPARATION_LOG \
           OUTPUT_MANIFEST

4. Arm observers:

       ./scripts/testkit/issue7-arm-observers.sh DUT_MANIFEST

5. Invoke the generic deployer with the exact manifest-derived ELF identity and
   write its evidence to the armed run's `deployment.json`.
6. Launch and physically exercise the DUT.
7. Stop and seal observers:

       ./scripts/testkit/issue7-stop-observers.sh RUN

8. Evaluate machine evidence:

       python3 scripts/testkit/issue7-result.py RUN

Physical/operator judgment remains separate from the machine evaluator.

The Issue #7 manifest tool intentionally binds to the Issue #7 branch and its
clean build authority. Do not use it as a generic manifest generator.

## M4 retained state tooling

M4 tools remain because the repository preserves M4 authority/history and
occasionally needs to validate or reproduce those state transitions.

They are not the normal path for current clean reconstruction.

Do not invoke an M4 activation or supersession tool merely to make current
state "look consistent." Use them only when the task specifically concerns the
preserved M4 authority contract.

The project-wide policy and exact M4 authority surfaces are documented in:

    docs/development/tooling.md

## Complete file catalog

This catalog is exhaustive for regular files directly inside
`scripts/testkit/`.

`scripts/testkit/self-test.sh` mechanically checks this section. Adding,
removing, or renaming a file without updating the catalog is a TestKit
regression failure.

<!-- TESTKIT_CATALOG_BEGIN -->

| File | Lifecycle | Purpose | Normal use |
|---|---|---|---|
| `README.md` | CURRENT / GENERIC | Canonical operator-facing TestKit guide and exhaustive file catalog. | Read this first when choosing or invoking a TestKit tool. |
| `SYMBOLS.md` | CURRENT / SUPPORT | Implementation dictionary for deliberately adopted clean TestKit symbols, ownership, scope, and purpose. | Read when navigating or modifying TestKit implementation; it is not the operator manual. |
| `activate-m4-hardware-checkpoint.py` | M4 STATE / RETAINED | Applies one manifest-driven M4 hardware-pending authority transition across the preserved M4 state surfaces without granting hardware qualification. | `python3 scripts/testkit/activate-m4-hardware-checkpoint.py MANIFEST [--root REPO]`; only for explicit M4 authority work. |
| `activation-self-test.py` | CURRENT / REGRESSION | Exercises M4 checkpoint activation on disposable copies and proves idempotence/state coherence. | `./scripts/testkit/activation-self-test.py`; no PS2 contact. |
| `deploy-elf-self-test.sh` | CURRENT / REGRESSION | Proves generic repository selection, dry-run isolation, authorization, collision refusal, dual upload/readback, and evidence creation with fake FTP. | `./scripts/testkit/deploy-elf-self-test.sh`; never a real deployment. |
| `deploy-elf.py` | CURRENT / GENERIC | Deploys one exact caller-selected ELF to unique archival and rolling `/mass/0/PS2VNC.ELF` targets, reads both back, and records generic deployment evidence. | Use `--repo`, `--elf`, `--test-id`, optional expected identity/evidence arguments, plus `--dry-run` or `--operator-authorized`; `--help` is authoritative. |
| `elf-identity.py` | CURRENT / SUPPORT | Implements the embedded PS2VNC runtime identity stamp/verify contract. | `python3 scripts/testkit/elf-identity.py stamp ELF TEST_ID` or `verify ELF TEST_ID IDENTITY_SHA256`; normally reached through wrapper/preparation tools. |
| `identity-digest-forensics.py` | FORENSIC / REFERENCE | Re-evaluates plausible historical identity-digest formulas against sealed fixtures. | `./scripts/testkit/identity-digest-forensics.py`; forensic/compatibility work, not routine DUT preparation. |
| `identity-runtime-message-self-test.sh` | CURRENT / REGRESSION | Proves runtime identity datagram serialization boundaries and malformed-input rejection. | `./scripts/testkit/identity-runtime-message-self-test.sh`; no hardware contact. |
| `issue7-apparatus-common.sh` | ISSUE #7 / INTERNAL | Shared Issue #7 manifest validation, observer ownership, run-path, and apparatus helper functions. | Source from Issue #7 apparatus scripts; do not invoke directly. |
| `issue7-apparatus-self-test.sh` | ISSUE #7 / REGRESSION | Exercises Issue #7 UDP/evidence ordering and apparatus logic with loopback-only traffic. | `./scripts/testkit/issue7-apparatus-self-test.sh`; no FTP, sudo, or PS2 contact. |
| `issue7-arm-observers.sh` | ISSUE #7 / OPERATOR | Validates an Issue #7 manifest and starts the owned UDP 5999 observer plus bounded PS2-facing packet capture. | `./scripts/testkit/issue7-arm-observers.sh DUT_MANIFEST`; used immediately before an Issue #7 hardware run. |
| `issue7-dut-manifest-self-test.sh` | ISSUE #7 / REGRESSION | Proves deterministic Issue #7 DUT-manifest creation against the canonical linked clean ELF and fail-closed tamper behavior. | `./scripts/testkit/issue7-dut-manifest-self-test.sh` after the canonical Issue #7 linked reproducibility build; branch/CI-specific. |
| `issue7-dut-manifest.py` | ISSUE #7 / OPERATOR | Converts passing hardware-ELF preparation evidence into the exact pre-hardware Issue #7 DUT manifest. | `python3 scripts/testkit/issue7-dut-manifest.py PREPARATION_LOG OUTPUT_MANIFEST`; not a generic manifest generator. |
| `issue7-result.py` | ISSUE #7 / OPERATOR | Verifies sealed Issue #7 raw evidence, deployment readback identity, ordered startup diagnostics, and required PS2-facing packet evidence. | `python3 scripts/testkit/issue7-result.py RUN`; machine result only, not physical qualification. |
| `issue7-stop-observers.sh` | ISSUE #7 / OPERATOR | Stops only observers proven to belong to the selected Issue #7 run and SHA256-seals the raw evidence before interpretation. | `./scripts/testkit/issue7-stop-observers.sh RUN`. |
| `issue7-udp-observer.py` | ISSUE #7 / INTERNAL | Receives Issue #7 runtime UDP diagnostics and appends timestamped byte-preserving JSONL records. | Normally started by `issue7-arm-observers.sh`; direct form is `--log PATH --bind IP --port PORT --owner-token TOKEN`. |
| `m4-authority-state-check.py` | M4 STATE / RETAINED | Checks preserved M4 migration/source authority coherence and hardware-qualified versus hardware-pending state rules. | `python3 scripts/testkit/m4-authority-state-check.py MIGRATION_STATE M4_SOURCE_AUTHORITY`; explicit M4 state work only. |
| `m4-authority-state-self-test.py` | CURRENT / REGRESSION | Exercises the retained M4 authority-state checker across its supported state matrix. | `./scripts/testkit/m4-authority-state-self-test.py`; no PS2 contact. |
| `pending-supersession-self-test.py` | M4 STATE / RETAINED | Validates a proposed pending-to-pending M4 checkpoint replacement on disposable authority copies before live activation. | `./scripts/testkit/pending-supersession-self-test.py ACTIVATION.env`; explicit M4 supersession work only. |
| `prepare-hardware-elf.sh` | CURRENT / GENERIC | Creates one reproducible identity-stamped hardware ELF and proves stamp/verify/PT_LOAD relationships without executing it. | `./scripts/testkit/prepare-hardware-elf.sh PRISTINE_ELF TEST_ID OUTPUT_ELF`. |
| `pt-load-fingerprint.sh` | CURRENT / GENERIC | Hashes the exact file bytes belonging to ELF PT_LOAD segments and optionally writes their concatenation. | `./scripts/testkit/pt-load-fingerprint.sh ELF [OUTPUT_BIN]`. |
| `self-test.sh` | CURRENT / REGRESSION | Top-level non-hardware TestKit regression covering identity preparation/compatibility, generic deployment fake-FTP behavior, documentation inventory, and Issue #7 apparatus logic. | `./scripts/testkit/self-test.sh`; run after TestKit changes and before first live use. |
| `stamp-elf-identity.sh` | CURRENT / SUPPORT | Thin shell entry point for in-place runtime identity stamping. | `./scripts/testkit/stamp-elf-identity.sh ELF TEST_ID`; normally prefer `prepare-hardware-elf.sh`. |
| `successor-identity-compat-self-test.py` | CURRENT / REGRESSION | Proves the successor identity implementation reproduces sealed historical stamped fixtures byte-for-byte and fails closed on invalid restamping/IDs. | `./scripts/testkit/successor-identity-compat-self-test.py`; no hardware contact. |
| `verify-elf-identity.sh` | CURRENT / SUPPORT | Thin shell entry point for verifying one stamped ELF's test ID and embedded identity digest. | `./scripts/testkit/verify-elf-identity.sh ELF TEST_ID IDENTITY_SHA256`. |

<!-- TESTKIT_CATALOG_END -->

## Which tool should I choose?

For **put this exact ELF on the PS2**:

    deploy-elf.py

For **prove what would be deployed without contacting the PS2**:

    deploy-elf.py --dry-run

For **prepare a runtime-identity-stamped ELF**:

    prepare-hardware-elf.sh

For **compare hardware-relevant PT_LOAD bytes**:

    pt-load-fingerprint.sh

For **low-level identity stamping or verification**:

    stamp-elf-identity.sh
    verify-elf-identity.sh
    elf-identity.py

For **run all ordinary non-hardware TestKit regressions**:

    self-test.sh

For **Issue #7 hardware qualification**:

    issue7-dut-manifest.py
    issue7-arm-observers.sh
    deploy-elf.py
    issue7-stop-observers.sh
    issue7-result.py

For **historical identity-contract investigation**:

    identity-digest-forensics.py

For **preserved M4 authority-state work**:

    activate-m4-hardware-checkpoint.py
    m4-authority-state-check.py
    pending-supersession-self-test.py

If the task does not explicitly concern preserved M4 authority, those M4 tools
are almost certainly not the right starting point.

## Hardware-contact summary

The generic deployer's live mode is the routine TestKit operation that writes
to PS2 mass storage over FTP.

`issue7-arm-observers.sh` and `issue7-stop-observers.sh` manage hardware-run
observation infrastructure but do not replace the DUT ELF themselves.

Self-tests are designed to avoid real PS2 contact. In particular,
`deploy-elf-self-test.sh` uses fake FTP and `issue7-apparatus-self-test.sh` uses
loopback UDP.

Preparing, stamping, fingerprinting, manifest generation, result analysis, and
M4 disposable self-tests do not by themselves execute the DUT on the PS2.

## Documentation maintenance contract

Whenever a regular file is added, removed, or renamed directly under
`scripts/testkit/`:

1. update the complete file catalog in this README;
2. classify the file's lifecycle accurately;
3. state its purpose and normal invocation;
4. say explicitly when a specialized/historical tool is not appropriate for
   routine current work;
5. update `docs/development/tooling.md` if project-wide tooling policy or
   ownership changed;
6. update `SYMBOLS.md` when clean implementation symbols are added or changed;
7. run:

       ./scripts/testkit/self-test.sh
       ./scripts/check.sh

The TestKit self-test mechanically verifies that the catalog contains exactly
the regular files currently present in this directory.

That mechanical coverage check answers **whether every tool is documented**.
Semantic review still answers **whether the description is correct**.
