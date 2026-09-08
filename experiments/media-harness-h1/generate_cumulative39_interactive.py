from pathlib import Path
import re

ROOT = Path(".")
H1 = ROOT / "experiments/media-harness-h1"
INC = H1 / "cumulative39-interactive-include"
INC.mkdir(parents=True, exist_ok=True)

def write(path, text):
    from textwrap import dedent
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(dedent(text).lstrip())
    print(f"WROTE={path}")

def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"ABORT={label}_COUNT_{count}")
    return text.replace(old, new, 1)

def replace_function(text, signature, replacement):
    from textwrap import dedent
    start = text.find(signature)
    if start < 0:
        raise SystemExit(f"ABORT=FUNCTION_NOT_FOUND:{signature}")
    brace = text.find("{", start)
    if brace < 0:
        raise SystemExit(f"ABORT=FUNCTION_BRACE_NOT_FOUND:{signature}")
    depth = 0
    end = None
    for i in range(brace, len(text)):
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end is None:
        raise SystemExit(f"ABORT=FUNCTION_END_NOT_FOUND:{signature}")
    return text[:start] + dedent(replacement).strip() + text[end:]

# Build-specific H1 transport header: same identity, reserved RFB lane activated.
header = (H1 / "h1_transport_runtime.h").read_text()
header = replace_once(
    header,
    '#define PSTVNC_H1_SERVER_PORT 5902\n',
    '''#define PSTVNC_H1_SERVER_PORT 5902
#define PSTVNC_H1_RFB_QUEUE_CAPACITY 32768u
#define PSTVNC_H1_RFB_CREDIT_BATCH_BYTES 4096u
''',
    "HEADER_CONSTANTS",
)
header = replace_once(
    header,
    '''    int audio_queue_sema_id;
    int mpeg_queue_sema_id;
    int send_sema_id;
''',
    '''    int audio_queue_sema_id;
    int mpeg_queue_sema_id;
    int rfb_queue_sema_id;
    int send_sema_id;
''',
    "HEADER_SEMAS",
)
header = replace_once(
    header,
    '''    uint32_t audio_credit_pending;
    uint32_t mpeg_credit_pending;

    pstvnc_transport_queue_t audio_queue;
    pstvnc_transport_queue_t mpeg_queue;

    uint8_t *audio_queue_storage;
    uint8_t *mpeg_queue_storage;
''',
    '''    uint32_t audio_credit_pending;
    uint32_t mpeg_credit_pending;
    uint32_t rfb_credit_pending;

    pstvnc_transport_queue_t audio_queue;
    pstvnc_transport_queue_t mpeg_queue;
    pstvnc_transport_queue_t rfb_queue;

    uint8_t *audio_queue_storage;
    uint8_t *mpeg_queue_storage;
    uint8_t *rfb_queue_storage;
''',
    "HEADER_RFB_QUEUE",
)
header = replace_once(
    header,
    '''size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_audio_read(
''',
    '''size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime);

size_t pstvnc_h1_transport_rfb_queue_size(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_rfb_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read);

int pstvnc_h1_transport_rfb_send(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *buffer,
    size_t byte_count);

int pstvnc_h1_transport_audio_read(
''',
    "HEADER_RFB_API",
)
write(INC / "h1_transport_runtime.h", header)

# Build-specific config: allow the already-reserved RFB mode.
config = (H1 / "h1_config.c").read_text()
config = replace_once(
    config,
    '''        config->video_mode > PSTVNC_H1_VIDEO_MPEG2_ES ||
        config->rfb_mode != PSTVNC_H1_RFB_OFF)
''',
    '''        config->video_mode > PSTVNC_H1_VIDEO_MPEG2_ES ||
        config->rfb_mode > PSTVNC_H1_RFB_ON_RESERVED)
''',
    "CONFIG_RFB_ENABLE",
)
write(H1 / "h1_config_cumulative39_interactive.c", config)

