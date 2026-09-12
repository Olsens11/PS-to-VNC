/*
 * File synopsis:
 * CP2P application shell mechanically derived from the qualified CP2O
 * visible-RFB + interaction + optional-PCM resident main.
 *
 * The CP2O source remains byte-for-byte untouched. This derivative changes
 * only application-level composition: it instantiates the CP2P session
 * coordinator so calibration acceptance owns generation prepare/START and the
 * combined RFB policy. Item #10 now accepts the exact visible-RFB + MPEG2 +
 * optional-PCM CONFIG while START remains the sole decode-generation trigger.
 */
#include "h1_audio_runtime.h"
#include "h1_config.h"
#include "h1_interaction_coordinator.h"
#include "h1_media_clock.h"
#include "h1_rfb_session_runtime.h"
#include "h1_transport_runtime.h"
#include "h1_cp2p_session_coordinator.h"
#include "h1_cp2p_mpeg_worker.h"
#include "platform/ps2_graphics.h"
#include "ps2_network.h"
#include "ps2_system.h"

#include <delaythread.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PSTVNC_H1_CP2O_CONNECT_RETRY_DELAY_US 250000u
#define PSTVNC_H1_CP2O_AUDIO_FINISH_POLL_US 1000u
#define PSTVNC_H1_CP2O_TRANSPORT_END_POLL_US 1000u
#define PSTVNC_H1_CP2O_BETWEEN_SESSION_DELAY_US 100000u

static int h1_cp2o_wait_for_session_transport(
    pstvnc_h1_transport_runtime_t *transport)
{
    for (;;) {
        memset(transport, 0, sizeof(*transport));

        if (pstvnc_h1_transport_start(transport) == 0)
            return 0;

        printf(
            "H1_WAITING_FOR_SESSION connect_error=%d\n",
            (int)transport->error);

        if (DelayThread(PSTVNC_H1_CP2O_CONNECT_RETRY_DELAY_US) < 0)
            return -1;
    }
}

static int h1_cp2o_wait_for_transport_end(
    pstvnc_h1_transport_runtime_t *transport)
{
    if (transport == NULL)
        return -1;

    while (!transport->receiver_done &&
           pstvnc_h1_transport_last_error(transport) ==
                PSTVNC_H1_ERROR_NONE) {
        if (DelayThread(PSTVNC_H1_CP2O_TRANSPORT_END_POLL_US) < 0)
            return -1;
    }

    return transport->receiver_done &&
        transport->end_received &&
        pstvnc_h1_transport_last_error(transport) == PSTVNC_H1_ERROR_NONE
        ? 0
        : -1;
}

static int h1_cp2o_wait_for_audio_completion(
    pstvnc_h1_audio_runtime_t *audio)
{
    while (!pstvnc_h1_audio_finished(audio)) {
        if (DelayThread(PSTVNC_H1_CP2O_AUDIO_FINISH_POLL_US) < 0)
            return -1;
    }

    return 0;
}

