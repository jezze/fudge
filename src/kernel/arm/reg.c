#include "reg.h"

unsigned int reg_read8(unsigned int address)
{

    unsigned char *mem = (unsigned char *)address;

    return *mem;

}

unsigned int reg_read32(unsigned int address)
{

    unsigned int *mem = (unsigned int *)address;

    return *mem;

}

void reg_write8(unsigned int address, unsigned char value)
{

    unsigned char *mem = (unsigned char *)address;

    *mem = value;

}

void reg_write32(unsigned int address, unsigned int value)
{

    unsigned int *mem = (unsigned int *)address;

    *mem = value;

}

