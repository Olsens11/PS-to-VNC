#!/usr/bin/env python3
"""One-shot implementation patch for CP2P item #11B live generation boundary."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text()


def write(rel: str, text: str) -> None:
    (ROOT / rel).write_text(text)


def replace_block(rel: str, old: str, new: str, marker: str, label: str) -> None:
    text = read(rel)
    if marker in text:
        print(f"ITEM11B_ALREADY={label}")
        return
    if text.count(old) != 1:
        raise SystemExit(f"{label}: expected one source block, found {text.count(old)}")
    write(rel, text.replace(old, new, 1))


def write_new(rel: str, content: str, marker: str) -> None:
    path = ROOT / rel
    if path.exists():
        current = path.read_text()
        if marker not in current:
            raise SystemExit(f"{rel}: existing file does not match item #11B authority")
        print(f"ITEM11B_ALREADY={rel}")
        return
    path.write_text(content)


# ---------------------------------------------------------------------------
# Shared bounded queue: add an O(1) discard primitive that preserves telemetry.
# ---------------------------------------------------------------------------
replace_block(
    "experiments/audio-transport/common/transport_queue.h",
    """int pstvnc_transport_queue_read(\n    pstvnc_transport_queue_t *queue,\n    void *data,\n    size_t count);\n\n#endif\n""",
    """int pstvnc_transport_queue_read(\n    pstvnc_transport_queue_t *queue,\n    void *data,\n    size_t count);\n\n/*\n * Discard every currently queued byte without reallocating or lowering the\n * historical high-water mark. Returns the number of discarded bytes.\n */\nsize_t pstvnc_transport_queue_discard_all(\n    pstvnc_transport_queue_t *queue);\n\n#endif\n""",
    "pstvnc_transport_queue_discard_all",
    "transport queue header",
)

queue_c = read("experiments/audio-transport/common/transport_queue.c")
if "size_t pstvnc_transport_queue_discard_all(" not in queue_c:
    queue_c += r'''

size_t pstvnc_transport_queue_discard_all(
    pstvnc_transport_queue_t *queue)
{
    size_t discarded;

    if (queue == NULL || queue->byte_count > queue->capacity)
        return 0u;

    discarded = queue->byte_count;
    queue->read_position = queue->write_position;
    queue->byte_count = 0u;
    return discarded;
}
'''
    write("experiments/audio-transport/common/transport_queue.c", queue_c)
else:
    print("ITEM11B_ALREADY=transport queue implementation")

write_new(
    "experiments/audio-transport/common/transport_queue_generation_boundary_test.c",
    r'''/* Host contract for the queue primitive used by CP2P generation retirement. */
#include "transport_queue.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    pstvnc_transport_queue_t queue;
    unsigned char storage[8];
    unsigned char first[6] = {0, 1, 2, 3, 4, 5};
    unsigned char second[4] = {6, 7, 8, 9};
    unsigned char out[3];

    memset(&queue, 0, sizeof(queue));
    assert(pstvnc_transport_queue_init(&queue, storage, sizeof(storage)));
    assert(pstvnc_transport_queue_write(&queue, first, sizeof(first)));
    assert(pstvnc_transport_queue_read(&queue, out, 2u));
    assert(pstvnc_transport_queue_write(&queue, second, sizeof(second)));
    assert(pstvnc_transport_queue_size(&queue) == 8u);
    assert(pstvnc_transport_queue_high_water(&queue) == 8u);

    assert(pstvnc_transport_queue_discard_all(&queue) == 8u);
    assert(pstvnc_transport_queue_size(&queue) == 0u);
    assert(pstvnc_transport_queue_free(&queue) == 8u);
    assert(pstvnc_transport_queue_high_water(&queue) == 8u);
    assert(queue.read_position == queue.write_position);

    assert(pstvnc_transport_queue_write(&queue, first, 3u));
    assert(pstvnc_transport_queue_read(&queue, out, 3u));
    assert(memcmp(out, first, 3u) == 0);
    assert(pstvnc_transport_queue_discard_all(&queue) == 0u);

    puts("H1_CP2P_QUEUE_GENERATION_BOUNDARY_HOST_TEST=PASS");
    return 0;
}
''',
    "H1_CP2P_QUEUE_GENERATION_BOUNDARY_HOST_TEST=PASS",
)

