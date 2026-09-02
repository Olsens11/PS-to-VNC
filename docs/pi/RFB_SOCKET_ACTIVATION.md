# PS2-Facing RFB Socket-Activation Candidate

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    CLASSIFICATION=EVALUATING
    BASE_AUTHORITY=87baebce32e3c07ffc12298d6a168ac890231cf2
    ADOPTED_RUNTIME=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    SERVICE_MODEL=EVALUATING
    LIVE_APPLICATION=NOT_YET_PERFORMED
    HARDWARE_QUALIFICATION=NOT_YET_PERFORMED

This document develops a standards-first but product-adapted answer to the
cold-boot / first-connect race observed during clean-Pi experiments.

The design rule is:

> Use conventional Linux mechanisms for ownership and lifecycle; customize the
> policy only where PS-to-VNC has a demonstrated product benefit.

The candidate therefore uses NetworkManager for interface/address ownership,
systemd socket activation for demand startup, and TigerVNC's own documented
`-inetd` support for accepting an inherited listening socket. It does not add a
project-specific port watcher, link-state daemon, or TCP proxy.

Nothing in this document promotes the service model before live validation.

## Problem statement

The PS2 client uses a fixed peer address, `192.168.50.1:5900`. Legacy PS2VNC
source shows a vulnerable startup ordering: after the PS2 Ethernet PHY reports
link-up, the client immediately attempts one TCP connection to the VNC peer. A
carrier transition on the same cable can simultaneously be what allows the Pi
to finish activating the private Ethernet profile and VNC endpoint.

The desired product behavior is stronger:

1. the Pi may boot with the PS2 disconnected;
2. the private service identity remains deterministic;
3. no VNC process needs to consume resources merely because the Pi is powered;
4. the first PS2 TCP connect should encounter an already-listening kernel socket;
5. that first connection may demand-start the VNC server;
6. after activation, the VNC desktop may remain persistent across PS2 viewer
   disconnect/reconnect;
7. failure and restart behavior remain owned by standard service machinery.

## Conventional mechanisms available on the clean OS

### NetworkManager

The clean companion already uses NetworkManager and a static `ps2-link` profile:

    interface = eth0
    address = 192.168.50.1/24
    ipv4.method = manual
    ipv4.never-default = yes
    ipv6.method = disabled
    autoconnect = yes

Debian 13 currently carries NetworkManager 1.52.x. Its documentation states that
static Layer-3 profiles can be activated without carrier in appropriate cases,
and also exposes a per-device `ignore-carrier` setting specifically for
server-like interfaces whose services must bind before physical carrier exists.

Therefore the next live test must measure the actual clean-Pi boot behavior
before adding configuration. If the existing static profile already owns
`192.168.50.1/24` before carrier, no `ignore-carrier` override should be added.
If it does not, the smallest candidate is a per-`eth0` NetworkManager setting,
not a global `ignore-carrier=*` policy.

NetworkManager documentation:

- https://networkmanager.pages.freedesktop.org/NetworkManager/NetworkManager/NetworkManager.conf.html
- https://networkmanager.dev/docs/admins/

### systemd socket activation

systemd socket units are the conventional Linux mechanism for keeping a listening
socket available while demand-starting the service that will own it. With
`Accept=no`, systemd retains the listening socket and starts one service on the
first incoming connection. A single socket can be passed in traditional
inetd-compatible form using `StandardInput=socket`.

Relevant systemd concepts:

- `.socket` unit owns the listening socket;
- `Accept=no` passes the listening socket itself to one service;
- the first connection pays the service startup cost but does not require the
  service to have been running beforehand;
- queued TCP state remains in the kernel while the service starts;
- if the service later exits, the socket unit can remain available for a future
  activation;
- `BindToDevice=eth0` can constrain traffic to the private Ethernet device;
- `FreeBind=yes` allows the socket unit itself to bind a specific IP even if the
  address is not present yet at the instant the socket unit starts.

`FreeBind=yes` is a boot-order robustness aid, not a replacement for assigning
`192.168.50.1/24` to `eth0`: the private Layer-3 identity still must exist before
PS2 traffic can be routed locally.

