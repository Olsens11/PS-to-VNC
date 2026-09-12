# CP2P / MPEG calibration experiment history

## Purpose

This file is the durable chronological history of the current H1 concurrent-media / MPEG-calibration experimental drive.

It exists so that the working history is not scattered across chat transcripts, handoff files, automation snapshots, Git commits, CI runs, hardware records, and later summaries.

This is **not** a timeless specification. It is a log of the voyage.

The governing rule is:

> **Each entry is assumed to be an accurate representation of the project's understanding, evidence, uncertainty, decisions, and intended next action at the time it was written.**

A later entry may supersede an earlier belief without making the earlier entry false as history. If an earlier entry says the project is lost and a later entry says it returned safely to port, both entries belong. Together they describe what actually happened.

Accordingly:

- do not rewrite older entries merely to make them agree with later knowledge;
- do not erase failures because they were later diagnosed;
- do not silently replace an earlier design label with a later source-accurate name;
- do not retroactively describe host-only evidence as hardware evidence;
- do not collapse temporally distinct proof boundaries into one flat current-state summary;
- append later corrections, supersessions, and new evidence in chronological order;
- preserve the original exact artifacts whenever possible and use this file as the obvious chronological index into them.

For current implementation status, use a current-state handoff. For history, use this file and the exact evidence it indexes.

---

# 1. Scope of this experimental drive

The broader PS-to-VNC project predates this file. This history begins where the current experimental drive became one coherent line of investigation:

1. prove useful live MPEG on the PS2;
2. preserve the already-qualified RFB / Issue-39 interaction path;
3. understand why separate concurrent media paths could destabilize the machine;
4. move concurrent media onto the single PSTV multiplexed transport;
5. prove visible RFB, then visible RFB + PCM;
6. add a native PS2 MPEG-region calibration and explicit presentation ownership;
7. compose RFB + PCM + MPEG without returning to competing sockets/readers;
8. make runtime calibration drive the exact MPEG source and presentation region;
9. qualify the eventual all-guns system with exact provenance and physical PS2 evidence.

The experiment is intentionally developed under `experiments/media-harness-h1/` and related experiment-local files. `src/` remains the clean architectural/reference model and is not to be modified as part of this experiment unless David explicitly decides otherwise later.

---

# 2. Engineering principles already in force before this drive

These principles shaped the work before the automation and continued through the later live sessions:

- evidence before speculation;
- exact branch / commit / tree / artifact identity;
- preserve failures and their evidence;
- distinguish host-tested, PS2-cross-compiled, deployed, physically observed, and hardware-qualified states;
- root-cause failures instead of masking them;
- instrument before broad rewrites;
- treat a freeze or stall as a symptom, not a diagnosis;
- identify the lowest layer still proven to make progress;
- prefer controlled bisection to shotgun optimization;
- do not add silent watchdog/reconnect behavior during root-cause debugging merely to hide failures;
- preserve qualified machinery rather than recreating it casually;
- keep PS2 mechanisms bounded and explicit;
- do not turn a successful experiment into an unnecessarily large framework;
- do not reset, clean, overwrite, or discard unknown work;
- do not treat a static debug screen as proof that EE execution is still progressing.

Useful freeze-analysis layers included Pi/VNC service, Ethernet/IOP, TCP/socket, PSTV mux, RFB framing/parser, rectangle/decode, memory corruption, GS/DMA, synchronization/threading, input, EE control flow, and true system/hardware lockup.

---

# PHASE A — MEDIA FEASIBILITY AND THE CONCURRENCY PROBLEM

## 3. September 6–7, 2026 — separate-stream behavior exposes the problem

Before the unified H1 media composition was mature, RFB/video-like traffic and PCM audio could operate through separate competing streams.

A particularly important operator observation became a design driver:

- with YouTube playing but minimized, PCM audio could run for a very long time without the characteristic shudder/stall;
- when the YouTube window was maximized and generated heavy visible framebuffer activity, the PS2 would eventually freeze.

At the time, this suggested that "audio is too much" was an incomplete explanation. The failure correlated with concurrent visual/network/service pressure and therefore pointed toward contention, ownership, scheduling, queueing, or shared-resource progress.

David explicitly rejected blindly increasing queues as a default solution. A queue large enough for real hardware behavior is acceptable; using ever-larger queues to conceal an architectural defect is not.

This observation became one of the strongest reasons to converge on:

- **one physical PSTV TCP connection**;
- **one PS2-side receive owner**;
- multiple **logical channels** inside that connection;
- explicit queue/credit/service policy rather than competing socket owners.

This principle remains non-negotiable without new evidence.

A later comprehensive handoff preserved this motivation explicitly in the September 12 CP2P handoff.

---

## 4. September 7, 2026 — MPEG EXP3 turns video into a credible media plane

The `experiment/media-stream-exp3` campaign moved MPEG-2 from "can the PS2 decode this at all?" to a credible live accelerated-media path.

The experiment used hardware bisection and source provenance rather than assuming the modern library path was correct. Important steps included graph/IPU/link-only probes, one-picture smoke tests, comparison against mature SMS MPEG assembly code, multi-picture playback, EOF characterization, network fixture replay, deep-buffer experiments, resolution changes, and live streaming.

By the end of that campaign, the project had evidence that useful live MPEG-2 was feasible on the PS2. The synthesis explicitly did **not** yet claim combined RFB/audio/MPEG behavior.

Historical handoff authority included:

- branch `experiment/media-stream-exp3`;
- final handoff head `e0fd8ef3066e42b3cfc32e20c38e5260bf141114`;
- last hardware-result authority beneath documentation `a002c3a5c74ba9111b081704768a1b0eafb66994`;
- `experiments/media-stream-exp3/O3_MEDIA_STREAM_OPTIMIZATION_SYNTHESIS_2026-09-07.md`;
- `experiments/media-stream-exp3/SUCCESSOR_HANDOFF_2026-09-07.md`.

The same handoff identified `experiment/audio-transport-mux` at `15a879095cf6bc24df10268b6516bbb39c7a83ce` as the transport lineage to reuse rather than inventing a separate MPEG transport.

---

# PHASE B — H1 MUXED RFB AND RFB+PCM BASELINES

## 5. September 9, 2026 — CP2N hardware-qualifies muxed visible RFB + real Issue-39 interaction

CP2N established the full existing remote-interaction composition over the muxed RFB logical channel on a real PS2.

The qualified composition included:

- local VNC source on the Pi;
- Pi raw RFB bridge;
- PSTV logical channel 1;
- one physical Pi↔PS2 PSTV TCP connection;
- PS2 credit-controlled RFB queue;
- the existing through-Issue-39 RFB parser and CPU framebuffer;
- graphics presentation;
- controller/mouse semantics;
- local-controller routing;
- local UI / OSK foreground ownership;
- keyboard tap building and RFB key serialization;
- live remote pointer and keyboard interaction;
- deterministic finite RFB quiescence machinery for qualification.

Recorded hardware result:

- `CP2N_HARDWARE_QUALIFIED=YES`;
- `CURRENT_HARDWARE_AUTHORITY=CP2N_VISIBLE_RFB_REAL_ISSUE39_INTERACTION`;
- test id `H1-RFB-CP2N-HW1-20260909T221954Z`.

The operator reported a physical PASS for desktop visibility, pointer movement, left/right click, drag, OSK entry, OSK navigation/typing, modifier/backspace/enter/tab behavior, clean exit, release quarantine, and resumed mouse control.

The cursor was noted as somewhat sluggish/laggy. That was preserved as a performance observation, not treated as a functional qualification failure.

At that moment CP2N explicitly did **not** qualify RFB+AUDIO, RFB+MPEG, simultaneous RFB+AUDIO+MPEG, hybrid composition, or production transport consolidation.

Historical CP2N candidate identity recorded in the runbook:

- build head `7e047a9cb9dfef9283f66a80ccd865664f075c11`;
- ELF SHA256 `f55b2620903ea0ba2a64c53c4196b1e1959a6bb13be1c8a3ae6bf52a08df1000`;
- ELF bytes `3041300`;
- PT_LOAD SHA256 `544117b8bdf4c0cacefb7d6b2e8fa01eba90809a369cfd8bfdfc0abc36722914`;
- PT_LOAD bytes `491028`.

Those values are historical CP2N evidence and are not implicitly transferred to later candidates.

---

## 6. September 9–10, 2026 — CP2O extends the baseline toward visible RFB + PCM

After CP2N, the next narrow coexistence checkpoint became visible RFB + the already-developed PCM path.

The important architectural result was that composition did **not** require gratuitously turning every path into a new independent foreground/thread architecture:

- RFB could remain the foreground loop;
- interaction service could continue at safe RFB service boundaries;
- PCM could use its existing concurrent runtime/worker;
- the one PSTV physical connection / one PS2 receive owner architecture could remain intact.

This later became the composition model used when planning CP2P: preserve foreground RFB, preserve PCM concurrency, add MPEG as another controlled media worker/lifecycle rather than rewriting the entire application around concurrency.

A later handoff recorded CP2O provenance as:

- source commit `ae1bccd1bac0c3cd8abe8393e0c59b7212ea70d8`;
- branch `experiment/h1-cp2o-rfb-pcm-prep`;
- ELF SHA256 `2a68367046a2b99fc575be2ce194949495010b644cfeef3c95b6cf1713f97e4b`.

A later cleanup-only commit changed branch history without changing semantic tree content; that historical artifact was explicitly preserved rather than rewritten.

---

# PHASE C — CALIBRATION IDEA, PROTOTYPE, AND LOCKED UX BEFORE AUTOMATION

## 7. September 10–11, 2026 — the MPEG presentation region becomes first-class runtime state

The project needed a clean way to decide where MPEG should appear and, critically, where the Pi should capture from the desktop.

The calibration concept separated three geometries.

### Base rectangle

The base rectangle is the actual MPEG source/capture region and the PS2 MPEG presentation region. It is not merely a cosmetic destination rectangle.

### Inner matte

The inner matte is presentation-only. It changes the black inset/visible MPEG presentation inside the base rectangle and does not change Pi capture geometry.

### Outer matte

The outer matte expands MPEG's visual-ownership footprint and therefore drives the RFB suppression perimeter. It does not change Pi capture geometry.

This distinction prevented the profile/config layer and the runtime calibration layer from becoming competing geometry authorities.

---

## 8. Pi/PyQt calibration mock establishes the intended UX

Before the PS2-native implementation, a Pi/PyQt prototype was used as a behavioral/visual reference.

The accepted visual language included:

- 704×462 working canvas;
- default base region 352×224 at x=176, y=119;
- 16-pixel-aligned size changes for MPEG-friendly dimensions;
- alternating burnt-orange `#CC5500` and dark-teal `#01796F` selector segments;
- no aqua selector;
- no black halo around the selector itself;
- uncluttered edit mode;
- idle hint outside the rectangle, right-aligned where possible;
- preferred one-line hint `TRIANGLE: CONTROLS    X: CONFIRM`;
- thin black text halo rather than a translucent idle-text panel;
- controls and review screens allowed to use the accepted full-rectangle translucent panel style.

The state model was:

- EDIT;
- CONTROLS;
- REVIEW;
- ACCEPTED / exit path.

A held X was forbidden from entering REVIEW and immediately confirming it. REVIEW acceptance required X release and a new press.

