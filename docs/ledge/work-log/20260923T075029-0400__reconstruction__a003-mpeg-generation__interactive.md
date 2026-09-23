DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-23T07:50:29-04:00
COMPLETED_AT=2026-09-23T08:01:35-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=102010653642af43e7b1e32d585c4d16b280b12f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R20 private MPEG Wire-session binding

## Timestamp provenance

The exact `STARTED_AT` above was captured after the initial read-only recovery
of live Foreman/repository authority and before the first R20 source mutation.
No earlier wall-clock value is invented for the preceding read-only lookup.

## Objective and authority consumed

This interactive Reconstruction shift recovered live authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0051`, whose active
bounded packet was:

`A003-MPEG-PRIVATE-SESSION-BINDING-R20`

The assigning branch authority was:

`102010653642af43e7b1e32d585c4d16b280b12f`

R20 required the public PS2 Transport MPEG generation-control seam to stop
requiring callers to manufacture Transport-private Wire identity or protocol
version fields. Callers were to express only run-owned generation/geometry
meaning; Transport was to stamp the active Q4 Wire session identity and accepted
control version below the bridge while preserving the existing START/RETIRE
bytes exactly.

Governing authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0006;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0051;
- the assigning Foreman immutable log
  `docs/ledge/work-log/20260923T072100-0400__foreman__a003-mpeg-generation__interactive.md`;
- `docs/development/source-naming-and-symbols.md`;
- `docs/development/module-lifecycle.md`;
- `docs/development/mpeg-generation-control.md`;
- current Transport bridge/runtime/protocol source and focused fixtures;
- accepted R18 MPEG Transport run-boundary authority;
- accepted R17 Pi exact-session/exact-generation authority.

The GitHub connector exposes repository/branch authority but not the external
developer Pi worktree. External staged/unstaged/untracked state and local
`scripts/resume-state.sh` execution remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim is made about
external worktree cleanliness.

## Result

R20 is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`3e39753b1b3bce9fe187748deb7eeb4d6201151c`

The public Transport boundary now uses three owner-correct semantic values:

- `pstvnc_transport_mpeg_start_request_t` — generation plus base/suppression
  geometry only;
- `pstvnc_transport_mpeg_retire_request_t` — generation only;
- `pstvnc_transport_mpeg_retire_completion_t` — completed generation only.

None contains a Wire session ID or generation-control protocol version.

At the bridge:

1. current opaque Transport access is validated first;
2. START validates only Transport-knowable structural run facts:
   - nonzero generation;
   - base dimensions at least 16 pixels and 16-pixel aligned;
   - nonempty overflow-safe base/suppression rectangles;
   - suppression contains the base rectangle;
3. desktop bounds and selected MPEG-profile limits remain outside Transport;
4. Transport stamps `PSTVNC_MPEG_GENERATION_CONTROL_VERSION` and
   `pstvnc_transport_bridge_private_session_id` into the existing private
   full START payload;
5. RETIRE similarly stamps version/private session identity around caller-owned
   generation;
6. unchanged runtime/protocol internals continue to operate on the accepted full
   wire payload structs;
7. runtime continues full submitted-RETIRE correlation before publishing a
   completion;
8. bridge completion consumption revalidates current private version/session
   authority and projects only generation above Transport;
9. an impossible bridge-level private-identity contradiction marks the current
   Transport runtime failed instead of leaking/repairing identity upward.

No public Q4 session-ID accessor was added. The existing private
`pstvnc_transport_bridge_private_session_id` remains file-local Transport
authority and is still cleared when its Wire Session dies.

No START/RETIRE protocol codec, frame kind/channel/flags, Q4 product
compatibility, fixed PSTV header version, runtime R18 lifecycle mechanism or Pi
R17 product source changed.

## Exact wire representation preserved

Protocol codec source was untouched.

Accepted representation remains:

- START frame kind 11;
- control channel 0;
- flags 0;
- 44-byte payload;
- generation-control version 1;
- big-endian fields in the existing version/session/generation/geometry order;
- RETIRE frame kind 10;
- control channel 0;
- flags 0;
- 12-byte version/session/generation payload.

Existing protocol codec tests remained green on the final source head.

## Stale-session and completion proof

Focused bridge/runtime tests prove:

- Session A with private ID `0x10203040` stamps exact START/RETIRE payloads
  with A's identity even though the public requests cannot carry that field;
- after A is retired and Session B establishes private ID `0x55667788`, A's
  stale access ticket is rejected before runtime control submission;
- the same caller-owned START/RETIRE meaning used with B can stamp only B's
  current private identity;
- successful exact completion is exposed above the bridge only as one generation
  value;
- an impossible private-session contradiction at the bridge fails the Transport
  runtime closed and exposes no private identity;
- runtime exact correlation rejects wrong generation, wrong session, and wrong
  control version without latching retirement or surfacing a completion.

Public test values also prove the RETIRE request and completion are each exactly
one `uint32_t`, while START is the nine caller-owned generation/geometry
words. No private session/version field exists in those public structs.

## Structural validation boundary

R20 deliberately does not move calibration/display/profile ownership into
Transport.

Transport rejects before runtime admission:

- generation zero;
- empty/too-small base dimensions;
- non-16-aligned base dimensions;
- empty/overflowing rectangles;
- suppression that cannot contain the base rectangle;
- null semantic requests/completion outputs;
- stale/invalid Transport access.

Current desktop bounds and selected MPEG producer-profile maximums remain with
their existing owners and the accepted Pi R17 generation controller.

## Commits

The complete pre-log R20 range is eight commits ahead / zero behind the assigning
Foreman authority:

1. `0486f9e208944bc2760da20a515a4c8273805e75` —
   `transport: bind MPEG control to private Wire identity`
2. `fe32adfe8abc581747169e8ca1da164a4ec69dcd` —
   `test(transport): prove R20 private MPEG identity seam`
3. `778cdfd447f5cf0df1ee7f0da0342f5d3877c02c` —
   `test(transport): prove full MPEG completion correlation`
4. `5c0904d38cf86c1d4ff7106e2f9c4e9eaaba7b84` —
   `transport: fail closed on MPEG identity contradiction`
5. `b8550223d6d52eafe740c180be9b674bb1bc5281` —
   `docs(transport): record R20 private MPEG control seam`
6. `c73046e862bb0af6646b14345c9011e766523ca9` —
   `tooling(symbols): run deterministic dictionary reconciliation`
7. `399f1ecbf9a4d44426aefa8de663a34d4fe96998` —
   `docs(symbols): reconcile current clean definitions`
8. `3e39753b1b3bce9fe187748deb7eeb4d6201151c` —
   `test: verify final R20 MPEG identity authority`

## Changed paths

The complete assigning-Foreman-to-pre-log diff is confined to the authorized R20
Transport/test/documentation/dictionary surface:

- `src/transport/transport.h`
- `src/transport/bridge.h`
- `src/transport/bridge.c`
- `tests/unit/transport_bridge_test.c`
- `tests/unit/transport_mpeg_test.c`
- `src/transport/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- `docs/development/mpeg-generation-control.md`
- `docs/development/module-lifecycle.md`

