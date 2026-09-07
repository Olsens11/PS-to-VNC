# Media Stream EXP3

## Purpose

This directory contains isolated PlayStation 2 MPEG-2 experiments for the
proposed PS2VNC persistent-media architecture.

The product architecture under investigation is:

- RFB remains the ordinary interactive desktop path.
- Audio remains continuously active.
- MPEG-2 video becomes an optional full-motion visual path.
- Audio and MPEG video will eventually share one media playback timeline.
- The existing dedicated-link mux remains the transport foundation.

Nothing in this directory is product-qualified merely because it builds.
Hardware results are the authority for experimental qualification.

---

# Qualified milestone: SMS MPEG decode and 60-picture presentation

**Hardware PASS: September 7, 2026.**

This is the authoritative starting point for future EXP3 work.

## Qualified code authority

Repository:

`Olsens11/PS-to-VNC`

Branch:

`experiment/media-stream-exp3`

Qualified experiment-code commit:

`202b7d6dddaf5870b0cf231689a1f025e9631f53`

Commit message:

`experiment: play 60 SMS MPEG pictures`

The documentation commit following that code commit changes this README only.
The code authority above remains the hardware-qualified experiment until a
later hardware-qualified increment explicitly supersedes it.

Qualified file identities:

- `experiments/media-stream-exp3/reference_60_picture_playback_smoke.c`
  - SHA-256: `76ae55b24f1f428802fc40f0a019d24c4286d2e41b586198a77b6b30d35bbb41`
- `mk/media-stream-exp3-sms-60-picture-playback.mk`
  - SHA-256: `f41259b27fec37dfc8b16e6a6a056f7245809bc0dac5f116fae43d541356f0b5`
- `experiments/media-stream-exp3/reference_one_picture_display_smoke.c`
  - SHA-256: `f5c726434ed5ec6c14c3b291fb73e4a69e3baeb9fe9cc379307530a3f36e5d5d`
- `mk/media-stream-exp3-sms-one-picture-display.mk`
  - SHA-256: `5737afded3759c4f503fa761989a5b445a1cf7168dd404057efff40add46d2a9`
- `experiments/media-stream-exp3/reference_one_picture_smoke.c`
  - SHA-256: `63c924ba4f0570a1b9a9e1506f01d74effa4180dfbc651bdd7b0d6ec013cec7e`
- `mk/media-stream-exp3-sms-one-picture.mk`
  - SHA-256: `64b71741cd17ecdb3a0884ec38cd8fbe64f04bc406008a26e4a669fe80d9c0b4`

## What is now proven on real PS2 hardware

The following complete local media path has been demonstrated:

```text
embedded raw MPEG-2 elementary stream
        |
        v
2048-byte aligned EE staging buffer
        |
        v
ordinary TO_IPU DMA
        |
        v
SMS libmpeg public decoder
        |
        v
mature SMS assembly MPEG/IPU core
        |
        v
decoded RGBA32 picture
in consecutive 16x16 macroblocks
        |
        v
GIF source-chain REF transfer
        |
        v
GS texture memory
        |
        v
full-screen textured sprite
        |
        v
visible moving MPEG picture
```

The qualified bounded test decoded and displayed exactly **60 pictures**.

Observed terminal sequence:

1. a loader transition may briefly show black with a thin green line;
2. CYAN — graph/display chassis reached;
3. YELLOW — explicit IPU/DMAC quiesce/reset returned;
4. ORANGE — `MPEG_Initialize()` returned;
5. GREEN — the first `MPEG_Picture()` returned nonzero;
6. BLUE — the first macroblock source-chain upload returned;
7. the MPEG test image moved normally through the bounded playback period;
8. LIGHT GRAY — all 60 requested pictures completed;
9. the final decoded picture was redrawn and held.

The final held picture visibly showed:

```text
00:00:01:969
59
```

The test pattern's frame number is zero-based, so the 60th displayed picture
is source frame 59.

For a 30000/1001 frame-rate stream:

```text
59 * 1001 / 30000 = 1.968633... seconds
```

which rounds to `00:00:01:969`.

That agreement is an independent visual confirmation that successive MPEG
pictures were decoded and displayed rather than one decoded buffer being
repeated.

## Qualified scope

This milestone qualifies:

- raw MPEG-2 elementary-stream parsing;
- PS2 TO_IPU DMA feed;
- SMS MPEG decoding on the EE/IPU path;
- first-picture forward progress;
- predictive-frame forward progress for at least 60 pictures;
- decoded RGBA32 macroblock output;
- GS macroblock texture upload;
- GS textured presentation;
- reuse of one decoded-picture buffer with explicit ownership serialization;
- at least 60 consecutive decode/upload/draw cycles.

This milestone does **not** yet qualify:

- MPEG data arriving from the network;
- mux channel 4;
- transport scheduling under simultaneous RFB/audio/MPEG load;
- simultaneous audio playback;
- A/V synchronization;
- PTS-based presentation policy;
- frame dropping;
- long-duration playback;
- entry/exit switching between RFB and MPEG;
- integration into the qualified PS2VNC application runtime.

Do not infer any of those later properties from this local playback result.

---

# Critical findings that must be preserved

## 1. Modern PS2SDK libmpeg is not the qualified decoder

The modern PS2SDK libmpeg control reached `MPEG_Initialize()` but hung inside
the first `MPEG_Picture()` request.

That remained true after aligning the important external contracts with the
PS2SDK sample:

- ordinary aligned decoded-picture pointer;
- 2048-byte input staging blocks;
- `dma_channel_wait(DMA_CHANNEL_toIPU, 0)`;
- `dma_channel_send_normal()` TO_IPU submission;
- the same known-good MPEG stream;
- explicit IPU/DMAC reset;
- no source-chain feed into IPU;
- no `dma_wait_fast()` in the decoder path.

The same external harness, stream, feed, and picture-buffer contract succeeded
when the decoder implementation alone was replaced by the pinned mature SMS
assembly implementation.

Therefore the observed first-picture failure tracks with the modern PS2SDK
libmpeg implementation/build lineage rather than those shared external
contracts.

A useful source-history fact is that PS2SDK replaced its old assembly
`libmpeg_core.s` with a C core in 2025. The hardware A/B does not by itself
prove the exact defect inside that rewrite, but it provides strong evidence
that EXP3 should use the mature SMS implementation unless a later isolated
experiment repairs and requalifies modern PS2SDK libmpeg.

## 2. `dma_wait_fast()` is prohibited in this EXP3 path

A dedicated hardware bisection proved that `dma_wait_fast()` can hang before
the first visible graphics marker in this environment.

The qualified MPEG path therefore uses ordinary channel waits such as:

```c
dma_channel_wait(DMA_CHANNEL_GIF, 0);
dma_channel_wait(DMA_CHANNEL_toIPU, 0);
```

Do not reintroduce `dma_wait_fast()` as an optimization without a new isolated
hardware proof.

## 3. The libmpeg picture pointer is an ordinary aligned EE pointer

The open PS2SDK/SMS libmpeg contract uses an ordinary aligned pointer for
decoded picture storage.

An earlier EXP3 revision incorrectly imported an uncached-pointer convention
from Sony's different `sceMpeg` API. Correcting that pointer did not fix the
modern PS2SDK decoder hang, but the ordinary aligned pointer remains the
correct contract and is used by the qualified SMS path.

## 4. The input callback uses the canonical 2048-byte normal-DMA model

The qualified feed path uses:

- a 64-byte-aligned staging buffer;
- 2048-byte normal blocks;
- a final short block rounded only to the required 16-byte DMA boundary;
- `dma_channel_wait(DMA_CHANNEL_toIPU, 0)` before submission;
- `dma_channel_send_normal()` for TO_IPU DMA;
- no source-chain submission to IPU.

This reproduces the relevant PS2SDK sample ownership model while avoiding the
known-bad fast-wait helper.

## 5. GIF ownership is serialized before the decoder can reuse the picture buffer

The bounded playback test intentionally uses one decoded-picture buffer.

For each picture, the control flow is serialized:

```text
MPEG_Picture
-> GIF source-chain texture upload
-> ordinary GIF-channel wait
-> textured draw
-> ordinary GIF-channel wait
-> VSYNC cadence
-> next MPEG_Picture
```

The GIF source-chain REF tags point directly at the EE picture buffer.
Therefore the ordinary GIF-channel wait after the upload is the ownership
boundary: only after that wait returns may the decoder overwrite the buffer
for the next picture.

