unsigned int ascii_toint(unsigned char c);
unsigned int ascii_fromint(char *out, unsigned int count, unsigned int value, unsigned int base);
unsigned int ascii_read_value(const char *in, unsigned int count, unsigned int base);
unsigned int ascii_write_value(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int offset);
unsigned int ascii_write_zerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset);