Notably unchanged:

- `src/transport/runtime.c/.h` product implementation;
- `src/transport/protocol.c/.h` codec implementation;
- `src/app.c`;
- `src/app_mpeg_frame.*`;
- Pi product source;
- RFB product source;
- MPEG decoder/worker/backend;
- Display/Presentation;
- calibration/Input/UI;
- AUDIO product source;
- Configuration product source.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R20-C1 PUBLIC_START_REQUEST_CONTAINS_NO_PRIVATE_WIRE_IDENTITY` — MET.
- `A003-R20-C2 TRANSPORT_STAMPS_CURRENT_PRIVATE_SESSION_AND_VERSION_ON_START` — MET.
- `A003-R20-C3 PUBLIC_RETIRE_REQUEST_CONTAINS_ONLY_EXACT_RUN_MEANING` — MET.
- `A003-R20-C4 COMPLETION_REMAINS_FULLY_CORRELATED_INSIDE_TRANSPORT` — MET.
- `A003-R20-C5 PUBLIC_COMPLETION_DOES_NOT_LEAK_PRIVATE_SESSION_OR_VERSION` — MET.
- `A003-R20-C6 NO_PUBLIC_Q4_SESSION_ID_ACCESSOR_OR_DUPLICATE_OWNER` — MET.
- `A003-R20-C7 START_RETIRE_WIRE_BYTES_AND_PRODUCT_COMPATIBILITY_UNCHANGED` — MET.
- `A003-R20-C8 STALE_ACCESS_OR_REPLACEMENT_SESSION_CANNOT_REUSE_OLD_IDENTITY` — MET.
- `A003-R20-C9 R18_RUN_BOUNDARY_AND_FINALIZATION_CONTRACTS_UNCHANGED` — MET.
- `A003-R20-C10 PI_R17_EXACT_SESSION_GENERATION_BEHAVIOR_UNCHANGED` — MET.
- `A003-R20-C11 NO_APPLICATION_CALIBRATION_MEDIA_OR_PI_SCOPE_CREEP` — MET.
- `A003-R20-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`3e39753b1b3bce9fe187748deb7eeb4d6201151c`

