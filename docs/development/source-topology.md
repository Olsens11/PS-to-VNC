# Clean Product Source Topology Policy

## Status

    POLICY=ACTIVE
    ADOPTED_DATE=2026-09-05
    APPLIES_TO_CLEAN_PRODUCT_SOURCE=YES
    ROOT_SOURCE_FLATTENING=PROHIBITED
    LOCAL_SYMBOL_DICTIONARY=REQUIRED

This is the current file-placement authority for clean PS-to-VNC product
source.

It was adopted during Issue #39 closeout on 2026-09-05 after the clean
implementation had grown enough that the earlier mostly-flat `src/` layout no
longer served the project's local-understanding and directory-dictionary goals.

This document is a forward/current rule. It does not claim that earlier Issue
#7, Issue #38, semantic-audit, migration, or exploratory work used this
directory structure.

## Design rule

> Put a clean product mechanism with the responsibility that owns and
> understands it.

Directory boundaries are earned by coherent responsibility, ownership,
independent testing, hardware lifecycle, or expected local reworkability. Do
not maximize file or directory count for appearance.

Conversely, do not use `src/` as a generic landing zone merely because
`app.c` initializes or invokes a feature.

## Current clean topology

The current clean-generation directories are:

| Directory | Responsibility |
|---|---|
| `src/` | executable entry point and application coordinator only |
| `src/audio/` | synchronous PCM consumption of Transport AUDIO plus the narrow resident AUDSRV stream-operation adapter |
| `src/config/` | pure session CONFIG/profile decoding, validation, immutable owner-specific values, and small config-text helpers |
| `src/diagnostics/` | diagnostics transport and runtime identity |
| `src/display/` | platform-neutral display/presentation conversion |
| `src/framebuffer/` | authoritative CPU-side remote desktop image |
| `src/input/` | controller facts, libpad-facing project use, semantic input, keyboard, mouse |
| `src/media/` | session-scoped common media epoch, signed/saturating deadline math, synchronization contract, and host-testable wait boundary |
| `src/platform/` | genuinely PS2-specific system, network, and graphics mechanisms |
| `src/rfb/` | RFB wire/session parsing, logical-stream adaptation, and complete-message safe-boundary policy |
| `src/transport/` | sole physical PSTV stream/receiver, framing/sequence, logical-channel storage/flow control, and Transport-owned session lifecycle |
| `src/ui/` | local foreground, controller-to-local routing, OSK model/rendering/presentation |

Current clean C/H files directly in `src/` are restricted to:

    src/main.c
    src/app.c
    src/app.h

The root dictionary is:

    src/SYMBOLS.md

Each clean subdirectory owns its own sibling:

    SYMBOLS.md

## Retained pre-refresh source

`src/video/` currently contains retained pre-refresh source. Its physical
presence is not clean-generation adoption.

`src/config/` was deliberately adopted into the clean generation during A002
configuration/profile reconstruction on 2026-09-16. The new `profile.{c,h}`
owns the bounded production CONFIG value boundary; the existing `text.{c,h}`
helpers were adopted behavior-identically as local configuration utilities.
This adoption does not authorize live CONFIG negotiation, audio runtime,
media-clock runtime, or video/MPEG behavior.

`src/media/` was deliberately created during A002 common-media-clock
reconstruction on 2026-09-16 because the shared session epoch is neither
configuration ownership nor a PS2 platform mechanism. `clock.{c,h}` owns only
reusable session timing state/math plus injected synchronization and time
observer contracts. It does not own PCM/AUDSRV runtime, MPEG/video callsites,
or concrete PS2 lock/timer bindings.

`src/audio/` was deliberately created during the A002 synchronous PCM playback
core reconstruction on 2026-09-16. `playback.{c,h}` owns bounded logical AUDIO
consumption, exact immutable PCM profile application, wait-before-play ordering,
truthful post-submit accounting, normal finite exhaustion, and deterministic
stream retirement. `audsrv_service.{c,h}` binds that narrow operation contract
to the resident PS2SDK AUDSRV service without exposing or calling per-session
`audsrv_quit()`. This domain does not yet own a playback worker, startup
reservoir/common-clock timing, application orchestration, MPEG/presentation, or
hardware qualification.

