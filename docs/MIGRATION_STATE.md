# PS-to-VNC Migration State

This document is the human-readable companion to:

    runtime/MIGRATION_STATE.env

## Last completed stage

IMPORT

The independent PS-to-VNC repository has been created.

The exact D17AL-F8J2-B4A implementation and its dependency analysis are
preserved inside the successor repository as immutable reference material.

## Current stage

M0 — IN PROGRESS

M0 will establish the first writable, reproducible B4A-equivalent PS-to-VNC
working source and build authority.

## Current working source authority

NONE

The following location is historical reference material only:

    baseline/frozen-b4a/

It must not be edited as the working implementation.

## Frozen behavioral authority

Baseline DUT:

    D17AL-F8J2-B4A

Frozen source:

    baseline/frozen-b4a/ps2ip.c

Frozen source SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

Known tested ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Frozen dependency analysis:

    baseline/analysis/B4A-dependency-map/

Dependency-analysis SHA256SUMS manifest SHA256:

    075d68d9992332a32bdca54a32964363fb893071fdcc5c9e2dfc7fb26a2e1749

## Legacy repository boundary

Historical PS2VNC repository:

    /home/ps2/ps2vnc

That repository is read-only historical authority for successor development.

All new work belongs in:

    /home/ps2/PS-to-VNC

## Bootstrap preservation

Before the recovery-state machinery was added, the exact staged successor tree
was preserved as Git tree:

    2f7f3dcae878251bda1ad8169912df035ca34020

It was also preserved as:

    /home/ps2/ps2vnc-legacy-backups/PS-to-VNC-pre-recovery-staged-20260830-003258.tar

Archive SHA256:

    a8426ff95e82b06fe1dc9056d7dabeeca91b3bccb5d2711ac3d2d8c59ecd5251

## Current build state

No PS-to-VNC working build exists yet.

The tested B4A ELF is historical baseline evidence, not yet a successor build.

## Current documentation state

The successor documentation system is being completed before the first Git
commit.

Migration recoverability is a required part of every future stage closeout.

## Next action

M0:

Reproduce exact B4A as the first writable PS-to-VNC working build authority.

Do not begin M1 modular extraction until M0 has been completed, validated,
documented, and recorded in both human-readable and machine-readable migration
state.

## Conversation-limit recovery

A fresh conversation should begin with:

    START_HERE.md

Then run:

    scripts/resume-state.sh
    scripts/migration-check.sh
    scripts/docs-check.sh

The repository, not conversational memory, determines where development
resumes.

## Machine-state mirror

The following exact tokens mirror the canonical machine-readable migration
state and exist so consistency tooling does not have to infer status from
human prose.

    LAST_COMPLETE_STAGE=IMPORT
    CURRENT_STAGE=M0
    CURRENT_STAGE_STATUS=IN_PROGRESS
    CURRENT_WORKING_SOURCE=NONE
    NEXT_ACTION=M0C_reconstruct_and_fingerprint_historical_build_environment

The canonical machine-readable source remains:

    runtime/MIGRATION_STATE.env

## M0B checkpoint

The M0 reproduction contract is now defined at:

    docs/M0_BASELINE_CONTRACT.md

M0 has formally entered IN_PROGRESS state.

No writable source has been created yet.

Next operation:

    M0C_reconstruct_and_fingerprint_historical_build_environment
