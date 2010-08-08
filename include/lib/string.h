#ifndef LIB_STRING_H
#define LIB_STRING_H

extern int string_compare(const char *dest, const char *src);
extern char *string_concat(char *dest, const char *src);
extern char *string_copy(char *dest, const char *src);
extern unsigned int string_index(const char *dest, char value);
extern unsigned int string_length(const char *dest);
extern char *string_replace(char *dest, char value1, char value2);
extern unsigned int string_split(char *dest[], char *src, char value);

#endif

