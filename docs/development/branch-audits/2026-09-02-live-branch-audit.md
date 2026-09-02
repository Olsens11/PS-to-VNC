# Live Branch Provenance Audit — 2026-09-02

Temporal role: `SNAPSHOT_AUDIT`

Recorded at: `2026-09-02T15:35:00-04:00`

Repository: `Olsens11/PS-to-VNC`

This is a read-only provenance/lifecycle audit of the live GitHub branch set at
the recorded time. It does **not** authorize deletion of any branch.

Governing policy:

- `docs/development/BRANCH_LIFECYCLE.md`
- `docs/development/TEMPORAL_STATE_SEMANTICS.md`

Every deletion still requires verified cold storage when appropriate and explicit
user approval for that **specific branch**.

## Inventory observed

GitHub exposed 11 live branches:

1. `main`
2. `docs/fresh-session-reconstruction`
3. `pi/issue5-clean-baseline`
4. `pi/issue5-rfb-socket-activation-candidate`
5. `reconstruct/issue7-minimal-core`
6. `archive/w0vncserver-1.16.2-experiment`
7. `audit/behavioral-inventory`
8. `state/post-semantic-audit`
9. `reboot/activate-preservation-semantic-audit`
10. `reboot/intent-and-semantic-audit`
11. `reconstruction/m1-minimal-core`

No hidden second family of live GitHub branches was found in the branch listing
at this snapshot.

## Integrated authority

### `main`

- classification: `MAIN_AUTHORITY`
- head: `34b53d724dede1f3875d136a8adf59ce888fa680`
- head timestamp: `2026-09-01T02:42:17Z`
- head purpose: merge PR #13 and reconcile the post-semantic-audit state.

`main` is intentionally behind several active workstreams because PRs #15, #16,
and #17 are still draft/open. That is not branch ambiguity; it is normal
unmerged work.

## Active / evaluating work

### `docs/fresh-session-reconstruction`

- classification: `ACTIVE`
- PR: #16, draft/open
- purpose: fresh-session reconstruction, major-investigation casebook, temporal
  state semantics, branch lifecycle/indexing, and continuity work.
- head is self-moving while this audit is being written; future readers must
  resolve the live PR head from GitHub rather than trust an embedded self-head.

Disposition: keep live while PR #16 remains active.

### `pi/issue5-clean-baseline`

- classification: `ACTIVE`
- head: `87baebce32e3c07ffc12298d6a168ac890231cf2`
- head timestamp: `2026-09-01T12:35:03Z`
- head message: `pi: bind session candidate to reviewed Xtigervnc path`
- relationship: 12 commits ahead of the recorded `main` at audit time.
- role: clean Pi foundation / dependency / provisioning authority for Issue #5.

Disposition: keep live. It is the base of the current Pi lifecycle candidate and
contains unmerged project work.

### `pi/issue5-rfb-socket-activation-candidate`

- classification: `CANDIDATE`
- PR: #17, draft/open
- head: `66988ebcfb1eed57cb84f422fb84bdb6faf98952`
- head timestamp: `2026-09-02T12:39:35Z`
- head message: `pi: record exact systemd inetd handoff semantics`
- relationship: 28 commits ahead of `pi/issue5-clean-baseline` at audit time.
- role: standards-first evaluation of systemd socket activation plus Xtigervnc
  `-inetd`, with persistent-provider control retained for comparison.

Disposition: keep live until physical qualification/adoption/rejection is
recorded. It is not architecture merely because it exists.

### `reconstruct/issue7-minimal-core`

- classification: `ACTIVE`
- PR: #15, draft/open
- head: `e7b4a35f681101fe39e59c4b04b28740b0f536de`
- head timestamp: `2026-09-01T23:46:10Z`
- head message: `docs(issue7): reconcile successor TestKit dictionary portal`
- role: current clean minimal PS2 core workstream.

Disposition: keep live. The branch has current unmerged implementation,
qualification tooling, and evidence authority.

## Protected archive

### `archive/w0vncserver-1.16.2-experiment`

