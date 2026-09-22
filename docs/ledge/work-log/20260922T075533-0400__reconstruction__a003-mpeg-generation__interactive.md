DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T07:55:33-04:00
COMPLETED_AT=2026-09-22T08:00:09-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=944cd9d453697d00288f653c854bdeacf69b016c
ENDING_BRANCH_COMMIT=944cd9d453697d00288f653c854bdeacf69b016c
SELF_PAUSED=NO

# Reconstruction closeout — A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15

## Scope and continuity

The Foreman returned packet `A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15`
solely to repair the missing immutable Reconstruction-role closeout after the
previous interactive Reconstruction session timed out at the UI boundary.
The R15 source, tests, documentation, dictionary reconciliation, and CI evidence
had already landed before that timeout.

The original R15 coding shift's exact STARTED_AT and COMPLETED_AT values were not
durably recorded in repository authority before the timeout. This immutable
entry therefore does **not** invent or reconstruct those timestamps. Its metadata
truthfully describes this recovery/closeout Reconstruction shift, which began at
`2026-09-22T07:55:33-04:00` from branch authority
`944cd9d453697d00288f653c854bdeacf69b016c`.

No R15 source history was rewritten during this recovery shift. No Foreman-owned
state was edited. This worker does not accept its own work; independent Foreman
review remains required.

## Authority audited

- R15 assignment/Foreman activation base:
  `fa61f9db392d9984f520ec8559c3137a9b16e958`
  (`docs(foreman): accept R14 and activate ordinary RFB composition`).
- Final R15 source/evidence head before the missing role log:
  `377ab24f066a71a813aecccd4c92ccb096d94c27`.
- Foreman recovery/handoff head at the start of this closeout shift:
  `944cd9d453697d00288f653c854bdeacf69b016c`.
- `944cd9d453697d00288f653c854bdeacf69b016c` is a Foreman-owned log-only
  descendant of `377ab24f066a71a813aecccd4c92ccb096d94c27`; it does not modify the
  R15 source/test/runtime product.
- The authoritative compare from the R15 assignment base to the source/evidence
  head is exactly 18 commits ahead with no rewritten history.

## R15 commit inventory

The complete post-assignment R15 history ending at the source/evidence head is:

1. `7119fdad4093f9dffd66f9cf028c3a1fea3f2a0f` —
   `feat(pi): compose default RFB Wire runtime from R14 profile`
2. `f8f9e8f6db23667b8ebfcdd2127c4c0408b5227c` —
   `feat(pi): run composed RFB Wire product entrypoint`
3. `bcdb75f6b4a9fa3da3e3bb90b2c62c0e7f5860fa` —
   `feat(pi): stage R15 profile composition runtime`
4. `edab0367975a13a5652ad345bb4a6e5c5dd9262f` —
   `feat(app): activate selected RFB profile at default entry`
5. `319b60d7de03e6a5cfefea10904fc3fbe336e86d` —
   `test(app): verify R15 default profile activation`
6. `d8a4495a3c994cd332eabfa0f3d614f284bd04a7` —
   `test(pi): verify R15 selected default composition`
7. `986004a5e01cb609b345d5b3447b0df084b4226c` —
   `test(rfb): stabilize R13 wake and R15 service assertions`
8. `cc5f04956f43a9781de40134ffd80739a7002312` —
   `test(pi): reserve R12 provider legacy fixture`
9. `2f9071b94146d415efb60453c7a8833ad5e0e8b2` —
   `test(pi): reserve R14 profile legacy fixture`
10. `8e2e6e3c41c1fd4e36153c75fb008aa3279d8dd0` —
    `test(pi): make R12 provider fixture R15-aware`
11. `3e2876f526459c19f3fa814c157c1a30ac4b1158` —
    `test(pi): prove fresh R15 wake state before retirement`
12. `1e2fa43ef3feda2647500c071586df20047fdc5c` — `temp`
13. `34b6a22772fb5ae382be21b55bd5fc24fdfa6fcd` —
    `chore(test): remove accidental temporary file`
14. `51ba0e5a050fb756a1f7c3e753783552bceec7bf` —
    `test(pi): restore frozen pre-R15 fixtures`
15. `5def6d2cbc456f3767d770b2729cf2da256cee53` —
    `docs(pi): describe R15 ordinary RFB composition`
16. `0e5977a0d3629faf0b1d453db65ee8be4ed3a978` —
    `tooling(symbols): run deterministic dictionary reconciliation`
17. `fa9326e8b94f82876a2df30d47f6a3e509517d4c` —
    automation commit `docs(symbols): reconcile current clean definitions`
18. `377ab24f066a71a813aecccd4c92ccb096d94c27` —
    `test(pi): replace stale R14 activation absence assertion`

The accidental `temp` commit and its explicit cleanup remain visible by design.
This recovery did not squash, edit, or otherwise hide intermediate chronology.

## File and ownership inventory

Key packet-owned deltas include:

- `pi/wire_runtime.py` — new ordinary Pi product composition layer. It resolves
  the selected R14 RFB profile before constructing the listener and supplies the
  existing `WireServer` a fresh R13 attachment factory when semantic RFB is ON.
- `systemd/pi/ps-to-vnc-wire.service` — ordinary product service now invokes
  `wire_runtime.py` rather than the lower-level standalone `wire_server.py`.
- `scripts/pi/install-wire-runtime.sh` — exact tracked runtime staging,
  verification, and removal now includes the generated/hand-written RFB profile
  projection and the product composition entrypoint as well as the accepted Wire
  and RFB mechanism files.