A future display-model or other reconstruction stage may adopt, replace, move,
or delete remaining retained material deliberately. The moment a source file
becomes current clean product source, it must satisfy this policy, the
clean-source synopsis policy, and directory-owned dictionary completeness.

Retained pre-refresh files may also coexist with clean files inside an active
directory. Their presence does not expand clean-generation coverage implicitly.

## Adding a file to an existing domain

When a new clean product file belongs to an existing responsibility:

1. put it in the owning domain directory;
2. give maintained C source/header files the required file synopsis;
3. add every project-defined symbol to that directory's `SYMBOLS.md`;
4. update build/test/include dependencies that refer to concrete paths;
5. regenerate the lightweight dictionary portal when symbol counts change;
6. run the canonical project check and the relevant subsystem/build tests.

Do not move private mechanism into `app.c` or the `src/` root simply to avoid
updating build paths.

## Creating a new clean domain directory

A new clean domain directory is an architectural/topology change, not ordinary
file creation.

The same change must:

1. justify the responsibility boundary against `docs/CLEAN_ARCHITECTURE.md`;
2. update this topology policy and the living file/service map;
3. create the directory's local `SYMBOLS.md`;
4. update the source-topology contract in `scripts/continuity-check.sh`;
5. update build/include/test/tool paths;
6. update relevant architecture/navigation documentation;
7. preserve a chronological decision/reconstruction record when the change is
   significant enough that future agents could otherwise project the new shape
   backward into old history;
8. run long/complete/strict product dictionary discovery before completion;
9. regenerate and verify the product dictionary portal;
10. apply the ordinary PT_LOAD/hardware gate if runtime/build-input changes
    alter the loadable program.

This deliberate friction is intentional. A new top-level responsibility should
not appear accidentally.

## Moving existing clean source

A source move is one coherent change:

- use Git-aware moves where practical;
- move canonical dictionary rows with the owning source;
- update current build/test/tool/documentation paths;
- keep older chronological evidence historically truthful rather than
  rewriting it to the new path unless that older document is itself a living
  current authority;
- verify source bytes when the move is intended to be behavior-preserving;
- distinguish whole-ELF debug/path metadata changes from PT_LOAD changes;
- require hardware qualification or explicit qualification transfer under the
  normal PT_LOAD policy.

Stage-specific build/reproducibility tools require the same historical
discipline. If a later topology or application stage changes current source, an
older stage's object list must not be repaired by absorbing later-stage objects.

A stage-local check encountered outside its owning stage should be innocuous
when possible: report an explicit `SKIPPED` / `NOT_APPLICABLE` result, perform
no stage build, and return success so unrelated later-stage workflows can
continue.

When real evidence from that historical stage is required, run the stage
against its exact recorded source authority through a current-safe wrapper or
isolated checkout and require a genuine PASS result.

The 2026-09-05 continuity audit caught this exact class of problem with the
Issue #7 linked-reproducibility tool after Issue #39 had legitimately expanded
`app.c`.

## Dictionary topology

Directory-owned `SYMBOLS.md` files are the canonical human-maintained
descriptions.

The generated lightweight portal is:

    docs/reference/SOURCE_SYMBOL_DICTIONARIES.md

The comprehensive product index is generated on demand with:

    python3 scripts/source-dictionary.py aggregate

Neither generated view is a second description authority.

The validator structurally rejects a dictionary row that points into a child
directory. `scripts/continuity-check.sh` additionally verifies the current
active domain set, the narrow root-source allowlist, same-directory dictionary
ownership, per-file local coverage, and exact generated-portal synchronization.

Normal active development may still use the dictionary validator's
`ATTENTION` mode for ordinary in-progress symbol-description maintenance. A
completed stage requires the long/complete/strict gate.

## Historical adoption record

The change that established the domain topology is recorded at:

    docs/reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md

That record contains the original before/after context, commit identities,
dictionary snapshot, and qualification-transfer result. The A002 clean
configuration and common-media-clock domain adoptions are recorded by their
immutable reconstruction work logs and the living topology/map updates made
with those source tranches.