# Build-specific transport: channel 1 gets a fixed 32 KiB credited byte queue.
transport = (H1 / "h1_transport_runtime.c").read_text()
transport = replace_once(
    transport,
    '''        PSTVNC_TRANSPORT_CAP_AUDIO_PCM_S16 |
''',
    '''        PSTVNC_TRANSPORT_CAP_RFB |
        PSTVNC_TRANSPORT_CAP_AUDIO_PCM_S16 |
''',
    "TRANSPORT_HELLO_RFB_CAP",
)
transport = replace_function(
    transport,
    "static int h1_allocate_queues(",
    r'''
static int h1_allocate_queues(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int media_ok;
    int rfb_ok = 1;

    if (runtime->config.queue_allocation_order ==
        PSTVNC_H1_ALLOCATE_MPEG_FIRST) {
        media_ok = h1_allocate_one_queue(
            &runtime->mpeg_queue,
            &runtime->mpeg_queue_storage,
            runtime->config.mpeg_queue_capacity);
        media_ok = media_ok && h1_allocate_one_queue(
            &runtime->audio_queue,
            &runtime->audio_queue_storage,
            runtime->config.audio_queue_capacity);
    } else {
        media_ok = h1_allocate_one_queue(
            &runtime->audio_queue,
            &runtime->audio_queue_storage,
            runtime->config.audio_queue_capacity);
        media_ok = media_ok && h1_allocate_one_queue(
            &runtime->mpeg_queue,
            &runtime->mpeg_queue_storage,
            runtime->config.mpeg_queue_capacity);
    }

    if (media_ok &&
        runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED) {
        rfb_ok = h1_allocate_one_queue(
            &runtime->rfb_queue,
            &runtime->rfb_queue_storage,
            PSTVNC_H1_RFB_QUEUE_CAPACITY);
    }

    if (!media_ok || !rfb_ok) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ALLOCATION);
        free(runtime->audio_queue_storage);
        free(runtime->mpeg_queue_storage);
        free(runtime->rfb_queue_storage);
        runtime->audio_queue_storage = NULL;
        runtime->mpeg_queue_storage = NULL;
        runtime->rfb_queue_storage = NULL;
        return 0;
    }

    return 1;
}
''',
)
transport = replace_once(
    transport,
    '''    if (channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        *queue = &runtime->mpeg_queue;
        *sema = runtime->mpeg_queue_sema_id;
        *frame_bytes = 1u;
        return 1;
    }

    return 0;
''',
    '''    if (channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        *queue = &runtime->mpeg_queue;
        *sema = runtime->mpeg_queue_sema_id;
        *frame_bytes = 1u;
        return 1;
    }

    if (channel == PSTVNC_TRANSPORT_CHANNEL_RFB &&
        runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED) {
        *queue = &runtime->rfb_queue;
        *sema = runtime->rfb_queue_sema_id;
        *frame_bytes = 1u;
        return 1;
    }

    return 0;
''',
    "TRANSPORT_QUEUE_CHANNEL",
)
transport = replace_once(
    transport,
    '''    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        if (runtime->stats.audio_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->audio_crc_state = h1_crc32_update(
            runtime->audio_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.audio_frames_received++;
        runtime->stats.audio_bytes_enqueued += header->payload_length;
        runtime->audio_last_data_sequence = header->sequence;
    } else {
        if (runtime->stats.mpeg_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->mpeg_crc_state = h1_crc32_update(
            runtime->mpeg_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.mpeg_frames_received++;
        runtime->stats.mpeg_bytes_enqueued += header->payload_length;
        runtime->mpeg_last_data_sequence = header->sequence;
    }

    return 1;
''',
    '''    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        if (runtime->stats.audio_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->audio_crc_state = h1_crc32_update(
            runtime->audio_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.audio_frames_received++;
        runtime->stats.audio_bytes_enqueued += header->payload_length;
        runtime->audio_last_data_sequence = header->sequence;
    } else if (header->channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2) {
        if (runtime->stats.mpeg_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->mpeg_crc_state = h1_crc32_update(
            runtime->mpeg_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.mpeg_frames_received++;
        runtime->stats.mpeg_bytes_enqueued += header->payload_length;
        runtime->mpeg_last_data_sequence = header->sequence;
    } else if (header->channel != PSTVNC_TRANSPORT_CHANNEL_RFB) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    return 1;
''',
    "TRANSPORT_ACCEPT_TAIL",
)
transport = replace_once(
    transport,
    '''    if (channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        pending = &runtime->audio_credit_pending;
        batch = runtime->config.audio_credit_batch_bytes;
        flush = runtime->config.audio_credit_flush_on_empty;
        enabled = runtime->config.audio_credit_return_enabled;
    } else {
        pending = &runtime->mpeg_credit_pending;
        batch = runtime->config.mpeg_credit_batch_bytes;
        flush = runtime->config.mpeg_credit_flush_on_empty;
        enabled = runtime->config.mpeg_credit_return_enabled;
    }
''',
    '''    if (channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        pending = &runtime->audio_credit_pending;
        batch = runtime->config.audio_credit_batch_bytes;
        flush = runtime->config.audio_credit_flush_on_empty;
        enabled = runtime->config.audio_credit_return_enabled;
    } else if (channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2) {
        pending = &runtime->mpeg_credit_pending;
        batch = runtime->config.mpeg_credit_batch_bytes;
        flush = runtime->config.mpeg_credit_flush_on_empty;
        enabled = runtime->config.mpeg_credit_return_enabled;
    } else if (channel == PSTVNC_TRANSPORT_CHANNEL_RFB) {
        pending = &runtime->rfb_credit_pending;
        batch = PSTVNC_H1_RFB_CREDIT_BATCH_BYTES;
        flush = 1u;
        enabled =
            runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED ? 1u : 0u;
    } else {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }
''',
    "TRANSPORT_RETURN_CREDIT",
)
transport = replace_once(
    transport,
    '''    runtime->audio_queue_sema_id = -1;
    runtime->mpeg_queue_sema_id = -1;
    runtime->send_sema_id = -1;
''',
    '''    runtime->audio_queue_sema_id = -1;
    runtime->mpeg_queue_sema_id = -1;
    runtime->rfb_queue_sema_id = -1;
    runtime->send_sema_id = -1;
''',
    "TRANSPORT_INIT_RFB_SEMA",
)
transport = replace_once(
    transport,
    '''    if (!h1_receive_config(runtime)) {
        (void)h1_send_error_code(runtime);
        goto fail;
    }

    if (!h1_allocate_queues(runtime) ||
''',
    '''    if (!h1_receive_config(runtime)) {
        (void)h1_send_error_code(runtime);
        goto fail;
    }

    if (runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED) {
        runtime->rfb_queue_sema_id = h1_create_mutex();
        if (runtime->rfb_queue_sema_id < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            (void)h1_send_error_code(runtime);
            goto fail;
        }
    }

    if (!h1_allocate_queues(runtime) ||
''',
    "TRANSPORT_CREATE_RFB_SEMA",
)
transport = replace_once(
    transport,
    '''    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&
        !h1_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_MPEG2,
            runtime->config.mpeg_initial_credit_bytes))
        goto fail;

    if (!h1_start_receiver(runtime))
''',
    '''    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&
        !h1_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_MPEG2,
            runtime->config.mpeg_initial_credit_bytes))
        goto fail;

    if (runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED &&
        !h1_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            PSTVNC_H1_RFB_QUEUE_CAPACITY))
        goto fail;

    if (!h1_start_receiver(runtime))
''',
    "TRANSPORT_INITIAL_RFB_CREDIT",
)
transport = replace_once(
    transport,
    '''    if (runtime->mpeg_queue_sema_id >= 0) {
        if (DeleteSema(runtime->mpeg_queue_sema_id) < 0)
            result = -1;
        runtime->mpeg_queue_sema_id = -1;
    }

    if (runtime->send_sema_id >= 0) {
''',
    '''    if (runtime->mpeg_queue_sema_id >= 0) {
        if (DeleteSema(runtime->mpeg_queue_sema_id) < 0)
            result = -1;
        runtime->mpeg_queue_sema_id = -1;
    }

    if (runtime->rfb_queue_sema_id >= 0) {
        if (DeleteSema(runtime->rfb_queue_sema_id) < 0)
            result = -1;
        runtime->rfb_queue_sema_id = -1;
    }

    if (runtime->send_sema_id >= 0) {
''',
    "TRANSPORT_DELETE_RFB_SEMA",
)
transport = replace_once(
    transport,
    '''    free(runtime->receiver_thread_stack_allocation);
    free(runtime->audio_queue_storage);
    free(runtime->mpeg_queue_storage);

    runtime->receiver_thread_stack_allocation = NULL;
    runtime->receiver_thread_stack = NULL;
    runtime->audio_queue_storage = NULL;
    runtime->mpeg_queue_storage = NULL;
''',
    '''    free(runtime->receiver_thread_stack_allocation);
    free(runtime->audio_queue_storage);
    free(runtime->mpeg_queue_storage);
    free(runtime->rfb_queue_storage);

    runtime->receiver_thread_stack_allocation = NULL;
    runtime->receiver_thread_stack = NULL;
    runtime->audio_queue_storage = NULL;
    runtime->mpeg_queue_storage = NULL;
    runtime->rfb_queue_storage = NULL;
''',
    "TRANSPORT_FREE_RFB",
)
transport = replace_once(
    transport,
    '''size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_queue_size(
        runtime,
        runtime->mpeg_queue_sema_id,
        &runtime->mpeg_queue,
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES);
}

int pstvnc_h1_transport_audio_read(
''',
    r'''size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_queue_size(
        runtime,
        runtime->mpeg_queue_sema_id,
        &runtime->mpeg_queue,
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES);
}

size_t pstvnc_h1_transport_rfb_queue_size(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_queue_size(
        runtime,
        runtime->rfb_queue_sema_id,
        &runtime->rfb_queue,
        runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED);
}

int pstvnc_h1_transport_rfb_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read)
{
    size_t available;
    size_t take;
    int queue_empty;

    if (runtime == NULL || buffer == NULL || bytes_read == NULL ||
        buffer_capacity == 0u ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return -1;
    }

    *bytes_read = 0u;

    if (runtime->error != PSTVNC_H1_ERROR_NONE)
        return -1;

    if (WaitSema(runtime->rfb_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    available = pstvnc_transport_queue_size(&runtime->rfb_queue);
    take = available < buffer_capacity ? available : buffer_capacity;

    if (take != 0u &&
        !pstvnc_transport_queue_read(&runtime->rfb_queue, buffer, take)) {
        (void)SignalSema(runtime->rfb_queue_sema_id);
        h1_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
        return -1;
    }

    queue_empty = pstvnc_transport_queue_size(&runtime->rfb_queue) == 0u;

    if (SignalSema(runtime->rfb_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    if (take == 0u)
        return 0;

    if (!h1_return_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            (uint32_t)take,
            queue_empty))
        return -1;

    *bytes_read = take;
    return 1;
}

int pstvnc_h1_transport_rfb_send(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *buffer,
    size_t byte_count)
{
    const uint8_t *source = (const uint8_t *)buffer;
    size_t offset = 0u;

    if (runtime == NULL ||
        (buffer == NULL && byte_count != 0u) ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return 0;
    }

    while (offset < byte_count) {
        size_t chunk = byte_count - offset;
        if (chunk > PSTVNC_TRANSPORT_MAX_PAYLOAD)
            chunk = PSTVNC_TRANSPORT_MAX_PAYLOAD;

        if (!h1_send_frame(
                runtime,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0,
                source + offset,
                chunk))
            return 0;

        offset += chunk;
    }

    return 1;
}

int pstvnc_h1_transport_audio_read(
''',
    "TRANSPORT_RFB_API_IMPL",
)
write(H1 / "h1_transport_runtime_cumulative39_interactive.c", transport)

