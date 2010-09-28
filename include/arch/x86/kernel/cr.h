#ifndef ARCH_CR_H
#define ARCH_CR_H

extern unsigned int cr0_read();
extern void cr0_write(unsigned int value);
extern unsigned int cr2_read();
extern void cr2_write(unsigned int value);
extern unsigned int cr3_read();
extern void cr3_write(unsigned int value);
extern unsigned int cr4_read();
extern void cr4_write(unsigned int value);

#endif