# ---------------------------------------------------------------------------
# PS2 transport: explicit active-generation acceptance gate + ACK wire fence +
# exact queue discard/final-credit boundary.
# ---------------------------------------------------------------------------
replace_block(
    "experiments/media-harness-h1/h1_transport_runtime.h",
    """    volatile uint32_t mpeg_frames_received;\n    volatile uint32_t mpeg_bytes_enqueued;\n    volatile uint32_t mpeg_bytes_consumed;\n    volatile uint32_t mpeg_credit_frames_sent;\n""",
    """    volatile uint32_t mpeg_frames_received;\n    volatile uint32_t mpeg_bytes_enqueued;\n    volatile uint32_t mpeg_bytes_consumed;\n    volatile uint32_t mpeg_bytes_discarded_generation_boundary;\n    volatile uint32_t mpeg_credit_frames_sent;\n""",
    "mpeg_bytes_discarded_generation_boundary",
    "transport stats field",
)
replace_block(
    "experiments/media-harness-h1/h1_transport_runtime.h",
    """    /* Exact CP2P Pi-retirement request/ack state; receiver thread owns ACK. */\n    volatile uint32_t mpeg_retire_pending_generation;\n    volatile uint32_t mpeg_retire_ack_generation;\n""",
    """    /*\n     * CP2P MPEG epoch state. DATA belongs to the one active generation until\n     * the sole receiver observes that generation's ordered Pi RETIRE ACK.\n     */\n    volatile uint32_t mpeg_data_generation;\n    volatile uint32_t mpeg_retire_pending_generation;\n    volatile uint32_t mpeg_retire_ack_generation;\n""",
    "volatile uint32_t mpeg_data_generation;",
    "transport generation state",
)
replace_block(
    "experiments/media-harness-h1/h1_transport_runtime.h",
    """int pstvnc_h1_transport_mpeg_retire_begin(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\n/* 1=exact completion consumed, 0=still pending, -1=invalid/failed transport. */\nint pstvnc_h1_transport_mpeg_retire_poll(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n""",
    """/* Bind channel-4 DATA acceptance to one immutable CP2P generation. */\nint pstvnc_h1_transport_mpeg_generation_open(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\n/* Abort a just-opened generation only while no MPEG bytes are queued/pending. */\nint pstvnc_h1_transport_mpeg_generation_abort(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_begin(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\n/* 1=ordered Pi completion observed, 0=still pending, -1=invalid/failed. */\nint pstvnc_h1_transport_mpeg_retire_poll(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\n/*\n * Called only after the generation's decoder worker is stopped. Discards the\n * residual old-generation queue, returns all withheld credit, and closes the\n * exact retirement transaction so a fresh generation may open.\n */\nint pstvnc_h1_transport_mpeg_retire_finalize(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation,\n    uint32_t *bytes_discarded);\n""",
    "pstvnc_h1_transport_mpeg_generation_open",
    "transport generation API",
)

runtime_c = "experiments/media-harness-h1/h1_transport_runtime.c"
replace_block(
    runtime_c,
    """#ifdef PSTVNC_H1_RFB_MUX_PREP\n    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_RFB)\n        return pstvnc_h1_rfb_transport_accept_data(\n            runtime,\n            runtime->receiver_payload,\n            header->payload_length);\n#endif\n\n    if (header->payload_length == 0u ||\n""",
    """#ifdef PSTVNC_H1_RFB_MUX_PREP\n    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_RFB)\n        return pstvnc_h1_rfb_transport_accept_data(\n            runtime,\n            runtime->receiver_payload,\n            header->payload_length);\n#endif\n\n    /*\n     * Channel-4 DATA is session-framed but generation-owned by CP2P. The ACK\n     * receiver closes mpeg_data_generation at the ordered wire fence, so any\n     * later stale DATA is a protocol error rather than input to N+1.\n     */\n    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&\n        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&\n        runtime->mpeg_data_generation == 0u) {\n        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);\n        return 0;\n    }\n\n    if (header->payload_length == 0u ||\n""",
    "Channel-4 DATA is session-framed but generation-owned",
    "MPEG DATA generation gate",
)
replace_block(
    runtime_c,
    """        session_id != runtime->config.session_id ||\n        generation == 0u ||\n        runtime->mpeg_retire_pending_generation != generation ||\n        runtime->mpeg_retire_ack_generation != 0u) {\n""",
    """        session_id != runtime->config.session_id ||\n        generation == 0u ||\n        runtime->mpeg_data_generation != generation ||\n        runtime->mpeg_retire_pending_generation != generation ||\n        runtime->mpeg_retire_ack_generation != 0u) {\n""",
    "runtime->mpeg_data_generation != generation",
    "RETIRE ACK exact active generation",
)
replace_block(
    runtime_c,
    """    runtime->mpeg_retire_ack_generation = generation;\n    return 1;\n}\n\nstatic int h1_accept_frame(\n""",
    """    /*\n     * TCP/PSTV order makes this ACK the wire fence: all earlier generation-N\n     * DATA has already been accepted into the queue by this sole receiver.\n     * Close DATA acceptance immediately; local worker/queue cleanup follows.\n     */\n    runtime->mpeg_data_generation = 0u;\n    runtime->mpeg_retire_ack_generation = generation;\n    return 1;\n}\n\nstatic int h1_accept_frame(\n""",
    "TCP/PSTV order makes this ACK the wire fence",
    "RETIRE ACK closes DATA gate",
)

