# Module Lifecycle Contract

## Purpose

This document defines the default lifecycle rules for PS-to-VNC modules that
perform cross-Wire work.

The central rule is:

> **Wire owns communication validity. Modules own behavior.**

Wire Transport decides whether cross-Wire work is valid. Each module decides
whether and when it should run.

These responsibilities remain separate.

## Wire is the containing communication lifetime

A cross-Wire module may perform remote work only while a valid Wire Session
exists.

Loss of that session permanently ends that communication lifetime. A later
connection establishes a new Wire Session; the dead session is never resumed.

Modules do not need to know or store the Wire Session ID merely to obey this
rule.

## Module generation is normally a lifecycle fact

In architecture discussion, one uninterrupted period during which a module is
running may be called a generation.

For the ordinary case, this does not imply that the implementation needs a
generation counter, generation ID, copied Wire Session ID, or globally unique
module-run identity.

The ordinary lifecycle is simply:

    STOPPED
       |
       | module-owned start condition becomes true
       v
    STARTING
       |
       v
    RUNNING
       |
       | module stop, Wire loss, or module failure
       v
    STOPPING
       |
       | all instance-owned work has retired
       v
    STOPPED

A module may implement fewer explicit states when its behavior is simple. The
invariant matters more than the enum shape.

## One ordinary running instance at a time

A module completely stops its current running instance before starting its next
running instance.

For the default case:

> **Stop completely before restart.**

Completely stopped means that no work owned by the previous instance can later:

- mutate the module's live runtime state;
- enqueue or dequeue work for a replacement instance;
- publish readiness or completion into replacement-instance state;
- invoke callbacks against replacement-instance state;
- access resources after those resources have been reassigned;
- perform new cross-Wire work through replacement-session authority.

Once complete retirement is established, ordinary module storage may be reused
for the next instance.

If a particular subsystem cannot establish complete retirement before restart,
that subsystem has earned additional module-specific fencing. Such machinery is
not added project-wide in advance of a demonstrated need.

## Wire availability is a prerequisite, not universal module policy

An active Wire Session means cross-Wire communication is possible.

It does not universally mean every distributed module must start.

Every module owns its own start condition.

Conceptually:

    Wire ACTIVE
        |
        +-- makes cross-Wire work possible
        |
        +-- module independently evaluates its own start condition

Whether Wire activation itself participates in that start condition is a
module-specific contract.

### RFB

RFB is expected to establish for each usable Wire Session.

Wire loss stops the current RFB instance. A replacement Wire Session uses the
same ordinary RFB startup path. It does not resume, migrate, or rebind the dead
instance.

### PCM/audio

PCM/audio is expected to establish for each usable Wire Session according to
its ordinary prerequisites.

Wire loss stops the current audio instance. A replacement Wire Session uses the
normal audio startup path.

### Input and mouse forwarding

Local controller/input acquisition is a PS2-local facility and need not die
merely because Wire is unavailable.

Remote input forwarding is cross-Wire behavior. It stops when Wire is
unavailable and starts again through its ordinary policy when a suitable
replacement Wire Session exists.

This distinction allows local UI/input behavior to remain alive while
disconnected.

### MPEG

MPEG is Wire-dependent, but Wire Session establishment is not itself the MPEG
start trigger.

Current policy requires the MPEG-specific user initiation/calibration path.
Future policy may allow MPEG-owned automatic detection or another autonomous
trigger.

The rule is:

    Wire ACTIVE
        +
    MPEG-owned start condition
        =
    MPEG may start

Wire loss stops the current MPEG instance.

When a replacement Wire Session becomes active, MPEG remains stopped until its
own start condition is satisfied again.

This generalizes: Transport availability does not own module behavior.

## Transport owns cross-Wire transaction validity

A module is not responsible for comparing Wire Session IDs in order to prevent
cross-session traffic.

Transport owns that correctness boundary.

When a running module receives access to cross-Wire Transport, that access is
valid only for the Wire Session in which Transport issued it. Every operation
Transport admits through that access belongs permanently to the same validity
domain.

A useful mental model is a round-trip ticket:

    Wire Session A active
        |
    module submits cross-Wire work
        |
    Transport admits the work under A
        |
    work has an A-only round-trip ticket

The concrete implementation need not expose a literal ticket or numeric session
identity to the module.

Transport may use an opaque access object, runtime identity, capability, epoch,
pointer identity, or another internal mechanism that preserves the same
invariant.

## A Transport ticket is never transferable

Once Transport admits an operation under Wire Session A, that operation belongs
permanently to A's validity domain.

If A ends, outstanding A work becomes terminal.

This applies regardless of where the work is when session loss becomes known:

    queued outbound work      -> cannot enter a replacement session
    partly processed send     -> cannot continue through a replacement session
    waiting response          -> becomes terminal
    late response             -> cannot affect replacement-session state
    late completion           -> cannot complete as replacement-session work
    stale module operation    -> rejected through its old Transport authority

A new Wire Session B creates a new Transport validity domain.

No A operation may be migrated, rebound, retried automatically, completed, or
silently redirected through B.

## Transport is the final fence

Modules should normally stop promptly when Wire becomes unavailable.

That is the normal lifecycle path.

Transport nevertheless remains the final correctness fence, including when
module work races with shutdown.

