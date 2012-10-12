#ifndef ARCH_REG_H
#define ARCH_REG_H

unsigned int reg_read8(unsigned int address);
unsigned int reg_read32(unsigned int address);
void reg_write8(unsigned int address, unsigned char value);
void reg_write32(unsigned int address, unsigned int value);

#endif