insert_marker = "int pstvnc_h1_transport_mpeg_retire_begin(\n"
text = read(runtime_c)
if "int pstvnc_h1_transport_mpeg_generation_open(" not in text:
    if text.count(insert_marker) != 1:
        raise SystemExit("transport generation insertion point missing")
    generation_code = r'''static int h1_mpeg_queue_empty_for_generation_boundary(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int empty;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_OFF)
        return 1;
    if (runtime->config.video_mode != PSTVNC_H1_VIDEO_MPEG2_ES)
        return 0;

    if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }
    empty = pstvnc_transport_queue_size(&runtime->mpeg_queue) == 0u;
    if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }
    return empty;
}

int pstvnc_h1_transport_mpeg_generation_open(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->mpeg_data_generation != 0u ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u ||
        runtime->mpeg_credit_pending != 0u ||
        !h1_mpeg_queue_empty_for_generation_boundary(runtime))
        return 0;

    runtime->mpeg_data_generation = generation;
    return 1;
}

int pstvnc_h1_transport_mpeg_generation_abort(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->mpeg_data_generation != generation ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u ||
        runtime->mpeg_credit_pending != 0u ||
        !h1_mpeg_queue_empty_for_generation_boundary(runtime))
        return 0;

    runtime->mpeg_data_generation = 0u;
    return 1;
}

'''
    text = text.replace(insert_marker, generation_code + insert_marker, 1)
    write(runtime_c, text)
else:
    print("ITEM11B_ALREADY=transport generation open/abort")

replace_block(
    runtime_c,
    """        runtime->error != PSTVNC_H1_ERROR_NONE ||\n        runtime->stop_requested || runtime->receiver_done ||\n        runtime->mpeg_retire_pending_generation != 0u ||\n""",
    """        runtime->error != PSTVNC_H1_ERROR_NONE ||\n        runtime->stop_requested || runtime->receiver_done ||\n        runtime->mpeg_data_generation != generation ||\n        runtime->mpeg_retire_pending_generation != 0u ||\n""",
    "runtime->stop_requested || runtime->receiver_done ||\n        runtime->mpeg_data_generation != generation",
    "RETIRE begin generation gate",
)
replace_block(
    runtime_c,
    """    if (runtime->mpeg_retire_ack_generation == generation) {\n        runtime->mpeg_retire_ack_generation = 0u;\n        runtime->mpeg_retire_pending_generation = 0u;\n        return 1;\n    }\n""",
    """    if (runtime->mpeg_retire_ack_generation == generation)\n        return 1;\n""",
    "if (runtime->mpeg_retire_ack_generation == generation)\n        return 1;",
    "RETIRE poll latches fence",
)

text = read(runtime_c)
if "int pstvnc_h1_transport_mpeg_retire_finalize(" not in text:
    anchor = """int pstvnc_h1_transport_start(\n    pstvnc_h1_transport_runtime_t *runtime)\n"""
    if text.count(anchor) != 1:
        raise SystemExit("RETIRE finalize insertion point missing")
    finalize_code = r'''int pstvnc_h1_transport_mpeg_retire_finalize(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation,
    uint32_t *bytes_discarded)
{
    size_t queued = 0u;
    uint32_t credit_amount = 0u;

    if (bytes_discarded != NULL)
        *bytes_discarded = 0u;

    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->mpeg_data_generation != 0u ||
        runtime->mpeg_retire_pending_generation != generation ||
        runtime->mpeg_retire_ack_generation != generation)
        return 0;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        queued = pstvnc_transport_queue_size(&runtime->mpeg_queue);
        if (queued > 0xffffffffu ||
            runtime->mpeg_credit_pending > 0xffffffffu - (uint32_t)queued ||
            runtime->stats.mpeg_bytes_discarded_generation_boundary >
                0xffffffffu - (uint32_t)queued) {
            (void)SignalSema(runtime->mpeg_queue_sema_id);
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        if (pstvnc_transport_queue_discard_all(&runtime->mpeg_queue) != queued) {
            (void)SignalSema(runtime->mpeg_queue_sema_id);
            h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
            return 0;
        }

        if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        credit_amount = runtime->mpeg_credit_pending + (uint32_t)queued;
        if (credit_amount != 0u &&
            !h1_send_credit(
                runtime,
                PSTVNC_TRANSPORT_CHANNEL_MPEG2,
                credit_amount))
            return 0;

        runtime->mpeg_credit_pending = 0u;
        runtime->stats.mpeg_bytes_discarded_generation_boundary +=
            (uint32_t)queued;
    } else if (runtime->config.video_mode == PSTVNC_H1_VIDEO_OFF) {
        if (runtime->mpeg_credit_pending != 0u)
            return 0;
    } else {
        return 0;
    }

    runtime->mpeg_retire_ack_generation = 0u;
    runtime->mpeg_retire_pending_generation = 0u;
    if (bytes_discarded != NULL)
        *bytes_discarded = (uint32_t)queued;
    return 1;
}

'''
    text = text.replace(anchor, finalize_code + anchor, 1)
    write(runtime_c, text)
