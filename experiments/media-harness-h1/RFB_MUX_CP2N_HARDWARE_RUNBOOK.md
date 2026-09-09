# CP2N real Issue-39 interaction hardware runbook

## Purpose

This is the operator runbook for the first real-PS2 execution of the exact CP2N
candidate pinned in `RFB_MUX_CP2N_VISIBLE_INTERACTION_CANDIDATE.md`.

It does not change the candidate, begin Issue #40, enable AUDIO/MPEG, or qualify
anything by itself. It exists to make the hardware gate reproducible and
fail-closed.

The hardware authority before this run remains CP2L visible RFB plus PS2 mouse
input.

## Candidate identity

The DUT is the pristine CI-built CP2N runtime from build head:

    7e047a9cb9dfef9283f66a80ccd865664f075c11

Expected ELF identity:

    SHA256=f55b2620903ea0ba2a64c53c4196b1e1959a6bb13be1c8a3ae6bf52a08df1000
    BYTES=3041300

Expected PT_LOAD identity:

    SHA256=544117b8bdf4c0cacefb7d6b2e8fa01eba90809a369cfd8bfdfc0abc36722914
    BYTES=491028

The candidate is intentionally not identity-restamped for this gate. Stamping
would change the PT_LOAD and create a different hardware candidate.

## Before running the packet

The PS2 must be powered and in the same FTP-ready state used for the prior H1
hardware deployments so `192.168.50.2:21` is reachable.

The Pi's demand-ready RFB endpoint is expected at `127.0.0.1:5903`. Its socket
unit is `ps-to-vnc-rfb.socket`, with provider service
`ps-to-vnc-rfb-tigervnc.service`. The packet below verifies the RFB 3.8 banner
before touching the DUT.

For the qualitative keyboard test, leave a harmless focused remote text target
available on the Pi desktop, such as a terminal prompt. An external Windows VNC
client may be used to arrange that desktop beforehand; it is separate from the
single Pi-to-PS2 PSTV connection and is not part of the PSTV stream count.

## Canonical interactive packet

Paste the complete block into `ps2run-interactive`. The first line is the
required clipboard-safety guard.

