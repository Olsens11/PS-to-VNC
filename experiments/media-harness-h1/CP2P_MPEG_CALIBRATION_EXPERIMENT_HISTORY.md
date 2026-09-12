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

# PHASE J — CLEAN RECALIBRATION TRANSACTION LIFECYCLE

## 31. September 12 — START+SELECT becomes a fresh calibration transaction boundary

A live design review simplified the MPEG recalibration lifecycle.

The settled operator rule became:

- START+SELECT always starts a **new calibration transaction**;
- an existing MPEG generation is never edited or evolved in place;
- if no MPEG generation exists, the current RFB desktop may freeze and calibration may enter immediately;
- if a generation exists (including `WAIT_FIRST_FRAME`), its immutable committed rectangle and inner/outer matte settings are preserved as the seed for the new transaction;
- that generation is retired before calibration begins;
- RFB must return to a **new full-frame state after retirement** before calibration freezes the screen;
- a full request already in flight before retirement is not sufficient, because it may have been generated while the old MPEG suppression footprint still applied;
- after the new post-retirement full RFB update completes and has had its presentation opportunity, calibration enters using the preserved settings;
- confirm creates a **new generation** from the newly committed settings, whether unchanged or edited;
- cancel creates no generation and returns to full-frame RFB + PCM audio.

This intentionally eliminates long-running generation mutation bookkeeping. The only continuity carried between MPEG generations is the user's last confirmed calibration configuration.

The first experiment-local lifecycle primitive was implemented in commit:

`d2af6c4918b6d9af1123d8704ca97829e6e73271`

`h1: add clean CP2P recalibration retirement lifecycle`

It added the exact-generation retirement helper plus a small RFB restoration watch. Host validation run `34694084275` passed, including:

- `H1_MPEG_CP2P_RFB_RESTORATION_HOST_TEST=PASS`;
- `H1_MPEG_RECALIBRATION_HOST_TEST=PASS`.

The restoration watch explicitly rejects a FULL request that was already outstanding when the new restoration epoch was armed.

## 32. September 12 — lifecycle wired into the CP2P interaction path

David then authorized wiring the settled lifecycle into the CP2P application-interaction path.

The implementation deliberately preserved CP2O behavior. The ordinary `h1_interaction_coordinator.c` remained unchanged and CP2O continues compiling it directly. CP2P instead gained a narrow translation unit that reuses the exact coordinator source while redirecting only the existing 750 ms calibration-entry hold `observe` / `poll` calls through the recalibration bridge.

Primary wiring commit:

`4d307d022a7595da28059d31dd0fa496a10e1f31`

`h1: wire clean recalibration into CP2P interaction path`

The new CP2P composition now provides:

- reuse of the real 750 ms START+SELECT hold rather than a second gesture detector;
- immediate ordinary calibration entry while RFB-only;
- exact-generation MPEG retirement when the held chord matures with MPEG active;
- consumption of controller observations while the post-retirement RFB restoration is pending, preventing START/SELECT from leaking into desktop/OSK routing;
- re-issuance of the coordinator's existing synthetic START+SELECT calibration activation only after the new FULL restoration has completed;
- one CP2P-owned `mpeg_handoff`;
- one combined calibration + MPEG RFB flow policy;
- one recalibration transaction state;
- a caller-supplied exact-generation `clear_mpeg` seam for withdrawing producer/visible MPEG state without duplicating presentation-owner retirement;
- continued use of the existing calibration presenter.

Source inspection confirmed that the CP2P shared graphics module already replaces the public `pstvnc_ps2_graphics_present()` owner. Therefore ordinary/calibration RFB presentation through the existing presenter naturally updates the shared compositor's cached desktop while active MPEG remains protected by its suppression/matte footprint; no second presenter was added.

Validation run:

`34694695086`

completed successfully at exact head `4d307d022a7595da28059d31dd0fa496a10e1f31`.

The strict host suite remained green, including the new:

`H1_MPEG_RECALIBRATION_ENTRY_BRIDGE_HOST_TEST=PASS`

The pinned PS2 toolchain also compiled all CP2P interaction-side pieces successfully, with markers:

- `H1_CP2P_SHARED_COMPOSITOR_COMPILE=PASS`;
- `H1_CP2P_VIDEO_RUNTIME_COMPILE=PASS`;
- `H1_CP2P_RECALIBRATION_ENTRY_BRIDGE_COMPILE=PASS;
- `H1_CP2P_INTERACTION_COORDINATOR_COMPILE=PASS`;
- `H1_CP2P_INTERACTION_WRAPPER_COMPILE=PASS`.

The temporary diagnostic workflow was then removed in commit:

`e549daaa4a144b39b6cfc7b00fd2d1f979b8f2ed`

`ci(h1): retire CP2P recalibration diagnostic`

Proof boundary at this entry: the clean START+SELECT retirement/re-entry lifecycle is now wired into a CP2P-specific interaction composition and host/PS2-cross-compile proven. This does **not** yet claim a complete runnable all-guns ELF or hardware qualification. The final CP2P main still has to instantiate this composition alongside the concrete MPEG worker/producer-stop callback, accepted→START live send, Pi START/suppression/producer path, and exact CONFIG gate.