write(H1 / "h1_rfb_mux_io.h", r'''
#ifndef PSTVNC_H1_RFB_MUX_IO_H
#define PSTVNC_H1_RFB_MUX_IO_H
#include "h1_transport_runtime.h"
void pstvnc_h1_rfb_mux_io_bind(pstvnc_h1_transport_runtime_t *transport);
void pstvnc_h1_rfb_mux_io_request_stop(void);
void pstvnc_h1_rfb_mux_io_unbind(void);
#endif
''')
write(H1 / "h1_rfb_mux_io.c", r'''
#include "h1_rfb_mux_io.h"
#include "rfb_io.h"
#include <delaythread.h>
#include <stddef.h>
#include <stdint.h>

static pstvnc_h1_transport_runtime_t *s_transport;
static volatile int s_stop_requested;

void pstvnc_h1_rfb_mux_io_bind(pstvnc_h1_transport_runtime_t *transport)
{
    s_transport = transport;
    s_stop_requested = 0;
}
void pstvnc_h1_rfb_mux_io_request_stop(void)
{
    s_stop_requested = 1;
}
void pstvnc_h1_rfb_mux_io_unbind(void)
{
    s_stop_requested = 1;
    s_transport = NULL;
}
static int h1_rfb_binding_matches(int socket_fd)
{
    return s_transport != NULL &&
        s_transport->socket_fd >= 0 &&
        socket_fd == s_transport->socket_fd &&
        s_transport->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED;
}
int pstvnc_rfb_io_read_exact(int socket_fd, void *buffer, size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    size_t done = 0u;
    if (!h1_rfb_binding_matches(socket_fd) ||
        (buffer == NULL && count != 0u))
        return -1;
    while (done < count) {
        size_t got = 0u;
        int result;
        if (s_stop_requested)
            return -1;
        result = pstvnc_h1_transport_rfb_read(
            s_transport, destination + done, count - done, &got);
        if (result < 0)
            return -1;
        if (result > 0) {
            done += got;
            continue;
        }
        if (s_transport->end_received ||
            pstvnc_h1_transport_last_error(s_transport) != PSTVNC_H1_ERROR_NONE)
            return -1;
        if (DelayThread(1000) < 0)
            return -1;
    }
    return 0;
}
int pstvnc_rfb_io_poll_receive(int socket_fd)
{
    if (!h1_rfb_binding_matches(socket_fd))
        return -1;
    if (pstvnc_h1_transport_rfb_queue_size(s_transport) != 0u)
        return 1;
    if (s_stop_requested || s_transport->end_received ||
        pstvnc_h1_transport_last_error(s_transport) != PSTVNC_H1_ERROR_NONE)
        return -1;
    return 0;
}
int pstvnc_rfb_io_write_exact(int socket_fd, const void *buffer, size_t count)
{
    if (!h1_rfb_binding_matches(socket_fd) ||
        (buffer == NULL && count != 0u) || s_stop_requested)
        return -1;
    return pstvnc_h1_transport_rfb_send(
        s_transport, buffer, count) ? 0 : -1;
}
''')

write(H1 / "h1_cumulative39_graphics.h", r'''
#ifndef PSTVNC_H1_CUMULATIVE39_GRAPHICS_H
#define PSTVNC_H1_CUMULATIVE39_GRAPHICS_H
#include <stdint.h>
int pstvnc_h1_graphics_present_video_macroblocks(
    const uint16_t *macroblock_pixels,
    unsigned int source_width,
    unsigned int source_height,
    unsigned int draw_x,
    unsigned int draw_y,
    unsigned int draw_width,
    unsigned int draw_height);
int pstvnc_h1_graphics_clear_video(void);
#endif
''')

