#ifndef CR_H
#define CR_H

extern unsigned int cr0_read();
extern unsigned int cr2_read();
extern unsigned int cr3_read();
extern unsigned int cr4_read();
extern void cr0_write(unsigned int value);
extern void cr2_write(unsigned int value);
extern void cr3_write(unsigned int value);
extern void cr4_write(unsigned int value);

#endif