## 6. SMS must be built with its historical small-data model

Pinned SMS repository:

`ps2homebrew/SMS`

Pinned SMS commit:

`c1898094725ad750ec20e10cc148b39d7c8a9c65`

Required decoder-object flags:

```text
-G8192
-mgpopt
-mno-abicalls
```

The first attempted SMS build inherited `-G0`. The modern assembler then
expanded pseudo-instructions inside MIPS branch delay slots, so that ELF was
rejected without hardware testing.

`-G8192 -mgpopt` matches SMS's historical small-data model. With the current
compiler, `-mno-abicalls` must also be explicit because `-G` small-data access
cannot coexist with ABICALLS mode.

Qualified build diagnostics:

- ABICALL/small-data errors: `0`;
- multi-instruction macro-expansion warnings: `0`;
- branch-delay-slot macro-expansion warnings: `0`;
- `$at` diagnostics from the historical SMS assembly source: `213`;
- mixed ABICALL/non-ABICALL static-link diagnostics: `3`;
- final MPEG symbols resolved;
- SMS `.sdata` and `.sbss` sections present.

The `$at` diagnostics are retained as evidence. Do not edit the byte-pinned
upstream assembly merely to silence them.

---

# Exact source authorities

## Vendored SMS decoder blobs

The vendored decoder files must remain byte-identical to pinned SMS commit
`c1898094725ad750ec20e10cc148b39d7c8a9c65`.

```text
experiments/media-stream-exp3/vendor/sms-libmpeg/include/libmpeg.h
Git blob: ee2195b52dc3a7112537046426a80aa0e603fa6c

experiments/media-stream-exp3/vendor/sms-libmpeg/include/libmpeg_internal.h
Git blob: c2f80a9104380634c3ef6749baa2e5f0acb5c13e

experiments/media-stream-exp3/vendor/sms-libmpeg/src/libmpeg.c
Git blob: f9e5f11689fa6ed3759365249c2d7cfb7335e2fb

experiments/media-stream-exp3/vendor/sms-libmpeg/src/libmpeg_core.S
Git blob: 93638fd62e58bfac8c6ed1c5fc84ef119d318438
```

Do not reformat or whitespace-clean those files.

The vendor directory also contains `UPSTREAM.txt` recording the pinned SMS
authority.

## Current PS2SDK reference authority

PS2SDK source commit used for source-level investigation:

`3e46a44a21db66f42a56320b6f64bfc52ea279f8`

Current system libmpeg archive in the pinned image:

```text
/usr/local/ps2dev/ps2sdk/ee/lib/libmpeg.a
SHA-256:
7b1447183eef75eb9cf896a429c61e2b8439deaa65aec531764952afa55417ee
```

Current libmpeg header:

```text
/usr/local/ps2dev/ps2sdk/ee/include/libmpeg.h
SHA-256:
01ecd96deb1fa49491d59d7c034ad692dc09fb96ff0471328f89825647d2568e
```

The qualified SMS targets deliberately do **not** link system `-lmpeg`.

---

# Exact MPEG fixture

Hardware-qualified fixture:

```text
SHA-256:
5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f

bytes:
8815372

codec:
MPEG-2 video, Main Profile

dimensions:
704x480

pixel format:
yuv420p

level:
8

nominal/r_frame_rate:
30000/1001

known picture count from earlier inspection:
599
```

An `ffprobe` check on the qualified host reported:

```text
codec_name=mpeg2video
profile=Main
width=704
height=480
pix_fmt=yuv420p
level=8
r_frame_rate=30000/1001
avg_frame_rate=25/1
nb_frames=N/A
```

The `r_frame_rate` matches the visible frame/time relationship observed on
hardware. The `avg_frame_rate` report is not used as the qualified playback
cadence authority for this fixture.

## Fixture-provenance limitation

The exact command or source provenance originally used to create this
`test.bin` was **not captured in EXP3 repository history**.

Therefore a bit-for-bit reproduction currently requires possession of the
exact binary fixture identified above.

Do not generate a new 704x480 MPEG-2 test stream and silently treat it as the
same input. Encoder version, GOP structure, quantization, headers, and other
bitstream details can differ even when the visible image looks equivalent.