```sh
exit # PS2RUN_INTERACTIVE_ONLY
set -eu

BASE="$HOME/src/PS-to-VNC"
WORK="$HOME/src/PS-to-VNC-h1-rfb-cp2n-7e047a9c"
COMMIT="7e047a9cb9dfef9283f66a80ccd865664f075c11"
BRANCH="experiment/h1-rfb-mux-prep"
REMOTE="github-ps-to-vnc"
EXPECTED_ELF_SHA="f55b2620903ea0ba2a64c53c4196b1e1959a6bb13be1c8a3ae6bf52a08df1000"
EXPECTED_ELF_BYTES="3041300"
EXPECTED_PT_LOAD_SHA="544117b8bdf4c0cacefb7d6b2e8fa01eba90809a369cfd8bfdfc0abc36722914"
EXPECTED_PT_LOAD_BYTES="491028"
BUILD_DIR="build/experiments/media-harness-h1-cp2n-visible-rfb-interaction/ps2"
ELF_REL="$BUILD_DIR/PS2VNC-H1-CP2N-VisibleRFBInteraction.ELF"

export PS2DEV="${PS2DEV:-/usr/local/ps2dev}"
export PS2SDK="${PS2SDK:-$PS2DEV/ps2sdk}"
export GSKIT="${GSKIT:-$PS2DEV/gsKit}"
export PATH="$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin:$PATH"

printf '%s\n' '===== CP2N EXACT-SOURCE PREFLIGHT ====='
git -C "$BASE" fetch "$REMOTE" "$BRANCH"
git -C "$BASE" cat-file -e "$COMMIT^{commit}"

if [ -e "$WORK" ]; then
    ACTUAL_WORK_HEAD="$(git -C "$WORK" rev-parse HEAD)"
    [ "$ACTUAL_WORK_HEAD" = "$COMMIT" ] || {
        echo "CP2N_WORKTREE_HEAD=FAIL actual=$ACTUAL_WORK_HEAD expected=$COMMIT"
        exit 1
    }
    [ -z "$(git -C "$WORK" status --porcelain=v1)" ] || {
        echo 'CP2N_WORKTREE_CLEAN=FAIL'
        git -C "$WORK" status --short
        exit 1
    }
else
    git -C "$BASE" worktree add --detach "$WORK" "$COMMIT"
fi

printf 'CP2N_WORKTREE=%s\n' "$WORK"
printf 'CP2N_WORKTREE_HEAD=%s\n' "$(git -C "$WORK" rev-parse HEAD)"
printf 'CP2N_WORKTREE_CLEAN=PASS\n'

cd "$WORK"
rm -rf "$BUILD_DIR"
make -f mk/media-harness-h1-cp2n-visible-rfb-interaction.mk
python3 experiments/media-harness-h1/check_rfb_visible_interaction.py \
    --build-dir "$BUILD_DIR"
python3 experiments/media-harness-h1/check_rfb_mux_seam.py \
    --build-dir "$BUILD_DIR"

ELF="$WORK/$ELF_REL"
ACTUAL_ELF_SHA="$(sha256sum "$ELF" | awk '{print $1}')"
ACTUAL_ELF_BYTES="$(stat -c '%s' "$ELF")"
printf 'CP2N_LOCAL_ELF_SHA256=%s\n' "$ACTUAL_ELF_SHA"
printf 'CP2N_LOCAL_ELF_BYTES=%s\n' "$ACTUAL_ELF_BYTES"
[ "$ACTUAL_ELF_SHA" = "$EXPECTED_ELF_SHA" ] || {
    echo 'CP2N_LOCAL_ELF_IDENTITY=FAIL_SHA256'
    exit 1
}
[ "$ACTUAL_ELF_BYTES" = "$EXPECTED_ELF_BYTES" ] || {
    echo 'CP2N_LOCAL_ELF_IDENTITY=FAIL_BYTES'
    exit 1
}
printf 'CP2N_LOCAL_ELF_IDENTITY=PASS\n'

PT_LOAD_OUT="$(./scripts/testkit/pt-load-fingerprint.sh "$ELF")"
printf '%s\n' "$PT_LOAD_OUT"
printf '%s\n' "$PT_LOAD_OUT" | grep -Fx "PT_LOAD_SHA256=$EXPECTED_PT_LOAD_SHA" >/dev/null
printf '%s\n' "$PT_LOAD_OUT" | grep -Fx "PT_LOAD_BYTES=$EXPECTED_PT_LOAD_BYTES" >/dev/null
printf 'CP2N_PT_LOAD_IDENTITY=PASS\n'

printf '%s\n' '===== PI RFB/PSTV PREFLIGHT ====='
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

if ss -H -ltn | awk '{print $4}' | grep -Eq '(^|:)5902$'; then
    echo 'PSTV_PORT_5902=BUSY'
    ss -H -ltnp | grep -E '(^|:)5902([[:space:]]|$)' || true
    exit 1
fi
printf 'PSTV_PORT_5902=FREE\n'

python3 - <<'PY'
import socket

try:
    with socket.create_connection(("192.168.50.2", 21), timeout=5.0):
        pass
except OSError as exc:
    raise SystemExit(f"PS2_FTP=FAIL {exc}")
print("PS2_FTP=PASS")
PY

STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
TEST_ID="H1-RFB-CP2N-HW1-$STAMP"
DEPLOY_EVIDENCE="$HOME/ps2vnc-evidence/deployments/$TEST_ID.json"
RUN_EVIDENCE="$HOME/ps2vnc-evidence/h1-rfb-cp2n-hw1-$STAMP"

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

printf 'CP2N_TEST_ID=%s\n' "$TEST_ID"
printf 'CP2N_DEPLOY_EVIDENCE=%s\n' "$DEPLOY_EVIDENCE"
printf 'CP2N_RUN_EVIDENCE=%s\n' "$RUN_EVIDENCE"
printf '%s\n' 'OPERATOR: if H1_PS2_CONNECTED appears automatically, DO NOT relaunch the ELF.'
printf '%s\n' 'OPERATOR: if H1_LISTENING appears without H1_PS2_CONNECTED, launch mass:/0/PS2VNC.ELF once.'
printf '%s\n' 'OPERATOR: exercise the CP2N checklist on the physical PS2 television during the session.'

export H1_RFB_UPSTREAM_HOST="127.0.0.1"
export H1_RFB_UPSTREAM_PORT="5903"
python3 experiments/media-harness-h1/h1_tool.py run \
    --profile H1_RFB_ONLY \
    --set rfb_mode=2 \
    --duration 90 \
    --display :0.0 \
    --listen 0.0.0.0 \
    --port 5902 \
    --evidence "$RUN_EVIDENCE"

printf '%s\n' '===== CP2N HOST RUN RETURNED ====='
printf 'CP2N_TEST_ID=%s\n' "$TEST_ID"
printf 'CP2N_DEPLOY_EVIDENCE=%s\n' "$DEPLOY_EVIDENCE"
printf 'CP2N_RUN_EVIDENCE=%s\n' "$RUN_EVIDENCE"
printf 'CP2N_MACHINE_RUN_COMPLETE=YES\n'
printf 'CP2N_HARDWARE_QUALIFIED=NO_OPERATOR_JUDGMENT_STILL_REQUIRED\n'
```

## Physical operator checklist

During the 90-second finite session, judge the physical television and
controller behavior, not merely the host console:

1. live Pi desktop appears and continues updating;
2. CP2L desktop mouse behavior still works: left stick/D-pad motion, Cross left
   click/drag, Circle right click;
3. Select opens the real Issue-39 OSK without a phantom remote click;
4. desktop mouse movement/click interpretation is suspended while the OSK owns
   foreground input;
5. D-pad moves OSK selection;
6. Cross activates a selected key and the focused remote text target receives
   the resulting character;
7. Triangle Shift behavior is observable with an appropriate selected key;
8. Square Backspace, Start Enter, and R1 Tab produce their normal effects where
   safe to observe;
9. Circle or Select closes the OSK; after releasing the transition buttons, the
   desktop mouse resumes without a carried-through click/action;
10. the host reaches REQUEST -> BOUNDARY -> COMMIT -> COMPLETE and finishes with
    clean session validation, no integrity failure, and no transport error.

If the console freezes, the OSK/input path behaves incorrectly, the host fails,
or the exact ELF/PT_LOAD/readback checks do not pass, CP2N remains hardware
unqualified. Preserve the returned console output and evidence rather than
masking or auto-recovering the failure.

## Qualification rule

A green machine result is necessary but insufficient. Only the operator's
physical observation can complete this gate.

Until both machine evidence and operator judgment pass:

    CP2N_HARDWARE_QUALIFIED=NO
    CURRENT_HARDWARE_AUTHORITY=CP2L_VISIBLE_RFB_PLUS_PS2_MOUSE_INPUT

No RFB+AUDIO, RFB+MPEG, hybrid composition, production transport extraction, or
Issue #40 work is authorized by this runbook.