else:
    print("ITEM11B_ALREADY=transport RETIRE finalize")

# ---------------------------------------------------------------------------
# Session coordinator: bind transport generation before START and finalize the
# ACK-latched queue boundary only after the exact old worker is stopped.
# ---------------------------------------------------------------------------
coord_c = "experiments/media-harness-h1/h1_cp2p_session_coordinator.c"
replace_block(
    coord_c,
    """int pstvnc_h1_transport_mpeg_retire_begin(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_poll(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n""",
    """int pstvnc_h1_transport_mpeg_generation_open(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_generation_abort(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_begin(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_poll(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_finalize(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation,\n    uint32_t *bytes_discarded);\n""",
    "pstvnc_h1_transport_mpeg_generation_open",
    "coordinator transport declarations",
)

text = read(coord_c)
if "h1_cp2p_session_clear_mpeg_after_pi_retire" not in text:
    anchor = """static int h1_cp2p_session_calibration_entry_gate(\n"""
    if text.count(anchor) != 1:
        raise SystemExit("coordinator clear wrapper insertion point missing")
    wrapper = r'''static int h1_cp2p_session_clear_mpeg_after_pi_retire(
    void *context,
    uint32_t generation)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    uint32_t discarded = 0u;

    if (coordinator == NULL || !coordinator->initialized ||
        coordinator->clear_mpeg == NULL || !coordinator->pi_retire_pending ||
        coordinator->pi_retire_generation != generation)
        return 0;

    /* Stop the old decoder/pixels before touching its transport queue. */
    if (!coordinator->clear_mpeg(
            coordinator->clear_mpeg_context, generation))
        return 0;

    /* ACK was the wire fence; discard only the now-unconsumed old epoch. */
    if (!pstvnc_h1_transport_mpeg_retire_finalize(
            coordinator->transport, generation, &discarded))
        return 0;

    return 1;
}

'''
    text = text.replace(anchor, wrapper + anchor, 1)
    write(coord_c, text)
else:
    print("ITEM11B_ALREADY=coordinator retirement wrapper")

replace_block(
    coord_c,
    """        coordinator->pi_retire_pending = 0;\n        coordinator->pi_retire_generation = 0u;\n    } else if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY) {\n""",
    """        /* Keep the exact retirement latched through local worker + queue cleanup. */\n    } else if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY) {\n""",
    "Keep the exact retirement latched through local worker + queue cleanup",
    "coordinator ACK latch",
)
replace_block(
    coord_c,
    """            coordinator->clear_mpeg,\n            coordinator->clear_mpeg_context,\n            &begin_result))\n        return 0;\n\n    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW) {\n""",
    """            coordinator->pi_retire_pending\n                ? h1_cp2p_session_clear_mpeg_after_pi_retire\n                : coordinator->clear_mpeg,\n            coordinator->pi_retire_pending\n                ? coordinator\n                : coordinator->clear_mpeg_context,\n            &begin_result))\n        return 0;\n\n    if (coordinator->pi_retire_pending) {\n        coordinator->pi_retire_pending = 0;\n        coordinator->pi_retire_generation = 0u;\n    }\n\n    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW) {\n""",
    "h1_cp2p_session_clear_mpeg_after_pi_retire",
    "coordinator finalize callback",
)

replace_block(
    coord_c,
    """    if (!pstvnc_h1_mpeg_start_transport_send(\n            coordinator->transport,\n            coordinator->session_id,\n            &contract)) {\n""",
    """    /*\n     * Open the PS2 channel-4 generation gate before START can reach the Pi. A\n     * very fast Pi may answer immediately on the receiver thread.\n     */\n    if (!pstvnc_h1_transport_mpeg_generation_open(\n            coordinator->transport, contract.generation)) {\n        if (coordinator->arm_mpeg != NULL &&\n            (coordinator->clear_mpeg == NULL ||\n             !coordinator->clear_mpeg(\n                 coordinator->clear_mpeg_context,\n                 contract.generation))) {\n            coordinator->current_start_contract_valid = 0;\n            return 0;\n        }\n        (void)pstvnc_h1_mpeg_start_handoff_abort_start(\n            &coordinator->mpeg_handoff, contract.generation);\n        coordinator->current_start_contract_valid = 0;\n        return 0;\n    }\n\n    if (!pstvnc_h1_mpeg_start_transport_send(\n            coordinator->transport,\n            coordinator->session_id,\n            &contract)) {\n""",
    "Open the PS2 channel-4 generation gate before START",
    "coordinator generation open",
)
replace_block(
    coord_c,
    """        (void)pstvnc_h1_mpeg_start_handoff_abort_start(\n            &coordinator->mpeg_handoff,\n            contract.generation);\n        coordinator->current_start_contract_valid = 0;\n        return 0;\n    }\n\n    coordinator->current_start_contract = contract;\n""",
    """        if (!pstvnc_h1_transport_mpeg_generation_abort(\n                coordinator->transport, contract.generation)) {\n            coordinator->current_start_contract_valid = 0;\n            return 0;\n        }\n        (void)pstvnc_h1_mpeg_start_handoff_abort_start(\n            &coordinator->mpeg_handoff,\n            contract.generation);\n        coordinator->current_start_contract_valid = 0;\n        return 0;\n    }\n\n    coordinator->current_start_contract = contract;\n""",
    "pstvnc_h1_transport_mpeg_generation_abort(\n                coordinator->transport, contract.generation)",
    "coordinator send failure generation abort",
)