Until the original generation recipe is recovered or a new deterministic
fixture-generation procedure is intentionally qualified, the fixture
SHA-256 above is part of the experimental authority.

This is the known fresh-clone reproducibility gap for this milestone.

---

# Pinned build environment

Docker image:

`ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Resolved Docker image ID:

`sha256:f4ec5e8dfd748f2e451731c5da73660fce6549a8cfda7fc2e494b588db17ca1c`

Compiler:

`mips64r5900el-ps2-elf-gcc (GCC) 15.2.0`

Pinned PATH:

```text
/usr/local/ps2dev/bin
/usr/local/ps2dev/ee/bin
/usr/local/ps2dev/iop/bin
/usr/local/ps2dev/dvp/bin
/usr/local/ps2dev/ps2sdk/bin
/usr/local/sbin
/usr/local/bin
/usr/sbin
/usr/bin
/sbin
/bin
```

The container installs only the host build prerequisites required by the
existing project build procedure:

```sh
apk add --no-cache bash build-base
```

---

# Hardware/environment used for the qualified test

Development host:

`ps-to-vnc-dev`

EXP3 worktree used during qualification:

`/home/ps2/src/PS-to-VNC-media-stream-exp3`

PS2 dedicated-link address used by deployment:

`192.168.50.2`

Pi dedicated-link address:

`192.168.50.1`

PS2 launch environment:

- FreeMcBoot;
- wLaunchELF 4.43x_ISR / file manager;
- USB mass storage exposed as `mass:/0/`;
- canonical testkit deployment over PS2 FTP.

An initial black screen with a thin green line may appear while an ELF is
launched. The same visual transition was also observed when FreeMcBoot
launched wLaunchELF itself. It occurs before EXP3's first CYAN application
marker and is not part of the MPEG stage contract.

---

# Reproduction procedure: qualified 60-picture milestone

The goal of this procedure is to reproduce the **qualified local
MPEG decode/presentation experiment**, not the future network architecture.

## Step 1: check out exact qualified experiment code

Use:

`202b7d6dddaf5870b0cf231689a1f025e9631f53`

Example:

```sh
git fetch origin
git checkout --detach 202b7d6dddaf5870b0cf231689a1f025e9631f53
git status --short
```

`git status --short` must be empty before building.

A documentation-only descendant may also be used if comparison proves that
the qualified experiment source, Makefiles, and vendored SMS files listed in
this README are bit-identical to the code authority above.

## Step 2: provide and verify the exact MPEG fixture

Place the exact fixture at:

`build/experiments/media-stream-exp3/embedded-libmpeg-smoke/test.bin`

Verify:

```sh
sha256sum build/experiments/media-stream-exp3/embedded-libmpeg-smoke/test.bin
stat -c '%s' build/experiments/media-stream-exp3/embedded-libmpeg-smoke/test.bin
```

Required values:

```text
SHA-256 = 5f207fca420c15794cfebb18e858c0cd8aa8f7c632c901d2dfe60696d1997f3f
bytes   = 8815372
```

Fail closed on either mismatch.

## Step 3: verify pinned SMS source

Run:

```sh
git hash-object experiments/media-stream-exp3/vendor/sms-libmpeg/include/libmpeg.h
git hash-object experiments/media-stream-exp3/vendor/sms-libmpeg/include/libmpeg_internal.h
git hash-object experiments/media-stream-exp3/vendor/sms-libmpeg/src/libmpeg.c
git hash-object experiments/media-stream-exp3/vendor/sms-libmpeg/src/libmpeg_core.S
```

Required values, in the same order:

```text
ee2195b52dc3a7112537046426a80aa0e603fa6c
c2f80a9104380634c3ef6749baa2e5f0acb5c13e
f9e5f11689fa6ed3759365249c2d7cfb7335e2fb
93638fd62e58bfac8c6ed1c5fc84ef119d318438
```

## Step 4: stage the fixture into the playback build directory

```sh
BUILD=build/experiments/media-stream-exp3/sms-60-picture-playback

rm -rf "$BUILD"
mkdir -p "$BUILD"

cp \
  build/experiments/media-stream-exp3/embedded-libmpeg-smoke/test.bin \
  "$BUILD/test.bin"

