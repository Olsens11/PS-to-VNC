/*
 * File synopsis:
 * Supplies the minimal PS2SDK AUDSRV declarations needed to compile the clean
 * A002 concrete audio-service adapter in deterministic host tests. The stub
 * mirrors the production EE API shape without providing playback behavior.
 */

#ifndef PSTVNC_TEST_AUDIO_HOST_AUDSRV_H
#define PSTVNC_TEST_AUDIO_HOST_AUDSRV_H

typedef struct audsrv_fmt_t {
    int freq;
    int bits;
    int channels;
} audsrv_fmt_t;

int audsrv_init(void);
int audsrv_quit(void);
int audsrv_set_format(audsrv_fmt_t *format);
int audsrv_wait_audio(int bytes);
int audsrv_set_volume(int volume);
int audsrv_play_audio(const char *chunk, int bytes);
int audsrv_stop_audio(void);

#endif
