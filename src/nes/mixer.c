#include "mixer.h"
#include "emulator.h"

// the high pass filter's pole, 1 - 2 * pi * 20 Hz / 48000 Hz in 1/32768
#define HPF_POLE 32682

void init_mixer(Emulator* emulator, uint32_t frequency) {
    Mixer* mixer = &emulator->mixer;
    memset(mixer, 0, sizeof(Mixer));
    mixer->apu = &emulator->apu;
    // twice the cpu cycles per frame (29780.5 or 33247.5) times the frame rate is twice the cpu clock
    mixer->period = emulator->type == PAL ? 66495 * PAL_FRAME_RATE : 59561 * NTSC_FRAME_RATE;
    mixer->step = frequency * 2;
}

void sample(Mixer* mixer) {
    mixer->sum += get_sample(mixer->apu);
    mixer->count++;
    mixer->phase += mixer->step;
    if(mixer->phase < mixer->period)
        return;
    mixer->phase -= mixer->period;

    // the average of the samples since the last one keeps what the output rate can't hold from aliasing
    int32_t in = mixer->sum / mixer->count;
    int32_t out = in - mixer->last_in + (int32_t)(((int64_t)mixer->last_out * HPF_POLE) >> 15);
    mixer->last_in = in;
    mixer->last_out = out;
    mixer->sum = 0;
    mixer->count = 0;

    if(mixer->index < AUDIO_BUFF_SIZE)
        mixer->buff[mixer->index++] = out > 32767 ? 32767 : out < -32768 ? -32768 : out;
}