sha256sum "$BUILD/test.bin"
```

The copied fixture must retain the qualified SHA-256.

## Step 5: build in the pinned container

From the repository root:

```sh
docker run --rm \
  --entrypoint /bin/sh \
  -v "$PWD:/repo" \
  -w /repo \
  ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11 \
  -lc '
    set -eu

    apk add --no-cache bash build-base >/dev/null

    export PATH="/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

    make \
      -f mk/media-stream-exp3-sms-60-picture-playback.mk \
      all
  '
```

Output ELF:

`build/experiments/media-stream-exp3/sms-60-picture-playback/PS2VNC-EXP3-SMS-60Picture-Playback.ELF`

Qualified hardware-tested ELF identity:

```text
SHA-256:
5b6cb3062e69fd9156d11a7f4a803664b093ed04d30b0d3d5ddbf76de8fd2298

bytes:
10251512
```

If a supposedly identical rebuild produces a different ELF, stop and identify
the changed authority before hardware testing.

## Step 6: inspect build diagnostics

The SMS decoder objects must visibly compile with:

```text
-G8192 -mgpopt -mno-abicalls
```

The harness remains on the ordinary project/PS2SDK sample flags.

Required diagnostics from the qualified build:

```text
ABICALL/small-data errors                   = 0
multi-instruction macro-expansion warnings = 0
branch-delay macro-expansion warnings      = 0
$at warnings                                = 213
mixed ABI static-link warnings              = 3
```

System `-lmpeg` must **not** be linked by the SMS target.

Useful symbol checks:

```sh
mips64r5900el-ps2-elf-nm \
  build/experiments/media-stream-exp3/sms-60-picture-playback/sms_libmpeg.o \
  | grep ' T MPEG_Initialize'

mips64r5900el-ps2-elf-nm \
  build/experiments/media-stream-exp3/sms-60-picture-playback/sms_libmpeg_core.o \
  | grep ' T _MPEG_Initialize'

mips64r5900el-ps2-elf-nm \
  build/experiments/media-stream-exp3/sms-60-picture-playback/sms_libmpeg_core.o \
  | grep ' T _MPEG_CSCImage'
```

The final ELF must resolve its MPEG implementation from the vendored SMS
objects.

## Step 7: canonical deployment

Do not manually reconstruct FTP deployment.

Dry run:

```sh
python3 \
  scripts/testkit/deploy-elf.py \
  --repo "$PWD" \
  --elf \
    build/experiments/media-stream-exp3/sms-60-picture-playback/PS2VNC-EXP3-SMS-60Picture-Playback.ELF \
  --test-id exp3-sms-core-60-picture-playback \
  --expected-sha256 5b6cb3062e69fd9156d11a7f4a803664b093ed04d30b0d3d5ddbf76de8fd2298 \
  --expected-bytes 10251512 \
  --rolling-path \
    /mass/0/PS2VNC-exp3-sms-60-picture-playback-current.ELF \
  --dry-run
```

After the dry run passes, repeat with:

`--operator-authorized`

Qualified archival path:

`/mass/0/PS2VNC-exp3-sms-core-60-picture-playback-5b6cb306.ELF`

Qualified deployment evidence file:

`build/testkit/deployments/exp3-sms-core-60-picture-playback-5b6cb306.json`

Deployment evidence SHA-256:

`9f62cf7312517bf1c538b6d6cbb1e5a41a77fee1e32bf0a48e25e52bf12a1faa`

Never overwrite the product baseline:

`/mass/0/PS2VNC.ELF`

## Step 8: launch the archival ELF

From wLaunchELF, launch the archival 60-picture ELF from `mass:/0/`.

Expected visible sequence:

```text
CYAN
YELLOW
ORANGE
GREEN
BLUE
moving MPEG test pattern
LIGHT GRAY
final picture held
```

Stage meanings:

```text
CYAN
  graphics/display chassis is alive

YELLOW
  explicit IPU/DMAC reset returned

ORANGE
  MPEG_Initialize returned

GREEN
  first MPEG_Picture returned nonzero

BLUE
  first GIF macroblock source-chain upload returned

MOVING PICTURE
  repeated bounded decode/upload/draw is making progress

LIGHT GRAY
  all 60 pictures completed

