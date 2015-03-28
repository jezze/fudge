void memory_clear(void *out, unsigned int count);
void memory_copy(void *out, const void *in, unsigned int count);
unsigned int memory_findbyte(const void *in, unsigned int count, char value);
unsigned int memory_match(const void *in1, const void *in2, unsigned int count);
unsigned int memory_read(void *out, unsigned int ocount, const void *in, unsigned int icount, unsigned int size, unsigned int offset);
unsigned int memory_write(void *out, unsigned int ocount, const void *in, unsigned int icount, unsigned int size, unsigned int offset);