### TigerVNC 1.15 `-inetd` wait mode

The adopted Debian package's `Xtigervnc(1)` manual explicitly supports on-demand
startup through `-inetd`. In its documented **wait** mode, the supervisor hands
Xtigervnc the listening socket. TigerVNC then accepts the first and subsequent
viewers itself and remains running after the original viewer disconnects.

The v1.15.0 source confirms the important implementation detail. `Xtigervnc`
duplicates file descriptor 0 for `-inetd`; when that descriptor is a listening
socket, `vncExtInit.cc` constructs a `network::TcpListener` from it and logs
`inetd wait`. If the inherited descriptor is an already-connected socket, it
instead adds that one client directly. This gives a source-level mapping from
systemd `Accept=no` + `StandardInput=socket` to TigerVNC's persistent wait-mode
behavior.

Sources:

- `TigerVNC/tigervnc` tag `v1.15.0`, `unix/xserver/hw/vnc/xvnc.c`;
- `TigerVNC/tigervnc` tag `v1.15.0`, `unix/xserver/hw/vnc/vncExtInit.cc`;
- Debian trixie `Xtigervnc(1)` man page.

## Candidate architecture

The preferred experiment is:

```text
Pi boot
  |
  +-- NetworkManager owns eth0 / 192.168.50.1/24
  |
  +-- systemd owns 192.168.50.1:5900 listening socket
  |      (no Xtigervnc process required yet)
  |
PS2 starts PS-to-VNC
  |
  +-- carrier appears
  +-- TCP SYN reaches an already-listening kernel socket
  +-- connection queues and triggers ps-to-vnc-rfb.service
  |
  +-- Xtigervnc starts in -inetd wait mode
  +-- inherits the listening socket
  +-- accepts the queued PS2 connection
  +-- serves the fixed 704x462 / depth-16 desktop
  |
PS2 disconnects
  |
  +-- Xtigervnc remains alive and owns the inherited listener
  +-- a later PS2 connection reuses the same desktop
```

This combines the strongest parts of the earlier ideas:

- conventional OS ownership rather than a custom watcher;
- genuine demand startup;
- no race between PS2 `connect()` and userspace calling `listen()`;
- no wildcard VNC exposure on Wi-Fi;
- persistent desktop after first activation;
- standard systemd failure/lifecycle reporting.

## Candidate unit contract

Tracked candidate unit files live in `systemd/pi/` on this evaluation branch.
They are not installed automatically.

The socket unit owns the PS2-facing network exposure:

    ListenStream=192.168.50.1:5900
    Accept=no
    BindToDevice=eth0
    FreeBind=yes

The service uses the exact adopted TigerVNC binary and the inherited listener:

    /usr/bin/Xtigervnc :1 -inetd

Product-significant RFB/desktop settings remain the current clean contract:

    geometry = 704x462
    depth = 16
    pixel format = RGB565
    SecurityTypes = None
    client desktop resize = disabled
    X11 TCP listener = disabled

In `-inetd` mode the service must not also configure its own RFB port/interface;
systemd owns that layer.

TigerVNC's `-inetd` startup redirects its normal stderr path internally, so the
candidate explicitly routes TigerVNC logs to syslog rather than assuming
`StandardError=journal` will capture all server diagnostics. Live validation must
confirm usable logs before promotion.

## Why `Accept=no`, not per-connection activation

`Accept=yes` would hand one already-connected stream to a new Xtigervnc process
for every viewer connection. That maps to TigerVNC's documented `nowait` mode:
the X/VNC session dies when that viewer disconnects. It would also allow
multiple independent X servers to be spawned for concurrent connection attempts.
That is not the desired PS-to-VNC product model.

`Accept=no` maps to TigerVNC wait mode: one listener, one Xtigervnc instance,
subsequent connections accepted by that instance, and persistent desktop state.

## PS2 retry policy remains defense in depth

A socket-activated design can make the first TCP handshake succeed before
Xtigervnc has finished initialization, because the kernel listener already
exists. The PS2 can then block normally while waiting for the RFB banner.