FINAL HELD PICTURE
  picture 60 was uploaded/drawn again after the completion marker
```

## Step 9: hardware acceptance criteria

PASS requires all of the following:

- no RED early-decoder-return state;
- no persistent ORANGE first-picture hang;
- no persistent GREEN first-upload hang;
- coherent moving MPEG test-pattern content;
- visible time/frame progression;
- LIGHT GRAY completion marker;
- final picture redraw after the gray marker.

Qualified observation:

```text
final visible time = 00:00:01:969
source frame       = 59
requested pictures = 60
```

This is the current hardware acceptance result.

---

# Chronological EXP3 bisection ledger

This ledger exists so future debugging does not accidentally repeat already
cleared hypotheses.

| Step | Commit | Experiment | Hardware result | Conclusion |
|---|---|---|---|---|
| Original | pre-bisection | full embedded system-libmpeg smoke | black | contaminated because `dma_wait_fast()` executed before the first visible marker |
| A | `f4e0dac1fd28d8c283460886ec5561f9ea48a7a5` | graph-only | BLUE persistent | ELF launch plus graph/display chassis pass |
| B | `409f2ad785ba3ff9d92a6880849695b762dfee78` | exact embedded payload, no MPEG | MAGENTA persistent | ~9 MiB ELF, embedded fixture, and EE access pass |
| C | `cc3a8a423fe51e7e5670864a36822a9a94af5fe0` | TO_IPU DMA initialization | YELLOW persistent | IPU DMA-channel initialization itself passes |
| D | `7ce45fbeef93d42b2abb83a4d3b2086a59f5b09a` | forced system-libmpeg link only | CYAN persistent | linking libmpeg code/ABI footprint does not itself fail |
| E | `c009bf571603e9ac7250bffafbe47f0b646e780f` | `MPEG_Initialize()` probe | black / transient green line | invalid boundary because `dma_wait_fast()` still preceded the visible result |
| F | `de2fdbc53256af8c4536610685c42046efb875bb` | fast-wait-only probe | BLACK indefinitely | `dma_wait_fast()` itself is a hardware-proven hang source here |
| G | `6ea2ea17686be29f9b57f33a55b6372b339c227b` | reference one-picture system-libmpeg probe | CYAN -> YELLOW -> ORANGE forever | `MPEG_Initialize()` returns; first `MPEG_Picture()` does not |
| H | `c932215eaded06babaff68e3362592f0ed456d8f` | normal aligned picture pointer | ORANGE forever | uncached picture-pointer mistake was not the modern decoder blocker |
| I | `7eca3a09e229fd8f615c97b319e0190c9b0fdba3` | canonical PS2SDK 2048-byte input feed | ORANGE forever | custom feed also cleared as blocker |
| J | `ab88f0432edc0d343fb116bebd84f86cc770a641` | pinned mature SMS assembly decoder A/B | GREEN persistent | first picture decodes when decoder implementation changes |
| K | `ad438fb9ba139b77c7f7c2f1da7b09b2c8b23451` | one-picture SMS GS presentation | coherent decoded test image | complete one-picture decode -> GS path passes |
| L | `202b7d6dddaf5870b0cf231689a1f025e9631f53` | 60-picture bounded SMS playback | moving test image -> GRAY -> final `00:00:01:969 / 59` frame | repeated decode/upload/draw passes for 60 pictures |

## Important historical ELF/result identities

Reference-derived first-picture system-libmpeg probe:

- commit: `6ea2ea17686be29f9b57f33a55b6372b339c227b`
- ELF SHA-256: `e99085a9087207d04e3fcaaed94cfb429d5a7298b47d098d5f395f9d80b51d60`
- bytes: `9028116`

Normal-aligned-pointer control:

- commit: `c932215eaded06babaff68e3362592f0ed456d8f`
- ELF SHA-256: `875446d2ef9715dff442bbb5b150d414e09484dfaa5e05899d57ccf8e7b55982`
- bytes: `9027988`

Canonical PS2SDK-feed control:

- commit: `7eca3a09e229fd8f615c97b319e0190c9b0fdba3`
- ELF SHA-256: `beb979c29ae84fb4c0bf81d5a5a27cbe2e4761a67089a1e41fcfcf668939e884`
- bytes: `9027988`

Rejected SMS `-G0` preflight:

- ELF SHA-256: `19bb512dbab3b0741d18ff4e57b98b5a35aeed5fa0045adfb9b7619daec67168`
- bytes: `9022116`
- not deployed or hardware-tested

Qualified SMS assembly-core first-picture decode:

- commit: `ab88f0432edc0d343fb116bebd84f86cc770a641`
- ELF SHA-256: `9d3c5726fb3310851cea539115c4db324ad229a08c6701cbed9fc5cbc6e3b56f`
- bytes: `10241688`
- archival path: `/mass/0/PS2VNC-exp3-sms-core-one-picture-9d3c5726.ELF`

Qualified 60-picture playback:

- code commit: `202b7d6dddaf5870b0cf231689a1f025e9631f53`
- ELF SHA-256: `5b6cb3062e69fd9156d11a7f4a803664b093ed04d30b0d3d5ddbf76de8fd2298`
- bytes: `10251512`
- archival path: `/mass/0/PS2VNC-exp3-sms-core-60-picture-playback-5b6cb306.ELF`

Where an early bisection ELF is referenced only by a shortened SHA in old
session logs, recover the full identity from archived test/deployment evidence
rather than guessing missing digits.

---

# Detailed hardware progression

## Reference-derived one-picture system-libmpeg result

Authority:

- commit: `6ea2ea17686be29f9b57f33a55b6372b339c227b`
- ELF SHA-256: `e99085a9087207d04e3fcaaed94cfb429d5a7298b47d098d5f395f9d80b51d60`

Observed sequence:

`CYAN -> YELLOW -> ORANGE -> ORANGE indefinitely`

Interpretation:

- graph chassis passed;
- explicit IPU/DMAC reset returned;
- `MPEG_Initialize()` returned;
- the first `MPEG_Picture()` operation did not return.

That revision returned an uncached `0x2xxxxxxx` alias from the libmpeg sequence
callback. This was corrected because PS2SDK/SMS open libmpeg expects an
ordinary aligned EE pointer.

## Normal aligned output-buffer result

Authority:

- commit: `c932215eaded06babaff68e3362592f0ed456d8f`
- ELF SHA-256: `875446d2ef9715dff442bbb5b150d414e09484dfaa5e05899d57ccf8e7b55982`

Observed sequence:

`CYAN -> YELLOW -> ORANGE -> ORANGE indefinitely`

Changing only the output-buffer addressing contract did not remove the
first-picture hang.

## Canonical PS2SDK-feed one-picture result

Authority:

- commit: `7eca3a09e229fd8f615c97b319e0190c9b0fdba3`
- ELF SHA-256: `beb979c29ae84fb4c0bf81d5a5a27cbe2e4761a67089a1e41fcfcf668939e884`

This revision used:

- 2048-byte staging blocks;
- `dma_channel_wait(DMA_CHANNEL_toIPU, 0)`;
- `dma_channel_send_normal()`;
- ordinary aligned decoded-picture pointer;
- no `dma_wait_fast()` in the decoder path.

Observed result:

ORANGE indefinitely, with no GREEN first-picture success and no RED clean
return.

At this point both major external contracts had been aligned with the PS2SDK
sample, so the next experiment changed only the decoder implementation.

## SMS build preflight

The pinned mature SMS decoder was compiled without modifying upstream source.

The initial `-G0` build was rejected because the modern assembler expanded
pseudo-instructions inside branch delay slots.

The corrected build used:

`-G8192 -mgpopt -mno-abicalls`

Successful preflight identity:

- ELF SHA-256: `9d3c5726fb3310851cea539115c4db324ad229a08c6701cbed9fc5cbc6e3b56f`
- bytes: `10241688`

Preflight diagnostics:

- ABICALL/small-data errors: 0;
- multi-instruction macro expansions: 0;
- branch-delay-slot macro expansions: 0;
- `$at` diagnostics: 213;
- mixed ABI static-link diagnostics: 3;
- final SMS MPEG symbols resolved;
- `.sdata` / `.sbss` present.

## SMS first-picture hardware result: PASS

Authority:

- commit: `ab88f0432edc0d343fb116bebd84f86cc770a641`
- ELF SHA-256: `9d3c5726fb3310851cea539115c4db324ad229a08c6701cbed9fc5cbc6e3b56f`

Observed sequence:

`CYAN -> YELLOW -> ORANGE -> GREEN`

GREEN is reached only after `MPEG_Picture()` returns nonzero. The test then
intentionally sleeps.

Result:

`SMS_FIRST_MPEG_PICTURE_DECODE=PASS`

This was the first hardware-proven complete MPEG-2 picture decode in EXP3.

## SMS first-picture GS presentation: PASS

Authority:

- commit: `ad438fb9ba139b77c7f7c2f1da7b09b2c8b23451`
- display source SHA-256: `f5c726434ed5ec6c14c3b291fb73e4a69e3baeb9fe9cc379307530a3f36e5d5d`

Observed sequence:

`CYAN -> YELLOW -> ORANGE -> GREEN -> BLUE -> coherent picture`

The visible frame contained:

- vertical red, green, yellow, blue, purple, and cyan columns;
- a black timecode area around `00:00:00:000`;
- a diagonal rainbow-gradient stripe;
- a diagonal series of cyan squares;
- a textured checkerboard pattern in the lower-right area;
- a small vertical gray rectangle near the lower part of the green field.

The image remained spatially coherent across many 16x16 macroblock
boundaries.

Result:

`SMS_FIRST_PICTURE_GS_PRESENTATION=PASS`

This proved the complete one-picture path from MPEG input through visible GS
output.

## SMS 60-picture bounded playback: PASS

Authority:

- commit: `202b7d6dddaf5870b0cf231689a1f025e9631f53`
- ELF SHA-256: `5b6cb3062e69fd9156d11a7f4a803664b093ed04d30b0d3d5ddbf76de8fd2298`
- bytes: `10251512`

The source retains the qualified one-frame path and changes only bounded
repetition for a total of 60 successful pictures.

Observed hardware result:

- normal startup stages completed;
- the test pattern moved coherently;
- the test reached the LIGHT GRAY completion marker;
- the final picture was redrawn and held;
- final visible timecode: `00:00:01:969`;
- final visible source-frame number: `59`.

Result:

`EXP3_SMS_60_PICTURE_PLAYBACK=PASS`

This qualifies repeated MPEG decode/upload/draw progress across predictive
frames for the bounded 60-picture test.

---

# Reference hierarchy

When evidence conflicts, use this order:

1. hardware results in this EXP3 ledger;
2. pinned mature SMS implementation and its original integration behavior;
3. current PS2SDK libmpeg internals for the API contract being investigated;
4. PS2SDK MPEG sample for integration examples;
5. public retail-game reconstruction only as corroborating behavioral evidence.

Sony `sceMpeg` behavior must not be assumed to define the PS2SDK/SMS open
libmpeg contract.

---

# Rules for future EXP3 increments

Before changing this qualified path:

1. preserve commit `202b7d6dddaf5870b0cf231689a1f025e9631f53` as the qualified code authority;
2. record each new hardware result before creating the next source variant;
3. change one experimental variable at a time;
4. keep the SMS decoder blobs byte-identical unless the experiment explicitly tests a decoder modification;
5. do not reintroduce `dma_wait_fast()`;
6. do not silently change the MPEG fixture;
7. preserve exact build SHA/byte identity before deployment;
8. use canonical testkit deployment;
9. record visible hardware result and telemetry for every test;
10. do not infer network/audio/A-V qualification from this local playback result;
11. preserve qualified source variants instead of overwriting them;
12. if an experimental build is contaminated by another variable, mark it invalid rather than interpreting it as decoder evidence.

---

# Historical initial smoke: superseded

The earliest EXP3 idea was a standalone system-PS2SDK-libmpeg smoke test using
`libmpeg_embedded_smoke.c` with a target of at least 550 decoded pictures from
the approximately 599-picture embedded stream.

That test used the old PS2SDK sample's `dma_channel_fast_waits()` /
`dma_wait_fast()` pattern. Later bisection proved that `dma_wait_fast()` itself
hangs in this environment before the first useful visible boundary.

Therefore the old 550-picture acceptance contract is **superseded** and must
not be used as the current qualification procedure.

The historical source remains useful because it records the original direction
and the sample-derived presentation structure, but the qualified implementation
is the SMS path documented above.