write(H1 / "h1_cumulative39_graphics.c", r'''
#define pstvnc_ps2_graphics_init pstvnc_ps2_graphics_init_base_c39
#define pstvnc_ps2_graphics_present pstvnc_ps2_graphics_present_base_c39
#define pstvnc_ps2_graphics_shutdown pstvnc_ps2_graphics_shutdown_base_c39
#include "../../src/platform/ps2_graphics.c"
#undef pstvnc_ps2_graphics_init
#undef pstvnc_ps2_graphics_present
#undef pstvnc_ps2_graphics_shutdown

#include "h1_cumulative39_graphics.h"
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define H1_VIDEO_MAX_WIDTH 704u
#define H1_VIDEO_MAX_HEIGHT 480u
#define H1_VIDEO_MAX_PIXELS (H1_VIDEO_MAX_WIDTH * H1_VIDEO_MAX_HEIGHT)
static GSTEXTURE h1_video_texture;
static int h1_video_texture_configured;
static unsigned int h1_video_source_width;
static unsigned int h1_video_source_height;
static unsigned int h1_video_x;
static unsigned int h1_video_y;
static unsigned int h1_video_width;
static unsigned int h1_video_height;
static int h1_video_visible;
static uint16_t h1_video_linear[H1_VIDEO_MAX_PIXELS]
    __attribute__((aligned(128)));
static const uint16_t *h1_latest_desktop;
static pstvnc_ps2_graphics_overlay_t h1_latest_overlay;
static int h1_latest_overlay_visible;
static int h1_graphics_sema = -1;

static int h1_create_graphics_mutex(void)
{
    ee_sema_t sema;
    memset(&sema, 0, sizeof(sema));
    sema.init_count = 1;
    sema.max_count = 1;
    return CreateSema(&sema);
}
static int h1_lock(void)
{
    return h1_graphics_sema >= 0 && WaitSema(h1_graphics_sema) >= 0;
}
static int h1_unlock(void)
{
    return h1_graphics_sema >= 0 && SignalSema(h1_graphics_sema) >= 0;
}
static int h1_configure_video_texture(unsigned int width, unsigned int height)
{
    if (h1_video_texture_configured) {
        if (width != h1_video_source_width || height != h1_video_source_height)
            return -1;
        h1_video_texture.Mem = (u32 *)h1_video_linear;
        return 0;
    }
    memset(&h1_video_texture, 0, sizeof(h1_video_texture));
    h1_video_texture.Width = width;
    h1_video_texture.Height = height;
    h1_video_texture.PSM = GS_PSM_CT16;
    h1_video_texture.Mem = (u32 *)h1_video_linear;
    h1_video_texture.Filter = GS_FILTER_NEAREST;
    h1_video_texture.VramClut = 0;
    h1_video_texture.Vram = gsKit_vram_alloc(
        display, gsKit_texture_size(width, height, GS_PSM_CT16),
        GSKIT_ALLOC_USERBUFFER);
    h1_video_source_width = width;
    h1_video_source_height = height;
    h1_video_texture_configured = 1;
    return 0;
}
static void h1_detile_macroblocks(
    const uint16_t *source, unsigned int width, unsigned int height)
{
    unsigned int mb_columns = width >> 4;
    unsigned int mb_rows = height >> 4;
    unsigned int mb_y;
    unsigned int mb_x;
    for (mb_y = 0u; mb_y < mb_rows; ++mb_y) {
        for (mb_x = 0u; mb_x < mb_columns; ++mb_x) {
            const uint16_t *block =
                source + ((mb_y * mb_columns + mb_x) * 256u);
            unsigned int row;
            for (row = 0u; row < 16u; ++row) {
                uint16_t *destination = h1_video_linear +
                    ((mb_y * 16u + row) * width) + (mb_x * 16u);
                memcpy(destination, block + row * 16u,
                    16u * sizeof(uint16_t));
            }
        }
    }
}
static int h1_render_locked(int upload_desktop, int upload_video, int upload_overlay)
{
    const u64 clear_color = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x80,0x00);
    const u64 texture_color = GS_SETREG_RGBAQ(0x80,0x80,0x80,0x80,0x00);
    if (display == NULL || h1_latest_desktop == NULL)
        return -1;
    if (!desktop_texture_configured)
        configure_desktop_texture(h1_latest_desktop);
    else
        desktop_texture.Mem = (u32 *)h1_latest_desktop;
    if (upload_desktop)
        gsKit_texture_upload(display, &desktop_texture);
    if (h1_video_visible) {
        h1_video_texture.Mem = (u32 *)h1_video_linear;
        if (upload_video)
            gsKit_texture_upload(display, &h1_video_texture);
    }
    if (h1_latest_overlay_visible) {
        if (configure_local_overlay_texture(&h1_latest_overlay) < 0)
            return -1;
        local_overlay_texture.Mem = (u32 *)h1_latest_overlay.pixels;
        if (upload_overlay)
            gsKit_texture_upload(display, &local_overlay_texture);
    }
    gsKit_clear(display, clear_color);
    gsKit_prim_sprite_texture(display, &desktop_texture,
        0.0f,0.0f,0.0f,0.0f,
        (float)PSTVNC_DISPLAY_WIDTH,(float)PSTVNC_DISPLAY_HEIGHT,
        (float)PSTVNC_DISPLAY_WIDTH,(float)PSTVNC_DISPLAY_HEIGHT,
        1, texture_color);
    if (h1_video_visible) {
        gsKit_prim_sprite_texture(display, &h1_video_texture,
            (float)h1_video_x,(float)h1_video_y,0.0f,0.0f,
            (float)(h1_video_x+h1_video_width),
            (float)(h1_video_y+h1_video_height),
            (float)h1_video_source_width,(float)h1_video_source_height,
            2, texture_color);
    }
    if (h1_latest_overlay_visible) {
        gsKit_prim_sprite_texture(display, &local_overlay_texture,
            (float)h1_latest_overlay.x,(float)h1_latest_overlay.y,0.0f,0.0f,
            (float)(h1_latest_overlay.x+h1_latest_overlay.width),
            (float)(h1_latest_overlay.y+h1_latest_overlay.height),
            (float)h1_latest_overlay.width,(float)h1_latest_overlay.height,
            3, texture_color);
    }
    gsKit_queue_exec(display);
    gsKit_sync_flip(display);
    return 0;
}
int pstvnc_ps2_graphics_init(void)
{
    if (pstvnc_ps2_graphics_init_base_c39() < 0)
        return -1;
    if (h1_graphics_sema < 0) {
        h1_graphics_sema = h1_create_graphics_mutex();
        if (h1_graphics_sema < 0)
            return -1;
    }
    h1_video_texture_configured = 0;
    h1_video_visible = 0;
    h1_latest_desktop = NULL;
    h1_latest_overlay_visible = 0;
    memset(&h1_video_texture, 0, sizeof(h1_video_texture));
    return 0;
}
int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels, size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay)
{
    int result;
    if (desktop_pixels == NULL ||
        desktop_pixel_count != PSTVNC_DISPLAY_PIXEL_COUNT ||
        !local_overlay_is_valid(local_overlay))
        return -1;
    if (!h1_lock())
        return -1;
    h1_latest_desktop = desktop_pixels;
    if (local_overlay != NULL) {
        h1_latest_overlay = *local_overlay;
        h1_latest_overlay_visible = 1;
    } else {
        memset(&h1_latest_overlay, 0, sizeof(h1_latest_overlay));
        h1_latest_overlay_visible = 0;
    }
    result = h1_render_locked(1, 0, h1_latest_overlay_visible);
    if (!h1_unlock())
        return -1;
    return result;
}
int pstvnc_h1_graphics_present_video_macroblocks(
    const uint16_t *macroblock_pixels,
    unsigned int source_width, unsigned int source_height,
    unsigned int draw_x, unsigned int draw_y,
    unsigned int draw_width, unsigned int draw_height)
{
    int result;
    if (macroblock_pixels == NULL || source_width == 0u || source_height == 0u ||
        source_width > H1_VIDEO_MAX_WIDTH || source_height > H1_VIDEO_MAX_HEIGHT ||
        (source_width & 15u) != 0u || (source_height & 15u) != 0u ||
        draw_width == 0u || draw_height == 0u ||
        draw_x + draw_width > PSTVNC_DISPLAY_WIDTH ||
        draw_y + draw_height > PSTVNC_DISPLAY_HEIGHT)
        return -1;
    if (!h1_lock())
        return -1;
    if (h1_latest_desktop == NULL ||
        h1_configure_video_texture(source_width, source_height) < 0) {
        (void)h1_unlock();
        return -1;
    }
    h1_detile_macroblocks(macroblock_pixels, source_width, source_height);
    h1_video_x = draw_x;
    h1_video_y = draw_y;
    h1_video_width = draw_width;
    h1_video_height = draw_height;
    h1_video_visible = 1;
    result = h1_render_locked(0, 1, 0);
    if (!h1_unlock())
        return -1;
    return result;
}
int pstvnc_h1_graphics_clear_video(void)
{
    int result = 0;
    if (!h1_lock())
        return -1;
    h1_video_visible = 0;
    if (h1_latest_desktop != NULL)
        result = h1_render_locked(0, 0, 0);
    if (!h1_unlock())
        return -1;
    return result;
}
void pstvnc_ps2_graphics_shutdown(void)
{
    if (h1_graphics_sema >= 0) {
        (void)DeleteSema(h1_graphics_sema);
        h1_graphics_sema = -1;
    }
    h1_video_texture_configured = 0;
    h1_video_visible = 0;
    h1_latest_desktop = NULL;
    h1_latest_overlay_visible = 0;
    pstvnc_ps2_graphics_shutdown_base_c39();
}
''')

video = (H1 / "h1_video_runtime.c").read_text()
video = replace_once(video, '#include "h1_video_runtime.h"\n',
    '#include "h1_video_runtime.h"\n#include "h1_cumulative39_graphics.h"\n',
    "VIDEO_GRAPHICS_INCLUDE")
video = replace_function(video, "static void h1_video_setup_environment(void)",
    'static void h1_video_setup_environment(void)\n{\n}\n')
video = replace_function(video, "static void h1_video_show_color(",
    'static void h1_video_show_color(int red, int green, int blue)\n{\n    (void)red;\n    (void)green;\n    (void)blue;\n}\n')
video = replace_function(video, "static void h1_video_upload(",
    'static void h1_video_upload(h1_video_session_t *session)\n{\n    (void)session;\n}\n')
video = replace_function(video, "static void h1_video_draw(", r'''
static void h1_video_draw(h1_video_session_t *session)
{
    if (session == NULL || session->sequence_info == NULL)
        return;
    if (pstvnc_h1_graphics_present_video_macroblocks(
            (const uint16_t *)session->picture_buffer,
            session->sequence_info->m_Width,
            session->sequence_info->m_Height,
            session->config->video_draw_x,
            session->config->video_draw_y,
            session->config->video_draw_width,
            session->config->video_draw_height) < 0)
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
}
''')
video = replace_function(video, "static void h1_video_release_session(", r'''
static void h1_video_release_session(h1_video_session_t *session)
{
    dma_channel_wait(DMA_CHANNEL_toIPU, 0);
    if (session->mpeg_initialized) {
        MPEG_Destroy();
        session->mpeg_initialized = 0;
    }
    if (session->transfer_packet != NULL) {
        packet_free(session->transfer_packet);
        session->transfer_packet = NULL;
    }
    if (session->draw_packet != NULL) {
        packet_free(session->draw_packet);
        session->draw_packet = NULL;
    }
    free(session->picture_allocation);
    free(session->feed_allocation);
    session->picture_allocation = NULL;
    session->feed_allocation = NULL;
    session->picture_buffer = NULL;
    session->feed_buffer = NULL;
    (void)pstvnc_h1_graphics_clear_video();
}
''')
video = replace_function(video, "int pstvnc_h1_video_chassis_init(void)", r'''
int pstvnc_h1_video_chassis_init(void)
{
    if (s_h1_chassis.initialized)
        return 0;
    memset(&s_h1_chassis, 0, sizeof(s_h1_chassis));
    dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);
    s_h1_chassis.initialized = 1;
    printf("H1_VIDEO_CHASSIS=READY shared_compositor=1\n");
    return 0;
}
''')
write(H1 / "h1_video_runtime_cumulative39_interactive.c", video)

