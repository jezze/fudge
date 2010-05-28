#ifndef STRING_H
#define STRING_H

extern char *string_concat(char *dest, const char *src);
extern int string_compare(char *str1, char *str2);
extern char *string_copy(char *dest, const char *src);
extern int string_length(const char *str);

#endif

