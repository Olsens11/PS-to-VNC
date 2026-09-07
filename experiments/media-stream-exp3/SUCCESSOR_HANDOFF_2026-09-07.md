# PS-to-VNC Media-Streaming Successor Handoff — 2026-09-07

## Read this before continuing the MPEG/audio/media-region work

This handoff is intentionally redundant with individual evidence records and
`O3_MEDIA_STREAM_OPTIMIZATION_SYNTHESIS_2026-09-07.md`. The redundancy is
purposeful. The September 7 media campaign produced one of the most important
architectural/performance breakthroughs in the project and must be recoverable
without relying on conversation history.

The repository itself is authority. Conversation summaries are convenience only.

Before proposing code or a new hardware test:

1. read repository-root `AGENTS.md`;
2. read `docs/status.md`;
3. read `docs/README.md`;
4. read `docs/PROJECT_INTENT.md`;
5. read `docs/CLEAN_ARCHITECTURE.md`;
6. read `docs/development/README.md`;
7. read this handoff;
8. read `O3_MEDIA_STREAM_OPTIMIZATION_SYNTHESIS_2026-09-07.md`;
9. read `O3_LIVE_P12_TRUE_LIVE_4M_HARDWARE_RESULT.md`;
10. read `O3_LIVE_PERFORMANCE_P11_DEEP_4M_HARDWARE_RESULT.md`;
11. read the audio-transport experiment README at the pinned audio branch;
12. inspect branch/history/status/staged/unstaged/untracked state;
13. run the repository's normal resume/check procedures applicable to this
    branch before changing source.

Do not reset, clean, or overwrite dirty work merely to make the worktree match
this document. First determine whether the state is new intentional work.

---

# 1. What just happened and why it matters

PS2VNC historically had a major weakness: ordinary RFB framebuffer traffic is a
poor fit for high-motion desktop/video regions. During this session, an isolated
MPEG-2 media path evolved from “it can decode a file” to **true live Pi desktop
video that looks good on a real PS2 and has clean timing telemetry**.

The key breakthrough was not a new decoder algorithm. It was understanding the
whole pipeline correctly:

```text
Pi desktop/browser
    -> X11 capture
    -> FFmpeg MPEG-2 encoder
    -> PSTV logical MPEG2 channel
    -> PS2 network receiver
    -> bounded compressed-byte queue
    -> SMS libmpeg / IPU
    -> RGB16 decoded macroblocks
    -> GS upload
    -> absolute presentation clock
    -> visible video
```

Earlier playback looked clunky and alternated between slow periods and fast
catch-up. Hardware evidence eventually showed two independent design problems:

1. fixed post-frame VSYNC waits were the wrong presentation scheduler;
2. the 32 KiB compressed-byte reservoir was far too shallow for variable-rate,
   bursty real-time MPEG delivery.

Once presentation moved to an absolute clock and the compressed reservoir became
large enough, the *same general decoder and transport path* became extremely
smooth.

The current strongest result, P12, is true-live X11→MPEG→Ethernet→PS2 playback
using the same P11 PS2 binary that had already handled a harder fixed 4M stress
stream. P12 measured:

- zero decoder-feed waits;
- zero presentation deadline misses;
- zero presentation lateness;
- `MPEG_Picture()` average about 6.97 ms;
- `MPEG_Picture()` maximum about 23.68 ms;
- RGB16 GS upload about 0.824 ms average;
- GS draw about 0.0084 ms average;
- total serial decode+upload+draw indicator about 7.805 ms average;
- frame period 33.3667 ms;
- approximately 25.56 ms average nominal frame-time headroom;
- 720 coded pictures→719 decoded/displayed;
- full transport/CRC/end contract PASS.

The operator watched a true-live YouTube video and described it as surprisingly
good. A few subtle lulls/catch-ups may have been visible, but none correlated with
PS2 feed starvation or deadline misses; they are therefore likely upstream in the
Pi/browser/X11/encoder cadence rather than the PS2 presentation path.

This is a foundational architecture result: MPEG is now a credible accelerated
media plane beside RFB rather than an emergency fallback or speculative idea.

---

# 2. Collaboration/debugging contract from the session

The user expects evidence-driven embedded development, not speculative tuning.
Preserve these working rules:

- change one major variable at a time;
- preserve exact DUT/source/apparatus identity;
- instrument first when the failure class is unclear;
- distinguish machine evidence from operator visual observation;
- treat “freeze” or “stutter” as symptoms, not root causes;
- identify the lowest layer still proven to make progress;
- preserve qualified results before proceeding;
- use mature/public PS2 implementations to study hardware behavior before
  inventing obscure low-level mechanisms;
- prefer root-cause fixes to masks;
- do not add silent automatic recovery/watchdogs while debugging a root cause;
- do not interpret a static screen as proof the main loop is alive unless the
  experiment deliberately uses static colors as terminal classification;
- keep discussion/design separate from implementation when requested;
- source must remain clearly named/commented with file synopses in clean product
  code;
- Pi should own flexible/sophisticated policy; PS2 should own bounded mechanisms.

The user is comfortable with experimental crashes/high memory values if the test
is controlled and evidence-bearing. Do not invent arbitrary “safe” ceilings that
hide the true frontier.

---

# 3. Worktrees and authorities

## 3.1 Qualified product worktree — do not mutate during EXP3 work

```text
/home/ps2/src/PS-to-VNC-issue39
branch: reconstruct/issue39-keyboard-osk-local-ui
HEAD: 5a8457d7e78ec1e847b06bca821880dc13b50eae
```

