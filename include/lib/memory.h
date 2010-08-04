#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

extern int memory_compare(void *dest, void *src, unsigned int count);
extern void *memory_copy(void *dest, const void *src, unsigned int count);
extern void *memory_set(void *dest, int value, unsigned int count);
extern unsigned short *memory_setw(unsigned short *dest, unsigned short value, unsigned int count);

#endif

