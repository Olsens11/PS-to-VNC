# Reconstruction shift — A003 R14 shared RFB runtime profile

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T22:59:56-04:00
COMPLETED_AT=2026-09-21T23:40:40-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=db36b13ff58840a64d844ef0d6c9da9635dbdb84
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Executed only active Foreman packet
`A003-RFB-SHARED-RUNTIME-PROFILE-R14` from
`LEDGE_FOREMAN_STATE` revision 0043, under Reconstruction Contract revision
0006 and work-log contract revision 0007.

R14 was configuration authority only: establish one canonical current RFB
runtime profile, deterministically project it into the existing PS2 Transport
session-config and Pi R13 flow-config owner types, preserve semantic ON/OFF,
and keep Wire/Q4/default-service/Application activation unchanged.

Repository authority was refreshed from accepted corrected-R13 Foreman HEAD
`db36b13ff58840a64d844ef0d6c9da9635dbdb84`.

## Work performed

Created one canonical machine-readable selected profile:

- `src/config/rfb_runtime_profile.json`

Selected semantic mode and values:

- RFB mode = ON;
- RFB window = 32768 bytes;
- credit batch = 8192 bytes;
- flush-on-empty = enabled;
- credit return = enabled;
- receiver priority = 63;
- receiver stack = 16384 bytes;
- max DATA payload = 8192 bytes.

The canonical record preserves CP2N hardware-baseline provenance and the CP2J
starting-profile record. It explicitly records that equality of the new Pi
provider-write capacity to 32768 does not hardware-qualify that new direction.

Added deterministic projection tooling:

- `scripts/generate-rfb-runtime-profile.py`;
- `src/config/rfb_runtime_profile_generated.h`;
- `pi/rfb_runtime_profile_generated.py`.

The generator has write and `--check` modes. Canonical project checks invoke
`--check`, so a changed canonical profile makes checked-in projections stale
until regenerated.

Added the narrow PS2 Configuration projection:

- `src/config/rfb_runtime_profile.{c,h}`.

It projects ON into the existing `pstvnc_transport_session_config_t` with:

- `rfb_queue_capacity = 32768`;
- `rfb_initial_credit_bytes = 32768`;
- `rfb_credit_batch_bytes = 8192`;
- `rfb_credit_flush_on_empty = 1`;
- `rfb_credit_return_enabled = 1`;
- `receiver_thread_priority = 63`;
- `receiver_thread_stack_size = 16384`;
- `max_data_payload = 8192`.

OFF is represented as no active Transport projection: the projection fails
without mutating caller storage. No parallel Transport config type or
Configuration-owned Wire Session identity was created.

Added the narrow Pi projection:

- `pi/rfb_runtime_profile.py`.

Semantic ON produces exactly one existing R13 `RfbFlowConfig` with:

- `provider_read_credit_limit = 32768`;
- `provider_write_capacity = 32768`;
- `max_data_payload = 8192`.

Both Pi capacities derive from the one canonical RFB window. Semantic OFF
returns `None`; it does not manufacture a zero-valued active attachment.

A final R14 tightening at `c4b6df7b288ce6758f472fc18b0b0f2c37967ddc`
removed PS2-only receiver/credit-policy constants from the generated Pi artifact.
The Pi generated projection now contains only semantic mode, the shared window,
and max payload.

Updated canonical PS2 compile/link inputs, host tests, project check,
source-topology documentation, Pi documentation, and the living file/service
map. Deterministic source-dictionary reconciliation was invoked through the
repository-owned automation rather than hand-editing symbol tables.

## Commits

R14 commits from the accepted R13 base:

- `a0834637fa181db430d1ad08ab38f3b2b4c1d149` —
  `feat(config): add shared RFB runtime profile authority`;
- `e1daf7de55b10ed3c86f059dc905ddfbafafe8db` —
  `docs(config): bind RFB profile ownership and provenance`;
- `01293fbfe80bf8966a92909b9d000ae0f9cd410f` —
  deterministic dictionary-reconciliation trigger;
- `ecc7f06385456f353129caa5e6734a242e12b28f` —
  automated `docs(symbols): reconcile current clean definitions`;
- `8483bf53788293ee0166031d4e78557c2ab0fd9b` —
  tree-identical canonical verification run;
- `e88ece91c05e3092f3148c966363ba65cb7ecc1f` —
  `fix(pi): enroll RFB profile projection in clean source`;
- `8f57703b7039a6a56f2f90b8ae772bb344749b67` —
  deterministic dictionary-reconciliation trigger;
- `47d0eba2ba9deefb1aededb1cf026e9f6ba86243` —
  automated `docs(symbols): reconcile current clean definitions`;
- `bcba91c069444736edf7405e89c692a553e7d287` —
  tree-identical clean-source verification run;
- `c4b6df7b288ce6758f472fc18b0b0f2c37967ddc` —
  `refactor(config): narrow Pi RFB profile projection`;
- `df1ad8a1f3d43c767756c1b596b898a4f0fff406` —
  final deterministic dictionary-reconciliation trigger; reconciliation found
  no additional dictionary delta.

## Evidence

