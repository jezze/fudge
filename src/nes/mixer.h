#pragma once

#include "apu.h"

// enough for one frame at 48000 Hz and 50 frames per second
#define AUDIO_BUFF_SIZE 1024

struct Emulator;

typedef struct {
    int16_t buff[AUDIO_BUFF_SIZE];
    size_t index;
    // APU samples since the last output sample, averaged into it
    uint32_t sum;
    uint32_t count;
    // an output sample is due each time phase reaches period, phase grows by step every cpu cycle
    uint32_t phase;
    uint32_t step;
    uint32_t period;
    // high pass filter state
    int32_t last_in;
    int32_t last_out;
    APU* apu;
} Mixer;

void init_mixer(struct Emulator* emulator, uint32_t frequency);
void sample(Mixer* mixer);
