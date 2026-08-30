# PS-to-VNC Project State

## Current state

Repository bootstrap is in progress.

No modular working source has yet been promoted.

Current immutable behavioral reference:

`D17AL-F8J2-B4A`

Frozen source:

`baseline/frozen-b4a/ps2ip.c`

SHA256:

`67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac`

Source size:

`18,938 lines`

Frozen dependency analysis:

`baseline/analysis/B4A-dependency-map/`

Analysis manifest SHA256:

`075d68d9992332a32bdca54a32964363fb893071fdcc5c9e2dfc7fb26a2e1749`

## Development authority

Current working source authority:

**NONE**

The first source-development stage is M0.

M0 will establish a reproducible B4A-equivalent working build inside
PS-to-VNC without modifying the immutable baseline.

## Documentation state

PS-to-VNC documentation is living documentation.

Code movement and documentation movement are one operation.

Current source location authority is:

`docs/reference/FILE_AND_SERVICE_MAP.md`

## Durable recovery authority

Conversation state is not project authority.

Canonical machine-readable migration state:

`runtime/MIGRATION_STATE.env`

Canonical human-readable migration state:

`docs/MIGRATION_STATE.md`

Mandatory recovery entry point:

`START_HERE.md`

Recovery command:

`scripts/resume-state.sh`

Consistency gate:

`scripts/migration-check.sh`

The next source-changing operation must establish the expected migration state
before mutation.

## M0 reproduction contract

M0 is now in progress.

The governing build/reproduction contract is:

`docs/M0_BASELINE_CONTRACT.md`

No writable source authority has been created yet.

The next operation is read-only reconstruction and fingerprinting of the
historical build environment before `working/b4a/` is populated.

## M0 Outcome A reproduction

PS-to-VNC has independently rebuilt the writable B4A authority to the exact
historical ELF bytes.

Result:

    OUTCOME_A_BYTE_EXACT_ELF

The sole non-identical intermediate was the slim-LTO gsHires object container.
Its LTO payloads are identical; only generated section-name metadata differs.

The final executable is byte-identical, so this does not constitute a new DUT
identity.

See:

    docs/M0_BUILD_RESULT.md
