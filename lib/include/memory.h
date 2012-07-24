#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

void *memory_clear(void *out, unsigned int count);
void *memory_copy(void *out, const void *in, unsigned int count);
void *memory_find(const void *in1, const void *in2, unsigned int count1, unsigned int count2);
unsigned int memory_match(const void *in1, const void *in2, unsigned int count);

#endif