Release quarantine was required so calibration controls could not become unintended desktop input when local ownership ended.

Accepted controller semantics:

- plain D-pad resizes the base region in macroblock-compatible increments;
- R1 + D-pad moves the rectangle at one-pixel precision;
- R2 adjusts the inner matte;
- L2 adjusts the outer matte;
- Start resets EDIT geometry to defaults;
- Triangle opens/closes controls/help;
- Circle backs out/cancels according to state;
- X enters REVIEW and later confirms after release/re-press.

The apparently opposite inner/outer matte directional semantics were intentional because the visible model was one continuous black matte and the controls represented movement of perceived boundaries, not merely increasing/decreasing arbitrary numbers.

---

## 9. Calibration ownership/lifecycle intent before automation

Before implementation, several behavior decisions were already important:

- physical controller acquisition continues while calibration owns semantic controller input;
- calibration entry occurs above ordinary Select→OSK routing;
- mouse interpretation is suspended while calibration owns foreground;
- visible RFB updates freeze during calibration;
- the RFB protocol/session stays alive;
- no new framebuffer-update requests are issued after the appropriate safe boundary;
- local calibration UI is drawn over the frozen desktop;
- cancel returns to full RFB using one fresh nonincremental/full update;
- accept does not instantly give MPEG visual ownership;
- accepted geometry begins MPEG preparation;
- RFB remains protected while MPEG is not yet physically visible;
- the first valid/physically presented MPEG frame is the visual-ownership boundary;
- once MPEG owns the region, RFB is forbidden from repainting from the outer suppression edge inward.

A temporary development entry gesture became held `START+SELECT`; later direct work refined the hold duration to 0.75 seconds for the candidate path.

---

# PHASE D — SEPTEMBER 11 AUTOMATION

## 10. Automation identity and operating rule

Hourly automation:

- title `Prepare MPEG calibration ELF`;
- automation id `6aa39154c0348191802bc591aee5d746`.

The automation repeatedly advanced one narrow tranche, wrote repository evidence, and updated `MPEG_CALIBRATION_CONTINUITY.md` with what was known **at that hour**.

Those snapshots are preserved exactly in Git history and indexed by:

`experiments/media-harness-h1/MPEG_CALIBRATION_AUTOMATION_HISTORY.md`

The exact snapshot can be recovered with:

```sh
git show <commit>:experiments/media-harness-h1/MPEG_CALIBRATION_CONTINUITY.md
```

The current-path `MPEG_CALIBRATION_CONTINUITY.md` has been restored byte-for-byte to the final automation snapshot rather than being reused as a later flat state document.

---

## 11. Automation progression — contemporaneous snapshots

### 12:48 UTC — portable calibration core

`210cfd0b0e79985a262d0d06e9dba726a3fbd94c`

`docs(h1): add MPEG calibration continuity handoff`

Portable calibration state/geometry core established and host-tested.

### 13:47 UTC — adapter integration

`d6adaae4e59ad68b1ca4bdb3dbfb0f5223b02e4a`

`docs(h1): hand off calibration adapter integration`

Calibration advanced from portable state toward normalized controller/action adaptation.

### 14:47 UTC — foreground ownership bridge

`22a29413e57b8bdf48d212d31b946e0f8b783a7f`

`docs(h1): hand off calibration foreground ownership bridge`

Foreground ownership was introduced so calibration could suspend desktop interpretation while retaining physical controller acquisition and release quarantine.

### 15:49 UTC — RFB gate tranche

`efb24d51abc80583d9d3bd4fc7fb4da2d75e0816`

`docs(h1): hand off calibration RFB gate tranche`

Explicit RFB request/presentation gating entered the design instead of treating calibration as only a local overlay.

### 16:48 UTC — render-plan tranche

`69dfd7e416a2ea0355da521516b8d568344f870c`

`docs(h1): hand off calibration render-plan tranche`

The prototype UX was represented through experiment-local render-plan/runtime structure without making calibration itself the permanent MPEG compositor.

### 17:48 UTC — runtime tranche

`a91f5b3fa0e4df25c6e967409445dde5e0058077`

`docs(h1): hand off calibration runtime tranche`

Portable state, ownership, render, and RFB-related pieces were composed into a coordinator-facing runtime boundary.

### 18:49 UTC — host-contract evidence

`961fcae90045357b76655d548b03589304640315`

`docs(h1): hand off calibration host-contract evidence`

The then-current proof boundary was preserved without assuming the next integration would work.

### 19:46 UTC — RFB scheduler tranche

`c2f855e88e338e9e7096c695f72999511d8a9915`

`docs(h1): hand off calibration RFB scheduler tranche`

Explicit request scheduling behavior was added around safe complete-message boundaries and restoration-refresh obligations.

### 20:50 UTC — RFB flow-policy tranche

`dd6c5c8cc9aa9aae24976e8ef233aadbd487a9ce`

`docs(h1): hand off calibration RFB flow-policy tranche`

Calibration restrictions were represented through a generic flow-policy seam instead of being welded into the RFB parser/socket implementation.

### 21:47 UTC — interaction-binding tranche, with unresolved failure preserved

`d5db0e324fd229332645d87be1ed0bbc160801b6`

`docs(h1): hand off calibration interaction-binding tranche`

The coordinator-facing binding existed, but its host contract was still failing. The failure was preserved as part of the live state rather than retroactively described as success.

### 22:52 UTC — green calibration binding evidence

`df11959c748b0abf019d74e6e11d06a789765f05`

`docs(h1): hand off green calibration binding evidence`

The earlier host failure had been diagnosed as the ordinary Ubuntu host build lacking PS2SDK `<libpad.h>` through the real production header graph. The chosen fix was a host-only type shim, not weakening or replacing the production input interface.

Strict host contracts and pinned-PS2 regression compilation were green.

The automation deliberately stopped before live coordinator insertion. At this moment the history said, correctly for that moment, that live coordinator integration, native PS2 rendering, accepted-geometry handoff to MPEG, permanent compositor suppression, and a true calibration-enabled MPEG candidate remained incomplete.

---

# PHASE E — LIVE DIRECT SESSIONS AFTER THE AUTOMATION

## 12. Writer handoff from automation to live work

The hourly automation was intentionally paused/disabled so there would be only one writer.

The intended relay was recorded as:

```text
autonomous → handoff → direct work → handoff → autonomous
```

The same automation could be resumed later; a replacement automation was not required.

A direct-session handoff preserved that `MPEG_CALIBRATION_CONTINUITY.md` could lag the live repository because the conversation had advanced beyond the last autonomous snapshot.

---

## 13. September 12 00:51 UTC onward — calibration ownership enters the live composition

The direct live work started immediately after the automation boundary.

The Git range `df11959...` → `d3a6ba7...` contains 22 commits. Selected exact milestones follow; the full commit range remains the authoritative source of detail.

### Live coordinator ownership flow

`c9b8781caa843697986f02fbbaabc6f1def79a26`

`feat(h1): wire live MPEG calibration ownership flow`

This crossed the automation's deliberate stop boundary by inserting calibration ownership behavior into the live experiment-local H1 composition.

`73379ab08321e8d69df1e1d514e03d268a6ec55c`

`test(h1): recognize CP2O calibration flow-policy seam`

The source/checking machinery was reconciled with the new live flow-policy ownership.

---

## 14. Native PS2 calibration rendering

A short sequence implemented and tested the native raster/presenter path:

- `5e6fd9f2d900151e630ecc6d5116d48a7d84961e` — `feat(h1): add native MPEG calibration raster interface`;
- `c4da0bafdb717d62dec7af961572c036faee9d42` — `feat(h1): implement native MPEG calibration CT16 raster`;
- `21d3505c406bf3ad6723c7d1fadeba03e8fb6d7f` — `test(h1): cover native MPEG calibration raster`;
- `8498b3c97b23df332b247d2f3b7b0dac0b0370ba` — `feat(h1): wrap coordinator with native calibration presenter`;
- `e3519a82c9d6b59644cc7784f04cca37f392ad30` — `test(h1): add calibration raster to strict host suite`;
- `368d934459b19c9f3e0c63c9d37a5745b4042668` — `feat(h1): link native calibration presenter into CP2O`;
- `eec15182cdca3a4b7b2f0dec5f55d54912a0ab22` — `test(h1): verify native calibration presenter wiring`;
- `72480420434544bc968043594e00d84fe3bf4c51` — `test(h1): verify native calibration presentation wiring`.

At `724804...`, the handoff recorded that the live calibration ownership/RFB-flow path plus native calibration renderer host-tested, PS2-cross-compiled, linked, and fingerprinted inside the real visible-RFB + Issue-39 interaction + PCM composition.

That was still CP2O: `video_mode` remained OFF. The handoff explicitly warned not to call it the final RFB+PCM+MPEG candidate.

Historical CP2O build evidence at that point included:

- workflow run `34664137998`;
- artifact `10288368109`;
- ELF `PS2VNC-H1-CP2O-VisibleRFBInteractionPCM.ELF`;
- ELF SHA256 `6c998a98643661fc527801ff793b319db5edaa15d5a0b541da72486eac67c538`;
- PT_LOAD SHA256 `280d7d94c0382cd1dfcb46b05fca66eff9914fc43ea0a231b5d3f8b2f552b567`.

Those values represent that specific CP2O build boundary and were not a CP2P hardware qualification.

---

## 15. Presentation ownership and generation safety

`5b058f16651c3ff6878e5d9a247aef8664468995`

`h1: add MPEG presentation ownership contract`

This introduced the explicit ownership model that later supported `WAIT_FIRST_FRAME`, active MPEG ownership, retirement, and generation-specific safety instead of a loose global "video active" boolean.

Two commits then ensured the older CP2N target remained inert with respect to new calibration hooks:

- `8d6074dbaf330d18fe97ca1dc7fbeb2b029ee353` — `fix(h1): keep CP2N calibration hooks inert`;
- `919eec3d50853f50eaeb61c19509bd9e2c75904c` — `fix(h1): link inert calibration seam in CP2N`.

This preserved earlier qualified target semantics while extending the experiment.

---

## 16. Immutable CP2P MPEG start handoff

The next sequence created the in-memory generation/geometry handoff:

- `8df00e6253bf2dbc1ee3fa138f125382468b9e03` — `h1: add CP2P MPEG start handoff contract`;
- `698929bfb08c4f03687016f7c660b8aacb1abaf9` — `h1: implement CP2P MPEG start handoff`;
- `f962df1fa6ecbf3fc43aac2d65521a2d38db19ba` — `test(h1): verify CP2P MPEG start handoff`;
- `5e398833d7b7c3327c5441d642e3ffd92dcfaf0e` — `test(h1): run CP2P MPEG start handoff contract`.

This contract froze a generation-specific accepted configuration instead of sharing mutable calibration state across lifecycle boundaries.

At this stage `h1_mpeg_start_handoff` was an **in-memory ownership/geometry contract**, not yet a PSTV wire serializer. That distinction became important in later source archaeology.

---

## 17. Shared compositor recovery and first-physical-frame boundary

`fd9f6b809e235f4de501bf7ae60eb77915794332`

`h1: recover calibrated shared compositor`

`a9425bed1887c84b03c460070ca5b829e779570b`

`ci(h1): compile dormant CP2P compositor`

