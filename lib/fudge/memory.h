void memory_clear(void *out, unsigned int count);
void memory_copy(void *out, void *in, unsigned int count);
unsigned int memory_findbyte(void *in, unsigned int count, char value);
unsigned int memory_match(void *in1, void *in2, unsigned int count);
unsigned int memory_read(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset);
unsigned int memory_write(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset);
unsigned int memory_pagecount(unsigned int value);
unsigned int memory_pagealign(unsigned int value);
