# PS2-Facing RFB Endpoint Lifecycle

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    CLASSIFICATION=ADOPTED_RUNTIME
    BASE_AUTHORITY=87baebce32e3c07ffc12298d6a168ac890231cf2
    CURRENT_PROVIDER=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    CURRENT_PROVIDER_ROLE=REPLACEABLE_IMPLEMENTATION
    NETWORK_OWNER=NetworkManager
    LIFECYCLE_OWNER=systemd
    SERVICE_MODEL=SYSTEMD_SOCKET_ACTIVATED
    LIVE_APPLICATION=PERFORMED
    HARDWARE_QUALIFICATION=PASS
    QUALIFICATION_EVIDENCE_HEAD=b40f422a760a0b7b6f2ab41699c5e72fda83bb83

This document develops the PS2-facing RFB connection-establishment design under
the project's clean-reconstruction principle:

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

The goal is not to make TigerVNC start cleverly. The goal is to establish a
clean, conventional, replaceable PS2-facing service boundary whose **current**
provider happens to be TigerVNC.

The pre-test design below is now reconciled with the completed live qualification.

## Reconstruction decision check

### Requirement / lesson

The PS2 needs a deterministic private peer and a reliable RFB endpoint on its
first connection attempt. Legacy PS2VNC shows why this matters: it waits for its
own Ethernet link and then immediately performs a one-shot TCP `connect()` to
`192.168.50.1:5900`. On a cold Pi, the same carrier transition can be what lets
the Pi finish bringing up the private interface and listener.

The durable lesson is **endpoint readiness**, not "run TigerVNC at boot."

### Conventional machinery

- NetworkManager owns `eth0` and `192.168.50.1/24`.
- systemd owns service lifecycle and, for the demand candidate, the listening
  socket.
- the Linux TCP stack owns connection establishment and pending connection
  queues.
- the current RFB provider uses its upstream-supported inherited-socket
  interface rather than a project-specific watcher or proxy.

### PS-to-VNC adaptation

Adopt the qualified always-ready kernel listener with demand-started userspace.
It directly addresses the PS2's one-shot first-connect behavior while retaining
standard Linux mechanisms.

### Replaceable provider / mechanism

The stable adopted concept is the **PS2-facing RFB endpoint**. The
current provider is Xtigervnc. TigerVNC-specific process arguments live only in
the provider unit. A future PS-to-VNC Pi gateway may replace that provider and
consume the same listener through a standard inherited-file-descriptor interface,
or the lifecycle mechanism itself may be replaced if future requirements justify
it.

The private `ps2-link` network identity is independent of either choice.

### Deferred concerns

Do not design or implement today:

- MPEG-2 high-motion overlay transport;
- audio transport;
- remote-computer source normalization;
- a generalized multiplexed PS-to-VNC gateway protocol;
- provider-selection frameworks;
- speculative port allocation for future media channels.

Those possibilities justify preserving a clean seam, not building the future
system before its requirements are known.

### Qualification gate

Promotion requires live evidence that the chosen mechanism improves first-launch
reliability without increasing lifecycle ambiguity, coupling, exposure, or
reproducibility cost. The conventional always-running provider remains the
control/fallback.

## Hardware qualification result

The 2026-09-02 campaign passed every required socket-lifecycle gate:

- the untouched timing run showed NetworkManager install `192.168.50.1/24`
  about 1.18 seconds before the sole PS2 SYN;
- the providerless socket owned `192.168.50.1:5900` while carrier, address, and
  Xtigervnc were absent;
- the first SYN activated Xtigervnc about 5.6 ms later and valid RFB server bytes
  followed about 379 ms after the SYN;
- one launch completed RFB 3.8/SecurityType None and the fixed framebuffer
  contract;
- reset/relaunch reused the same provider PID and preserved the red framebuffer;
- orderly stop left the socket ready and the next launch created a fresh
  provider;
