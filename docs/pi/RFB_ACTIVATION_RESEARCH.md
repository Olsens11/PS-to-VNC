# RFB Connection-Establishment Research

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    CLASSIFICATION=EVALUATING_RESEARCH
    BASE_AUTHORITY=87baebce32e3c07ffc12298d6a168ac890231cf2
    RECOMMENDED_EXPERIMENT=SYSTEMD_SOCKET_PLUS_XTIGERVNC_INETD_WAIT
    LIVE_PI_PROOF=PENDING

This note records the standards research behind
`RFB_SOCKET_ACTIVATION.md`. It is deliberately broader than the candidate unit
files so future sessions can distinguish mechanisms that were considered from
the narrower experiment that was selected.

The governing design principle is:

> Prefer conventional Linux mechanisms and upstream-supported application
> interfaces. Adapt the policy when that produces a real PS-to-VNC product
> benefit; do not reject a useful design merely because it is not the distro's
> default configuration.

## Requirements derived from the real system

The connection-establishment problem is unusual in one important way: the PS2
Ethernet PHY becoming active can be the same event that produces carrier on the
Pi. Legacy PS2VNC then immediately attempts one blocking TCP connection to
`192.168.50.1:5900` and does not retry that initial `connect()` after failure.

A strong Pi design therefore should not merely start the VNC process quickly. It
should arrange for TCP 5900 to be a valid listening endpoint before the PS2's
first SYN whenever possible.

Other requirements remain conventional:

- fixed private service address, independent of Wi-Fi/management identity;
- no default route through the PS2 interface;
- no unauthenticated VNC exposure on Wi-Fi or wildcard interfaces;
- normal OS supervision, logging and rollback;
- no custom polling daemon when standard mechanisms provide the same behavior;
- reproducible configuration from a supported Debian/Raspberry Pi OS base.

## NetworkManager findings

NetworkManager is already the OS network owner on the clean Pi. Its official
administrator documentation explicitly calls out the server case where services
need to bind an interface address at boot while no carrier is present. Current
NetworkManager also documents a per-device `ignore-carrier` setting for this
case; the older global `main.ignore-carrier` mechanism is superseded by the
per-device setting.

Current documentation additionally notes that a profile containing only static
Layer-3 configuration can already autoconnect without carrier in some startup
paths. That means the clean Pi must be measured before adding any override.

Resulting policy:

1. keep `ps2-link` as the NetworkManager-owned static profile;
2. first test whether `192.168.50.1/24` is already owned before carrier;
3. if yes, add nothing;
4. if no, evaluate only the scoped `eth0` `ignore-carrier=true` snippet;
5. never use a broad global ignore-carrier rule just to make this project work.

References:

- https://networkmanager.dev/docs/admins/
- https://networkmanager.pages.freedesktop.org/NetworkManager/NetworkManager/NetworkManager.conf.html

## systemd socket-activation findings

A `.socket` unit is the conventional systemd mechanism for keeping a kernel
listening socket available while demand-starting its service.

For this product, `Accept=no` is the important mode. systemd documents that:

- only one service is spawned;
- the **listening socket itself** is passed to that service;
- only the first connection pays activation cost;
- `FreeBind=yes` is recommended when binding a specific address before that IP
  may have been configured yet;
- `BindToDevice=eth0` applies `SO_BINDTODEVICE`, accepts traffic only from that
  interface, and adds an implicit dependency on the kernel device unit;
- pending socket data/connections are normally retained when the triggered
  service exits (`FlushPending=no` is the default).

For inetd-compatible programs, systemd can expose the inherited socket as file
descriptor 0 with `StandardInput=socket`. `systemd-socket-activate --inetd`
provides the same style of handoff for command-line testing.

References:

- `systemd.socket(5)`
- `systemd.exec(5)`
- `systemd-socket-activate(1)`
- https://www.freedesktop.org/software/systemd/man/

### What this changes about the PS2 race

With direct process startup, the race is:

```text
PS2 carrier -> Pi address -> Xtigervnc starts -> userspace listen()
     `---------------- PS2 connect() can arrive here ----------------'
