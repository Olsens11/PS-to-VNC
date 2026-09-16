#!/usr/bin/env python3
"""Temporary A003 evidence helper for source-owned dictionary candidates.

This file exists only on the disposable evidence branch. It mechanically
enumerates current clean definitions using repository authority, then replaces
new mechanical rows with semantic descriptions derived from A003 ownership.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path.cwd()
RECONCILE = ROOT / "scripts/reconcile-source-dictionaries.py"


def load(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


mod = load(RECONCILE, "a003_reconcile")
validator = mod.load_validator()

mpeg_dictionary = ROOT / "src/mpeg/SYMBOLS.md"
if not mpeg_dictionary.exists():
    mpeg_dictionary.write_text(
        "# Clean symbols — `src/mpeg`\n\n"
        "DIRECTORY=src/mpeg\n"
        "GENERATION=CLEAN_RECONSTRUCTION\n"
        "COVERAGE=COMPLETE\n\n"
        "This directory owns the clean MPEG decoder lifecycle and safe-stop core. "
        "It consumes only the public logical MPEG Transport seam, owns explicit "
        "caller-supplied feed/sequence/resource bounds, known-state "
        "prepare/initialize/picture/destroy ordering, decoder-call lifetime "
        "fencing, and truthful payload-versus-padded-transfer accounting. It "
        "does not own the physical PSTV receiver, exact-generation "
        "START/retirement orchestration, Pi producer/capture control, "
        "presentation/compositor work, first-presentation clock arming, "
        "scheduling/drop policy, calibration, application orchestration, or "
        "hardware qualification.\n\n"
        "The inventory below covers maintained clean-generation symbols "
        "defined directly in this directory.\n\n"
        "| Name | Kind | File | Owner | Scope | Description | Context |\n"
        "|---|---|---|---|---|---|---|\n",
        encoding="utf-8",
    )

for directory in mod.TARGET_DIRECTORIES:
    before, removed, added = mod.reconcile_dictionary(validator, directory)
    print(
        f"DICTIONARY_RECONCILED={directory} before={before} "
        f"removed={removed} added={added}"
    )

before, removed, added = mod.reconcile_dictionary(validator, Path("src/mpeg"))
print(
    f"DICTIONARY_RECONCILED=src/mpeg before={before} "
    f"removed={removed} added={added}"
)

FUNCTIONS = {
    "pstvnc_transport_runtime_channel_config_valid": "Validates bounded queue and credit authority shared by logical media channels.",
    "pstvnc_transport_runtime_mpeg_config_valid": "Validates optional MPEG queue and credit authority against the framed Transport payload limit.",
    "pstvnc_transport_runtime_accept_mpeg_frame": "Commits one validated channel-4 DATA payload into the bounded MPEG queue owned by the sole receiver.",
    "pstvnc_transport_runtime_initialize_with_mpeg": "Initializes the sole Transport runtime with explicit MPEG authority and no AUDIO channel.",
    "pstvnc_transport_runtime_initialize_with_audio_mpeg": "Initializes the sole Transport runtime with independent AUDIO and MPEG logical-channel authority.",
    "pstvnc_transport_runtime_publish_media_activity_locked": "Advances media activity under its queue lock and records whether an armed waiter must be woken.",
    "pstvnc_transport_runtime_signal_media_activity": "Signals a media activity semaphore only when the locked publication step requires a wake.",
    "pstvnc_transport_runtime_publish_media_terminal": "Publishes terminal progress to one media waiter using the queue-lock generation protocol.",
    "pstvnc_transport_runtime_publish_mpeg_terminal": "Publishes MPEG stop, close, or failure progress without inventing producer exhaustion.",
    "pstvnc_transport_runtime_media_activity_snapshot": "Snapshots a media activity generation while holding the matching queue lock.",
    "pstvnc_transport_runtime_media_wait_activity": "Performs the three-state wake-before-sleep wait for one logical media queue.",
    "pstvnc_transport_runtime_media_waiter_live": "Reports whether a media waiter still owns its activity rendezvous under the queue lock.",
    "pstvnc_transport_runtime_return_credit": "Accumulates dequeue-earned credit and emits only credit justified by bytes actually removed.",
    "pstvnc_transport_runtime_return_mpeg_credit": "Returns MPEG credit earned only by bytes actually removed from the MPEG queue.",
    "pstvnc_transport_runtime_mpeg_read_available": "Removes available MPEG bytes while keeping starvation, exhaustion, stop, close, and failure distinct.",
    "pstvnc_transport_runtime_mpeg_status": "Reports MPEG queue occupancy and the independent finite-producer completion fact.",
    "pstvnc_transport_runtime_mpeg_activity_snapshot": "Returns the MPEG activity generation used for event-driven consumer waits.",
    "pstvnc_transport_runtime_mpeg_wait_activity": "Waits for MPEG producer or terminal activity without timer-backed polling.",
    "pstvnc_transport_runtime_mpeg_mark_producer_done": "Records one-shot real MPEG producer completion and wakes a waiting consumer.",
    "pstvnc_transport_session_open_with_mpeg": "Opens the single Transport session with explicit logical MPEG channel authority.",
    "pstvnc_transport_session_open_with_audio_mpeg": "Opens the single Transport session with independent AUDIO and MPEG channel authority.",
    "pstvnc_transport_mpeg_read_available": "Exposes bounded MPEG dequeue through the public Transport bridge without exposing the socket.",
    "pstvnc_transport_mpeg_status": "Exposes MPEG occupancy and real producer completion through the public Transport bridge.",
    "pstvnc_transport_mpeg_activity_snapshot": "Exposes the MPEG activity generation used for safe event-driven waiting.",
    "pstvnc_transport_mpeg_wait_activity": "Exposes event-driven MPEG activity waiting through the public Transport bridge.",
    "pstvnc_transport_mpeg_mark_producer_done": "Records explicit finite MPEG producer completion through the public Transport bridge.",
    "pstvnc_transport_mpeg_channel_initialize": "Binds caller storage as an empty bounded MPEG ring queue with producer completion clear.",
    "pstvnc_transport_mpeg_channel_commit_data": "Appends one nonempty MPEG DATA payload atomically when bounded capacity permits.",
    "pstvnc_transport_mpeg_channel_mark_producer_done": "Records finite MPEG producer completion separately from Transport terminal state.",
    "pstvnc_transport_mpeg_channel_read_available": "Removes currently queued MPEG bytes while preserving bounded ring-buffer order.",
    "pstvnc_transport_mpeg_channel_available": "Reports current MPEG payload-byte occupancy of the bounded queue.",
    "pstvnc_transport_mpeg_channel_producer_done": "Reports the independent one-shot finite MPEG producer completion fact.",
    "pstvnc_mpeg_decoder_alignment_valid": "Validates an injected transfer or buffer alignment as usable power-of-two authority.",
    "pstvnc_mpeg_decoder_config_valid": "Validates caller-supplied sequence, feed, pixel-width, and alignment bounds without defaults.",
    "pstvnc_mpeg_decoder_memory_ops_valid": "Verifies the injected decoder memory allocation boundary is complete.",
    "pstvnc_mpeg_decoder_sync_ops_valid": "Verifies the injected decoder lifetime synchronization boundary is complete.",
    "pstvnc_mpeg_decoder_platform_ops_valid": "Verifies the injected known-state, decoder, transfer, and destruction boundary is complete.",
    "pstvnc_mpeg_decoder_align_up": "Rounds a payload size to explicit transfer alignment while rejecting integer overflow.",
    "pstvnc_mpeg_decoder_picture_capacity": "Computes bounded macroblock-rounded picture capacity from explicit caller limits.",
    "pstvnc_mpeg_decoder_free_buffers": "Releases feed and picture buffers only after decoder-call ownership has returned.",
    "pstvnc_mpeg_decoder_observe_stop": "Reads owner stop only at a completed decoder-call boundary under the state lock.",
    "pstvnc_mpeg_decoder_set_call_active": "Publishes acquisition or release of decoder-call ownership under the state lock.",
    "pstvnc_mpeg_decoder_call_is_active": "Reads decoder-call ownership before any decoder-visible resource reclamation.",
    "pstvnc_mpeg_decoder_record_transport_failure": "Retains the first non-exhaustion Transport result observed by decoder feed.",
    "pstvnc_mpeg_decoder_feed": "Feeds an active decoder from real MPEG data, real exhaustion, or real failure without consulting local stop.",
    "pstvnc_mpeg_decoder_sequence": "Validates decoder-reported dimensions against caller authority before surface use.",
    "pstvnc_mpeg_decoder_initialize": "Allocates bounded resources and orders known-state preparation before decoder initialization.",
    "pstvnc_mpeg_decoder_request_stop": "Records owner cancellation without converting it into decoder feed EOF.",
    "pstvnc_mpeg_decoder_run": "Owns one decoder picture call, classifies its truthful outcome, then honors pending stop.",
    "pstvnc_mpeg_decoder_release": "Destroys decoder state and reclaims resources only after active call ownership ends.",
}

SPECIAL = {
    "pstvnc_transport_mpeg_channel_config": "Groups explicit caller-owned MPEG queue and credit authority with no guessed production defaults.",
    "pstvnc_transport_mpeg_channel_config_t": "Names the caller-owned MPEG queue and credit configuration contract.",
    "pstvnc_transport_mpeg_channel": "Owns bounded MPEG ring-buffer state and the independent finite-producer completion fact.",
    "pstvnc_transport_mpeg_channel_t": "Names the Transport-owned bounded MPEG logical-channel state.",
    "pstvnc_mpeg_decoder_config": "Groups explicit caller sequence, pixel, feed-payload, and alignment bounds for one decoder owner.",
    "pstvnc_mpeg_decoder_config_t": "Names the caller-owned decoder resource and sequence-bound authority.",
    "pstvnc_mpeg_decoder_memory_ops": "Defines the injected aligned memory boundary used by decoder-owned buffers.",
    "pstvnc_mpeg_decoder_memory_ops_t": "Names the injected decoder buffer allocation and release table.",
    "pstvnc_mpeg_decoder_sync_ops": "Defines the lock boundary protecting owner stop and active-call lifetime state.",
    "pstvnc_mpeg_decoder_sync_ops_t": "Names the decoder owner synchronization table.",
    "pstvnc_mpeg_decoder_platform_ops": "Defines known-state, decoder-call, feed-transfer, destruction, and state-release operations.",
    "pstvnc_mpeg_decoder_platform_ops_t": "Names the injected decoder platform operation table.",
    "pstvnc_mpeg_decoder_result": "Enumerates truthful completion, cancellation, Transport, bounds, platform, and lifecycle outcomes.",
    "pstvnc_mpeg_decoder_result_t": "Names the decoder terminal-result classification.",
    "pstvnc_mpeg_decoder_report": "Collects validated sequence, picture/feed counts, payload/transfer accounting, and Transport outcome.",
    "pstvnc_mpeg_decoder_report_t": "Names the caller-visible decoder execution report.",
    "pstvnc_mpeg_decoder": "Owns decoder resources, injected boundaries, stop state, active-call lifetime, feed state, and report.",
    "pstvnc_mpeg_decoder_t": "Names the session-scoped MPEG decoder owner state.",
    "pstvnc_mpeg_feed_callback_t": "Names the decoder data callback that supplies Transport bytes or truthful EOF/error.",
    "pstvnc_mpeg_sequence_callback_t": "Names the sequence callback that validates dimensions before picture-surface use.",
    "PSTVNC_MPEG_DECODER_H": "Guards the clean MPEG decoder ownership interface declarations.",
    "PSTVNC_TRANSPORT_MPEG_CHANNEL_H": "Guards the bounded logical MPEG channel declarations.",
}

ENUMS = {
    "PSTVNC_MPEG_DECODER_COMPLETE": "Reports real finite MPEG exhaustion after a normal decoder return.",
    "PSTVNC_MPEG_DECODER_STOPPED": "Reports owner cancellation observed only outside an active decoder call.",
    "PSTVNC_MPEG_DECODER_INVALID": "Reports invalid decoder arguments, configuration, or owner state.",
    "PSTVNC_MPEG_DECODER_ALLOCATION_FAILED": "Reports failure to acquire a bounded decoder-owned buffer.",
    "PSTVNC_MPEG_DECODER_PREPARE_FAILED": "Reports failure to establish required known decoder platform state.",
    "PSTVNC_MPEG_DECODER_INITIALIZE_FAILED": "Reports failure from injected decoder initialization.",
    "PSTVNC_MPEG_DECODER_SEQUENCE_INVALID": "Reports sequence dimensions outside explicit caller bounds.",
    "PSTVNC_MPEG_DECODER_TRANSPORT_FAILED": "Reports a real non-exhaustion Transport result from decoder feed.",
    "PSTVNC_MPEG_DECODER_TRANSFER_FAILED": "Reports failure submitting one aligned feed transfer.",
    "PSTVNC_MPEG_DECODER_PICTURE_FAILED": "Reports an explicit failure from the decoder picture operation.",
    "PSTVNC_MPEG_DECODER_UNEXPECTED_END": "Reports decoder return without real exhaustion or owner cancellation.",
    "PSTVNC_MPEG_DECODER_SYNC_FAILED": "Reports failure synchronizing owner stop or active-call state.",
    "PSTVNC_MPEG_DECODER_CALL_ACTIVE": "Refuses release while a decoder call still owns decoder-visible resources.",
    "PSTVNC_MPEG_DECODER_DESTROY_FAILED": "Reports failure destroying an initialized decoder after call ownership returns.",
    "PSTVNC_MPEG_DECODER_STATE_RELEASE_FAILED": "Reports failure releasing previously prepared decoder platform state.",
    "PSTVNC_MPEG_DECODER_ACCOUNTING_FAILED": "Reports overflow or inconsistency in payload-versus-transfer accounting.",
}

MEANING = {
    "runtime": "the sole Transport runtime instance",
    "config": "explicit caller-owned configuration authority",
    "mpeg_config": "explicit MPEG queue and credit authority",
    "audio_config": "independent AUDIO queue and credit authority",
    "socket_fd": "the already-owned physical PSTV socket descriptor",
    "queue_capacity": "bounded logical queue capacity in bytes",
    "initial_credit_bytes": "initial producer credit authorized by the caller",
    "credit_batch_bytes": "the dequeue-credit batching threshold",
    "credit_flush_on_empty": "whether earned credit flushes when the queue empties",
    "credit_return_enabled": "whether dequeue-earned credit is returned",
    "activity_sequence": "the producer or terminal activity generation",
    "current_sequence": "the activity generation read under the queue lock",
    "observed_sequence": "the generation previously observed by the consumer",
    "activity_wait_armed": "the three-state waiter ownership marker",
    "waiter_live": "whether the consumer still owns its activity rendezvous",
    "queue_semaphore_id": "the lock serializing queue and waiter-visible state",
    "activity_semaphore_id": "the event semaphore used for producer-driven wakeups",
    "signal_waiter": "whether the published activity must wake an armed consumer",
    "accepted": "whether the state transition preserved channel invariants",
    "header": "the validated framed Transport header",
    "payload": "the real unpadded MPEG payload bytes",
    "payload_length": "the real unpadded MPEG payload byte count",
    "buffer": "the caller destination for logical MPEG bytes",
    "maximum_count": "the maximum byte count the caller permits this dequeue",
    "read_count": "the payload bytes actually removed from Transport",
    "available_count": "the current MPEG queue occupancy in payload bytes",
    "producer_done": "the independent finite-producer completion fact",
    "queue_empty": "whether the channel queue became empty after dequeue",
    "taken": "the bytes actually removed and therefore eligible to earn credit",
    "consumed": "the actual dequeued byte count eligible to earn credit",
    "pending": "earned but not yet emitted dequeue credit",
    "batch_bytes": "the configured credit batching threshold",
    "flush_on_empty": "the configured credit flush-on-empty policy",
    "return_enabled": "whether credit emission is enabled for the channel",
    "amount": "the exact earned credit amount selected for emission",
    "channel": "the bounded MPEG logical channel being inspected or mutated",
    "storage": "caller-owned byte storage backing the MPEG ring queue",
    "capacity": "the bounded byte capacity of the MPEG ring queue",
    "read_offset": "the ring offset of the next MPEG byte to remove",
    "byte_count": "the number of real MPEG payload bytes currently queued",
    "destination": "the caller buffer receiving queued MPEG bytes",
    "count": "the actual MPEG byte count selected for removal",
    "first_count": "the first contiguous ring span handled before wraparound",
    "write_offset": "the ring offset where the next MPEG payload byte is appended",
    "decoder": "the decoder owner whose resources and call state are controlled",
    "active": "the decoder-call ownership state being published or observed",
    "observed": "the synchronized owner-state value returned to the caller",
    "alignment": "the explicit caller-supplied byte alignment",
    "value": "the unaligned byte count to round safely",
    "aligned_value": "the checked aligned byte count returned to the caller",
    "mask": "the power-of-two rounding mask derived from alignment",
    "context": "callback context carrying the owning decoder instance",
    "transfer_bytes": "the padded transfer size submitted to the platform decoder",
    "transport_result": "the truthful MPEG Transport outcome seen by decoder feed",
    "memory_ops": "the injected allocation/release boundary for decoder buffers",
    "sync_ops": "the injected synchronization boundary for stop and call lifetime",
    "platform_ops": "the injected known-state and decoder platform boundary",
    "feed_transfer_capacity": "aligned feed-buffer capacity derived from caller limits",
    "picture_capacity": "bounded macroblock-rounded picture-surface capacity",
    "stop_requested": "owner cancellation observed only outside active decoder calls",
    "bytes": "the checked byte count required for the picture surface",
    "macroblock_width": "sequence width rounded to the decoder macroblock grid",
    "macroblock_height": "sequence height rounded to the decoder macroblock grid",
    "macroblocks": "the checked macroblock count required by the picture surface",
    "result": "the first truthful Transport failure retained by the decoder owner",
    "report": "caller-visible decoder sequence, feed, picture, and accounting report",
    "picture_result": "decoder picture-call return code classified after ownership ends",
    "width": "decoder-reported sequence width validated before surface use",
    "height": "decoder-reported sequence height validated before surface use",
    "required": "the checked picture bytes required by reported sequence dimensions",
    "sequence_seen": "whether a valid sequence callback established picture dimensions",
    "sequence_invalid": "whether a sequence callback violated caller bounds",
    "feed_exhausted": "whether real finite-producer exhaustion reached active decoder feed",
    "feed_failed": "whether decoder feed observed a non-exhaustion Transport failure",
    "feed_transport_result": "the exact retained Transport failure classification",
    "decoder_initialized": "whether injected decoder state requires destruction",
    "known_state_prepared": "whether known platform state was acquired and requires release",
    "decoder_call_active": "whether a picture call currently owns decoder-visible resources",
    "initialized": "whether decoder resource and state acquisition completed",
    "feed_buffer": "the decoder-owned aligned feed staging buffer",
    "picture_buffer": "the decoder-owned bounded picture surface",
    "feed_payload_capacity": "maximum real payload bytes one feed callback may consume",
    "transfer_alignment": "alignment applied only after real payload accounting",
    "buffer_alignment": "required alignment for decoder-visible allocations",
    "max_width": "caller-authorized maximum sequence width",
    "max_height": "caller-authorized maximum sequence height",
    "bytes_per_pixel": "caller-authorized decoded picture byte width per pixel",
    "allocate": "the injected aligned allocation operation",
    "release": "the injected resource release operation",
    "lock": "the injected lock operation protecting owner state",
    "unlock": "the matching injected owner-state unlock operation",
    "prepare_known_state": "the operation establishing known decoder/IPU-DMAC state",
    "initialize": "the platform decoder initialization operation",
    "picture": "the decoder picture call that temporarily owns visible resources",
    "submit_feed": "the operation receiving a padded transfer after payload accounting",
    "destroy": "the operation destroying initialized decoder state after call return",
    "release_known_state": "the operation releasing prepared platform decoder state",
    "sequence_width": "validated decoder sequence width reported to the caller",
    "sequence_height": "validated decoder sequence height reported to the caller",
    "pictures_decoded": "successful decoder picture calls reported by the core",
    "feed_callbacks": "decoder data-callback requests serviced by the core",
    "payload_bytes_consumed": "sum of real MPEG payload bytes removed from Transport",
    "transfer_bytes_submitted": "sum of padded aligned bytes submitted to the decoder",
}


def description(name: str, kind: str, owner: str) -> str:
    if name in ENUMS:
        return ENUMS[name]
    if name in SPECIAL:
        return SPECIAL[name]
    if name in FUNCTIONS:
        base = FUNCTIONS[name]
        if kind == "function declaration":
            return "Declares the interface that " + base[0].lower() + base[1:]
        return base
    semantic = MEANING.get(name, name.replace("_", " "))
    if kind in {"parameter", "prototype parameter"}:
        return f"Supplies {semantic} to {owner}."
    if kind == "field":
        return f"Stores {semantic} as state owned by {owner}."
    if kind in {"variable", "constant", "array"}:
        return f"Tracks {semantic} while {owner} executes."
    if kind in {"structure", "type", "enum"}:
        return f"Represents {semantic} in the bounded A003 Transport/MPEG ownership contract."
    if kind in {"macro", "include guard", "include-guard macro"}:
        return f"Names or guards {semantic} required by the A003 Transport/MPEG interface."
    return f"Represents {semantic} within {owner} for the bounded A003 Transport/MPEG core."


def semanticize(path: Path) -> None:
    lines = path.read_text(encoding="utf-8").splitlines()
    out: list[str] = []
    for line in lines:
        if line.startswith("| ") and "mechanically reconciled current clean source" in line:
            cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
            if len(cells) != 7:
                raise RuntimeError(f"malformed dictionary row in {path}: {line}")
            name, kind, _file, owner, _scope, _description, _context = cells
            cells[5] = description(name, kind, owner)
            cells[6] = "A003 MPEG Transport/decoder core"
            line = "| " + " | ".join(cells) + " |"
        out.append(line)
    path.write_text("\n".join(out).rstrip() + "\n", encoding="utf-8")


semanticize(ROOT / "src/transport/SYMBOLS.md")
semanticize(ROOT / "src/mpeg/SYMBOLS.md")

transport = ROOT / "src/transport/SYMBOLS.md"
text = transport.read_text(encoding="utf-8")
old = (
    "This directory owns the one physical PSTV stream, sole receive owner, "
    "framed sequence/serialized send, logical RFB storage and credit, explicit "
    "finite-session quiescence, and Transport-owned fatal-session convergence."
)
new = (
    "This directory owns the one physical PSTV stream and sole receive owner; "
    "framed sequence and serialized send; independent bounded logical RFB, "
    "AUDIO, and MPEG storage/credit authority; producer-driven media activity "
    "wakeups; explicit finite-producer MPEG completion; finite-session "
    "quiescence; and Transport-owned terminal convergence. It does not own "
    "MPEG decoder resources, exact-generation START/retirement orchestration, "
    "presentation, or application policy."
)
if old not in text:
    raise RuntimeError("Transport dictionary preamble authority changed")
transport.write_text(text.replace(old, new, 1), encoding="utf-8")