- classification: `PROTECTED_ARCHIVE`
- head: `13f0a0279f14c9d2c6ab3e00625b5c74f7607657`
- head message: `reference: archive TigerVNC 1.16.2 w0vncserver experiment`
- preserved binary SHA-256:
  `76dd46e40a3b1d82da8a86b3bff622b2f6c9c703c2d1ed38dee81c660d0ef804`
- role: exact feasibility artifact and provenance for the successful-but-unadopted
  w0vncserver experiment.

Disposition: retain. It is deliberately named and documented as an archive and
is not ordinary branch-cleanup material.

## Fully merged historical branches

The following branches all compare as **zero commits ahead of `main`** at this
audit point. Their useful commits are already reachable through merged `main`
history and their associated merged PRs preserve discussion/context.

### `audit/behavioral-inventory`

- classification: `SUPERSEDED`
- head: `ea72838fd7e5a756bade0b0deb60297b4a122154`
- head timestamp: `2026-09-01T02:39:03Z`
- head message: `state: mark semantic audit rebuild ready`
- related PR: #12, merged
- current relationship: 0 ahead / 4 behind `main` at the prior compare.

The branch culminated the B01-B14 semantic audit and clean-architecture
promotion. That work is on `main`; no branch-only product source or evidence was
found by the ancestry comparison.

Disposition: `SUPERSEDED`, suitable for cold-storage retirement preparation if
the user later wants live-branch reduction. Not deletion-authorized.

### `state/post-semantic-audit`

- classification: `SUPERSEDED`
- head: `f767a43f8bb118ad46f12cec8c509a92588a6199`
- head timestamp: `2026-09-01T02:41:56Z`
- head message: `state: reconcile post-audit promotion`
- related PR: #13, merged
- current relationship: 0 ahead / 1 behind `main` at the prior compare.

This branch was a state-only reconciliation after PR #12. Its only branch-tip
change updated the machine state to the already-promoted audit head and advanced
`NEXT_ACTION` to Issues #5 and #7. That commit is merged into `main`.

Disposition: `SUPERSEDED`, suitable for cold-storage retirement preparation if
requested. Not deletion-authorized.

### `reboot/activate-preservation-semantic-audit`

- classification: `SUPERSEDED`
- head: `dbb82d92c2591f0eea58627257693246dbb8b17f`
- head timestamp: `2026-09-01T00:28:35Z`
- head message: `reboot: activate preservation and semantic audit`
- related PR: #11, merged
- current relationship: 0 ahead / 35 behind `main` at the prior compare.

This was a short transition branch that advanced status/state after the formal
clean-reconstruction reboot. Its tip changed only `docs/status.md` and
`runtime/PROJECT_STATE.env`; those changes are reachable through `main`.

Disposition: `SUPERSEDED`, suitable for cold-storage retirement preparation if
requested. Not deletion-authorized.

### `reboot/intent-and-semantic-audit`

- classification: `SUPERSEDED`
- head: `64b72c90fa81e4cdbe3628323190a0d9331cb2a2`
- head timestamp: `2026-09-01T00:19:19Z`
- head message: `reboot: polish current-state transition`
- related PR: #10, merged
- current relationship: 0 ahead / 37 behind `main` at the prior compare.

This branch established the formal clean-reconstruction reboot, project intent,
current machine state, and continuity checks. PR #10 merged that history into
`main`; the branch tip itself is therefore historical routing, not a parallel
current implementation.

Disposition: `SUPERSEDED`, suitable for cold-storage retirement preparation if
requested. Not deletion-authorized.

## Special harvest: `reconstruction/m1-minimal-core`

### Identity

- classification before harvest: `SUPERSEDED` with unique-history review
  required
- head: `00a6a22e77a20ca830a3d2cca75abb9072900273`
- head timestamp: `2026-09-01T02:53:07Z`
- relationship: three commits ahead of `main`; diverged from the later
  `reconstruct/issue7-minimal-core` branch.

The three unique commits are:

1. `f62b4dcbb54d43f6e8b299d6eb5a7a1157d3f1ac`
   - `2026-09-01T02:48:40Z`
   - `rfb: establish host-testable handshake protocol core`
2. `2aee9ff18936ba2de488942ad997593b1856f944`
   - `2026-09-01T02:51:16Z`
   - `reconstruction: add minimal PS2 network and RFB handshake runtime`
