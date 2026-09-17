# Foreman work log — A004 MPEG calibration baton recovery

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-17T16:07:58-04:00
COMPLETED_AT=2026-09-17T16:11:41-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation-calibration
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=9151ba9895d53aeecb17b996a9bf92aa42f69c24
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Recover the Foreman baton after a replacement A004 Reconstruction attempt
returned `BLOCKED` without landing repository changes; preserve the deliberate
A003 Transport/Pi manual deferral; identify the exact role-owned blocker;
settle the pure MPEG CALIBRATION owner boundary from current architecture
authority; encode the binding role-baton and execution-admission rules; and
issue exactly one bounded A004 Reconstruction packet.

## Authority consumed

The shift refreshed `ledge/h1-all-guns` at
`9151ba9895d53aeecb17b996a9bf92aa42f69c24` before mutation and read the
governing/current authority including:

- `AGENTS.md`
- `CONTRIBUTING.md`
- `docs/CLEAN_ARCHITECTURE.md`
- `docs/PROJECT_INTENT.md`
- `docs/status.md`
- `docs/development/README.md`
- `docs/development/source-topology.md`
- current source naming/symbol conventions
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md`
- `docs/ledge/work-log/README.md` revision `0006`
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision `0016`
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003`
- `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md`
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md`
- newest relevant Foreman/Reconstruction immutable work logs
- current `scripts/continuity-check.sh`
- current unit-test/source topology
- frozen H1 MPEG-calibration evidence at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

## Failed Reconstruction wake accounting

The previous replacement Reconstruction wake is **not** accepted repository
evidence.

Verified disposition:

- no A004 product/source/test commit landed;
- no branch mutation resulted from that worker attempt;
- no canonical Reconstruction shift log landed;
- the worker's exact `STARTED_AT` is unavailable;
- no retroactive worker timestamp/log was fabricated;
- no A004 criterion was accepted `MET`;
- the worker's report is forensic handoff information only.

This shift independently verified the useful pure-mechanics notes against the
frozen H1 MPEG-calibration lineage rather than accepting the blocked report as
authority.

## Evidence and findings

### Owner boundary

The pure MPEG CALIBRATION geometry plus default/draft/committed state is a
coherent owner distinct from both current `src/mpeg/` decoder/resource lifetime
ownership and current `src/display/` display/presentation conversion ownership.

Foreman state revision `0017` therefore settles the clean owner directory as:

`src/mpeg_calibration/`

DESKTOP CALIBRATION remains a separate historical system and was not used as
MPEG-calibration mechanics authority.

### Verified pure mechanics

Frozen H1 evidence confirms:

- base = exact MPEG capture/presentation rectangle;
- inner matte = PS2-local presentation-only inset;
- outer matte = MPEG visual-ownership / RFB-suppression expansion;
- centered half-canvas default with 16-pixel base size alignment;
- 16-pixel base minimum;
- center-preserving 16-pixel resize;
- pixel-precise movement with canvas clamp;
- bounded inner matte;
- clipped outer suppression;
- explicit default/draft/committed state;
- cancel -> committed when present, otherwise default;
- commit -> one immutable accepted geometry authority.

The deterministic 704x462 vector was rechecked as:

- default `(176,119,352,224)`;
- horizontal `+16` resize -> `x=168 width=368`;
- `+1,+1` move -> `x=169 y=120`;
- inner `1,1`;
- outer `1,1`;
- suppression `(168,119,370,226)`.

### Exact Foreman-owned blocker

A new clean `src/mpeg_calibration/` domain triggers the canonical new-domain
adoption obligations in `docs/development/source-topology.md`:

- topology policy;
- living file/service map;
- local directory dictionary;
- `scripts/continuity-check.sh` expected topology;
- build/include/test/tool path registration;
- architecture/navigation record;
- strict dictionary discovery;
- generated dictionary portal.

Foreman state `0016` reserved the canonical/global portions of those chores to
Foreman/Integration.

The continuity checker requires exact agreement among actual clean directories,
directory dictionaries, and generated portal directories. Therefore a
Foreman-created empty/pre-admitted component would deliberately fail current
continuity authority and is not a valid prerequisite.

The correct dependency sequence is **A**:

1. Reconstruction lands coherent behavior source/API, local `SYMBOLS.md`, and
   focused host test source in the new owner directory.
2. Reconstruction runs the focused test directly when aggregate registration
   does not yet exist.
3. Reconstruction commits its coherent worker-owned tranche and returns the
   baton at the first canonical integration chore.
4. Foreman/Integration performs topology/map/continuity/generated-portal and
   canonical build/test/tool registration against the actual landed files.

This matches the recent A003 MPEG-domain precedent, where behavior source landed
before the later canonical owner/topology closure.

## Foreman-owned work performed

Commit:

`f3978d60563237b04f563d31c1cb4e087d926220`

updated:

`docs/ledge/LEDGE_FOREMAN_STATE.md`

to revision `0017`.

Revision `0017`:

- preserves the A003 manual Transport/Pi deferral and accepted A003 work;
- accounts for the failed A004 worker wake without fabricating evidence;
- settles `src/mpeg_calibration/` as the genuine pure-core owner boundary;
- records the verified frozen-H1 mechanics and deterministic test vector;
- identifies the exact Foreman-owned new-domain integration blocker;
- selects sequence A rather than an invalid empty-domain scaffold;
- makes the worker/Foreman baton rule binding;
- makes early execution-environment preflight binding;
- forbids Reconstruction workarounds/raw Git-object substitution for missing
  role-owned or worktree capabilities;
- reissues exactly one bounded
  `A004-MPEG-CALIBRATION-CORE-R1` Reconstruction packet;
- keeps canonical topology/generated portal/build/test integration on the
  Foreman side of the next baton.

No A004 product behavior was implemented by Foreman.

## Checks / evidence result

- live branch before state mutation:
  `9151ba9895d53aeecb17b996a9bf92aa42f69c24`
- state update commit:
  `f3978d60563237b04f563d31c1cb4e087d926220`
- post-write branch refresh confirmed the state update as live `HEAD`;
- post-write fetch confirmed `LEDGE_FOREMAN_STATE` revision `0017`;
- historical mechanics were checked against frozen H1 MPEG-calibration source;
- A003 recent source/topology history was inspected and confirmed the
  source-first / canonical-closure precedent.

`PENDING_LOCAL=canonical ./scripts/check.sh was not executed from this connector-based Foreman seat; the newly issued Reconstruction execution preflight requires an existing applicable host command before any product work.`

No hardware or cross-machine qualification was claimed.

## Criterion disposition

No A004 product criterion is accepted from the failed worker wake:

- `A004-C1 MPEG_CALIBRATION_OWNER_BOUNDARY = PENDING_RECONSTRUCTION_SOURCE`
- `A004-C2 GEOMETRY_MEANINGS = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C3 SINGLE_COMMITTED_AUTHORITY = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C4 DESKTOP_SEPARATION = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C5 CONTRACT_HOST_TESTED = PENDING_RECONSTRUCTION_HOST_TEST`
- `A004-C6 BOUNDED_SCOPE = PENDING_RECONSTRUCTION_HANDOFF`

The Foreman architectural decision that the owner belongs at
`src/mpeg_calibration/` is now binding packet authority, but it is not counted
as product-source completion for C1.

## Pending evidence

PENDING_MANUAL_TRANSPORT=A003 real Pi PSTV owner; exact START semantic/prepared-generation implementation; producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof

PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction; downstream all-guns owner integration

HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification

## Next pickup

`A004-MPEG-CALIBRATION-CORE-R1` from Foreman state revision `0017`.

The next Reconstruction worker must perform the execution preflight before
archaeology, implement only the worker-owned pure core/source/test/local
dictionary tranche, stop at the first Foreman-owned canonical integration chore,
write its one canonical immutable log when possible, and return the baton.

Do not reopen autonomous A003 Transport/Pi work. Do not advance to A005/A006.
