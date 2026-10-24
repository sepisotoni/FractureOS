#ifndef FRACTURE_AUDIO_H
#define FRACTURE_AUDIO_H

enum FractureSound {
    SOUND_CLICK = 0,
    SOUND_CHIME,
    SOUND_ALARM,
    SOUND_SWOOSH
};

int  audio_init(void);
void audio_close(void);
void audio_set_muted(int muted);
int  audio_is_muted(void);
void audio_play(int sound_id);

#endif /* FRACTURE_AUDIO_H */
