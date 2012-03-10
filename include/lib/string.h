#ifndef LIB_STRING_H
#define LIB_STRING_H

char *string_find(const char *in, const char *str);
unsigned int string_length(const char *in);
unsigned int string_read_num(const char *in, unsigned int base);
unsigned int string_split(char *out[], char *in, char value);
char *string_triml(char *in, char c);
char *string_trimr(char *in, char c);
char *string_trim(char *in, char c);
char *string_write_format(char *out, char *in, ...);

#endif

