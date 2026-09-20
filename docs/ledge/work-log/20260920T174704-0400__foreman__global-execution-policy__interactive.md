# Foreman shift — establish autonomous Reconstruction execution policy

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T17:47:04-04:00
COMPLETED_AT=2026-09-20T17:48:33-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=global-execution-policy
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=747562c82d61f094f55cd644c60339437bd5b11b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Correct the Ledge Reconstruction execution model so ordinary Foreman and
Reconstruction repository work is autonomous, the user is not used as a
general-purpose terminal proxy merely because the Pi worktree is unavailable to
a chat environment, and genuine local/hardware requirements are explicitly
classified and bounded.

Apply the process correction to the currently active
`A004-RFB-FREEZE-REFRESH-R2` packet without broadening, restarting, or changing
its engineering scope or acceptance criteria.

## Authority refreshed

Live branch authority at pickup was exactly:

`747562c82d61f094f55cd644c60339437bd5b11b`

Consumed:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0005;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0023;
- `docs/ledge/work-log/README.md` revision 0006;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0004.

No active Reconstruction worker commit had appeared after the Foreman authority
that published A004 P2.

## Policy finding

The prior Reconstruction contract required canonical repository tooling and
distinguished machine evidence from physical qualification, but it did not
explicitly define GitHub/repository authority as the normal autonomous
development surface or prohibit use of the user as a substitute remote shell.

The active packet also still carried
`EXECUTION_MODE=NORMAL_RECONSTRUCTION`, named the Pi execution seat, and marked
worktree preflight required. In a worker environment without that mounted Pi
worktree, those fields could be misread as requiring user-mediated terminal
execution for ordinary development work.

That ambiguity is corrected by policy rather than by changing the engineering
packet.

## Reconstruction Contract revision 0006

Updated:

`docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md`

Revision 0006 establishes the default execution classification:

```text
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
```

The governing rule now states:

- GitHub/repository tooling is the normal autonomous development surface;
- GitHub Actions is the canonical machine-verification surface when its
  established Ledge jobs cover the required claim;
- ordinary branch/history inspection, archaeology, edits, commits/ref updates,
  CI inspection, dictionary/integration work inside role scope, retries, and
  repository follow-up repairs are not delegated to the user merely because a
  Pi shell is unavailable;
- `/home/ps2/src/PS-to-VNC-ledge-manual` remains a legitimate local execution
  surface but is not a mandatory backend and does not make the user a remote
  shell;
- genuinely unavailable local evidence is classified explicitly as
  `USER_ASSISTED_LOCAL_EVIDENCE`;
- genuine physical work is classified explicitly as
  `USER_ASSISTED_HARDWARE`;
- a missing required environment is first checked against repository/CI
  capability, then stopped at the exact unsupported requirement and returned to
  Foreman rather than expanded into a sequence of user-run development
  commands;
- all future Foreman packets must state execution classification explicitly;
- changing execution classification alone does not change packet engineering
  scope, acceptance criteria, invariants, or non-goals.

The goal-packet quality requirements now require this execution metadata.

## Active A004 packet reconciliation

Updated:

`docs/ledge/LEDGE_FOREMAN_STATE.md`

State revision 0024 adopts Reconstruction Contract revision 0006.

The active packet remains:

`A004-RFB-FREEZE-REFRESH-R2`

Its engineering objective, required behavior, placement constraints,
A004-P2-C1 through C9, explicit non-goals, Q7 compatibility requirement, and
dependency ordering are unchanged.

Only execution assumptions were corrected to:

```text
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual
EXECUTION_SEAT_USE=OPTIONAL_LOCAL_SURFACE_ONLY_WHEN_EXPLICITLY_REQUIRED
WORKTREE_PREFLIGHT_REQUIRED=CONDITIONAL_ON_EXPLICIT_PI_LOCAL_EXECUTION
```

The active packet now explicitly states that ordinary source reconstruction,
repository mutation, dictionary/integration work, commits/ref updates and CI
inspection are autonomous. It currently requires no live-Pi observation or
physical PS2 action.

If future evidence creates one truly unavailable local requirement, the worker
must stop at that exact point and return it to Foreman for deliberate
reclassification rather than turning the packet into a terminal-proxy
workflow.

## Scope / role boundary

FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
ACTIVE_PACKET_ENGINEERING_SCOPE_CHANGED=NO
ACTIVE_PACKET_ACCEPTANCE_CRITERIA_CHANGED=NO
ACTIVE_PACKET_RESTARTED=NO
USER_TERMINAL_PROXY_DEFAULT=PROHIBITED
AUTONOMOUS_REPOSITORY_EXECUTION_DEFAULT=YES

This Foreman shift changes process/state authority only.

## State / contract revisions

Consumed:

- Reconstruction Contract revision 0005
- Foreman State revision 0023
- work-log contract revision 0006
- architecture overlay revision 0004

Produced:

- Reconstruction Contract revision 0006
- Foreman State revision 0024

## Evidence gaps

PENDING_LOCAL=NONE_FOR_POLICY_CORRECTION
HARDWARE_PENDING=UNCHANGED
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Next pickup

The Reconstruction worker should execute the already-active
`A004-RFB-FREEZE-REFRESH-R2` packet under the new autonomous execution
authority.

No user terminal action is currently required.

The worker remains responsible only for the packet's existing bounded
Reconstruction scope and returns the baton to Foreman when that packet reaches
its normal stop point.
