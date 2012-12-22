#include "reg.h"

unsigned int reg_read8(unsigned int address)
{

    volatile unsigned char *mem = (volatile unsigned char *)address;

    return *mem;

}

unsigned int reg_read32(unsigned int address)
{

    volatile unsigned int *mem = (volatile unsigned int *)address;

    return *mem;

}

void reg_write8(unsigned int address, unsigned char value)
{

    volatile unsigned char *mem = (volatile unsigned char *)address;

    *mem = value;

}

void reg_write32(unsigned int address, unsigned int value)
{

    volatile unsigned int *mem = (volatile unsigned int *)address;

    *mem = value;

}

