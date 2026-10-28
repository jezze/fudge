#include "controller.h"


void init_joypad(struct JoyPad* joyPad, uint8_t player){
    joyPad->status = 0;
    joyPad->reg = 0;
    joyPad->player = player;
}

uint8_t read_joypad(struct JoyPad* joyPad){
    uint8_t val = joyPad->reg & 1;
    joyPad->reg >>= 1;
    // refill BIT 7 with 1
    joyPad->reg |= 0x80;
    return val;
}


void update_joypad(struct JoyPad* joyPad, uint16_t key, uint8_t pressed){
    if(!pressed) {
        joyPad->status &= ~key;
        if(key == TURBO_A) {
            // clear button A
            joyPad->status &= ~BUTTON_A;
        }
        if(key == TURBO_B) {
            // clear button B
            joyPad->status &= ~BUTTON_B;
        }
    } else {
        joyPad->status |= key;
        if(key == TURBO_A) {
            // set button A
            joyPad->status |= BUTTON_A;
        }
        if(key == TURBO_B) {
            // set button B
            joyPad->status |= BUTTON_B;
        }
    }
}

void turbo_trigger(struct JoyPad* joyPad){
    // toggle BUTTON_A AND BUTTON_B if TURBO_A and TURBO_B are set respectively
    joyPad->status ^= joyPad->status >> 8;
}

