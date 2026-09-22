# Reconstruction shift — R16A RFB provider-failure representation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T09:44:31-04:00
COMPLETED_AT=2026-09-22T15:20:26-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=594f2fdcebbf0f23e724b203532aeb2a14882f36
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
HARDWARE_PENDING=YES
GOAL_STATUS=COMPLETED

## Objective and consumed authority

Executed only the active Foreman packet
`A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A` from
`LEDGE_FOREMAN_STATE` revision 0046. The worker received the baton after Foreman
closed its round at `594f2fdcebbf0f23e724b203532aeb2a14882f36`
(`docs(work-log): confirm R16 blocker and hand off R16A`). The underlying Foreman
state activation is `ea9b07f21bab5e1686f024cae154697f0a85b1c0`.

R16A's bounded objective was to add the missing explicit cross-Wire mechanism
for terminal RFB-provider CONNECT, READ/EOF, and WRITE failure while preserving
physical Wire health as a separate fact. This shift did not perform Foreman
acceptance, Application recovery policy, provider restart/retry, media
activation, or hardware qualification.

## Result

R16A is source-complete as a Reconstruction candidate at pre-log evidence head
`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`.

The chosen representation deliberately assigns the previously dormant
`ERROR=7` framing reservation a complete bounded RFB-local semantic:

- frame kind: `ERROR = 7`;
- channel: `RFB = 1`;
- flags: `0`;
- payload: exactly one 4-byte big-endian terminal-reason value;
- `CONNECT = 1`;
- `READ = 2`;
- `WRITE = 3`.

Unknown reasons, malformed payloads, wrong flags, or wrong channel do not become
valid provider-terminal facts. Existing zero-length channel-1 DATA remains the
exact R13 quiesce representation and is not overloaded.

The fixed physical Wire framing/header version remains `1`. Because an old
product peer cannot safely interpret the new semantic, the Q4 product
establishment compatibility version advances from `1` to `2` on both peers.
Incompatible peers therefore fail at establishment rather than disagreeing
inside an ACTIVE session.

## Pi producer and sole-owner path

`pi/rfb_attachment.py` now preserves the first mechanically distinguishable
provider terminal cause and retires the dead provider attachment's provider,
Relay, and private wake resources without retry, rebind, backoff, or replacement
startup. Successful transmission of the report does not erase the original
local terminal cause.

`pi/wire_server.py` remains the sole physical Wire receive/send/sequence owner.
The attachment publishes only the local pending fact; the Wire owner serializes
the typed ERROR frame and confirms report delivery. Correct provider-terminal
reporting does not itself terminate or rebind the physical Wire Session.

`pi/wire_protocol.py` owns the symmetric provider-failure codec/header predicate
and the product-establishment compatibility version. R10 Relay ownership and
R12 provider endpoint/process boundaries remain separate.

## PS2 consumer and RFB-local termination path

`src/transport/protocol.[ch]` mirrors the exact typed payload/reason contract.
`src/transport/runtime.[ch]` accepts the typed RFB provider-terminal frame as a
channel-local fact rather than setting generic physical `runtime->failed`.
The runtime latches the first specific reason, clears unused outbound provider
capacity, wakes RFB-side waiters, and exposes status without declaring physical
Wire failure.

Already sequenced inbound RFB bytes remain bounded old-session data and may be
drained in order. Once the terminal provider fact is visible, no fresh provider
read authority is intentionally returned for those old bytes and no stale
provider credit/status is rebound to a replacement attachment or Wire Session.
A later Wire Session constructs fresh authority through the existing R13/Q4
lifecycle.

`src/transport/bridge.[ch]`, `src/rfb/bridge.[ch]`, and
`src/rfb/rfb_session.[ch]` expose the terminal fact through the existing narrow
RFB-facing seam. The RFB session converges that provider terminal into its
existing RFB-session I/O failure semantics while physical Transport state
remains separately observable.

## R16A acceptance-criterion evidence

- `A003-R16A-C1 CONNECT_FAILURE_CROSSES_AS_TYPED_RFB_FACT` — MET in source and
  Pi protocol/attachment/server tests.
- `A003-R16A-C2 EOF_READ_FAILURE_CROSSES_AS_TYPED_RFB_FACT` — MET in source and
  Pi attachment/server tests.
- `A003-R16A-C3 WRITE_FAILURE_CROSSES_AS_TYPED_RFB_FACT` — MET in source and Pi
  attachment/server tests.
- `A003-R16A-C4 FIRST_SPECIFIC_PROVIDER_FAILURE_PRESERVED` — MET; first cause is
  latched and survives successful report confirmation.
- `A003-R16A-C5 WIRE_SESSION_REMAINS_DISTINCT_AND_NOT_FALSELY_FAILED` — MET;
  typed provider failure is RFB-local and the Wire worker/session stays live.
- `A003-R16A-C6 SOLE_WIRE_SEND_RECEIVE_AND_SEQUENCE_OWNERSHIP_PRESERVED` — MET;
  the attachment never sends the physical frame itself.
- `A003-R16A-C7 NORMAL_R13_QUIESCE_BYTES_AND_MEANING_UNCHANGED` — MET; exact
  zero-length channel-1 DATA lifecycle remains distinct from ERROR.