app = (ROOT / "src/app.c").read_text()
cut = app.find("int pstvnc_app_run(void)")
if cut < 0:
    raise SystemExit("ABORT=APP_RUN_MARKER_NOT_FOUND")
interactive = app[:cut]
interactive = replace_once(interactive, '#include "app.h"\n',
    '#include "app.h"\n#include "h1_cumulative39_interactive.h"\n#include "h1_rfb_mux_io.h"\n',
    "INTERACTIVE_INCLUDES")
interactive = replace_once(interactive, '#include <stdint.h>\n',
    '#include <stdint.h>\n#include <stdlib.h>\n', "INTERACTIVE_STDLIB")
interactive += '''
#define H1_INTERACTIVE_THREAD_PRIORITY 32
#define H1_INTERACTIVE_THREAD_STACK_BYTES 32768u
#define H1_INTERACTIVE_READY_WAIT_STEPS 10000u
#define H1_INTERACTIVE_STOP_WAIT_STEPS 3000u

static void *h1_interactive_allocate_stack(size_t byte_count, void **allocation)
{
    uintptr_t address;
    void *raw = malloc(byte_count + 15u);
    if (raw == NULL) return NULL;
    address = ((uintptr_t)raw + 15u) & ~(uintptr_t)15u;
    *allocation = raw;
    return (void *)address;
}

static int h1_interactive_session(pstvnc_h1_cumulative39_interactive_t *control)
{
    static pstvnc_input_runtime_t input_runtime;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_local_controller_t local_controller;
    pstvnc_local_ui_t local_ui;
    pstvnc_osk_t osk;
    pstvnc_rfb_session_t session;
    app_published_pointer_state_t published_pointer;
    int input_runtime_ready = 0;
    int mouse_interpretation_suspended = 0;
    int success = 0;

    if (control == NULL || control->transport == NULL ||
        control->transport->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED)
        return -1;
    pstvnc_h1_rfb_mux_io_bind(control->transport);
    if (!pstvnc_framebuffer_init(&framebuffer, remote_pixels, PSTVNC_DISPLAY_PIXEL_COUNT)) goto done;
    if (!pstvnc_framebuffer_set_geometry(&framebuffer, PSTVNC_DISPLAY_WIDTH, PSTVNC_DISPLAY_HEIGHT)) goto done;
    pstvnc_local_controller_init(&local_controller);
    pstvnc_local_ui_init(&local_ui);
    pstvnc_osk_reset_for_open(&osk);
    pstvnc_rfb_session_init(&session);
    if (!pstvnc_rfb_session_start(&session, control->transport->socket_fd,
            PSTVNC_DISPLAY_WIDTH, PSTVNC_DISPLAY_HEIGHT)) goto done;
    if (!pstvnc_rfb_session_receive_initial_frame(&session, &framebuffer)) goto done;
    if (!present_current_application_frame(&framebuffer, 1, &local_ui, &osk)) goto done;
    if (pstvnc_input_runtime_init(&input_runtime, PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT, PSTVNC_APP_CONTROLLER_PORT,
            PSTVNC_APP_CONTROLLER_SLOT) < 0) goto done;
    input_runtime_ready = 1;
    published_pointer.cursor_x = PSTVNC_DISPLAY_WIDTH / 2u;
    published_pointer.cursor_y = PSTVNC_DISPLAY_HEIGHT / 2u;
    published_pointer.click_buttons = 0;
    if (!pstvnc_rfb_session_send_pointer_event(&session, 0,
            (uint16_t)published_pointer.cursor_x,
            (uint16_t)published_pointer.cursor_y)) goto done;
    if (pstvnc_input_runtime_start(&input_runtime) < 0) goto done;
    if (!pstvnc_rfb_session_request_update(&session, 1)) goto done;
    control->ready = 1;
    printf("H1_INTERACTIVE=READY rfb=1 input=1 osk=1\\n");

    while (!control->stop_requested && !control->transport->end_received) {
        pstvnc_rfb_session_receive_result_t receive_result;
        if (pstvnc_h1_transport_last_error(control->transport) != PSTVNC_H1_ERROR_NONE) goto done;
        if (!service_semantic_input_events(&input_runtime, &session,
                &published_pointer, &local_controller, &local_ui, &osk,
                &mouse_interpretation_suspended)) goto done;
        if (pstvnc_local_ui_needs_present(&local_ui) &&
            !present_current_application_frame(&framebuffer, 0, &local_ui, &osk)) goto done;
        if (!resume_desktop_mouse_if_ready(&input_runtime, &local_ui,
                &mouse_interpretation_suspended)) goto done;
        receive_result = pstvnc_rfb_session_try_receive_update(&session, &framebuffer);
        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_FAILED) {
            if (control->transport->end_received || control->stop_requested) break;
            goto done;
        }
        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_IDLE) {
            if (pstvnc_ps2_system_delay_us(PSTVNC_APP_IDLE_POLL_DELAY_US) < 0) goto done;
            continue;
        }
        if (receive_result != PSTVNC_RFB_SESSION_RECEIVE_UPDATE || !framebuffer.valid) goto done;
        if (framebuffer.dirty &&
            !present_current_application_frame(&framebuffer, 1, &local_ui, &osk)) goto done;
        if (!pstvnc_rfb_session_request_update(&session, 1)) goto done;
    }
    success = 1;
done:
    if (input_runtime_ready) (void)pstvnc_input_runtime_shutdown(&input_runtime);
    pstvnc_h1_rfb_mux_io_unbind();
    if (!success) printf("H1_INTERACTIVE=FAIL rfb_error=%d transport_error=%d end=%d stop=%d\\n",
        (int)session.error, (int)pstvnc_h1_transport_last_error(control->transport),
        control->transport->end_received, control->stop_requested);
    else printf("H1_INTERACTIVE=PASS\\n");
    return success ? 0 : -1;
}

static void h1_interactive_thread(void *argument)
{
    pstvnc_h1_cumulative39_interactive_t *control =
        (pstvnc_h1_cumulative39_interactive_t *)argument;
    control->result = h1_interactive_session(control);
    control->done = 1;
    ExitThread();
}

int pstvnc_h1_cumulative39_interactive_start(
    pstvnc_h1_cumulative39_interactive_t *control,
    pstvnc_h1_transport_runtime_t *transport)
{
    ee_thread_t thread;
    if (control == NULL || transport == NULL) return -1;
    memset(control, 0, sizeof(*control));
    control->transport = transport;
    control->thread_id = -1;
    control->result = -1;
    control->thread_stack = (unsigned char *)h1_interactive_allocate_stack(
        H1_INTERACTIVE_THREAD_STACK_BYTES, &control->thread_stack_allocation);
    if (control->thread_stack == NULL) return -1;
    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)h1_interactive_thread;
    thread.stack = control->thread_stack;
    thread.stack_size = H1_INTERACTIVE_THREAD_STACK_BYTES;
    thread.gp_reg = &_gp;
    thread.initial_priority = H1_INTERACTIVE_THREAD_PRIORITY;
    control->thread_id = CreateThread(&thread);
    if (control->thread_id < 0) goto fail;
    if (StartThread(control->thread_id, control) < 0) goto fail;
    control->started = 1;
    return 0;
fail:
    if (control->thread_id >= 0) (void)DeleteThread(control->thread_id);
    free(control->thread_stack_allocation);
    control->thread_stack_allocation = NULL;
    control->thread_stack = NULL;
    control->thread_id = -1;
    return -1;
}

int pstvnc_h1_cumulative39_interactive_wait_ready(
    pstvnc_h1_cumulative39_interactive_t *control)
{
    unsigned int step;
    if (control == NULL || !control->started) return -1;
    for (step = 0u; step < H1_INTERACTIVE_READY_WAIT_STEPS; ++step) {
        if (control->ready) return 0;
        if (control->done) return -1;
        if (DelayThread(1000) < 0) return -1;
    }
    return -1;
}

int pstvnc_h1_cumulative39_interactive_shutdown(
    pstvnc_h1_cumulative39_interactive_t *control)
{
    unsigned int step;
    int result = 0;
    if (control == NULL) return -1;
    if (!control->started) {
        free(control->thread_stack_allocation);
        control->thread_stack_allocation = NULL;
        control->thread_stack = NULL;
        return 0;
    }
    control->stop_requested = 1;
    pstvnc_h1_rfb_mux_io_request_stop();
    for (step = 0u; step < H1_INTERACTIVE_STOP_WAIT_STEPS; ++step) {
        ee_thread_status_t status;
        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(control->thread_id, &status) < 0) { result = -1; break; }
        if (status.status == THS_DORMANT) {
            if (DeleteThread(control->thread_id) < 0) result = -1;
            else { control->thread_id = -1; control->started = 0; }
            break;
        }
        if (DelayThread(1000) < 0) { result = -1; break; }
    }
    if (control->started) result = -1;
    free(control->thread_stack_allocation);
    control->thread_stack_allocation = NULL;
    control->thread_stack = NULL;
    return result;
}
'''
write(H1 / "h1_cumulative39_interactive.c", interactive)

