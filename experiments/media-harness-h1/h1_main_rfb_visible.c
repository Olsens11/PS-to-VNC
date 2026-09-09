/*
 * File synopsis:
 * CP2K RFB-only visible-presentation coordinator.
 *
 * This deliberately narrow executable keeps the CP2J one-socket RFB transport,
 * unchanged through-Issue-39 parser, CPU framebuffer, credit policy, and clean
 * quiesce while reconnecting only the already-existing through-Issue-39 display
 * conversion and PS2 graphics presenter. AUDIO, MPEG, controller input, pointer,
 * keyboard, OSK, and local UI are never started by this coordinator.
 *
 * This is not the future hybrid compositor. It is an isolated hardware gate for
 * proving that a complete mux-fed RFB framebuffer can once again become visible
 * on the television before any second presentation plane is admitted.
 */

#include "display.h"
#include "h1_config.h"
#include "h1_rfb_session_runtime.h"
#include "h1_transport_runtime.h"
#include "platform/ps2_graphics.h"
#include "ps2_network.h"
#include "ps2_system.h"

#include <delaythread.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define H1_CONNECT_RETRY_DELAY_US 250000u
#define H1_TRANSPORT_END_POLL_US 1000u
#define H1_BETWEEN_SESSION_DELAY_US 100000u

/*
 * Disposable GS presentation pixels. The authoritative remote desktop remains
 * the dynamically allocated framebuffer owned by h1_rfb_session_runtime.
 */
static uint16_t s_h1_visible_gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));

