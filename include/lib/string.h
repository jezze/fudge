#ifndef LIB_STRING_H
#define LIB_STRING_H

char *string_find(const char *in, const char *str);
unsigned int string_length(const char *in);
unsigned int string_read_num(const char *in, unsigned int base);
char *string_write_string(char *out, const char *in);
char *string_write_num(char *out, unsigned int num, unsigned int base);
char *string_write(char *out, const char *in, ...);

#endif

