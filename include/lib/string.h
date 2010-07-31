#ifndef STRING_H
#define STRING_H

extern int string_compare(const char *str1, const char *str2);
extern char *string_concat(char *dest, const char *src);
extern char *string_copy(char *dest, const char *src);
extern int string_offset(const char *dest, char c);
extern int string_length(const char *dest);
extern char *string_replace(char *dest, char c1, char c2);
extern int string_split(char *dest[], char *src, char c);

#endif

