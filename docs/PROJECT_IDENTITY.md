# PS-to-VNC Project Identity

## Name

**PS-to-VNC**

PS-to-VNC is an independent homebrew project and is not affiliated with or
endorsed by Sony Interactive Entertainment.

Newly designed modular APIs should normally use the internal prefix:

`pstvnc_`

Historical identifiers are not renamed retroactively.

## Repository roles

Historical PS2VNC repository:

`/home/ps2/ps2vnc`

Historical separation commit:

`5781f4f7105ee290fa77819d7b9ec96e833bd2b8`

Current PS-to-VNC repository:

`/home/ps2/PS-to-VNC`

The repositories have separate Git metadata.

PS-to-VNC begins with no configured Git remote.

## Historical preservation

The legacy repository remains the authority for its historical implementation,
test campaigns, forensic evidence and original documentation.

Complete legacy Git refs/history are additionally preserved in:

`/home/ps2/ps2vnc-legacy-backups/ps2vnc-legacy-allrefs-20260830-002434.bundle`

Bundle SHA256:

`7f018a3623c5cdd7e7ed30bfbc3f00040b632735fc83f92d49c5a0c672bea85a`

## Successor philosophy

PS-to-VNC does not reorganize the legacy repository.

The exact proven B4A implementation is imported only as immutable baseline
reference material.

A clean modular working implementation will be built beside that reference in
controlled stages.