The recovered shared compositor gave the experiment a place to combine remote desktop, MPEG presentation/matte, and local UI while preserving a single physical GS presentation boundary.

The desired ordering was preserved conceptually as:

1. remote desktop;
2. suppression/matte footprint;
3. MPEG base rectangle;
4. inner matte;
5. local overlay;
6. one GS synchronization/presentation path;
7. physical flip.

Ownership was not considered transferred merely because decode succeeded. The significant boundary was physical presentation through `gsKit_sync_flip()`.

---

## 18. Generated calibrated CP2P MPEG runtime

`c1c2ee578d7006677fba98db0f94985a945872df`

`h1: generate calibrated CP2P MPEG runtime`

The generated CP2P runtime kept the mature decoder/IPU/timing machinery while delegating presentation to the shared compositor and binding the session to an immutable start contract/generation.

Important source-derived properties later confirmed:

- start contract copied by value into the session;
- decoder output remains in EE memory;
- shared compositor owns physical presentation;
- compositor returns after `gsKit_sync_flip()`;
- only then can matching `WAIT_FIRST_FRAME` ownership promote;
- release before first frame aborts the exact generation;
- stale generation state cannot promote a newer presentation owner.

---

## 19. Combined CP2P RFB ownership policy

`7d5a2de8066078311cd1236477f0fffa5f755c79`

`test(h1): compose CP2P RFB ownership policy`

This composed calibration RFB gating and MPEG presentation ownership over the existing RFB request scheduler.

The useful invariant was that a one-shot full-refresh obligation is not consumed while another owner still keeps RFB frozen. Coincident restoration obligations collapse into one full/nonincremental refresh.

This solved the accept→first-frame gap cleanly:

- calibration may leave its foreground state after acceptance;
- MPEG presentation owner can remain `WAIT_FIRST_FRAME`;
- `WAIT_FIRST_FRAME` continues protecting the owned footprint;
- the first physically presented matching frame promotes MPEG;
- retirement later permits a single restoration refresh.

`d3a6ba722695ab6f0f78f034bcf14b6daed7a422`

`ci(h1): fix CP2P RFB flow host includes`

Tree: `50dd7ebd1232aa4bcb36edecd252217ebc9c336a`.

This became the important qualified-source/provenance anchor used by the next live sessions.

---

# PHASE F — LIVE SOURCE ARCHAEOLOGY REFINES THE CP2P TARGET

## 20. Handoff state after `d3a6...`

The live handoffs converged on the first meaningful CP2P qualification target:

- RFB begins immediately;
- PCM begins immediately;
- MPEG capability is armed but Pi MPEG production remains dormant;
- user performs calibration;
- acceptance creates an immutable generation-specific start contract;
- PS2 sends that start contract to Pi through the existing PSTV transport;
- Pi installs the generation-specific suppression/capture state before MPEG production;
- Pi captures the exact accepted base rectangle;
- MPEG joins the already-running RFB+PCM system;
- first physical MPEG frame promotes ownership;
- RFB remains excluded from the MPEG-owned footprint;
- retirement/failure restores RFB safely with exactly one full refresh.

David deliberately chose the first CP2P test to be **all guns** rather than first creating an RFB+MPEG/no-audio candidate. RFB+PCM was to be treated as the baseline onto which MPEG joins.

The test question became:

> Can the already-qualified RFB+audio system remain healthy when MPEG joins it under realistic active load?

---

## 21. Historical label later corrected by source archaeology

One handoff referred to the intended active profile as:

`rfb-incremental-live-pcm`

That statement is preserved as what the handoff called the intended profile at the time.

Later source inspection established that no literal profile by that name existed in `h1_profiles.py` at `d3a6...`. The actual source profiles included names such as `H1_RFB_ONLY`, and the CP2O/CP2P activation architecture was structural through concrete mode/config fields rather than a C-side comparison against that literal label.

This is a deliberate example of temporal history:

- the earlier label remains part of the historical handoff;
- the later source-derived correction is appended here;
- the earlier handoff is not rewritten to pretend it always used the source-accurate terminology.

---

## 22. Source archaeology identifies the remaining exact gaps

The post-`d3a6` source review established several important facts.

### Existing transport authority

The PSTV transport already had:

- one sole PS2 receiver thread;
- one send semaphore;
- direction-local sequence ownership;
- fixed PSTV framing;
- logical MPEG2 channel 4;
- the internal framed-send seam `pstvnc_h1_transport_send_frame_internal(...)`.

Therefore a CP2P START request did not justify another raw socket writer.

### No START wire codec existed yet

Although the in-memory start handoff existed, there was no implemented 44-byte START serializer/parser at `d3a6...`.

The later agreed wire design was 11 big-endian 32-bit words:

1. version;
2. session id;
3. generation;
4. base x;
5. base y;
6. base width;
7. base height;
8. suppression x;
9. suppression y;
10. suppression width;
11. suppression height.

Inner matte remained local PS2 presentation state rather than Pi capture state.

### Exact `accepted` edge drop point

The calibration core already emitted a one-observation `accepted` effect. Adapter and foreground layers preserved it.

The first loss occurred in `pstvnc_h1_mpeg_calibration_runtime_service_controller()` because its public result retained consume/freeze/visible information but did not carry the real accepted edge outward.

Therefore the correct change was propagation, not a second detector/event framework.

### CP2O activation gate was structural

The live CP2O gate permitted visible RFB plus optional PCM while keeping MPEG OFF. It normalized RFB-specific fields and delegated ordinary validation rather than using a magic profile string.

The eventual CP2P gate should therefore deliberately open the exact supported visible-RFB + PCM + MPEG combination without weakening unrelated validation.

### Pi all-guns path still incomplete

No current qualified-tree Pi triple-media runner/START receiver/suppression lifecycle had yet been completed. Pi generation-scoped suppression and deferred MPEG producer startup remained open implementation work.

---

# PHASE G — CURRENT LIVE SESSION: ACCEPTED EDGE + START WIRE TRANCHE

## 23. Isolated implementation branch from exact `d3a6...`

A new isolated branch was created from the exact qualified-source anchor:

`experiment/h1-cp2p-start-wire-accepted`

Base:

`d3a6ba722695ab6f0f78f034bcf14b6daed7a422`

The qualified branch itself was not moved or rewritten.

The intended tranche was deliberately narrow:

1. propagate the existing accepted edge outward;
2. define a strict START wire codec;
3. send it only through the existing PSTV serialized writer;
4. retain CP2O compatibility;
5. stop before Pi producer/suppression/all-guns activation.

---

## 24. Accepted-edge propagation implemented

Experiment-local runtime/binding code was changed so that:

- `pstvnc_h1_mpeg_calibration_runtime_result_t` carries the existing `accepted` edge;
- runtime copies that edge directly from foreground calibration effects;
- a result-rich binding API exposes controller-consumption plus acceptance information;
- the previous consume-only binding API remains as a compatibility wrapper;
- the existing committed-region accessor remains the geometry authority rather than duplicating state.

A focused host test proves that acceptance is true on the intended observation and false afterward while committed geometry remains available.

Implementation/evidence head before later documentation repair:

`71874a91f24bc706858f8c2ec739f80f98b4ad89`.

The `d3a6...` → `71874...` diff remained confined to `experiments/media-harness-h1/mpeg_presentation_calibration/`.

---

## 25. MPEG START wire contract made executable

New experiment-local START wire code defined version 1 as exactly 44 bytes / 11 BE32 words.

The payload carries:

- protocol version;
- session identity;
- generation;
- exact base/capture rectangle;
- exact RFB-suppression rectangle.

The base rectangle and suppression rectangle remain distinct authorities:

- base = exact MPEG capture/presentation geometry;
- suppression = RFB exclusion footprint;
- inner matte = PS2-local presentation state.

A narrow transport sender emits the serialized START only through the existing PSTV framed-send path as `DATA` on MPEG2 logical channel 4. It does not expose the raw socket and does not create another send owner.

Focused host contracts were added for:

- one-shot accepted-edge propagation;
- exact 44-byte START encoding/validation;
- exact use of the serialized PSTV transport seam.

The strict host suite produced green markers including:

- `MPEG_CALIBRATION_ACCEPT_EDGE_HOST_TEST=PASS`;
- `H1_MPEG_START_WIRE_HOST_TEST=PASS`;
- `H1_MPEG_START_TRANSPORT_HOST_TEST=PASS`.

The pinned PS2 CP2O regression build also remained green. This was regression evidence only; the START modules were not yet linked into a live CP2P PS2 application path and no new CP2P hardware qualification was claimed.

Historical regression artifact from this tranche:

- ELF SHA256 `0f30ae41ac7cf77ddf7f0a9d8e2c40639b6cfd03d232363cca3799e79f6969ad`;
- PT_LOAD SHA256 `8737d4c3735f165d667b63d3a1c3f174ebd0db96ba17d402b55cabdd5e34d08c`.

---

# PHASE H — DOCUMENTARY CONTINUITY CORRECTION

## 26. September 12 — continuity file was flattened, objection raised, history restored

After the accepted-edge/START-wire tranche, a later assistant handoff mistakenly replaced the working copy of `MPEG_CALIBRATION_CONTINUITY.md` with a flat current-state summary.

David objected because the automation's progressive continuity entries were the only contemporaneous view of what the autonomous work knew, believed, had proved, and still did not understand at each stage.

That objection was correct.

Investigation showed that the historical automation snapshots had **not** been destroyed: all 11 exact versions remained addressable in Git history.

Corrective action then separated the two documentary purposes:

- `MPEG_CALIBRATION_CONTINUITY.md` was restored byte-for-byte to the automation's final snapshot at `df11959c748b0abf019d74e6e11d06a789765f05`;
- the later flattened handoff was preserved separately as `MPEG_CALIBRATION_CURRENT_STATE.md`;
- `MPEG_CALIBRATION_AUTOMATION_HISTORY.md` was added as an append-only index of the 11 exact automation snapshots;
- the repository gained an explicit preservation rule that current-state summaries must not replace historical continuity.

The corrective branch commit was:

`423c14d32ad99babf3bbee924c49f3e2444ad77e`

`docs(h1): preserve automation continuity timeline`

This event is itself part of the history and is not hidden from the log.

---

# PHASE I — MASTER HISTORY ESTABLISHED

## 27. September 12 — one obvious chronological home for the whole experimental drive

David then requested that the working history of this **specific experimental drive** be gathered into one obvious place, beginning with the work and reasoning that led into the automation, continuing through every automation tranche, then through the live sessions and the current accepted-edge/START-wire work.

He clarified the intended historical philosophy with a captain's-log analogy:

> If a captain writes that he is lost and cannot find his way, and a later entry says he has returned safely to port, there is no untruth. The entries describe different moments in the voyage.

That principle is now the governing rule of this file.

This document therefore does not attempt to make history temporally coherent by rewriting the past. It attempts to make the **sequence of changing truths and knowledge states** coherent and recoverable.

---

# 28. Primary evidence map

The most important supporting records for this history are:

### Automation progression

`experiments/media-harness-h1/MPEG_CALIBRATION_CONTINUITY.md`

Exact historical versions at the commits indexed in:

`experiments/media-harness-h1/MPEG_CALIBRATION_AUTOMATION_HISTORY.md`

### Current-state synthesis

