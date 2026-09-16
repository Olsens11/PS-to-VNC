# Interactive Reconstruction — A001 public Transport/RFB bridge

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T20:22:10-04:00
COMPLETED_AT=2026-09-15T20:40:05-04:00
ELAPSED=00:17:55
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=interactive
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=151ab77c8b1afe2ce18db29fde2f5ba9f0d77100
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Pick up the reconstruction capacity left unused by Reconstruction B's bounded bridge-design-gate shift and continue the same active `a001-sole-receiver` item as a sustained reconstruction shift. Consumed reconstruction contract revision 0002, immutable work-log format revision 0001, audit state revision 0007/A001 disposition, reconstruction state revision 0006, B's 20:09 bridge-design-gate handoff, the newer Validation A001 handoff, clean architecture/ledge overlay, current source-topology and symbol-dictionary policy, and pinned forensic H1 authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

Validation and Continuity advanced documentation/global authority while this shift was active. Their changes were re-read and did not overlap the reconstructed product source being modified here. Continuity advanced global state to revision 0011 and explicitly characterized the bridge seam as a bounded reconstruction design gate rather than justification for idle capacity. Branch and reconstruction-state authority were re-read between bounded writes. Unknown external Pi-local dirty work remained outside this GitHub-native mutation surface and was neither overwritten nor declared absent.

## Work performed

This shift did not stop at Reconstruction B's observation that the provisional Transport session surface lacked enough validated CONFIG-derived values. It followed that question through the existing audit, H1 forensic source, clean architecture, current RFB/application/platform call chain, and configuration ownership evidence and then made several bounded source checkpoints:

- replaced the provisional socket-plus-enable Transport surface with one explicit stable `pstvnc_transport_session_config_t` carrying the already-validated A001 queue, credit, receiver-thread, and maximum-DATA-payload values required by the runtime; no concrete defaults were invented;
- made `runtime.h` consume the stable cross-component config value and retained the former runtime-config name only as an exact transitional alias so no second configuration/default authority exists;
- created Transport's single `bridge.h/.c` cross-component bridge, organized by application session lifecycle, logical RFB delivery, and finite-session quiesce processes;
- made descriptor adoption ownership explicit: successful runtime adoption clears the caller descriptor to `-1`; failed initialization leaves caller ownership unchanged; failed receiver start retires already-adopted Transport resources rather than leaving ambiguous double-close ownership;
- made bridge close refuse receiver-visible resource reclamation while a started receiver is not complete;
- exposed logical RFB exact-read/readiness/exact-write and ordered REQUEST/BOUNDARY/COMMIT/residual/COMPLETE operations without exposing the physical descriptor or moving complete-RFB-message safe-boundary policy into Transport;
- migrated `src/transport/SYMBOLS.md` to canonical directory/generation/coverage metadata plus seven-column inventory while deliberately leaving `COVERAGE=IN_PROGRESS` and V004 open;
- added `tests/unit/transport_bridge_test.c`, using deterministic runtime stubs to cover ownership transfer, failed-open ownership, failed receiver-start cleanup, live-receiver close refusal, receiver-completion release, logical RFB result mapping, and quiesce wrapper mapping;
- created RFB's own `bridge.h/.c` logical-stream adapter, whose protocol-facing read/poll/write interface carries no socket identity and translates Transport result semantics into RFB's small exact-stream contract;
- advanced reconstruction current state from revision 0006 to revision 0007 with the new phase `A001_PUBLIC_BRIDGE_SOURCE_IN_PROGRESS`.

## Design gate resolution / evidence reasoning

Reconstruction B was correct that the old provisional `pstvnc_transport_session_open(int socket_fd, int enable_rfb)` could not initialize the new runtime without guessed values. The available evidence was nevertheless sufficient to continue rather than stop:

- the pinned H1 CONFIG/runtime authority already identifies the exact queue/credit/receiver-thread/payload value family Transport needs;
- clean architecture says App/config owns configuration choice while Transport owns the shared physical/logical transport mechanism;
- the clean RFB seam must consume a logical byte stream rather than retain raw socket ownership;
- current `src/config/` is only generic retained text parsing, so this shift defined the required validated Transport value boundary but intentionally did **not** invent concrete application defaults.

That resolves enough of the design question to build the ownership/bridge structure while preserving the remaining typed-config/application integration work as an explicit follow-on.

## Cross-check against current and forensic RFB I/O

The pinned H1 logical-RFB adapter confirms the intended architecture: no second physical receive owner, logical channel-1 read/poll/write behind Transport, and the socket-shaped H1 adapter was only a compatibility identity seam around logical mechanics. The clean ledge bridge goes one step further and removes physical socket identity from the new RFB bridge API entirely.

However, current clean product integration is still incomplete: `src/rfb/rfb_session.*` still carries the older socket-shaped `rfb_io.h` contract, `src/platform/ps2_network.c` still implements direct socket RFB exact read/poll/write, and `src/app.c` still establishes/passes the pre-ledge RFB socket. The new bridge is therefore source reconstruction progress, not yet proof that the linked DUT has eliminated raw-socket-shaped RFB authority.

## Exact commits / files

Product/reconstruction commits made by this interactive shift:

- `2d56e05bccd1cfec2d4894caf2b6d9bdef32f838` — `transport: define validated session config boundary`; updates `src/transport/transport.h`.
- `fdf561b975cb87f2c4df9d029a386eff3837788b` — `transport: consume stable session config in runtime`; updates `src/transport/runtime.h`.
- `478b03e56801e779f252e01629193bae3952b5d2` — `transport: keep runtime config spelling as stable-value alias`; keeps one value authority while existing runtime implementation transitions.
- `745104865f3586c54c93d684664d54172a62586b` — `transport: define single cross-component bridge surface`; adds `src/transport/bridge.h`.
- `e5c96e40b377aea8349481c0500fcbcdae79cb33` — `transport: implement session and RFB bridge processes`; adds `src/transport/bridge.c`.
- `53ba4f0a73a2e83fcbb5e3cb62967d1a2d415eb3` — `docs(transport): adopt canonical in-progress symbol dictionary`; rewrites `src/transport/SYMBOLS.md` into canonical in-progress form.
- `af4a148936f2447d36b76b4914bc43bfa304f0a7` — `test(transport): cover bridge ownership and lifecycle mapping`; adds `tests/unit/transport_bridge_test.c`.
- `523675210ca4740095239fc47d245bc4aea1d931` — `rfb: define logical Transport byte-stream bridge`; adds `src/rfb/bridge.h`.
- `de801156672d5ad5cf4012cff2abdb68214d41d1` — `rfb: adapt protocol I/O to logical Transport bridge`; adds `src/rfb/bridge.c`.
- `3915ea1c8d1578d2b643915496e317a31fc3b5c4` — `docs(ledge): advance A001 public bridge reconstruction state`; produces reconstruction state revision 0007.

Cross-lane commits observed/reconciled during the shift include Validation's `cb566e04236882ae8f7aad4ce17a932d5ea787f2` and Continuity/global reconciliation through `f734299c6e994aa22c732bd0717589abf45d77a2` / `c7fe00a69ec3616c6315fdd3f047e15198803a72`; those did not overlap this shift's product-source ownership.

## Checks / evidence

PASS / PROVISIONAL PASS by repository and source inspection only:

- the new public Transport config boundary contains the required H1-derived runtime value family and manufactures no defaults;
- the runtime and public config spelling are one C type, not duplicated configuration authorities;
- Transport has one process-organized cross-component bridge body rather than destination-specific transport bridge sprawl;
- the new RFB bridge API contains no physical socket parameter;
- Transport session-open ownership transfer is explicit and avoids caller/Transport double-close ambiguity on both successful adoption and failed receiver start;
- bridge close does not reclaim started-receiver state before explicit receiver completion;
- complete-RFB-message safe-boundary choice remains outside Transport;
- the bridge host-test source covers meaningful ownership/lifecycle/result cases;
- the Transport dictionary now uses canonical metadata/table shape and truthfully remains `IN_PROGRESS`.

PENDING_LOCAL / not claimed PASS:

- `transport_bridge_test` compilation/execution; its source exists but this shift did not yet wire it into `tests/Makefile`;
- already wired `transport_rfb_channel_test` execution and full host unit suite;
- PS2DEV compilation of runtime/quiesce/bridge/channel changes;
- canonical `scripts/check.sh`;
- strict definition-level dictionary and topology checks;
- generated source-dictionary portal synchronization;
- linked clean-product build integration, reproducibility, exact ELF identity, and PT_LOAD evidence;
- PS2 hardware execution/qualification.

## Dictionary / topology consequences

V004 remains OPEN.

`src/transport/SYMBOLS.md` is now structurally closer to current policy but remains intentionally `COVERAGE=IN_PROGRESS`. The newly added `src/rfb/bridge.h/.c` make the previously complete `src/rfb/SYMBOLS.md` stale until those definitions are indexed. The generated portal still requires regeneration/verification. Current `docs/development/source-topology.md` and the active domain allowlist in `scripts/continuity-check.sh` still predate `src/transport`; that current-policy integration must be deliberate rather than hidden by hand-edited generated output.

## Known defects / blockers

No inherited H1 defect was silently fixed. The historical post-session receiver/mailbox poison remains unresolved and visible. No generic timeout or nonblocking-receive workaround was introduced.

There is no persistent reconstruction-source blocker at handoff. The remaining A001 integration problem is substantial but understood: migrate RFB session/application/platform use from the old socket-shaped I/O path to the new logical bridges while sourcing Transport session values from explicit validated configuration authority, then close topology/build/dictionary/evidence gaps and prove lifecycle behavior.

## State / contract revisions

Consumed: reconstruction contract 0002; work-log protocol 0001; audit state 0007/A001; reconstruction state 0006; global state 0010 then reconciled global state 0011; current Validation V003/V004 authority.

Produced: reconstruction state revision 0007 and this immutable shift record.

## Exact next pickup

Continue `a001-sole-receiver` from reconstruction state 0007. First finish independent bridge evidence/metadata: wire and execute `transport_bridge_test` on an execution-capable surface, index `src/rfb/bridge.*`, complete Transport definition-level inventory, and deliberately adopt `src/transport` into current topology/checker/portal authority.

Then migrate `src/rfb/rfb_session.*`, `src/app.c`, and the obsolete direct RFB I/O responsibility in `src/platform/ps2_network.c` coherently onto `src/rfb/bridge.*` / `src/transport/bridge.*`. Supply the stable Transport session config only from explicit validated configuration authority; do not invent queue/credit/thread/payload defaults. Preserve one physical PSTV receiver, parser-consumption credit, complete-message safe-boundary ownership outside Transport, explicit receiver completion before reclamation, and the historical receive-poison defect boundary. Do not begin A002 while A001 remains incoherent.

This interactive shift ends at a coherent handoff before Reconstruction A's upcoming scheduled window rather than competing with the next reconstruction writer.