Exact-head workflow:

- workflow: `Ledge reconstruction checks`
- run: `35857721371`
- attempt: `1`
- conclusion: `success`
- head SHA: `3e39753b1b3bce9fe187748deb7eeb4d6201151c`

Job results:

- `host-unit` — SUCCESS
  - `transport protocol tests passed`
  - `transport bridge tests passed`
  - `transport_runtime_test: PASS`
  - `transport_mpeg_test: PASS`
  - `RFB_FLOW_POLICY_TEST=PASS`
  - Pi R17 generation fixture: 12 tests, OK
  - `app R15/R16B/R19 tests: PASS`
  - `transport_rfb_provider_failure_test: PASS`
- `project-check` — SUCCESS
  - `WORK_LOG_CHECK=PASS`
  - `SOURCE_DICTIONARIES=PASS`
  - `PS_TO_VNC_PROJECT_CHECK=PASS`
- `dictionary-long` — SUCCESS
  - `SOURCE_DICTIONARY_CHECK_MODE=LONG`
  - `SOURCE_DICTIONARIES=PASS`
- `ps2-compile` — SUCCESS
- `ps2-link` — SUCCESS
  - `ISSUE7_LINKED_BUILD=PASS`
  - `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`
- `dictionary-reconcile` — SKIPPED as expected on the non-trigger final
  evidence commit.

Final linked identity:

- `ELF_PRISTINE_SHA256=43f2c43f537a32f7205ab4b7711f0c53a6ac8bdb049818f5416d24a14fdb63d0`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
- `PT_LOAD_BYTES=491540`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R20 changes PT_LOAD from accepted R19
(`47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`,
491156 bytes) to this new identity. Repository reproducibility does not transfer
physical qualification.

## Intermediate evidence

The frozen pre-reconciliation source at
`b8550223d6d52eafe740c180be9b674bb1bc5281` passed host-unit,
project-check, pinned PS2 compile/link and current-source reproducibility.
`dictionary-long` correctly failed only because the new Transport definitions
had not yet been deterministically reconciled.

The canonical reconciliation trigger
`c73046e862bb0af6646b14345c9011e766523ca9` produced
`399f1ecbf9a4d44426aefa8de663a34d4fe96998`, changing only
`src/transport/SYMBOLS.md` and the generated source-dictionary portal.
No product source changed during reconciliation.

The content-identical checkpoint `3e39753...` binds final canonical evidence to
that reconciled authority.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R20_INDEPENDENT_VALIDATION=NOT_RUN`
- `R20_OPERATOR_OBSERVED=NO`
- `R20_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine/host evidence proves source behavior and reproducibility only. The new
PT_LOAD identity remains hardware debt until separately qualified.

## State/contract accounting

Consumed:

- Foreman State revision `0051`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted R19 source/log authority named by State 0051;
- accepted R18 Transport run-boundary authority;
- accepted R17 Pi exact-session generation authority.

Produced:

- no Foreman state revision;
- no Reconstruction-contract revision;
- no Wire/protocol version revision;
- no packet selection or Foreman acceptance.

## Findings / blockers

R20 has no remaining known source blocker within the authorized packet.

The owner-correct Transport seam is now ready for a later separately authorized
Application MPEG generation transaction. This worker did not allocate
Application generation numbers, choose calibration/product geometry, invoke RFB
freeze/thaw, start MPEG workers, arm Presentation, modify Pi product source,
activate AUDIO, or begin final orchestration.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R20_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
current repository authority, inspect this source/evidence/log, decide R20
acceptance, publish any Foreman-owned state/integration evidence, and select any
next bounded packet. Reconstruction must not self-accept R20 or begin the queued
Application MPEG transaction from this record.
