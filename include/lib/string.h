#ifndef LIB_STRING_H
#define LIB_STRING_H

extern int string_compare(const char *in1, const char *in2);
extern unsigned int string_index(const char *in, char value, unsigned int skip);
extern unsigned int string_index_reversed(const char *in, char value, unsigned int skip);
extern char *string_find(const char *in, const char *str);
extern unsigned int string_length(const char *in);
extern unsigned int string_read_num(const char *in, unsigned int base);
extern char *string_replace(char *out, char value1, char value2);
extern unsigned int string_split(char *out[], char *in, char value);
extern char *string_triml(char *in, char c);
extern char *string_trimr(char *in, char c);
extern char *string_trim(char *in, char c);
extern char *string_write(char *out, const char *in);
extern char *string_write_format(char *out, char *in, ...);

#endif

