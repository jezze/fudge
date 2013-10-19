unsigned int ascii_toint(unsigned char c);
unsigned char ascii_tochar(unsigned int value, unsigned int base);
unsigned int ascii_read_value(const char *in, unsigned int count, unsigned int base);
unsigned int ascii_write_value(void *out, unsigned int count, unsigned int num, unsigned int base, unsigned int offset);
unsigned int ascii_write_zerovalue(void *out, unsigned int count, unsigned int num, unsigned int base, unsigned int padding, unsigned int offset);
