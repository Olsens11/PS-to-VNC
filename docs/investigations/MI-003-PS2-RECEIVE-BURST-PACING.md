# MI-003 — PS2 receive burst tolerance and server-side pacing

Status: `OPEN_NON_BLOCKING`

Historical campaign: legacy PS2VNC Test15B Resource Maps 1-28 and runtime
Implementations 31-36

Successor working issue: to be linked from the casebook index

## Executive summary

Legacy PS2VNC performance work discovered an important distinction between
**available TCP receive credit** and **how aggressively a Linux sender consumes
that credit**.

A roughly 64 KiB receive-window configuration was repeatably healthy around the
low-50-Mbit/s range under the historical high-change workload. Simply increasing
PS2 TCP receive capacity to roughly 128 KiB or 256 KiB did not monotonically
improve throughput. The larger advertised windows permitted longer line-rate
packet trains and produced much heavier TCP retransmission and worse useful
throughput.

The decisive historical result was that a fresh 128 KiB-window PS2 session became
healthy again when the Pi paced only server-to-PS2 RFB traffic with a Linux TBF
at the demonstrated operating point:

```text
rate    = 60 Mbit/s
burst   = 16 KiB
latency = 50 ms
```

The final matched B2K validation acknowledged approximately 53.095 Mbit/s for
32.470 seconds with 10,694 retransmitted bytes and zero qdisc drops. The legacy
runtime then operationalized this as a flow-specific, self-healing pacing
service.

This was a sound **legacy engineering solution**, but it did not prove the exact
physical/software point at which unrestricted burst trains fail. The historical
record explicitly warns that 60 Mbit/s is an operating point, not a PS2/SMAP
hardware ceiling, and that the data does not prove SMAP FIFO overflow or a
specific NETMAN ring-drop mechanism.

The clean successor should therefore preserve the lesson, not fossilize the old
constant or TigerVNC-specific implementation:

> The PS2 receive path has a demonstrated burst-service envelope. Sender-side
> burst shaping/admission may be valuable, especially when larger receive credit
> is advertised, but exact pacing policy must remain transport/provider-aware and
> must be requalified against the clean workload.

This case matters more as future MPEG-2, audio, remote-computer, or gateway paths
are considered. Those paths may create multiple simultaneous traffic classes.
The project must not accidentally treat `60 Mbit/s on TCP/5900` as either a
universal limit or a permanent RFB-only architecture.

## Symptom and impact

The weak historical workload was rapid, large-area desktop change, especially at
1080i. Ordinary interactive desktop use was already viable.

During Test15B, larger TCP receive windows looked attractive because the
64 KiB connection could finish receive-window-limited. In practice, giving the
Linux sender more flight credit exposed a different problem:

- useful throughput dropped;
- update completion rate dropped;
- retransmission rose sharply;
- repeated runs became much more variable;
- simply doubling one PS2-side queue did not restore the known-good behavior.

The practical risk is architectural: a naive future design could assume that
"more window/buffering is always better" or that the PS2 can safely accept any
burst pattern up to the nominal 100BASE-T line rate.

## Historical investigation

### B2G — repeatable 64 KiB control

The exact archived B2G 64 KiB-window ELF was redeployed and tested repeatedly on
the deterministic workload.

Historical checkpoint results:

```text
53.12, 53.14, 52.58 Mbit/s
1.70,  1.70,  1.75 completed updates/s
retransmitted bytes: 9,926 / 22,688 / 35,450
```

This result was tight enough to serve as the control. It also demonstrated that
`rfb_rx_syscalls` itself was not a useful throughput target: recv-call rate could
vary substantially while useful throughput remained similar.

The 64 KiB connection could finish receive-window-limited. Therefore the result
did **not** prove that 64 KiB represented maximum useful network capacity.

### B2J — approximately 256 KiB receive credit

B2J enabled native lwIP window scaling and advertised approximately 256 KiB of
receive capacity (`wscale:2,7`).

