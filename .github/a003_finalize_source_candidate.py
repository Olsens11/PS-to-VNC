#!/usr/bin/env python3
from pathlib import Path

TRANSPORT = Path('src/transport/SYMBOLS.md')
MPEG = Path('src/mpeg/SYMBOLS.md')
TOPOLOGY = Path('docs/development/source-topology.md')
CONTINUITY = Path('scripts/continuity-check.sh')

FUNCTIONS = {
    'pstvnc_transport_audio_channel_available': 'Reports current AUDIO payload-byte occupancy of the bounded queue.',
    'pstvnc_transport_audio_channel_commit_data': 'Appends one nonempty AUDIO DATA payload atomically when bounded capacity permits.',
    'pstvnc_transport_audio_channel_initialize': 'Binds caller storage as an empty bounded AUDIO ring queue with producer completion clear.',
    'pstvnc_transport_audio_channel_mark_producer_done': 'Records finite AUDIO producer completion separately from Transport terminal state.',
    'pstvnc_transport_audio_channel_producer_done': 'Reports the independent one-shot finite AUDIO producer completion fact.',
    'pstvnc_transport_audio_channel_read_available': 'Removes currently queued AUDIO bytes while preserving bounded ring-buffer order.',
    'pstvnc_transport_audio_activity_snapshot': 'Exposes the AUDIO activity generation used for wake-before-sleep consumer waiting.',
    'pstvnc_transport_audio_read_available': 'Exposes bounded AUDIO dequeue through the public Transport bridge without exposing the physical socket.',
    'pstvnc_transport_audio_status': 'Exposes AUDIO queue occupancy and the independent finite-producer completion fact.',
    'pstvnc_transport_audio_wait_activity': 'Exposes event-driven AUDIO activity waiting through the public Transport bridge.',
    'pstvnc_transport_session_open_with_audio': 'Opens the single Transport session with explicit logical AUDIO channel authority.',
    'pstvnc_transport_runtime_accept_audio_frame': 'Commits one validated channel-3 DATA payload into the bounded AUDIO queue owned by the sole receiver.',
    'pstvnc_transport_runtime_audio_activity_snapshot': 'Returns the AUDIO activity generation used for event-driven consumer waits.',
    'pstvnc_transport_runtime_audio_config_valid': 'Validates optional AUDIO queue and credit authority against the framed Transport payload limit.',
    'pstvnc_transport_runtime_audio_read_available': 'Removes available AUDIO bytes while keeping starvation, exhaustion, stop, close, and failure distinct.',
    'pstvnc_transport_runtime_audio_status': 'Reports AUDIO queue occupancy and the independent finite-producer completion fact.',
    'pstvnc_transport_runtime_audio_wait_activity': 'Waits for AUDIO producer or terminal activity without timer-backed polling.',
    'pstvnc_transport_runtime_initialize_with_audio': 'Initializes the sole Transport runtime with explicit AUDIO authority and no MPEG channel.',
    'pstvnc_transport_runtime_publish_audio_terminal': 'Publishes AUDIO stop, close, or failure progress without inventing producer exhaustion.',
    'pstvnc_transport_runtime_return_audio_credit': 'Returns AUDIO credit earned only by bytes actually removed from the AUDIO queue.',
    'pstvnc_transport_runtime_publish_rfb_activity_locked': 'Advances RFB activity under the RFB queue lock and records whether an armed reader must be woken.',
    'pstvnc_transport_runtime_return_rfb_credit': 'Returns RFB credit earned only by bytes actually removed from the RFB queue.',
    'pstvnc_transport_runtime_signal_rfb_activity': 'Signals the RFB activity semaphore only when locked publication requires a wake.',
    'pstvnc_transport_runtime_reset_identifiers': 'Resets thread and semaphore identifiers to the unowned sentinel state used by rollback and release.',
    'pstvnc_transport_session_open_internal': 'Centralizes one physical Transport-session open while attaching only explicitly requested logical AUDIO/MPEG channels.',
    'pstvnc_transport_runtime_initialize_internal': 'Acquires one sole-receiver Transport runtime plus whichever logical AUDIO/MPEG channels the caller explicitly enables.',
}

SPECIAL = {
    'PSTVNC_TRANSPORT_AUDIO_CHANNEL_H': 'Guards the bounded logical AUDIO channel declarations.',
    'pstvnc_transport_audio_channel': 'Owns bounded AUDIO ring-buffer state plus its independent finite-producer completion fact.',
    'pstvnc_transport_audio_channel_t': 'Names the Transport-owned bounded AUDIO logical-channel state.',
    'pstvnc_transport_audio_channel_config': 'Groups explicit caller-owned AUDIO queue and dequeue-credit authority with no hidden defaults.',
    'pstvnc_transport_audio_channel_config_t': 'Names the explicit caller-owned AUDIO queue and credit configuration contract.',
    'PSTVNC_TRANSPORT_EXHAUSTED': 'Identifies normal finite-producer exhaustion after a logical queue is empty, distinct from stop, close, failure, and temporary starvation.',
    'PSTVNC_TRANSPORT_STOPPED': 'Identifies owner-requested Transport cancellation, distinct from normal producer exhaustion.',
}