# ---------------------------------------------------------------------------
# Pi producer: a closed-by-default emission lease. #10 may open it later, but
# RETIRE already closes it and waits for all in-flight sends before producer
# stop/drain, making the ACK an ordered wire fence.
# ---------------------------------------------------------------------------
producer_py = "experiments/media-harness-h1/h1_cp2p_mpeg_producer.py"
replace_block(
    producer_py,
    """import subprocess\nimport time\nfrom typing import Callable\n""",
    """import subprocess\nimport threading\nimport time\nfrom typing import Callable\n""",
    "import threading",
    "producer threading import",
)
replace_block(
    producer_py,
    """        self.producer: object | None = None\n        self.archive_path: Path | None = None\n\n    def active_generation(self) -> int:\n""",
    """        self.producer: object | None = None\n        self.archive_path: Path | None = None\n        self._emission_condition = threading.Condition()\n        self._emission_open = False\n        self._emission_in_flight = 0\n\n    def active_generation(self) -> int:\n""",
    "self._emission_condition = threading.Condition()",
    "producer emission state",
)

text = read(producer_py)
if "def open_emission_exact(" not in text:
    anchor = """    def start_exact(self, plan: H1Cp2pCapturePlan) -> dict[str, object]:\n"""
    if text.count(anchor) != 1:
        raise SystemExit("producer emission method insertion point missing")
    methods = r'''    def emission_open_for_generation(self, generation: int) -> bool:
        with self._emission_condition:
            return (
                int(generation) > 0
                and self.generation == int(generation)
                and self._emission_open
            )

    def open_emission_exact(self, generation: int) -> None:
        """Item #10 hook: permit scheduler leases for one exact live generation."""
        generation = int(generation)
        with self._emission_condition:
            if (
                generation <= 0
                or self.producer is None
                or self.generation != generation
                or self._emission_open
                or self._emission_in_flight != 0
            ):
                raise base.ProtocolError(
                    "CP2P MPEG emission-open generation/state mismatch "
                    f"active={self.generation} requested={generation}"
                )
            self._emission_open = True

    def begin_emission_exact(self, generation: int) -> object | None:
        """Acquire one scheduler lease; returns None while the public gate is closed."""
        generation = int(generation)
        with self._emission_condition:
            if generation <= 0 or self.producer is None or self.generation != generation:
                raise base.ProtocolError(
                    "CP2P MPEG emission generation mismatch "
                    f"active={self.generation} requested={generation}"
                )
            if not self._emission_open:
                return None
            self._emission_in_flight += 1
            return self.producer

    def finish_emission_exact(self, generation: int) -> None:
        generation = int(generation)
        with self._emission_condition:
            if (
                generation <= 0
                or self.generation != generation
                or self._emission_in_flight <= 0
            ):
                raise base.ProtocolError(
                    "CP2P MPEG emission-release generation/state mismatch"
                )
            self._emission_in_flight -= 1
            self._emission_condition.notify_all()

    def _close_emission_exact(self, generation: int, timeout: float) -> None:
        generation = int(generation)
        deadline = time.monotonic() + timeout
        with self._emission_condition:
            if generation <= 0 or self.producer is None or self.generation != generation:
                raise base.ProtocolError(
                    "CP2P MPEG emission-close generation mismatch "
                    f"active={self.generation} requested={generation}"
                )
            self._emission_open = False
            while self._emission_in_flight != 0:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise base.ProtocolError(
                        f"MPEG generation {generation} emission did not quiesce"
                    )
                self._emission_condition.wait(timeout=min(remaining, 0.05))

'''
    text = text.replace(anchor, methods + anchor, 1)
    write(producer_py, text)
else:
    print("ITEM11B_ALREADY=producer emission methods")