This is qualified product work and must stay exact/clean unless a deliberate
product integration stage explicitly starts from it.

## 3.2 MPEG media experiment

```text
/home/ps2/src/PS-to-VNC-media-stream-exp3
branch: experiment/media-stream-exp3
```

Hardware-result authority before the synthesis/handoff documentation:

`a002c3a5c74ba9111b081704768a1b0eafb66994`

That commit records P12.

Consolidated synthesis document commit:

`5a0f7782ef3318d1ee08dd78bfcfd0cb0b172729`

Fetch the branch and inspect its current head before any new work; this handoff is
being committed after the synthesis and therefore the branch may be one or more
document-only commits ahead of the P12 hardware-result commit.

Canonical EXP3 source used as the mechanical generator base throughout the later
campaign:

`experiments/media-stream-exp3/reference_network_live_streaming.c`

canonical blob:

`4e34795823cbb386f7ac958c43a2ddbce4e255e5`

Later P4/P5/P6/P7A/P8/P9/P11 sources were generated mechanically from this
canonical reference so the canonical source itself remained unchanged.

## 3.3 Audio transport experiment

```text
/home/ps2/src/PS-to-VNC-audio-transport-exp
branch: experiment/audio-transport-mux
HEAD: 15a879095cf6bc24df10268b6516bbb39c7a83ce
```

This workstream already established the transport principles needed for future
combined RFB/audio/MPEG work. Do not build a parallel unrelated media transport.

The earlier audio prototype used 48 kHz signed 16-bit stereo PCM and demonstrated
why independent uncoordinated RFB/audio streams are risky under heavy desktop
load. The dedicated-link experiment established bounded logical channels,
receiver credit, Pi-side policy, PS2-side truthful capacity, and a backend seam.

## 3.4 Audio configuration EXP2

```text
/home/ps2/src/PS-to-VNC-audio-config-exp2
branch: experiment/audio-config-exp2
implementation: 447143b31f026564e801da91f3422e83945db4fc
result record: 005f57effef6002d43baf6970eddbbae3013562e
```

Consult it if future audio integration needs configuration/capability history.

---

# 4. Build/toolchain authority

Pinned PS2DEV container:

`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Expected image ID:

`sha256:f4ec5e8dfd748f2e451731c5da73660fce6549a8cfda7fc2e494b588db17ca1c`

Compiler:

`mips64r5900el-ps2-elf-gcc 15.2.0`

Fresh container build environment generally restores:

```sh
export PS2DEV=/usr/local/ps2dev
export PS2SDK="$PS2DEV/ps2sdk"
export GSKIT="$PS2DEV/gsKit"
export PATH="$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin:$PATH"
apk add --no-cache bash build-base >/dev/null
```

PS2SDK source commit used in this investigation:

`3e46a44a21db66f42a56320b6f64bfc52ea279f8`

Pinned PS2IP archive SHA-256:

`b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

The modern system libmpeg is **not** the hardware-qualified MPEG decoder for
EXP3. Use the pinned vendored SMS implementation as established by the experiment.

---

# 5. Deployment and test-control rules

Canonical deployment tool:

`scripts/testkit/deploy-elf.py`

Do not manually reconstruct deployment when this tool applies.

Never overwrite:

`/mass/0/PS2VNC.ELF`

Experimental deployments use unique paths plus an experiment-specific rolling
path. The deployment tool records exact SHA/byte identity.

Current important rolling binary:

`/mass/0/PS2VNC-exp3-p11-deep-4m.ELF`

P11 ELF SHA-256:

`5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`

This exact ELF was reused by P12; P12 changed only the live producer.

PS2 FTP port 21 is available while wLaunchELF's FTP service is active. P12 did
not require a new upload because the P11 ELF was already on the card/USB mass.

---

# 6. `ps2run-interactive` rules and trap to remember

The Windows workflow uses `ps2run-interactive` to send a shell payload to the Pi
and capture/archive all output.

Scripts normally begin with something like:

```sh
id="descriptive-id"
exit # PS2RUN_INTERACTIVE_ONLY
set -euo pipefail
```

Critical wrapper behavior:

> Shell heredocs are unreliable through this transport.

Do not use:

```sh
cat <<EOF
...
EOF
```

or a `read ... <<EOF` construction. The outer wrapper may swallow/truncate at
heredoc delimiters.

Use variables, Python one-liners, `printf`, or a here-string `<<<` instead.

One earlier P10 command accidentally reintroduced a heredoc; do not repeat that
mistake.

---

# 7. Pi/network environment during qualification

```text
Pi Ethernet: 192.168.50.1
PS2:         192.168.50.2
link:        direct 100/full
Pi Wi-Fi:    192.168.0.132 (observed during this project)
X11:         :0 / :0.0
X desktop:   704x462, 16-bit depth
baseline VNC port: 5900
isolated RFB experiment: 5903 localhost
Windows tunnel used previously: localhost:5905 -> Pi:5903
historical audio port: 5901
EXP3 MPEG port: 5904
EXP3 UDP timing telemetry: 5999
```

NetworkManager can withdraw the Ethernet address on no-carrier transitions;
prior test wrappers sometimes included explicit link readiness handling.

---

# 8. Decoder/IPU/GS facts — do not relearn these by trial and error

## 8.1 Mature SMS decoder is qualified

Public SMS source studied:

