# PS2VNC Development-Continuity Harvest

Status: `REVIEWED_REFERENCE`

Reviewed: `2026-09-02`

Historical source repository: `Olsens11/PS2VNC`

Reviewed historical branch:

`experiment/test15E3K-D17AL-F-display-cleanup`

## Purpose

PS2VNC developed a strong documentation, status, provenance, TestKit, and
session-resumption system under real project pressure. PS-to-VNC should preserve
the lessons that made that system useful without copying obsolete product
architecture or recreating all of its machinery by inertia.

This document records the deliberate harvest. It is a design/reference record,
not a claim that every historical PS2VNC tool should be ported.

The governing clean-reconstruction question is the same one used for product
architecture:

> What would we build now if we had known from the beginning what the historical
> project taught us?

## Historical material reviewed

The primary continuity sources reviewed for this harvest are:

- `docs/START_HERE.md`
- `docs/DOCUMENTATION.md`
- `docs/DEVELOPMENT_INFRASTRUCTURE.md`
- `docs/status/STATUS_MODEL.md`
- `docs/reference/FILE_AND_SERVICE_MAP.md`
- `docs/testing/TEST_TOOLKIT.md`
- `docs/provenance/README.md`
- `docs/HISTORICAL_HARVEST.md`
- `docs/reference/KNOWN_GOOD_BUILDS.md`

The historical repository remains read-only. The lessons below are restated in
PS-to-VNC so a future session does not need the old repository merely to know the
successor's continuity design.

## Disposition summary

| Historical lesson | PS-to-VNC disposition | Successor authority / direction |
| --- | --- | --- |
| One canonical session entrance with a completeness contract | `ADOPT` | `AGENTS.md` + `SESSION_RECONSTRUCTION.md` |
| Repository must replace conversation memory as durable project authority | `ADOPT` | `AGENTS.md`, development docs |
| Status authority is conditional evidence, not a filename hierarchy | `ADOPT` | `TEMPORAL_STATE_SEMANTICS.md`; identity-aware live-state work remains to mature |
| Age and identity/freshness are different questions | `ADOPT` | timestamped snapshot semantics plus reconciliation |
| Volatile next-action state must not live inside bootstrap docs | `ADOPT` | bootstrap discovers active work dynamically |
| Canonical tool/service map prevents procedure reinvention | `ADAPT` | successor needs a clean-current tool/context router rather than migration-era duplication |
| Test apparatus is part of experiment identity | `ADOPT` | `development/testing.md`, TestKit discipline |
| Machine evidence and human interpretation are separate authorities | `ADOPT` | current qualification/casebook rules |
| Evidence classes and missing provenance arrows remain explicit | `ADOPT` | preserve `PROVEN`, `CORROBORATED`, `UNRESOLVED`, `DISPROVEN`, `RETROSPECTIVE` semantics where useful |
| Known-good means known-good for a stated role/workload | `ADOPT` | future qualified-milestone index should remain role-scoped |
| Durable knowledge should normally be recorded when created | `ADAPT` | retain the rule now; successor automation should grow from demonstrated need |
| Documentation mutations should validate the complete transaction before publish | `ADOPT` | continuity/documentation policy |
| End-of-session development wrap with exact identity | `ADAPT` | useful, but successor implementation must fit current branch/worktree model |
| Historical-harvest ledger prevents repeated archaeology | `ADAPT` | casebook + branch audits + future harvest ledger where needed |
| Context routing/concordance can progressively load relevant authorities | `ADAPT` | bootstrap routing now; automated context bundle remains deferred |
| Every repeated useful procedure should graduate into tested tooling | `ADOPT` | toolkit-first development rule |
| Copy the old service graph, product architecture, or filenames because continuity worked | `REJECT` | preserve continuity principles, not old product structure |
| Require a fresh session to blindly read every large document regardless task | `REJECT` | require complete authority reconstruction, then progressive task-sensitive depth |
| Build a generalized context/orchestration framework before real pressure demands it | `DEFER` | consider only after repeated successor friction |

## 1. Canonical entrance plus a completeness contract — ADOPT

The strongest PS2VNC continuity idea was not merely having a `START_HERE.md`.
It was defining what **being caught up** meant.

A new session was not considered bootstrapped merely because it read one status
page. It had to understand current state, documentation authority, tooling,
TestKit/evidence discipline, architecture, decisions, history, and how to keep
those systems updated during the rest of the session.

PS-to-VNC adopts that principle through:

- `AGENTS.md` as the single canonical development-session entrance;
- `docs/development/SESSION_RECONSTRUCTION.md` as the detailed procedure;
- a reconstruction summary gate before substantive work.