replace_block(
    producer_py,
    """            \"state\": \"live-local-producer-public-mpeg-gate-closed\",\n            \"pstv_mpeg_data_emission\": False,\n""",
    """            \"state\": \"live-local-producer-public-mpeg-gate-closed\",\n            \"pstv_mpeg_data_emission\": False,\n            \"emission_fence_open\": False,\n""",
    '"emission_fence_open": False',
    "producer evidence emission fence",
)
replace_block(
    producer_py,
    """        producer = self.producer\n        discarded = 0\n        producer.stop()\n""",
    """        producer = self.producer\n        discarded = 0\n\n        # Close scheduler admission first and wait out any send already holding\n        # a lease. The later RETIRE ACK is therefore ordered after every N send.\n        self._close_emission_exact(generation, timeout)\n        producer.stop()\n""",
    "Close scheduler admission first and wait out any send",
    "producer retirement emission fence",
)
replace_block(
    producer_py,
    """        self.producer = None\n        self.plan = None\n        self.generation = 0\n        self.archive_path = None\n        if self.attach is not None:\n""",
    """        self.producer = None\n        self.plan = None\n        self.generation = 0\n        self.archive_path = None\n        with self._emission_condition:\n            self._emission_open = False\n            self._emission_in_flight = 0\n            self._emission_condition.notify_all()\n        if self.attach is not None:\n""",
    "self._emission_condition.notify_all()",
    "producer retirement fence reset",
)
# Forced session cleanup gets the same local fence reset, but this replacement
# must target the later shutdown block only if it still lacks the reset.
text = read(producer_py)
shutdown_marker = """            self.archive_path = None\n            if self.attach is not None:\n                self.attach(None)\n"""
shutdown_new = """            self.archive_path = None\n            with self._emission_condition:\n                self._emission_open = False\n                self._emission_in_flight = 0\n                self._emission_condition.notify_all()\n            if self.attach is not None:\n                self.attach(None)\n"""
if shutdown_new not in text:
    if text.count(shutdown_marker) != 1:
        raise SystemExit("producer forced-cleanup block mismatch")
    write(producer_py, text.replace(shutdown_marker, shutdown_new, 1))
else:
    print("ITEM11B_ALREADY=producer shutdown fence reset")

# ---------------------------------------------------------------------------
# Producer host contract: prove RETIRE closes admission and waits for the exact
# in-flight send lease before even signaling the FFmpeg process.
# ---------------------------------------------------------------------------
producer_test = "experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py"
replace_block(
    producer_test,
    """from pathlib import Path\nimport tempfile\nimport unittest\n""",
    """from pathlib import Path\nimport tempfile\nimport threading\nimport time\nimport unittest\n""",
    "import threading",
    "producer test imports",
)
text = read(producer_test)
if "test_retirement_waits_for_inflight_emission_lease" not in text:
    anchor = """    def test_stuck_producer_fails_closed_after_terminate_grace(self) -> None:\n"""
    if text.count(anchor) != 1:
        raise SystemExit("producer test insertion point missing")
    test = r'''    def test_retirement_waits_for_inflight_emission_lease(self) -> None:
        fake = FakeProducer(b"queued-after-send")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(15))

        # #10 is the only future caller allowed to open this fence. Once open,
        # one scheduler lease represents DATA that may already be in send_frame.
        owner.open_emission_exact(15)
        self.assertIs(owner.begin_emission_exact(15), fake)

        result = []
        failure = []

        def retire():
            try:
                result.append(owner.retire_exact(15))
            except BaseException as exc:
                failure.append(exc)

        thread = threading.Thread(target=retire)
        thread.start()

        deadline = time.monotonic() + 1.0
        while owner.emission_open_for_generation(15) and time.monotonic() < deadline:
            time.sleep(0.001)

        self.assertFalse(owner.emission_open_for_generation(15))
        self.assertFalse(fake.stop_called)
        self.assertIsNone(owner.begin_emission_exact(15))

        owner.finish_emission_exact(15)
        thread.join(timeout=1.0)
        self.assertFalse(thread.is_alive())
        self.assertEqual(failure, [])
        self.assertEqual(len(result), 1)
        self.assertTrue(fake.stop_called)
        self.assertEqual(owner.active_generation(), 0)

    def test_emission_fence_is_closed_by_default_and_exact_generation_only(self) -> None:
        fake = FakeProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(17))
        self.assertFalse(owner.emission_open_for_generation(17))
        self.assertIsNone(owner.begin_emission_exact(17))
        with self.assertRaises(base.ProtocolError):
            owner.open_emission_exact(18)
        owner.retire_exact(17)

'''
    text = text.replace(anchor, test + anchor, 1)
    write(producer_test, text)
else:
    print("ITEM11B_ALREADY=producer emission tests")

