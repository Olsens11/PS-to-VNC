# MI-004 — Display reconstruction liveness and green-corruption failures

Status: `OPEN_NON_BLOCKING`

Historical campaigns: legacy F8J2/B3C/B4A display-transition work and successor
M3Y migration qualification

Successor working issue: to be linked from the casebook index

## Executive summary

PS2VNC accumulated a small but important family of failures during temporary
display-mode reconstruction back to the ordinary 480p baseline. The visible
symptom was often green/corrupted output or a frozen transition, while the
machine-side display-control transaction stopped making expected progress.

These observations must **not** be collapsed into one proven root cause.
Different experiments demonstrated different discriminators:

- one matched historical B3C experiment showed sustained controller activity
  could reproducibly distinguish a failing multi-mode reconstruction run from a
  hands-off pass;
- later hard controller/queue isolation improved the ownership contract, but a
  B4A hands-off run still produced a rollback stall inside the preserved display
  switch core, proving controller activity was not a sufficient universal cause;
- changing mode order allowed the same B4A candidate to pass all five modes,
  weakening a simple "720p restore is inherently broken" explanation;
- during successor M3Y qualification, the exact same M3X ELF failed once during
  the final 1080i -> 480p restore and then passed the complete five-mode sequence
  after reboot/explicit stale-state cleanup.

The successor classification is therefore appropriately conservative:

> **intermittent display-transition liveness failure with unresolved exact root
> cause**.

This does not block the current minimal clean PS2 core. It does create a future
qualification obligation: when multi-mode display reconstruction is rebuilt,
its ownership, RFB epoch, presentation/GS progress, controller quiescence, and
durable transaction generation must be instrumented from the start rather than
assuming that a successful single transition proves the whole state machine.

## Scope

This case concerns the **cross-machine display reconstruction transaction**:

```text
Pi/display-control request
  -> PS2 transaction acquisition
  -> controller/input ownership change where applicable
  -> RFB stream/request epoch interruption or replacement
  -> GS/display-mode reconstruction
  -> authoritative framebuffer acquisition/presentation
  -> destination stabilization
  -> durable RESTORED result / acknowledgement
  -> return to normal input/RFB operation
```

A failure can manifest as a green or static screen while some other layers remain
alive. The case therefore deliberately spans application control, RFB ownership,
input/controller state, GS/display reconstruction, and durable Pi transaction
state.

This is **not** the already-resolved Test14 HSync/`ExitHandler()` case. MI-001
proves a specific HIRES interrupt-delivery contract. The evidence here does not
show that every later green reconstruction failure is another instance of that
same mechanism.

## Historical lineage

### Healthy control: B3B proved ordinary 1080i round-trip reconstruction can work

Before the later activity-sensitive failures, B3B established important healthy
controls.

A 480p -> 1080i transition followed by explicit rollback returned to live 480p,
with deliberate damage probes and controller motion proving restored RFB and
presentation activity.

A separate B3B run allowed the PS2-owned lease to expire autonomously. The PS2
aborted the old in-flight receive, rebuilt 480p through a replacement RFB
transport, and resumed live framebuffer/controller activity.

Thus there was no universal defect saying "1080i cannot restore to 480p" or
"automatic lease rollback is always broken."

### B3C matched input-activity experiment

A later matched fresh-generation A/B deliberately changed controller workload
while holding DUT, configuration, mode sequence, harness, timing, and generation
policy fixed.

Hands-off side:

```text
480i        PASS
480p-hires  PASS
576i        PASS
720p        PASS
1080i       PASS
```

Continuous left-analog side:

```text
480i        PASS
480p-hires  PASS
576i        PASS
720p        entered ACTIVE_TEMP and began rollback
then         transition failure / telemetry stopped
```

Last preserved PS2 telemetry in the failing side included:

```text
frame=249
WAIT_RFB on rectangle xywh=1185,670,18,9
POST_RECEIVE bytes=-1
480p reconstruction geometry
GS_INIT for 704x462 Standard framebuffer
then no further PS2 UDP telemetry
```

