# Clean Raspberry Pi Companion

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    BASELINE_CAPTURE=AWAITING_FIRST_BOOT
    PRODUCT_MUTATIONS=NONE
    DEPENDENCY_LEDGER=ACTIVE

This directory is the current authority for building the clean Raspberry Pi
companion from a normal supported Raspberry Pi OS installation.

The governing rule is:

> Every product-significant deviation from the virgin supported OS must be
> declared, reproducible, and justified.

The old PS2VNC Pi remains forensic/reference authority. It is not an installation
template for this machine.

## Before the first product change

Capture `BASELINE.md` from the newly imaged Pi before installing packages,
copying old configuration, adding services, changing the private Ethernet
interface, or creating PS-to-VNC runtime state.

The baseline records what the OS supplied on its own. A package/configuration
that is already present may later be classified `OS_BASE`; it must not be
silently treated as a project installation step.

## Dependency states

`DEPENDENCY_LEDGER.md` uses the project states established in
`docs/PROJECT_INTENT.md`:

- `EVALUATING`
- `ADOPTED_RUNTIME`
- `ADOPTED_BUILD`
- `ADOPTED_OPTIONAL`
- `OS_BASE`
- `REJECTED`
- `REPLACED`

The reason for a state is part of the record. Rejected and replaced experiments
remain documented.

## Configuration rule

Package installation is only one kind of dependency. Record meaningful
configuration changes too, including:

- interface/address/routing changes;
- NetworkManager/systemd units;
- VNC desktop/session configuration;
- traffic-control settings;
- management-service state paths and permissions;
- desktop/window-manager/panel setup;
- Samba/file-service configuration when adopted;
- boot/sysctl/firewall changes;
- project users/groups or privilege changes.

At a reproducibility milestone, a fresh supported OS must be transformable into
the required companion state using tracked project definitions rather than
operator memory.

## Current next step

Capture the virgin first-boot OS/hardware/package/service/network baseline into
`BASELINE.md` and classify what is already supplied by the image before any
PS-to-VNC-specific mutation.
