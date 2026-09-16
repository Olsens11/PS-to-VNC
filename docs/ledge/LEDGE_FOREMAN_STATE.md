# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0006
RECORDED_AT=2026-09-16T07:42:25-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0003
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0023
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman continuation supersedes revision 0005. It consumes the completed A001 C1-C8 Reconstruction packet, the reconciled current source dictionaries/portal, work-log policy revision 0003, and canonical machine evidence from the exact current-source CI authority `20b2e21b7718d987892bb71b498d609a5db0ec5d`.

The branch was re-read immediately before this state write and remained exactly `20b2e21b7718d987892bb71b498d609a5db0ec5d`. No A002 product behavior was started.

## Current foreman phase

`A001_VALIDATION_READY_HANDOFF`

A001 has no remaining Foreman-owned integration or known Reconstruction-owned behavior obligation. Foreman therefore hands the coherent A001 tranche to independent Validation as `VALIDATION_READY`.

This is **not** a Validation PASS and is **not** physical PS2 qualification. Validation still owns formal V003/V004/V005 disposition. Hardware remains `HARDWARE_PENDING`.

## A001 behavior and ownership disposition

The completed Reconstruction packet and current source/test topology cover the A001 audit behaviors without a newly identified DUT defect:

1. one physical PSTV connection owner and one sole receive owner in Transport;
2. Transport-owned physical framing/sequence authority and logical-channel dispatch;
3. RFB parser ownership above a logical byte-stream bridge rather than a physical socket;
4. parser-consumed RFB credit distinguished from terminal residual discard;
5. producer-driven RFB activity/wakeup rather than blind polling;
6. finite REQUEST -> BOUNDARY -> COMMIT -> residual -> COMPLETE quiescence, distinct from application-local fatal abort;
7. RFB/application service only at complete parser message boundaries;
8. application/presentation flow policy above parser/framebuffer authority.

V005's reconstructed fatal-abort path remains the governing current source shape: Transport requests stop, interrupts its own blocking physical receive, waits for sole-receiver completion/dormancy, then reclaims receiver-visible resources and the descriptor. The application does not directly close an adopted descriptor. Independent Validation previously supported this source shape statically; the canonical machine evidence below now executes the registered lifecycle/runtime fixtures needed for independent final disposition.

## V004 integration disposition

The prior V004 source/portal gap is complete at current source level and canonically machine-checked:

- `src/rfb/SYMBOLS.md`: `COVERAGE=COMPLETE`, current logical bridge/session source represented, 345 entries;
- `src/transport/SYMBOLS.md`: `COVERAGE=COMPLETE`, 422 entries;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` includes both RFB and Transport;
- clean source topology/continuity owns `src/transport`;
- canonical complete long/strict source-dictionary audit: PASS in run `35091578944` at source authority `20b2e21...`;
- canonical project check, including portal/topology and work-log validation: PASS in the same run.

Work-log contract revision 0003 preserves seven exact already-frozen legacy/malformed records through explicit path-level compatibility rather than rewriting immutable history. The canonical checker now passes with those seven explicit grandfather records; this is a governance compatibility repair, not a reusable exception mechanism.

## Canonical machine evidence

Canonical workflow: `.github/workflows/ledge-reconstruction.yml`

Exact successful run:

- workflow run ID: `35091578944`;
- exact checked-out source authority: `20b2e21b7718d987892bb71b498d609a5db0ec5d`;
- `make -C tests unit`: PASS, including direct `transport_physical_stream_test` and `transport_runtime_test` plus the existing Transport/RFB/application suite;
- `./scripts/check.sh`: PASS;
- `python3 scripts/source-dictionary.py check --long --require-complete --strict`: PASS;
- pinned PS2Dev clean compile check: PASS;
- current linked clean build: PASS;
- repeated build ELF byte comparison: PASS;
- normalized PT_LOAD comparison: PASS;
- preserved unqualified linked ELF artifact: PASS.

Exact linked-build identity from the run:

- `ELF_PRISTINE_SHA256=3093b390b2f0e9cbd62786116151cd34dc991441e162ffe523ef5adae02aed26`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=55ef86f7684b43f8c87b9e461a09d4155856691201711e3b3412fc6d7898e1cb`;
- `PT_LOAD_BYTES=412680`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`;
- runtime identity blob present but intentionally unqualified/unstamped (`ELF_IDENTITY_TEST_ID=UNSTAMPED`);
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The preserved artifact is unqualified build evidence only. It is not a physical-hardware PASS.

## A001 CONFIG boundary

Unchanged and now explicitly non-blocking for A001: all eight `pstvnc_transport_session_config_t` values are Pi-supplied per-session CONFIG authority. A001 accepts a complete caller-supplied Transport subprofile and does not invent defaults. The production producer/decoder is A002-owned configuration/profile work. `pstvnc_app_run()` may therefore remain fail-closed until A002 supplies that producer.

## Validation disposition handed forward

The append-only findings register still formally contains:

- V003 OPEN / `WAIT_FOR_COHERENT_TRANCHE`;
- V004 OPEN;
- V005 HIGH / OPEN.

Those statuses are intentionally not rewritten by Foreman. Their previously stated missing handoff evidence is now present at exact machine authority `20b2e21...`; independent Validation must revision-chain or otherwise formally disposition them.

FOREMAN_A001_HANDOFF=VALIDATION_READY
VALIDATION_PASS=UNCLAIMED
HARDWARE_STATUS=HARDWARE_PENDING

## Reconstruction assignment status

RECONSTRUCTION_A_PACKET=NONE
RECONSTRUCTION_B_PACKET=NONE
INTERACTIVE_B_PACKET=NONE
A002_PACKET=NONE
REASON=A001_FOREMAN_INTEGRATION_COMPLETE;INDEPENDENT_VALIDATION_DISPOSITION_REQUIRED_BEFORE_A002

Reconstruction workers must remain idle on A001 unless Validation exposes a concrete reconstruction-owned defect. They must not begin A002 until the A001 Validation handoff boundary is formally consumed and Foreman publishes a fresh A002 packet.

## Exact next pickup

Independent Validation should consume exact run `35091578944` at `20b2e21...`, independently review the canonical host/runtime/lifecycle evidence, the complete strict dictionary/project checks, and the exact linked/PT_LOAD identity, then disposition V003/V004/V005. If Validation finds no reconstruction-owned defect and accepts the A001 tranche, Foreman may then open A002 planning. Physical PS2 qualification remains a separate `HARDWARE_PENDING` obligation and must not be inferred from CI.