```

With socket activation, the listener is OS-owned before Xtigervnc exists:

```text
Pi boot -> systemd LISTEN on 192.168.50.1:5900
PS2 carrier -> SYN/SYN-ACK -> connect() succeeds/queues
                           -> systemd starts Xtigervnc
                           -> Xtigervnc accepts queued connection
                           -> RFB banner/handshake
```

This is materially better than merely making a service start quickly. The
kernel can complete/queue TCP establishment while userspace is still starting,
which directly accommodates the legacy PS2 client's one-shot initial connect.
The client then waits in its ordinary RFB receive path for Xtigervnc to become
ready.

`FreeBind=yes` alone does **not** make `192.168.50.1` a usable local Layer-3
identity and does not answer ARP for an address the host does not own. It makes
the listening socket boot-order tolerant. NetworkManager still owns the actual
address assignment.

## TigerVNC `-inetd` is a real supported interface

The exact adopted server is Debian trixie's
`tigervnc-standalone-server=1.15.0+dfsg-2.1~deb13u1`.

TigerVNC's `Xtigervnc(1)` manual documents two inetd modes:

- **nowait**: the supervisor passes one connected viewer stream; one Xvnc is
  created per viewer and dies with that connection/session;
- **wait**: the supervisor passes the **listening socket**; one Xvnc accepts the
  first and later viewers itself and remains alive after the first viewer
  disconnects.

The v1.15.0 source matches the documentation. `xvnc.c` duplicates file
 descriptor 0 when `-inetd` is present. `vncExtInit.cc` checks whether that
inherited descriptor is listening; if so it constructs a `network::TcpListener`
and logs `inetd wait`. Otherwise it treats the descriptor as one connected
client.

That maps naturally to:

    systemd Accept=no
    + StandardInput=socket
    + Xtigervnc -inetd
    = TigerVNC wait mode

References:

- TigerVNC tag `v1.15.0`, `unix/xserver/hw/vnc/xvnc.c`
- TigerVNC tag `v1.15.0`, `unix/xserver/hw/vnc/vncExtInit.cc`
- Debian trixie `Xtigervnc(1)`

## Important 1.15.0 inetd regression and Debian's fix

Upstream TigerVNC 1.15.0 had a real regression in `-inetd` mode: Xvnc printed its
human-readable startup banner to stdout before protocol handling. Traditional
xinetd configurations commonly attach stdout to the network stream, so VNC
clients received banner text before `RFB 003.008` and rejected the server.
TigerVNC issue #1937 documents the failure and upstream fix.

This is directly relevant to evaluating whether `-inetd` is mature enough for
PS-to-VNC, and it has a favorable answer for the **exact Debian package we
adopted**: Debian carries `0030 fix inetd mode.patch` specifically to restore
Xtigervnc inetd operation. Debian's patch tracker describes the 1.15 banner
regression, the resulting RFB protocol corruption, and the carried fix. The
same patch is present in the trixie
`1.15.0+dfsg-2.1~deb13u1` source package.

References:

- https://github.com/TigerVNC/tigervnc/issues/1937
- https://sources.debian.org/patches/tigervnc/1.15.0%2Bdfsg-2.1~deb13u1/

Two consequences matter:

1. we are not proposing to depend on a known-broken vanilla 1.15.0 inetd path;
   the reviewed Debian runtime carries the distro fix;
2. the live gate should still verify the installed Debian package identity and
   actual first bytes on the wire rather than trusting version text alone.

The candidate service also keeps `StandardOutput=journal` and directs TigerVNC's
own logger to syslog. The network socket is inherited on fd 0; ordinary process
stdout is therefore not being used as our RFB transport. This is useful
separation, but the package-level inetd fix remains part of the dependency
identity and should be verified rather than treated as unnecessary.

## Alternatives considered

### 1. Always-running direct Xtigervnc service

**Conventionality:** excellent.

**Advantages:** smallest conceptual service graph; no inherited-socket behavior;
server already initialized before the PS2 arrives.

**Costs:** Xtigervnc and its X framebuffer run from boot even if never used;
startup still needs deterministic ordering after the private address exists; it
does not exploit the useful demand-start product idea.

**Role:** control/fallback. If socket activation is less reliable or harder to
operate on the real Pi, this wins immediately.

### 2. systemd socket activation + native Xtigervnc wait mode

**Conventionality:** excellent machinery plus an application-supported inetd
interface.

**Advantages:** kernel listener is ready before userspace; no custom watcher;
first TCP connection naturally triggers server startup; one persistent desktop
accepts later viewers; exact private address/device exposure is systemd-owned;
clean failure/retrigger semantics are testable through standard units.

**Costs:** depends on a less commonly used TigerVNC startup path and therefore
requires exact package/source verification and live qualification.

**Role:** preferred experiment.

### 3. NetworkManager dispatcher or carrier-triggered service start

**Conventionality:** uses supported OS hooks, but adds project policy script.

**Advantages:** intuitive mapping from physical link-up to service start.

**Costs:** it reacts to the **same carrier event** that causes the PS2 to call
`connect()`, so it preserves the timing race we are trying to remove. The
service must still initialize and call `listen()` after the trigger.

**Role:** rejected as the primary race solution unless later evidence reveals a
separate need for carrier-triggered work.

### 4. Custom daemon watching TCP attempts/link state

**Conventionality:** poor when socket activation already exists.

**Advantages:** arbitrary policy is possible.

**Costs:** new code, lifecycle, logging, failure modes, race windows and
maintenance for a job the OS already implements.

**Role:** rejected absent a requirement standard mechanisms cannot satisfy.

### 5. `systemd-socket-proxyd` in front of ordinary Xtigervnc

**Conventionality:** systemd-provided and useful for daemons that cannot inherit
sockets.

**Advantages:** can retrofit socket activation around a non-socket-aware
backend.

**Costs:** extra local data path and process; backend readiness/connection
ordering must still be coordinated; TigerVNC already has native `-inetd` wait
mode, so the proxy currently solves no missing capability.

**Role:** fallback technique only if native TigerVNC listener inheritance proves
unusable for a reason not visible in source/docs.

### 6. PS2 initial-connect retry only

**Conventionality:** bounded retry/backoff is normal client robustness.

**Advantages:** handles temporary peer unavailability and should eventually be
added as defense in depth.

**Costs:** does not make the Pi service architecture correct; normal startup
would depend on the PS2 repeatedly probing until the Pi catches up.

**Role:** later client hardening, not the primary Pi solution. It remains
separate from automatic recovery of an unexplained established-session stall,
which is intentionally disabled during current freeze debugging.

## Optional future lifecycle policy

TigerVNC also exposes `MaxDisconnectionTime`: terminate the server after no VNC
client has been connected for N seconds. Combined with a persistent systemd
socket, that could create a fully cyclic demand model:

```text
first PS2 connect -> start desktop -> keep session for reconnects
idle/disconnected N seconds -> Xtigervnc exits -> socket remains ready
next PS2 connect -> fresh desktop starts
```

This is an upstream-supported mechanism, not a custom idle watcher. It is **not**
part of the first candidate because session persistence is currently more
valuable than speculative resource savings. It should be considered later only
if a measured product benefit justifies losing desktop state after the timeout.

## Recommendation before hardware testing

Prepare and test candidate 2 first, with candidate 1 retained as the simple
control/fallback.

Do not add the NetworkManager no-carrier snippet unless the real clean Pi proves
it is necessary. Do not add a dispatcher, proxy or PS2 retry merely to improve
the odds of the first experiment.

The decisive live evidence is straightforward:

1. what owns `192.168.50.1/24` before carrier;
2. whether systemd can own `192.168.50.1:5900` while Xtigervnc is absent;
3. whether one PS2 launch completes TCP and triggers the service;
4. whether the first network payload from the server is valid RFB, not process
   output;
5. whether Xtigervnc remains alive and accepts a second viewer connection;
6. whether service exit returns the system cleanly to socket-ready state;
7. whether a cold reboot repeats the behavior without operator intervention.
