/*
 * File synopsis:
 * CP2M visible RFB plus PS2 mouse and transient L1+D-pad keyboard coordinator.
 *
 * This executable extends the hardware-qualified CP2L boundary only by
 * serializing the historical Test11F L1+D-pad arrow-key chord on the same
 * RFB-owning main thread. The controller worker remains a semantic producer.
 * AUDIO, MPEG, OSK, local UI, and general runtime-hotkey behavior remain
 * inactive.
 *
 * The Pi-to-PS2 transport remains exactly one physical PSTV TCP connection.
 * This is not the future hybrid compositor and does not begin Issue #40.
 */

#include "display.h"
#include "h1_config.h"
#include "h1_rfb_keyboard_input_service.h"
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

static uint16_t s_h1_visible_keyboard_gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));

static int h1_visible_keyboard_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    (void)context;

    if (!pstvnc_display_prepare_gs16(
            framebuffer,
            s_h1_visible_keyboard_gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        return 0;

    if (pstvnc_ps2_graphics_present(
            s_h1_visible_keyboard_gs_pixels,
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

    static pstvnc_h1_rfb_keyboard_input_service_t input_service;

    printf("H1_CP2M_VISIBLE_RFB_KEYBOARD_START\n");

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

    if (pstvnc_ps2_graphics_init() < 0) {
        printf("H1_BOOT=GRAPHICS_INIT_FAIL\n");
        SleepThread();
        return 13;
    }

    printf(
        "H1_BOOT=PASS mode=RFB_VISIBLE_KEYBOARD audio=0 mpeg=0 input=1 "
        "keyboard=1 osk=0 local_ui=0\n");

    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_rfb_session_runtime_t rfb;
        const pstvnc_h1_config_t *config;
        int session_ok = 1;
        int input_shutdown_failed = 0;
        int rfb_result_code = -1;

        pstvnc_h1_rfb_session_runtime_init(&rfb);
        pstvnc_h1_rfb_keyboard_input_service_init(&input_service);

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

        if (config->rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
            config->audio_mode != PSTVNC_H1_AUDIO_OFF ||
            config->video_mode != PSTVNC_H1_VIDEO_OFF) {
            printf("H1_CP2M=PROFILE_REJECT_NOT_VISIBLE_RFB_ONLY\n");
            session_ok = 0;
        }

        if (session_ok) {
            rfb_result_code =
                pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(
                    &rfb,
                    &transport,
                    h1_visible_keyboard_present,
                    NULL,
                    pstvnc_h1_rfb_keyboard_input_service_service,
                    &input_service);

            if (pstvnc_h1_rfb_keyboard_input_service_shutdown(
                    &input_service) < 0) {
                printf("H1_CP2M=INPUT_SHUTDOWN_UNPROVEN\n");
                input_shutdown_failed = 1;
                session_ok = 0;
            }

            if (rfb_result_code < 0) {
                printf(
                    "H1_RFB_VISIBLE_KEYBOARD=FAIL state=%u error=%u "
                    "handshake=%u initial=%u updates=%u service=%u "
                    "worker_error=%u\n",
                    (unsigned int)rfb.session.state,
                    (unsigned int)rfb.session.error,
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.application_service_calls,
                    (unsigned int)pstvnc_input_runtime_last_error(
                        &input_service.input_runtime));
                session_ok = 0;
            } else if (h1_wait_for_transport_end(&transport) < 0) {
                printf(
                    "H1_RFB_VISIBLE_KEYBOARD=END_WAIT_FAIL "
                    "transport_error=%d end=%d done=%d\n",
                    (int)pstvnc_h1_transport_last_error(&transport),
                    (int)transport.end_received,
                    (int)transport.receiver_done);
                session_ok = 0;
            } else {
                printf(
                    "H1_RFB_VISIBLE_KEYBOARD=PASS handshake=%u initial=%u "
                    "requests=%u updates=%u initial_present=%u "
                    "incremental_present=%u idle=%u service=%u "
                    "controller_events=%u mouse_events=%u pointer_messages=%u "
                    "wheel_pulses=%u keyboard_taps=%u keyboard_messages=%u "
                    "chord_entries=%u chord_exits=%u boundary=%u commit=%u "
                    "complete=%u\n",
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_requests_sent,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.initial_presentations,
                    (unsigned int)rfb.stats.incremental_presentations,
                    (unsigned int)rfb.stats.idle_polls,
                    (unsigned int)rfb.stats.application_service_calls,
                    (unsigned int)input_service.controller_state_events_consumed,
                    (unsigned int)input_service.mouse_update_events_consumed,
                    (unsigned int)input_service.pointer_messages_sent,
                    (unsigned int)input_service.wheel_pulses_sent,
                    (unsigned int)input_service.keyboard_taps_sent,
                    (unsigned int)input_service.keyboard_messages_sent,
                    (unsigned int)input_service.keyboard_chord_entries,
                    (unsigned int)input_service.keyboard_chord_exits,
                    (unsigned int)rfb.stats.quiesce_boundary_sent,
                    (unsigned int)rfb.stats.quiesce_commit_observed,
                    (unsigned int)rfb.stats.quiesce_complete_sent);
            }
        }

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

        if (input_shutdown_failed) {
            printf("H1_CP2M=HALT_UNPROVEN_INPUT_DORMANCY\n");
            SleepThread();
            return 22;
        }

        if (DelayThread(H1_BETWEEN_SESSION_DELAY_US) < 0) {
            printf("H1_SESSION=BETWEEN_DELAY_FAIL\n");
            SleepThread();
            return 21;
        }
    }
}
