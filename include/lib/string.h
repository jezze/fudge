#ifndef LIB_STRING_H
#define LIB_STRING_H

extern int string_compare(const char *out, const char *in);
extern char *string_concat(char *out, const char *in);
extern char *string_copy(char *out, const char *in);
extern unsigned int string_index(const char *in, char value);
extern unsigned int string_length(const char *in);
extern char *string_replace(char *out, char value1, char value2);
extern unsigned int string_split(char *out[], char *in, char value);

#endif

