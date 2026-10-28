#include "utils.h"

uint32_t next_power_of_2(uint32_t num) {
    uint32_t power = 1;
    while(power < num)
        power*=2;
    return power;
}