3. `00a6a22e77a20ca830a3d2cca75abb9072900273`
   - `2026-09-01T02:53:07Z`
   - `build: add isolated M1A reconstruction build contract`

### What that prototype contained

The branch was a very early clean-reconstruction prototype. It deliberately
split the first milestone into a handshake-only `M1A` before a later fixed
framebuffer/presentation `M1B`.

Its useful ideas were:

- start with pure host-testable RFB parsing before PS2 integration;
- keep one simple top-level application coordinator;
- keep PS2 system and network platform mechanisms behind narrow source seams;
- use exact send/receive loops rather than assuming one socket call equals one
  RFB field;
- keep the initial milestone blocking and single-threaded;
- pin the qualified PS2IP archive by SHA-256;
- isolate clean build output from the historical build;
- refuse to call a source/build checkpoint hardware authority;
- require later exact DUT/TestKit/machine/operator qualification.

The intentionally temporary behavior included:

- accepting ServerInit geometry up to `1280x720` merely to prove the existing Pi
  transport/handshake path;
- no SetPixelFormat or SetEncodings;
- no framebuffer decode or authority;
- no GS/presentation;
- no diagnostics/runtime identity;
- handshake success followed by clean socket close and OSDSYS return.

### Relationship to the current Issue #7 branch

The later `reconstruct/issue7-minimal-core` branch did **not** descend from this
prototype; it independently rebuilt the milestone and is substantially stricter.
The current Issue #7 authority now preserves or improves every durable M1 lesson:

- host-testable wire code and scripted session tests;
- explicit one-owner RFB session and exact I/O seam;
- fixed application-selected `704x462` geometry rather than the temporary
  `1280x720` envelope;
- qualified 16-bpp/depth-15 pixel format and Raw-only encoding;
- exact startup framebuffer unique-pixel coverage, including overlap/gap
  rejection;
- owned authoritative framebuffer and separate GS presentation buffer;
- Standard 480p display path;
- buffered receive independent of `recv()` boundaries;
- deterministic runtime identity and diagnostics;
- successor-owned identity preparation;
- reproducible linked build and PT_LOAD fingerprinting;
- defined real-hardware qualification and separate machine/operator evidence.

The original M1 branch therefore contains **historically useful chronology but
no implementation that should be cherry-picked over the current Issue #7
branch**. Doing so would regress the clean core to a handshake-only temporary
contract.

### Harvest disposition

The three unique commits have now been explicitly inventoried and their useful
design lessons are preserved in this audit. Their source remains reachable from
the live branch and GitHub commit objects.

Recommended lifecycle state after this harvest:

`SUPERSEDED / UNIQUE_HISTORY_HARVESTED / COLD_STORAGE_CANDIDATE`

This is deliberately **not** `SAFE_TO_RETIRE` yet because no verified cold-storage
bundle/manifest has been produced and the user has not approved branch deletion.

If retirement is later desired:

1. make a self-contained Git bundle preserving the exact head and reachable
   history;
2. record SHA-256, size, related Issue #7 context, and archive location;
3. independently restore/verify the exact head;
4. present that evidence to the user;
5. delete only if the user explicitly approves this named branch.

## Branch set after deobfuscation

At this audit point the 11 branches reduce conceptually to:

- **1 integrated authority:** `main`;
- **3 active/evaluating development lines:** PR #16, Issue #5/PR #17, Issue
  #7/PR #15 (with the Issue #5 foundation branch as their stable base);
- **1 protected experiment archive:** w0vncserver;
- **4 fully merged historical transition/audit branches:** retirement candidates
  only after cold-storage procedure;
- **1 unique early M1 prototype:** harvest complete, cold-storage candidate, not
  deletion-authorized.

The branch count is therefore larger than the number of real current directions.
The index should communicate that distinction so humans and fresh sessions do not
mistake historical refs for parallel product strategies.

## What this audit does not do

This audit does not:

- delete a branch;
- create cold-storage bundles;
- remove or detach local worktrees;
- inspect unexplained dirty local worktree state;
- promote PR #16, #17, or #15;
- change product architecture;
- declare any unqualified candidate adopted.

Cold-storage creation and any live-ref deletion remain separate, explicit work.