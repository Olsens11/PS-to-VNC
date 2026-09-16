# Ledge Reconstruction Foreman Shift — A002 PCM integration and audio-worker packet

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T12:59:32-04:00
COMPLETED_AT=2026-09-16T13:15:55-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2f1e71d9f277417fbe0292ed7aa52eb2c12abf32
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Took the interactive Foreman baton after the completed A002 synchronous PCM/AUDSRV playback-core Reconstruction handoff. This round independently inspected the actual P1-P8 product source/tests, checked the potentially ambiguous AUDSRV API semantics against current PS2SDK EE authority, accepted or rejected the worker result criterion-by-criterion, performed only Foreman-owned canonical integration/evidence work, preserved concurrent support movement, issued the next bounded A002 worker-lifecycle/common-clock packet, and inspected the settled state-0011 workflow before closing.

Authority consumed included:

- Reconstruction contract revision `0005`;
- work-log contract revision `0005`;
- Foreman state revision `0010` and its active P1-P8 PCM playback-core packet;
- global work state revision `0028`;
- Validation state revision `0006` and findings revision `0005`, which remain independent A001 authority only;
- A002 audit `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision `0001`;
- Reconstruction handoff commit `2f1e71d9f277417fbe0292ed7aa52eb2c12abf32` and its canonical immutable log;
- current clean CONFIG, Transport AUDIO, common-media-clock, and audio source/test authority;
- current PS2SDK EE AUDSRV implementation for API-semantic verification;
- non-authoritative Architecture/dictionary support records only as planning/support evidence.

The exact Foreman round start was captured before work as `2026-09-16T12:59:32-04:00`. Unknown external/Pi-local dirty work remains outside this GitHub-native surface and was neither overwritten nor declared absent.

## Reconstruction P1-P8 handoff inspected

Worker immutable handoff:

`docs/ledge/work-log/20260916T123335-0400__reconstruction__a002-audio-clock__interactive.md`

Worker result:

`FOREMAN_GOAL_RESULT=MET`

Substantive commits inspected:

- `103791438a4afae42edd67224466f0767a37b790` — define synchronous PCM playback core;
- `e1d6a8f60e54cf933d2b326182e184605850cb5f` — implement playback core;
- `ad36d3a88da9c3a0bc4520975ba6f8c6219afbf2` — declare resident AUDSRV adapter;
- `c774fdda51acaa4b107793ee3b81e6eaf9c78b0c` — bind PCM core to resident AUDSRV;
- `2ce299ca6af66b0cb6c1accb69137a705949915d` — host AUDSRV stub;
- `206a07162d8d14ea01e189c533ce73a02cf1a643` — deterministic playback semantics tests;
- `87af39599449a68849b69c45513e4d4dfcd95a42` — concrete AUDSRV adapter tests;
- `d21eb53ce41859ba1d04cfa94846c0d665709927` — clean audio dictionary;
- `7d00270af88149ab7c0b61d771cf6f5ef7b65f72` — clean audio-domain topology adoption;
- `2f1e71d9f277417fbe0292ed7aa52eb2c12abf32` — immutable Reconstruction closeout.

Directly inspected product files included `src/audio/playback.{c,h}`, `src/audio/audsrv_service.{c,h}`, `src/audio/SYMBOLS.md`, `tests/unit/audio_playback_test.c`, `tests/unit/audio_audsrv_service_test.c`, `tests/unit/audio_host_stubs/audsrv.h`, CONFIG PCM profile authority, public Transport AUDIO bridge authority, current common-media-clock API, and A002 audit semantics.

## P1-P8 Foreman disposition

P1 `MET` — `src/audio` is a coherent narrow clean owner. The synchronous core depends on immutable PCM profile authority, the public Transport AUDIO seam, and injected service operations rather than Transport internals or application-owned mutable playback state.

P2 `MET` — resident AUDSRV service semantics are explicit. Session cleanup exposes stop/mute but no `audsrv_quit()` path. A Foreman API cross-check resolved a potential unit ambiguity: current PS2SDK EE `audsrv_set_volume()` accepts the public 0-100 volume scale and performs the SPU2 conversion internally, matching `volume_percent`. The EE `audsrv_init()` wrapper also treats an already-bound RPC client as initialized rather than creating a second lifetime owner.

P3 `MET` — AUDIO consumption stays on the public Transport bridge. Temporary unavailability snapshots/waits on Transport activity rather than busy-spinning. Normal finite `EXHAUSTED`, stop, closed, invalid, and terminal failure remain distinguishable.

P4 `MET` — every nonzero selected chunk performs wait-before-play and advances successful-byte/chunk accounting only after the play operation reports exact full-byte success. Current PS2SDK EE source confirms `audsrv_play_audio()` returns the total bytes actually sent, so the worker's exact-count success requirement is semantically correct and fail-closed.

P5 `MET` — setup, Transport, wait, play, and cleanup failures retire deterministically. Cleanup does not overwrite a prior meaningful error and playback does not continue after terminal outcome.

P6 `MET` — deterministic host tests cover exact setup/volume/format ordering, partial and multiple chunks, zero-byte avoidance, temporary-unavailability rendezvous, normal exhaustion, stop/closed/failure distinctions, wait failure, partial/negative play failure, truthful accounting, cleanup behavior, and absence of a quit operation.

P7 `MET` — the concrete AUDSRV adapter faithfully maps the narrow service boundary to current EE API behavior without creating a second service owner.

P8 `MET` — no worker thread/stack lifecycle, production resource defaults, startup reservoir, audio common-clock presentation gate, application orchestration, MPEG/presentation behavior, receive-poison repair, or hardware qualification was introduced.

No product-source correction packet was required.

## Foreman-owned canonical integration

Foreman made only canonical integration/evidence changes:

- `b2e93dceac75d80d29351bad380cae96d5f206f6` — `ci(a002): stage audio dictionary reconciliation`; allowed generated reconciliation to stage `src/audio/SYMBOLS.md`.
- `ba1ab97c1e8926d7ce05dc5505ce7e969601e945` — `tooling(a002): compile all adopted clean domains`; corrected the strict clean-PS2 compile inventory so it now includes all already-adopted A002 CONFIG/media/Transport AUDIO source plus the new `src/audio` source rather than claiming to compile every clean unit while omitting those domains.
- `4c3628d21b190783757dc77f48a59a56d7bf3b24` — `build(a002): link synchronous audio core`; added `audio_playback.o`, `audio_audsrv_service.o`, `src/audio` include authority, and PS2SDK `-laudsrv` to the clean linked build.
- `1f4bb786a300b508e68b13472748b7a559d8562b` — `test(a002): register synchronous audio fixtures`; registered both new behavior fixtures in canonical `make -C tests unit`.
- `473b4b111f5dbdd2b5db830269e23fc4c8d89387` — `tooling(a002): admit clean audio domain`; updated the clean topology checker to recognize `src/audio`.
- `37ae75801f7ab558c3b6b0cbb2e8a7879790421f` — exact trigger `tooling(symbols): run deterministic dictionary reconciliation`; adopted `src/audio` in the canonical reconciliation helper.
- generated child `6d25e2c0cea036d1f7cd57b42b4bf55df8d05548` — `docs(symbols): reconcile current clean definitions`; regenerated the portal. The local worker-authored `src/audio/SYMBOLS.md` required no generated correction.
- `edd0d7a78f315234119c9f81f9eee507d955bf24` — `docs(a002): register clean audio owner`; added the new audio responsibility to the living file/service map.
- `784e638d133c84acc32b8b5ff244268137b0298a` — Foreman state revision `0011`, phase `A002_AUDIO_WORKER_CLOCK_PACKET_ISSUED`, containing the fresh W1-W8 packet.

The living-map whole-file replacement in `edd0d7a7...` incidentally changed one historical M0 phrase from “durable executable byte authority” to the semantically equivalent “durable binary authority.” This is recorded as nonsemantic documentation drift; no source/product behavior or historical evidence claim depends on it.

## Generated dictionary/topology evidence

Deterministic reconciliation accepted the worker's local audio dictionary without needing generated row repair and produced a portal with:

- `src/audio` status `COMPLETE`;
- `src/audio` symbol count `82`.

Settled continuity/topology output on the integrated tree reports:

- clean domain set includes `src/audio`;
- `SOURCE_TOPOLOGY_CLEAN_FILE_COUNT=68`;
- `SOURCE_DICTIONARY_COUNT=src/audio:82`;
- `SOURCE_DICTIONARY_TOTAL=2552`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `CONTINUITY_DIFF_INTEGRITY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`.

Concurrent dictionary-prep commits `ea2df09c8247260e47eb03c224dbf9e87b0e1655` and `ad1ee46731fd0413f2b3226cb53f7b528ac4a3ad` were preserved as support records only. They correctly observed the already-landed audio dictionary/portal/living-map state and did not modify behavior authority.

## Canonical host/build evidence

Integration-trigger workflow run `35126397761` at `37ae75801f7ab558c3b6b0cbb2e8a7879790421f` reported:

- canonical host-unit `PASS`, including `audio_playback_test: PASS` and `audio_audsrv_service_test: PASS`;
- existing Transport/RFB/CONFIG/common-clock/application/input/UI regressions remained passing;
- strict long dictionary `PASS`;
- corrected pinned PS2 clean compile `PASS`;
- clean PS2 linked build `PASS`;
- current-source linked reproducibility `PASS`;
- deterministic dictionary reconciliation `PASS` and generated child `6d25e2c0...`.

The linked build explicitly compiled the two new audio objects and linked them with `-laudsrv`. Recorded identity:

- `ELF_PRISTINE_SHA256=001e22e49d3b9c945f3e4a6d13862617fca1d91696745bdca78b7c03e594ac41` on both builds;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=4ba33bd1097800e7a926895ca5ce88e7332e1e22ad256139c4ebc08cc74e05c7`;
- `PT_LOAD_BYTES=423304`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `ISSUE7_RUNTIME_IDENTITY_LINKED=YES`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Settled living-map run `35126537427` and settled Foreman-state-0011 run `35126998196` independently preserve the positive product evidence: host-unit PASS, strict dictionary PASS, pinned PS2 compile PASS, and clean PS2 linked-build/reproducibility PASS. State-0011's ordinary dictionary reconciliation job skipped as expected.

