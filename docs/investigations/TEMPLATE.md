# MI-XXX — <Major investigation title>

Status: `OPEN`

GitHub working issue: <link/number>

Opened: <date>

Resolved: <date or N/A>

## Executive summary

State the problem, why it matters, the current best understanding, and whether
present development is blocked or protected by a qualified workaround.

## Symptom and impact

Describe what was observed without prematurely naming a root cause.

## Scope / affected layers

List the product/hardware/protocol layers plausibly involved and the boundaries
already ruled out.

## Historical timeline

Record the important discovery, reproduction, experiment, and corrective steps
in chronological order. Preserve failed theories when they materially explain
why later experiments were chosen.

## What is known

Separate facts by evidence grade. Prefer the project's established classes such
as hardware-proven, test-supported, source-supported, product requirement,
inference, and open question.

## What remains unknown

State uncertainty explicitly. Do not convert a plausible mechanism into fact
merely because a workaround succeeds.

## Hypotheses / dispositions

For each serious hypothesis, record its current state: supported, weakened,
rejected, or untested, and cite the discriminator that changed its status.

## Workaround / corrective action

Describe any current workaround or fix, its exact ownership layer, and whether
it is experimental, qualified, or production authority.

## Design guardrails while open

State what future development must avoid assuming while the issue remains
unresolved. Keep temporary workarounds from leaking into unrelated architecture.

## Investigation plan / next discriminators

Prefer experiments that change one meaningful variable and distinguish competing
mechanisms. Record required DUT identity, apparatus, workload, and qualification
criteria.

## Closure criteria

Define what evidence is sufficient to call the engineering problem resolved.
Resolution may retain explicitly bounded low-level unknowns.

## Final closeout / service bulletin

Fill this section when resolved:

- root cause or reconstructed mechanism;
- corrective contract;
- qualification evidence;
- rejected explanations;
- residual limitations/open details;
- durable lesson for future designs;
- whether any temporary workaround can be removed.

Do not delete the investigative sections when this is filled in.

## Evidence and authority

Link exact issue/PR/branch/commit/test/evidence/source/reference authorities.
Distinguish legacy read-only evidence from successor authority.