# ---------------------------------------------------------------------------
# Coordinator host test: model transport-generation open/abort, ACK fence, old
# worker clear, queue finalize, and only then presentation retirement.
# ---------------------------------------------------------------------------
coord_test = "experiments/media-harness-h1/mpeg_presentation_calibration/h1_cp2p_session_coordinator_test.c"
replace_block(
    coord_test,
    """static int retire_poll_result;\nstatic pstvnc_h1_config_t fake_config;\n""",
    """static int retire_poll_result;\nstatic unsigned int generation_open_count;\nstatic unsigned int generation_abort_count;\nstatic unsigned int retire_finalize_count;\nstatic uint32_t transport_generation;\nstatic uint32_t finalized_generation;\nstatic uint32_t finalized_discarded;\nstatic unsigned int lifecycle_order;\nstatic unsigned int clear_order;\nstatic unsigned int finalize_order;\nstatic pstvnc_h1_config_t fake_config;\n""",
    "generation_open_count",
    "coordinator test generation globals",
)

text = read(coord_test)
if "int pstvnc_h1_transport_mpeg_generation_open(" not in text:
    anchor = """int pstvnc_h1_transport_mpeg_retire_begin(\n"""
    if text.count(anchor) != 1:
        raise SystemExit("coordinator test transport stub insertion point missing")
    stubs = r'''int pstvnc_h1_transport_mpeg_generation_open(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || transport_generation != 0u)
        return 0;
    transport_generation = generation;
    generation_open_count++;
    return 1;
}

int pstvnc_h1_transport_mpeg_generation_abort(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || transport_generation != generation)
        return 0;
    transport_generation = 0u;
    generation_abort_count++;
    return 1;
}

'''
    text = text.replace(anchor, stubs + anchor, 1)
    write(coord_test, text)
else:
    print("ITEM11B_ALREADY=coordinator test generation stubs")

replace_block(
    coord_test,
    """    if (generation == 0u || retire_begin_count != 0u)\n        return 0;\n""",
    """    if (generation == 0u || retire_begin_count != 0u ||\n        transport_generation != generation)\n        return 0;\n""",
    "transport_generation != generation",
    "coordinator test retire begin exact generation",
)
replace_block(
    coord_test,
    """    if (generation == 0u || generation != retire_generation)\n        return -1;\n    return retire_poll_result;\n}\n\nstatic int clear_mpeg(void *context, uint32_t generation)\n""",
    """    if (generation == 0u || generation != retire_generation)\n        return -1;\n    if (retire_poll_result == 1)\n        transport_generation = 0u;\n    return retire_poll_result;\n}\n\nint pstvnc_h1_transport_mpeg_retire_finalize(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation,\n    uint32_t *bytes_discarded)\n{\n    (void)runtime;\n    if (generation == 0u || generation != retire_generation ||\n        transport_generation != 0u || clear_count == 0u)\n        return 0;\n    retire_finalize_count++;\n    finalized_generation = generation;\n    finalized_discarded = 17u;\n    finalize_order = ++lifecycle_order;\n    if (bytes_discarded != NULL)\n        *bytes_discarded = finalized_discarded;\n    retire_generation = 0u;\n    return 1;\n}\n\nstatic int clear_mpeg(void *context, uint32_t generation)\n""",
    "pstvnc_h1_transport_mpeg_retire_finalize",
    "coordinator test finalize stub",
)
replace_block(
    coord_test,
    """    clear_count++;\n    cleared_generation = generation;\n    return 1;\n}\n""",
    """    clear_count++;\n    cleared_generation = generation;\n    clear_order = ++lifecycle_order;\n    return 1;\n}\n""",
    "clear_order = ++lifecycle_order",
    "coordinator test clear ordering",
)

