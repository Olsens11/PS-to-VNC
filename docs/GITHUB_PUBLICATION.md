# GitHub Publication Authority

## Status

PS-to-VNC has been published to its own independent GitHub repository.

Repository:

    Olsens11/PS-to-VNC

Visibility:

    PRIVATE

Default branch:

    main

Remote name:

    origin

Remote URL:

    https://github.com/Olsens11/PS-to-VNC.git

## Publication point

The first GitHub publication occurred after M0 was completed and after M1A
selected and committed the first mechanical extraction boundary.

At publication:

- M0 was complete;
- the exact B4A reproduction authority was committed;
- M1A was complete;
- no M1 implementation source mutation had occurred;
- the first M1 extraction plan was already documented;
- M1B had not begun.

This creates a durable remote checkpoint immediately before the successor
starts changing the historical monolithic implementation.

## Visibility policy

The repository was deliberately created PRIVATE.

The project is still under active architectural migration and is not being
presented as ready for public use or release.

Changing repository visibility is a separate future decision.

It must not happen implicitly as part of an unrelated development task.

## Independence from legacy PS2VNC

Legacy repository:

    Olsens11/PS2VNC

Legacy local root:

    /home/ps2/ps2vnc

Successor repository:

    Olsens11/PS-to-VNC

Successor local root:

    /home/ps2/PS-to-VNC

These are separate Git repositories and separate GitHub repositories.

The successor is not a renamed remote for the legacy project.

Publishing PS-to-VNC must never push or mutate the legacy PS2VNC repository.

## Non-affiliation

PS-to-VNC is an independent homebrew project and is not affiliated with or
endorsed by Sony Interactive Entertainment.

## M1 consequence

The publication gate required by M1A is satisfied.

M1B may now begin the documented mechanical extraction of:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

into the transitional module:

    working/b4a/ps2vnc_config_text.c

with public header:

    working/b4a/ps2vnc_config_text.h

M1B remains governed by:

    docs/M1A_EXTRACTION_BOUNDARY.md
    docs/M1_EXTRACTION_CONTRACT.md