For example:

    module instance is running under Wire A
        |
    Wire A dies
        |
    module worker has not observed shutdown yet
        |
    Wire B establishes
        |
    old worker attempts cross-Wire work

The old worker must not gain access to B merely because B is now current.

Its old Transport access is terminal. Work trying to leave through that access
is rejected before it can become B business, and any old work returning later
is rejected before it can affect B.

The module need only interpret the terminal Transport condition as a reason to
stop its current instance. It does not need to know the Wire Session number.

## Transport fencing does not replace module retirement

Transport and module lifecycle defend different boundaries.

Transport owns whether a cross-Wire operation may use the current Wire Session.
The module owns whether work from its old running instance may still exist
locally.

That means the default contract is deliberately two-part:

    Transport:
        old Wire authority cannot become replacement-session authority

    Module:
        old running-instance work is completely retired before a replacement
        module instance starts or old module resources are reassigned

Transport is therefore allowed to rely on the module's complete-stop invariant
instead of maintaining a project-wide generation/refcount scheme merely to
police module retirement.

An operation whose Wire exchange completed while Session A was still valid may
finish as the last valid A work. If Wire A ends before a further cross-Wire leg
can complete, Transport returns a terminal result rather than completing through
Session B. Any local continuation that still exists after that point belongs to
the old module instance and must disappear as part of that module's stop.

The stale-Transport-access fence remains a backstop: if old module code tries to
start new cross-Wire work after A has ended, even after B exists, the old access
is terminal and cannot be retargeted to B.

If a module cannot prove complete local retirement before replacement startup,
that module has earned module-specific fencing. Transport does not grow a
generic module-lifecycle mechanism to compensate for that module deficiency.

## Normal restart, not reconnect-specific migration

Reconnect does not create a second lifecycle implementation inside each module.

The intended pattern is:

    first usable Wire Session
        |
    ordinary module start
        |
    ordinary module running
        |
    Wire loss
        |
    ordinary module stop
        |
    complete retirement
        |
    replacement Wire Session
        |
    ordinary module start if its normal start conditions are true

The hundredth reconnect exercises the same module start/stop mechanisms as the
first usable connection.

Avoid module APIs whose purpose is merely to adopt a replacement Wire Session,
migrate a running instance, resume an old instance across reconnect, rebind old
runtime state, or patch a new session identity into an old instance.

## Durable state is not running-instance state

A module may have configuration or other durable state that survives Wire loss.

Examples include saved configuration, confirmed calibration, user preferences,
and other explicitly persistent module data.

A new module instance may reconstruct itself from durable state after its normal
start condition is satisfied.

It does not inherit opaque live runtime state from the dead instance.

## When IDs or epochs are justified

This contract does not ban IDs, epochs, generations, or sequence numbers.

They are appropriate when the owning subsystem has a real independent need for
them, for example overlapping asynchronous work that cannot be fully retired,
protocol-level sequence identity, decoder/presentation work requiring explicit
stale-completion rejection, or durable correlation required by a module's own
protocol.

The rule is:

> Do not add a Wire Session ID or module generation ID merely because reconnect
> exists.

Extra identity is earned by the subsystem that actually needs it.

## Minimum requirements for a cross-Wire module

Every cross-Wire module must satisfy these minimum requirements:

- own its start condition and stop condition;
- treat Wire loss or terminal Transport access as a stop condition for the
  affected cross-Wire behavior;
- have at most one ordinary running instance at a time;
- completely retire the old instance before a replacement instance starts or
  its live resources are reassigned;
- ensure no old worker, callback, queued item, or local completion can publish
  into replacement-instance state after stop completes;
- acquire fresh Transport access through ordinary startup rather than rebinding
  or migrating old access;
- keep only explicitly durable state across instances;
- add module-specific identity/fencing only when complete retirement cannot be
  established by ordinary lifecycle ownership.

These are module-owned requirements even though Transport documentation depends
on them when describing the complete cross-session safety argument.

## Required lifecycle description for a module

Before implementing or materially changing a module that performs cross-Wire
work, identify the answers to these questions:

1. Is the module local-only, Wire-dependent, or mixed?
2. What exact condition starts its running instance?
3. Is Wire activation itself a start trigger, or only a prerequisite?
4. What conditions stop the running instance?
5. What happens immediately when Wire becomes unavailable?
6. What local work, if any, may continue while disconnected?
7. What proves the running instance has completely stopped?
8. Can any asynchronous work remain after stop completes?
9. What state is durable and may survive into a future instance?
10. What runtime state must die with the current instance?
11. How does its Transport access become terminal when the containing Wire
    Session ends?
12. Does restart use the same ordinary startup path?
13. Does the module genuinely need an ID or epoch for a module-specific reason?

If these answers are unclear, the lifecycle design is not ready for
implementation.

## Review invariant

A reviewer working on a cross-Wire module should be able to state:

    Wire owns communication validity.
    The module owns whether it runs.
    Only one ordinary module instance runs at a time.
    The old instance completely stops before restart.
    Old Transport work cannot cross into a replacement Wire Session.
    A replacement session permits ordinary startup; it does not resume old runtime.
    IDs exist only where an owning subsystem genuinely needs them.

That is the default PS-to-VNC distributed-module lifecycle contract.
