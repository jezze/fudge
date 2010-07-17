#ifndef MEMORY_H
#define MEMORY_H

extern void *memory_copy(void *dest, const void *src, unsigned int count);
extern void *memory_set(void *dest, char val, unsigned int count);
extern unsigned short *memory_setw(unsigned short *dest, unsigned short val, unsigned int count);

#endif

