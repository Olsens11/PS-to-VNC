# Ledge Wire Terminology Policy

DOCUMENT=LEDGE_WIRE_TERMINOLOGY
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-20T15:54:52-04:00
SOURCE_COMMIT=SELF
BASED_ON_DOCUMENT_REVISION=0001
SUPERSEDES_DOCUMENT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
TEMPORAL_CLASS=ARCHITECTURE_POLICY_REVISION
TEMPORAL_SEMANTICS=GOVERNING_UNTIL_SUPERSEDED
SCOPE=ledge/h1-all-guns
STATUS=ACTIVE

This document records a terminology-only architecture decision. It changes names used for project-owned architecture concepts going forward; it does not change wire behavior, ownership semantics, runtime behavior, historical evidence, or current reconstruction packet authority.

Where older current documents use endpoint-specific architecture vocabulary such as `PSTV Transport`, `PSTV Session`, or similar project-owned names, interpret the architecture-neutral responsibility using the canonical terms below. Existing committed source identifiers and historical text may remain unchanged until the separately queued bounded migration runs.

## Canonical project-owned terminology

### Wire Transport

**Wire Transport** is the component that owns:

- the physical peer connection/session;
- ordered physical send and receive;
- framing;
- multiplexed logical channels;
- Transport flow control;
- mechanism-level control delivery;
- mechanism-level connection/session failure and teardown facts.

Wire Transport owns mechanism. It does not own generation, presentation, RFB, input, or other business semantics merely because their messages cross the wire.

### Wire Protocol

**Wire Protocol** is the project-owned framed protocol carried by Wire Transport, including:

- header representation;
- frame kinds;
- channel identities;
- physical sequence semantics;
- START/RETIRE wire identities and representations;
- other project-owned framing/control representation.

### Related neutral terms

Use these project-owned terms where the concept is architecture-neutral:

- **Wire Session**
- **Wire Frame**
- **Wire Channel**

## Naming principle

For architecture-neutral project-owned concepts, do not introduce names containing `PS2`, `Pi`, or `PSTV` merely because those happen to be the current deployed endpoints.

Project-owned names should describe responsibility rather than deployment hardware.

This rule does **not** require removing endpoint/platform terms where the specificity is materially true. Preserve externally defined or genuinely hardware-specific names such as `ps2sdk`, `ps2ip`, `libpad`, `GS`, `EE`, `IOP`, `audsrv`, and other real platform/library/API names.

Historical/forensic source and immutable historical logs are evidence and must not be rewritten merely to modernize terminology.

## No opportunistic half-migration

Until the bounded terminology migration runs:

- existing committed identifiers may remain unchanged;
- unrelated functional work must not opportunistically half-rename existing APIs/files/symbols merely to adopt the new vocabulary;
- new architecture-neutral concepts must not introduce additional `PSTV`, `PS2`, or `Pi` terminology when a neutral responsibility name is available;
- existing quoted names in current packets remain interpretable historical/current-transition vocabulary rather than authorization for new endpoint-specific architecture names.

The former `A003-P2A-MANUAL-START-PREPARED-R1` packet is historical and superseded. Current packet authority is published only by the current `LEDGE_FOREMAN_STATE.md`. Terminology migration remains separate and must not be mixed into functional reconstruction.

## Wire compatibility invariant

Terminology migration must not change wire behavior. Preserve exactly unless separately authorized by a behavior packet:

- wire magic bytes;
- protocol version;
- frame kinds;
- channel IDs;
- header representation;
- sequence behavior;
- START identity and payload layout;
- RETIRE identity and payload layout;
- all already accepted wire behavior.

Names and source organization are not permission to alter representation or semantics.

## Migration classification rule

The later bounded migration must classify each candidate occurrence before changing it:

- **A. project-owned architecture term** -> migrate to canonical Wire terminology;
- **B. external/platform named entity** -> preserve;
- **C. historical/forensic evidence** -> preserve;
- **D. stable wire-format identifier/constant** -> preserve where compatibility requires;
- **E. ambiguous occurrence** -> inspect and decide from responsibility/evidence rather than mechanically replace.

Do not perform global string replacement.

## Explicit naming questions deferred to the migration

The migration must inspect rather than assume:

- whether the `pstvnc_` prefix is functioning as a stable project namespace or encodes deprecated `PSTV` component terminology;
- whether `src/transport/` is already sufficiently responsibility-based and should remain unchanged.

Neither is pre-decided by this terminology policy.

## Relationship to existing architecture authority

`docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003` remains authoritative for ownership, bridge, concurrency, lifecycle, and behavior rules.

This document supersedes only conflicting **project-owned architecture terminology** in older current authority. It does not reinterpret endpoint-specific statements when the endpoint itself is genuinely relevant, and it does not rewrite historical evidence.

The binding architecture rule remains:

> Inside one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Under the canonical terminology, the architecture-neutral mechanism owner described by that rule is **Wire Transport**.
