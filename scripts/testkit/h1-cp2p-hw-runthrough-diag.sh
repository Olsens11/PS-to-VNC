#!/usr/bin/env bash
# CP2P hardware run-through diagnostic.
#
# Purpose:
#   Preserve the sealed item-#10 candidate while building a disposable diagnostic
#   derivative that reports pre-ACK progress, ACKs immediately after a valid CONFIG
#   decode, treats PS2 ERROR frames as warnings on the Pi, and continues until an
#   actually indispensable runtime operation/socket/hardware boundary stops progress.
#
# This script does NOT qualify the diagnostic ELF and does NOT mutate the sealed
# candidate branch.

set -eu

BASE="$HOME/src/PS-to-VNC"
WORK="$HOME/src/PS-to-VNC-h1-cp2p-runthrough-diag"
SEALED_COMMIT="4c2d08ea28fe5c1d462f078ae688a24515914d25"
IMAGE="ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11"
BUILD_DIR="build/experiments/media-harness-h1-cp2p-application-link/ps2"
ELF_REL="$BUILD_DIR/PS2VNC-H1-CP2P-ApplicationLink.ELF"
RUNNER="experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py"

printf '%s\n' '===== CP2P RUN-THROUGH DIAGNOSTIC PREP ====='
git -C "$BASE" cat-file -e "$SEALED_COMMIT^{commit}"

if [ -e "$WORK" ]; then
    git -C "$BASE" worktree remove --force "$WORK" >/dev/null 2>&1 || true
    rm -rf "$WORK"
fi
git -C "$BASE" worktree add --detach "$WORK" "$SEALED_COMMIT"

python3 - "$WORK" <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])

transport = root / "experiments/media-harness-h1/h1_transport_runtime.c"
s = transport.read_text()

needle = '''static int h1_send_config_ack(\n    pstvnc_h1_transport_runtime_t *runtime)\n{\n    return h1_send_frame(\n        runtime,\n        PSTVNC_TRANSPORT_FRAME_CONFIG,\n        PSTVNC_TRANSPORT_CHANNEL_CONTROL,\n        PSTVNC_H1_CONFIG_ACK_FLAG,\n        runtime->receiver_payload,\n        runtime->config_payload_length);\n}\n'''
insert = needle + '''\n/* Disposable HW run-through telemetry.  These frames are observations, not gates. */\nstatic void h1_diag_stage(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t stage)\n{\n    uint8_t payload[8];\n\n    pstvnc_transport_write_be32(&payload[0], stage);\n    pstvnc_transport_write_be32(&payload[4], (uint32_t)runtime->error);\n    (void)h1_send_frame(\n        runtime,\n        PSTVNC_TRANSPORT_FRAME_HEARTBEAT,\n        PSTVNC_TRANSPORT_CHANNEL_CONTROL,\n        0u,\n        payload,\n        sizeof(payload));\n}\n'''
if needle not in s:
    raise SystemExit("RUNTHROUGH_PATCH=FAIL missing config-ack helper")
s = s.replace(needle, insert, 1)

