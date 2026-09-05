# 2026-09-05 Clean Source Topology Adoption

## Record status

    DATE=2026-09-05
    RECORD_ROLE=CHRONOLOGICAL_RECONSTRUCTION_HISTORY
    ISSUE_CONTEXT=ISSUE_39_CLOSEOUT
    RETROACTIVE_HISTORY_REWRITE=NO

This record documents when the current clean product source topology actually
changed.

It does not claim that this structure existed during the semantic audit, the
initial clean-architecture proposal, Issue #7, Issue #38, or the earlier parts
of Issue #39.

## Trigger

Issue #39 added keyboard, OSK, local foreground, controller routing, and related
input/runtime mechanisms. By closeout, most clean product files still lived
directly in `src/`.

That arrangement was mechanically valid but defeated the intended local
dictionary model: `src/SYMBOLS.md` had grown to 1,426 rows and a developer
working on one coherent module family had to navigate the effective
project-wide dictionary.

The project therefore distinguished two separate requirements:

1. the comprehensive symbol index remains an aggregate generated from local
   dictionaries; and
2. the source tree itself should expose earned responsibility families so a
   nearby `SYMBOLS.md` is genuinely local.

## Pre-change authority

Immediately before the topology reorganization, the Issue #39 implementation
authority was:

    904483bc4547939e60d85b13bd7b5c7bc2208610

The clean implementation was still primarily flat under `src/`, except for
already-existing platform/diagnostics and retained pre-refresh directories.

## Topology change

Commit:

    0b3e20c7c4840073618a1bb487d554bc3309d057

Commit message:

    reconstruct: organize clean source by domain

Thirty-two C/H paths moved into earned responsibility directories.

The resulting active clean directories were:

    src
    src/diagnostics
    src/display
    src/framebuffer
    src/input
    src/platform
    src/rfb
    src/ui

The root was reduced to the executable/application files:

    src/main.c
    src/app.c
    src/app.h

The move preserved the moved source bytes. Two PS2 platform source files
required include-spelling repairs because their old relative include paths
referred to headers that had moved:

    src/platform/ps2_network.c
        ../rfb_io.h -> rfb_io.h

    src/platform/ps2_graphics.c
        ../display.h -> display.h

Those were include-location changes only.

## Hardware qualification transfer

The topology state was built twice at the same fixed build path.

Whole ELF:

    SHA256=971a66385477f98daf112ff4522afaf1006967ad7d024027384af5292f111916
    BYTES=2610680

The pristine loadable payload was exactly the previously hardware-qualified
Issue #39 HW2 payload:

    PT_LOAD_SHA256=512f5220c2be2c001c8641975c2045b3eb2319b4d384a0a311906e012944b85b
    PT_LOAD_BYTES=403080

Therefore the directory/path reorganization did not alter the loadable PS2
program and existing HW2 qualification transferred objectively.

## Dictionary reorganization

The existing machine-clean symbol inventory was partitioned by the new owning
directories. The validator was strengthened so a parent dictionary could no
longer document source definitions located in a child directory.

The 2026-09-05 post-partition snapshot was:

| Directory | Entries |
|---|---:|
| `src` | 113 |
| `src/diagnostics` | 69 |
| `src/display` | 17 |
| `src/framebuffer` | 94 |
| `src/input` | 495 |
| `src/platform` | 117 |
| `src/rfb` | 339 |
| `src/ui` | 351 |
| **Total** | **1595** |

These numbers are a dated snapshot. Future current counts belong to the
generated portal, not to this historical record.

## Semantic curation

The directory-local dictionaries were subsequently curated at:

    5d67c60fc273500f7f3b7082e2c2f7b9b7ce389c

Commit message:

    docs: curate Issue 39 source dictionaries

That pass improved 263 high-value descriptions without changing runtime source.

A genuine comprehensive long/complete/strict definition audit passed with all
43 current clean product source/header files covered.

## Post-adoption continuity/tooling audit

The same-day continuity audit intentionally tested tools outside the ordinary
Issue #39 build path.

Host tests, product dictionary discovery, the generated portal, canonical
project checking, and strict current PS2 translation-unit compilation all
survived the topology move.

One historical-stage tool exposed a separate contract problem:

    scripts/check-issue7-linked-reproducibility.sh

Direct execution from the Issue #39 checkout attempted to compile the current
Stage-2 `src/app.c` but link only the historical Issue #7 object set. Later
keyboard, local-UI, OSK, input-runtime, and controller symbols therefore failed
at link time.

The failure did **not** mean Issue #7 or the topology move was broken. Re-running
the unmodified historical Issue #7 tool from exact source authority:

    d94c9280035e288ccbec692ea21e89d3ffb4ffec

reproduced the recorded final Issue #7 identities exactly:

    ELF_SHA256=39984bfb5e30a796c07c4cfabf1521084efd6dfad0dbb657f9747bbb322ae796
    PT_LOAD_SHA256=17a36c3ce541b5dd0b309163e0ecfd4b9f9d8284bbbc5ca1787b2738fb6e4f5e
    PT_LOAD_BYTES=335240

The permanent repair has two complementary parts.

First, the original stage-local command remains historically meaningful but is
now harmless when encountered from a later stage:

    scripts/check-issue7-linked-reproducibility.sh

Outside the exact final Issue #7 authority it performs no build, reports
`SKIPPED` / `NOT_APPLICABLE`, and returns success so an unrelated workflow is
not interrupted.

Second, an explicit later-stage-safe wrapper provides real historical evidence:

    scripts/check-historical-issue7-reproducibility.sh

It exports that exact historical source authority into a disposable tree and
runs the original stage-local proof there. It requires a genuine PASS and the
recorded Issue #7 identities.

The Issue #7 makefile/object set remains historically correct and is not
broadened with later-stage objects.

This distinction is intentional: `SKIPPED` means the check is irrelevant to the
current checkout; it never substitutes for a real Issue #7 PASS when such
evidence is actually required.

The same audit also caught an atomic-update apparatus defect: replacing tracked
shell scripts through fresh temporary files temporarily removed their executable
bits. The modes were restored, the tooling policy now requires mode
preservation during atomic replacement, and the continuity checker verifies the
expected executable development tools.

## Tool-selection applicability lesson

The audit also exposed a broader workflow lesson that is separate from any
individual tool defect.

During broad qualification,
`scripts/check-clean-ps2-compile.sh` was selected because it was maintained,
documented, executable, and portable from the current worktree. Inspection
showed that its explicit 11-translation-unit scope was not stale current-source
discovery: the tool had been intentionally created as the strict Issue #7
translation-unit checker.

A separate read-only probe compiled all 21 current clean Issue #39 C translation
units successfully under the same strict R5900 warning flags. That result proved
the current source could satisfy those flags, but it did not justify redefining
the older tool's purpose.

The durable rule adopted from this finding is:

> Generalize portability; preserve purpose.

Tool inventory, dictionary membership, executable status, and ability to run
from the current checkout establish discoverability, not applicability. Before
invocation, a workflow must identify the tool's purpose, the specific evidence
need, and why the tool applies to the current stage/subsystem/environment.

The resulting selection sequence is:

    inventory -> applicability -> evidence need -> invocation

This avoids wrong-context tool failures and avoids spending qualification time
on tests that do not answer a relevant question.

## Forward policy

The topology adopted here became a forward development rule rather than a
one-time cleanup.

The normative policy is:

    docs/development/source-topology.md

A future clean source file belongs in its owning domain. A new responsibility
directory must be established deliberately with its companion dictionary,
build/test/documentation integration, and topology gate updated together.

This prevents another silent return to one giant root source family or one
giant effective local dictionary.