Pi management subsequently recorded the tx4 720p transaction as RESTORED. The
harness then created tx5 requesting 1080i, but PS2-facing management evidence did
not show the PS2 acquiring tx5. The operator visually believed a later 1080i
transition had occurred before the green freeze; the experiment correctly kept
that physical observation separate from the stronger transaction identity.

Supported conclusion at that checkpoint:

> sustained controller workload was a reproducible discriminator under the
> matched B3C setup.

Visible-pointer suppression alone was not sufficient. The next investigation
was RFB/reconstruction epoch ownership, especially queued/outstanding
controller-driven RFB state crossing the reconstruction boundary.

### Isolated hostile-stick 720p control

Another isolated 720p automatic-rollback run under continuous stick activity
completed successfully: the cursor was intentionally suppressed during remote
transaction ownership, 720p displayed, rollback returned to clean 480p, and
cursor movement resumed after ownership release.

That result matters because it weakens a simplistic equation of:

```text
controller activity + 720p rollback = guaranteed failure
```

Sequence/history/epoch conditions were relevant candidates.

### B4A — hard transition curtain / controller ownership

B4A strengthened the ownership contract around each one-way remote display
transition:

1. acquire controller ownership;
2. stop libpad polling and controller-derived pointer/button/key/hotkey activity;
3. wait for pause acknowledgement;
4. discard the controller-to-RFB queue;
5. draw a `CHANGING DISPLAY MODES` curtain;
6. settle for 500 ms;
7. enter the preserved display-switch core;
8. reconstruct the destination;
9. draw/stabilize the destination curtain;
10. explicitly present the coherent destination desktop;
11. release controller ownership only after clearing stale edge/hold/analog/
    scroll/UI history.

This is an important durable lesson even though it did not explain every failure:
controller ownership across a hazardous transition is a transaction, not merely
"ignore visible pointer movement."

### B4A hands-off failure inside the preserved switch core

A B4A original-order hands-off sweep nevertheless failed during a 720p rollback.
Phase instrumentation showed:

```text
phase 1
phase 2
phase 3
GS_INIT while still phase 3
(no phase 4)
(no RESTORED)
```

Source mapping placed phase 3 immediately before the preserved
`ps2vnc_switch_display_mode_core()` call and phase 4 only after successful return.
Thus the visible stall manifested **inside the preserved switch core before
return**. Destination curtain, coherent final presentation, and controller
release were not reached.

That does not prove B4A's pre-core conditions were irrelevant, but it proves that
controller activity is not required for the entire failure family.

### B4A reordered sweep

With the candidate/apparatus otherwise unchanged, a reordered hands-off sweep put
720p first:

```text
720p, 480i, 480p-hires, 576i, 1080i
```

All five temporary transactions and all five returns to 480p passed physically
and mechanically.

This was a valuable discriminator:

- 720p successfully restored when placed first;
- 576i successfully occupied the fourth ordinal position where the earlier run
  had failed with 720p;
- 1080i also completed fifth.

Therefore neither "720p restore always fails" nor "the fourth transition always
fails" explains the observations.

### Later B4A steady-state hands-off qualification

A later Timing-V2 hands-off benchmark recorded five machine passes across:

```text
480i, 480p-hires, 576i, 720p, 1080i
```

with the final transaction state returning to fresh IDLE and independent witness
activity continuing after the run. This further established that the transition
machinery could complete the full sequence successfully under some matched
conditions.

### Successor M3Y — exact same ELF, intermittent final restore failure

During successor migration qualification, M3Y ran the same five-mode sequence
from a 480p baseline.

Attempt 1:

- 480i passed;
- 480p-hires passed;
- 576i passed;
- 720p passed;
- during final 1080i -> 480p restore, the screen became almost entirely green
  except near the bottom;
- machine state reached `RESTORING` and never reached `RESTORED` before timeout.

The failure was preserved rather than erased.

The Pi-side VNC TCP connection still existed in the captured failure snapshot,
with queued data visible on the Pi socket, but absence of subsequent PS2 progress
telemetry prevents that from proving where the PS2 itself stopped.

After a full Pi and PS2 reboot plus explicit archival/removal of the abandoned
control record, the **exact same M3X ELF** ran the exact same five-mode harness
and passed 5/5 with full physical pass.

