# Ledge Reconstruction Shift — A002 CONFIG/profile foundation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T08:33:00-04:00
COMPLETED_AT=2026-09-16T08:56:15-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=91eb25bcd69183e4d0ac2ec216e2f5a630cc9628
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
FOREMAN_GOAL_RESULT=MET
START_TIME_PRECISION=MINUTE_NORMALIZED_TO_SECOND_ZERO

## Objective and authority consumed

Executed only the active `a002-audio-clock` C1-C8 packet from `LEDGE_FOREMAN_STATE.md` revision `0007` as one sustained interactive Reconstruction round.

Consumed current `ledge/h1-all-guns` authority, global work-state revision `0024`, the `20260916T082812-0400` Continuity transition, Validation state `0006` / findings `0005`, reconstruction contract `0005`, work-log contract `0003`, `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision `0001`, pinned H1 CONFIG/media-clock evidence at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, current A001 Transport types, and current source naming/topology policy.

The retained conversation state preserved the round start only to minute precision after context compaction. `STARTED_AT` therefore records that observed minute with seconds normalized to `00` instead of inventing sub-minute precision.

## Reconstruction result

A002 now has a small clean production CONFIG/profile value boundary in `src/config/` rather than an H1-shaped 61-field god object.

`src/config/profile.h` defines narrow immutable value families:

- `pstvnc_config_composition_profile_t` for session identity and RFB/audio/video activation facts;
- the unchanged existing `pstvnc_transport_session_config_t` for all eight A001 Transport values;
- `pstvnc_config_pcm_profile_t` for PCM rate/channels/bits/volume;
- `pstvnc_config_media_clock_profile_t` for common epoch lead plus signed audio/video presentation offsets;
- `pstvnc_config_session_profile_t` only as the decoded aggregate owner.

`src/config/profile.c` implements a pure side-effect-free production decoder. The clean wire representation is intentionally smaller than H1:

- 8-byte big-endian header: version plus entry count;
- fixed 8-byte entries: field ID plus raw `u32` value;
- clean version `1`;
- 19 mandatory fields, each exactly once, accepted in arbitrary entry order;
- decode into an unpublished local candidate, with output published only after complete validation.

The decoder rejects unsupported version, malformed exact length/count, unknown field IDs, duplicates, missing fields, invalid raw enum/boolean representation, Transport structural contradictions, PCM format/volume contradictions, inactive-feature/value contradictions, and other invalid owner-level combinations before publishing state.

All eight existing A001 Transport values remain explicit decoded authority with no application default path:

1. RFB queue capacity;
2. RFB initial credit bytes;
3. RFB credit batch bytes;
4. RFB flush-on-empty;
5. RFB credit-return enable;
6. receiver-thread stack size;
7. receiver-thread priority;
8. maximum DATA payload.

Receiver priority is constrained to the H1/PS2 representable operational range `1..127`; receiver stack must be signed-int representable, at least 256 bytes, and 16-byte aligned; maximum DATA payload remains bounded by the A001 protocol maximum and queue relationship. PCM validation accepts only positive signed-int-representable rate, mono/stereo, 8/16-bit samples, and volume `0..100`. Signed presentation offsets preserve exact `int32_t` bit interpretation rather than applying unsigned policy.

Inactive RFB requires the five RFB queue/credit values to be explicit zero while receiver stack/priority and maximum physical DATA payload remain explicit Transport authority. Inactive PCM requires its PCM-only values and audio presentation offset to be explicit zero. Neutral video activation/common-clock facts are represented only at the composition/offset boundary; no A003 decoder or presentation tuning was imported.

`src/config/text.{c,h}` were adopted behavior-identically into the clean config domain with required file synopses. `src/config/SYMBOLS.md`, `docs/development/source-topology.md`, and `docs/reference/FILE_AND_SERVICE_MAP.md` now reflect source-side config ownership. An intermediate whole-file map edit introduced unrelated historical formatting noise; commit `0cd60129977f4f4fac1c55b8f9e012345139ee5c` restored those unrelated lines before handoff.

## C1-C8 result

- **C1 — MET.** One explicit config-owned immutable session/profile authority exists with narrow composition, Transport, PCM, and media-clock values. No H1 61-field production struct was reproduced.
- **C2 — MET.** All eight A001 Transport values are mandatory decoded fields mapping directly into the unchanged `pstvnc_transport_session_config_t`. The test fixture removes each one individually and requires failure. No fixed application defaults were added.
- **C3 — MET.** Version, malformed length/count, unknown, duplicate, missing, raw-representation, and structural-invalid inputs fail before output publication or runtime side effects.
- **C4 — MET.** PCM activation/rate/channels/bits/volume and signed common-clock offsets are represented and validated. A003 MPEG/presentation tuning remains absent.
- **C5 — MET.** Transport, future PCM ownership, and future media-clock ownership have separate narrow immutable subprofiles rather than depending on the whole wire representation.
- **C6 — MET.** `tests/unit/config_profile_test.c` covers complete valid decode, wrong version, malformed input, unknown/duplicate/missing fields, every missing A001 Transport field, invalid Transport relationships, raw representation rejection, invalid PCM format/volume, activation contradictions, exact signed-offset extremes, RFB-off inert policy, PCM-off inert policy, and no partial output publication on failure.
- **C7 — MET.** A001 runtime was not changed. `src/app.c` remains on the existing fail-closed `pstvnc_app_run_with_transport_config(NULL)` entry; no live CONFIG handshake, Transport receive ownership, RFB lifecycle, fatal abort, or quiesce behavior was modified.
- **C8 — MET.** No PCM worker, AUDSRV lifecycle, shared-clock waiting/arming runtime, MPEG/video reconstruction, next-session receive-poison repair, or physical-hardware claim was introduced.

## Commits / files

Round commits after starting authority `91eb25bcd69183e4d0ac2ec216e2f5a630cc9628`:

- `57abc3d85e7ef09e042bec08912d76683ab046d5` — `reconstruct(a002): define bounded session profile authority`
- `d9fd5c0fef51fa6ea07a8ff029b5299caa095133` — `reconstruct(a002): decode and validate session profile`
- `1d4bec9fa9178623754b89ee21ca7adcfedff027` — `test(a002): cover bounded session profile decode`
- `f2992a472fd685c0a25591d1b8cba3018cee1ab3` — `docs(config): adopt text helper header as clean source`
- `73ee94367466ff6c6a7790f7aec901c6086af571` — `docs(config): adopt text helpers as clean source`
- `5f53802469e3297ff12f7be25cfe7cd5247ce366` — `docs(config): add clean config symbol authority`
- `8c7a6c5790a4a2d125a37ffb187960d8bbe1e5d0` — `docs(a002): adopt config as clean source domain`
- `0882101bd19c92f45b06afbaba39e02f48b2f8e4` — `docs(a002): map clean config profile ownership`
- `cff9ca4582d8f02305b97726fdf338e0716f59e5` — `reconstruct(a002): reject unrepresentable raw profile values`
- `3f4d2480b825739b8cf425ddec5e98fd489ad3cf` — `test(a002): prove every Transport field is mandatory`
- `def52043730f3ef3c8d56d67b7291abbcb130930` — `test(a002): cover raw wire representation rejection`
- `0cd60129977f4f4fac1c55b8f9e012345139ee5c` — `docs(a002): remove unrelated map diff noise`

Touched packet files:

- `src/config/profile.h`
- `src/config/profile.c`
- `src/config/text.h`
- `src/config/text.c`
- `src/config/SYMBOLS.md`
- `tests/unit/config_profile_test.c`
- `docs/development/source-topology.md`
- `docs/reference/FILE_AND_SERVICE_MAP.md`

No `src/app.c`, `src/transport/`, `src/rfb/`, runtime audio/clock implementation, or `src/video/` behavior file was changed.

## Checks actually executed

A direct strict host compile/execution of the committed pure decoder fixture was run with the repository host warning profile (`-O2 -std=c99 -Wall -Wextra -Werror -pedantic`) against `src/config/profile.c` and `tests/unit/config_profile_test.c`; result:

    config_profile_test: PASS

The final executed local source/test bytes were checked against current GitHub blob identities before closure. The source call-chain review also confirmed the new decoder has no live production callsite and `pstvnc_app_run()` remains fail-closed.

This is **not** a claim that the new fixture is already part of the canonical `make -C tests unit` graph. `tests/Makefile` did not yet contain `config_profile_test` at handoff, as expected under Foreman 0007's ownership split.

## Explicit pending integration / evidence

Routine canonical registration and generated integration chores remain Foreman-owned under the packet:

- register `config_profile_test` in the canonical host suite;
- reconcile any generated dictionary portal/checker/topology registration required by the adopted config domain;
- run canonical project checks and long/complete/strict dictionary validation;
- run any clean PS2 compile/link/reproducibility evidence required once Foreman integrates this source into the canonical build graph;
- decide the next dependency-ordered A002 packet.

No A002 physical hardware evidence was attempted or claimed. A001's separately recorded machine/source PASS remains intact and its physical status remains `HARDWARE_PENDING`.

## Exact next pickup

Foreman should independently inspect this source/test/log, perform only the Foreman-owned canonical registration/integration/evidence chores above, then issue the next bounded A002 packet. The next Reconstruction packet should remain dependency-ordered inside A002; this worker did not begin live CONFIG negotiation, PCM/AUDSRV runtime, shared-clock wait/arming runtime, or A003 behavior.
