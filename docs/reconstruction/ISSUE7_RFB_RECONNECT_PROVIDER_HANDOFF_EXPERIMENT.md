# Issue #7 RFB reconnect and provider-handoff experiment

## Status

**EXPERIMENT COMPLETE / PRESERVED**

This document records the successful Issue #7 experiment that tested rebuilding
an RFB session after explicit transport loss and then used the same mechanism
to switch between two Pi-side RFB providers without relaunching the PS2 ELF.

This is experimental evidence, not yet production recovery policy. Future
production recovery work is tracked by GitHub Issue #25:

**[Recovery] Restore lost RFB sessions and support provider handoff**

## Purpose

The experiment asked two related questions:

1. Can the clean PS2 client lose an established RFB TCP session, reconnect to
   the same stable Pi endpoint, establish a fresh RFB session, reacquire an
   authoritative framebuffer, and continue without relaunching the ELF?

2. Can the same mechanism handle deliberate replacement of one RFB provider
   with another behind that stable endpoint, without provider-specific PS2
   logic?

Under the tested conditions, the answer to both was **yes**.

## Source and DUT identity

Relevant Git identities:

    clean Issue #7 pre-reconnect base:
    d94c9280035e288ccbec692ea21e89d3ffb4ffec

    reconnect implementation:
    56e1d00bea70cfa8424a5c4104bebbd27990f709

    preserved provider-handoff apparatus:
    ff6e6f363dbd0431ceb4061f26cd9f9963a0c86c

Experimental reconnect ELF:

    SHA256=
    8b7040b753f421e6475712476f07576350e44301d0f8db9aa2a1f85616b93801

The experiment-specific ELF did not replace the normal rolling Issue #7 DUT.

Later compatibility testing used the ordinary pre-reconnect DUT:

    identity=ISSUE7-RAW480P-HW2
    source=d94c9280035e288ccbec692ea21e89d3ffb4ffec
    SHA256=52325b80df604b14baa3ca1a5a2d16469b5651f77a80e660168e3ae19aafd2ca

## Reconnect behavior tested

On explicit EOF, reset, socket failure, or RFB I/O failure, the experimental
client:

1. discarded the invalid RFB transport/session;
2. kept Ethernet/IOP networking alive;
3. kept GS/display, input, local UI, and process lifetime intact;
4. retried the stable Pi endpoint `192.168.50.1:5900`;
5. performed a complete new RFB handshake;
6. required a new non-incremental full framebuffer before restoring framebuffer
   authority;
7. resumed ordinary incremental service only after that authority was restored.

A failed connection attempt waited approximately 500 ms before retrying.

Malformed or semantically invalid RFB data remained fail-closed. The experiment
did not reinterpret protocol corruption as ordinary connection loss.

Silent starvation/stalls deliberately received no automatic watchdog.

## Stable Pi endpoint

The PS2 continued to address one durable endpoint:

    192.168.50.1:5900
    ps-to-vnc-rfb.socket

The provider behind that endpoint remained replaceable.

Canonical operation selected:

    ps-to-vnc-rfb-tigervnc.service

The alternate experiment used the existing Raspberry Pi OS Wayland desktop and
a preserved w0vncserver binary:

    /home/ps2/.local/lib/ps-to-vnc/w0vncserver

    SHA256=
    76dd46e40a3b1d82da8a86b3bff622b2f6c9c703c2d1ed38dee81c660d0ef804

w0vncserver listened on:

    127.0.0.1:5901

A temporary `systemd-socket-proxyd` service adapted the inherited durable
socket to that loopback server.

## Provider-handoff sequence

The tested provider sequence was:

    Xtigervnc
        ->
    systemd socket proxy -> w0vncserver / existing Wayland desktop
        ->
    Xtigervnc

The Wayland `NOOP-1` output was temporarily changed to the exact test geometry:

    704x462

Teardown restored the observed stock Raspberry Pi OS state:

    1920x1080
    position 0,0
    scale 1

The apparatus did not replace or modify stock Raspberry Pi OS kanshi policy.

## Hardware result

Switching the durable socket provider intentionally destroyed the old TCP/RFB
connection. The experimental PS2 client then created a genuinely fresh
connection and negotiated a new RFB session.