The successor correctly classified this as:

```text
INTERMITTENT_DISPLAY_TRANSITION_LIVENESS
```

rather than claiming a deterministic M3X regression.

## Separate but important aftermath: stale durable transaction state

The first M3Y failure left this durable state behind:

```text
RESTORING
txid=1
mode=1080i
baseline=480p
```

That abandoned record survived:

- management-service restart;
- Raspberry Pi reboot;
- PlayStation 2 reboot.

It prevented a new independent display-control test until the exact stale state
was archived and deliberately cleared.

This is classified separately from the physical green/liveness failure: it is a
**recovery/control-plane design defect** exposed by the failure.

Durable requirement:

> A fresh PS2VNC runtime generation must not inherit an abandoned hazardous
> display-control transaction from a previous runtime generation merely because a
> file says `RESTORING`.

Session generation, boot identity, lease ownership, or an equivalent explicit
stale-state reconciliation mechanism must distinguish live authority from
orphaned historical transaction state.

## What is known with high confidence

- Healthy 480p <-> high-resolution reconstruction is possible; neither 1080i nor
  automatic rollback is universally broken.
- At least one matched B3C experiment proved sustained controller workload was a
  reproducible discriminator.
- Suppressing visible pointer motion is weaker than acquiring/quiescing controller
  ownership and invalidating stale input state.
- Hard controller/queue isolation alone does not explain every failure: a B4A
  hands-off run still stalled in the preserved switch core.
- In the B4A hands-off failure, phase evidence localized the non-return to inside
  the preserved display switch/reconstruction core, after GS_INIT and before the
  post-core phase.
- Mode identity alone is insufficient: 720p restored successfully in other
  isolated/reordered runs.
- Simple ordinal position alone is insufficient: another mode passed in the same
  ordinal position during the reordered run.
- Successor M3Y observed an intermittent 1080i -> 480p restoration failure using
  an ELF that subsequently passed the exact complete five-mode harness.
- A failed transition can leave durable Pi control state that survives both
  machines rebooting unless stale-generation ownership is explicitly reconciled.

## What remains unknown

We do **not** yet know whether the B3C, B4A, and M3Y failures share one physical
root cause.

Plausible classes still include:

1. RFB receive/request epoch ownership crossing synchronous reconstruction;
2. stale queued input or another producer perturbing the transition at a
   vulnerable time;
3. GS/display reconstruction liveness or synchronization beyond the already
   resolved MI-001 HSync contract;
4. mode-sequence/history-dependent state not fully reset between profiles;
5. stale framebuffer/texture/cache or geometry ownership during reconstruction;
6. network receive timing interacting with a synchronous transition;
7. a race among several of those rather than one isolated defect.

The evidence does not justify selecting one of these as the root cause today.

## Relationship to MI-001 / ExitHandler

MI-001 established a specific HIRES HSync interrupt-delivery problem and a
hardware-validated callback ownership contract.

The present case must retain that lesson, but a green screen is not evidence by
itself that the same interrupt defect has recurred. Future instrumentation should
observe HSync/GS progress as one layer in the classification ladder instead of
assuming either guilt or innocence.

## Current successor disposition

This is non-blocking for the current minimal reconstruction because the clean core
is intentionally not rebuilding the full historical display transaction yet.

When display-mode switching returns, it should be rebuilt from the durable
behavioral contract rather than copied from B4A's large historical state machine.
The case exists so the new implementation does not mistakenly qualify a single
successful transition and forget the old sequence/workload/intermittency evidence.

## Design guardrails while open

- Display switching remains an application-owned cross-machine transaction, not a
  raw GS mode-set call.
- The active mode/profile, RFB logical geometry, presentation geometry,
  framebuffer authority, and safe-area calibration remain separate concepts.
- A replacement RFB epoch begins from authoritative full state; do not continue
  incremental traffic on uncertain local representation.
- Controller/libpad ownership must be explicitly quiesced across hazardous
  reconstruction and stale logical input must be invalidated before release.
