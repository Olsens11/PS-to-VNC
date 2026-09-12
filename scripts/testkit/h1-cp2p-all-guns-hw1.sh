#!/usr/bin/env bash
# Exact operator packet for CP2P item #12 HW1 qualification.
# This script is documentation/evidence apparatus only. It always builds and
# deploys the frozen candidate commit named below; it does not qualify its own
# branch HEAD.

set -eu

BASE="${HOME}/src/PS-to-VNC"
WORK="${HOME}/src/PS-to-VNC-h1-cp2p-hw1-4c2d08e"
COMMIT="4c2d08ea28fe5c1d462f078ae688a24515914d25"
BRANCH="qualification/h1-cp2p-all-guns-hw1"
REMOTE="origin"
IMAGE="ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11"
EXPECTED_ELF_SHA="89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad"
EXPECTED_ELF_BYTES="3238996"
EXPECTED_PT_LOAD_SHA="4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494"
EXPECTED_PT_LOAD_BYTES="512276"
BUILD_DIR="build/experiments/media-harness-h1-cp2p-application-link/ps2"
ELF_REL="${BUILD_DIR}/PS2VNC-H1-CP2P-ApplicationLink.ELF"
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
printf '%s\n' 'OPERATOR: execute the CP2P all-guns physical checklist during this 300-second session.'

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
