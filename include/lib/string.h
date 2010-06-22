#ifndef STRING_H
#define STRING_H

extern char *string_concat(char *dest, const char *src);
extern int string_compare(const char *str1, const char *str2);
extern char *string_copy(char *dest, const char *src);
extern int string_length(const char *str);

#endif