Historical result:

```text
~11.82 Mbit/s
~0.30 completed updates/s
severe retransmission/recovery behavior
```

More receive credit made the workload much worse.

### B2K — approximately 128 KiB receive credit, initially unpaced

B2K advertised approximately 128 KiB (`wscale:1,7`). Initial deterministic
performance improved relative to B2J but remained substantially worse than B2G:

```text
~33.78 Mbit/s
~1.10 completed updates/s
```

This established that simply reducing the scaled window from 256 KiB to 128 KiB
did not solve the problem.

### Receive-resource mapping

Source and runtime inspection established several relevant facts:

- stock NETMAN uses a 64-frame IOP-to-EE receive ring;
- when that allocation is full, the IOP path busy-waits for EE reuse rather than
  simply returning NULL;
- SMAP has a 16,384-byte receive FIFO, roughly ten full Ethernet frames;
- NETMAN/SMAP deliberately group frame delivery around short time intervals;
- the lwIP `MEMP_NUM_TCPIP_MSG_INPKT` knob was not active in this receive path
  because `LWIP_TCPIP_CORE_LOCKING_INPUT` was enabled;
- exposed SMAP drop/overrun counters remained zero in relevant lossy runs.

The last point is important: the experiments did **not** justify saying
"the SMAP FIFO overflowed." A zero exposed counter also does not prove that no
upstream/downstream pressure effect occurred.

### B2O — double the NETMAN receive ring

The matched EE/IOP NETMAN ring was doubled from 64 to 128 slots while retaining
the roughly 128 KiB TCP configuration.

Historical result:

```text
~41.77 Mbit/s
~1.35 updates/s
~820 KiB retransmitted in the 20-second sample
```

The larger ring helped relative to the worst B2K result but did not restore B2G
quality. This substantially weakened a simplistic "64-slot ring capacity is the
single bottleneck" theory.

### B2N/B2P — apparent timing/query improvement not proven causal

Some runs that added or isolated periodic NETMAN queries initially appeared
better. Later source inspection showed the normal EAGAIN path already performs a
synchronous NETMAN link-status ioctl on every idle poll.

Repeated B2P runs varied substantially:

```text
32.83, 44.43, 43.74 Mbit/s
1.00,  1.45,  1.45 updates/s
```

The historical checkpoint correctly refused to promote the apparent timing
effect to causal fact.

### Resource Maps 15-20 — wire-burst discriminator

Packet analysis produced a much stronger discriminator.

Known-good B2G 64 KiB behavior had a modeled maximum continuous 100BASE-T busy
interval of approximately:

```text
5.506 ms
```

and about:

```text
1.064 ms maximum Pi transmit-queue delay
```

Under the same high-change load, unpaced B2K 128 KiB produced approximately:

```text
12.208 ms modeled continuous wire interval
3.126 ms transmit-queue delay
360,786 retransmitted bytes
```

The larger window therefore correlated with materially longer line-rate packet
trains, not merely with a larger abstract TCP number.

### B2K + Pi TBF60 — decisive recovery

Pi-side TBF shaping at:

```text
60 Mbit/s
16 KiB burst
50 ms latency
```

changed the result dramatically.

A fresh B2K session sustained approximately 32.452 seconds at 53.166 Mbit/s
acknowledged payload with only 4,254 retransmitted bytes and zero TBF drops.

A later matched flow-specific validation recorded:

```text
B2G 64K unpaced:          49.294 Mbit/s acknowledged
B2G 64K + TBF60:          45.741 Mbit/s acknowledged
B2K 128K + TBF60:         53.095 Mbit/s acknowledged
```

The final B2K run acknowledged 215,498,700 bytes over 32.470 seconds, retransmitted
10,694 bytes, and recorded zero qdisc drops.

This demonstrated that the larger 128 KiB transport credit could be useful when
the sender did not consume it as an unrestricted long burst.