Both settled runs remain overall red only at `project-check`, after documentation, development continuity, clean source topology, local-file coverage, portal sync, and diff-integrity checks all pass.

## Remaining governance/checker mismatch

Work-log contract revision `0005` explicitly declares nine exact grandfather exceptions and adds:

`20260916T122559-0400__diagnostics__global-ci-regression__ci-sentinel.md`

as the ninth already-frozen record.

Current `scripts/work-log-check.py` still identifies itself as the revision-0004 compatibility set and contains only eight grandfathered paths. Therefore the settled project-check reports:

`...20260916T122559-0400__diagnostics__global-ci-regression__ci-sentinel.md: missing required metadata: ENDING_BRANCH_COMMIT, LOG_FORMAT_REVISION, SELF_PAUSED`

with `grandfathered=8` instead of policy revision 0005's nine exact records.

This is a governance/tooling synchronization mismatch, not a PCM/AUDSRV product defect. Foreman did not rewrite the immutable Diagnostics record and did not silently extend the checker independently of its governing policy. The next A002 behavior packet is not blocked by this separate debt.

## Foreman state / next worker packet

Foreman state revision `0011` was produced at commit `784e638d133c84acc32b8b5ff244268137b0298a`.

Current phase:

`A002_AUDIO_WORKER_CLOCK_PACKET_ISSUED`