replace_block(
    coord_test,
    """    assert(send_count == 1u);\n    assert(sent_session_id == UINT32_C(0x1234abcd));\n""",
    """    assert(send_count == 1u);\n    assert(generation_open_count == 1u);\n    assert(transport_generation == sent_contract.generation);\n    assert(sent_session_id == UINT32_C(0x1234abcd));\n""",
    "assert(generation_open_count == 1u);",
    "coordinator test first generation open",
)
replace_block(
    coord_test,
    """    assert(pstvnc_h1_mpeg_start_handoff_abort_start(\n        &coordinator.mpeg_handoff, sent_contract.generation));\n    coordinator.current_start_contract_valid = 0;\n""",
    """    assert(pstvnc_h1_mpeg_start_handoff_abort_start(\n        &coordinator.mpeg_handoff, sent_contract.generation));\n    assert(pstvnc_h1_transport_mpeg_generation_abort(\n        transport, sent_contract.generation));\n    assert(generation_abort_count == 1u);\n    coordinator.current_start_contract_valid = 0;\n""",
    "assert(generation_abort_count == 1u);",
    "coordinator test manual first abort",
)
replace_block(
    coord_test,
    """    assert(send_count == 2u);\n    assert(pstvnc_h1_mpeg_presentation_owner_state(\n        &coordinator.mpeg_handoff.owner) ==\n        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);\n    assert(!coordinator.current_start_contract_valid);\n""",
    """    assert(send_count == 2u);\n    assert(generation_open_count == 2u);\n    assert(generation_abort_count == 2u);\n    assert(transport_generation == 0u);\n    assert(pstvnc_h1_mpeg_presentation_owner_state(\n        &coordinator.mpeg_handoff.owner) ==\n        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);\n    assert(!coordinator.current_start_contract_valid);\n""",
    "assert(generation_open_count == 2u);",
    "coordinator test send-failure abort",
)
replace_block(
    coord_test,
    """    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(\n        &coordinator.mpeg_handoff, active_contract.generation));\n    coordinator.current_start_contract = active_contract;\n""",
    """    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(\n        &coordinator.mpeg_handoff, active_contract.generation));\n    assert(pstvnc_h1_transport_mpeg_generation_open(\n        transport, active_contract.generation));\n    assert(generation_open_count == 3u);\n    coordinator.current_start_contract = active_contract;\n""",
    "assert(generation_open_count == 3u);",
    "coordinator test active retire generation binding",
)
replace_block(
    coord_test,
    """    assert(!coordinator.pi_retire_pending);\n    assert(coordinator.pi_retire_generation == 0u);\n    assert(clear_count == 1u);\n    assert(cleared_generation == active_contract.generation);\n""",
    """    assert(!coordinator.pi_retire_pending);\n    assert(coordinator.pi_retire_generation == 0u);\n    assert(clear_count == 1u);\n    assert(cleared_generation == active_contract.generation);\n    assert(retire_finalize_count == 1u);\n    assert(finalized_generation == active_contract.generation);\n    assert(finalized_discarded == 17u);\n    assert(clear_order != 0u && finalize_order > clear_order);\n    assert(transport_generation == 0u);\n""",
    "assert(retire_finalize_count == 1u);",
    "coordinator test clear-then-finalize",
)

# ---------------------------------------------------------------------------
# Durable item-#11B mechanism note. History/checklist promotion waits for proof.
# ---------------------------------------------------------------------------
write_new(
    "experiments/media-harness-h1/CP2P_LIVE_GENERATION_BOUNDARY.md",
    r'''# H1 CP2P Live MPEG Generation Boundary — Item #11B

Status: implementation/proof contract for the pre-public-gate #11B tranche.

## Purpose

The H1 transport MPEG queue is intentionally one bounded session-scoped ring. Item
#11B prevents stale generation N bytes from becoming generation N+1 input without
adding a second queue, second socket, or a generation field to every MPEG DATA
frame.

## Ordered-fence mechanism

The existing one PSTV connection is ordered TCP. Pi retirement therefore closes
new scheduler admission for generation N and waits for any send already holding an
emission lease to finish **before** stopping/draining the local producer. The exact
RETIRE completion is sent only after that Pi cleanup succeeds. Every generation-N
DATA frame written before completion is therefore earlier than completion on the
same Pi->PS2 byte stream.

The sole PS2 receiver continues accepting channel-4 DATA for N until it observes
the exact RETIRE completion. When that completion is accepted it immediately
closes MPEG DATA admission. At that instant all earlier N DATA has already either
been consumed by the N worker or is resident in the existing PS2 MPEG ring; any
later channel-4 DATA is a protocol error.

The application then stops/joins the exact N worker. With no producer-side sends
in flight and no consumer reading the ring, transport finalization atomically
discards the residual N bytes under the existing MPEG queue semaphore, returns all
withheld credit (previous batched credit plus discarded queued bytes), and clears
the exact retirement latch. A new generation may open only when no prior data
generation/retirement is active, the ring is empty, and no old MPEG credit remains
pending.

This is an epoch boundary, not per-packet tagging.

## Public-gate boundary

Item #10 remains closed in this tranche. `H1Cp2pMpegProducer` therefore starts with
its emission fence closed and the CP2P runner does not call `open_emission_exact()`.
The future #10 scheduler must acquire/release the exact-generation emission lease
around each MPEG `_send_from`/PSTV write and may open the fence only for the current
prepared generation after the public MPEG CONFIG gate accepts the all-guns mode.

## Required proof

- producer emission fence is closed by default and exact-generation only;
- retirement closes admission and waits for an in-flight send lease before process
  stop/drain;
- queue discard empties wrapped data while preserving high-water telemetry;
- coordinator opens transport generation before START and aborts it on START-send
  failure;
- exact Pi ACK closes PS2 channel-4 admission but leaves retirement latched;
- old worker clear precedes residual queue discard/final-credit return;
- fresh generation cannot open until the old boundary is fully finalized;
- pinned PS2 build links the generation-open/abort/finalize and queue-discard APIs;
- public #10 gate remains closed and no live MPEG DATA transmission is claimed.
''',
    "This is an epoch boundary, not per-packet tagging.",
)

print("H1_CP2P_ITEM11B_PATCH=APPLIED")