### Map22 — normal RFB update cadence is application-level stop-and-wait

Map22 established that ordinary live PS2VNC behavior sends one incremental
`FramebufferUpdateRequest`, waits for the complete corresponding
`FramebufferUpdate`, applies/presents it, and then sends the next request.

Therefore historical instability was not explained by a queue of many stale RFB
updates piling up at the application level. One update could still be several
MiB, so one request/response transaction could itself generate a large network
burst.

This finding was also why a custom RFB proxy solely to coalesce a backlog of old
updates was not justified.

### Flow-specific implementation

The final legacy runtime used a PRIO root with a TBF child only for traffic
matching:

```text
destination = 192.168.50.2
TCP source port = 5900
```

Unmatched Ethernet traffic was not intentionally rate-limited.

The helper refused to replace an unfamiliar root qdisc and later gained
nondisruptive self-healing: deliberate deletion of the pacing tree was repaired
in roughly 0.2 seconds while the TigerVNC PID remained unchanged.

This was operationally qualified historical behavior, not merely an ad-hoc test
command.

## What is known with high confidence

### Historical test-supported

- Larger advertised TCP receive credit could materially worsen useful RFB
  throughput under the historical high-change workload.
- The bad 128 KiB case correlated with longer line-rate wire busy intervals and
  much heavier retransmission than the known-good 64 KiB control.
- Doubling the NETMAN receive ring alone did not restore known-good performance.
- Sender-side TBF pacing restored healthy B2K useful throughput with very low
  retransmission compared with the unpaced large-window condition.
- A 128 KiB receive window remained useful when paired with controlled sender
  burst behavior.
- The ordinary live RFB loop had only one outstanding framebuffer-update request
  at a time.
- The legacy flow-specific pacing helper could be installed, validated, removed,
  and self-healed without globally throttling unrelated Ethernet traffic.

### Source-supported

- Stock NETMAN's historical receive ring was 64 frames.
- Full-ring behavior busy-waits for reuse rather than proving a simple drop at
  allocation.
- SMAP RX FIFO is physically shallow relative to a long line-rate burst.
- The historical exposed SMAP drop/overrun counters did not report the large TCP
  retransmission event.

## What is **not** proven

The historical evidence does not establish:

- that 60 Mbit/s is a physical PS2 Ethernet, SMAP, IOP, SIF, EE, or lwIP ceiling;
- the exact internal packet-loss/stall point;
- that SMAP FIFO overflow is the root cause;
- that the 64-credit NETMAN ring is the root cause;
- that all 64 NETMAN credits are consumed by one TCP flight;
- that one specific cache, interrupt, DMA, or scheduling mechanism explains the
  burst sensitivity;
- that the historical 60/16KiB/50ms constants are optimal for every provider,
  resolution, encoding, future transport, or clean implementation;
- that every future stream should be shaped identically to RFB TCP/5900.

## Leading interpretation

The best-supported engineering interpretation is:

> The legacy PS2 receive pipeline had enough aggregate capacity for useful
> low-50-Mbit/s desktop throughput, but it could not reliably absorb some long,
> concentrated sender burst trains produced when Linux was given larger flight
> credit. Keeping elasticity on the Pi and consuming PS2 receive credit at a
> controlled rate produced a much healthier operating envelope.

This describes the observed system without pretending to know the exact internal
queue or hardware event that first causes retransmission.

## Current successor disposition

The clean project should **not** automatically install the historical pacing
configuration merely because it existed.

The current Pi audit correctly treats pacing as `EVALUATING` historical runtime
state to preserve/requalify rather than folklore to silently clone or silently
remove.

The present clean minimal PS2 RFB work is intentionally simple. When sustained
high-change performance becomes an active qualification target, pacing should be
retested against the clean client, current provider, current RFB request policy,
and current network dependency.

## Design guardrails while open