- Do not use remote pointer movement or unrelated RFB damage as the primary local
  transition/UI wake mechanism.
- Durable transaction state needs runtime-generation ownership and stale-state
  reconciliation.
- Do not automatically recover unexplained silent stalls while debugging them;
  preserve the failed state long enough to classify the lowest layer still
  making progress.
- Do not equate a green/static image with total process death.
- Do not call one mode inherently defective without a matched mode-order/sequence
  discriminator.
- Do not assume a hands-off pass eliminates race conditions; the M3Y exact-ELF
  repeat demonstrates genuine intermittency.

## Investigation plan when display reconstruction returns

### 1. Start with a clean minimal transaction

Qualify one mode pair from a fresh 480p runtime generation before adding a
five-mode loop. Record exact PS2 ELF/PT_LOAD/runtime identity, Pi control-plane
identity, RFB provider/session identity, and physical observation.

### 2. Instrument the ownership boundaries from the start

Use compact progress markers/counters around:

```text
request acquired
controller quiesced
RFB old epoch abandoned/closed
new display profile admitted
GS mode reconstruction begin/end
RFB replacement connected/negotiated
authoritative full framebuffer received
presentation coherent
RESTORED published
controller released
```

Also retain independent heartbeats for application/main loop, controller, RFB
receive, and relevant GS/HSync progress so a static screen cannot hide which
owner stopped.

### 3. Recreate discriminating matrices rather than random stress

Vary one axis at a time:

- mode pair;
- mode order / number of preceding transitions;
- hands-off versus controlled controller workload;
- quiet desktop versus deterministic RFB damage load;
- explicit rollback versus lease/failure rollback;
- fresh RFB generation versus any permitted reuse;
- fresh runtime generation versus repeated transaction sequence.

### 4. Preserve failures without poisoning the next experiment

On timeout/freeze, capture the failed state before recovery. Then start the next
experiment with a **new explicitly identified runtime/control generation** so the
old `RESTORING` state cannot become an uncontrolled variable.

### 5. Bisection if the failure returns

Use the project's layered method:

- display reconstruction with network receive discarded;
- RFB replacement/full-state receive without GS publication;
- synthetic GS/profile switching without live network traffic;
- controller quiesced versus controlled workload;
- repeated exact pair versus multi-mode sequence.

The goal is to identify the lowest owner that still demonstrably progresses.

## Closure criteria

This case can close when the clean implementation has either:

1. reproduced and isolated the transition-liveness mechanism well enough to state
   its corrective contract; **or**
2. replaced the historical implementation with a simpler ownership model and
   passed a qualification matrix strong enough to cover the known historical
   discriminators without recurrence.

At minimum, closure qualification should include:

- repeated fresh-generation pair transitions;
- multi-mode sequences in more than one order;
- hands-off and controlled input workload;
- deterministic RFB change workload;
- explicit and safety/failure rollback paths that the product retains;
- exact runtime identity and failure-progress witnesses;
- stale transaction state across process/machine restart;
- physical/operator validation separate from machine state.

If no failure can be reproduced in the clean design, the historical root cause
may remain unknown, but the report should explicitly say that the old mechanism
was superseded by a simpler independently qualified transaction model rather than
pretending the historical observations never happened.

## Evidence and authority

Legacy PS2VNC is read-only historical authority.

Relevant legacy branch/evidence lineage includes:

- `Olsens11/PS2VNC`
- branch `experiment/test15E3K-D17AL-F-display-cleanup`
- F8J2 B3A/B3B/B3C/B4A transaction experiments;
- matched B3C hands-off/continuous-left-analog benchmark;
- B4A transition-phase instrumentation and reordered sweep;
- associated `docs/TEST_HISTORY.md`, display/input architecture records, and
  forensic run directories.

Successor evidence:

- `evidence/m3/m3y-hardware-20260830-102350/CLASSIFICATION.md`;
- M3Y attempt-1 failure snapshot and exact retry evidence;
- successor durable historical lessons around display transactions, controller
  ownership, RFB full-state replacement, and failure classification.

This case is a synthesis/index. It does not rewrite legacy evidence or promote a
single unproven root-cause theory.