`experiments/media-harness-h1/MPEG_CALIBRATION_CURRENT_STATE.md`

This is intentionally **not** the historical log.

### Later direct-session handoffs

Conversation/library handoffs dated September 12, including the CP2P comprehensive session handoff and the transition handoffs that recorded live branch state, source archaeology, calibration semantics, all-guns intent, and remaining gaps.

### Hardware / experiment authorities

- EXP3 MPEG optimization and hardware-result documents under `experiments/media-stream-exp3/`;
- CP2N candidate/runbook/result records under `experiments/media-harness-h1/`;
- CP2O candidate/build records under `experiments/media-harness-h1/`;
- exact Git commit history for `df11959...` → `d3a6ba7...`;
- exact Git history for the isolated `experiment/h1-cp2p-start-wire-accepted` continuation.

---

# 29. Current historical endpoint at creation of this file

At the time this master history was created:

### Proven / implemented in the current experimental line

- live MPEG feasibility on PS2 from EXP3;
- muxed visible RFB + real Issue-39 interaction hardware qualification in CP2N;
- visible RFB + PCM composition baseline in CP2O lineage;
- calibration geometry/state/action/foreground/RFB-flow host contracts;
- native PS2 calibration raster/presenter integrated into CP2O build lineage;
- generation-scoped MPEG presentation ownership;
- immutable in-memory MPEG start handoff;
- shared compositor preparation;
- generated calibrated CP2P MPEG runtime;
- composed calibration + MPEG RFB ownership/one-refresh policy;
- outward propagation of the real one-shot calibration `accepted` edge;
- exact 44-byte START wire codec;
- START transmission through the existing PSTV serialized writer;
- green strict-host contracts and green pinned CP2O regression build for the latest narrow tranche.

### Not yet claimed complete at this historical endpoint

- final CP2P all-guns application coordinator wiring from accepted edge through `prepare_start()` and live START send;
- exact-generation abort/unwind from that live send seam;
- Pi-side START receiver integrated into the current qualified tree;
- generation-scoped Pi RFB suppression implementation;
- Pi deferred MPEG producer startup from accepted base geometry;
- live triple-media Pi runner in the current qualified lineage;
- final CP2P config activation gate;
- concurrent MPEG worker/lifetime orchestration in the final live PS2 path;
- a built/deployed/hardware-qualified RFB+PCM+MPEG candidate;
- physical proof that high-change RFB + PCM + MPEG remains stable under the all-guns workload.

These statements are the current endpoint **only as of this entry**. Future entries are expected to supersede parts of this section without rewriting it.

---

# 30. Append-only rule for future sessions

Future sessions working on this experimental drive should append a new dated entry to this file when they materially change any of the following:

- what is believed about the failure mechanism;
- transport/ownership architecture;
- calibration semantics;
- generation/start lifecycle;
- source or capture geometry authority;
- RFB suppression behavior;
- proof boundary;
- candidate identity;
- host/cross-compile/hardware evidence;
- a failed experiment that changes the next move;
- a correction to an earlier assumption;
- an architectural decision that future refactoring must understand.

Do not edit an old entry simply because a later experiment proved it incomplete.

If an old entry turns out to be wrong in a source-factual sense, preserve it and append the later discovery that corrected it.

That is the point of the log.

---

# PHASE J — SEPTEMBER 12 CHECKPOINT BEFORE REAL CP2P TARGET LINKAGE

## 31. Twelve-item implementation scoreboard after accepted→START coordinator work

Immediately before beginning the next real-CP2P-target tranche, the project reconciled the remaining work against the current branch rather than against older handoff language alone. This entry records the checkpoint as understood at that moment; later entries may supersede it without rewriting it.

1. **PS2 Accept → START session coordinator — DONE.** The authoritative live path is `accepted → committed calibration → prepare_start() → fresh generation N → WAIT_FIRST_FRAME → START(session_id, N, geometry)`. The old experimental macro/global bridge was removed rather than left as a competing path.
2. **PS2 START-send failure unwind — DONE.** If generation N is prepared but START cannot be sent, that exact generation is aborted. It cannot remain stranded in `WAIT_FIRST_FRAME` or later acquire MPEG ownership. The RFB restoration obligation is preserved.
3. **Prove accepted → START transition — DONE.** The integrated coordinator path plus failure/no-send cases are host-tested; the strict host suite passed; the CP2P pieces compiled with the pinned PS2 toolchain; CP2O regression remained green; and bridge-removal checks passed. This is software proof, not hardware qualification.
4. **Link a real CP2P PS2 target — PARTIAL.** Major CP2P pieces compile individually, including interaction/session coordination, presentation ownership, RFB flow, shared compositor, generated MPEG runtime, and START machinery. No final real CP2P application target/ELF yet runs the all-guns composition. This is the next concrete item.
5. **Pi START receive + validation — OPEN.** The protocol is explicit, but the current historical Pi mux path still accepts PS2 `DATA` only on RFB and does not yet implement PS2→Pi `DATA`/MPEG2 START decoding and validation.
6. **Pi generation-scoped RFB suppression — OPEN.** Required semantics are settled—generation-scoped, installed before MPEG production, RFB live outside the footprint, retired with the generation—but the exact suppression mechanism remains to be chosen from source evidence.
7. **Exact calibrated geometry end-to-end — PARTIAL.** PS2-side exact `X,Y,W,H` authority is implemented and remains distinct from the expanded suppression rectangle. Pi-side use of those exact values as the actual capture source remains unimplemented.
8. **Make MPEG production START-driven — OPEN.** Intended Pi lifecycle remains `producer dormant → valid START → install suppression → configure exact capture → launch producer → permit MPEG bytes`; current Pi implementation does not yet provide that lifecycle.
9. **Integrate the concurrent PS2 MPEG worker — PARTIAL.** CP2P video runtime/compositor and presentation machinery exist and compile, but the live all-guns target in which RFB + PCM + MPEG participate together over the one-transport/one-receive-owner architecture is not complete. This remains distinct from item 4: item 4 constructs/links the real application target; item 9 makes the MPEG worker participate correctly in that target.
10. **Open the exact CP2P CONFIG gate — OPEN.** The final target needs a narrow structural gate for the intended visible-RFB + PCM + MPEG combination. No imaginary profile-name shortcut is to be introduced.
11. **Complete presentation / retirement / recalibration lifecycle — PARTIAL, PS2 SIDE MOSTLY DONE.** The settled PS2 lifecycle preserves immutable generations, first-frame physical-presentation ownership, safe RFB restoration, and fresh START on reconfirmation. Remaining work is chiefly Pi-side exact-generation producer stop/drain, suppression retirement, and stale-generation exclusion. A mature normal stop/return path is not automatically a prerequisite for the first all-guns hardware run unless the first candidate implements that path.
12. **Seal the immutable all-guns hardware candidate — OPEN.** After the runnable system exists, record exact PS2 source commit, Pi source commit, CONFIG, pinned toolchain evidence, ELF SHA256/size, PT_LOAD fingerprint, Pi runtime/deployment identity, and then perform the physical test.

Checkpoint scoreboard:

- **DONE:** #1, #2, #3.
- **PARTIAL:** #4, #7, #9, #11.
- **OPEN:** #5, #6, #8, #10, #12.

The immediate next action from this checkpoint is item **#4**: inspect the existing CP2N/CP2O/CP2P build composition and create the smallest real CP2P PS2 application target that links the already-proven components without inventing new runtime behavior or claiming all-guns hardware readiness.

---
# PHASE K — SEPTEMBER 12 CP2P APPLICATION TARGET LINKAGE

## 32. CP2P is deliberately evolved from the CP2O application baseline

At the start of item #4, David confirmed the intended lineage: the real CP2P PS2 target should be updated out of the CP2O baseline rather than reconstructed as a new application main.

Source inspection supported that choice. CP2O already represented the cumulative visible-RFB + optional-PCM application composition, with one PSTV transport owner, the through-Issue-39 interaction path, the experiment-local calibration presenter/policy integration, and MPEG still OFF. The new CP2P target therefore became a direct build descendant of that exact CP2O composition.

The intended #4 proof boundary was deliberately narrower than all-guns execution:

- produce a distinct real CP2P PS2 ELF;
- inherit the CP2O main/application and transport composition rather than duplicate it;
- link the already-proven CP2P presentation-owner, start-handoff, combined RFB-flow, START-wire, and START-transport machinery;
- make the recovered shared compositor the single public graphics owner;
- keep MPEG startup dormant;
- do not yet link/activate the generated CP2P video runtime or concurrent MPEG worker;
- do not yet open the final visible-RFB + PCM + MPEG CONFIG gate;
- make no hardware-qualification claim.

This preserves the scoreboard distinction between item #4 (construct/link the real application target), item #9 (make the MPEG worker participate in it), and item #10 (open the exact all-guns CONFIG gate).

### First link attempt — useful failure preserved

Initial implementation commit:

`9f25c3bc0c814548d8a3c03021a21bc7c746a114`

`build(h1): derive CP2P application target from CP2O`

The CP2O makefile's target identity/build variables were changed to overridable defaults so a descendant could inherit the exact CP2O composition without copying its large application/object recipe. A new CP2P application-link makefile inherited CP2O and initially added the recovered shared compositor as an extra object alongside the inherited `ps2_graphics39.o`.

GitHub Actions run:

`34698487101`

The host-contract job passed. The pinned PS2 build job failed at link time with duplicate definitions of:

- `pstvnc_ps2_graphics_init`;
- `pstvnc_ps2_graphics_present`;
- `pstvnc_ps2_graphics_shutdown`.

This failure corrected the composition model. `h1_cumulative39_graphics.c` is not an independent second graphics module: it is a wrapper/drop-in evolution of the through-Issue-39 graphics owner. It directly includes the base graphics implementation under private `*_base_c39` names and then exports the one public graphics API plus the MPEG-compositor functions. Therefore the correct CP2P linkage is to replace the inherited graphics object with the shared-compositor wrapper, not to link both.

Before the CP2P duplicate-owner failure, the same run rebuilt CP2O and reproduced its exact regression SHA256:

`0f30ae41ac7cf77ddf7f0a9d8e2c40639b6cfd03d232363cca3799e79f6969ad`

That independently showed that making the CP2O target variables inheritable had not perturbed the CP2O binary.

### Corrected single-owner CP2P target

Corrective implementation commit:

`fc5145a46d2a3f167be9c132e94ddcf9ea154075`

`fix(h1): make CP2P compositor sole graphics owner`

The CP2P descendant retained the CP2O application composition and the canonical dormant `h1_video_runtime.o`, linked the CP2P ownership/START/RFB-flow objects, and overrode only the inherited `ps2_graphics39.o` recipe so that `h1_cumulative39_graphics.c` became the one public graphics owner.

The generated CP2P runtime symbol `pstvnc_h1_video_run_cp2p_session` was intentionally required to be absent from this #4 ELF. That absence is part of the proof boundary, not an omission: live concurrent MPEG runtime/worker integration remains item #9.

Pinned verification run:

`34698655541`

Head under test:

`fc5145a46d2a3f167be9c132e94ddcf9ea154075`

Pinned PS2DEV image:

