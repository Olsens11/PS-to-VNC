/*
 * File synopsis:
 * Resident H1 audio/video media-harness coordinator.
 *
 * Process lifetime:
 *   once: prepare IOP/network/link and GS/IPU display chassis
 *   loop: fresh PSTV mux connection -> CONFIG -> session -> RESULT -> teardown
 *
 * A failed or completed media session therefore does not require restarting
 * the ELF unless the experiment itself wedges/crashes the PS2. That is the
 * intended boundary-finding behavior for automated H1 batches.
 */

#include "h1_audio_runtime.h"
#include "h1_media_clock.h"
#include "h1_transport_runtime.h"
#include "h1_video_runtime.h"

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

static int h1_wait_for_audio_completion(
    pstvnc_h1_audio_runtime_t *audio)
{
    while (!pstvnc_h1_audio_finished(audio)) {
        if (DelayThread(H1_AUDIO_FINISH_POLL_US) < 0)
            return -1;
    }

    return 0;
}

int main(void)
{
    uint32_t completed_sessions = 0u;

    printf("H1_RESIDENT_MEDIA_HARNESS_START\n");

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

    if (pstvnc_h1_video_chassis_init() < 0) {
        printf("H1_BOOT=VIDEO_CHASSIS_FAIL\n");
        SleepThread();
        return 13;
    }

    printf("H1_BOOT=PASS\n");

    for (;;) {
        pstvnc_h1_transport_runtime_t transport;
        pstvnc_h1_audio_runtime_t audio;
        pstvnc_h1_media_clock_t clock;
        pstvnc_h1_video_result_t video;
        const pstvnc_h1_config_t *config;
        int audio_active = 0;
        int video_result_code = 0;
        int session_ok = 1;
        uint32_t diagnostic_word;

        memset(&audio, 0, sizeof(audio));
        memset(&video, 0, sizeof(video));

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

        pstvnc_h1_media_clock_init(
            &clock,
            config->media_epoch_lead_us);

        printf(
            "H1_SESSION_BEGIN id=%u profile=%u audio_mode=%u video_mode=%u "
            "audio_offset_us=%d video_offset_us=%d epoch_lead_us=%u\n",
            (unsigned int)config->session_id,
            (unsigned int)config->profile_id,
            (unsigned int)config->audio_mode,
            (unsigned int)config->video_mode,
            (int)pstvnc_h1_config_audio_offset_us(config),
            (int)pstvnc_h1_config_video_offset_us(config),
            (unsigned int)config->media_epoch_lead_us);

        if (config->audio_mode == PSTVNC_H1_AUDIO_PCM) {
            if (pstvnc_h1_audio_load_modules_once() < 0) {
                printf("H1_AUDIO=MODULE_LOAD_FAIL\n");
                session_ok = 0;
            } else if (pstvnc_h1_audio_start(
                    &audio,
                    &transport,
                    &clock) < 0) {
                printf(
                    "H1_AUDIO=START_FAIL error=%d\n",
                    (int)pstvnc_h1_audio_last_error(&audio));
                session_ok = 0;
            } else {
                audio_active = 1;
                printf("H1_AUDIO=STARTED\n");
            }
        }

        if (session_ok &&
            config->video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
            video_result_code = pstvnc_h1_video_run_session(
                &transport,
                &clock,
                &video);

            if (video_result_code < 0) {
                printf(
                    "H1_VIDEO=FAIL error=%d decoded=%u displayed=%u dropped=%u\n",
                    (int)video.error,
                    (unsigned int)video.pictures_decoded,
                    (unsigned int)video.pictures_displayed,
                    (unsigned int)video.pictures_dropped);
                session_ok = 0;
            } else {
                printf(
                    "H1_VIDEO=PASS decoded=%u displayed=%u dropped=%u "
                    "misses=%u max_late_ticks=%llu\n",
                    (unsigned int)video.pictures_decoded,
                    (unsigned int)video.pictures_displayed,
                    (unsigned int)video.pictures_dropped,
                    (unsigned int)video.deadline_misses,
                    (unsigned long long)video.max_deadline_late_ticks);
            }
        } else if (session_ok) {
            /* Audio-only/control sessions still need a defined common epoch. */
            pstvnc_h1_media_clock_arm_now(&clock);

            while (!transport.receiver_done &&
                   pstvnc_h1_transport_last_error(&transport) ==
                        PSTVNC_H1_ERROR_NONE) {
                if (DelayThread(H1_AUDIO_FINISH_POLL_US) < 0) {
                    session_ok = 0;
                    break;
                }
            }
        }

        if (audio_active && session_ok) {
            if (h1_wait_for_audio_completion(&audio) < 0) {
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

        diagnostic_word =
            0xB1000000u |
            (((uint32_t)video.error & 0xffu) << 8) |
            ((uint32_t)pstvnc_h1_audio_last_error(&audio) & 0xffu);

        pstvnc_h1_transport_set_diagnostic_word(
            &transport,
            diagnostic_word);

        if (!pstvnc_h1_transport_send_result(
                &transport,
                video.pictures_decoded,
                video.pictures_displayed,
                video.feed_calls,
                video.payload_bytes_submitted,
                video.dma_bytes_submitted,
                video.deadline_misses,
                (uint32_t)video.max_deadline_late_ticks)) {
            printf(
                "H1_RESULT=SEND_FAIL transport_error=%d\n",
                (int)pstvnc_h1_transport_last_error(&transport));
            session_ok = 0;
        } else {
            printf("H1_RESULT=SEND_PASS\n");
        }

        if (audio_active && pstvnc_h1_audio_shutdown(&audio) < 0) {
            printf("H1_TEARDOWN=AUDIO_FAIL\n");
            session_ok = 0;
        }

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