old = '''    if (!h1_receive_config(runtime)) {\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n\n    if (!h1_allocate_queues(runtime) ||\n        !h1_allocate_receiver_stack(runtime) ||\n        !h1_apply_socket_options(runtime)) {\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n\n#ifdef PSTVNC_H1_RFB_MUX_PREP\n    /*\n     * Critical ordering invariant: if channel 1 is ever accepted by CONFIG, its\n     * queue/semaphore/binding must exist before CONFIG ACK, initial credit, and\n     * especially before the sole physical receiver thread can accept DATA.\n     */\n    if (!pstvnc_h1_rfb_transport_prepare(runtime)) {\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n#endif\n\n    if (!h1_send_config_ack(runtime))\n        goto fail;\n\n    if (runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM &&\n        !h1_send_credit(\n            runtime,\n            PSTVNC_TRANSPORT_CHANNEL_AUDIO,\n            runtime->config.audio_initial_credit_bytes))\n        goto fail;\n\n    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&\n        !h1_send_credit(\n            runtime,\n            PSTVNC_TRANSPORT_CHANNEL_MPEG2,\n            runtime->config.mpeg_initial_credit_bytes))\n        goto fail;\n\n#ifdef PSTVNC_H1_RFB_MUX_PREP\n    if (!pstvnc_h1_rfb_transport_send_initial_credit(runtime))\n        goto fail;\n#endif\n\n    if (!h1_start_receiver(runtime))\n        goto fail;\n'''
new = '''    if (!h1_receive_config(runtime)) {\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n\n    /*\n     * HW run-through diagnostic: report that CONFIG decoded, then ACK immediately.\n     * The ACK is observational only; it deliberately does not certify subsequent\n     * queue/stack/RFB preparation.  We keep driving until a real operation fails.\n     */\n    h1_diag_stage(runtime, 0xD1000001u); /* CONFIG_DECODED */\n    if (!h1_send_config_ack(runtime))\n        goto fail;\n    h1_diag_stage(runtime, 0xD1000002u); /* EARLY_ACK_SENT */\n\n    if (!h1_allocate_queues(runtime)) {\n        h1_diag_stage(runtime, 0xD1E00001u); /* QUEUE_ALLOCATION_FAILED */\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n    h1_diag_stage(runtime, 0xD1000003u); /* QUEUES_ALLOCATED */\n\n    if (!h1_allocate_receiver_stack(runtime)) {\n        h1_diag_stage(runtime, 0xD1E00002u); /* RECEIVER_STACK_FAILED */\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n    h1_diag_stage(runtime, 0xD1000004u); /* RECEIVER_STACK_ALLOCATED */\n\n    if (!h1_apply_socket_options(runtime)) {\n        h1_diag_stage(runtime, 0xD1E00003u); /* SOCKET_OPTIONS_FAILED */\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n    h1_diag_stage(runtime, 0xD1000005u); /* SOCKET_OPTIONS_APPLIED */\n\n#ifdef PSTVNC_H1_RFB_MUX_PREP\n    if (!pstvnc_h1_rfb_transport_prepare(runtime)) {\n        h1_diag_stage(runtime, 0xD1E00004u); /* RFB_PREPARE_FAILED */\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n    h1_diag_stage(runtime, 0xD1000006u); /* RFB_PREPARED */\n#endif\n\n    if (runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM) {\n        if (!h1_send_credit(\n                runtime,\n                PSTVNC_TRANSPORT_CHANNEL_AUDIO,\n                runtime->config.audio_initial_credit_bytes))\n            goto fail;\n        h1_diag_stage(runtime, 0xD1000007u); /* AUDIO_CREDIT_SENT */\n    }\n\n    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {\n        if (!h1_send_credit(\n                runtime,\n                PSTVNC_TRANSPORT_CHANNEL_MPEG2,\n                runtime->config.mpeg_initial_credit_bytes))\n            goto fail;\n        h1_diag_stage(runtime, 0xD1000008u); /* MPEG_CREDIT_SENT */\n    }\n\n#ifdef PSTVNC_H1_RFB_MUX_PREP\n    if (!pstvnc_h1_rfb_transport_send_initial_credit(runtime))\n        goto fail;\n    h1_diag_stage(runtime, 0xD1000009u); /* RFB_CREDIT_SENT */\n#endif\n\n    if (!h1_start_receiver(runtime)) {\n        h1_diag_stage(runtime, 0xD1E00005u); /* RECEIVER_THREAD_FAILED */\n        (void)h1_send_error_code(runtime);\n        goto fail;\n    }\n    h1_diag_stage(runtime, 0xD100000Au); /* RECEIVER_THREAD_STARTED */\n'''
if old not in s:
    raise SystemExit("RUNTHROUGH_PATCH=FAIL missing transport-start block")
s = s.replace(old, new, 1)
transport.write_text(s)

base = root / "experiments/media-harness-h1/h1_mux_server.py"
s = base.read_text()
old = '''                elif frame.kind == FRAME_ERROR:\n                    code = (\n                        struct.unpack(\">I\", frame.payload)[0]\n                        if len(frame.payload) == 4\n                        else -1\n                    )\n                    raise ProtocolError(f\"PS2 reported H1 transport error={code}\")\n\n                else:\n'''
new = '''                elif frame.kind == FRAME_HEARTBEAT:\n                    if frame.channel == CHANNEL_CONTROL and len(frame.payload) == 8:\n                        stage, error = struct.unpack(\">II\", frame.payload)\n                        print(\n                            f\"H1_DIAG_PS2_STAGE=0x{stage:08X} error={error}\",\n                            flush=True,\n                        )\n                    else:\n                        print(\n                            \"H1_DIAG_HEARTBEAT_WARNING=\"\n                            + json.dumps({\n                                \"channel\": frame.channel,\n                                \"flags\": frame.flags,\n                                \"payload_hex\": frame.payload.hex(),\n                            }, sort_keys=True),\n                            flush=True,\n                        )\n\n                elif frame.kind == FRAME_ERROR:\n                    code = (\n                        struct.unpack(\">I\", frame.payload)[0]\n                        if len(frame.payload) == 4\n                        else -1\n                    )\n                    print(\n                        f\"H1_DIAG_PS2_ERROR_WARNING={code} CONTINUE_UNTIL_SOCKET_OR_RUNTIME_STOPS\",\n                        flush=True,\n                    )\n\n                else:\n'''
if old not in s:
    raise SystemExit("RUNTHROUGH_PATCH=FAIL missing Pi ERROR handler")