- `SIGKILL` exposed a failed provider and stale X artifacts, but the next demand
  retriggered without `reset-failed`, manual cleanup, or a `Restart=` loop;
- cold reboot with the PS2 off restored the providerless socket and one launch
  activated the endpoint successfully.

The PS2's post-disconnect transition after orderly provider shutdown varied
between remaining black and returning to Free McBoot. The Pi-side socket,
provider-stop, and listener results were repeatable, so that observation is a
separate PS2 client exit-timing concern rather than lifecycle ambiguity.

The preserved evidence is under `evidence/issue5/`, including the first-launch
timing, first socket activation, same-provider relaunch, controlled stop,
controlled failure, retrigger, repeated-stop, and cold-reboot/one-launch runs.

Decision classifications:

- **Adopt:** generic systemd ownership of the PS2-facing RFB endpoint;
- **Adapt:** packaged Xtigervnc `-inetd` mode localized in the current provider;
- **Reject for the current requirement:** scoped NetworkManager no-carrier
  override;
- **Adopt as optional:** persistent provider unit retained as a mutually
  exclusive fallback/control, not the selected runtime;
- **Defer:** desktop/window-manager contents beyond the qualified bare
  framebuffer.

## Stable layer boundaries

The adopted design deliberately separates four concerns:

```text
PS2
 |
 |  RFB/TCP contract: 192.168.50.1:5900
 v
+----------------------------------------------+
| PS2-facing RFB endpoint boundary             |
| current lifecycle candidate: systemd socket  |
+----------------------------------------------+
                     |
                     | inherited listening socket
                     v
+----------------------------------------------+
| Current provider adapter                     |
| ps-to-vnc-rfb-tigervnc.service               |
| Xtigervnc -inetd                             |
+----------------------------------------------+
                     |
                     v
              dedicated desktop
```

The underlying network identity is separate:

```text
NetworkManager
    -> eth0
    -> 192.168.50.1/24
    -> no default route
    -> IPv6 disabled for the PS2 profile
```

A future provider can therefore replace TigerVNC without changing how the Pi
owns the private Ethernet identity. Likewise, a future gateway that eventually
owns RFB plus additional media/control transports does not require us to pretend
those transports exist today.

## Why systemd socket activation is architecturally useful

A `.socket` unit is not inherently a TigerVNC mechanism. It is a standard
service-manager boundary around a listening socket. With `Accept=no`, systemd
keeps one listening socket and activates one provider service.

For the current provider, `StandardInput=socket` maps that listener into
TigerVNC's documented `-inetd` wait mode. A future custom provider could instead
consume systemd-passed descriptors through the normal systemd socket-activation
API. We do **not** implement such a provider now.

This matters because the durable idea is:

> The OS may own endpoint readiness before the implementation that serves the
> endpoint is running.

That remains useful even if the provider changes.

## NetworkManager no-carrier decision

The clean companion uses a static `ps2-link` profile:

    interface = eth0
    address = 192.168.50.1/24
    ipv4.method = manual
    ipv4.never-default = yes
    ipv6.method = disabled
    autoconnect = yes

The untouched cold/no-carrier snapshot had no private address. That fact alone
did not establish a product failure: after carrier appeared, NetworkManager
installed the address about 1.18 seconds before the PS2's sole SYN. The first
connection therefore had the required Layer-3 identity without an override.

The scoped `ignore-carrier=true` candidate is rejected for the current
requirement and was not installed. NetworkManager remains the address owner.
`FreeBind=yes` serves the separate purpose of allowing systemd to own the
listener before address assignment.

## Candidate mechanisms

### Candidate A — persistent TigerVNC provider control

Tracked unit:

    ps-to-vnc-rfb-tigervnc-persistent.service

This is the conventional control. Xtigervnc owns `192.168.50.1:5900` directly
and remains running.

**Strengths:** simplest process model, mature direct TigerVNC path, server fully
initialized before client arrival once the service is running.

**Costs:** graphical server runs whenever the Pi is powered; boot ordering still
must ensure the private address exists before direct bind.

