# PS2-Facing RFB Endpoint Lifecycle Evaluation

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    CLASSIFICATION=EVALUATING
    BASE_AUTHORITY=87baebce32e3c07ffc12298d6a168ac890231cf2
    CURRENT_PROVIDER=Xtigervnc_1.15.0+dfsg-2.1~deb13u1
    CURRENT_PROVIDER_ROLE=REPLACEABLE_IMPLEMENTATION
    NETWORK_OWNER=NetworkManager
    LIFECYCLE_OWNER=systemd
    SERVICE_MODEL=EVALUATING
    LIVE_APPLICATION=NOT_YET_PERFORMED
    HARDWARE_QUALIFICATION=NOT_YET_PERFORMED

This document develops the PS2-facing RFB connection-establishment design under
the project's clean-reconstruction principle:

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

The goal is not to make TigerVNC start cleverly. The goal is to establish a
clean, conventional, replaceable PS2-facing service boundary whose **current**
provider happens to be TigerVNC.

Nothing here promotes a service model before live validation.

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

Evaluate an always-ready kernel listener with demand-started userspace because
it directly addresses the PS2's one-shot first-connect behavior while retaining
standard Linux mechanisms.

### Replaceable provider / mechanism

The stable concept under evaluation is the **PS2-facing RFB endpoint**. The
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

## Stable layer boundaries

The current candidate deliberately separates four concerns:

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

## Why systemd socket activation remains architecturally useful

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

## NetworkManager no-carrier policy

The clean companion already uses a static `ps2-link` profile:

    interface = eth0
    address = 192.168.50.1/24
    ipv4.method = manual
    ipv4.never-default = yes
    ipv6.method = disabled
    autoconnect = yes

Current NetworkManager documentation says a static Layer-3 profile can already
autoconnect without carrier in some startup paths and also provides a per-device
`ignore-carrier` setting for server-like static interfaces.

Therefore **measure before configuring**:

1. if `192.168.50.1/24` already exists with no PS2 carrier, add nothing;
2. if it does not, evaluate only the scoped `eth0` `ignore-carrier=true`
   candidate;
3. do not use a global ignore-carrier policy;
4. prove management/Wi-Fi routing remains independent.

`FreeBind=yes` on the systemd socket does not substitute for this. It permits an
early bind but does not make an unassigned address a usable Layer-3 identity or
answer ARP for it.

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

**Costs:** current TigerVNC provider depends on its less-common `-inetd` wait
path and the exact Debian package behavior, so hardware qualification is
required.

**Role:** preferred experiment.

### Candidate C — scoped NetworkManager no-carrier behavior

Tracked candidate snippet:

    config/pi/NetworkManager/90-ps-to-vnc-ps2-link.conf

This is **conditional**, not a third lifecycle design. Use it only if the
untouched clean Pi proves the static profile does not own `192.168.50.1/24`
before carrier.

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

For this Issue #5 experiment, TigerVNC remains the current desktop provider. Its
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

## Prepared hardware-test sequence

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

### Phase 1 — establish early private identity only if baseline requires it

If and only if the baseline proves the address is absent before carrier:

1. stage the scoped NetworkManager candidate with
   `install-ps2-link-no-carrier-candidate.sh`;
2. inspect the exact staged file;
3. reboot as a separate operator-visible step;
4. prove `192.168.50.1/24` exists before PS2 carrier;
5. prove no default route moved to `eth0` and management remains intact.

If the address already exists before carrier, skip this entire phase.

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

Stop the socket candidate completely, then start
`ps-to-vnc-rfb-tigervnc-persistent.service`.

Use the same RFB and physical stimulus checks. This tells us whether any failure
belongs to socket inheritance/lifecycle rather than TigerVNC/RFB itself.

The persistent service wins if socket activation adds fragility without a clear
product benefit.

### Phase 7 — cold-boot qualification

Only after the layers above are understood:

1. select exactly one lifecycle candidate;
2. reboot with no active PS2 VNC session;
3. confirm private identity/listener readiness;
4. launch the PS2 client once;
5. prove desktop and incremental update behavior without operator repair.

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

If socket activation passes, the adopted statement should be approximately:

> systemd owns the PS2-facing RFB listener and activates the selected RFB
> provider on demand; TigerVNC is the current provider implementation.

It should **not** be:

> PS-to-VNC architecture requires TigerVNC `-inetd`.

That distinction preserves the clean-reconstruction intent while still choosing
a concrete, testable implementation today.
