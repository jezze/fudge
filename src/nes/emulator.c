#include "emulator.h"
#include "mapper.h"
#include "utils.h"

static uint16_t TURBO_SKIP;

int init_emulator(Emulator* emulator, ROMData* data, uint32_t frequency){
    memset(emulator, 0, sizeof(Emulator));
    if(load_data(data, &emulator->mapper) < 0)
        return -1;
    emulator->type = emulator->mapper.type;
    emulator->mapper.emulator = emulator;
    if(emulator->type == PAL) {
        TURBO_SKIP = PAL_FRAME_RATE / PAL_TURBO_RATE;
    }else{
        TURBO_SKIP = NTSC_FRAME_RATE / NTSC_TURBO_RATE;
    }

    init_mixer(emulator, frequency);
    init_mem(emulator);
    init_ppu(emulator);
    init_cpu(emulator);
    init_APU(emulator);
    emulator->ppu.enabled = 1;
    return 0;
}

void tick_master_clock(Emulator* emulator) {
    emulator->cpu.t_cycles++;
    if (emulator->ppu.enabled) {
        execute_ppu(&emulator->ppu);
        execute_ppu(&emulator->ppu);
        execute_ppu(&emulator->ppu);
        if (emulator->type == PAL) {
            emulator->PAL_check++;
            if (emulator->PAL_check == 5) {
                // run extra PPU cycle on 5th cycle
                execute_ppu(&emulator->ppu);
                emulator->PAL_check = 0;
            }
        }
    }
    if (emulator->mem.strobe && emulator->apu.cycles & 1) {
        // controllers are strobed on a get cycle
        emulator->mem.joy1.reg = emulator->mem.joy1.status & 0xff;
        emulator->mem.joy2.reg = emulator->mem.joy2.status & 0xff;

    }
    execute_apu(&emulator->apu);
    sample(&emulator->mixer);
}

void run_frame(Emulator* emulator){
    PPU* ppu = &emulator->ppu;

    // trigger turbo events
    if(ppu->frames % TURBO_SKIP == 0) {
        turbo_trigger(&emulator->mem.joy1);
        turbo_trigger(&emulator->mem.joy2);
    }

    // if ppu.render is set a frame is complete
    while (!ppu->render) {
        execute(&emulator->cpu);
    }
    ppu->render = 0;
}

void reset_emulator(Emulator* emulator) {
    reset_cpu(&emulator->cpu);
    reset_APU(&emulator->apu);
    reset_ppu(&emulator->ppu);
    if(emulator->mapper.reset != NULL) {
        emulator->mapper.reset(&emulator->mapper);
    }
}

