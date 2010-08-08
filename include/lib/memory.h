#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

extern int memory_compare(const void *dest, const void *src, unsigned int count);
extern void *memory_copy(void *dest, const void *src, unsigned int count);
extern unsigned int memory_index(const void *dest, char value, unsigned int count);
extern void *memory_replace(void *dest, char value1, char value2, unsigned int count);
extern void *memory_set(void *dest, char value, unsigned int count);
extern short *memory_setw(void *dest, short value, unsigned int count);

#endif

