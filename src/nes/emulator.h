#pragma once

#include "cpu6502.h"
#include "ppu.h"
#include "mmu.h"
#include "apu.h"
#include "mapper.h"
#include "mixer.h"


// frame rate in Hz
#define NTSC_FRAME_RATE 60
#define PAL_FRAME_RATE 50

// turbo keys toggle rate (Hz)
// value should be a factor of FRAME_RATE
// and should never exceed FRAME_RATE for best result
#define NTSC_TURBO_RATE 30
#define PAL_TURBO_RATE 25


typedef struct Emulator{
    c6502 cpu;
    PPU ppu;
    APU apu;
    Memory mem;
    Mapper mapper;
    Mixer mixer;

    TVSystem type;

    uint8_t PAL_check;
} Emulator;


int init_emulator(Emulator* emulator, ROMData* data, uint32_t frequency);
void reset_emulator(Emulator* emulator);
void tick_master_clock(Emulator* emulator);
void run_frame(Emulator* emulator);
