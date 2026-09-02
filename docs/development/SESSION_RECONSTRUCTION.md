# Fresh Development Session Reconstruction

## Purpose

A new development session must be able to reconstruct the project accurately
from the repository and GitHub record without depending on prior conversation
history or a hand-written handoff.

This procedure is deliberately issue-number agnostic. The active work today will
not necessarily be the active work later. A fresh session must discover the
current state rather than being told to assume it.

The goal is not merely to read the newest status file. It is to understand:

- what PS-to-VNC is trying to become;
- what has already been proven;
- why important architecture and tooling choices exist;
- what experiments were adopted, rejected, replaced, or left evaluating;
- what major failures have unresolved or resolved case reports that constrain
  present design;
- what work is active now;
- what was most recently completed;
- what remains blocked or uncertain;
- what the documented next action is;
- what state must be protected before any new work begins.

## Core rule

> A fresh development session must reconstruct the project from its complete
> repository and GitHub record before proposing a new direction. Do not infer
> current intent from open issues alone, and do not assume prior conversation
> context.

Repository authority defines current truth. GitHub history explains how that
truth was reached. Neither should be read in isolation.

## 1. Read current repository authority first

Read these entry points before proposing source, architecture, test, or recovery
changes:

1. `AGENTS.md`;
2. `docs/status.md`;
3. `runtime/PROJECT_STATE.env`;
4. `docs/PROJECT_INTENT.md`;
5. `docs/README.md`;
6. `docs/development/README.md`;
7. `docs/investigations/README.md` and every major case relevant to the active
   subsystem;
8. the current architecture authority routed by those documents;
9. relevant ADR, audit, test, runbook, Pi, reconstruction, and reference
   documents for the subsystem that appears active.

Do not treat a historical roadmap, migration document, superseded architecture,
or old issue description as current authority merely because it contains more
text.

A resolved major investigation is not merely historical trivia. It may explain
why a present invariant exists. An open major investigation may define a
qualified workaround or design guardrail that current work must not accidentally
bake into unrelated architecture.

## 2. Reconstruct the complete GitHub work record

Do not look only at open issues.

Survey the repository's GitHub record broadly enough to understand the project
history and present work:

- all open issues;
- all closed issues;
- issue descriptions and substantive comments;
- linked pull requests;
- all open pull requests;
- merged and closed pull requests;
- PR descriptions, review discussion, and substantive comments;
- current branch inventory and relevant non-main branch heads;
- recent commit history on `main` and active branches;
- releases and milestone records when present;
- GitHub Discussions, Projects, or other planning surfaces when the repository
  uses them for durable project reasoning.

Closed work is not disposable history. A closed issue may contain the experiment
or rationale that explains a current architectural invariant. A rejected or
superseded proposal may be important because it records what must not be
rediscovered.

When the GitHub record is large, reconstruct it systematically rather than
sampling only the most recent items:

1. inventory issues and PRs;
2. read them in a useful chronological and topical order;
3. follow cross-links between issues, PRs, major-investigation cases, commits,
   evidence, and docs;
4. note promotion, rejection, replacement, and supersession decisions;
5. identify unresolved threads that remain genuinely open.

## 3. Reconcile history with current authority

Use the following distinction:

- repository current-state documents answer **what is authoritative now**;
- architecture and ADR material answer **how the system is intended to work and
  why**;
- major-investigation cases answer **what significant failures taught us, what
  workaround or correction exists, and what uncertainty still constrains us**;
- empirical evidence and qualification records answer **what has actually been
  demonstrated**;
- GitHub issues, PRs, and comments answer **how the project arrived here**;
- Git history answers **what changed, when, and on which branch**.

If these sources appear to disagree, investigate the discrepancy.

Do not silently choose the newest-looking source, the longest source, or the
open issue over a closed one. Determine whether one source superseded another,
whether a result was only exploratory, or whether current-state documentation
has become stale.

Important distinctions to preserve include:

- `ADOPTED` versus `EVALUATING`;
- current authority versus preserved experiment;
- machine result versus physical/operator result;
- product failure versus apparatus or infrastructure failure;
- historical implementation versus current architectural intent;
- branch-local work versus merged `main` authority;
- resolved engineering contract versus residual low-level unknown detail.

## 4. Re-apply the reconstruction philosophy before choosing a solution

Understanding the current state is not enough. Before proposing a new direction,
re-read the active reconstruction principle from `docs/PROJECT_INTENT.md` and
apply it to the specific decision being made.

For every meaningful implementation or architecture choice, determine:

1. the product behavior or historical lesson that must survive;
2. which parts of the historical implementation are merely accidental structure;
3. the conventional supported mechanism that would normally solve the problem
   on the target platform today;