write(H1 / "h1_cumulative39_interactive.h", '''
#ifndef PSTVNC_H1_CUMULATIVE39_INTERACTIVE_H
#define PSTVNC_H1_CUMULATIVE39_INTERACTIVE_H
#include "h1_transport_runtime.h"
typedef struct pstvnc_h1_cumulative39_interactive {
    pstvnc_h1_transport_runtime_t *transport;
    int thread_id;
    int started;
    volatile int ready;
    volatile int done;
    volatile int stop_requested;
    volatile int result;
    void *thread_stack_allocation;
    unsigned char *thread_stack;
} pstvnc_h1_cumulative39_interactive_t;
int pstvnc_h1_cumulative39_interactive_start(
    pstvnc_h1_cumulative39_interactive_t *control,
    pstvnc_h1_transport_runtime_t *transport);
int pstvnc_h1_cumulative39_interactive_wait_ready(
    pstvnc_h1_cumulative39_interactive_t *control);
int pstvnc_h1_cumulative39_interactive_shutdown(
    pstvnc_h1_cumulative39_interactive_t *control);
#endif
''')

write(H1 / "h1_main_cumulative39_interactive.c", '''
#include "h1_audio_runtime.h"
#include "h1_cumulative39_interactive.h"
#include "h1_media_clock.h"
#include "h1_transport_runtime.h"
#include "h1_video_runtime.h"
#include "platform/ps2_graphics.h"
#include "ps2_network.h"
#include "ps2_system.h"
#include <delaythread.h>
#include <kernel.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define H1_CONNECT_RETRY_DELAY_US 250000u
#define H1_AUDIO_FINISH_POLL_US 1000u
#define H1_BETWEEN_SESSION_DELAY_US 100000u
static int h1_wait_for_session_transport(pstvnc_h1_transport_runtime_t *transport)
{
    for (;;) {
        memset(transport, 0, sizeof(*transport));
        if (pstvnc_h1_transport_start(transport) == 0) return 0;
        printf("H1_WAITING_FOR_SESSION connect_error=%d\\n", (int)transport->error);
        if (DelayThread(H1_CONNECT_RETRY_DELAY_US) < 0) return -1;
    }
}
static int h1_wait_for_audio_completion(pstvnc_h1_audio_runtime_t *audio)
{
    while (!pstvnc_h1_audio_finished(audio))
        if (DelayThread(H1_AUDIO_FINISH_POLL_US) < 0) return -1;
    return 0;
}
int main(void)
{
    uint32_t completed_sessions = 0u;
    printf("H1_CUMULATIVE39_INTERACTIVE_START\\n");
    if (pstvnc_ps2_system_prepare_iop() < 0) { SleepThread(); return 10; }
    if (pstvnc_ps2_network_init() < 0) { SleepThread(); return 11; }
    if (pstvnc_ps2_network_wait_link() < 0) { SleepThread(); return 12; }
    if (pstvnc_ps2_graphics_init() < 0) { SleepThread(); return 13; }
    if (pstvnc_h1_video_chassis_init() < 0) { SleepThread(); return 14; }
    printf("H1_BOOT=PASS compositor=RFB_MPEG_OSK\\n");
    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_audio_runtime_t audio;
        pstvnc_h1_media_clock_t clock;
        pstvnc_h1_video_result_t video;
        pstvnc_h1_cumulative39_interactive_t interactive;
        const pstvnc_h1_config_t *config;
        int audio_active = 0;
        int interactive_active = 0;
        int session_ok = 1;
        uint32_t diagnostic_word;
        memset(&audio, 0, sizeof(audio));
        memset(&video, 0, sizeof(video));
        memset(&interactive, 0, sizeof(interactive));
        printf("H1_WAITING_FOR_SESSION completed=%u\\n", (unsigned int)completed_sessions);
        if (h1_wait_for_session_transport(&transport) < 0) { SleepThread(); return 20; }
        config = pstvnc_h1_transport_config(&transport);
        if (config == NULL) { (void)pstvnc_h1_transport_shutdown(&transport); continue; }
        pstvnc_h1_media_clock_init(&clock, config->media_epoch_lead_us);
        printf("H1_SESSION_BEGIN id=%u profile=%u rfb=%u audio=%u video=%u\\n",
            (unsigned int)config->session_id, (unsigned int)config->profile_id,
            (unsigned int)config->rfb_mode, (unsigned int)config->audio_mode,
            (unsigned int)config->video_mode);
        if (config->rfb_mode == PSTVNC_H1_RFB_ON_RESERVED) {
            if (pstvnc_h1_cumulative39_interactive_start(&interactive, &transport) < 0) {
                printf("H1_INTERACTIVE=START_FAIL\\n"); session_ok = 0;
            } else {
                interactive_active = 1;
                if (pstvnc_h1_cumulative39_interactive_wait_ready(&interactive) < 0) {
                    printf("H1_INTERACTIVE=READY_FAIL\\n"); session_ok = 0;
                }
            }
        }
        if (session_ok && config->audio_mode == PSTVNC_H1_AUDIO_PCM) {
            if (pstvnc_h1_audio_load_modules_once() < 0) session_ok = 0;
            else if (pstvnc_h1_audio_start(&audio, &transport, &clock) < 0) session_ok = 0;
            else { audio_active = 1; printf("H1_AUDIO=STARTED\\n"); }
        }
        if (session_ok && config->video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
            if (pstvnc_h1_video_run_session(&transport, &clock, &video) < 0) {
                printf("H1_VIDEO=FAIL error=%d decoded=%u displayed=%u\\n",
                    (int)video.error, (unsigned int)video.pictures_decoded,
                    (unsigned int)video.pictures_displayed); session_ok = 0;
            } else printf("H1_VIDEO=PASS decoded=%u displayed=%u misses=%u\\n",
                (unsigned int)video.pictures_decoded,
                (unsigned int)video.pictures_displayed,
                (unsigned int)video.deadline_misses);
        }
        if (audio_active && session_ok) {
            if (h1_wait_for_audio_completion(&audio) < 0 ||
                pstvnc_h1_audio_last_error(&audio) != PSTVNC_H1_AUDIO_ERROR_NONE)
                session_ok = 0;
            else printf("H1_AUDIO=PASS played_bytes=%u\\n",
                (unsigned int)transport.stats.audio_bytes_played);
        }
        if (interactive_active &&
            pstvnc_h1_cumulative39_interactive_shutdown(&interactive) < 0) {
            printf("H1_INTERACTIVE=SHUTDOWN_FAIL\\n"); session_ok = 0;
        }
        diagnostic_word = 0xB1000000u |
            (((uint32_t)video.error & 0xffu) << 8) |
            ((uint32_t)pstvnc_h1_audio_last_error(&audio) & 0xffu);
        pstvnc_h1_transport_set_diagnostic_word(&transport, diagnostic_word);
        if (!pstvnc_h1_transport_send_result(&transport,
                video.pictures_decoded, video.pictures_displayed,
                video.feed_calls, video.payload_bytes_submitted,
                video.dma_bytes_submitted, video.deadline_misses,
                (uint32_t)video.max_deadline_late_ticks)) session_ok = 0;
        else printf("H1_RESULT=SEND_PASS\\n");
        if (audio_active && pstvnc_h1_audio_shutdown(&audio) < 0) session_ok = 0;
        if (pstvnc_h1_transport_shutdown(&transport) < 0) session_ok = 0;
        completed_sessions++;
        printf("H1_SESSION_END id=%u result=%s completed=%u\\n",
            (unsigned int)config->session_id, session_ok ? "PASS" : "FAIL",
            (unsigned int)completed_sessions);
        if (DelayThread(H1_BETWEEN_SESSION_DELAY_US) < 0) { SleepThread(); return 21; }
    }
}
''')

