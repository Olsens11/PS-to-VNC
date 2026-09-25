DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T10:17:50-04:00
COMPLETED_AT=2026-09-25T10:35:01-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=501d6ade655e0ef68692ac42b9b9959c340f3e17
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A003 R25 ordinary Pi MPEG product composition

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`501d6ade655e0ef68692ac42b9b9959c340f3e17`.

Foreman State revision `0064` had accepted P10 and activated exactly:

`A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `ARCHITECTURE_BLOCKER=NONE`;
- PS2 permanent trigger/main-loop MPEG activation deferred;
- active-MPEG recalibration deferred;
- live-system mutation and real-FFmpeg testing prohibited.

The worker consumed current Reconstruction/Work Log contracts, State 0064,
Architecture Overlay, Wire Runtime Decisions, module lifecycle/source naming/
source topology authority, the accepted R15/R16A/R17 Pi Wire/RFB/MPEG source,
and the latest Foreman handoff.

The GitHub-connected Worker seat cannot observe external Pi/local checkout
staging, untracked files or local worktree dirtiness. No local reset, clean,
stash or checkout-over is claimed.

## Grounded selected product inputs

State 0064 authorized these existing product facts for ordinary R25 composition:

- native capture/provider display `:0`;
- exact clean desktop coordinate bounds `704x462`;
- exact-generation producer retirement deadline `8.0` seconds.

The Worker independently recovered the named frozen H1 authority at forensic
commit:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

and located:

`experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`

whose exact source declares:

`DEFAULT_RETIRE_TIMEOUT_SECONDS = 8.0`.

R25 carries forward only that product deadline value, not the historical
process wrapper or experimental diagnostics.

The accepted R17 Configuration-owned producer profile remains unchanged:

- channel window: 524288;
- producer buffer: 524288;
- max dimensions: 704x480;
- frame rate: 30000/1001.

## Result

R25 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

The packet activates the existing R17 factory seam in ordinary Pi composition
without changing R17 or WireServer mechanisms.

## Selected Pi MPEG composition profile

New maintained product source:

`pi/mpeg_product_profile.py`

owns only the composition inputs that are not R17 producer tuning:

- display `:0`;
- desktop width 704;
- desktop height 462;
- retirement deadline 8.0 seconds.

`MpegProductCompositionProfile` is immutable and
`selected_mpeg_product_profile()` validates the selected values.

The file synopsis records the exact H1 forensic source/commit for the 8.0-second
deadline and explicitly excludes Wire identity, FFmpeg launch, generation
identity, RFB provider state and R17 channel/buffer/frame-rate tuning.

## Ordinary product runtime composition

`pi/wire_runtime.py` now imports the accepted R17 generation owner and selected
composition/producer profiles.

It adds:

- `_make_mpeg_generation(session_id, composition, producer_profile)`;
- `selected_mpeg_generation_factory()`.

The factory constructs one fresh `MpegGenerationController` using:

- the exact nonzero Wire Session ID passed by WireServer after Q4;
- selected display/bounds/deadline;
- the existing selected R17 producer profile.

`build_product_wire_server()` now supplies both:

- the existing R14/R15 `rfb_attachment_factory`;
- the new R25 `mpeg_generation_factory`.

The ordinary runtime still owns no physical socket send/recv, sequence
allocation, START/RETIRE interpretation, producer launch, generation counter or
session retirement mechanism.

## Post-Q4 attachment fence

No change was made to `pi/wire_server.py`.

Focused tests use the real accepted WireServer and prove:

- incompatible product Q4 invokes the MPEG factory zero times;
- malformed sequence-2 establishment invokes the MPEG factory zero times;
- exact Q4 ACCEPT publishes a real allocated session identity first;
- only then is the MPEG factory invoked once with that exact session ID;
- the returned exact-session controller is attached through the existing
  WireServer seam.

Thus no provisional/rejected peer receives R17 authority.

## Producer-inert construction and CREDIT

Focused R25 tests use real `MpegGenerationController` with only process
ownership replaced by an in-memory fake producer factory.

After exact Q4/controller construction the test proves:

- controller producer is `None`;
- plan is `None`;
- highest_generation is zero;
- credit is zero.

A valid MPEG CREDIT then changes only the controller's existing
Wire-session-capacity fact. The fake producer factory remains uncalled.

Exact START is the first producer-factory invocation.

No real FFmpeg process is launched by R25 tests.

## Independent RFB/MPEG rider lifecycles

The ordinary product server carries both rider factories.

The focused MPEG lifecycle deliberately sends no RFB CREDIT and proves the
fresh RFB attachment's provider connect-attempt count remains zero while MPEG
CREDIT, START, DATA, RETIRE, N+1 and stale-generation handling all execute.

RFB therefore retains its own lazy channel-1 CREDIT edge while MPEG retains its
own exact START/RETIRE generation lifecycle.

Neither rider acquires the other's lifecycle authority.

## Sole physical Wire serializer preserved

No change was made to `pi/wire_server.py`,
`pi/mpeg_generation.py` or `pi/wire_protocol.py`.

The focused composition test exercises the real accepted WireServer, not a fake
physical serializer.

After exact START and fake producer creation, MPEG DATA is observed on channel
4 only after `WireConnectionOwner` serializes it.

Exact RETIRE completion is likewise observed through the owner's existing
control-channel serialization path.

Direct final-source scan of `pi/wire_runtime.py` plus
`pi/mpeg_product_profile.py` found:

- no `sendall(`;
- no `.recv(`;
- no `start_exact(`;
- no real `subprocess.Popen`;
- no PS2 `pstvnc_app_mpeg` reference;
- no START+SELECT trigger.

## Same-session generation history and fresh-session ownership

Within one exact Wire Session, focused R25 evidence uses one real R17
controller for:

- generation 1 START;
- exact generation 1 RETIRE/completion;
- generation 2 START;
- exact generation 2 RETIRE/completion.

The same controller retains `highest_generation == 2`.

A stale generation-1 START on that same session is rejected by R17 and makes the
physical session fail closed.

The final test-only hardening then proves two successful sequential physical
Wire Sessions on the same persistent server:

- Session A receives one exact controller object;
- clean Session A finish closes that controller;
- Session B receives a different session ID;
- Session B receives a distinct newly constructed controller object;
- Session B starts with no producer, no plan and generation high-water zero;
- clean Session B finish closes that exact new controller.

No dead-generation object, producer, suppression, lease, local buffer or wake
authority is reused across sessions.

## Exact session retirement

The accepted WireServer finish path remains unchanged and invokes the exact
attached controller's `close()`.

Focused R25 evidence injects a close result of false and proves the corresponding
otherwise-clean accepted Wire Session ends with `protocol_failed=true`.

The test also records the exact close call against the current session ID.

Timeout remains failure detection only; R17's real retirement proof remains the
owner mechanism.

## Factory failure containment

Focused evidence injects an exception from the first exact post-Q4 MPEG factory
invocation.

The accepted current physical session ends protocol-failed rather than:

- retrying the factory on the same session;
- rebinding the session;
- silently falling back to RFB-only interpretation.

The same persistent WireServer then accepts a later fresh physical session with
a new session ID and successfully constructs a new controller for that session.

## Safe product staging

Because R25 adds one maintained Pi product file,
`scripts/pi/install-wire-runtime.sh` now includes
`mpeg_product_profile.py` in:

- source existence/syntax checks;
- safe-target checks;
- stage;
- verify;
- remove.

The existing fail-closed service-state guard remains intact.

No daemon-reload, enable/disable, start/stop/restart, LightDM/Xorg/
X0tigervnc mutation or provider/display mutation was added or executed.

## Deliberate non-scope

R25 changes no:

- `pi/wire_server.py`;
- `pi/mpeg_generation.py`;
- `pi/wire_protocol.py`;
- PS2 `src/` product source;
- systemd unit/runtime state;
- AUDIO;
- calibration behavior;
- active-MPEG recalibration;
- Wire framing/protocol bytes.

The permanent PS2 MPEG entry gesture and ordinary main-loop P9/P10/R21-R24
composition remain deferred to later authority.

## Intermediate deterministic-test corrections

Three test-only follow-ups corrected fixture/temporal assumptions without
changing R25 product behavior.

1. The old R17 repository-boundary test explicitly required ordinary
   `wire_runtime.py` to omit `mpeg_generation_factory`. R25 intentionally
   supersedes exactly that dormant-composition assertion. The test now requires
   the factory seam while still proving runtime contains no direct
   `subprocess.Popen` or `start_exact(` call.
2. The first R25 fixture initially waited for an ordinary initial RFB CREDIT.
   Source review confirmed R13 grants reverse capacity only after its provider
   is composed; with no channel-1 client CREDIT the RFB provider correctly
   remains inert and emits no such initial frame. The fixture now directly
   proves zero provider-connect attempts instead.
3. The accepted-Q4 fixture initially waited for entry into the patched factory
   before indexing its controller publication list. Because the call record was
   appended immediately before construction, that was a host scheduling race.
   The fixture now waits for actual controller publication.

A final test-only hardening added explicit clean Session A -> clean Session B
distinct-controller proof for criterion C7.

## Dictionary reconciliation

After behavior/tests/docs stabilized, the Worker emitted the exact authorized
no-tree-change trigger:

`7f6b154df4dc8a4ec646051ae49b02b5ccbfa070`

message:

`tooling(symbols): run deterministic dictionary reconciliation`.

The repository-authorized reconciler produced:

`b395bbc0f727447dcb40a6c5c59bddecc8ef89f7`

message:

`docs(symbols): reconcile current clean definitions`.

That generated commit changed only:

- `pi/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No dictionary tooling implementation changed.

## Commits

The complete pre-log Worker range is ten commits ahead / zero behind starting
Foreman authority:

1. `aafc2fb583b388f7c1cd05e42b5dcbe28ad934a2` —
   `pi: compose ordinary MPEG generation factory`;
2. `7bc38563034e4e737e907b202d87607ae15ac05f` —
   `test(pi): prove R25 ordinary MPEG composition`;
3. `a9312a7d9653c5c290747d31133513a74401582c` —
   `test(pi): supersede dormant R17 composition assertion`;
4. `651e0ca50490d30d3106bb6d7b32cc44f1b17050` —
   `test(pi): align R25 with lazy RFB credit edge`;
5. `839ae894d4a66288806f0ed6c8d2ba5d95f251d3` —
   `test(pi): wait for exact R25 controller publication`;
6. `8f3e8e7137232f633b878dec2b3b187b8d018ddb` —
   `docs(pi): record R25 MPEG product composition`;
7. `7f6b154df4dc8a4ec646051ae49b02b5ccbfa070` —
   deterministic dictionary reconciliation trigger;
8. `b395bbc0f727447dcb40a6c5c59bddecc8ef89f7` —
   generated Pi dictionary/portal reconciliation;
9. `db86b263bbf326509a3d1c0ac441454c39989e15` —
   `test: verify final R25 Pi MPEG composition authority`;
10. `60b7759fb78d5f555a589b9ce8cb58ce96096945` —
    `test(pi): prove fresh R25 controller per session`.

No concurrent branch movement or write collision occurred during Worker writes.

## Changed paths

Exact compare from starting authority
`501d6ade655e0ef68692ac42b9b9959c340f3e17` to final pre-log authority
`60b7759fb78d5f555a589b9ce8cb58ce96096945` is ten commits ahead / zero
behind.

Changed paths are only:

- `pi/mpeg_product_profile.py` — new;
- `pi/wire_runtime.py`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `scripts/pi/install-wire-runtime.sh`;
- `tests/unit/pi_mpeg_product_composition_test.py` — new;
- `tests/unit/pi_mpeg_generation_test.py`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R25-C1 SELECTED_PI_MPEG_COMPOSITION_PROFILE_HAS_GROUNDED_DISPLAY_BOUNDS_AND_RETIRE_DEADLINE`
  — MET.
- `A003-R25-C2 ORDINARY_RUNTIME_SUPPLIES_ONE_FRESH_EXACT_SESSION_MPEG_FACTORY`
  — MET.
- `A003-R25-C3 MPEG_CONTROLLER_ATTACHMENT_REMAINS_POST_Q4_ACTIVE_ONLY`
  — MET.
- `A003-R25-C4 CONTROLLER_CONSTRUCTION_AND_CREDIT_ARE_PRODUCER_INERT_UNTIL_EXACT_START`
  — MET.
- `A003-R25-C5 RFB_AND_MPEG_RIDERS_COEXIST_WITH_INDEPENDENT_LIFECYCLES`
  — MET.
- `A003-R25-C6 WIRE_CONNECTION_OWNER_REMAINS_SOLE_PHYSICAL_MPEG_SERIALIZER`
  — MET.
- `A003-R25-C7 SAME_SESSION_REPEATED_GENERATIONS_PRESERVE_HIGH_WATER_FRESH_SESSION_REPLACES_OWNER`
  — MET.
- `A003-R25-C8 SESSION_FINISH_REQUIRES_PROVEN_MPEG_CONTROLLER_RETIREMENT`
  — MET.
- `A003-R25-C9 FACTORY_OR_ATTACHMENT_FAILURE_TERMINATES_SESSION_WITHOUT_REBIND_OR_FALLBACK`
  — MET.
- `A003-R25-C10 NO_PS2_TRIGGER_MAIN_LOOP_RECALIBRATION_OR_AUDIO_SCOPE_CREEP`
  — MET.
- `A003-R25-C11 NO_LIVE_PI_SYSTEM_OR_REAL_FFMPEG_TEST_MUTATION`
  — MET.
- `A003-R25-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN`
  — MET.

All twelve R25 criteria are therefore `MET` as Worker dispositions.

## Final machine evidence

Exact final pre-log authority:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

Canonical GitHub Actions workflow:

`36148209311`

attempt:

`1`

conclusion:

`SUCCESS`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed final Pi regression evidence includes:

- `pi_wire_server_test.py`: 20 tests, OK;
- `pi_internal_rfb_provider_test.py`: 9 tests, OK;
- `pi_mpeg_generation_test.py`: 12 tests, OK;
- `pi_mpeg_product_composition_test.py`: 9 tests, OK;
- Pi RFB attachment/native-provider/runtime-profile suites — OK.

Observed cross-domain regression evidence includes:

- `transport_mpeg_test: PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=212 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked PS2 identity:

- `ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`;
- `PT_LOAD_BYTES=512788`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

This is exactly the Foreman-accepted P10 PS2 loadable identity. R25 therefore
changes maintained Pi/runtime bytes but changes no PS2 PT_LOAD bytes.

No hardware/operator qualification is claimed for R25 Pi composition. The
existing accepted P10 PS2 identity remains hardware-pending under prior
authority.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R25`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`PS2_PT_LOAD_CHANGED=NO`
`PI_PRODUCT_BYTES_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO_NEW_CLAIM`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0064`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted R15/R16A/R17 Pi authority;
- frozen H1 retirement deadline authority named by State 0064.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no R17 mechanism revision;
- no WireServer mechanism revision;
- no PS2 Application activation;
- no permanent controller trigger;
- no active-MPEG recalibration;
- no live system mutation;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No R25 source blocker remains within the authorized packet.

The accepted WireServer and R17 public seams were sufficient. Neither
`pi/wire_server.py` nor `pi/mpeg_generation.py` required modification.

The permanent PS2 MPEG trigger/main-loop composition remains downstream
authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R25_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect R25 source/evidence/log, decide acceptance,
and select any later bounded packet. Reconstruction must not self-accept R25 or
begin PS2 ordinary MPEG product activation.
