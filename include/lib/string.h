#ifndef LIB_STRING_H
#define LIB_STRING_H

char *string_find(const char *in, const char *str);
unsigned int string_length(const char *in);
unsigned int string_read_num(const char *in, unsigned int base);
char *string_replace(char *out, char value1, char value2);
unsigned int string_split(char *out[], char *in, char value);
char *string_triml(char *in, char c);
char *string_trimr(char *in, char c);
char *string_trim(char *in, char c);
char *string_write(char *out, const char *in);
char *string_write_format(char *out, char *in, ...);

#endif

