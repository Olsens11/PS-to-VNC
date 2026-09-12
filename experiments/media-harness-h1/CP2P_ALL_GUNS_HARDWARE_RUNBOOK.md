# CP2P all-guns hardware qualification runbook

## Purpose and authority

This is the operator/evidence runbook for checklist item #12: physical qualification of the exact CP2P all-guns software candidate.

The source candidate itself is intentionally frozen and is **not** modified by this runbook:

- candidate branch: `qualification/h1-cp2p-all-guns-hw1`
- sealed candidate HEAD: `4c2d08ea28fe5c1d462f078ae688a24515914d25`
- item-#10 product commit beneath the documentation seal: `6c24fddbf4618d2e0bf69eba909be640dcff005f`
- clean committed-source proof run: `34718230541`
- pinned PS2DEV image: `ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11`

Expected exact PS2 candidate identity:

- ELF: `PS2VNC-H1-CP2P-ApplicationLink.ELF`
- SHA256: `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`
- bytes: `3238996`
- PT_LOAD segments: `1`
- PT_LOAD SHA256: `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`
- PT_LOAD bytes: `512276`

The original proof artifact is `h1-cp2p-item10-all-guns-unqualified-elf`, artifact ID `10305431287`. It remains explicitly unqualified until this hardware gate is completed.

## Qualification invariants

A run is invalid rather than merely failed if any of these identities drift:

- source commit differs from the sealed candidate HEAD;
- candidate worktree is dirty;
- PS2DEV image differs from the pinned digest;
- rebuilt ELF SHA/size differs;
- PT_LOAD SHA/size differs;
- deployment readback does not match the exact ELF;
- a different Pi runner/profile is used.

The active Pi composition is the repository's real CP2P runner:

`experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`

with profile:

`CP2P_ALL_GUNS`

That profile is installed by the CP2P runner itself from the canonical P11+PCM profile and changes only the CP2P all-guns composition: profile id 3, visible RFB mode 2, MPEG2 video enabled, canonical PCM enabled, and the already-proven RFB queue/credit fields. CONFIG authorizes capability only; calibration acceptance and START remain the MPEG generation/producer authority.

## Apparatus expectations

Before the packet:

1. PS2 and Pi are on and connected by the normal direct Ethernet path.
2. PS2 is in the usual wLaunchELF/PS2Net FTP-ready state so `192.168.50.2:21` returns `220 ps2ftpd ready` for deployment.
3. Pi demand-ready RFB endpoint is available at `127.0.0.1:5903` through the existing `ps-to-vnc-rfb.socket` / `ps-to-vnc-rfb-tigervnc.service` path.
4. The Pi desktop has a harmless focused text target available for the keyboard/OSK test.
5. A moving visual source and audible audio are available on the Pi desktop. A playing browser video is ideal because the same session can exercise MPEG motion, PCM, and high-change RFB outside the calibrated region.
6. No other H1 PSTV listener is using TCP port 5902.

An external Windows VNC observer may be used to arrange/observe the Pi desktop. It is not a PS2-facing PSTV stream and does not alter the one-PSTV-socket invariant.

## Canonical preflight, build, identity, deployment, and run packet

Paste the complete block into `ps2run-interactive`. The first line is the clipboard-safety guard.

