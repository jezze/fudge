unsigned int string_length(const char *in);
unsigned int string_read_num(const char *in, unsigned int base);
unsigned int string_write_num(void *out, unsigned int count, unsigned int num, unsigned int base);
unsigned int string_write_format(void *out, unsigned int count, const char *format, const void **args);