`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Both jobs passed:

- host-contracts `103566409592` — SUCCESS;
- ps2-build `103566409510` — SUCCESS.

Important emitted proof markers included:

- `H1_CP2P_APPLICATION_LINK=PASS`;
- `H1_CP2P_BASELINE=CP2O`;
- `H1_CP2P_GRAPHICS_OWNER=SHARED_COMPOSITOR`;
- `H1_CP2P_MPEG_STARTUP=DORMANT`;
- `H1_CP2P_SINGLE_GRAPHICS_OWNER=PASS`;
- `H1_CP2P_DORMANT_LINK_SYMBOLS=PASS`;
- CP2O RFB/PCM source/object checks PASS;
- RFB mux source/object/seam checks PASS;
- CP2O regression SHA256 remained exactly `0f30ae41ac7cf77ddf7f0a9d8e2c40639b6cfd03d232363cca3799e79f6969ad`.

The first distinct CP2P application-link ELF from this proof boundary was:

`build/experiments/media-harness-h1-cp2p-application-link/ps2/PS2VNC-H1-CP2P-ApplicationLink.ELF`

Identity:

- ELF SHA256 `d889c81db99afa94a18e5d039610eef3a58c83b9e388aaa7567fc997cb8ed91d`;
- ELF bytes `3236720`;
- PT_LOAD segments `1`;
- PT_LOAD SHA256 `d91ce2f4532a4e514129b87a8c2ed0f7173b97366fd210f980fa87012d815417`;
- PT_LOAD bytes `515220`;
- `TESTKIT_PT_LOAD_FINGERPRINT=PASS`.

Uploaded workflow artifact:

- name `h1-cp2p-application-link-unqualified-elf`;
- artifact id `10299134524`;
- artifact ZIP SHA256 `33a20ec20a0d30864a0b4546151e8e1abeea3d6b783ff8b21f88711154f6bc1c`.

One provenance imperfection is preserved explicitly: the generated `cp2p-application-identity.txt` printed `SOURCE_COMMIT=unknown` because `GITHUB_SHA` was not passed through into the Docker container. The run itself is nevertheless tied unambiguously to `fc5145a46d2a3f167be9c132e94ddcf9ea154075`: the Actions run head and checkout log identify that exact commit. The self-description defect is therefore not promoted into a false claim that the artifact file independently embeds its source commit.

### Item #4 proof-boundary update

At this point scoreboard item **#4 — Link a real CP2P PS2 target — is DONE at the software build/link proof boundary.**

What this proves:

- CP2P has a real, distinct PS2 application target descended directly from CP2O;
- the CP2O visible-RFB + optional-PCM application/transport composition remains the baseline;
- the existing CP2P START/ownership/RFB-flow machinery is present in that ELF;
- the shared compositor is the sole public PS2 graphics owner;
- the target builds under the pinned PS2 toolchain with exact ELF/PT_LOAD identity;
- CP2O regression identity remains unchanged.

What this does **not** prove:

- live MPEG startup or a concurrent MPEG worker in the application target;
- Pi START receive/validation;
- Pi generation-scoped RFB suppression;
- Pi deferred exact-geometry capture/producer startup;
- the final CP2P CONFIG gate;
- deployment or physical PS2 behavior;
- all-guns stability or hardware qualification.

Accordingly, #4 moves from **PARTIAL → DONE** while #5, #6, #8, #10, and #12 remain OPEN and #7, #9, and #11 remain PARTIAL at this point in the voyage.

---

# PHASE L — SEPTEMBER 12 SIBLING-LINE CONVERGENCE AND FINAL ITEM #4 SEAL

## 33. Sibling lifecycle/session and application-link lines are converged before advancing

Repository reconstruction after the first item-#4 application-link proof exposed an important lineage fact: items #1–#3/session-lifecycle work and the first #4 target proof lived on sibling branches and had never been proven together in one tree.

The lifecycle line `experiment/h1-cp2p-recalibration-lifecycle` had established the fresh-generation rule: START+SELECT begins a new calibration transaction; an active/waiting generation is retired first; a **new full RFB restoration frame** must present before calibration freezes; cancel creates no generation; confirm creates a fresh higher generation; and stale retired generations cannot reacquire ownership. Important milestones included `d2af6c4918b6d9af1123d8704ca97829e6e73271`, `4d307d022a7595da28059d31dd0fa496a10e1f31`, and final sibling head `4d56f487357e1c73f6c025ad923c9966e69ab622`.

That line also established the real CP2P session coordinator as owner of interaction/calibration state, immutable MPEG generation/start handoff, combined RFB-flow policy, recalibration state, the existing PSTV transport/session identity, one-shot accepted→START, and exact-generation send-failure abort.

The application-link line `experiment/h1-cp2p-start-wire-accepted` separately proved the CP2O-derived CP2P target and one-public-graphics-owner linkage. Rather than advance to Pi #5 with only a narrative assertion that these lines composed, the new branch `experiment/h1-cp2p-integrated-pre-pi` converged them with true two-parent commit `b45b00b443d79af8096ad7131798eda807ce1b4f` (merged PR #44). Neither historical branch was rewritten.

The explicit CP2P target was then wired to instantiate and use the real session coordinator while keeping the MPEG worker deliberately dormant. CP2O's historical main was restored byte-for-byte; CP2P uses its own clearly named CP2O-derived main. Thus the earlier #4 proof remains true for what it tested, while this later convergence closes the missing forward-integration boundary.

---

## 34. Item #4 closes on one integrated tree with pinned provenance

Final integrated item-#4 source proof commit:

`a14afa66108a033bd9bc5da6362385c547515d4c`

Final proof run: `34701548058` using pinned image `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`.

Both jobs passed: host-contracts `103574087813` and ps2-build `103574087729`.

Historical CP2O provenance was refined rather than flattened. The exact `0f30ae41...` artifact belongs to source commit `a9037fd12115d48113423394703dfb3e313631b7`, originally proven by run `34690484266`. Rebuilding that exact commit at the original `/repo` source path reproduced:

- ELF SHA256 `0f30ae41ac7cf77ddf7f0a9d8e2c40639b6cfd03d232363cca3799e79f6969ad`;
- PT_LOAD SHA256 `8737d4c3735f165d667b63d3a1c3f174ebd0db96ba17d402b55cabdd5e34d08c`;
- PT_LOAD bytes `507412`;
- `H1_CP2O_ANCHOR_IDENTITY=PASS`.

This established that `f200949...` was a later branch checkpoint, not the source of that historical artifact, and that the complete ELF hash is DWARF/source-path sensitive.

The forward-integrated tree intentionally contains newer shared coordinator machinery, so historical identity and forward regression were proven separately. Current-tree CP2O passed its RFB/PCM source/object and mux-seam contracts with `H1_CP2O_INTEGRATED_RFB_PCM_REGRESSION=PASS`; its proof-boundary ELF SHA256 was `c00573ce36c1b4c2e1c678bfc054ee61e217365a82168caa9140e9a4a160d7fe`.

The integrated CP2P ELF proved in one linked tree: CP2O-derived visible RFB + optional PCM baseline; real CP2P session coordinator; accepted calibration→immutable generation→START; recalibration/full-RFB-restoration lifecycle; START wire through the existing serialized PSTV writer; combined CP2P RFB policy; one public graphics owner; and the existing one-transport/one-PS2-receive-owner mux seam. The generated concurrent MPEG worker remains intentionally dormant.

Final item-#4 CP2P identity:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `328981b54a396bca78e3a3fb41d592a5bc20ec3674d1d91b7fd6288031972ec3`;
- ELF bytes `3269860`;
- PT_LOAD SHA256 `3df932c9b8752803f38926b21238ab000fd4dc9d255495b9df2be414ea177bbe`;
- PT_LOAD bytes `518804`;
- artifact `h1-cp2p-integrated-pre-pi-unqualified-elf`, id `10299788893`;
- artifact ZIP SHA256 `609c221f3861763c2c78b2e527bb2f670aee20ead86efe0274cce39e09e1e43d`.

Proof refinement also established that `check_rfb_pcm_interaction.py` is CP2O-specific because it requires `h1_main.o` to reference the bare interaction coordinator. Applying it to CP2P was therefore invalid: CP2P correctly delegates through the session coordinator. The final proof uses the CP2O checker only for CP2O and proves CP2P through its own source contract, inherited mux seam, and linked-symbol assertions. Production behavior was not changed to satisfy an inapplicable checker.

At this entry scoreboard item **#4 — Link a real CP2P PS2 target — is complete for the intended pre-Pi software boundary.** This does not claim Pi START receive/validation, Pi suppression/capture activation, a running PS2 MPEG worker, final all-guns CONFIG activation, deployment, physical behavior, or hardware qualification. The next incomplete implementation item is **#5 — Pi START receive + validation**.

---
# PHASE M — SEPTEMBER 12 PI START RECEIVE + VALIDATION

## 35. Item #5 — Pi START receive + validation closes

With the integrated item-#4 tree sealed, work moved to the first missing Pi-side control-plane endpoint rather than waking MPEG prematurely.

A new non-destructive continuation branch, `experiment/h1-cp2p-pi-start-receive-validation`, was created from integrated pre-Pi head `e61b0ce50d7e3e72fc97d6f382247eb1f35cf5de`.

The implementation deliberately preserves the established one-transport/one-reader architecture. The existing Pi RFB session adapter already consumes only logical channel 1 on the sole `H1Session.reader()` call chain and returns all other frames to the base dispatcher. The new CP2P START receiver therefore wraps that already-active receive owner after RFB attachment and consumes only PS2->Pi PSTV `DATA` on logical MPEG2 channel 4. It creates no second PS2-facing socket, transport reader, frame-sequence owner, or receive thread.

The Pi decoder mirrors the existing PS2 wire authority: exactly 44 bytes / eleven big-endian 32-bit words containing version, session id, generation, exact base rectangle, and expanded suppression rectangle. Validation now requires:

- wire version 1 and exact 44-byte length;
- active session-id equality;
- nonzero generation;
- geometry representable in the PS2 signed-int domain;
- base width/height at least 16 and 16-pixel aligned;
- positive suppression size;
- suppression fully containing the exact base rectangle;
- both base and suppression rectangles lying inside the active Pi desktop described by existing Pi-only profile metadata.

No new CONFIG field or imaginary profile string was introduced.

A successful START becomes one immutable prepared Pi request. Only one generation may remain prepared at a time. Replacing it before exact release is rejected, and once a generation has been prepared, that generation or an older one is stale. The exact-generation release seam exists for later producer/lifecycle work but performs no activation itself.

The CP2P Pi runner is a narrow descendant of the existing CP2O visible-RFB/optional-PCM runner. At this checkpoint a valid START only records `mpeg_start_prepared.json` and emits `H1_CP2P_MPEG_START_PREPARED=...`. It does **not** install RFB source suppression, alter capture geometry, launch ffmpeg, emit MPEG bytes, run the PS2 concurrent MPEG worker, or open the final all-guns CONFIG gate.

Durable implementation files introduced in this tranche:

- `experiments/media-harness-h1/h1_cp2p_start_receiver.py`;
- `experiments/media-harness-h1/h1_cp2p_start_receiver_test.py`;
- `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`;
- `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md`.

Important commits leading to the tested checkpoint:

- `57373a83fa6b128353eb8efe54152b8abcccd3ef` — Pi START receive/validation seam;
- `2a02513bfce0d1f275e19bc71ea0951980fd5221` — CP2P Pi runner integration;
- `5caadf746fb5cf1eaa4caab7d75024f9db471352` — focused host tests;
- tested/documented source head `c37c22c64d159d8f1814a3462ac6dbfa2e0b4557`.

GitHub Actions proof run `34702828032`, job `103577491937`, completed SUCCESS. Evidence included:

- Python compile proof PASS;
- 14 focused START receive/validation tests PASS;
- `H1_CP2P_PI_START_RECEIVE_HOST_TEST=PASS`;
- existing `h1_rfb_pi_bridge_test.py` PASS;
- existing `h1_rfb_session_adapter_test.py` PASS;
- existing cumulative RFB bridge runner test PASS;
- `H1_CP2P_PI_START_RFB_REGRESSION=PASS`;
- inert CP2P runner `--validate-only` PASS;
- `H1_CP2P_PI_START_RUNNER_VALIDATE=PASS`;
- static proof that the START module creates no receive thread, socket, or producer process;
- `H1_CP2P_PI_START_BOUNDARY=PASS`.

The proof boundary remains software-only. It does not claim Pi suppression/capture activation, START-driven MPEG production, live concurrent PS2 MPEG consumption, all-guns CONFIG activation, deployment, or hardware qualification.

At this entry scoreboard item **#5 — Pi START receive + validation — OPEN -> DONE**. The chosen next implementation order is **#9 — integrate the concurrent PS2 MPEG worker**, before allowing later Pi work to make a valid START wake MPEG production.

---

# PHASE N — SEPTEMBER 12 GENERATION-BOUND PS2 MPEG WORKER

## 36. Item #9 — concurrent PS2 MPEG worker integration closes at the pre-activation boundary

After item #5 established the real Pi START receive/validation endpoint, work intentionally returned to the PS2 before allowing a valid START to wake a Pi MPEG producer. A new non-destructive continuation branch, `experiment/h1-cp2p-ps2-mpeg-worker`, was created exactly from sealed item-#5 head `c25907162d6f533c14877582196f4b1fdc832d1f`.

The principal lifecycle constraint discovered during implementation was that the existing transport MPEG queue is session-scoped: `pstvnc_h1_transport_mpeg_read()` can block until MPEG bytes arrive or the whole PSTV session receives `MEDIA_END`. A generation-bound worker therefore could not be retired safely by setting a thread flag alone. The smallest architecture-preserving solution was a consumer-local cancellable MPEG read. The historical `pstvnc_h1_transport_mpeg_read()` API remains as a compatibility wrapper; CP2P uses `pstvnc_h1_transport_mpeg_read_cancellable()` with a worker-owned stop predicate. No second socket, transport reader, queue, sequence owner, or receive thread was introduced.

Item #9 adds `h1_cp2p_mpeg_worker.c/.h`. The worker owns only EE decode-thread, libmpeg, and concrete MPEG-pixel lifetime. The CP2P session coordinator remains the sole authority for presentation-generation state. The worker arms only an exact nonzero generation already in `WAIT_FIRST_FRAME`, stores that generation's immutable START contract, and rejects mismatched or re-entrant arm requests.

Accepted-confirm ordering is now:

`prepare fresh generation -> arm exact worker -> send START`

If worker arm fails, START is not sent and the coordinator aborts the exact prepared generation. If START transmission fails after arm, the coordinator first requires the exact worker to stop/drain/withdraw its concrete MPEG state and only then aborts that generation. If worker clear fails, ownership is not falsely retired beneath a potentially live worker.

The generated compositor-aware CP2P video runtime now accepts the worker stop predicate. Its MPEG feed and prefill waits are cancellable. Explicit worker cancellation exits without independently aborting or stopping presentation ownership; natural pre-first-frame decoder failure retains the existing handoff-abort behavior. First MPEG ownership is still promoted only after decoded presentation reaches the shared compositor and physical `gsKit_sync_flip()` completes.

Recalibration retains one ownership path: worker/pixels clear first, then the existing session/recalibration coordinator retires the exact generation and begins the already-proven full-RFB restoration transaction. Whole-session coordinator shutdown uses the same ordering for either `WAIT_FIRST_FRAME` or `MPEG_OWNED` state.

The CP2P application target now links `h1_cp2p_mpeg_worker.o` and the generated compositor-aware `h1_video_runtime_cp2p.o`. The inherited standalone `h1_video_runtime.o` may still be compiled by the inherited make dependency graph but is filtered from the final CP2P linker command. The linked image contains one public graphics owner and the CP2P runtime symbols, not the standalone `pstvnc_h1_video_run_session` path.

The public all-guns CONFIG gate remains deliberately unchanged at this checkpoint. CP2P still requires `video_mode=OFF`; in that state the exact-generation worker may be armed but decode-thread creation remains dormant. The `MPEG2_ES` live-thread path is compiled and linked for the later gate-opening milestone, but item #9 does not claim an ordinary live-MPEG session, Pi MPEG production, or hardware qualification. Item #10 remains the authority that will expose that path through public CP2P configuration.

Durable implementation commits before final proof:

- `ac4d557184a597004c8739b3d580d11e68dcef6e` — integrate generation-bound CP2P MPEG worker;
- `d1978a57151b410691912626435f923236843dae` — tighten exact worker/coordinator lifecycle edges.

Final software proof source head was `6925034802a5d847bffe09c8d4c783e5db1306bd`. GitHub Actions run `34706240485` completed `SUCCESS` under pinned PS2DEV image `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`.

The proof established together:

- current-tree CP2O build and RFB/PCM regressions PASS;
- CP2P application link PASS;
- generation-bound MPEG worker PASS;
- cancellable MPEG transport read linked;
- generated CP2P video runtime linked and standalone runtime excluded from the final image;
- START transport and first-frame handoff symbols linked;
- single public graphics owner PASS;
- one-transport/one-reader architecture preserved;
- public MPEG gate still dormant.

Proof artifact `h1-cp2p-item9-unqualified-elf` has artifact ID `10302040505`.

Exact proof ELF identity:

- file `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `89b57d33ce407cfb2420a7ef83819e426d5a2f8a5d1ba3e7a4dd1227c4f2966d`;
- ELF bytes `3228136`;
- PT_LOAD SHA256 `6a33a03b11d282768f1d64c19d45f62b2de071a7bfcd4e0bc5b0cae0bc7da6f5`;
- PT_LOAD bytes `509716`.