SEMANTICS = {
    'activity_semaphore_id': 'the event semaphore used for producer-driven consumer wakeups',
    'activity_sequence': 'the producer or terminal activity generation used by wake-before-sleep logic',
    'activity_wait_armed': 'the three-state waiter ownership marker used to avoid lost wakeups and unsafe reclaim',
    'accepted': 'whether the current framed operation preserved logical-channel invariants',
    'amount': 'the exact dequeue-earned CREDIT amount selected for emission',
    'audio_config': 'explicit AUDIO queue and credit authority',
    'audio_wake_ok': 'whether stop publication made AUDIO terminal state event-visible',
    'available_count': 'the current logical queue occupancy in real payload bytes',
    'batch_bytes': 'the configured threshold for batching dequeue-earned CREDIT',
    'buffer': 'the caller destination buffer for dequeued logical payload bytes',
    'capacity': 'the bounded byte capacity of the logical ring queue',
    'channel': 'the logical channel identifier or bounded channel state required by the operation',
    'config': 'explicit caller-owned Transport configuration and bounds',
    'consumed': 'the actual dequeued byte count eligible to earn CREDIT',
    'count': 'the actual queued payload-byte count selected for removal',
    'credit_batch_bytes': 'the caller-authorized dequeue-credit batching threshold',
    'credit_flush_on_empty': 'the caller-authorized policy for flushing earned credit when the queue empties',
    'credit_pending': 'dequeue-earned bytes accumulated but not yet emitted as CREDIT',
    'credit_return_enabled': 'whether bytes actually dequeued earn CREDIT returns',
    'current_sequence': 'the activity generation read while holding the corresponding queue lock',
    'destination': 'the caller destination buffer for dequeued logical payload bytes',
    'enabled': 'whether the selected logical media channel exists and owns queue/wait state',
    'first_count': 'the first contiguous ring-buffer span handled before wraparound',
    'flush_on_empty': 'the configured policy for flushing dequeue-earned CREDIT when the queue becomes empty',
    'header': 'the validated framed Transport header received by the sole physical receiver',
    'initial_credit_bytes': 'the caller-authorized initial producer credit',
    'initialized': 'whether Transport runtime acquisition completed so rollback and release touch only owned state',
    'maximum_count': 'the maximum queued payload-byte count this dequeue may remove',
    'mpeg_config': 'explicit MPEG queue and credit authority',
    'mpeg_wake_ok': 'whether stop publication made MPEG terminal state event-visible',
    'observed_sequence': 'the activity generation previously observed by the consumer',
    'payload': 'the real logical DATA payload bytes excluding framing and decoder padding',
    'payload_length': 'the real logical DATA payload byte count',
    'pending': 'the accumulator of earned but not yet emitted CREDIT bytes',
    'producer_done': 'the independent one-shot finite-producer completion fact',
    'queue_capacity': 'the bounded logical queue capacity in bytes',
    'queue_empty': 'whether the logical queue is empty after the current dequeue',
    'queue_semaphore_id': 'the lock serializing queue data and waiter-visible state',
    'queue_storage': 'the runtime-owned backing storage for the bounded logical queue',
    'read_count': 'the number of payload bytes actually removed from the logical queue',
    'read_offset': 'the ring offset of the next payload byte to remove',
    'return_enabled': 'whether dequeue-earned CREDIT emission is enabled for this logical channel',
    'runtime': 'the sole-receiver Transport runtime instance',
    'shutdown_ok': 'whether physical-stream shutdown succeeded during the stop request',
    'signal_waiter': 'whether the newly published activity generation must wake an armed consumer',
    'snapshot': 'the caller output receiving the activity generation snapshot',
    'socket_fd': 'the already-owned physical PSTV socket descriptor',
    'storage': 'caller-provided backing storage for the bounded ring queue',
    'taken': 'the number of bytes actually removed and therefore eligible to earn CREDIT',
    'waiter_live': 'whether the consumer still owns an activity rendezvous that release must preserve',
    'write_offset': 'the ring offset where the next payload byte is appended',
}


def domain(name: str, file_name: str, owner: str) -> str:
    joined = f'{name} {file_name} {owner}'.lower()
    if '_rfb_' in joined or 'rfb_channel' in joined:
        return 'rfb'
    if ('audio_channel' in file_name or '_audio_' in joined or name.startswith('audio_')) and 'audio_mpeg' not in joined:
        return 'audio'
    if 'mpeg_channel' in file_name or '_mpeg_' in joined or name.startswith('mpeg_'):
        return 'mpeg'
    return 'shared'


def context(which: str) -> str:
    return {
        'rfb': 'A001 Transport',
        'audio': 'A002 AUDIO Transport core',
        'mpeg': 'A003 MPEG Transport core',
        'shared': 'A003 shared media Transport core',
    }[which]


