#ifndef STRING_H
#define STRING_H

extern char *string_concat(char *base, const char *str);
extern int string_compare(const char *str1, const char *str2);
extern char *string_copy(char *base, const char *str);
extern int string_index(const char *base, char c);
extern int string_length(const char *base);

#endif