One boundary remains deliberately open for later lifecycle work: the transport MPEG byte queue is still session-scoped rather than generation-tagged. Because the Pi producer is not yet active, that does not invalidate item #9; before real generation transitions can carry MPEG bytes, later item #11 work must prevent stale old-generation queued bytes from being consumed by a fresh generation.

**Checklist transition:** item #9 — concurrent PS2 MPEG worker — **PARTIAL -> DONE at the pre-activation software-proof boundary**. The next planned tranche is combined item #6 + #7: Pi generation-scoped RFB suppression plus exact calibrated capture geometry, still without starting MPEG production.

---

## 37. Post-item-#9 checklist reconciliation and revised dependency order

With item #9 sealed, the twelve-item implementation list was reconciled again against the actual forward branch rather than against earlier numbering alone.

Current status:

1. **PS2 Accept -> START session coordinator — DONE.** The session coordinator is the authoritative accepted-calibration -> fresh immutable generation -> worker arm -> START transaction owner.
2. **PS2 START-send failure unwind — DONE.** Failure cannot leave a phantom prepared generation or armed MPEG worker; exact worker clear precedes exact generation abort.
3. **Prove accepted -> START transition — DONE.** Host contracts, pinned PS2 compile/link evidence, and later integrated proofs cover the control-plane transition and failure boundaries.
4. **Link a real CP2P PS2 target — DONE.** One integrated CP2O-derived CP2P ELF contains the real coordinator, shared compositor, START path, recalibration/RFB flow, and now the item-#9 MPEG worker machinery.
5. **Pi START receive + validation — DONE.** One existing Pi reader accepts/validates the 44-byte channel-4 START and creates one immutable prepared generation without activating suppression or MPEG production.
6. **Pi generation-scoped RFB suppression — OPEN.** The required ownership semantics are defined, but no real source-suppression mechanism is installed yet.
7. **Exact calibrated geometry end-to-end — PARTIAL.** PS2 accepted geometry and START serialization are complete; Pi validation retains exact base and suppression rectangles, but the exact base rectangle is not yet bound to a real capture source and the suppression rectangle is not yet applied to RFB production.
8. **START-driven MPEG producer — OPEN.** START is received and retained, but deliberately does not start ffmpeg/producer output yet.
9. **Concurrent PS2 MPEG worker — DONE at the pre-activation software-proof boundary.** Exact-generation arm/clear, cancellable MPEG queue read, generated compositor-aware runtime, first-frame physical-presentation ownership, and pinned link proof are complete; public CONFIG still keeps the live decode path dormant.
10. **Exact CP2P CONFIG gate — OPEN.** The public CP2P configuration still rejects MPEG-enabled operation; this remains intentionally late so configuration exposes an already-correct mechanism rather than being used as scaffolding.
11. **Complete presentation / retirement / recalibration lifecycle — PARTIAL.** The PS2 side is substantially complete, including worker clear before exact ownership retirement and full-RFB restoration. Remaining live-data-plane work includes a concrete Pi-facing exact-generation retirement seam, removal of generation-scoped Pi suppression/producer state, and prevention/drain of stale old-generation MPEG bytes from the currently session-scoped PS2 MPEG queue.
12. **Seal the immutable all-guns hardware candidate — OPEN.** Final source/config/runtime identities, deployment, physical qualification, and evidence remain after the live all-guns lifecycle is complete.

The dependency order is therefore refined from the earlier simple numerical sequence.

Recommended next order:

`#6 + #7 -> #11A Pi exact-generation retirement control seam -> #8 START-driven producer -> finish #11 live generation cleanup/stale-byte exclusion -> #10 CONFIG gate -> #12 immutable hardware candidate`

Rationale:

- #6 and #7 can safely prepare suppression and exact capture state without emitting MPEG bytes.
- A real producer must **not** be activated before the Pi can be told that generation N has retired. Recalibration requires old suppression and producer state to disappear before the fresh full-RFB restoration that precedes a new calibration transaction; waiting for the next START is too late.
- Therefore the Pi-facing retirement-control subset of #11 is now a hard dependency of #8, even though the remainder of #11 can close after the producer exists.
- Once exact Pi retirement exists, #8 can safely make valid START ordering `validate -> bind generation -> install suppression -> configure exact capture -> start producer -> emit MPEG`.
- The remaining #11 closure then proves stop/drain/removal across both machines and excludes stale old-generation bytes before a fresh generation can consume MPEG.
- Only after those mechanisms are live and correct should #10 expose MPEG through the public CP2P CONFIG gate.
- #12 remains the final frozen-source/config/deployment/hardware qualification milestone.

This replaces the earlier proposed `#6 + #7 -> #8 -> #11` ordering. The change is a dependency refinement discovered after item #9 made worker retirement and the session-scoped MPEG queue concrete; it does not invalidate any completed item.

---

## 38. Checklist refinement — item #11 is split into #11A and #11B

After item #9 was sealed and the post-item-#9 dependency review made the Pi retirement requirement concrete, the remaining lifecycle work is now tracked explicitly as two sub-items rather than one broad item #11.