static int h1_visible_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    (void)context;

    if (!pstvnc_display_prepare_gs16(
            framebuffer,
            s_h1_visible_gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        return 0;

    if (pstvnc_ps2_graphics_present(
            s_h1_visible_gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT,
            NULL) < 0)
        return 0;

    return 1;
}

static int h1_wait_for_session_transport(
    pstvnc_h1_transport_runtime_t *transport)
{
    for (;;) {
        memset(transport, 0, sizeof(*transport));

        if (pstvnc_h1_transport_start(transport) == 0)
            return 0;

        printf(
            "H1_WAITING_FOR_SESSION connect_error=%d\n",
            (int)transport->error);

        if (DelayThread(H1_CONNECT_RETRY_DELAY_US) < 0)
            return -1;
    }
}

static int h1_wait_for_transport_end(
    pstvnc_h1_transport_runtime_t *transport)
{
    if (transport == NULL)
        return -1;

    while (!transport->receiver_done &&
           pstvnc_h1_transport_last_error(transport) ==
                PSTVNC_H1_ERROR_NONE) {
        if (DelayThread(H1_TRANSPORT_END_POLL_US) < 0)
            return -1;
    }

    return transport->receiver_done &&
        transport->end_received &&
        pstvnc_h1_transport_last_error(transport) ==
            PSTVNC_H1_ERROR_NONE
        ? 0
        : -1;
}

int main(void)
{
    uint32_t completed_sessions = 0u;

    printf("H1_CP2K_VISIBLE_RFB_START\n");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        printf("H1_BOOT=IOP_PREPARE_FAIL\n");
        SleepThread();
        return 10;
    }

    if (pstvnc_ps2_network_init() < 0) {
        printf("H1_BOOT=NETWORK_INIT_FAIL\n");
        SleepThread();
        return 11;
    }

    if (pstvnc_ps2_network_wait_link() < 0) {
        printf("H1_BOOT=LINK_FAIL\n");
        SleepThread();
        return 12;
    }

    /*
     * CP2K has exactly one graphics owner: the clean through-Issue-39 PS2
     * platform presenter. The H1 MPEG video chassis is intentionally not
     * initialized in this executable.
     */
    if (pstvnc_ps2_graphics_init() < 0) {
        printf("H1_BOOT=GRAPHICS_INIT_FAIL\n");
        SleepThread();
        return 13;
    }

    printf(
        "H1_BOOT=PASS mode=RFB_VISIBLE_ONLY audio=0 mpeg=0 input=0 osk=0\n");

    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_rfb_session_runtime_t rfb;
        const pstvnc_h1_config_t *config;
        int session_ok = 1;
        int rfb_result_code;

        pstvnc_h1_rfb_session_runtime_init(&rfb);

        printf(
            "H1_WAITING_FOR_SESSION completed=%u\n",
            (unsigned int)completed_sessions);

        if (h1_wait_for_session_transport(&transport) < 0) {
            printf("H1_SESSION=TRANSPORT_WAIT_FAIL\n");
            SleepThread();
            return 20;
        }

        config = pstvnc_h1_transport_config(&transport);
        if (config == NULL) {
            printf("H1_SESSION=CONFIG_UNAVAILABLE\n");
            (void)pstvnc_h1_transport_shutdown(&transport);
            continue;
        }

        printf(
            "H1_SESSION_BEGIN id=%u profile=%u audio_mode=%u video_mode=%u "
            "rfb_mode=%u\n",
            (unsigned int)config->session_id,
            (unsigned int)config->profile_id,
            (unsigned int)config->audio_mode,
            (unsigned int)config->video_mode,
            (unsigned int)config->rfb_mode);

        /*
         * This executable is intentionally stricter than the generic H1 CONFIG
         * vocabulary. It cannot silently become a media or interactive test.
         */
        if (config->rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
            config->audio_mode != PSTVNC_H1_AUDIO_OFF ||
            config->video_mode != PSTVNC_H1_VIDEO_OFF) {
            printf("H1_CP2K=PROFILE_REJECT_NOT_RFB_ONLY\n");
            session_ok = 0;
        }

        if (session_ok) {
            rfb_result_code =
                pstvnc_h1_rfb_session_runtime_run_with_presenter(
                    &rfb,
                    &transport,
                    h1_visible_present,
                    NULL);

            if (rfb_result_code < 0) {
                printf(
                    "H1_RFB_VISIBLE=FAIL state=%u error=%u handshake=%u "
                    "initial=%u updates=%u initial_present=%u "
                    "incremental_present=%u boundary=%u commit=%u complete=%u\n",
                    (unsigned int)rfb.session.state,
                    (unsigned int)rfb.session.error,
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.initial_presentations,
                    (unsigned int)rfb.stats.incremental_presentations,
                    (unsigned int)rfb.stats.quiesce_boundary_sent,
                    (unsigned int)rfb.stats.quiesce_commit_observed,
                    (unsigned int)rfb.stats.quiesce_complete_sent);
                session_ok = 0;
            } else if (h1_wait_for_transport_end(&transport) < 0) {
                printf(
                    "H1_RFB_VISIBLE=END_WAIT_FAIL transport_error=%d end=%d "
                    "done=%d\n",
                    (int)pstvnc_h1_transport_last_error(&transport),
                    (int)transport.end_received,
                    (int)transport.receiver_done);
                session_ok = 0;
            } else {
                printf(
                    "H1_RFB_VISIBLE=PASS handshake=%u initial=%u requests=%u "
                    "updates=%u initial_present=%u incremental_present=%u "
                    "idle=%u boundary=%u commit=%u complete=%u\n",
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_requests_sent,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.initial_presentations,
                    (unsigned int)rfb.stats.incremental_presentations,
                    (unsigned int)rfb.stats.idle_polls,
                    (unsigned int)rfb.stats.quiesce_boundary_sent,
                    (unsigned int)rfb.stats.quiesce_commit_observed,
                    (unsigned int)rfb.stats.quiesce_complete_sent);
            }
        }

        /*
         * Preserve the parser/quiesce diagnostic word already published by the
         * RFB runtime. Media counters are intentionally zero in this checkpoint.
         */
        if (!pstvnc_h1_transport_send_result(
                &transport,
                0u,
                0u,
                0u,
                0u,
                0u,
                0u,
                0u)) {
            printf(
                "H1_RESULT=SEND_FAIL transport_error=%d\n",
                (int)pstvnc_h1_transport_last_error(&transport));
            session_ok = 0;
        } else {
            printf("H1_RESULT=SEND_PASS\n");
        }

        pstvnc_h1_rfb_session_runtime_shutdown(&rfb);

        if (pstvnc_h1_transport_shutdown(&transport) < 0) {
            printf("H1_TEARDOWN=TRANSPORT_FAIL\n");
            session_ok = 0;
        }

        completed_sessions += 1u;

        printf(
            "H1_SESSION_END id=%u result=%s completed=%u\n",
            (unsigned int)config->session_id,
            session_ok ? "PASS" : "FAIL",
            (unsigned int)completed_sessions);

        if (DelayThread(H1_BETWEEN_SESSION_DELAY_US) < 0) {
            printf("H1_SESSION=BETWEEN_DELAY_FAIL\n");
            SleepThread();
            return 21;
        }
    }
}
