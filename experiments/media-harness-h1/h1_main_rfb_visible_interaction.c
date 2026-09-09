/*
 * File synopsis:
 * CP2N visible RFB plus real through-Issue-39 local interaction coordinator.
 *
 * This checkpoint retains CP2L's hardware-qualified one-socket muxed RFB,
 * visible desktop, and PS2 mouse path, then composes the existing clean
 * local-controller, local-UI, OSK, keyboard, and overlay-presentation modules.
 * It adds no experiment-specific controller gesture or keyboard meaning.
 *
 * AUDIO and MPEG remain disabled. This is a basic hardware-proof checkpoint,
 * not the later architectural-alignment pass and not Issue #40.
 */

#include "h1_config.h"
#include "h1_interaction_coordinator.h"
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

#define PSTVNC_H1_CP2N_CONNECT_RETRY_DELAY_US 250000u
#define PSTVNC_H1_CP2N_TRANSPORT_END_POLL_US 1000u
#define PSTVNC_H1_CP2N_BETWEEN_SESSION_DELAY_US 100000u

static int h1_cp2n_wait_for_session_transport(
    pstvnc_h1_transport_runtime_t *transport)
{
    for (;;) {
        memset(transport, 0, sizeof(*transport));

        if (pstvnc_h1_transport_start(transport) == 0)
            return 0;

        printf(
            "H1_WAITING_FOR_SESSION connect_error=%d\n",
            (int)transport->error);

        if (DelayThread(PSTVNC_H1_CP2N_CONNECT_RETRY_DELAY_US) < 0)
            return -1;
    }
}

static int h1_cp2n_wait_for_transport_end(
    pstvnc_h1_transport_runtime_t *transport)
{
    if (transport == NULL)
        return -1;

    while (!transport->receiver_done &&
           pstvnc_h1_transport_last_error(transport) ==
                PSTVNC_H1_ERROR_NONE) {
        if (DelayThread(PSTVNC_H1_CP2N_TRANSPORT_END_POLL_US) < 0)
            return -1;
    }

    return transport->receiver_done &&
        transport->end_received &&
        pstvnc_h1_transport_last_error(transport) == PSTVNC_H1_ERROR_NONE
        ? 0
        : -1;
}

int main(void)
{
    uint32_t completed_sessions = 0u;

    /*
     * Static storage is deliberate: if cooperative input shutdown cannot prove
     * worker dormancy, CP2N halts rather than reclaiming or reusing the worker's
     * owned stack and queue storage.
     */
    static pstvnc_h1_interaction_coordinator_t interaction;

    printf("H1_CP2N_VISIBLE_RFB_INTERACTION_START\n");

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
        "H1_BOOT=PASS mode=RFB_VISIBLE_INTERACTION audio=0 mpeg=0 input=1 "
        "keyboard=1 osk=1 local_ui=1\n");

    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_rfb_session_runtime_t rfb;
        const pstvnc_h1_config_t *config;
        int session_ok = 1;
        int interaction_shutdown_failed = 0;
        int rfb_result_code = -1;

        pstvnc_h1_rfb_session_runtime_init(&rfb);
        pstvnc_h1_interaction_coordinator_init(&interaction);

        printf(
            "H1_WAITING_FOR_SESSION completed=%u\n",
            (unsigned int)completed_sessions);

        if (h1_cp2n_wait_for_session_transport(&transport) < 0) {
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
            printf("H1_CP2N=PROFILE_REJECT_NOT_VISIBLE_RFB_ONLY\n");
            session_ok = 0;
        }

        if (session_ok) {
            rfb_result_code =
                pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(
                    &rfb,
                    &transport,
                    pstvnc_h1_interaction_coordinator_present,
                    &interaction,
                    pstvnc_h1_interaction_coordinator_service,
                    &interaction);

            if (pstvnc_h1_interaction_coordinator_shutdown(&interaction) < 0) {
                printf("H1_CP2N=INTERACTION_SHUTDOWN_UNPROVEN\n");
                interaction_shutdown_failed = 1;
                session_ok = 0;
            }

            if (rfb_result_code < 0) {
                printf(
                    "H1_RFB_VISIBLE_INTERACTION=FAIL state=%u error=%u "
                    "handshake=%u initial=%u updates=%u service=%u "
                    "worker_error=%u\n",
                    (unsigned int)rfb.session.state,
                    (unsigned int)rfb.session.error,
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.application_service_calls,
                    (unsigned int)pstvnc_input_runtime_last_error(
                        &interaction.input_runtime));
                session_ok = 0;
            } else if (h1_cp2n_wait_for_transport_end(&transport) < 0) {
                printf(
                    "H1_RFB_VISIBLE_INTERACTION=END_WAIT_FAIL "
                    "transport_error=%d end=%d done=%d\n",
                    (int)pstvnc_h1_transport_last_error(&transport),
                    (int)transport.end_received,
                    (int)transport.receiver_done);
                session_ok = 0;
            } else {
                printf(
                    "H1_RFB_VISIBLE_INTERACTION=PASS handshake=%u initial=%u "
                    "requests=%u updates=%u initial_present=%u "
                    "incremental_present=%u idle=%u service=%u "
                    "controller_events=%u mouse_events=%u pointer_messages=%u "
                    "wheel_pulses=%u keyboard_taps=%u key_messages=%u "
                    "osk_open=%u osk_close=%u local_present=%u "
                    "boundary=%u commit=%u complete=%u\n",
                    (unsigned int)rfb.stats.handshake_complete,
                    (unsigned int)rfb.stats.initial_frame_complete,
                    (unsigned int)rfb.stats.incremental_requests_sent,
                    (unsigned int)rfb.stats.incremental_updates_complete,
                    (unsigned int)rfb.stats.initial_presentations,
                    (unsigned int)rfb.stats.incremental_presentations,
                    (unsigned int)rfb.stats.idle_polls,
                    (unsigned int)rfb.stats.application_service_calls,
                    (unsigned int)interaction.stats.controller_state_events_consumed,
                    (unsigned int)interaction.stats.mouse_update_events_consumed,
                    (unsigned int)interaction.stats.pointer_messages_sent,
                    (unsigned int)interaction.stats.wheel_pulses_sent,
                    (unsigned int)interaction.stats.keyboard_taps_published,
                    (unsigned int)interaction.stats.key_messages_sent,
                    (unsigned int)interaction.stats.osk_open_count,
                    (unsigned int)interaction.stats.osk_close_count,
                    (unsigned int)interaction.stats.local_presentations,
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

        completed_sessions++;

        printf(
            "H1_SESSION_END id=%u result=%s completed=%u\n",
            (unsigned int)config->session_id,
            session_ok ? "PASS" : "FAIL",
            (unsigned int)completed_sessions);

        if (interaction_shutdown_failed) {
            printf("H1_CP2N=HALT_UNPROVEN_INPUT_DORMANCY\n");
            SleepThread();
            return 22;
        }

        if (DelayThread(PSTVNC_H1_CP2N_BETWEEN_SESSION_DELAY_US) < 0) {
            printf("H1_SESSION=BETWEEN_DELAY_FAIL\n");
            SleepThread();
            return 21;
        }
    }
}