This is a refinement of the checklist, not a renumbering of historical work. Earlier references to item #11 remain historically correct for the broader lifecycle milestone as it was understood at the time.

The current checklist is:

1. **PS2 Accept -> START session coordinator — DONE.**
2. **PS2 START-send failure unwind — DONE.**
3. **Prove accepted -> START transition — DONE.**
4. **Link a real CP2P PS2 target — DONE.**
5. **Pi START receive + validation — DONE.**
6. **Pi generation-scoped RFB suppression — OPEN.**
7. **Exact calibrated geometry end-to-end — PARTIAL.**
8. **START-driven MPEG producer — OPEN.**
9. **Concurrent PS2 MPEG worker — DONE at the pre-activation software-proof boundary.**
10. **Exact CP2P CONFIG gate — OPEN.**
11A. **Pi exact-generation retirement control — OPEN.** This is the control-plane prerequisite that allows the Pi to retire one exact generation before any replacement START exists: stop/drain that generation's producer if present, remove that generation's RFB suppression, release that generation's prepared/active capture state, and leave every other generation untouched.
11B. **Complete cross-machine presentation / retirement / recalibration lifecycle — PARTIAL.** This is the end-to-end lifecycle closure: coordinate PS2 worker retirement with Pi retirement, ensure fresh full-RFB restoration occurs only after old Pi suppression is gone, prevent/drain stale old-generation MPEG bytes before a fresh generation can consume media, and prove confirm/cancel/recalibration transitions across both machines.
12. **Seal the immutable all-guns hardware candidate — OPEN.**

The current preferred implementation order is:

`#6 + #7 -> #11A -> #8 -> #11B -> #10 -> #12`

The reasoning at this checkpoint is:

- #6 and #7 can safely prepare exact generation-scoped suppression and exact capture geometry without emitting MPEG bytes.
- #11A must exist before #8 can safely make a valid START launch a real Pi producer, because generation N must be independently retireable before N+1 exists and before the fresh full-RFB restoration used for recalibration.
- Once exact Pi retirement exists, #8 can activate the already-defined ordering `validate START -> bind exact generation -> install suppression -> configure exact capture -> start producer -> emit MPEG`.
- #11B can then close the fully live cross-machine lifecycle, including stale-byte exclusion on the currently session-scoped PS2 MPEG queue.
- #10 should expose MPEG through the public CP2P CONFIG gate only after the mechanisms behind that gate are correct.
- #12 remains the final provenance/deployment/physical-qualification milestone.

This order is **not declared immutable**. It is the best dependency interpretation supported by the evidence available at this point in the experiment. Future implementation or hardware evidence may expose a better ordering or reveal that a remaining item should be split, merged, or moved. If that happens, the later understanding should be appended to this history rather than retroactively rewriting this entry.

The next intended implementation tranche is combined **#6 + #7**: real Pi generation-scoped RFB suppression plus exact calibrated capture geometry, while deliberately keeping MPEG production dormant.


---

# PHASE O — SEPTEMBER 12 PI SUPPRESSION + EXACT CAPTURE PREPARATION

## 39. Items #6/#7 advance together; compound START preparation is made atomic before seal

After item #9 and the explicit #11A/#11B checklist split, work continued in the intended dependency order on a new branch:

`experiment/h1-cp2p-pi-suppression-geometry`

The branch was created from checklist checkpoint `0f016fea0264c0b892e1356c81793657f06c1670`. The tranche deliberately kept MPEG production dormant while making the Pi state that must exist before producer launch concrete.

### Item #6 — generation-scoped Pi RFB suppression

The new CP2P-only bridge is:

`experiments/media-harness-h1/h1_cp2p_rfb_suppression.py`

The normal CP2O bridge is not rewritten. CP2P substitutes this bridge through the existing construction seam while preserving one VNC connection, one PSTV connection, one Pi PSTV reader, and the existing framebuffer authority.

A valid immutable START installs one exact generation as **pending** suppression. Pending suppression does not activate merely because START was received. It becomes active immediately before the first **new** RFB `FramebufferUpdateRequest` forwarded after START. Therefore an older request already outstanding at calibration acceptance may complete unchanged while the PS2 remains frozen in `WAIT_FIRST_FRAME`.

Once active, the bridge parses one upstream Raw `FramebufferUpdate` transaction at a time, removes pixels covered by the active suppression rectangle, emits the remaining Raw rectangles with a correct rectangle count, and preserves other server-message classes. The implementation relies on the established Raw-only RFB encoding contract while suppression is active; it does not attempt to clip arbitrary compressed encodings.

No desktop resolution is hard-coded. The maximum Raw pixel transaction budget is derived for each active session as:

`current_desktop_width * current_desktop_height * 2`

where the factor two is the negotiated 16-bit RFB pixel format, not a resolution assumption. Host tests explicitly exercised 640x448, 704x462, 704x480, and 1280x720.

The bridge also exposes `retire_suppression_exact(generation)`. It removes only the named generation and rejects mismatches. This is a useful local primitive for item #11A but is **not** itself the missing PS2->Pi retirement control path.

### Item #7 — exact START-derived capture preparation

The new capture authority is:

`experiments/media-harness-h1/h1_cp2p_capture_geometry.py`

The current active desktop dimensions are used only as bounds authority. The MPEG capture source is taken directly from the immutable START base rectangle. The prepared FFmpeg x11grab command uses:

`DISPLAY+X,Y`

and:

`-video_size W×H`

for the exact accepted START base geometry. The suppression rectangle is not substituted for capture. No fixed startup-profile rectangle, full-desktop grab plus crop, or scale-to-fit workaround is introduced.

The encoder command is only prepared and recorded. FFmpeg is not launched and no MPEG bytes are emitted. Producer activation remains item #8.

### Ordered compound preparation

The CP2P Pi runner now performs:

`validated START -> immutable prepared generation -> pending exact-generation suppression -> exact START-base capture plan -> evidence`

with the producer still dormant.

Initial host proof at source head `c02d7ac8cc33fa40b594637f1fa215e5d3c7c475` passed GitHub Actions run `34709167702`. That proof covered the existing START contract, qualified Raw RFB bridge regression, five suppression tests, five exact-capture tests, and the dynamic desktop resolutions above.

A later source audit identified one real atomicity edge before seal: suppression was installed before exact capture preparation, but a later capture-preparation failure (for example an empty X11 display string) released the item-#5 prepared START without removing the already-installed suppression generation.

That half-installed-state loophole was closed in commit:

`81d150988e5bc0435c580465b3a826429aa7c139`

`h1: make CP2P START suppression/capture setup atomic`

The compound transaction now rolls back exact suppression, usable prepared START state, capture state, and prepared evidence together if any later setup/evidence step fails. Crucially, rollback does **not** lower item-#5's generation high-water: the failed generation remains stale and cannot be reused.

A focused host contract `h1_cp2p_start_preparation_transaction_test.py` forces failure after suppression installation, proves no pending/active suppression or capture/prepared state remains, then retries the same generation after fixing the environmental error and proves that generation remains stale/rejected.

Final corrected software proof:

- branch: `experiment/h1-cp2p-pi-suppression-geometry`;
- source head: `81d150988e5bc0435c580465b3a826429aa7c139`;
- GitHub Actions run: `34710050027`;
- host-contracts job: `103597045614`;
- result: `SUCCESS`.

The final proof passed together:

- Python compile for START/suppression/capture/compound-rollback modules;
- existing item-#5 START receive/validation contract;
- qualified Raw RFB bridge regression;
- generation-scoped dynamic-desktop suppression contract;
- exact START-derived capture geometry contract;
- compound START preparation rollback/high-water contract.

This remains host/software proof. It does not claim live producer execution, cross-machine retirement, public MPEG CONFIG activation, deployment, or hardware qualification.

### Checklist status after this tranche

The current list is now:

1. **PS2 Accept -> START session coordinator — DONE.**
2. **PS2 START-send failure unwind — DONE.**
3. **Prove accepted -> START transition — DONE.**
4. **Link a real CP2P PS2 target — DONE.**
5. **Pi START receive + validation — DONE.**
6. **Pi generation-scoped RFB suppression — DONE at the current software-proof boundary.** The concrete suppression mechanism, dynamic current-desktop budgeting, exact generation ownership, pending activation boundary, Raw filtering, local exact-retirement primitive, and rollback behavior are host-proven. Cross-machine retirement signaling remains #11A.
7. **Exact calibrated geometry end-to-end — PARTIAL.** Exact START base geometry now reaches one prepared FFmpeg capture command with current-desktop bounds validation, but the plan has not yet been consumed by a live producer; that activation remains #8.
8. **START-driven MPEG producer — OPEN.**
9. **Concurrent PS2 MPEG worker — DONE at the pre-activation software-proof boundary.**
10. **Exact CP2P CONFIG gate — OPEN.**
11A. **Pi exact-generation retirement control — OPEN.**
11B. **Complete cross-machine presentation / retirement / recalibration lifecycle — PARTIAL.**
12. **Seal the immutable all-guns hardware candidate — OPEN.**

The best current dependency order is therefore:

`#11A -> #8 -> #11B -> #10 -> #12`

This ordering remains subject to later refinement as implementation and hardware evidence improve understanding. At this checkpoint, however, #11A remains the next architectural prerequisite: generation N must be independently retireable on the Pi, with completion known before the fresh full-RFB restoration used for recalibration, **before** item #8 is allowed to turn the prepared capture plan into a live producer.


---

# PHASE P — SEPTEMBER 12 EXACT-GENERATION PI RETIREMENT CONTROL

## 40. Item #11A closes with Pi-first exact retirement before local PS2 ownership retirement

After the sealed #6/#7 checkpoint, work continued in the documented dependency order on:

`experiment/h1-cp2p-pi-retirement-control`

The purpose of item #11A was narrow but architectural: generation N had to be independently retireable on the Pi **before** item #8 was allowed to make the prepared exact capture plan launch a real producer, and completion had to be known before recalibration could create the fresh full-RFB restoration transaction.

### Retirement wire/control seam

The implemented H1 retirement control uses a new experiment-local PSTV frame kind rather than overloading MPEG channel-4 DATA:

- frame kind `10` (`PSTVNC_H1_FRAME_MPEG_RETIRE`);
- control channel `0`;
- flags `0`;
- exactly 12 payload bytes;
- big-endian words: `version=1`, active H1 `session_id`, exact nonzero `generation`;
- PS2 -> Pi is the retirement request;
- Pi -> PS2 with the identical payload is completion;
- direction plus one exact pending generation on each side disambiguates request/completion.

This deliberately leaves channel-4 DATA as the START/media data-plane vocabulary rather than introducing retirement bytes into the future MPEG stream.

The PS2 transport arms the exact pending generation **before** it sends RETIRE so an immediate Pi completion cannot race the sole receiver thread. The receiver accepts completion only for the exact active session and exact pending generation. The application coordinator polls that receiver-owned completion asynchronously; it does not become another socket reader and does not block the sole receive owner.

### Pi exact cleanup before completion

The existing same-reader CP2P START shim now also consumes RETIRE on the same H1 reader call chain. No second PSTV socket or reader thread is introduced.

At this pre-producer checkpoint, exact Pi retirement requires all of these to name the requested generation:

- immutable prepared START;
- exact capture plan;
- RFB suppression owner.