- `src/app.c` — the default PS2 application entrypoint projects the selected
  shared R14 profile into the existing Transport session configuration and
  fails closed if that projection is unavailable.
- R15 unit/fixture changes cover the PS2 application default-profile path, Pi
  ordinary product composition, R13 wake/quiesce assertions, fresh session
  state, R12 provider and R14 profile legacy fixtures, and the final replacement
  of the now-stale R14 activation-absence assertion.
- Pi documentation was reconciled to describe the ordinary R15 composition.
- Clean-source dictionaries were deterministically reconciled by the trigger at
  `0e5977a0d3629faf0b1d453db65ee8be4ed3a978` and automation commit
  `fa9326e8b94f82876a2df30d47f6a3e509517d4c`.

The 18-commit range from the assignment base through `377ab24...` is canonical
for the exact per-file delta and preserves every intermediate correction.

## Functional disposition

From this Reconstruction worker's packet-level audit, R15's bounded objective is
implemented at source/test/evidence level:

- Ordinary Pi product composition consumes the selected R14 profile instead of
  inventing an independent tuning profile.
- Semantic RFB ON supplies the existing Wire server with a factory producing one
  fresh R13 `RfbAttachment` per sequential Wire connection.
- Semantic RFB OFF supplies no attachment factory; invalid/failed selected-profile
  projection remains fail-closed rather than guessing defaults.
- R13 attachment construction itself stays inert. The accepted R12 internal
  provider endpoint `127.0.0.1:5900` remains lazily contacted only after the
  first valid post-Q4 nonzero channel-1 RFB CREDIT reaches the attachment.
- Fresh per-Wire-session attachment state is preserved; R13 quiesce/wake and
  RFB-local failure-containment ownership remains with the accepted R13
  mechanism rather than the new composition layer.
- The ordinary systemd product service invokes `wire_runtime.py` at the existing
  product Wire listener.
- Runtime staging/verification copies exact tracked bytes and does not reload,
  enable, or start the service as a side effect.
- On PS2, `pstvnc_app_run()` obtains the selected R14 Transport projection and
  passes that existing configuration type into
  `pstvnc_app_run_with_transport_config`; profile selection failure returns `-1`.
- The packet does not add a second Wire Session identity owner and does not
  intentionally alter the accepted physical Wire/Q4 protocol mechanism.
- CONFIG delivery, AUDIO Wire relay, MPEG control/production, and their broader
  lifecycle activation are outside this R15 packet.

These are Reconstruction dispositions only. They are not Foreman acceptance or
hardware qualification claims.

## Validation evidence

### Exact R15 source/evidence head

GitHub Actions run `35719827570` executed on exact head
`377ab24f066a71a813aecccd4c92ccb096d94c27` and completed successfully.
Its substantive jobs were:

- `host-unit` — PASS
- `project-check` — PASS
- `dictionary-long` — PASS
- `ps2-compile` — PASS
- `ps2-link` — PASS, including current-source reproducibility
- `dictionary-reconcile` — correctly skipped because no reconciliation trigger
  remained at that final source commit

### Current pre-log handoff authority

GitHub Actions run `35723962283` executed on exact pre-log head
`944cd9d453697d00288f653c854bdeacf69b016c` and also completed successfully:

- `host-unit` — PASS
- `project-check` — PASS
- `dictionary-long` — PASS
- `ps2-compile` — PASS
- `ps2-link` — PASS
- `dictionary-reconcile` — skipped

Thus the Foreman handoff commit did not disturb the fully green R15 source/test
state. No source edits were made during this recovery shift before this immutable
log.

## Preserved failures and corrections

R15 history intentionally retains its intermediate problems and their later
corrections:

- service/R13 wake assertion stabilization is visible at
  `986004a5e01cb609b345d5b3447b0df084b4226c`;
- R12 provider and R14 profile legacy-fixture reservation/awareness work is
  visible in commits 8 through 10 above;
- fresh R15 wake-state proof and subsequent frozen-fixture restoration remain
  visible in commits 11 through 14;
- the accidental `temp` commit `1e2fa43...` remains followed by the explicit
  cleanup `34b6a227...` rather than being rewritten out of history;
- source/dictionary reconciliation remains visible as its trigger and automation
  commits;
- the stale R14 assertion that still expected activation absence was corrected
  by the final source/evidence commit `377ab24...`, after which the exact-head CI
  run was fully green.

## Claim boundary

`HARDWARE_PENDING=YES`

R15 does **not** claim:

- physical PS2/Pi hardware qualification;
- live Pi deployment, service state, or Pi-local worktree cleanliness;
- provider-write performance optimality or transfer of historical qualification
  to newly composed runtime behavior;
- CONFIG Wire delivery;
- AUDIO Wire relay;
- Pi MPEG control/production;
- Application MPEG activation;
- retirement/restoration decisions outside this bounded packet;
- final all-guns qualification;
- Foreman acceptance of this worker-owned packet.

## Pending gaps and exact next pickup

This recovery audit found no remaining packet-owned R15 source, test,
documentation, dictionary, or GitHub-CI gap requiring another Reconstruction
source commit before independent review.

The next pickup belongs to the Foreman: independently inspect this R15 history,
this role log, and live repository authority; accept or reject the packet under
Foreman-owned criteria; update durable Foreman state only if warranted; and
choose any subsequent bounded packet. If the Foreman discovers a concrete R15
defect, it can return a correction packet to Reconstruction. Physical hardware
qualification remains pending wherever the reconstruction/retirement rules
require it.