- Do not encode `60 Mbit/s` as a hardware capability constant.
- Do not make TCP/5900 the permanent architectural identity of burst control.
  Today it identifies RFB traffic; a future gateway/provider may classify traffic
  differently.
- Keep rate/burst policy outside RFB message semantics. RFB must not assume a
  particular packet train or shaper.
- Preserve the ability to tune or disable pacing without rewriting the PS2 RFB
  parser or framebuffer path.
- Distinguish **receive credit** from **burst admission**. Increasing window or
  buffering must not be assumed safe without observing the resulting wire
  behavior.
- Do not enlarge scarce IOP queues merely to absorb arbitrary Linux bursts unless
  measurement shows that is the best tradeoff.
- Future MPEG-2, audio, remote-source, or gateway traffic must be qualified as
  separate/aggregate traffic classes. Do not blindly apply the old RFB shaper to
  everything, and do not assume the PS2 can receive all streams simultaneously at
  their independent single-stream maxima.
- Preserve conventional Linux traffic-control/service mechanisms where they fit;
  any project-specific policy should remain a replaceable provider/lifecycle
  layer.
- When testing performance, ensure observers themselves are accounted for. The
  historical Windows TigerVNC observer roughly halved measured PS2 throughput in
  one controlled comparison and was therefore excluded from performance runs.

## Investigation / requalification plan

When performance networking is active again:

1. Establish a clean deterministic high-change workload with exact PS2 ELF,
   network dependency, Pi provider, and packet-capture authority.
2. Measure the unpaced baseline before changing window sizes or qdiscs.
3. Record TCP window/MSS/options, actual flight/burst distribution, retransmits,
   send-queue behavior, application update cadence, and useful delivered bytes.
4. Re-run a bounded rate/burst matrix using standard Linux shaping so the
   operating envelope can be explored without one PS2 ELF per rate.
5. Compare 64 KiB versus larger native receive credit with **matched** burst
   policy.
6. Where practical, instrument high-water/occupancy or stall timing across
   SMAP -> IOP -> NETMAN/SIF -> EE -> lwIP to narrow the first limiting stage.
7. Test whether packet geometry findings from MI-002 (MTU1458/SMAP residual-tail)
   interact with burst sensitivity or are independent phenomena.
8. Before future media work, measure aggregate coexistence: RFB static desktop +
   MPEG-2 + audio/control rather than extrapolating from RFB-only throughput.
9. Promote only the minimum policy demonstrated necessary. If the clean pipeline
   is stable unpaced at the required workload, omit historical pacing rather than
   preserving it ceremonially.

## Closure criteria

This successor case can close when the project can state:

- whether the clean PS2 receive path still needs sender-side pacing for its target
  workloads;
- the measured operating envelope and qualification workload;
- whether the controlling variable is rate, burst duration, queue occupancy,
  packet geometry, or another measured condition to the degree needed for
  engineering action;
- the correct ownership layer for any required pacing/admission policy;
- whether one policy is sufficient or multiple traffic classes require separate
  budgets;
- which historical constants can be retired and which, if any, remain qualified.

It is acceptable for the exact first lost packet inside undocumented hardware to
remain unknown if a measured, standards-based service contract is sufficient to
make the product reliable and extensible.

## Evidence and authority

Legacy PS2VNC is read-only historical authority.

Primary historical synthesis:

- repository: `Olsens11/PS2VNC`
- branch: `experiment/test15-display-progression`
- `docs/tests/TEST15B_ARCHITECTURAL_CHECKPOINT_20260822.md`
- Resource Maps 1-28
- runtime Implementations 31-36
- historical pacing helper and installer.

Successor references:

- `docs/audit/HISTORICAL_LESSONS.md` L22;
- `docs/audit/B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md`;
- `docs/CLEAN_ARCHITECTURE.md` historical-pacing evaluation note;
- current clean Pi dependency/runtime ledger.

This case does not reopen arbitrary TCP-window tuning. It preserves the old
lesson and defines a disciplined future requalification path.