Observed PS2 TCP source-port progression included:

    52452 -> 52454 -> 52456

and a later connection using:

    52458

The exact port numbers are not architectural constants. Their significance is
that fresh TCP sessions were created while the same PS2 application continued
running.

Machine evidence and physical display observation passed for:

    Xtigervnc -> Wayland/w0 -> Xtigervnc

No ELF relaunch was required.

A controlled RFB outage of approximately four seconds also recovered through
the reconnect path.

After each successful reconnect, the client performed fresh RFB negotiation and
reacquired a complete authoritative framebuffer before incremental updates
resumed.

## Teardown and cold-reboot result

The temporary apparatus was removable.

Teardown verified:

- the temporary socket override was absent;
- the temporary proxy unit was absent;
- the transient w0 backend was stopped and collected;
- port 5901 was no longer listening;
- canonical Xtigervnc socket activation was restored;
- Wayland returned to stock 1920x1080;
- the exact w0vncserver binary remained intentionally installed.

A subsequent Pi cold reboot proved that none of the temporary proxy, socket
override, or transient backend state returned.

The ordinary pre-reconnect `ISSUE7-RAW480P-HW2` ELF then connected normally to
fresh socket-activated Xtigervnc and displayed the physical test stimulus.

Therefore the Pi did not become dependent on the experimental PS2 reconnect
patch.

## Preserved apparatus

The exact repeatable Pi-side scripts are preserved in:

`reference/pi-experiments/issue7-rfb-reconnect-provider-handoff/`

That directory contains:

- the operator README;
- the complete human-readable `SYMBOLS.md`;
- setup;
- Xtigervnc-to-Wayland switch;
- Wayland-to-Xtigervnc switch;
- teardown;
- offline self-test.

The tested operational script bodies were preserved unchanged; documentation
synopses and symbol descriptions were added separately.

## What this experiment demonstrates

Under the tested conditions:

- an RFB transport/session can be rebuilt without relaunching PS2VNC;
- Ethernet/IOP networking does not need to be reinitialized merely because the
  RFB server disappears;
- GS/display state does not need to be rebuilt merely because the RFB provider
  changes;
- a fresh RFB handshake plus mandatory full framebuffer can restore desktop
  authority;
- the stable Pi endpoint can decouple the PS2 from a specific VNC
  implementation;
- deliberate provider replacement can appear to the PS2 as an ordinary
  recoverable loss of the old RFB stream.

## What this experiment does not demonstrate

It does not prove that:

- this exact patch is already the final production recovery policy;
- all RFB failures should automatically reconnect;
- silent stalls should automatically trigger recovery;
- malformed protocol data should enter a reconnect loop;
- w0vncserver is the production provider;
- `systemd-socket-proxyd` is required production architecture;
- input-provider handoff through w0 was qualified;
- display timing continuity across providers was formally measured;
- formal Issue #7 semantic/pcap qualification is complete.

## Durable design implication

The important architectural result is that connection-loss recovery and
deliberate provider replacement can share one PS2-side transaction:

    old RFB stream becomes invalid
        -> close the invalid session
        -> reconnect the stable endpoint
        -> perform fresh RFB negotiation
        -> reacquire an authoritative full framebuffer
        -> resume incremental service

If this reconnect behavior, or an adapted successor of it, becomes the standard
production fix for a lost RFB stream, then the preserved provider-handoff
scripts should work under that recovery mechanism by default.

The PS2 should not need separate logic for "switch to TigerVNC", "switch to
w0vncserver", or some future compatible provider. The durable abstraction is:

**recover the RFB service behind the stable endpoint.**

The provider remains a replaceable Pi-side mechanism.

GitHub Issue #25 tracks this future production recovery/failsafe work and should
reuse the preserved provider-handoff apparatus as a hardware qualification
case.

## Final disposition

The experiment is complete, successful for its stated scope, reproducible, and
preserved.

Do not promote the experimental reconnect patch solely because this experiment
passed. When connection-loss recovery becomes active product work, evaluate the
mechanism against the complete recovery, manual Refresh, service-recovery,
failsafe, and user-visible-state requirements.

If the same fresh-session reconnect contract is retained, provider handoff
should fall naturally out of it rather than becoming a second PS2-side feature.