The successor improves the historical model by requiring the **complete GitHub
work record** as well: open and closed issues, substantive comments, merged and
closed PRs, relevant branches, and Git history. This prevents open issues alone
from masquerading as the project's full intent.

## 2. Conversation is input, repository is authority — ADOPT

PS2VNC explicitly treated conversation continuity as a project requirement.
Useful ideas, results, decisions, next steps, and tool contracts were supposed to
be promoted into repository state instead of depending on an old chat remaining
available.

PS-to-VNC keeps that rule. Conversation memory may accelerate orientation, but a
required project fact that exists only in conversation is a continuity defect.

This is why the successor now contains:

- the major-investigation casebook;
- the full copied MI-001/Test14 mechanism report;
- branch lifecycle/audit records;
- the continuity follow-up backlog;
- temporal state semantics.

## 3. Status is conditional evidence — ADOPT and strengthen

The historical `STATUS_MODEL.md` captured a critical rule:

> A status record is authoritative only for the exact repository/content identity
> that created it. Existence is not authority.

It also separated **age** from **identity**. A recent record can become stale
immediately after source/worktree identity changes, while an older exact record
can still be useful.

PS-to-VNC has now generalized the same problem at the documentation level:

- mutable status records are explicit timestamped snapshots;
- old `CURRENT`, `ACTIVE`, `NEXT_ACTION`, `BLOCKED`, progress, and running-state
  prose must not be repeated as present truth without reconciliation;
- durable facts such as an observed hash or hardware result remain historical
  facts even when the status snapshot is old.

Current authority:

`docs/development/TEMPORAL_STATE_SEMANTICS.md`

The historical project went further with an identity-bound local live state that
fingerprinted branch, HEAD, source, ELF, and the **actual tracked diff**. That is
a strong successor direction, but it must be adapted to PS-to-VNC's multiple
active worktrees/workstreams rather than copied verbatim.

## 4. Keep volatile next action out of bootstrap docs — ADOPT

PS2VNC learned that the durable bootstrap should teach a session **how to find
current work**, not contain a volatile embedded current-test block that becomes
stale.

PS-to-VNC adopts the same principle more strongly:

- `AGENTS.md` is procedural and issue-number agnostic;
- status snapshots are explicitly temporal;
- active branches/workstreams are discovered from current GitHub/repository/live
  evidence;
- `CONTINUITY_FOLLOWUPS.md` is explicitly a timestamped backlog rather than a
  hidden current instruction.

## 5. Canonical tool/context routing — ADAPT

PS2VNC's `FILE_AND_SERVICE_MAP.md` reduced a common failure mode: a new session
could discover the canonical build, deploy, TestKit, observer, status, evidence,
and documentation tools instead of reconstructing equivalent commands in chat.

PS-to-VNC already has historical/migration-era maps, but the clean successor
needs a **current clean tool/context router** that answers:

- what tool owns this repeated procedure?
- where is its usage contract?
- what workstream/branch owns it if not yet on `main`?
- when should this authority be read?
- what related evidence/case/state becomes relevant?

Disposition: `ADAPT`, not copy. The clean router should describe successor
reality rather than preserve migration-era filenames merely because they exist.

## 6. Apparatus is part of the experiment — ADOPT

PS2VNC's TestKit correctly treated workload, observer, geometry, network state,
capture filters, operator procedure, timing, and instrumentation as experimental
variables.

The durable rule is:

> Preserve the apparatus or explicitly version the change.

That remains essential for PS-to-VNC, especially for intermittent network,
display, and timing failures. A new ELF alone does not define a comparable run.

Machine evidence and operator observation remain separate. A sealed run can
prove what machines recorded; it cannot manufacture a physical observation the
operator did not make.

## 7. Provenance grades and missing arrows — ADOPT

The historical provenance model is worth carrying forward because it prevents
plausible reconstruction from silently becoming proof.

Useful evidence classes:

- `PROVEN`
- `CORROBORATED`
- `UNRESOLVED`
- `DISPROVEN`
- `RETROSPECTIVE`

Useful chain:

`SOURCE -> BUILD -> ARTIFACT -> DEPLOYMENT -> OBSERVATION -> CLASSIFICATION`

If an arrow is missing, keep it missing. Do not infer deployment/observation
identity merely because the source reproduces the same ELF or a commit subject
sounds right.

This same discipline already underlies the successor's Issue #7 qualification
and major-investigation cases.

## 8. Role-scoped known-good milestones — ADOPT

PS2VNC's known-good index made an important distinction: `known-good` is not a
claim of global correctness. A build may be known-good for a particular
workload, feature, or historical role.

