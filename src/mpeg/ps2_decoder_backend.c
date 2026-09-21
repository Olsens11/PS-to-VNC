/*
 * File synopsis:
 * Implements A003 R3's concrete PS2 SMS-libmpeg/IPU backend.
 *
 * Qualified mechanisms reconstructed from frozen H1 are deliberately narrow:
 * - unconditional D3/D4 plus IPU reset/BCLR known-state preparation;
 * - the exact pinned mature SMS libmpeg process-global decoder;
 * - RGB16 through _MPEG_Set16(1);
 * - ordinary aligned TO_IPU DMA for decoder-supplied feed buffers;
 * - first MPEG_Picture(NULL, ...) followed by the decoder-owned sequence buffer;
 * - TO_IPU ownership wait before MPEG_Destroy().
 *
 * Historical GIF/GS upload, VRAM, VBlank, timing, diagnostics and presentation
 * mechanisms are intentionally absent. Local stop also remains absent from this
 * layer: the decoder feed callback decides only real Transport stream facts.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#include "ps2_decoder_backend.h"

#include "libmpeg.h"
#include "libmpeg_internal.h"

#include <dma.h>
#include <ee_regs.h>
#include <kernel.h>
#include <tamtypes.h>

#include <stdint.h>
#include <string.h>

#define PSTVNC_PS2_MPEG_DMAC_START 0x00000100u
#define PSTVNC_PS2_MPEG_IPU_CTRL_BUSY 0x80000000u
#define PSTVNC_PS2_MPEG_IPU_CTRL_RST 0x40000000u
#define PSTVNC_PS2_MPEG_IPU_CMD_BCLR 0x00000000u

static pstvnc_ps2_mpeg_decoder_backend_t *pstvnc_ps2_mpeg_active_backend;

static void pstvnc_ps2_mpeg_known_state_reset(void)
{
    int interrupt_was_enabled;
    u32 dmac_enable_state;
    u32 old_ipu_control;

    interrupt_was_enabled = DIntr();
    dmac_enable_state = *R_EE_D_ENABLER;

    *R_EE_D_ENABLEW = dmac_enable_state | 0x00010000u;
    EE_SYNCL();

    *R_EE_D3_CHCR &= ~PSTVNC_PS2_MPEG_DMAC_START;
    *R_EE_D4_CHCR &= ~PSTVNC_PS2_MPEG_DMAC_START;
    *R_EE_D3_MADR = 0;
    *R_EE_D4_MADR = 0;
    *R_EE_D3_QWC = 0;
    *R_EE_D4_QWC = 0;

    EE_SYNCL();
    *R_EE_D_ENABLEW = dmac_enable_state;

    if (interrupt_was_enabled)
        EIntr();

    old_ipu_control =
        *R_EE_IPU_CTRL &
        ~(PSTVNC_PS2_MPEG_IPU_CTRL_BUSY | PSTVNC_PS2_MPEG_IPU_CTRL_RST);

    *R_EE_IPU_CTRL = PSTVNC_PS2_MPEG_IPU_CTRL_RST;
    while (*R_EE_IPU_CTRL & PSTVNC_PS2_MPEG_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CMD = PSTVNC_PS2_MPEG_IPU_CMD_BCLR;
    while (*R_EE_IPU_CTRL & PSTVNC_PS2_MPEG_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CTRL = old_ipu_control;
}

static int pstvnc_ps2_mpeg_reserve_backend(
    pstvnc_ps2_mpeg_decoder_backend_t *backend)
{
    int interrupt_was_enabled;

    if (backend == NULL)
        return 0;

    interrupt_was_enabled = DIntr();

    if (pstvnc_ps2_mpeg_active_backend != NULL ||
        backend->known_state_prepared ||
        backend->decoder_initialized) {
        if (interrupt_was_enabled)
            EIntr();
        return 0;
    }

    pstvnc_ps2_mpeg_active_backend = backend;

    if (interrupt_was_enabled)
        EIntr();

    return 1;
}

static int pstvnc_ps2_mpeg_release_backend_owner(
    pstvnc_ps2_mpeg_decoder_backend_t *backend)
{
    int interrupt_was_enabled;

    interrupt_was_enabled = DIntr();

    if (backend == NULL || pstvnc_ps2_mpeg_active_backend != backend) {
        if (interrupt_was_enabled)
            EIntr();
        return 0;
    }

    pstvnc_ps2_mpeg_active_backend = NULL;

    if (interrupt_was_enabled)
        EIntr();

    return 1;
}

static int pstvnc_ps2_mpeg_prepare_known_state(void *context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;

    if (!pstvnc_ps2_mpeg_reserve_backend(backend))
        return -1;

    pstvnc_ps2_mpeg_known_state_reset();
    dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);

    backend->known_state_prepared = 1;
    return 0;
}

/*
 * SMS libmpeg data callback.
 *
 * Deliberately delegates directly to the accepted decoder feed callback and
 * never consults local stop or manufactures EOF.
 */
static int pstvnc_ps2_mpeg_sms_feed(void *context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;

    if (backend == NULL ||
        backend->feed_callback == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend)
        return -1;

    return backend->feed_callback(backend->feed_context);
}