write(H1 / "h1_mux_server_cumulative39_interactive.py", '''
#!/usr/bin/env python3
from __future__ import annotations
import json
import socket
import struct
import threading
from pathlib import Path
import h1_mux_server_cumulative39_thread_census as cumulative
base = cumulative.base
CHANNEL_RFB = 1
CAP_RFB = 1 << 0
RFB_BUFFER_BYTES = 32768
RFB_QUANTUM = 4096
base.CHANNEL_RFB = CHANNEL_RFB
class RfbProxy:
    def __init__(self, evidence: Path) -> None:
        self.sock = socket.create_connection(("127.0.0.1", 5900), timeout=5.0)
        self.sock.settimeout(None)
        self.condition = threading.Condition()
        self.buffer = bytearray()
        self.maximum_buffer = RFB_BUFFER_BYTES
        self.stop_event = threading.Event()
        self.error = None
        self.bytes_from_server = 0
        self.bytes_to_server = 0
        self.archive = (evidence / "rfb-server-to-ps2.bin").open("wb")
        self.thread = threading.Thread(target=self._reader, daemon=True)
        self.thread.start()
        print("H1_RFB_UPSTREAM=127.0.0.1:5900", flush=True)
    def _reader(self):
        try:
            while not self.stop_event.is_set():
                data = self.sock.recv(8192)
                if not data: raise EOFError("upstream VNC server closed")
                self.archive.write(data); self.archive.flush()
                offset = 0
                while offset < len(data):
                    with self.condition:
                        while len(self.buffer) >= self.maximum_buffer and not self.stop_event.is_set():
                            self.condition.wait(timeout=0.05)
                        if self.stop_event.is_set(): return
                        room = self.maximum_buffer - len(self.buffer)
                        part = data[offset:offset+room]
                        self.buffer.extend(part); self.bytes_from_server += len(part)
                        offset += len(part); self.condition.notify_all()
        except BaseException as exc:
            if not self.stop_event.is_set(): self.error = exc
    def available(self):
        with self.condition: return len(self.buffer)
    def take(self, maximum):
        with self.condition:
            count = min(maximum, len(self.buffer))
            if count == 0: return b""
            result = bytes(self.buffer[:count]); del self.buffer[:count]
            self.condition.notify_all(); return result
    def write_from_ps2(self, payload):
        if not payload: raise base.ProtocolError("empty PS2 RFB DATA frame")
        self.sock.sendall(payload); self.bytes_to_server += len(payload)
    def check(self):
        if self.error is not None: raise RuntimeError("RFB upstream reader failed") from self.error
    def stop(self):
        self.stop_event.set()
        try: self.sock.shutdown(socket.SHUT_RDWR)
        except OSError: pass
        try: self.sock.close()
        except OSError: pass
        with self.condition: self.condition.notify_all()
        self.thread.join(timeout=2.0); self.archive.close()
        print("H1_RFB_PROXY_SUMMARY=" + json.dumps({"server_to_ps2_bytes":self.bytes_from_server,"ps2_to_server_bytes":self.bytes_to_server},sort_keys=True),flush=True)
_original_init = base.H1Session.__init__
def _init_interactive(self,*args,**kwargs):
    _original_init(self,*args,**kwargs)
    self.rfb = base.ChannelState(CHANNEL_RFB,"rfb")
    self.rfb_proxy = None
base.H1Session.__init__ = _init_interactive
def _reader_interactive(self):
    try:
        while not self.stop_event.is_set():
            frame = base.receive_frame(self.sock)
            if frame.sequence != self.expected_rx_sequence: raise base.ProtocolError("PS2 sequence mismatch")
            self.expected_rx_sequence += 1
            if frame.kind == base.FRAME_HELLO:
                self.hello = self._parse_hello(frame.payload)
                if (self.hello["capabilities"] & base.REQUIRED_CAPS) != base.REQUIRED_CAPS: raise base.ProtocolError("required media caps absent")
                if self.profile["rfb_mode"] and not (self.hello["capabilities"] & CAP_RFB): raise base.ProtocolError("RFB cap absent")
                print("H1_PS2_HELLO="+json.dumps(self.hello,sort_keys=True),flush=True); self.hello_event.set()
            elif frame.kind == base.FRAME_CONFIG:
                if frame.channel != base.CHANNEL_CONTROL or frame.flags != base.CONFIG_ACK_FLAG or frame.payload != self.config_payload: raise base.ProtocolError("bad CONFIG ACK")
                print("H1_CONFIG_ACK=PASS",flush=True); self.config_ack_event.set()
            elif frame.kind == base.FRAME_CREDIT:
                amount = struct.unpack(">I",frame.payload)[0]
                state = self.audio if frame.channel == base.CHANNEL_AUDIO else self.mpeg if frame.channel == base.CHANNEL_MPEG2 else self.rfb if frame.channel == CHANNEL_RFB else None
                if state is None: raise base.ProtocolError("bad CREDIT channel")
                with self.condition: state.credit += amount; self.condition.notify_all()
            elif frame.kind == base.FRAME_DATA:
                if frame.channel != CHANNEL_RFB or self.rfb_proxy is None: raise base.ProtocolError("bad PS2 DATA")
                self.rfb_proxy.write_from_ps2(frame.payload)
            elif frame.kind == base.FRAME_TELEMETRY:
                self.last_telemetry = self._parse_telemetry(frame.payload)
                print("H1_PS2_TELEMETRY="+json.dumps(self.last_telemetry,sort_keys=True),flush=True)
            elif frame.kind == base.FRAME_SESSION_RESULT:
                self.result = self._parse_result(frame.payload)
                print("H1_PS2_RESULT="+json.dumps(self.result,sort_keys=True),flush=True); self.result_event.set()
            elif frame.kind == base.FRAME_ERROR:
                raise base.ProtocolError("PS2 transport error")
            else: raise base.ProtocolError(f"unexpected frame kind={frame.kind} channel={frame.channel}")
    except BaseException as exc:
        if isinstance(exc, EOFError) and self.result_event.is_set():
            print("H1_POST_RESULT_SOCKET_CLOSE=PASS classification=normal_session_boundary",flush=True); return
        if not self.stop_event.is_set():
            self.reader_error = exc; self.result_event.set(); self.hello_event.set(); self.config_ack_event.set()
            with self.condition: self.condition.notify_all()
base.H1Session.reader = _reader_interactive
def _send_rfb(self):
    if self.rfb_proxy is None: return False
    with self.condition: credit = self.rfb.credit
    available = self.rfb_proxy.available()
    if credit <= 0 or available <= 0: return False
    count = min(RFB_QUANTUM,credit,available,base.MAX_PAYLOAD)
    payload = self.rfb_proxy.take(count)
    if not payload: return False
    seq = self.send_frame(base.FRAME_DATA,CHANNEL_RFB,payload)
    with self.condition: self.rfb.credit -= len(payload)
    self.rfb.frames_sent += 1; self.rfb.bytes_sent += len(payload); self.rfb.last_data_sequence = seq
    return True
def _send_media(self,channel):
    if channel == base.CHANNEL_AUDIO: return self._send_from(self.audio_producer,self.audio,base.MAX_PAYLOAD,4)
    return self._send_from(self.video_producer,self.mpeg,base.MAX_PAYLOAD,1)
def _heartbeat_loop(session,stop):
    sample=0
    while not stop.wait(1.0):
        if session.stop_event.is_set(): return
        try:
            session.send_frame(base.FRAME_HEARTBEAT,base.CHANNEL_CONTROL); sample+=1
            print(f"H1_INTERACTIVE_HEARTBEAT_SENT={sample}",flush=True)
        except BaseException: return
def _schedule_interactive(self):
    target=self.profile["audio_start_target_bytes"]
    schedule=(CHANNEL_RFB,base.CHANNEL_AUDIO,base.CHANNEL_MPEG2); index=0; reported=False
    stop=threading.Event(); t=threading.Thread(target=_heartbeat_loop,args=(self,stop),daemon=True); t.start()
    try:
        while True:
            self.check_reader(); self.video_producer.check(); self.audio_producer.check(); self.rfb_proxy.check()
            if self.video_producer.eof and not self.audio_stop_requested:
                self.audio_stop_requested=True; self.audio_producer.stop(); print("H1_AUDIO_STOP_REQUEST=VIDEO_EOF",flush=True)
            if self.video_producer.done_and_empty() and self.audio_producer.done_and_empty(): return
            if not reported and self.audio.bytes_sent>=target and self.mpeg.bytes_sent>=target:
                reported=True; print(f"H1_INTERACTIVE_MEDIA_PREFILL_COMPLETE audio_sent={self.audio.bytes_sent} mpeg_sent={self.mpeg.bytes_sent} rfb_sent={self.rfb.bytes_sent}",flush=True)
            sent=False
            for _ in range(3):
                channel=schedule[index]; index=(index+1)%3
                sent=_send_rfb(self) if channel==CHANNEL_RFB else _send_media(self,channel)
                if sent: break
            if not sent:
                with self.condition: self.condition.wait(timeout=0.001)
    finally: stop.set(); t.join(timeout=2.0)
base.H1Session.schedule_media = _schedule_interactive
def _run_interactive(self):
    self.cumulative39_census_records={}
    reader=threading.Thread(target=self.reader,daemon=True); reader.start()
    try:
        if self.profile["rfb_mode"]: self.rfb_proxy=RfbProxy(self.evidence)
        if not self.hello_event.wait(timeout=10.0): raise base.ProtocolError("HELLO timeout")
        self.check_reader(); self.send_frame(base.FRAME_CONFIG,base.CHANNEL_CONTROL,self.config_payload)
        print("H1_CONFIG_SENT="+json.dumps(self.profile,sort_keys=True),flush=True)
        if not self.config_ack_event.wait(timeout=10.0): raise base.ProtocolError("CONFIG timeout")
        self.check_reader(); self.start_producers(); self.schedule_media(); metadata=self.send_media_end()
        if not self.result_event.wait(timeout=60.0): raise base.ProtocolError("RESULT timeout")
        self.check_reader(); self.validate_result(metadata)
        print(f"H1_INTERACTIVE_RFB_BYTES server_to_ps2={self.rfb.bytes_sent} ps2_to_server={self.rfb_proxy.bytes_to_server}",flush=True)
    finally:
        cumulative._print_census_summary(self)
        if self.rfb_proxy is not None: self.rfb_proxy.stop(); self.rfb_proxy=None
        self.stop_event.set()
        try: self.sock.shutdown(socket.SHUT_RDWR)
        except OSError: pass
        try: self.sock.close()
        except OSError: pass
        reader.join(timeout=1.0)
base.H1Session.run = _run_interactive
if __name__ == "__main__": raise SystemExit(base.main())
''')