s = s.replace(old, new, 1)
base.write_text(s)

bridge = root / "experiments/media-harness-h1/h1_mux_server_cumulative39_rfb_pcm_bridge.py"
s = bridge.read_text()
old = '''        if not self.config_ack_event.wait(timeout=10.0):\n            raise base.ProtocolError(\"timed out waiting for exact CONFIG ACK\")\n        self.check_reader()\n\n        self._start_pcm_producer()\n'''
new = '''        if not self.config_ack_event.wait(timeout=10.0):\n            print(\n                \"H1_DIAG_CONFIG_ACK_WARNING=TIMEOUT CONTINUING_WITH_LIGHTS_AND_SIRENS\",\n                flush=True,\n            )\n        else:\n            print(\n                \"H1_DIAG_CONFIG_ACK_OBSERVED=YES CONTINUING_REGARDLESS_OF_ACK_AUTHORITY\",\n                flush=True,\n            )\n        # A reader/socket failure is a real transport boundary, not a policy gate.\n        self.check_reader()\n\n        self._start_pcm_producer()\n'''
if old not in s:
    raise SystemExit("RUNTHROUGH_PATCH=FAIL missing CONFIG wait block")
s = s.replace(old, new, 1)
bridge.write_text(s)

print("RUNTHROUGH_PATCH=PASS")
PY

printf '%s\n' '===== RUN-THROUGH DIFF ====='
git -C "$WORK" diff --check
git -C "$WORK" diff --stat
git -C "$WORK" diff -- \
    experiments/media-harness-h1/h1_transport_runtime.c \
    experiments/media-harness-h1/h1_mux_server.py \
    experiments/media-harness-h1/h1_mux_server_cumulative39_rfb_pcm_bridge.py

printf '%s\n' '===== PINNED DIAGNOSTIC BUILD ====='
docker run --rm \
    --entrypoint /bin/sh \
    -v "$WORK:/repo" \
    -w /repo \
    "$IMAGE" \
    -lc '
        set -eu
        apk add --no-cache bash binutils make python3 >/dev/null
        export PATH=/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
        rm -rf build/experiments/media-harness-h1-cp2p-application-link/ps2
        make -f mk/media-harness-h1-cp2p-application-link.mk cp2p-application-link-check
    '

ELF="$WORK/$ELF_REL"
ELF_SHA="$(sha256sum "$ELF" | awk '{print $1}')"
ELF_BYTES="$(stat -c '%s' "$ELF")"
SHORT_SHA="${ELF_SHA:0:8}"
printf 'RUNTHROUGH_ELF_SHA256=%s\n' "$ELF_SHA"
printf 'RUNTHROUGH_ELF_BYTES=%s\n' "$ELF_BYTES"

printf '%s\n' '===== DEPLOY DIAGNOSTIC ELF ====='
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
TEST_ID="H1-CP2P-RUNTHROUGH-DIAG-$STAMP-$SHORT_SHA"
DEPLOY_EVIDENCE="$HOME/ps2vnc-evidence/deployments/$TEST_ID.json"
RUN_EVIDENCE="$HOME/ps2vnc-evidence/h1-cp2p-runthrough-diag-$STAMP"

python3 "$WORK/scripts/testkit/deploy-elf.py" \
    --repo "$WORK" \
    --elf "$ELF" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$DEPLOY_EVIDENCE" \
    --operator-authorized

printf 'RUNTHROUGH_TEST_ID=%s\n' "$TEST_ID"
printf 'RUNTHROUGH_DEPLOY_EVIDENCE=%s\n' "$DEPLOY_EVIDENCE"
printf 'RUNTHROUGH_RUN_EVIDENCE=%s\n' "$RUN_EVIDENCE"
printf '%s\n' 'OPERATOR: launch mass:/0/PS2VNC.ELF now. This is the disposable run-through diagnostic.'
printf '%s\n' 'OPERATOR: the Pi will record ACK/ERROR/stage warnings but will not treat them as permission gates.'

if ss -H -ltn | awk '{print $4}' | grep -Eq '(^|:)5902$'; then
    echo 'PSTV_PORT_5902=BUSY'
    ss -H -ltnp | grep -E '(^|:)5902([[:space:]]|$)' || true
    exit 1
fi

export H1_RFB_UPSTREAM_HOST="127.0.0.1"
export H1_RFB_UPSTREAM_PORT="5903"
cd "$WORK"
python3 "$RUNNER" \
    --profile CP2P_ALL_GUNS \
    --duration 300 \
    --display :0.0 \
    --listen 0.0.0.0 \
    --port 5902 \
    --evidence "$RUN_EVIDENCE"