- `ps2homebrew/SMS`;
- commit `c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- Eugene Plotnikov;
- AFL-2.0.

The modern PS2SDK libmpeg control reached initialization but hung in the first
picture path. The same major external contracts worked when decoder lineage was
changed to mature SMS. Use SMS unless a later isolated test explicitly repairs
and requalifies modern PS2SDK libmpeg.

## 8.2 IPU feed model

Qualified behavior:

- 64-byte-aligned staging buffer;
- 2048-byte ordinary TO_IPU DMA blocks;
- final short block padded/rounded to 16-byte/qword boundary;
- ordinary `dma_channel_wait(DMA_CHANNEL_toIPU, 0)`;
- `dma_channel_send_normal()`;
- no TO_IPU source chain.

Do not adopt unsupported folklore that every input transfer must be 64-byte
length aligned or that the decoder must wait for an arbitrary 4 KiB “complete
macroblock layer” before being called.

## 8.3 `dma_wait_fast()` is known bad here

A hardware bisection found that the fast wait helper can hang. Ordinary DMA waits
are part of the qualified path.

## 8.4 Decoded output pointer/ownership

Use an ordinary aligned EE picture pointer. GIF source-chain upload ownership
must retire before that decoded storage is overwritten.

## 8.5 RGB16

SMS exports `_MPEG_Set16`. P8 proved it on hardware.

Qualified current output:

- `_MPEG_Set16(1)`;
- RGB16;
- PSMCT16;
- 2 bytes/pixel;
- 512-byte /32-qword macroblocks.

This should remain the default performance path for continued media experiments.
Color-gradient precision still needs a dedicated visual test.

## 8.6 EOF/sequence end

Bounded raw MPEG tests append:

`00 00 01 B7` + twelve zero bytes.

Temporary queue starvation is not EOF. An indefinite session needs explicit
session lifetime separate from queue occupancy.

## 8.7 Terminal color classifier

- PURPLE = coded-minus-one;
- WHITE = coded-all;
- RED = another return-count relation;
- CYAN = exact transport/integrity success.

RED has occurred on perfectly valid transport when SMS returns coded-minus-two.
Do not equate RED with network corruption.

---

# 9. Complete performance chronology

This section duplicates the synthesis intentionally so a successor can recover
causality from this file alone.

## O1 — whole network fixture before decode

- code `3c9ff944eb53f55dd903d361d626e923ccba1359`;
- ELF `3a40ca6bea39db5cdf8f0482a9028e21cf646a84d4c531ef2f768e413d791089`;
- fixture 8,815,372 bytes, SHA
  `5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f`;
- 599 coded pictures;
- 3.415425 MiB/s acquisition;
- MPEG playback and integrity PASS.

**Meaning:** raw PS2 network acquisition is fast. This test was not concurrent
decode.

## O2 Q8192 — minimum concurrent queue

- same fixture;
- O2 ELF SHA
  `7c5015832213414651a6aa7f98e2a63c05e1e1b4a01230e8abd4fe63d4d50fd4`;
- queue 8192;
- stack16384, priority63;
- 0.116510 MiB/s;
- feed waits 1077 /47418 /max216;
- 598 returned/displayed;
- correct but very slow/choppy.

**Meaning:** concurrent receive/decode is stable, but a one-DATA-frame reservoir
causes near stop-and-wait behavior.

## O2 Q32768 — same exact code/source, queue only

- queue32768;
- 0.334671 MiB/s;
- feed waits382 /1593 /max48;
- smooth/snappy.

**Meaning:** queue depth is a huge performance variable.

## O3 initial true-live YouTube 4M

- X11 704x462→704x480;
- 30000/1001;
- MPEG2 `-b:v 4M`, GOP15, bf0;
- q32768;
- 551 coded→550 returned;
- 7,017,283 bytes;
- 26.741170 s;
- 0.250258 MiB/s;
- ~96.1% Pi credit-wait time;
- feed566 /4306 /max53;
- visibly clunky.

**Meaning:** complete live path correct; performance not acceptable.

## P1 — live 3M only

- geometry/queue/clock unchanged;
- target 4M→3M only;
- 6,809,062 bytes;
- 520 coded→519 returned;
- 26.1093 s;
- 0.248709 MiB/s;
- feed501/4624/max52;
- first few seconds smoother, then clunky.

**Meaning:** target bitrate reduction alone did not solve the sustained limit.

## P2 — local X11/FFmpeg control

No PS2/network:

- 704x462→704x480;
- yuv420p;
- 30000/1001;
- 3M, GOP15, bf0;
- nominal20s;
- 600 pictures in20.441630s;
- 7,732,149 bytes;
- SHA `3e3b3ac84366e254a3e4610a4ff94a0bb2ac33bae4018faf5ec9fa60d2aeeedc`;
- 0.360732 MiB/s wall;
- software decode PASS.

**Meaning:** Pi capture/encoder itself sustains real time. Slowdown was downstream.

## P3 — exact P2 archive replay, q32768

- same encoded bytes + qualified sequence end;
- two per-picture VSYNC waits;
- 27.827575s;
- 0.264988MiB/s;
- smoother than live but still clunky;
- 600 coded→598 returned.

**Meaning:** fixed live producer coupling was not the whole problem.

## P4 — zero presentation VSYNC waits

- only remove both per-picture waits;
- ELF `abc7763aa9282ea9b3764ea522df2cc2f58d6b40e48218fbc03862dd78f2f368`;
- 16.747557s;
- 0.440301MiB/s;
- source requirement0.368698MiB/s;
- 1.1942× real-time rate;
- visibly fast/choppy.

**Meaning:** decoder/IPU/GS aggregate performance is sufficient; two fixed waits
were a major throttle.

## P5 — one presentation VSYNC

- ELF `efd680caa9d207754fd2dbcfc2efe731353c66d1898948421f227b190893484c`;
- 19.207771s;
- 0.383905MiB/s;
- average ~1.041× real-time requirement;
- visibly fast/slow bursting.

**Meaning:** a correct average delay is not a correct presentation scheduler.

## P6 / P6B — absolute frame deadlines, no drop

P6 established an absolute 30000/1001 frame clock. If early, wait. If late, record
miss and display immediately. Do not drop.

P6B instrumentation-only ELF:

`523ab812971641e397ac4e207df9a349380193fdc2b57c6ce87a65819428c5f2`

Against P2 archive/q32768:

- deadline misses289 /598;
- max lateness39.967649 frame periods (~1.33s);
- campaign feed counters512 events /5757 loops /max1213;
- visibly bursty.

**Meaning:** backlog causes catch-up bursts because late frames skip waits but are
still displayed. Need remove source of lateness before considering drops.

## P7 — do not call this qualified

Late-GS-presentation-drop apparatus was prepared but intentionally **not run**.
Buffering root cause was investigated first. Do not continue from P7 merely
because its generator exists.

## P7A — first giant breakthrough

- exact P2 704x480/3M archive;
- same absolute clock/no-drop;
- queue32KiB→256KiB;
- prefill224KiB;
- ELF `efbf929323423757043dd195f9a7ee1471c18797085dc0a5ed108a54da59be20`;
- deadline misses289→6;
- max late39.967649→0.274205 frame;
- feed events512→7;
- loops5757→7;
- max1213→1;
- operator: “very best test yet.”

**Meaning:** compressed-input starvation from the shallow queue was a dominant
root cause. Do not use frame dropping to mask it.

## P7B — resolution-only 704x480→608x416

- exact same P7A ELF/buffering;
- 608x416;
- nominal3M, yuv420p,GOP15,bf0;
- 600 coded;
- source7,704,609 bytes;
- 599 returned;
- deadline misses6;
- feed13/91/max49;
- visually smooth; PURPLE→CYAN.

**Meaning:** 608x416 is a strong geometry; about25.15% fewer pixels than704x480.
Same nominal bitrate meant compressed-byte pressure did not fall proportionally.

## Deterministic 608x416 2M stress benchmark

Lossless reference:

- FFV1;
- SHA `1d42d891f276918dc9a4d6211cc21eec4e546a490c426f2d880d832175b82f0c`;
- 720 frames/24 s;
- general pattern, fine-grid motion, constant hard-edge scroll, high-detail noise.

MPEG:

- 608x416;
- 30000/1001;
- Main/Main;
- yuv420p;
- target2,000,000;
- max2,516,000;
- VBV1,835,008 bits;
- GOP15;
- bf0;
- bytes6,209,147;
- SHA `6bcfb93badf653a3d72de83e7d9253d01fe0027283f171954c9de6cefbf40aca`;
- queue256KiB/prefill224KiB;
- **0 feed waits,0 misses,0 lateness**;
- visually very smooth;
- 720→718 returned, RED→CYAN due delta2 only.

This is the fixed decoder/output benchmark.

## P8 — SMS-native RGB16

Only output changes:

- `_MPEG_Set16(1)`;
- RGB16/PSMCT16;
- 2B/pixel;
- 512B/32QW macroblocks.

ELF:

`ca6ad330ccb9925dee5423d3349316e919597156f2a1fcb1b444f4d4fbd2e023`

Same zero waits/misses/lateness and same visual smoothness. User could not see a
color difference on the motion reel. Dedicated gradient test still pending.

## P9 — stage timing

Instrumentation-only RGB16 ELF:

`cea181e37c89990e76d99d22a6fae65bd3a7c92cbea3bcc04e4fd9128d635d83`

Zero-starvation 2M stress:

- frame period33.366667ms;
- MPEG_Picture avg7.316977ms,max19.118056ms;
- upload avg0.826026ms,max0.850694ms;
- draw avg0.008381ms,max0.013889ms;
- serial avg8.151384ms;
- ~25.215ms nominal frame headroom.

**Meaning:** decoder is nowhere near the real-time limit when supplied properly.

## P10 — 4M quality stress, same P9/q256

Encoder:

- exact same lossless stress content;
- Main/Main;
-608x416;
-30000/1001;
-yuv420p;
-target4,000,000;
-max5,000,000;
-minrate0;
-VBV1,835,008;
-GOP15;
-bf0;
-bytes10,910,594;
-actual avg3.636865Mbit/s;
-SHA `d98c21b50400236d5617b2bf42d612413909b79bff5887f507589060cb39321f`.

PS2 queue still256KiB/prefill224KiB.

User: smooth, quality great.

Telemetry:

- decode avg9.586989ms;
- max451.017379ms;
- feed409/955/max436;
- deadline misses56;
- max late434.505925ms;
- upload/draw essentially unchanged.

**Meaning:** the 451ms “decode” spike was mostly waiting inside the feed callback.
Worst feed span ~436ms almost exactly explains it. 4M exposed another reservoir
boundary, not an IPU ceiling.

## P11 — same exact 4M bitstream, deeper reservoir only

- generator authority `bc36d8f33c567d48500000502572463e847dead6`;
- ELF `5f55462432390535dbf758fc03a8e3b000dae952d7593b6a225658c54ffe654a`;
- exact P10 source;
- queue512KiB;
- prefill448KiB;
- same RGB16, decoder, GS, clock, no-drop, timing instrumentation.

User: looked great, PURPLE→CYAN,last frame.

Telemetry:

- queue highwater522240/524288;
- feed332/348/max5;
- deadline misses39;
- max late~89.805ms;
- decode avg~8.670ms;
- max~42.724ms;
- serial avg~9.504ms;
- ~23.863ms average nominal headroom.

P10→P11:

- worst feed wait436→5 loops;
- decode max451→42.7ms;
- max lateness434.5→89.8ms.

**Meaning:** pathological starvation almost vanished with buffer elasticity. The
network can fill a half-megabyte EE queue, which argues strongly against a hard
IOP→EE throughput ceiling at this operating point.

P11 result documentation later recorded at commit:

`657ca62024e326be82633277615b09d71dff7faf`

## P12 — true-live product-shaped result

P12 reused exact P11 PS2 ELF. Only fixed archive→live X11 producer changed.

Wrapper:

- commit `8c9bca2549c3d99fd7b17030487015d5835f4893`;
- blob `339498b14d4eb83a561a5930408c939038eab88e`.

Actual FFmpeg:

```text
x11grab :0.0 704x462 @ 30000/1001
-> scale=608:416:bicubic
-> yuv420p
-> mpeg2video Main/Main
-> target 4,000,000
-> maxrate 5,000,000
-> minrate 0
-> VBV 1,835,008 bits
-> GOP15
-> bf0
-> 30000/1001 output
```

24-second qualification:

- payload6,588,742 bytes;
- actual average ~2.20Mbit/s over nominal duration;
- coded720→displayed719;
- queue highwater524288/full;
- **feed0/0/0**;
- **deadline misses0**;
- **lateness0**;
- decode avg~6.9723ms;
- decode max~23.6788ms;
- upload~0.8243ms;
- draw~0.0084ms;
- serial avg~7.805ms;
- avg nominal headroom~25.56ms;
- CRC/integrity/end PASS.

User watched live YouTube and called it surprisingly good. Some lulls/catchups may
have been visible but PS2 timing did not record any corresponding starvation or
late presentation. Investigate source/browser/X11/encoder cadence before blaming
the PS2.

P12 hardware result authority:

`a002c3a5c74ba9111b081704768a1b0eafb66994`

---

# 10. Current recommended video operating point

For the next product-shaped experiments, start from the P11/P12 mechanism rather
than older P7/P9 settings.

## Pi video profile

```text
codec: MPEG-2 video
profile: Main
level: Main
pixel format: yuv420p
fps: 30000/1001
current tested full-desktop encoded size: 608x416
scale: bicubic from current 704x462 X11 full desktop
target bitrate: 4,000,000 bit/s
maxrate: 5,000,000 bit/s
minrate: 0
VBV: 1,835,008 bits
GOP: 15
B-frames: 0
```

## PS2 playback

```text
mature SMS libmpeg
_MPEG_Set16(1)
RGB16 / PSMCT16
512 KiB physical compressed queue
448 KiB startup prefill in the qualified P11/P12 harness
absolute 30000/1001 presentation clock
no presentation drops
ordinary TO_IPU DMA
ordinary GIF waits
```

### Critical latency caveat

Keep **512 KiB capacity** as the current evidence-supported burst absorber.
Do **not** assume **448 KiB prefill** is the final product target.

448 KiB represents approximately:

- ~1.0 s at the P10 3.64Mbit/s stress rate;
- ~1.67 s at P12's ~2.20Mbit/s actual average.

That is large for an interactive desktop and future lip sync. The next latency
campaign must separate physical capacity from startup/steady target occupancy.

---

# 11. Response to the “IOP-to-EE bridge is the real bottleneck” hypothesis

Do not treat that claim as established.

Evidence against the strong version:

- O1 network-only acquisition exceeded3.4MiB/s;
- q32768 was already dramatically better than q8192 with no IOP architecture
  change;
- P7A nearly eliminated starvation by increasing an EE compressed reservoir;
- P11's half-megabyte queue nearly filled;
- P12's half-megabyte queue filled completely;
- Pi-side credit waiting proves it frequently has data ready faster than current
  PS2 consumption;
- P12 recorded zero decoder starvation.

The current model is:

> adequate aggregate bandwidth + bursty/variable producer/consumer timing +
> insufficient elasticity in earlier queues.

A custom IOP-side receiver/SIF-DMA engine may be a future optimization only if
new measurements demonstrate a meaningful benefit. It is not the current next
step.

Similarly, do not adopt the unsupported claims that TO_IPU payload lengths must
always be64-byte multiples or that decoding should wait for an arbitrary 4KiB
amount representing a complete macroblock layer. The qualified implementation
already supplies the relevant actual contract.

---

# 12. Product architecture requested at the end of this session

The user wants the MPEG breakthrough integrated toward four coupled goals:

1. **open-ended live streaming** rather than bounded 20/24-second tests;
2. **audio** with correct latency/lip-sync behavior;
3. accelerated MPEG presentation only over a **portion of the desktop** inferred
   from RFB dirty rectangles that appear to represent video, while ordinary RFB
   continues outside that region;
4. explicit user overrides to choose a rectangle or force full-screen video mode.

Do not jump directly to automatic inference + audio + indefinite sessions in one
patch. Build the mechanisms in controlled stages.

---

# 13. Recommended media-session architecture

## 13.1 Pi owns media policy

A Pi-side media coordinator should own:

- session ID/generation;
- mode: OFF/AUTO_REGION/MANUAL_REGION/FULLSCREEN;
- logical video rectangle;
- aligned encoded capture rectangle;
- encoded geometry;
- FFmpeg video lifecycle;
- audio capture lifecycle;
- shared media epoch;
- video target runway;
- audio target runway;
- transport channel scheduling;
- automatic-region detector confidence/hysteresis.

This follows the established “PS2 = mechanism, Pi = policy” contract.

## 13.2 PS2 media subsystem owns bounded mechanisms

PS2 side should own:

- MPEG compressed queue/capacity/credits;
- SMS decoder/IPU lifecycle;
- decoded texture/presentation resources;
- video timestamps/deadline scheduling;
- audio queue/player;
- consumed audio sample counter;
- current media-owned presentation rectangle;
- media telemetry.

Keep RFB protocol ownership in RFB, authoritative desktop pixels in the desktop
framebuffer, and GS presentation mechanisms in display. The application
coordinator orchestrates media-entry/exit but does not absorb their internals.

---

# 14. Open-ended session protocol

Current bounded experiments terminate FFmpeg, append sequence end, send END, and
harvest a result. Product streaming needs explicit session lifetime.

Recommended initial control semantics:

## `MEDIA_BEGIN`

At minimum:

- protocol version;
- media session ID;
- logical desktop `(x,y,w,h)`;
- encoded width/height;
- frame rate/timebase;
- codec/profile identifier;
- shared epoch/timestamp;
- negotiated/declared buffer policy.

Start FFmpeg without `-t`.

While the session is active, an empty compressed queue means wait for more data,
not EOF.

## `MEDIA_END`

Clean stop/reconfigure:

1. stop encoder cleanly;
2. emit qualified B7+12zero suffix if required by raw elementary decoder;
3. send explicit END for the session ID/reason;
4. drain decoder;
5. keep last video texture presented;
6. ensure RFB pixels for region are fresh;
7. atomically return region ownership to RFB;
8. release media resources.

For the first implementation, geometry changes should end/restart the media
session rather than dynamically resizing an active MPEG sequence.

Session IDs are required so stale end/data from one generation cannot affect the
next.

---

# 15. Audio and A/V synchronization

## 15.1 First implementation: separate channels, shared epoch

Do **not** initially mux audio into MPEG Program Stream just to get sync.

Current video decoder expects raw MPEG-2 video elementary data. Existing audio
work already has a separate logical channel and proven 48kHz signed16-bit stereo
PCM semantics. Combining audio into MPEG would simultaneously add:

- demuxing;
- a new audio codec path if compressed;
- new buffering semantics;
- new decoder ownership;
- sync behavior.

That is too many variables.

First integrated A/V design:

- MPEG2 elementary video on MPEG2 logical channel;
- existing raw48kHz s16 stereo PCM on AUDIO;
- CONTROL for session/timestamp/region operations;
- one shared Pi capture epoch.

Compressed audio may be investigated later as an isolated bandwidth optimization.
A future MPEG Program/System Stream remains optional if mature PS2/SMS support
makes it compelling, not a prerequisite.

## 15.2 Audio should initially be master clock

Tag audio blocks with first sample index/time relative to media epoch. PS2 audio
playback can expose consumed sample count:

`media_time = consumed_samples / 48000`

Tag video frames/access units with intended presentation time relative to the
same epoch.

Video scheduler compares video PTS to audio-derived media time plus calibrated
offset:

- early video: hold;
- slightly late: present now;
- grossly obsolete after buffering is healthy: future policy may skip
  **presentation** while still decoding MPEG references;
- routine video jitter must not cause audio to speed/slow arbitrarily.

If audio catastrophically underflows/resets, explicitly rebuffer/re-establish the
shared epoch instead of silently drifting.

## 15.3 Measure latency instead of guessing

Audio path latency:

- Pi capture buffering;
- mux scheduler;
- PS2 audio queue;
- SPU2 buffering/playback.

Video path latency:

- X11 capture;
- FFmpeg encoder delay;
- target compressed runway;
- PS2 queue;
- MPEG decode;
- scheduled presentation.

Whichever path is naturally earlier receives intentional delay. Use a
deterministic flash+beep benchmark to measure offset and long-run drift.

Do not use 448KiB prefill as an implicit permanent audio delay.

---

# 16. RFB dirty-rectangle inferred media region

## 16.1 Detection belongs on Pi

The Pi can observe incoming/forwarded RFB dirty-rectangle metadata and has the
resources to maintain history.

First automatic detector should support one active region and use evidence over
time:

- rectangle frequency;
- stable spatial overlap;
- area;
- repeated changed-pixel density;
- stable bounding-box union;
- persistence;
- exclusion of tiny cursor/blink regions;
- start/stop hysteresis.

Do not infer video from a single dirty rectangle.

Once selected, expand capture bounds outward to 16x16 macroblock alignment and
clamp to desktop. Keep the user/logical display rectangle separately if clipping
back to exact desktop boundaries matters.

Detector must continue receiving **original update metadata** even after RFB
presentation inside the accelerated region is suppressed; otherwise it loses the
signal needed to determine when video motion stops.

## 16.2 Never drop arbitrary RFB bytes

The RFB session must remain synchronized.

The safest first product semantic model:

1. RFB parses every rectangle normally.
2. Validated pixels always update the authoritative CPU-side desktop framebuffer.
3. Display/presentation has one explicit media-owned region.
4. RFB presentation is clipped/suppressed inside that region only.
5. MPEG texture is drawn into that region.
6. Normal RFB presentation continues outside it.
7. PS2-local cursor/OSK/UI are drawn after MPEG and remain visible.

If a future Pi semantic adapter suppresses RFB transmission itself, it must split
or clip **complete rectangle semantics**; never discard arbitrary bytes from an
ordered RFB stream.

## 16.3 Transition into media ownership

- select/infer region;
- start encoder/session;
- build enough compressed runway;
- decode first frame;
- continue showing RFB until first media frame is ready;
- atomically establish media ownership and display first MPEG texture.

No black gap.

## 16.4 Transition back to RFB

- stop media cleanly;
- hold last MPEG frame;
- make sure authoritative RFB pixels for that region are current, requesting a
  refresh if necessary;
- atomically release media ownership and draw RFB region;
- then release last video resources.

Do not expose stale underlying pixels during the handoff.

---

# 17. Manual override requirements

The user explicitly wants manual control as a complement to auto-detection.

Support:

- `AUTO`;
- `MANUAL_RECT`;
- `FULLSCREEN`;
- `OFF/FORCE_RFB`.

A controller hotkey could open a local rectangle-selection overlay. Controller
input adjusts corners/size; UI emits a semantic media-region action. The media
coordinator performs the transition.

Also permit a human-readable configured rectangle where useful. Configuration is
desired policy, while the active region/session remains live runtime state.

Manual mode should override auto inference until explicitly released.

---

# 18. Buffer/latency strategy for product integration

The physical queue and desired working occupancy must be separate values.

Keep 512KiB physical capacity initially because P11/P12 prove it is a valuable
burst absorber.

Then experimentally reduce:

- startup prefill;
- steady-state Pi-side outstanding-byte/queue target;

while holding capacity fixed.

Prefer target runway in time units (milliseconds) when possible. Estimate bytes
from a recent bitrate window. The queue can temporarily rise toward physical
capacity during bursts without making that high-water point the normal latency.

Candidate experimental sequence should be derived from evidence, but examples
might step from448KiB downward through intermediate values while measuring:

- decoder feed waits/max span;
- presentation misses/lateness;
- queue occupancy distribution;
- actual frame age from capture to display;
- operator input responsiveness;
- later A/V offset.

Do not simply reduce physical capacity at the same time; P11 demonstrated why
headroom matters.

---

# 19. Combined transport scheduling

Reuse the dedicated-link mux architecture.

Priority intent:

1. CONTROL/input: tiny, reliable, urgent;
2. AUDIO: regular deadline-sensitive service;
3. MPEG2: bounded real-time bulk, keep desired runway;
4. RFB: reliable bulk outside accelerated region, yields to time-critical media;
5. TELEMETRY: low priority, bounded.

PS2 reports truthful physical queue credit. Pi policy may use less.

Do not let MPEG fill all512KiB merely because it can. Do not let bulk RFB
starve audio. Do not let media starve controller/control.

TCP is still an acceptable backend for the next integration. The logical channel
boundary should remain backend-neutral.

---

# 20. Exact recommended next experimental sequence

Do not combine all final goals at once.

## P13 — indefinite video-only first

Use the exact P12/P11 profile and mechanism:

- P11 PS2 ELF;
- RGB16;
-512KiB physical queue;
- initially same448KiB prefill so session lifetime is the only major variable;
-608x416;
- Main/Main;
-4M target/5M max;
-VBV1,835,008;
-GOP15;
-bf0;
-30000/1001;
- true X11 source;
- absolute clock;
- no drops.

Change only:

- remove24-second FFmpeg duration;
- add explicit open-ended `MEDIA_BEGIN`/controlled stop semantics;
- run for several minutes first;
- explicitly terminate and prove clean decoder/session exit;
- then repeat start/stop cycles;
- then longer soak.

Measure:

- queue occupancy/highwater;
- feed events/loops/max;
- deadline misses/max lateness;
- decode timing distribution;
- session clock drift;
- memory/resource stability;
- restart correctness.

Do not add audio yet in the first indefinite test.

## P14 — latency/capacity separation

Keep512KiB physical capacity and indefinite-session mechanism. Vary only
prefill/steady target. Add timestamp/frame-age measurement if possible.

Goal: preserve P12 smoothness with substantially lower live latency.

## Next — audio integration

Bring in existing audio channel and its known PCM payload. Initially use
fullscreen or a fixed manual video region so A/V timing is not confounded by
auto detection.

## Next — deterministic A/V sync

Flash+beep source; audio sample count as master timeline; measure offset/drift.

## Next — manual rectangle video ownership

Prove region capture/presentation/RFB suppression/restoration and overlays.

## Next — automatic dirty-rectangle inference

Only after manual region ownership is robust.

## Final combined stress

RFB outside region + MPEG region + audio + input/control + local UI/cursor +
repeated transitions.

Only after those are stable should the team seriously spend time on:

- B-frames;
- higher resolution;
- >4M quality targets;
- multiple simultaneous video regions;
- decoder/render overlap/triple buffering;
- alternative transport backend;
- custom IOP/SIF DMA network architecture.

---

# 21. Known unresolved issues / questions

1. **Open-ended lifetime** — P12 is only24s.
2. **Buffer latency** —512KiB capacity is good,448KiB prefill likely too much for
   final interactive latency.
3. **A/V sync** — not yet tested together.
4. **Audio clock implementation** — need consumed sample count/epoch contract.
5. **RGB16 color fidelity** — no dedicated gradient/subtle-tone test.
6. **X11 16-bpp warning** — FFmpeg logs warn that 16bpp screen is “not
   implemented,” despite successful full capture/decode. Investigate whether it
   affects source cadence/quality.
7. **P12 subtle lulls** — not explained by PS2 timing. Likely upstream; use a
   deterministic locally-rendered live animation to isolate browser/YouTube from
   X11/FFmpeg if needed.
8. **EOF returned-picture behavior** — coded-minus-one/two varies by source; keep
   separate from transport integrity.
9. **Actual media PTS** — P6 used known fixed cadence because current SMS
   `current_stream_pts` was not advanced meaningfully. Integrated A/V needs a
   real shared timestamp scheme.
10. **Late-frame presentation drop** — may become useful for A/V sync after
    buffering is healthy, but P7 itself was never qualified.
11. **Automatic region heuristics** — no thresholds qualified yet.
12. **Combined bandwidth/scheduling** — P12 video-only does not prove simultaneous
    AUDIO+MPEG+RFB stability.
13. **Long-term resource leaks/wraparound** — not yet soaked.

---

# 22. Things the successor must not do casually

- Do not mutate the Issue39 qualified product worktree while continuing EXP3.
- Do not overwrite `/mass/0/PS2VNC.ELF`.
- Do not bypass `scripts/testkit/deploy-elf.py` for ordinary experimental deploys.
- Do not use shell heredocs through `ps2run-interactive`.
- Do not reintroduce `dma_wait_fast()`.
- Do not switch from mature SMS to system/modern PS2SDK libmpeg without hardware
  requalification.
- Do not call P7 hardware-qualified; it was never run.
- Do not add frame dropping merely to make starvation look smoother.
- Do not infer EOF from temporary queue empty.
- Do not drop arbitrary RFB bytes to suppress a video region.
- Do not hard-code 448KiB as a product latency target simply because P11/P12 used
  it.
- Do not build a custom IOP/SIF DMA network subsystem unless a new measurement
  demonstrates the current transport itself is the limiting mechanism.
- Do not simultaneously change audio codec, transport, video session semantics,
  and region inference.
- Do not claim P12 sustained 4M average: its **profile target** was4M/5M max, but
  actual live archive averaged about2.2Mbit/s. P11's fixed stress source averaged
  ~3.64Mbit/s and is the stronger bandwidth pressure test.
- Do not treat RED terminal color alone as corruption; inspect the independent
  CYAN integrity result and returned-picture delta.

---

# 23. Successor first-session checklist

A new collaborator should do the following before producing another ELF:

1. `cd /home/ps2/src/PS-to-VNC-media-stream-exp3`.
2. Inspect current branch and `git status` without destroying anything.
3. Fetch `experiment/media-stream-exp3` and understand any commits after this
   handoff.
4. Read `AGENTS.md` and the clean architecture/current project docs.
5. Read:
   - this handoff;
   - `O3_MEDIA_STREAM_OPTIMIZATION_SYNTHESIS_2026-09-07.md`;
   - `O3_LIVE_P12_TRUE_LIVE_4M_HARDWARE_RESULT.md`;
   - `O3_LIVE_PERFORMANCE_P11_DEEP_4M_HARDWARE_RESULT.md`;
   - `O3_P9_RGB16_STAGE_TIMING_HARDWARE_RESULT.md`;
   - `O3_LIVE_PERFORMANCE_P7A_DEEP_BUFFER_HARDWARE_RESULT.md`;
   - audio-transport README at `15a879095cf6bc24df10268b6516bbb39c7a83ce`.
6. Run the repository's applicable resume/check scripts.
7. Confirm the P11 rolling ELF still exists on PS2 mass if reusing it.
8. Confirm Pi X11 and Ethernet link state.
9. Design P13 so **open-ended session lifetime is the only major new variable**.
10. Preserve exact start/stop timing and machine evidence.
11. Ask the operator separately for visual behavior; do not infer it from logs.
12. Record P13 in the repository before moving on.

---

# 24. Why this should influence the clean product architecture

The clean architecture already says:

- RFB owns the protocol session;
- desktop framebuffer owns authoritative remote pixels and dirty regions;
- display owns presentation mechanisms;
- application coordinator owns cross-domain orchestration;
- Pi should own complex policy where possible.

The media breakthrough fits those boundaries naturally if implemented as a new
coherent media responsibility rather than as RFB hacks:

```text
                         application coordinator
                                  |
             +--------------------+--------------------+
             |                    |                    |
          RFB session       media session          local UI/input
             |                    |
     desktop framebuffer     video + audio
             |                    |
             +--------- presentation ownership --------+
                                  |
                              display/GS