Final canonical GitHub Actions authority is run `35683895809`, attempt 2, at
exact HEAD `df1ad8a1f3d43c767756c1b596b898a4f0fff406`:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — PASS.

Host evidence includes:

- `config_rfb_runtime_profile_test` — PASS;
- `pi_rfb_runtime_profile_test.py` — PASS;
- `RFB_RUNTIME_PROFILE_GENERATED=PASS`;
- corrected R13 `pi_rfb_attachment_test.py` — PASS;
- Wire-server/Q4 tests — PASS;
- existing Transport/RFB/AUDIO/MPEG host suites — PASS on the final rerun.

Deterministic stale-artifact proof changes the canonical profile in a temporary
tree, observes `RFB_PROFILE_GENERATED_STALE=`, regenerates, then returns to
`--check` PASS.

Exact unchanged-R14-base blob identities prove Wire/Q4/default activation
surfaces were not modified:

- `pi/wire_protocol.py`:
  `a136640272ab942ce3e15096ff9da2089491745d`;
- `src/transport/protocol.c`:
  `d2cb66327b4d2bf0fc716332136e586cadb69475`;
- `src/transport/protocol.h`:
  `279de0b14785c74e4bba128dfcf2f34302bc0cd8`;
- `systemd/pi/ps-to-vnc-wire.service`:
  `316c6d2eafdfdf5ecd8c89678e2dfe227e709556`;
- `pi/wire_server.py`:
  `8ddf3122d279e979c75231c8031dbb7837c1d2d2`;
- `src/app.c`:
  `0b2738c594ad1039bff3c475d0482b48bda323b7`.

The default Pi service therefore still supplies no profile/attachment factory,
and `pstvnc_app_run()` remains fail-gated through
`pstvnc_app_run_with_transport_config(NULL)`.

Current linked source is reproducible across two builds. Final PT_LOAD identity:

- segments = 1;
- bytes = 484756;
- SHA256 =
  `e13e4fa85635095a7ba8226620d37c4cd9956c2e3bd973959088476812e6e9ac`.

Accepted R13 base PT_LOAD was:

- segments = 1;
- bytes = 484628;
- SHA256 =
  `8604dec6302a028575e44df03c49026052a99bd5df858f3be68798b1bf958538`.

R14 therefore changes PT_LOAD by +128 bytes. Reproducibility is proven; hardware
qualification is not.

Intermediate CI runs exposed repository-enrollment/dictionary work still needed
and intermittent host timing failures in unchanged Transport tests
(`transport_runtime_test` / `transport_audio_test`). No Transport behavior
was changed under R14. The exact final R14 tree passed those suites on rerun.

## Acceptance criteria disposition

- A003-R14-C1 SINGLE_CANONICAL_RFB_PROFILE_AUTHORITY — MET.
- A003-R14-C2 CURRENT_RFB_ON_SEMANTICS — MET.
- A003-R14-C3 EXACT_PS2_TRANSPORT_PROJECTION — MET.
- A003-R14-C4 EXACT_PI_ATTACHMENT_PROJECTION — MET.
- A003-R14-C5 NO_INDEPENDENT_DUPLICATE_DEFAULTS — MET.
- A003-R14-C6 NO_DUPLICATE_WIRE_SESSION_IDENTITY — MET.
- A003-R14-C7 RFB_OFF_IS_INERT_COMPOSITION — MET.
- A003-R14-C8 WIRE_PROTOCOL_AND_Q4_UNCHANGED — MET.
- A003-R14-C9 DEFAULT_SERVICES_REMAIN_INERT — MET.
- A003-R14-C10 APPLICATION_REMAINS_FAIL_GATED — MET.
- A003-R14-C11 STRICT_GENERATION_TEST_DOC_RECONCILIATION — MET.
- A003-R14-C12 CLAIM_BOUNDARY_PRESERVED — MET.

These are Reconstruction-worker dispositions subject to independent Foreman
acceptance.

## Evidence gaps and non-claims

HARDWARE_PENDING=YES

R14 does not claim live Pi activation, physical PS2/Pi qualification,
performance optimality, Application RFB startup, reconnect/restart policy,
CONFIG Wire delivery, AUDIO Wire relay, Pi MPEG control/production, Application
MPEG activation, Q7 retirement/restoration, or final all-guns qualification.

CP2N qualifies the historical RFB-only baseline direction/mechanism and selected
starting values. It does not transfer hardware qualification to the new R13 Pi
provider-write queue merely because the selected capacity is also 32768.

This interactive GitHub seat had no attached Pi-local worktree. No claim is made
about an external Pi worktree's cleanliness or live service state.

## State/contract revisions

Consumed:

- `LEDGE_FOREMAN_STATE` revision 0043;
- `LEDGE_RECONSTRUCTION_CONTRACT` revision 0006;
- work-log contract revision 0007;
- `LEDGE_ARCHITECTURE_OVERLAY` revision 0006;
- `LEDGE_WIRE_RUNTIME_DECISIONS` revision 0011.

No Foreman-owned current-state file was modified by this Reconstruction worker.

## Next pickup

Foreman should independently inspect and either accept or reject R14 from live
repository authority. Reconstruction must not begin Application RFB activation
until the Foreman issues the next bounded packet.