4. whether PS-to-VNC has a demonstrated reason to adapt or depart from that
   convention;
5. which owner/module/service boundary should remain stable if the provider or
   mechanism is replaced later;
6. which attractive future features are only context for preserving a seam and
   must remain deferred rather than being implemented speculatively;
7. the evidence required to classify the candidate as adopted, adapted,
   evaluating, rejected, or deferred.

The purpose is not to maximize abstraction or conventionality. It is to build
what we would have built from the beginning with today's knowledge: commodity
responsibilities handled by commodity mechanisms, project-specific policy where
it earns its complexity, and clean replaceable seams where future evolution is
plausible.

Before hardware testing or implementation promotion, summarize the candidate in
plain language as:

- **Requirement / lesson**
- **Conventional machinery**
- **PS-to-VNC adaptation**
- **Replaceable provider/mechanism**
- **Deferred concerns**
- **Qualification gate**

If the solution can only be justified as "this worked before" or "this should
work in theory," the reconstruction reasoning is incomplete.

## 5. Discover the active work dynamically

Do not hard-code specific issue or PR numbers into session bootstrap logic.

From the reconstructed record, determine:

- which issues, PRs, branches, or worktrees are active;
- which major-investigation cases are open, monitoring, or relevant resolved
  constraints for the active subsystem;
- which branch or commit is authoritative for each active workstream;
- what was most recently completed and what evidence qualified it;
- what work is blocked and by what;
- what experiment, if any, is currently in progress;
- what the documented next action is;
- what deferred work is intentionally not being pursued yet;
- what experimental candidates remain `EVALUATING` rather than adopted;
- what repositories, worktrees, evidence directories, or historical states are
  protected from mutation.

A fresh session should be able to explain the current work in plain language
before changing anything.

## 6. Inspect live repository state before mutation

When a local checkout is available, inspect:

- current branch and exact HEAD;
- `git status`;
- staged changes;
- unstaged changes;
- untracked files relevant to active work;
- `git worktree list --porcelain`;
- local branches and their relationship to remote branch heads;
- recent commits on active branches.

Then run the canonical continuity tooling when available:

- `scripts/resume-state.sh`;
- `scripts/check.sh`.

Do not reset, clean, checkout over, or otherwise discard dirty state during
catch-up. Dirty state is potentially active project evidence until understood.

## 7. Reconstruct empirical-test context separately

If hardware or live-system testing is active, identify before proposing another
run:

1. the exact DUT/source authority;
2. the exact deployed artifact identity;
3. the last proven machine result;
4. the last physical/operator result;
5. the apparatus used;
6. the workload/stimulus used;
7. known apparatus defects or environmental variables;
8. the intended next experiment;
9. the rollback or failure-safety boundary.

Do not turn a convenient exploratory result into an architecture decision unless
it has actually been promoted through the project's decision and qualification
process.

When an experiment materially changes a major investigation, update the case
report with the new evidence/disposition rather than leaving the conclusion only
in chat or a transient test comment.

## 8. Produce a short reconstruction summary before acting

Before beginning substantive work, a fresh session should be able to state:

- **Project intent:** what the product is and the reconstruction principle;
- **Current authority:** which docs/commits define present architecture and state;
- **Completed proof:** the most recent relevant qualified results;
- **Major cases:** relevant open and resolved major-investigation constraints;
- **Active work:** the workstreams currently in progress and their branch/commit
  authorities;
- **Current uncertainty:** what is still evaluating, blocked, or unproven;
- **Protected boundaries:** what must not be mutated or discarded;
- **Next action:** the next documented task and why it is next.

If any of those cannot be stated confidently, continue reconstructing context
instead of improvising a plan.

## 9. Only then begin project work

Once the reconstruction summary is internally consistent:

- apply the reconstruction decision discipline before selecting a mechanism;
- use existing canonical tools before generating replacements;
- continue the established plan unless evidence justifies changing it;
- preserve experiment/source/apparatus provenance;
- create a major-investigation case early when a problem meets the casebook
  threshold rather than waiting for root cause;
- update existing major cases as hypotheses/evidence change;
- promote durable new knowledge into repository documentation;
- keep current-state documents current as work is completed;
- leave rejected and superseded decisions discoverable rather than erasing them.

## Minimal handoff for a brand-new chat

A human handoff should normally need no project-history essay. The preferred
instruction is:

> Open the `Olsens11/PS-to-VNC` repository. Read `AGENTS.md` and follow the fresh
> development session reconstruction procedure completely before doing any
> project work. Reconstruct current state from the repository and the complete
> GitHub issue/PR/history record rather than assuming prior chat context.

If that instruction is insufficient for a capable development session, the
repository continuity system is incomplete and should be improved rather than
papered over with another conversation-specific handoff.
