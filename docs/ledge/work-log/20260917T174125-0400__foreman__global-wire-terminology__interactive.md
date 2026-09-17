# Foreman shift — record Wire terminology and defer bounded migration

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-17T17:41:25-04:00
COMPLETED_AT=2026-09-17T17:43:02-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=global-wire-terminology
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=4aff9d920fd61895a1aab0d9b8f09d004fc3537f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Record the user's architecture-neutral naming decision now without consuming or derailing the active user-assisted A003 engineering packet. Establish canonical Wire terminology, queue one later bounded migration, preserve all wire behavior and historical/platform-specific names, and leave tonight's functional work active.

## Authority consumed

The shift refreshed live `ledge/h1-all-guns` authority at `4aff9d920fd61895a1aab0d9b8f09d004fc3537f` and consumed current Foreman state revision `0019`, architecture overlay revision `0003`, the active A003 manual authority revision `0002`, and work-log contract revision `0006`.

## Architecture terminology decision

Created:

`docs/ledge/LEDGE_WIRE_TERMINOLOGY.md`

revision `0001`, committed as:

`53274f12dcd4988a8e77985db20a4c7a96d97ba8` — `docs(architecture): adopt neutral Wire terminology`

Canonical project-owned architecture-neutral vocabulary is now:

- `Wire Transport`;
- `Wire Protocol`;
- `Wire Session`;
- `Wire Frame`;
- `Wire Channel`.

The policy prohibits introducing `PS2`, `Pi`, or `PSTV` into new architecture-neutral project-owned names merely because those are the present deployment endpoints. It explicitly preserves real platform/library/API names, historical/forensic evidence, immutable historical logs, and stable wire identifiers where compatibility requires.

## No behavior change / no current-session migration

No product source or runtime identifiers were renamed in this Foreman shift. No wire bytes, versions, frame kinds, channel IDs, header representation, sequence behavior, START/RETIRE identity/layout, or accepted behavior changed.

Existing committed identifiers may remain until the bounded migration. Unrelated functional work must not perform opportunistic partial renames.

The active `A003-P2A-MANUAL-START-PREPARED-R1` packet remains active and unchanged in behavioral scope.

## Foreman state advance

Updated `docs/ledge/LEDGE_FOREMAN_STATE.md` to revision `0020`:

`fc51376c28763a037a1b9d98f9fba754aac6a3e9` — `docs(foreman): queue Wire terminology migration`

Revision `0020` adds only the terminology interpretation and deferred migration queue. It keeps the active A003 manual packet, user discussion gate, acceptance criteria, non-goals, A004 hold, and later lifecycle deferrals intact.

## Deferred bounded migration

Queued but **not activated**:

- `MIGRATION_ID=WIRE-TERMINOLOGY-MIGRATION-R1`
- `WORK_ITEM_KEY=global-wire-terminology`
- `PACKET_STATUS=DEFERRED__DO_NOT_WAKE_DURING_ACTIVE_A003_MANUAL_SESSION`

The later migration must classify each candidate occurrence before changing it:

- A: project-owned architecture term -> migrate;
- B: external/platform named entity -> preserve;
- C: historical/forensic evidence -> preserve;
- D: stable wire-format identifier/constant -> preserve where compatibility requires;
- E: ambiguous -> inspect.

It must specifically inspect rather than assume whether `pstvnc_` is a project namespace or deprecated component terminology, and whether `src/transport/` is already sufficiently responsibility-based. Global mechanical replacement is prohibited.

## Exact next pickup

Tonight's interactive A003 assistant continues its existing packet. It should refresh current authority, read `LEDGE_WIRE_TERMINOLOGY.md` revision `0001`/Foreman state revision `0020`, avoid introducing new endpoint-specific architecture names when a neutral responsibility name is available, and otherwise continue the active functional work without spending the session on terminology migration.

The deferred migration is picked up only after a later Foreman explicitly activates it.