- `A003-R16A-C8 COMPATIBILITY_VERSION_CONTRACT_EXPLICIT_AND_SYMMETRIC` — MET;
  Wire framing version stays 1 and Q4 product-establishment version is 2 on Pi
  and PS2 with mismatch rejection before ACTIVE traffic.
- `A003-R16A-C9 PS2_RFB_WAIT_TERMINATES_WITH_RFB_LOCAL_FAILURE` — MET by the
  deterministic dedicated host fixture
  `tests/unit/transport_rfb_provider_failure_test.c`: a genuinely blocked RFB
  reader and a writer blocked for provider credit are woken by the typed
  terminal fact while the Transport/Wire owner remains live.
- `A003-R16A-C10 LATE_CREDITED_RFB_BYTES_CONTAINED_NO_REBIND` — MET; dead
  attachment/session authority remains terminal and later sessions receive
  fresh state.
- `A003-R16A-C11 NO_APPLICATION_RESTART_PROTOCOL_MEDIA_TIMEOUT_OR_RETUNE_SCOPE_CREEP`
  — MET; no Application retry/restart/reconnect, fresh-Q4 recovery transaction,
  AUDIO/MPEG/CONFIG activation, heartbeat, direct-RFB fallback, generic timeout,
  watchdog, systemd restart policy, or R14 retune was added.
- `A003-R16A-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET on the
  exact pre-log evidence head, as recorded below.

These are Reconstruction findings for Foreman review, not self-acceptance.

## Documentation, dictionaries, and continuity

Added:

- `docs/ledge/LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT.md` — explicit
  Reconstruction/source contract, not Foreman acceptance;
- `docs/development/rfb-provider-failure.md` — developer continuity for the
  producer/owner/consumer boundary and downstream exclusions.

Updated the development router and exhaustive documentation index. Product
symbol dictionaries were reconciled with the repository's deterministic
reconciliation mechanism rather than weakening strict coverage. The generated
reconciliation commit is `74178c174fae516f4e20f3d2cf3eb55be3f79d35`.

The exact compare from starting authority `594f2fd...` to pre-log evidence head
`961ad59d...` is 34 commits ahead / 0 behind. Changes are confined to the
packet-authorized Pi/Transport/RFB source surfaces, directly affected tests,
dictionaries, workflow evidence, and documentation.

## Verification and preserved failure chronology

Final exact pre-log evidence head:

`961ad59d82b1c865b9d330a7dd9cdc1ed1e32528`

GitHub Actions run `35772175812` completed with:

- `host-unit` — SUCCESS, including the dedicated `Prove R16A PS2 RFB wait
  termination` step;
- `project-check` — SUCCESS;
- `dictionary-long` — SUCCESS;
- `ps2-compile` — SUCCESS using the pinned PS2 toolchain;
- `ps2-link` — SUCCESS, including current-source build, rebuild/reproducibility,
  and preservation of the identity-ready linked ELF;
- `dictionary-reconcile` — SKIPPED as expected on an ordinary source/evidence
  commit.

Intermediate failures remain part of the repository/CI evidence and were not
hidden:

1. Initial host CI exposed two private bridge fixtures that lacked the newly
   required provider-status stub. They were repaired with direct typed-status
   assertions rather than linker-only shims.
2. Strict dictionary audit exposed exactly 94 missing generated entries and no
   other dictionary defect. The repository's guarded deterministic dictionary
   reconciliation path was used; no validator/gate relaxation was introduced.
3. Run `35768752509` exposed an existing AUDIO fixture scheduling race at
   `transport_audio_test.c:716`; the same unchanged fixture passed a later clean
   run. R16A did not widen scope to modify AUDIO.
4. The same run found the two new maintained documents absent from the
   exhaustive `docs/INDEX.md`; the index was corrected.
5. Run `35769009589` showed the Pi R16A test receiving the ERROR frame before
   the Wire-owner thread completed the immediately following local
   report-confirmation transition. The assertion was made scheduling-stable by
   waiting for that state transition, matching the existing R15 quiesce-test
   pattern; product behavior was unchanged.
6. Run `35769173429` exposed an R12-era regression guard that demanded the
   pre-R16A `pi/wire_protocol.py` blob identity. Because R16A deliberately
   extends protocol semantics, the guard was updated to preserve its real
   architectural purpose: the R10 Relay remains byte-identical at blob
   `55e946f2cc8e7d449b83616a6fe8653695c06702`, and protocol/server/relay/service
   still do not acquire R12 provider endpoint/process policy. The final exact
   head then passed all canonical checks.

## Qualification boundary and next pickup

HARDWARE_PENDING remains YES. The linked PS2 bytes produced by this shift are
current-source build/identity evidence only. This shift makes no claim that the
R16A representation, the changed Q4 product compatibility fence, or the new PS2
runtime behavior has been physically qualified on PS2/Pi hardware, and it makes
no live-Pi service-state claim.

The worker does not accept its own result. Exact next pickup is Foreman review
of this R16A candidate, its preserved CI/failure evidence, and this immutable
record. Foreman owns acceptance/rejection, integration/state updates, hardware
debt disposition, and selection of any next bounded Reconstruction packet.