PS-to-VNC should retain a compact qualified-milestone map as its number of clean
hardware results grows. Each entry should identify:

- exact source/commit and artifact identity where relevant;
- what was actually qualified;
- evidence pointer;
- explicit limits of the claim.

Do not create a new index merely to have one before enough successor milestones
exist; the principle is adopted, the exact artifact can grow when useful.

## 9. Documentation at knowledge-creation time — ADAPT

PS2VNC evolved `docs-event.sh`, transactional candidate-tree validation,
pending-file/hash ledgers, and documentation-only commits because important
findings were otherwise lost or inconsistently copied into multiple documents.

The successor adopts the **behavioral requirement now**:

- significant result, finding, decision, deferred idea, major case update, or
  checkpoint becomes durable when it occurs;
- documentation changes must survey authority and reconcile cross-references;
- multi-file documentation mutation should validate assumptions before the first
  write and validate the resulting set before promotion.

The exact legacy `docs-event` implementation is `ADAPT`, not automatically
ported. PS-to-VNC should only grow as much automation as repeated friction earns.

## 10. Development wrap and identity-valid live state — ADAPT

The historical end-of-session wrap was valuable because it captured:

- what actually completed;
- what remained running;
- next intended action;
- exact Git/source/ELF identity;
- worktree state;
- recent evidence and apparatus.

The strongest rule was that a saved live state whose identity no longer matched
could not authorize its old `NEXT_INTENDED`.

PS-to-VNC should adapt this model to its current reality:

- multiple active GitHub branches/worktrees;
- Pi and PS2 hardware state;
- branch-local candidate authority;
- current clean Issue #5/#7 qualification records;
- timestamped documentation snapshots.

A future live-state implementation should not reintroduce a single global
`current` record that conflates independent workstreams.

## 11. Historical harvest ledger — ADAPT

PS2VNC's `HISTORICAL_HARVEST.md` kept track of which old conversations and
forensic areas had already been mined into durable authority. That prevented
repeated archaeology and made gaps explicit.

PS-to-VNC now has several more focused equivalents:

- semantic-audit records;
- major-investigation casebook;
- branch audit/harvest records;
- copied primary historical source where necessary.

If future legacy mining becomes broad again, add a successor harvest ledger that
records **what has been reviewed and where the durable result lives**, rather
than repeatedly rereading the whole legacy repository.

## 12. Progressive context routing / project concordance — ADAPT, automation DEFER

PS2VNC recognized that continuity should answer both:

- What should I read before doing this work?
- Given what changed, which authorities should I update or re-check?

That is a strong idea. PS-to-VNC should progressively route context:

`broad orientation -> active workstream -> subsystem -> case/evidence/tool`

rather than forcing every chat to load every large historical document before
every action.

The repository may eventually justify a generated context bundle or command, but
that automation is `DEFER` until repeated real use proves the manual routing is
still too slow or error-prone.

## 13. Repeated procedures graduate into tools — ADOPT

Historical TestKit made a useful maintenance rule explicit: if a procedure is
repeated, easy to get wrong, identity-sensitive, or likely to recur, it should
become repository-managed tooling with self-test and documentation rather than a
fresh chat-generated command packet every time.

PS-to-VNC adopts that rule. Chat command packets remain appropriate for genuinely
new investigation or tool repair; routine established mechanics should call a
saved tool.

## What is deliberately not inherited

The following are not continuity requirements merely because they existed in
PS2VNC:

- the historical Pi service graph;
- TigerVNC/Openbox/LXPanel as a bundle;
- legacy product source boundaries;
- old test names or phase numbering as successor architecture;
- one giant universal status file containing every domain;
- exact historical `docs-event`, status, or TestKit script layout;
- a requirement to reread every historical document for every task;
- speculative generic orchestration/context machinery.

Those belong to historical implementation, not the continuity lessons being
preserved.

## Successor implementation priorities from this harvest

The highest-value remaining continuity improvements are:

1. make the one-line fresh-chat handoff unmistakable at repository entry points;
2. make status/resume output refuse to promote stale snapshot `NEXT_ACTION` into
   present tense;
3. finish a clean-current tool/context router so canonical procedures are easy to
   discover;
4. adapt identity-bound live-state/development-wrap behavior to multiple active
   workstreams;
5. keep major investigations, branch harvests, and historical mining visibly
   closed-loop so the same archaeology is not repeatedly performed;
6. mechanically enforce temporal metadata and bootstrap routing where practical.

These priorities are continuity work. They must not consume an available
hardware session when the user is present and ready to perform Issue #5/#7
qualification.