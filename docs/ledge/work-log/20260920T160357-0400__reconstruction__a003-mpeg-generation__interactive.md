# Reconstruction shift — A003 Wire/session foundation R1

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T16:03:57-04:00
COMPLETED_AT=2026-09-20T16:34:30-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6417d709b9c0abfc6cfb48e0da5216cb32de77f5
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute Foreman packet `A003-WIRE-SESSION-FOUNDATION-R1`: land the smallest
coherent clean-product Wire/session foundation learned during the Q1-Q12
hardware detour without importing proof harnesses or the rejected generic
Transport admitted-call drain.

## Authority consumed

Governing current-state authority at shift start was
`LEDGE_FOREMAN_STATE.md` revision 0021 with
`ARCHITECTURE_BLOCKER=NONE`.

The normal execution seat was
`/home/ps2/src/PS-to-VNC-ledge-manual`.

Preflight found that local authority initially lagged current branch authority:
the local worktree was clean at `6c416293435203ea77a7afb95db218f3b5a977fe`
while `origin/ledge/h1-all-guns` was
`6417d709b9c0abfc6cfb48e0da5216cb32de77f5`. A clean fast-forward adopted the
current branch before product work. The reconstruction starting authority for
this packet is therefore `6417d709b9c0abfc6cfb48e0da5216cb32de77f5`.

Read and applied the governing repository bootstrap, clean architecture, source
topology/naming rules, module lifecycle contract, Q1-Q12 reconciliation, Wire
runtime decisions, architecture overlay, A003 supersession record, Wire
terminology policy, Reconstruction contract, tooling/testing policy, and current
immutable work-log contract.

## Product commit disposition

Authorized hardware-line product commits were classified as follows:

- `325cf8332212042bc78170fc8d5776a442985006` — `ADOPT_EXACT`
- `03b09eb5a917cfca43001ea3322432a91c3c7473` — `ADOPT_EXACT`
- `d226b59ba54ef2e3f114906564289dada132b361` — `ADOPT_EXACT`
- `8f4841968f47f20eadcb18ed166df54909a30e48` — `SEMANTICALLY_REAPPLY`;
  behavior matched the authorized source, but ten trailing-whitespace defects
  in `src/rfb/rfb_session.c` and `tests/unit/transport_bridge_test.c` were
  removed. Normalized comparison proved those were whitespace-only differences.
- `e5b42f80e36e4c77e1a2f4eccfc55c1ca4457a3d` — `ADOPT_EXACT`
- `2bbe74e7cce0eb914d9f874bb4270afa12b7cd96` — `ADOPT_EXACT`

Dictionary-reference disposition:

- `d31202b547e7d3502d93d285131bf2d3bad7a845` — semantically adopted for its
  surviving audio/MPEG/transport dictionary state; its intermediate RFB/portal
  state is superseded by `c69c578a86...`.
- `c69c578a869335b6bcf964e328b957adc0774b02` — adopted as the final RFB
  dictionary and generated portal state.

Explicitly rejected history remained excluded:
`70aafb4a...`, `f088080f...`, `d553f357...`, and `71a14dcf...`.
The governing `143c77b74c...` correction remains intact: Transport owns
cross-session communication validity; modules own complete local retirement.

## Exact work performed

### Tranche 1 — physical I/O owner and cooperative idle progress

Commit:
`da9053b1488ead933dde1408cdab875885ffa37c`
(`refactor(transport): centralize Wire physical I/O owner`)

Changed eight product/test files to establish:

- exactly one Transport-owned physical framed-I/O execution context;
- no rider raw socket send/receive calls;
- Transport-owned outbound submission/physical send ordering;
- PS2 idle-readiness cooperative scheduling opportunity;
- 1000 us retained only as the hardware-tested implementation baseline, not as
  Wire ABI or claimed global optimum.

The initially combined runtime fixture exposed the already-known stale AUDIO
test assumption: the old fixture waited for receive even when readiness reported
no frame. No source conclusion was drawn from the interrupted run. The exact
authorized `2bbe74e7...` fixture adaptation was then applied and the guarded
focused tests passed.

### Tranche 2 — opaque session-bound rider access

Commit:
`18025d9278940cec7cf0d8513f99e0a93aeefbe3`
(`feat(transport): bind rider access to Wire lifetime`)

Changed twenty-two product/test files to:

- issue opaque module-facing Transport access bound to the active Wire Session;
- require that access on RFB, AUDIO, and MPEG public bridge operations;
- reject stale Session-A access after replacement Session B becomes active;
- keep Wire Session identity private to Transport;
- adapt RFB/audio/MPEG deterministic fixtures to the access-bound seam.

No generic admitted-call drain, module-retirement manager, or copied numeric
session identifier was introduced.

### Dictionary reconciliation

Commit:
`170e2198d41b3d703a5fe86bd5c329f2248d031a`
(`docs(symbols): reconcile Wire session access`)

Updated only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- `src/audio/SYMBOLS.md`
- `src/mpeg/SYMBOLS.md`
- `src/rfb/SYMBOLS.md`
- `src/transport/SYMBOLS.md`

The final portal/dictionaries match the accepted Q12 access source shape and
remove obsolete pre-Q12 RFB helper ownership rows.

## R1 acceptance

### R1-C1 SINGLE_PHYSICAL_IO_OWNER — MET