int main(void)
{
    uint32_t completed_sessions = 0u;
    static pstvnc_h1_cp2p_session_coordinator_t cp2p;

    printf("H1_CP2P_VISIBLE_RFB_INTERACTION_PCM_START\n");

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
        "H1_BOOT=PASS mode=CP2P_ALL_GUNS "
        "mpeg_worker=GENERATION_BOUND_START_ACTIVATED input=1 keyboard=1 osk=1 local_ui=1\n");

    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_rfb_session_runtime_t rfb;
        pstvnc_h1_audio_runtime_t audio;
        pstvnc_h1_media_clock_t clock;
        pstvnc_h1_cp2p_mpeg_worker_t mpeg_worker;
        const pstvnc_h1_config_t *config;
        pstvnc_h1_interaction_coordinator_t *interaction_view;
        int session_ok = 1;
        int interaction_initialized = 0;
        int interaction_shutdown_failed = 0;
        int rfb_result_code = -1;
        int audio_active = 0;
        uint32_t rfb_diagnostic_word = 0u;

        memset(&audio, 0, sizeof(audio));
        memset(&mpeg_worker, 0, sizeof(mpeg_worker));
        pstvnc_h1_rfb_session_runtime_init(&rfb);
        interaction_view = &cp2p.interaction;

        printf(
            "H1_WAITING_FOR_SESSION completed=%u\n",
            (unsigned int)completed_sessions);

        if (h1_cp2o_wait_for_session_transport(&transport) < 0) {
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

        if (config->rfb_mode != PSTVNC_H1_RFB_ON_VISIBLE ||
            config->video_mode != PSTVNC_H1_VIDEO_MPEG2_ES ||
            (config->audio_mode != PSTVNC_H1_AUDIO_OFF &&
             config->audio_mode != PSTVNC_H1_AUDIO_PCM)) {
            printf("H1_CP2P=PROFILE_REJECT_NOT_ALL_GUNS\n");
            session_ok = 0;
        }

        if (session_ok) {
            if (!pstvnc_h1_cp2p_mpeg_worker_init(
                    &mpeg_worker, &transport, &clock)) {
                printf("H1_CP2P=MPEG_WORKER_INIT_FAIL\n");
                session_ok = 0;
            } else if (!pstvnc_h1_cp2p_session_coordinator_init(
                    &cp2p,
                    &transport,
                    pstvnc_h1_cp2p_mpeg_worker_clear,
                    &mpeg_worker)) {
                printf("H1_CP2P=SESSION_COORDINATOR_INIT_FAIL\n");
                session_ok = 0;
            } else {
                interaction_initialized = 1;
                if (!pstvnc_h1_cp2p_session_coordinator_set_mpeg_worker(
                        &cp2p,
                        pstvnc_h1_cp2p_mpeg_worker_arm,
                        &mpeg_worker)) {
                    printf("H1_CP2P=MPEG_WORKER_BIND_FAIL\n");
                    session_ok = 0;
                }
            }
        }

        pstvnc_h1_media_clock_init(&clock, config->media_epoch_lead_us);
        pstvnc_h1_media_clock_arm_now(&clock);

        if (session_ok && config->audio_mode == PSTVNC_H1_AUDIO_PCM) {
            if (pstvnc_h1_audio_load_modules_once() < 0) {
                printf("H1_AUDIO=MODULE_LOAD_FAIL\n");
                session_ok = 0;
            } else if (pstvnc_h1_audio_start(&audio, &transport, &clock) < 0) {
                printf(
                    "H1_AUDIO=START_FAIL error=%d\n",
                    (int)pstvnc_h1_audio_last_error(&audio));
                session_ok = 0;
            } else {
                audio_active = 1;
                printf("H1_AUDIO=STARTED\n");
            }
        }

        if (session_ok) {
            rfb_result_code =
                pstvnc_h1_rfb_session_runtime_run_with_flow_policy(
                    &rfb,
                    &transport,
                    pstvnc_h1_cp2p_session_coordinator_present,
                    &cp2p,
                    pstvnc_h1_cp2p_session_coordinator_service,
                    &cp2p,
                    pstvnc_h1_cp2p_session_coordinator_rfb_policy(&cp2p));

            rfb_diagnostic_word = transport.diagnostic_word;

            if (pstvnc_h1_cp2p_session_coordinator_shutdown(&cp2p) < 0) {
                printf("H1_CP2P=INTERACTION_SHUTDOWN_UNPROVEN\n");
                interaction_shutdown_failed = 1;
                session_ok = 0;
            }
            interaction_initialized = 0;

            if (config->video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
                printf(
                    "H1_CP2P_MPEG_RESULT decoded=%u displayed=%u feed=%u "
                    "payload=%u error=%u\n",
                    (unsigned int)mpeg_worker.result.pictures_decoded,
                    (unsigned int)mpeg_worker.result.pictures_displayed,
                    (unsigned int)mpeg_worker.result.feed_calls,
                    (unsigned int)mpeg_worker.result.payload_bytes_submitted,
                    (unsigned int)mpeg_worker.result.error);
                if (mpeg_worker.failure_latched ||
                    mpeg_worker.run_result != 0 ||
                    mpeg_worker.result.error != PSTVNC_H1_VIDEO_ERROR_NONE ||
                    mpeg_worker.result.pictures_displayed == 0u) {
                    printf(
                        "H1_CP2P=MPEG_RUNTIME_UNPROVEN failure_latched=%d "
                        "run_result=%d error=%u displayed=%u\n",
                        (int)mpeg_worker.failure_latched,
                        (int)mpeg_worker.run_result,
                        (unsigned int)mpeg_worker.result.error,
                        (unsigned int)mpeg_worker.result.pictures_displayed);
                    session_ok = 0;
                }
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
                        &interaction_view->input_runtime));
                session_ok = 0;
            } else if (h1_cp2o_wait_for_transport_end(&transport) < 0) {
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
                    (unsigned int)interaction_view->stats.controller_state_events_consumed,
                    (unsigned int)interaction_view->stats.mouse_update_events_consumed,
                    (unsigned int)interaction_view->stats.pointer_messages_sent,
                    (unsigned int)interaction_view->stats.wheel_pulses_sent,
                    (unsigned int)interaction_view->stats.keyboard_taps_published,
                    (unsigned int)interaction_view->stats.key_messages_sent,
                    (unsigned int)interaction_view->stats.osk_open_count,
                    (unsigned int)interaction_view->stats.osk_close_count,
                    (unsigned int)interaction_view->stats.local_presentations,
                    (unsigned int)rfb.stats.quiesce_boundary_sent,
                    (unsigned int)rfb.stats.quiesce_commit_observed,
                    (unsigned int)rfb.stats.quiesce_complete_sent);
            }
        }

        if (interaction_initialized) {
            if (pstvnc_h1_cp2p_session_coordinator_shutdown(&cp2p) < 0) {
                printf("H1_CP2P=INTERACTION_SHUTDOWN_UNPROVEN\n");
                interaction_shutdown_failed = 1;
                session_ok = 0;
            }
            interaction_initialized = 0;
        }

        if (audio_active && session_ok) {
            if (h1_cp2o_wait_for_audio_completion(&audio) < 0) {
                printf("H1_AUDIO=FINISH_WAIT_FAIL\n");
                session_ok = 0;
            } else if (pstvnc_h1_audio_last_error(&audio) !=
                       PSTVNC_H1_AUDIO_ERROR_NONE) {
                printf(
                    "H1_AUDIO=FAIL error=%d\n",
                    (int)pstvnc_h1_audio_last_error(&audio));
                session_ok = 0;
            } else {
                printf(
                    "H1_AUDIO=PASS played_bytes=%u chunks=%u\n",
                    (unsigned int)transport.stats.audio_bytes_played,
                    (unsigned int)transport.stats.audio_chunks_played);
            }
        }

        if (rfb_result_code >= 0)
            pstvnc_h1_transport_set_diagnostic_word(
                &transport,
                rfb_diagnostic_word);

        if (!pstvnc_h1_transport_send_result(
                &transport,
                mpeg_worker.result.pictures_decoded,
                mpeg_worker.result.pictures_displayed,
                mpeg_worker.result.feed_calls,
                mpeg_worker.result.payload_bytes_submitted,
                mpeg_worker.result.dma_bytes_submitted,
                mpeg_worker.result.deadline_misses,
                (uint32_t)mpeg_worker.result.max_deadline_late_ticks)) {
            printf(
                "H1_RESULT=SEND_FAIL transport_error=%d\n",
                (int)pstvnc_h1_transport_last_error(&transport));
            session_ok = 0;
        } else {
            printf("H1_RESULT=SEND_PASS\n");
        }

        if (mpeg_worker.initialized &&
            pstvnc_h1_cp2p_mpeg_worker_shutdown(&mpeg_worker) < 0) {
            printf("H1_TEARDOWN=MPEG_WORKER_FAIL\n");
            session_ok = 0;
        }

        if (audio_active && pstvnc_h1_audio_shutdown(&audio) < 0) {
            printf("H1_TEARDOWN=AUDIO_FAIL\n");
            session_ok = 0;
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
            printf("H1_CP2P=HALT_UNPROVEN_INPUT_DORMANCY\n");
            SleepThread();
            return 22;
        }

        if (DelayThread(PSTVNC_H1_CP2O_BETWEEN_SESSION_DELAY_US) < 0) {
            printf("H1_SESSION=BETWEEN_DELAY_FAIL\n");
            SleepThread();
            return 21;
        }
    }
}