```sh
exit # PS2RUN_INTERACTIVE_ONLY
set -eu

BASE="$HOME/src/PS-to-VNC"
WORK="$HOME/src/PS-to-VNC-h1-cp2p-hw1-4c2d08e"
COMMIT="4c2d08ea28fe5c1d462f078ae688a24515914d25"
BRANCH="qualification/h1-cp2p-all-guns-hw1"
REMOTE="origin"
IMAGE="ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11"
EXPECTED_ELF_SHA="89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad"
EXPECTED_ELF_BYTES="3238996"
EXPECTED_PT_LOAD_SHA="4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494"
EXPECTED_PT_LOAD_BYTES="512276"
BUILD_DIR="build/experiments/media-harness-h1-cp2p-application-link/ps2"
ELF_REL="$BUILD_DIR/PS2VNC-H1-CP2P-ApplicationLink.ELF"
RUNNER="experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py"

printf '%s\n' '===== CP2P HW1 EXACT-SOURCE PREFLIGHT ====='
git -C "$BASE" fetch "$REMOTE" "$BRANCH"
git -C "$BASE" cat-file -e "$COMMIT^{commit}"
REMOTE_HEAD="$(git -C "$BASE" rev-parse FETCH_HEAD)"
[ "$REMOTE_HEAD" = "$COMMIT" ] || {
    echo "CP2P_HW1_REMOTE_HEAD=FAIL actual=$REMOTE_HEAD expected=$COMMIT"
    exit 1
}
printf 'CP2P_HW1_REMOTE_HEAD=%s\n' "$REMOTE_HEAD"

if [ -e "$WORK" ]; then
    ACTUAL_WORK_HEAD="$(git -C "$WORK" rev-parse HEAD)"
    [ "$ACTUAL_WORK_HEAD" = "$COMMIT" ] || {
        echo "CP2P_HW1_WORKTREE_HEAD=FAIL actual=$ACTUAL_WORK_HEAD expected=$COMMIT"
        exit 1
    }
    [ -z "$(git -C "$WORK" status --porcelain=v1)" ] || {
        echo 'CP2P_HW1_WORKTREE_CLEAN=FAIL'
        git -C "$WORK" status --short
        exit 1
    }
else
    git -C "$BASE" worktree add --detach "$WORK" "$COMMIT"
fi

printf 'CP2P_HW1_WORKTREE=%s\n' "$WORK"
printf 'CP2P_HW1_WORKTREE_HEAD=%s\n' "$(git -C "$WORK" rev-parse HEAD)"
printf 'CP2P_HW1_WORKTREE_CLEAN=PASS\n'

printf '%s\n' '===== PINNED CP2P HW1 CONTAINER BUILD ====='
docker run --rm \
    --entrypoint /bin/sh \
    -v "$WORK:/repo" \
    -w /repo \
    "$IMAGE" \
    -lc '
        set -eu
        apk add --no-cache bash binutils make python3 >/dev/null
        export PATH=/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
        test -f /usr/local/ps2dev/ps2sdk/samples/Makefile.pref
        rm -rf build/experiments/media-harness-h1-cp2p-application-link/ps2
        make -f mk/media-harness-h1-cp2p-application-link.mk cp2p-application-link-check
    '

ELF="$WORK/$ELF_REL"
ACTUAL_ELF_SHA="$(sha256sum "$ELF" | awk '{print $1}')"
ACTUAL_ELF_BYTES="$(stat -c '%s' "$ELF")"
printf 'CP2P_HW1_LOCAL_ELF_SHA256=%s\n' "$ACTUAL_ELF_SHA"
printf 'CP2P_HW1_LOCAL_ELF_BYTES=%s\n' "$ACTUAL_ELF_BYTES"
[ "$ACTUAL_ELF_SHA" = "$EXPECTED_ELF_SHA" ] || {
    echo 'CP2P_HW1_LOCAL_ELF_IDENTITY=FAIL_SHA256'
    exit 1
}
[ "$ACTUAL_ELF_BYTES" = "$EXPECTED_ELF_BYTES" ] || {
    echo 'CP2P_HW1_LOCAL_ELF_IDENTITY=FAIL_BYTES'
    exit 1
}
printf 'CP2P_HW1_LOCAL_ELF_IDENTITY=PASS\n'

PT_LOAD_OUT="$(
    docker run --rm \
        --entrypoint /bin/sh \
        -v "$WORK:/repo:ro" \
        -w /repo \
        "$IMAGE" \
        -lc '
            set -eu
            apk add --no-cache bash binutils python3 >/dev/null
            ./scripts/testkit/pt-load-fingerprint.sh \
                build/experiments/media-harness-h1-cp2p-application-link/ps2/PS2VNC-H1-CP2P-ApplicationLink.ELF
        '
)"
printf '%s\n' "$PT_LOAD_OUT"
printf '%s\n' "$PT_LOAD_OUT" | grep -Fx "PT_LOAD_SHA256=$EXPECTED_PT_LOAD_SHA" >/dev/null
printf '%s\n' "$PT_LOAD_OUT" | grep -Fx "PT_LOAD_BYTES=$EXPECTED_PT_LOAD_BYTES" >/dev/null
printf 'CP2P_HW1_PT_LOAD_IDENTITY=PASS\n'

printf '%s\n' '===== ITEM-10 HOST ACTIVATION SANITY ====='
(
    cd "$WORK/experiments/media-harness-h1"
    python3 h1_cp2p_item10_activation_test.py
)
printf 'CP2P_HW1_ITEM10_HOST_SANITY=PASS\n'

printf '%s\n' '===== CP2P PROFILE VALIDATE-ONLY ====='
cd "$WORK"
python3 "$RUNNER" \
    --profile CP2P_ALL_GUNS \
    --duration 300 \
    --display :0.0 \
    --validate-only
printf 'CP2P_HW1_PROFILE_VALIDATE=PASS\n'

printf '%s\n' '===== PI RFB / AUDIO / PSTV PREFLIGHT ====='
printf 'RFB_SOCKET_STATE=%s\n' "$(systemctl is-active ps-to-vnc-rfb.socket 2>/dev/null || true)"
printf 'RFB_PROVIDER_STATE=%s\n' "$(systemctl is-active ps-to-vnc-rfb-tigervnc.service 2>/dev/null || true)"
python3 - <<'PY'
import socket
with socket.create_connection(("127.0.0.1", 5903), timeout=5.0) as stream:
    banner = stream.recv(12)
print(f"RFB_UPSTREAM_BANNER={banner!r}")
if banner != b"RFB 003.008\n":
    raise SystemExit("RFB_UPSTREAM=FAIL")
print("RFB_UPSTREAM=PASS")
PY

python3 - <<'PY'
import sys
sys.path.insert(0, "experiments/audio-transport/pi")
import mux_daemon
monitor = mux_daemon.discover_default_sink_monitor()
print(f"AUDIO_CAPTURE_TARGET={monitor}")
if not monitor:
    raise SystemExit("AUDIO_CAPTURE_TARGET=FAIL")
print("AUDIO_CAPTURE_PREFLIGHT=PASS")
PY

command -v ffmpeg >/dev/null
command -v pw-record >/dev/null
printf 'CP2P_HW1_MEDIA_TOOLS=PASS\n'

if ss -H -ltn | awk '{print $4}' | grep -Eq '(^|:)5902$'; then
    echo 'PSTV_PORT_5902=BUSY'
    ss -H -ltnp | grep -E '(^|:)5902([[:space:]]|$)' || true
    exit 1
fi
printf 'PSTV_PORT_5902=FREE\n'

python3 - <<'PY'
import socket
try:
    with socket.create_connection(("192.168.50.2", 21), timeout=5.0) as stream:
        stream.settimeout(5.0)
        banner = stream.recv(256)
except OSError as exc:
    raise SystemExit(f"PS2_FTP=FAIL {exc}")
print(f"PS2_FTP_BANNER={banner!r}")
if not banner.startswith(b"220"):
    raise SystemExit("PS2_FTP=FAIL_BAD_BANNER")
print("PS2_FTP=PASS")
PY

STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
TEST_ID="H1-CP2P-ALL-GUNS-HW1-$STAMP"
DEPLOY_EVIDENCE="$HOME/ps2vnc-evidence/deployments/$TEST_ID.json"
RUN_EVIDENCE="$HOME/ps2vnc-evidence/h1-cp2p-all-guns-hw1-$STAMP"

printf '%s\n' '===== DEPLOYMENT DRY RUN ====='
python3 scripts/testkit/deploy-elf.py \
    --repo "$WORK" \
    --elf "$ELF" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$EXPECTED_ELF_SHA" \
    --expected-bytes "$EXPECTED_ELF_BYTES" \
    --evidence "$DEPLOY_EVIDENCE" \
    --dry-run

printf '%s\n' '===== EXACT DUAL-TARGET DEPLOY + READBACK ====='
python3 scripts/testkit/deploy-elf.py \
    --repo "$WORK" \
    --elf "$ELF" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$EXPECTED_ELF_SHA" \
    --expected-bytes "$EXPECTED_ELF_BYTES" \
    --evidence "$DEPLOY_EVIDENCE" \
    --operator-authorized

printf 'CP2P_HW1_TEST_ID=%s\n' "$TEST_ID"
printf 'CP2P_HW1_DEPLOY_EVIDENCE=%s\n' "$DEPLOY_EVIDENCE"
printf 'CP2P_HW1_RUN_EVIDENCE=%s\n' "$RUN_EVIDENCE"
printf '%s\n' 'OPERATOR: launch mass:/0/PS2VNC.ELF once now if the resident CP2P candidate is not already running.'
printf '%s\n' 'OPERATOR: when the live desktop appears, execute the physical checklist below during this 300-second session.'

export H1_RFB_UPSTREAM_HOST="127.0.0.1"
export H1_RFB_UPSTREAM_PORT="5903"
python3 "$RUNNER" \
    --profile CP2P_ALL_GUNS \
    --duration 300 \
    --display :0.0 \
    --listen 0.0.0.0 \
    --port 5902 \
    --evidence "$RUN_EVIDENCE"

printf '%s\n' '===== CP2P ALL-GUNS HOST RUN RETURNED ====='
printf 'CP2P_HW1_TEST_ID=%s\n' "$TEST_ID"
printf 'CP2P_HW1_DEPLOY_EVIDENCE=%s\n' "$DEPLOY_EVIDENCE"
printf 'CP2P_HW1_RUN_EVIDENCE=%s\n' "$RUN_EVIDENCE"
printf 'CP2P_HW1_MACHINE_RUN_COMPLETE=YES\n'
printf 'CP2P_HW1_HARDWARE_QUALIFIED=NO_OPERATOR_JUDGMENT_STILL_REQUIRED\n'
```

