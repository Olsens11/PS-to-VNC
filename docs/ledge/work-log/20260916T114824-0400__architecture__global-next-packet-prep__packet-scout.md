# Next-Packet / H1 Archaeology Scout — A002 PCM consumer

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T11:48:24-04:00
COMPLETED_AT=2026-09-16T11:48:57-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6b3cb3ebab4c5ec84d3e514090c35350eab23370
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Prepared one dependency step ahead of the just-completed A002 Transport AUDIO-channel Reconstruction packet. This record is planning support only:

`NON_AUTHORITATIVE_PENDING_FOREMAN`

Authority/evidence inspected:

- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0009, whose exact next Foreman pickup names the PCM/AUDSRV consumer as the next dependency after a coherent AUDIO seam;
- `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision 0001;
- `docs/ledge/work-log/README.md` revision 0004;
- completed interactive Reconstruction handoff commit `6b3cb3ebab4c5ec84d3e514090c35350eab23370` and its exact A1-A8 result;
- forensic H1 `experiments/media-harness-h1/h1_audio_runtime.c` and `.h`;
- current clean `src/config/profile.h`;
- current clean `src/transport/bridge.h`.

The active worker packet has now returned `FOREMAN_GOAL_RESULT=MET`; Foreman still owns direct review, canonical AUDIO registration/link/dictionary/topology evidence, criterion disposition, and any fresh packet authority.

## Conditional next coherent responsibility

If Foreman independently accepts the settled AUDIO seam, the next coherent A002 behavior responsibility is a **PCM/AUDSRV audio consumer/lifecycle packet** built against the new narrow Transport AUDIO bridge and the already-reconstructed immutable PCM profile/common media clock.

Natural packet center: own PCM service/session behavior in a clean audio component without taking application orchestration, MPEG/presentation, or final startup-reservoir/media-clock composition beyond what can be kept as a narrow dependency.

### Historical H1 call chain that proves semantics

`pstvnc_h1_audio_load_modules_once()`:

1. load resident `rom0:LIBSD` once;
2. execute embedded `AUDSRV_irx` once;
3. retain resident-service lifetime for the ELF rather than per session.

`pstvnc_h1_audio_start()`:

1. consume the session CONFIG/profile and require PCM mode;
2. initialize runtime ownership;
3. allocate PCM chunk buffer;
4. allocate aligned worker stack;
5. create/start one session audio worker;
6. on partial-start failure, reclaim only resources whose worker cannot still own them.

`h1_audio_thread()`:

1. `audsrv_init()`;
2. `audsrv_set_format(freq,bits,channels)`;
3. `audsrv_set_volume(volume)`;
4. satisfy startup reservoir policy before presentation synchronization;
5. wait on shared media-clock audio offset;
6. repeatedly read bounded bytes from Transport AUDIO;
7. preserve exact playback ordering `audsrv_wait_audio(bytes)` then `audsrv_play_audio(bytes)`;
8. record/advance consumption only after successful playback submission;
9. treat finite producer completion + empty AUDIO queue as normal worker completion;
10. on stop/error/normal completion, call `audsrv_stop_audio()` for the current stream;
11. do **not** call `audsrv_quit()` at session teardown;
12. publish worker finished and exit.

`pstvnc_h1_audio_shutdown()`:

1. request stop;
2. prove worker dormancy/completion before deleting thread;
3. only then free stack and PCM buffer;
4. leave resident AUDSRV/LIBSD service intact for the next session.

## Current clean owners / bridges

- `src/config/profile.h` already owns `pstvnc_config_pcm_profile_t { rate_hz, channels, bits_per_sample, volume_percent }` and `pstvnc_config_media_clock_profile_t` with signed audio presentation offset. Do not duplicate these facts in an audio god-config.
- `src/transport/bridge.h` now exposes the exact logical source the future audio owner should consume: `pstvnc_transport_audio_read_available`, `pstvnc_transport_audio_status`, `pstvnc_transport_audio_activity_snapshot`, and `pstvnc_transport_audio_wait_activity`. Audio must not touch the physical descriptor or Transport internals.
- the common media clock is already a separate clean timing owner. Audio may consume its deadline/wait seam; it must not own or arm the shared epoch.
- a new clean `src/audio/` component is the natural owner for AUDSRV interaction, PCM session resources, worker lifecycle, format/volume application, and stream stop. Cross-directory Transport/clock consumption should be grouped behind one narrow audio bridge/process rather than leaking Transport internals throughout the component.

## Behavior that must survive

- resident ELF service/module lifetime: LIBSD/AUDSRV module loading is one-time; session teardown stops/mutes stream only;
- no session `audsrv_quit()`;
- exact `audsrv_wait_audio(bytes)` -> `audsrv_play_audio(bytes)` ordering;
- bytes are considered played/consumed only after successful submission, not merely after dequeue;
- finite producer-done + drained queue is normal completion;
- session worker stack/buffer are never freed while a worker can still touch them;
- first-error semantics survive cleanup failures rather than cleanup overwriting the original cause;
- audio consumes only the Transport-owned logical AUDIO channel;
- PCM format/volume come from the already-decoded immutable profile.

## Experimental H1 mechanisms not to promote blindly

- the exact 1000-us x 3000 shutdown polling loop is defensive mechanism, not product semantics; the requirement is deterministic worker quiescence before reclamation;
- H1 thread priority/stack/chunk sizes and idle-delay cadence were laboratory tuning fields. Do not invent production values or enlarge the public CONFIG wire vocabulary merely to copy them;
- H1 writes E2xx-style audio diagnostic stages directly into Transport. A002 classifies those writes DIAGNOSTIC_ONLY; clean audio correctness must not depend on Transport diagnostics;
- H1 couples reservoir polling and clock polling through one `audio_idle_delay_us`; clean event/timing waits need not preserve that coupling;
- H1's direct access to `runtime->transport->end_received` and other internals must be replaced by the narrow current Transport bridge outcomes/status.

## Known-defect / caution accounting

Do not reinterpret H1's session teardown comment as optional: the audit explicitly classifies resident AUDSRV/LIBSD lifetime as required behavior because per-session `audsrv_quit()` dismantles synchronization/callback/playback/SPU-DMA state without reliable later reconstruction.

Do not free a live worker stack/buffer merely because stop was requested or a bounded wait expired. A timeout may surface failure, but reclamation still requires proof the worker cannot execute.

Do not treat Transport `WOULD_BLOCK`, stop/failure, and normal `EXHAUSTED` as interchangeable. The newly reconstructed bridge deliberately distinguishes them.

## Likely packet split

Recommended core packet, subject to Foreman authority:

1. create the clean audio component and narrow immutable session/runtime boundary;
2. define an injectable AUDSRV/service seam so host tests can prove ordering/lifecycle without PS2 hardware;
3. consume PCM format/volume from `pstvnc_config_pcm_profile_t`;
4. consume bounded bytes exclusively through the Transport AUDIO bridge;
5. implement exact wait-before-play ordering and only commit playback accounting after successful submit;
6. implement normal finite exhaustion vs stop vs terminal failure;
7. implement session worker allocation/start/stop/completion/reclaim with no-free-while-live and first-error preservation;
8. implement resident-service load/init ownership such that session teardown never calls `audsrv_quit()`.

A sensible **stretch only after those semantics are complete** is a pure startup-reservoir policy helper or a narrow audio/common-clock wait integration if it can be proved without pulling application orchestration into audio. The reservoir and media-clock callsite may instead be cleaner as the following small A002 composition packet; Foreman should decide based on the settled audio boundary.

## Behavior-specific evidence likely needed

Host-testable with injected Transport/AUDSRV/thread/service seams:

- format and volume applied exactly from immutable PCM profile;
- partial Transport reads and `WOULD_BLOCK` do not submit phantom bytes;
- each nonzero chunk calls wait before play with identical byte count;
- wait failure prevents play and preserves first error;
- play failure is surfaced and bytes are not falsely recorded as successfully submitted;
- producer-done + empty/`EXHAUSTED` retires normally;
- Transport stop/failure remains abnormal/distinct from normal exhaustion;
- session stop calls stream stop but never `audsrv_quit()`;
- module/service one-time semantics survive repeated session start/stop;
- allocation/CreateThread/StartThread/AUDSRV init/format/volume/wait/play failures each converge without freeing worker-owned memory early;
- shutdown refuses/defers reclaim until worker completion is proven;
- diagnostics can be absent without changing correctness.

PS2-build / hardware evidence:

- real LIBSD + embedded AUDSRV module linkage/load;
- real EE thread stack alignment/priority viability once qualified values exist;
- actual `audsrv_wait_audio`/`audsrv_play_audio` ABI and sustained PCM playback;
- repeated-session stop/restart proving resident service remains reusable;
- timing quality and A/V synchronization once presentation can arm the common epoch.

Hardware is therefore not a blocker for reconstructing most of the component contract, but repeated-session service reuse remains a physical qualification obligation.

## Dependency / ownership hazards

1. **Transport dequeue vs playback accounting.** Current Transport returns credit when bytes are dequeued. The audit separately requires audio to record consumption only after successful playback submission. Foreman should decide whether "record consumption" is audio-owned accounting/diagnostics rather than Transport credit; do not silently redesign Transport credit semantics in the PCM packet.
2. **Service init scope.** H1 loads modules once but calls `audsrv_init()` in each audio worker while intentionally avoiding `audsrv_quit()`. Clean reconstruction should preserve proven behavior without inventing an unproven per-session full service reset.
3. **Worker tuning authority.** Current production CONFIG does not carry H1's audio chunk/stack/priority fields. The packet needs a narrow qualified internal audio runtime profile or explicit owner values; absence of final tuning numbers must not be solved with magic defaults.
4. **Clock readiness.** Audio may prefill before epoch arm but cannot present before its signed deadline. If the Foreman keeps clock gating out of the core PCM packet, the audio boundary must leave a clean point to add that gate before first playback without later redesign.
5. **Application ownership.** Application should compose already-owned services; it should not become the owner of AUDSRV calls, worker internals, or Transport byte reads.

## Open Foreman decisions

- whether the next packet should include the startup-reservoir + common-clock first-presentation gate or keep that as a subsequent A002 composition packet;
- the clean authority for audio chunk size / worker stack / worker priority while qualification values are not yet promoted into the 19-field public CONFIG profile;
- whether resident `audsrv_init()` semantics belong in a tiny ELF-lifetime audio service object with session stream start/stop beneath it, or a simpler equivalent that still proves repeated-session reuse and never calls `audsrv_quit()`;
- how to represent successful-playback accounting without coupling audio correctness back into Transport diagnostic state.

These are design choices for Foreman/Reconstruction authority, not facts this scout resolves.

## Exact consumption point

FOREMAN: consume this dossier only after directly reviewing the `6b3cb3e...` AUDIO handoff and completing/validating the canonical registration, link, dictionary/topology and evidence chores. If A1-A8 remain coherent, use the traced H1/current-source evidence above to issue the next bounded PCM/AUDSRV packet. Revalidate all named current-source blobs/HEAD first; this log is prep evidence, never packet authority.

PENDING_LOCAL=Foreman direct AUDIO handoff review and canonical integration/evidence; independent Validation; final clean audio worker resource/tuning authority
HARDWARE_PENDING=A001 current-DUT physical qualification remains pending; future A002 repeated-session AUDSRV reuse and real playback/timing qualification remain hardware evidence
