#define BUFFER_SIZE                     4096

void buffer_clear(void *out, unsigned int count);
void buffer_copy(void *out, void *in, unsigned int count);
unsigned int buffer_findbyte(void *in, unsigned int count, unsigned char value);
unsigned int buffer_findlastbyte(void *in, unsigned int count, unsigned char value);
unsigned int buffer_match(void *in1, void *in2, unsigned int count);
unsigned int buffer_read(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset);
unsigned int buffer_write(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset);