write(ROOT / "mk/media-harness-h1-cumulative39-interactive.mk", '''
BUILD_DIR := build/experiments/media-harness-h1-cumulative39-interactive/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-Cumulative39-Interactive.ELF
include mk/media-harness-h1-cumulative39-thread-census.mk
EE_INCS := -Iexperiments/media-harness-h1/cumulative39-interactive-include $(EE_INCS)
EE_OBJS += $(BUILD_DIR)/h1_rfb_mux_io.o $(BUILD_DIR)/h1_cumulative39_interactive.o
$(EE_BIN): $(BUILD_DIR)/h1_rfb_mux_io.o $(BUILD_DIR)/h1_cumulative39_interactive.o
$(BUILD_DIR)/h1_main.o: experiments/media-harness-h1/h1_main_cumulative39_interactive.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/h1_config.o: experiments/media-harness-h1/h1_config_cumulative39_interactive.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/h1_transport_runtime.o: experiments/media-harness-h1/h1_transport_runtime_cumulative39_interactive.c experiments/media-harness-h1/cumulative39-interactive-include/h1_transport_runtime.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/h1_video_runtime.o: experiments/media-harness-h1/h1_video_runtime_cumulative39_interactive.c experiments/media-harness-h1/h1_cumulative39_graphics.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/ps2_network_h1.o: src/platform/ps2_network.c src/platform/ps2_network.h src/rfb/rfb_io.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -Dpstvnc_rfb_io_read_exact=pstvnc_direct_rfb_io_read_exact -Dpstvnc_rfb_io_poll_receive=pstvnc_direct_rfb_io_poll_receive -Dpstvnc_rfb_io_write_exact=pstvnc_direct_rfb_io_write_exact -c $< -o $@
$(BUILD_DIR)/ps2_graphics39.o: experiments/media-harness-h1/h1_cumulative39_graphics.c experiments/media-harness-h1/h1_cumulative39_graphics.h src/platform/ps2_graphics.c src/platform/ps2_graphics.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/h1_rfb_mux_io.o: experiments/media-harness-h1/h1_rfb_mux_io.c experiments/media-harness-h1/h1_rfb_mux_io.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
$(BUILD_DIR)/h1_cumulative39_interactive.o: experiments/media-harness-h1/h1_cumulative39_interactive.c experiments/media-harness-h1/h1_cumulative39_interactive.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
''')

print("H1_CUMULATIVE39_INTERACTIVE_GENERATION=PASS")
