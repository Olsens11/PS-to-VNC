# Architecture — sustained reconstruction shift contract

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T21:14:07-04:00
COMPLETED_AT=2026-09-15T21:15:30-04:00
ELAPSED=00:01:23
ROLE_KEY=architecture
WORK_ITEM_KEY=reconstruction-shift-contract
WORKER_KEY=interactive
STATUS=COMPLETE
STARTING_BRANCH_COMMIT=c0cafde186b01dea4c41441c9d7a42ebb5ab18cd
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Replace the accumulated reconstruction-worker prompt patches with one clear sustained-engineering-shift model, and make that model durable in repository authority so later workers cannot reasonably interpret architecture/checkpoint granularity as shift-completion granularity.

## Evidence prompting the change

Recent scheduled reconstruction behavior repeatedly ended after a single small checkpoint despite immediately actionable same-item work remaining. The latest Reconstruction A wake recorded `STARTED_AT=2026-09-15T20:52:17-04:00`, `COMPLETED_AT=2026-09-15T20:53:18-04:00`, `SELF_PAUSED=NO`, one test-file commit, and then named additional safe same-item work in its own next pickup. That demonstrated that layering more exceptions onto the existing prompt had not changed the worker's underlying interpretation of its job.

## Durable policy change

Updated `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` from revision 0002 to revision 0003 in commit `cf77ac7015310d37cd5712b90395ae897c41bfd3`.

Revision 0003 now states from the outset that a reconstruction worker performs a sustained engineering shift, not one bounded microtask. It explicitly separates:

- architecture/checkpoint granularity: small owners, files, helpers, tests, dictionary edits, and coherent commits;
- shift granularity: repeated checkpoints across the available safe work window.

The governing loop is now: establish authority -> advance a coherent increment -> preserve it -> re-read authority/evidence -> correct reconstruction-owned errors -> choose the next safe increment -> continue until a real stopping condition.

The contract also defines fallback priority when one subtask blocks, a final continue-work gate before logging, truthful-time requirements, and the distinction between fixing current reconstruction errors and silently changing inherited H1 defects.

## Scheduler prompt overhaul

Both enabled scheduled reconstruction workers were rewritten from scratch rather than extended with additional patch clauses. The old accumulated prompt structure was replaced with the same simple job model used by contract revision 0003:

- primary identity is `SUSTAINED ENGINEERING SHIFT`, not one-task invocation;
- the worker repeatedly executes the authority/work/commit/re-read/correct/continue loop;
- commits and micro-increments are explicitly checkpoints, never automatic shift endings;
- a blocked subtask causes a same-item or next-authorized reconstruction pivot before an early stop;
- current reconstruction defects are corrected in-lane when understood and safe;
- before final logging, any remaining safe current-item, same-item, or dependency-permitted reconstruction work requires continued work;
- normal useful shifts aim for roughly 20-30 minutes without padding or fabricated time;
- under-ten-minute completion requires a real stopping condition and evidence that no other safe authorized reconstruction work could be progressed.

Worker-specific lane identities remain `recon-a` and `recon-b`; the 80-minute schedules and 40-minute staggering were not changed.

## Authority / boundaries

This change alters reconstruction execution procedure, not audit dispositions, validation findings, H1 behavior authority, product source, or hardware qualification state. Reconstruction state revision 0007 remains the current product baton until reconstruction work changes it. Earlier worker logs remain immutable evidence of the procedure under which those shifts ran.

## Result

The previous prompt's layered microtask-oriented wording is no longer the operating instruction for A or B. Both workers now receive a concise sustained-shift job definition, and that definition is independently reinforced by repository-governing contract revision 0003.

## Next observation

Judge the overhaul by the next scheduled reconstruction wakes: a short shift is valid only if its immutable log demonstrates a genuine stopping condition and no safe fallback work. Otherwise the worker should remain in the operating loop and continue through multiple checkpoints.