```

The authoritative RFB desktop remains complete even when part of it is not
currently being *presented* through RFB. That distinction is the key to safe
regional video acceleration.

The Pi can decide where media acceleration is worthwhile; the PS2 does not need
to understand “this looks like YouTube.” It only needs a bounded region/session
contract and reliable media mechanisms.

---

# 25. Closing state

At the end of this session, the project has not merely demonstrated that the PS2
can decode MPEG-2. It has established an evidence-backed route to a hybrid remote
desktop architecture:

- ordinary RFB for low-motion desktop/UI;
- accelerated MPEG-2 for sustained-motion regions;
- SMS-native RGB16 for efficient PS2 output;
- a large but policy-limited compressed reservoir for burst elasticity;
- absolute/timestamp presentation rather than fixed sleeps;
- audio as a separately scheduled logical channel on the same media timeline;
- explicit presentation-region ownership rather than corrupting/suppressing RFB
  protocol bytes;
- Pi-driven automatic region inference with manual rectangle/fullscreen override;
- eventual audio-master A/V synchronization and measured latency matching.

The highest-value lesson is methodological as well as technical: several early
symptoms looked like hard decoder/network limitations, but controlled A/B tests
showed that **buffering and scheduling architecture** were the real constraints.
Continue the same discipline. Do not sacrifice quality or redesign the transport
around an unmeasured bottleneck when a simpler mechanism has not yet been
isolated.

Preserve this branch, the individual result records, the synthesis, and this
handoff before product integration begins.
