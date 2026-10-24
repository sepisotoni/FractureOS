#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "audio.h"

static int is_muted = 1; /* Muted by default per user request */

int audio_init(void) {
    /* Test opening /dev/dsp once */
    int fd = open("/dev/dsp", O_WRONLY | O_NONBLOCK);
    if (fd >= 0) {
        close(fd);
        return 0;
    }
    return -1;
}

void audio_close(void) {
}

void audio_set_muted(int muted) {
    is_muted = muted;
}

int audio_is_muted(void) {
    return is_muted;
}

void audio_play(int sound_id) {
    /* If muted and it's just a click/swoosh, skip. Alarms always play! */
    if (is_muted && (sound_id == SOUND_CLICK || sound_id == SOUND_SWOOSH)) {
        return;
    }

    int fd = open("/dev/dsp", O_WRONLY | O_NONBLOCK);
    if (fd < 0) return;

    int speed = 8000;
    int channels = 1;
    int format = 8;
    ioctl(fd, SNDCTL_DSP_SPEED, &speed);
    ioctl(fd, SNDCTL_DSP_CHANNELS, &channels);
    ioctl(fd, SNDCTL_DSP_SETFMT, &format);

    int samples = 0;
    unsigned char buf[1600]; /* max 200ms */

    switch (sound_id) {
        case SOUND_CLICK:
            samples = 160; /* 20 ms */
            for (int i = 0; i < samples; i++) {
                buf[i] = (i % 8 < 4) ? 150 : 105;
            }
            break;
        case SOUND_SWOOSH:
            samples = 320; /* 40 ms */
            for (int i = 0; i < samples; i++) {
                int period = 16 - (i * 8 / samples);
                if (period < 4) period = 4;
                buf[i] = (i % period < period / 2) ? 160 : 95;
            }
            break;
        case SOUND_CHIME:
            samples = 1200; /* 150 ms ascending two-tone (523 Hz -> 659 Hz) */
            for (int i = 0; i < samples; i++) {
                int freq = (i < 600) ? 523 : 659;
                int period = speed / freq;
                buf[i] = (i % period < period / 2) ? 175 : 80;
            }
            break;
        case SOUND_ALARM:
            samples = 1600; /* 200 ms urgent triple beep */
            for (int i = 0; i < samples; i++) {
                int active = (i < 400) || (i >= 600 && i < 1000) || (i >= 1200);
                int period = speed / 880;
                buf[i] = active ? ((i % period < period / 2) ? 185 : 70) : 128;
            }
            break;
    }

    if (samples > 0) {
        write(fd, buf, samples);
    }
    close(fd);
}
