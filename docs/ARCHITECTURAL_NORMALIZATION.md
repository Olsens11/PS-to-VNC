# Architectural Normalization Plan

## Starting point

The starting point is the hardware-qualified post-M3Z implementation and
the M4B census proving:

    real translation units: 12
    active semantic leaves: 12
    largest active leaf: 1717 logical lines
    blocking coarse splits: 0

The coarse structural migration is therefore complete.

The remaining migration-era partitions are not automatically split again
because of their size.

## Goal

Transform the qualified migration structure into conventional,
responsibility-oriented embedded C modules without turning the work into a
rewrite.

Behavioral changes and architectural movement should remain separable
whenever practical.

## Step 1: responsibility census

Before moving additional implementation, classify the existing functions,
types, macros, and important state by responsibility.

The census should answer questions such as:

- what belongs to RFB protocol handling;
- what belongs to framebuffer/video presentation;
- what belongs to display profiles and calibration;
- what belongs to UI;
- what belongs to logical input;
- what belongs to configuration;
- what belongs to Pi management;
- what belongs to PS2 platform mechanisms;
- what is genuinely shared.

Historical `core` and `tail` location is not a responsibility.

## Step 2: state-ownership census

Identify important mutable global state and assign one intended owner to
each state group.

Record legitimate readers and writers.

Do not move state simply to reduce a global count. Ownership should match
the subsystem responsible for maintaining its invariants.

## Step 3: dependency graph

Construct the current cross-module dependency graph from real compiler and
source evidence.

Then define the desired dependency direction.

Potential cycles must be examined explicitly before implementation is
moved.

## Step 4: target module map

Map responsibilities to proposed permanent `.c/.h` modules and subsystem
directories.

Review this map before creating the final tree.

The map should be conventional and unsurprising rather than novel for its
own sake.

## Step 5: normalize one semantic domain at a time

For each approved domain:

1. identify current implementation and state;
2. define the smallest deliberate public interface;
3. move implementation into real `.c` modules;
4. keep implementation helpers private;
5. replace migration-only directional headers with domain headers;
6. build reproducibly;
7. compare ELF/load identity;
8. run the required machine/hardware validation for the change class;
9. only then retire the corresponding migration scaffold.

## Step 6: shrink compatibility scope

Compatibility headers are reduced as real modules obtain their own APIs.

Do not replace one broad compatibility header with another broad
application-wide header.

## Step 7: establish host tests

As pure logic obtains clean seams, add host-side tests for areas such as:

- configuration parsing and validation;
- display-profile validation;
- safe-area geometry;
- transaction parsing/state transitions;
- RFB message and rectangle parsing;
- encoding logic that does not require PS2 hardware.

## Step 8: converge source layout

Only after module ownership is established should implementation settle
into the permanent subsystem hierarchy described in `ARCHITECTURE.md`.

The migration evidence remains preserved as history.

The working application source should eventually cease to live conceptually
under a migration/baseline directory.

## Non-goals

Architectural normalization is not permission to:

- rewrite working code unnecessarily;
- introduce abstraction frameworks without need;
- split files to satisfy arbitrary size targets;
- mix unrelated feature development into structural moves;
- rename everything at once;
- weaken binary, reproducibility, or hardware evidence discipline.

## Next action

The first normalization operation is the architectural responsibility
census.

No further implementation movement should precede that census.