The fresh W1-W8 packet assigns Reconstruction the substantial next behavior tranche:

- one session-scoped audio worker with explicit stack/priority/buffer/reservoir/timing authority and no guessed production defaults;
- provable allocate/create/start/finish/join/reclaim lifecycle with no free-before-finished path;
- startup reservoir observation via public Transport status/activity without consuming bytes;
- short finite final reservoir handling and producer-done+empty normal completion;
- a publication-aware common-clock audio gate that safely spans initially unarmed -> armed -> audio deadline without audio arming/moving the epoch and without busy spin;
- wrapping rather than rewriting the accepted synchronous playback core;
- finite cancellation/Transport convergence and first-error preservation;
- deterministic resource/reservoir/clock/cancel/reclaim tests;
- no application orchestration, MPEG/video presentation, guessed tuning, receive-poison repair, or hardware claim.

The current common-clock API fact driving this packet is explicit: `pstvnc_media_clock_wait_audio()` returns `PSTVNC_MEDIA_CLOCK_UNARMED` if called before the epoch is armed. A002 permits prefill before arm but forbids early presentation, so Reconstruction must add the narrowest owner-correct finite publication-wait mechanism if no existing seam suffices. `UNARMED` must not be reinterpreted as success.

## Validation / hardware boundary

Validation state revision `0006` and findings revision `0005` remain independent A001 machine/source PASS authority. No newer independent A002 PASS was present in current state during this Foreman round. Positive Foreman CI evidence is not a substitute for Validation.

A001 physical PS2 qualification remains `HARDWARE_PENDING`. No A002 hardware qualification is claimed.

PENDING_LOCAL=independent Validation disposition for integrated A002 authority; governance synchronization of work-log contract revision 0005 into scripts/work-log-check.py; future canonical integration/evidence for W1-W8; later concrete PS2 audio lifecycle/application integration and physical qualification
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; no A002 physical qualification claimed
KNOWN_PRODUCT_DEFECTS_OPENED=NONE_THIS_FOREMAN_ROUND

## Revisions consumed / produced

CONSUMED_RECONSTRUCTION_CONTRACT_REVISION=0005
CONSUMED_WORK_LOG_CONTRACT_REVISION=0005
CONSUMED_GLOBAL_STATE_REVISION=0028
CONSUMED_VALIDATION_STATE_REVISION=0006
CONSUMED_VALIDATION_FINDINGS_REVISION=0005
CONSUMED_FOREMAN_STATE_REVISION=0010
PRODUCED_FOREMAN_STATE_REVISION=0011

## Exact next pickup

RECONSTRUCTION: execute only current Foreman state 0011 W1-W8 as one sustained behavior round. Preserve completed CONFIG/common-clock/Transport AUDIO/synchronous playback authority, use explicit resource/timing fixture authority rather than guessed production defaults, and return exactly one canonical immutable Reconstruction log with `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED`.

FOREMAN: when W1-W8 returns, inspect actual lifecycle/reservoir/common-clock source/tests criterion-by-criterion, perform only canonical integration/build/dictionary/evidence chores, and decide from current A002 audit/dependency authority whether a final bounded application-orchestration packet remains before independent A002 Validation.

VALIDATION: independently consume settled A002 authority when appropriate; do not infer PASS from Foreman evidence and preserve the separate A001 hardware obligation.

GOVERNANCE/CONTINUITY: reconcile `scripts/work-log-check.py` with the already-governing revision-0005 exact nine-record grandfather set without rewriting immutable historical logs.