A later clean PS2 milestone should still consider bounded retry/backoff for
**initial connection establishment**. That is conventional client robustness and
is separate from the project's deliberate prohibition on automatic recovery of
unexplained established-session stalls during debugging.

The Pi should not require client retry for normal startup if this candidate is
working correctly.

## Live validation sequence

Do not install the candidate before capturing the existing cold-boot behavior.
The operator session should proceed in layers.

### A. Read-only baseline capture

With the PS2 Ethernet cable present but the PS2 not creating carrier, record:

- exact NetworkManager and systemd versions;
- exact `ps2-link` profile values;
- effective NetworkManager configuration;
- `eth0` carrier state;
- whether `ps2-link` is active;
- whether `192.168.50.1/24` already exists;
- all listeners on TCP 5900;
- existing VNC/systemd/helper processes and units;
- whether exploratory w0/kanshi/helper state is still active.

Then launch the current PS2 client once while timestamping carrier, address,
listener and first TCP attempt. This preserves proof of the pre-change race.

### B. Establish early private address only if needed

If the baseline proves `192.168.50.1/24` is absent before carrier, test the
smallest NetworkManager-native fix. Prefer the existing static-profile behavior
if it can be made deterministic without new configuration. Only if necessary,
use the scoped candidate:

```ini
[device-ps-to-vnc-eth0]
match-device=interface-name:eth0
ignore-carrier=true
```

Reboot and prove that:

- `192.168.50.1/24` exists before PS2 carrier;
- no default route moves to `eth0`;
- Wi-Fi/management networking remains independent;
- the existing `netplan-eth0` profile is not destroyed;
- carrier reporting still reflects physical reality.

### C. Install socket activation candidate

Before enabling anything:

- run `systemd-analyze verify` against both unit files;
- prove port 5900 is free;
- prove display `:1` is free;
- prove `/usr/bin/Xtigervnc` belongs to the reviewed Debian package;
- verify no conflicting legacy/helper service is enabled.

Enable/start only `ps-to-vnc-rfb.socket`. Before any PS2 VNC connection:

- socket unit = active/listening;
- service unit = inactive;
- no Xtigervnc process exists;
- listener is exactly IPv4 `192.168.50.1:5900` and bound to `eth0`;
- no listener exists on Wi-Fi, wildcard IPv4, or IPv6.

### D. First-connect demand activation

From the PS2, launch the clean client exactly once.

Expected:

- TCP connection succeeds on first launch;
- systemd activates `ps-to-vnc-rfb.service`;
- Xtigervnc reports `inetd wait` through the configured logging path;
- RFB 3.8 / SecurityType None / 704x462 contract passes;
- initial full Raw state and ordinary incremental update remain correct;
- no second ELF launch is required.

### E. Persistence and recovery

After the first successful session:

1. disconnect/relaunch PS2 client and prove the same Xtigervnc PID/session accepts
   it;
2. stop Xtigervnc service cleanly and prove the socket returns to demand-ready
   state;
3. reconnect and prove a fresh Xtigervnc is activated;
4. induce a controlled service failure and classify restart behavior before
   deciding whether `Restart=on-failure` belongs in the promoted unit;
5. reboot the Pi with no PS2 carrier and repeat first-launch qualification.

## Promotion criteria

Promote socket activation only if it beats the persistent-service control on
simplicity, first-connect reliability, lifecycle clarity and reproducibility.
Minimum required evidence:

- first-launch success from cold boot without operator intervention;
- private-address/listener isolation;
- no custom watcher/daemon;
- persistent desktop across ordinary viewer disconnect;
- deterministic service stop/failure/retrigger behavior;
- usable diagnostics;
- no regression in RFB/Raw/704x462 behavior;
- every NetworkManager/systemd mutation represented in provisioning and the
  dependency ledger.

If TigerVNC `-inetd` behavior or systemd handoff proves fragile on the real Pi,
fall back to the simpler conventional persistent Xtigervnc service. Demand start
is a product optimization, not a requirement worth compromising reliability.