## Physical operator checklist — primary qualification run

Machine validation is necessary but not sufficient. Judge the physical TV/controller behavior while the 300-second finite session is running.

### A. Baseline before first MPEG generation

1. The live Pi desktop appears through visible RFB and continues updating.
2. PCM audio is audible and remains clean enough to judge continuity.
3. Qualified desktop interaction remains intact: left stick/D-pad mouse motion, Cross left click/drag, Circle right click.
4. Select opens the Issue-39 OSK without a phantom remote click; D-pad selection, Cross activation, Triangle Shift, Square Backspace, Start Enter, R1 Tab, and clean OSK exit still behave normally where safe to exercise.

### B. Initial calibration -> generation N

1. Hold `START+SELECT` until calibration takes foreground, then release both buttons.
2. Confirm the RFB desktop image is frozen visually while calibration owns foreground, without losing the session.
3. Calibration controls are the repository-authoritative mapping:
   - D-pad: resize around center in 16-pixel increments;
   - R1 + D-pad: move one pixel per sample;
   - R2 + D-pad: inner matte adjustment;
   - L2 + D-pad: outer matte adjustment;
   - Start: reset;
   - Triangle: Controls;
   - X: Review;
   - Circle: cancel/back.
4. Choose a valid region containing moving desktop content. Press X to enter Review, **release X**, then press X a second time to accept.
5. On acceptance, generation N must become live: MPEG motion appears in the accepted base region, PCM continues, and RFB resumes outside the suppression perimeter.
6. Exercise high desktop change outside the MPEG/suppression area. The outside RFB region must remain live rather than freezing behind MPEG ownership.
7. Exercise mouse and OSK again while MPEG is live. Input remains functional; MPEG ownership must not steal ordinary interaction outside calibration foreground.