If any identity mismatches, the request fails without mutating another generation and no completion is sent.

With the producer still deliberately dormant, successful retirement performs:

`validate exact N -> prove no unexpected live producer -> remove suppression(N) -> release prepared START(N) -> release capture plan/evidence(N) -> send RETIRE completion(N)`

The item-#5 generation high-water is not lowered. A retired generation remains stale and cannot be reused.

The live-producer guard is intentional rather than a missing error path: item #8 must replace it with exact producer stop/drain for N **before** suppression removal. The already-proven wire and later cleanup/completion ordering should not need to change when the producer becomes real.

### Why Pi retirement happens before local PS2 owner retirement

During review, two possible orderings were considered.

One possible design was:

`local PS2 retire -> request Pi retire -> wait ACK -> allow fresh full RFB`

That would require an additional RFB barrier because local owner retirement immediately creates the existing full-RFB restoration obligation. More importantly, a failed or lost Pi retirement could leave the PS2 already retired while the Pi still held old suppression/producer state.

The adopted design keeps the old PS2 generation authoritative while the Pi transaction is pending:

`active PS2 generation N -> request Pi RETIRE(N) -> wait exact completion -> existing local clear/owner retire -> existing fresh full-RFB restoration -> calibration entry`

This is both simpler and more transactional. While completion is pending, the coordinator test proves:

- no second RETIRE is sent;
- local `clear_mpeg` has not run;
- PS2 remains `MPEG_OWNED` for N;
- the current START contract remains valid;
- no local retirement-created full-RFB obligation exists yet.

Only exact Pi completion unlocks the already-proven local recalibration path. The local worker/owner is then retired and the next RFB scheduler decision is the required fresh FULL request. Therefore the restoration cannot precede proof that Pi suppression/capture state has disappeared.

### Source authorities

The item #11A source additions/changes include:

- `experiments/media-harness-h1/h1_cp2p_retirement_control.py` — 12-byte retirement codec and H1 kind-10 vocabulary;
- `experiments/media-harness-h1/h1_cp2p_start_receiver.py` — same-reader RETIRE dispatch and completion only after exact cleanup;
- `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py` — exact Pi prepared/capture/suppression retirement and dormant-producer guard;
- `experiments/media-harness-h1/h1_transport_runtime.[ch]` — PS2 send-before-race pending state, sole-receiver exact completion match, and asynchronous poll seam;
- `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]` — Pi-first recalibration gate ordering;
- `experiments/media-harness-h1/h1_cp2p_retirement_control_test.py` — Pi exact cleanup/failure/stale-generation contract;
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_cp2p_session_coordinator_test.c` — PS2 Pi-first ordering contract.

No change was required to the existing recalibration or RFB-flow mechanism itself because the coordinator does not invoke local recalibration retirement until Pi completion has already been proven.

### Software proof

Final source/proof authority:

- branch: `experiment/h1-cp2p-pi-retirement-control`;
- source head: `d8d731ce9f66fc14ea7bfc2777215085b69975ae`;
- GitHub Actions run: `34710972431`;
- host-contracts job: `103599586082` — SUCCESS;
- pinned PS2-build job: `103599586147` — SUCCESS;
- PS2DEV image: `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`;
- proof artifact: `h1-cp2p-item11a-unqualified-elf`, artifact ID `10302704205`.

The host proof passed the prior START, RFB bridge, suppression, exact-capture, compound-rollback, and full calibration host suites together with the new exact-retirement contract.

The pinned PS2 build proved current-tree CP2O regression, CP2P application linking, exact retirement begin/poll symbols, generation-bound worker/cancellable-read symbols, one public graphics owner, unchanged item-#10 MPEG gate, and the one-transport/one-reader architecture.

Exact proof ELF identity:

- file `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`;
- ELF bytes `3232156`;
- PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`;
- PT_LOAD bytes `510612`.

This remains a pre-producer software proof. It does **not** claim live FFmpeg execution, MPEG data flow, live producer stop/drain, stale queued MPEG exclusion, public MPEG CONFIG activation, deployment, or physical hardware qualification.

### Checklist status after item #11A

1. **PS2 Accept -> START session coordinator — DONE.**
2. **PS2 START-send failure unwind — DONE.**
3. **Prove accepted -> START transition — DONE.**
4. **Link a real CP2P PS2 target — DONE.**
5. **Pi START receive + validation — DONE.**
6. **Pi generation-scoped RFB suppression — DONE at the current software-proof boundary.**
7. **Exact calibrated geometry end-to-end — PARTIAL.** Exact START geometry reaches the prepared capture command; live consumption remains #8.
8. **START-driven MPEG producer — OPEN.** This is now the next implementation tranche. It must consume the existing exact prepared capture plan and extend #11A with exact producer stop/drain before suppression removal/completion.
9. **Concurrent PS2 MPEG worker — DONE at the pre-activation software-proof boundary.**
10. **Exact CP2P CONFIG gate — OPEN.**
11A. **Pi exact-generation retirement control — DONE at the pre-producer software-proof boundary.** Exact request/completion, Pi cleanup-before-completion, and Pi-completion-before-local-retire/full-RFB ordering are proven.
11B. **Complete cross-machine presentation / retirement / recalibration lifecycle — PARTIAL.** Live producer retirement and stale old-generation MPEG-byte exclusion remain once #8 creates the live data plane.
12. **Seal the immutable all-guns hardware candidate — OPEN.**

The best current dependency order becomes:

`#8 -> #11B -> #10 -> #12`

As before, this order is subject to later reinterpretation with better implementation or hardware evidence. At this checkpoint, however, the prerequisite that forced #11A ahead of #8 is satisfied: generation N can be exactly retired on the Pi and completion can be proven before local PS2 retirement creates the fresh full-RFB restoration transaction.


---

# PHASE Q — SEPTEMBER 12 START-DRIVEN PI MPEG PRODUCER

## 41. Item #8 — START-driven Pi MPEG producer closes behind the public gate

Work continued from sealed item-#11A head `d2283b44cd9c240b26b9bda2f045a7ca1452af06` on branch `experiment/h1-cp2p-start-driven-producer`. The objective was deliberately narrower than all-guns activation: make a valid immutable START wake a real exact-region Pi encoder owned by that generation, while keeping item #10's public MPEG CONFIG gate closed so no MPEG bytes could yet enter PSTV.

The implementation first landed as `f9ad2f991713256890409fdfb55399bff4eaabd9` (`h1: add START-driven generation MPEG producer`) and was then hardened at `0347e2f1a0295a83aa7051fc9b963b7f3ece85a2` (`h1: harden START-driven producer rollback ordering`). The latter is the item-#8 product-source authority.

`h1_cp2p_mpeg_producer.py` now owns one exact generation. It launches only the already-prepared FFmpeg/x11grab command derived from the accepted START base X/Y/W/H after that same generation's RFB suppression and capture plan are established. It does not reconstruct geometry independently. The producer is attached to the existing H1 session and uses the existing bounded producer-buffer behavior; it creates no second PS2-facing transport, reader, frame-sequence owner, or bypass writer.

The public all-guns gate remains intentionally closed. Item #8 therefore proves a real local encoder process and lifecycle, **not** MPEG transport activation: generated MPEG output remains locally bounded/backpressured and `h1_cp2p_mpeg_producer.py` has no PSTV channel-4 send path. `H1_CP2P_ITEM8_PUBLIC_GATE=CLOSED` is part of the proof boundary. Item #10 remains the later authority that will expose MPEG DATA only after the live generation-transition boundary is safe.

The important lifecycle addition is exact producer retirement. For generation N, Pi cleanup now orders:

`stop/drain/discard exact producer N -> prove subprocess + reader quiescent -> remove suppression N -> release prepared START/capture/evidence N -> send exact RETIRE completion N`.

Every locally buffered unsent MPEG byte is drained/discarded before suppression can be removed. Wrong-generation retirement is rejected. If graceful stop cannot quiesce the producer, termination is attempted; if quiescence still cannot be proven, retirement fails closed. A review during implementation found and corrected a subtler rollback edge: a failed post-launch START transaction originally could have continued removing suppression after producer stop/drain failed. The hardened path now stops rollback at that boundary and deliberately retains suppression plus prepared-generation state when producer dormancy cannot be proven.

That ordering preserves item #11A's Pi-first cross-machine rule. The PS2 still retains generation N as authoritative while waiting for Pi completion; the Pi cannot acknowledge until the exact local producer and suppression/capture state are safely retired; only then may PS2 local retirement create the fresh full-RFB restoration obligation.

Host coverage now includes:

- `h1_cp2p_mpeg_producer_test.py`: exact START-plan launch, no PSTV MPEG emission, exact retirement and unsent-byte drain, wrong-generation refusal, and stuck-producer fail-closed behavior;
- `h1_cp2p_retirement_control_test.py`: a live generation-owned producer retires before suppression removal and completion, while an unowned live legacy producer blocks acknowledgement;
- `h1_cp2p_start_preparation_transaction_test.py`: post-launch rollback failure that cannot prove producer quiescence retains suppression/prepared state fail-closed;
- the full prior #5/#6/#7/#11A Pi contract suite;
- the complete calibration/session host suite.

A first green proof run, `34711889280`, applied the final safety hardening in the runner worktree and then committed it as `0347e2f1...`. Because the project distinguishes tested working-tree state from immutable committed-source proof, a second clean verification was required. CI-only head `51e1141f252edec2a7c1b19f445d941033e6fa19` differs from the product source only by a one-line workflow trigger comment. GitHub Actions run `34712568840` checked out that committed tree; all item-#8 staging helpers reported `ALREADY_APPLIED`, the working tree remained product-clean, and both jobs passed:

- host-contracts `103603873398` — SUCCESS;
- ps2-regression `103603873498` — SUCCESS.

The pinned PS2 regression used `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11` and retained the exact item-#11A PS2 binary identity, as expected for this Pi-only product tranche:

- ELF `PS2VNC-H1-CP2P-ApplicationLink.ELF`;
- SHA256 `5667e14ff412b5848ced30801bac184320ed852a23603072aa13f7a4fcd75ddc`;
- ELF bytes `3232156`;
- PT_LOAD SHA256 `5bad594faf8555684f06cb854fad639fa5d3c8b69d9691e737fa6cf386d11336`;
- PT_LOAD bytes `510612`;
- `H1_CP2P_ITEM8_PS2_REGRESSION=PASS`;
- `TESTKIT_PT_LOAD_FINGERPRINT=PASS`.

This changes the checklist at the software boundary:

- **#7 exact calibrated geometry end-to-end: PARTIAL -> DONE**, because the immutable START base rectangle is now consumed by the real generation-owned encoder process rather than ending at a prepared command;
- **#8 START-driven MPEG producer: OPEN -> DONE behind the public gate**;
- **#11A exact Pi retirement control remains DONE and is now proven with a live local producer**;
- **#11B remains OPEN/PARTIAL and is next**: before item #10 permits real MPEG DATA, the cross-machine data plane must ensure stale generation-N bytes in the session-scoped MPEG transport path cannot be consumed by generation N+1;
- **#10 remains OPEN/CLOSED by design**;
- **#12 remains OPEN**.

No deployment, physical PS2 behavior, concurrent live MPEG transport, or all-guns hardware stability is claimed by item #8. The preferred remaining order is **#11B -> #10 -> #12**.