Static source inspection shows the physical framed send/receive primitives are
called by Transport runtime only. Riders do not call raw socket send/receive.

### R1-C2 COOPERATIVE_IDLE_PROGRESS — MET

The PS2 idle-readiness path explicitly yields cooperative EE scheduling
opportunity. The retained 1000 us value is documented as an evidence-backed
baseline only.

### R1-C3 SESSION_BOUND_ACCESS — MET

Transport bridge tests cover opaque access acquisition and stale access
rejection across replacement sessions. Modules do not receive numeric Wire
Session IDs.

### R1-C4 RIDER_INTEGRATION — MET

RFB, AUDIO, and MPEG product owners and deterministic fixtures use the
access-bound public seam.

### R1-C5 MODULE_RETIREMENT — MET FOR CURRENT SOURCE SCOPE

The materially touched module lifecycles retain owner-local retirement fences:

- AUDIO requires worker stop/completion and join before resource reclaim;
  release refuses a live unjoined worker.
- MPEG is synchronous, observes stop only at decoder-call boundaries, and
  refuses release while a decoder call is active.
- RFB's current session owner is main-thread owned and has no independent worker
  or callback lifetime that can survive replacement.

No Transport-wide retirement drain is needed or permitted. Future
application-level replacement orchestration must still sequence complete owner
stop/release before constructing a replacement live module instance; that
cross-domain orchestration remains a later Application owner obligation.

### R1-C6 NO_REJECTED_DRAIN — MET

Final source/test search found no admitted-call, active-call, inflight-call, or
Transport-drain implementation matching the rejected candidate design.

### R1-C7 DETERMINISTIC_CHECKS — MET FOR APPLICABLE CURRENT-STAGE GATES

Focused host tests passed for physical stream, Transport runtime, Transport
AUDIO/MPEG, Transport bridge, RFB bridge, AUDIO playback/session, and MPEG
decoder.

The complete host unit suite passed after final source shape.

`transport_mpeg_test` passed in both focused and complete-suite execution; the
earlier Foreman-reported nondeterministic occurrence did not recur in this
shift.

Final canonical checks at
`170e2198d41b3d703a5fe86bd5c329f2248d031a`:

- `make -C tests unit` — PASS
- `./scripts/check.sh` — PASS
- `python3 scripts/source-dictionary.py check --long --require-complete --strict`
  — PASS
- rejected-drain absence search — PASS
- final local/remote identity and clean worktree — PASS

Repository tooling explicitly identifies the existing strict PS2 compile and
linked-reproducibility tools as Issue #7 stage-specific. There is no applicable
current-stage clean-source PS2 compile/link/reproducibility gate for this R1
source, so those evidence classes are not claimed rather than being
misrepresented through historical tooling.

### R1-C8 EVIDENCE_BOUNDARY — MET

Evidence is recorded as:

- SOURCE=COMPLETE_FOR_R1_PACKET
- HOST=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARY=PASS
- PS2_COMPILE=NOT_CLAIMED_NO_APPLICABLE_CURRENT_STAGE_GATE
- PS2_LINK=NOT_CLAIMED_NO_APPLICABLE_CURRENT_STAGE_GATE
- REPRODUCIBILITY=NOT_CLAIMED_NO_APPLICABLE_CURRENT_STAGE_GATE
- HARDWARE=NOT_RUN_NOT_CLAIMED

No new hardware-proven claim is made.

## Findings / known-defect accounting

One operator/tooling mistake caused a captured-output text block to be executed
as shell early in the shift. Immediate read-only inspection proved no in-progress
Git operation, no unstaged/untracked work, unchanged HEAD, and only the intended
Tranche-1 staged file set. Those staged blobs were then verified against the
authorized hardware-line source before commit.

The first AUDIO focused run stalled because the pre-adaptation host fixture still
expected a receive call without readiness. That was the exact known fixture
mismatch corrected by authorized commit `2bbe74e7...`; after applying it,
AUDIO and all other focused tests passed.

The Q12 source reference contained ten trailing-whitespace defects. They were
removed before commit, and comparison after stripping trailing blanks proved no
other source difference from the authorized implementation.

## Evidence gaps

PENDING_LOCAL=NONE_FOR_APPLICABLE_R1_HOST_AND_REPOSITORY_GATES

PS2_COMPILE=PENDING_ONLY_IF_A_CURRENT_STAGE_GATE_IS_DEFINED_LATER
PS2_LINK=PENDING_ONLY_IF_A_CURRENT_STAGE_GATE_IS_DEFINED_LATER
REPRODUCIBILITY=PENDING_ONLY_IF_A_CURRENT_STAGE_GATE_IS_DEFINED_LATER

HARDWARE_PENDING=YES_FOR_LATER_PRODUCT_QUALIFICATION

No physical PS2 run was required by this bounded reconstruction packet.

## State / contract revisions

Consumed Foreman state revision 0021 and the current Q1-Q12/module-lifecycle
architecture authorities. Reconstruction did not rewrite Foreman current-state
authority.

## Next pickup

Return baton to the Foreman at pre-log source authority
`170e2198d41b3d703a5fe86bd5c329f2248d031a` plus this immutable log commit.

The R1 source packet is complete. Foreman should independently refresh live
branch authority, inspect this log/commits/evidence, perform any Foreman-owned
integration/state transition, and decide whether dependency-queued A004 may
advance.

Do not begin A004 from this Reconstruction shift.