Expected Pi-side evidence/console markers include:

- `H1_CP2P_MPEG_START_PREPARED=...generation=N...`
- `H1_CP2P_RFB_SUPPRESSION_PREPARED=...generation=N...`
- `H1_CP2P_MPEG_CAPTURE_PREPARED=...generation=N...`
- `H1_CP2P_MPEG_EMISSION=OPEN generation=N`

The exact capture evidence must use the accepted START base X/Y/W/H. The RFB suppression evidence may be larger because outer matte is a separate perimeter.

### C. Live recalibration N -> full RFB -> N+1

1. While generation N is visibly live, hold `START+SELECT` again to request recalibration.
2. Do not accept a new generation until the old MPEG ownership visibly disappears and the desktop has returned through the required **fresh full RFB** restoration after Pi retirement completion.
3. The next calibration UI must be seeded from the last confirmed settings rather than mutating generation N in place.
4. Make a visible small change to the seeded region so N+1 is distinguishable, then enter Review and perform the same release-X / second-X acceptance.
5. Generation N+1 must become live with the new geometry. No stale picture/data from N may appear inside N+1 ownership.
6. RFB outside the new suppression rectangle, PCM, mouse, keyboard/OSK, and local UI behavior must remain live.

Expected Pi-side evidence/console ordering is:

`... generation N live ...`

`H1_CP2P_MPEG_RETIRE_COMPLETE=...generation=N...`

then a new START:

`H1_CP2P_MPEG_START_PREPARED=...generation=N+1...`

followed by new capture/suppression preparation and:

`H1_CP2P_MPEG_EMISSION=OPEN generation=N+1`

The critical physical ordering is the same as the software contract: old Pi generation retires first, then local PS2 ownership retires, then one fresh full-RFB restoration is presented, then recalibration may create the fresh generation.

### D. Finite-session completion

At the end of the finite session, require all of the following:

- the Pi runner returns normally;
- `H1_CP2P_ITEM10_SESSION_VALIDATION=PASS` is printed;
- `H1_PI_MUX_SESSION=PASS profile=CP2P_ALL_GUNS ...` is printed;
- PS2 result reports `transport_error=0` and `integrity_pass=1`;
- `pictures_decoded > 0` and `pictures_displayed > 0`;
- MPEG wire CRC/count validation passes;
- canonical PCM CRC/count validation passes;
- any MPEG residual at final session teardown is accounted for as intentional queue residue rather than silently reclassified as consumed;
- the console did not freeze and no automatic recovery/watchdog was required.

If a failure occurs, preserve the complete `ps2run-interactive` output archive, deployment JSON, and run evidence directory. Record the **lowest layer still demonstrably making progress** before changing any implementation.

## Follow-up cancel-path run

Do not mix a cancel diagnosis into a failed primary run. After a green primary run, a second reproduction may specifically exercise:

`live N -> START+SELECT -> exact retirement -> fresh full RFB -> seeded calibration -> Circle cancel -> full live RFB + PCM, no new MPEG generation`.

That follow-up strengthens the already host-proven cancel lifecycle but does not replace the primary N -> N+1 qualification.

## Seal rule

Only after the machine result and the physical checklist are both green should item #12 be recorded as qualified. The final seal must append, not rewrite, history and must capture at minimum:

- frozen candidate commit;
- PS2DEV image digest;
- exact ELF and PT_LOAD identities;
- deployment test ID and readback evidence path;
- Pi run evidence directory;
- session ID;
- observed generation sequence;
- physical operator result for baseline, generation N, retirement/full-RFB restoration, N+1, high-change RFB, PCM, mouse, keyboard/OSK, and session completion.