**Role:** control/fallback, not a rejected design.

### Candidate B — generic RFB socket + TigerVNC provider adapter

Tracked units:

    ps-to-vnc-rfb.socket
    ps-to-vnc-rfb-tigervnc.service

The generic socket owns:

    ListenStream=192.168.50.1:5900
    Accept=no
    BindToDevice=eth0
    FreeBind=yes

The provider-specific unit runs:

    /usr/bin/Xtigervnc :1 -inetd ...

The socket unit currently selects that provider through its `Service=` setting.
Changing providers later should be a small, explicit lifecycle change rather
than a redesign of NetworkManager, the PS2 client, or unrelated Pi services.

**Strengths:** kernel listener can already exist before provider startup; demand
startup is standard systemd policy; no custom watcher; provider identity is
localized.

**Qualification cost:** the current TigerVNC provider depends on its
less-common `-inetd` wait path and exact Debian package behavior. Hardware
qualification was therefore required and passed.

**Role:** adopted runtime lifecycle after hardware qualification.

### Candidate C — scoped NetworkManager no-carrier behavior

Tracked candidate snippet:

    config/pi/NetworkManager/90-ps-to-vnc-ps2-link.conf

This was **conditional**, not a third lifecycle design. Live timing showed the
address was absent without carrier but available well before the PS2's first
SYN. The candidate is therefore rejected for the current requirement and remains
uninstalled.

### Rejected primary mechanisms for this problem

- carrier-triggered dispatcher start: reacts to the same event that triggers the
  PS2's connect and therefore retains the race;
- custom link/port watcher daemon: duplicates standard lifecycle/socket
  machinery;
- `systemd-socket-proxyd` in front of TigerVNC: adds a process/data hop while
  the current provider already supports inherited listener mode;
- PS2 retry as the normal Pi startup mechanism: useful later as client defense in
  depth, but it should not be required to make ordinary Pi readiness correct.

Any of these may be reconsidered if a future provider exposes a demonstrated
constraint that changes the problem.

## Current provider contract

For the qualified Issue #5 lifecycle, TigerVNC remains the current desktop
provider. Its
provider-local settings are:

    geometry = 704x462
    depth = 16
    pixel format = RGB565
    SecurityTypes = None
    client desktop resize = disabled
    X11 TCP listener = disabled

These settings qualify the current provider. They do not make TigerVNC itself a
permanent architecture dependency.

The PS2-facing RFB behavior remains the important compatibility contract.

## Why future media/gateway ideas do not require more abstraction now

Potential future work includes using MPEG-2 for high-motion regions while
retaining a crisp RFB desktop, adding audio, and using the Pi as a normalized
bridge to other computers. Those possibilities could eventually justify a
custom Pi-side gateway.

The present design prepares for that only by avoiding unnecessary coupling:

```text
TODAY
PS2 -> RFB endpoint -> TigerVNC provider

POSSIBLE FUTURE
PS2 -> PS-to-VNC gateway
          |- RFB desktop service
          |- media service(s)
          |- audio service
          `- local or remote desktop source