def label(which: str) -> str:
    return {'rfb': 'RFB', 'audio': 'AUDIO', 'mpeg': 'MPEG', 'shared': 'logical media'}[which]


def description(name, kind, file_name, owner, old, which):
    if name in FUNCTIONS:
        value = FUNCTIONS[name]
        if kind == 'function declaration':
            return 'Declares the interface that ' + value[0].lower() + value[1:]
        return value
    if name in SPECIAL:
        return SPECIAL[name]

    semantic = SEMANTICS.get(name)
    if semantic is not None:
        if name == 'channel' and 'return_credit' in owner:
            semantic = 'the framed logical channel identifier whose actual dequeues earned this CREDIT'
        if file_name.endswith('audio_channel.c') or file_name.endswith('audio_channel.h'):
            semantic = semantic.replace('logical payload', 'AUDIO payload').replace('logical ring', 'AUDIO ring')
        if file_name.endswith('mpeg_channel.c') or file_name.endswith('mpeg_channel.h'):
            semantic = semantic.replace('logical payload', 'MPEG payload').replace('logical ring', 'MPEG ring')
        if kind in ('parameter', 'prototype parameter'):
            return f'Supplies {semantic} to {owner}.'
        if kind == 'field':
            return f'Stores {semantic} as state owned by {owner}.'
        if kind in ('variable', 'constant', 'array'):
            return f'Tracks {semantic} while {owner} executes.'

    value = old
    if which == 'audio':
        value = value.replace('MPEG', 'AUDIO').replace('channel-4', 'channel-3')

    if value.startswith('Represents ') or value.startswith('Defines '):
        readable = name.replace('pstvnc_transport_', '').replace('pstvnc_', '').replace('_', ' ')
        if kind == 'function declaration':
            return f'Declares the {label(which)} interface operation for {readable}.'
        if kind == 'function':
            return f'Implements the {label(which)} Transport operation for {readable}.'
        if kind in ('structure', 'type', 'enum', 'enum value', 'macro', 'include guard'):
            return f'Names {readable} within the {label(which)} Transport ownership contract.'
    return value


lines = TRANSPORT.read_text(encoding='utf-8').splitlines()
out = []
repaired = 0
for line in lines:
    if line.startswith('| ') and line.endswith(' |'):
        cells = [cell.strip() for cell in line.strip().strip('|').split('|')]
        if len(cells) == 7 and cells[6] == 'A003 MPEG Transport/decoder core':
            name, kind, file_name, owner, scope, old, _ = cells
            which = domain(name, file_name, owner)
            cells[5] = description(name, kind, file_name, owner, old, which)
            cells[6] = context(which)
            line = '| ' + ' | '.join(cells) + ' |'
            repaired += 1
    out.append(line)
TRANSPORT.write_text('\n'.join(out).rstrip() + '\n', encoding='utf-8')
print(f'TRANSPORT_ROWS_REPAIRED={repaired}')

mpeg_text = MPEG.read_text(encoding='utf-8')
mpeg_text = mpeg_text.replace(
    '| call_active | variable | src/mpeg/decoder.c | pstvnc_mpeg_decoder_release | local | Tracks call active while pstvnc_mpeg_decoder_release executes. | A003 MPEG Transport/decoder core |',
    '| call_active | variable | src/mpeg/decoder.c | pstvnc_mpeg_decoder_release | local | Tracks whether a decoder picture call still owns decoder-visible resources and therefore forbids reclamation. | A003 MPEG Transport/decoder core |',
)
MPEG.write_text(mpeg_text, encoding='utf-8')

if '| `src/mpeg/` |' not in TOPOLOGY.read_text(encoding='utf-8'):
    raise SystemExit('MPEG topology policy adoption is missing from candidate')

continuity = CONTINUITY.read_text(encoding='utf-8')
if '    "src/mpeg",' not in continuity:
    needle = '    "src/media",\n    "src/platform",'
    if needle not in continuity:
        raise SystemExit('continuity topology insertion anchor changed')
    continuity = continuity.replace(needle, '    "src/media",\n    "src/mpeg",\n    "src/platform",', 1)
    CONTINUITY.write_text(continuity, encoding='utf-8')

# Reject only mechanically tautological descriptions, not useful prose that
# happens to start with a legitimate verb such as "Defines the injected...".
for path in (TRANSPORT, MPEG):
    for line in path.read_text(encoding='utf-8').splitlines():
        if not line.startswith('| '):
            continue
        if '| A003 MPEG Transport/decoder core |' in line and (
            '| Represents pstvnc' in line or '| Defines pstvnc' in line
        ):
            raise SystemExit(f'tautological A003 dictionary row remains: {line}')
        if 'audio_channel' in line.lower() and 'MPEG payload' in line:
            raise SystemExit(f'AUDIO row mislabeled as MPEG: {line}')

print('A003_SOURCE_CANDIDATE_FINALIZER=PASS')
