#pragma once
#include "port.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define LOG(...) ((void)0)

enum {
    BIT_7 = 1<<7,
    BIT_6 = 1<<6,
    BIT_5 = 1<<5,
    BIT_4 = 1<<4,
    BIT_3 = 1<<3,
    BIT_2 = 1<<2,
    BIT_1 = 1<<1,
    BIT_0 = 1
};

uint32_t next_power_of_2(uint32_t num);