```

We intentionally make **no claim** today about whether those future services use
one process, multiple processes, one TCP session, multiple sockets, UDP, or any
particular synchronization protocol. When those issues become active, the
network/session module can be revised from real requirements.

## Hardware-test sequence

The following sequence is retained as the completed qualification procedure.
Its results are summarized above.

The operator session is intentionally layered so each mutation answers one
question.

### Phase 0 — untouched read-only baseline

Run `scripts/pi/inspect-rfb-activation.sh` before changing anything.

Confirm:

- exact Pi/OS/NetworkManager/systemd/TigerVNC identity;
- current `ps2-link` profile and effective NetworkManager config;
- no-carrier state of `eth0`;
- whether `192.168.50.1/24` already exists;
- current listeners/processes/units;
- whether the preserved w0 experiment still has any live helper/listener state;
- display `:1` collision state.

Then run `scripts/pi/capture-rfb-activation-timeline.sh` and perform exactly one
current PS2 launch. Preserve the pre-change ordering of carrier, address,
ARP/TCP, and listener state.

### Phase 1 — classify private-identity timing

The address was absent before carrier. The captured carrier/address/SYN ordering
then proved that NetworkManager installed it about 1.18 seconds before the sole
PS2 SYN. Because the endpoint succeeded on the first attempt, the conditional
no-carrier override was not installed.

### Phase 2 — stage lifecycle candidates

Use `install-rfb-activation-units.sh install` only after the live state is
understood. It stages but does not start/enable:

- generic RFB socket;
- socket-activated TigerVNC provider;
- persistent TigerVNC control provider.

Before starting either lifecycle path, prove port 5900 and display `:1` are free
and no exploratory w0/helper owns the endpoint.

Never run the persistent control and socket candidate simultaneously. Their unit
contracts explicitly conflict.

### Phase 3 — socket-ready state with provider absent

Start only `ps-to-vnc-rfb.socket`.

Required pre-PS2 state:

- `ps-to-vnc-rfb.socket` = active/listening;
- `ps-to-vnc-rfb-tigervnc.service` = inactive;
- `ps-to-vnc-rfb-tigervnc-persistent.service` = inactive;
- no Xtigervnc process;
- listener exactly on IPv4 `192.168.50.1:5900`/`eth0`;
- no wildcard/Wi-Fi/IPv6 RFB listener.

This is the architectural control point: the endpoint boundary is ready while
the current provider is absent.

### Phase 4 — one-launch demand activation

Start the capture apparatus, then launch the clean PS2 client exactly once.

Expected:

- first TCP connection succeeds;
- systemd activates `ps-to-vnc-rfb-tigervnc.service`;
- Xtigervnc enters inherited-listener wait mode;
- the first server protocol bytes are valid RFB;
- RFB 3.8 / SecurityType None / 704x462 contract succeeds;
- initial full Raw state and ordinary incremental updates still work;
- no second ELF launch is required.

### Phase 5 — persistence and retrigger

After success:

1. disconnect/relaunch the PS2 client and prove the same Xtigervnc PID/session
   accepts it;
2. stop the provider deliberately and prove the generic socket remains or
   returns to demand-ready state;
3. reconnect and prove a fresh provider is activated;
4. classify controlled failure behavior before selecting any `Restart=` policy.

### Phase 6 — persistent-service control if needed or for comparison

The persistent service was retained as a mutually exclusive control/fallback.
The comparison gate was conditional: use it only if socket inheritance or
lifecycle remains ambiguous. All socket gates passed, so changing to the control
would not have answered an unresolved product question and was skipped.

### Phase 7 — cold-boot qualification

The generic socket was enabled as the sole lifecycle, then the Pi was rebooted
with the PS2 powered off. Before carrier or address assignment, systemd owned the
exact private listener and the provider was absent. One PS2 launch established
the address, activated Xtigervnc, negotiated the intended RFB endpoint, and
displayed the black 480p root framebuffer without operator repair.

## Promotion criteria

Promote a lifecycle only if evidence supports it. Minimum requirements are:

- first-launch reliability from cold boot;
- exact private-address/listener isolation;
- clean ownership between NetworkManager, systemd endpoint boundary, and current
  provider;
- no custom watcher/daemon without demonstrated necessity;
- deterministic stop/failure/retrigger semantics;
- usable diagnostics;
- no regression in RFB/Raw/704x462 behavior;
- reproducible tracked configuration;
- provider-specific assumptions localized to the provider layer.

The adopted statement is:

> systemd owns the PS2-facing RFB listener and activates the selected RFB
> provider on demand; TigerVNC is the current provider implementation.

The architecture does **not** require TigerVNC `-inetd`. That is the qualified
adapter used by the current replaceable provider. This distinction preserves the
clean-reconstruction intent while choosing a concrete implementation today.