static void *pstvnc_ps2_mpeg_sms_sequence(
    void *context,
    MPEGSequenceInfo *sequence)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;
    void *picture;
    size_t picture_capacity = 0u;

    if (backend == NULL ||
        sequence == NULL ||
        backend->sequence_callback == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend ||
        sequence->m_Width <= 0 ||
        sequence->m_Height <= 0)
        return NULL;

    picture = backend->sequence_callback(
        backend->sequence_context,
        (uint32_t)sequence->m_Width,
        (uint32_t)sequence->m_Height,
        &picture_capacity);

    if (picture == NULL || picture_capacity == 0u)
        return NULL;

    backend->sequence_picture = picture;
    backend->sequence_picture_capacity = picture_capacity;
    return picture;
}

static int pstvnc_ps2_mpeg_initialize(
    void *context,
    pstvnc_mpeg_feed_callback_t feed_callback,
    void *feed_context,
    pstvnc_mpeg_sequence_callback_t sequence_callback,
    void *sequence_context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;

    if (backend == NULL ||
        feed_callback == NULL ||
        sequence_callback == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend ||
        !backend->known_state_prepared ||
        backend->decoder_initialized)
        return -1;

    backend->feed_callback = feed_callback;
    backend->feed_context = feed_context;
    backend->sequence_callback = sequence_callback;
    backend->sequence_context = sequence_context;
    backend->sequence_picture = NULL;
    backend->sequence_picture_capacity = 0u;
    backend->current_stream_pts = 0;
    backend->picture_pts = 0;
    backend->first_picture_pending = 1;

    MPEG_Initialize(
        pstvnc_ps2_mpeg_sms_feed,
        backend,
        pstvnc_ps2_mpeg_sms_sequence,
        backend,
        (s64 *)&backend->current_stream_pts);

    _MPEG_Set16(1);
    backend->decoder_initialized = 1;
    return 0;
}

static int pstvnc_ps2_mpeg_picture(void *context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;
    void *picture_target;
    int result;

    if (backend == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend ||
        !backend->decoder_initialized ||
        MPEG_Picture == NULL)
        return -1;

    if (backend->first_picture_pending) {
        picture_target = NULL;
    } else {
        if (backend->sequence_picture == NULL)
            return -1;
        picture_target = backend->sequence_picture;
    }

    backend->picture_pts = 0;
    result = MPEG_Picture(
        picture_target,
        (s64 *)&backend->picture_pts);

    if (result > 0) {
        if (backend->sequence_picture == NULL ||
            backend->sequence_picture_capacity == 0u)
            return -1;
        backend->first_picture_pending = 0;
    }

    return result;
}

static int pstvnc_ps2_mpeg_submit_feed(
    void *context,
    const uint8_t *bytes,
    size_t payload_bytes,
    size_t transfer_bytes)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;
    uint32_t qwc;

    if (backend == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend ||
        !backend->decoder_initialized ||
        bytes == NULL ||
        payload_bytes == 0u ||
        payload_bytes > transfer_bytes ||
        transfer_bytes == 0u ||
        (transfer_bytes & 15u) != 0u ||
        ((uintptr_t)bytes & 15u) != 0u ||
        transfer_bytes > (size_t)UINT32_MAX)
        return -1;

    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0)
        return -1;

    qwc = (uint32_t)(transfer_bytes >> 4);
    if (qwc == 0u)
        return -1;

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        (void *)bytes,
        qwc,
        0,
        0);

    return 0;
}

static int pstvnc_ps2_mpeg_destroy(void *context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;

    if (backend == NULL ||
        pstvnc_ps2_mpeg_active_backend != backend ||
        !backend->decoder_initialized)
        return -1;

    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0)
        return -1;

    MPEG_Destroy();

    backend->decoder_initialized = 0;
    backend->sequence_picture = NULL;
    backend->sequence_picture_capacity = 0u;
    backend->current_stream_pts = 0;
    backend->picture_pts = 0;
    backend->first_picture_pending = 1;
    return 0;
}

static int pstvnc_ps2_mpeg_release_known_state(void *context)
{
    pstvnc_ps2_mpeg_decoder_backend_t *backend =
        (pstvnc_ps2_mpeg_decoder_backend_t *)context;

    if (backend == NULL ||
        !backend->known_state_prepared ||
        backend->decoder_initialized ||
        pstvnc_ps2_mpeg_active_backend != backend)
        return -1;

    if (!pstvnc_ps2_mpeg_release_backend_owner(backend))
        return -1;

    memset(backend, 0, sizeof(*backend));
    return 0;
}

void pstvnc_ps2_mpeg_decoder_backend_init(
    pstvnc_ps2_mpeg_decoder_backend_t *backend)
{
    if (backend != NULL)
        memset(backend, 0, sizeof(*backend));
}

int pstvnc_ps2_mpeg_decoder_backend_platform_ops(
    pstvnc_ps2_mpeg_decoder_backend_t *backend,
    pstvnc_mpeg_decoder_platform_ops_t *platform_ops)
{
    if (backend == NULL || platform_ops == NULL)
        return 0;

    memset(platform_ops, 0, sizeof(*platform_ops));
    platform_ops->prepare_known_state = pstvnc_ps2_mpeg_prepare_known_state;
    platform_ops->initialize = pstvnc_ps2_mpeg_initialize;
    platform_ops->picture = pstvnc_ps2_mpeg_picture;
    platform_ops->submit_feed = pstvnc_ps2_mpeg_submit_feed;
    platform_ops->destroy = pstvnc_ps2_mpeg_destroy;
    platform_ops->release_known_state = pstvnc_ps2_mpeg_release_known_state;
    platform_ops->context = backend;
    return 1;
}
