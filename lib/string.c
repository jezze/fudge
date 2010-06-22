#include <lib/types.h>
#include <lib/string.h>

char *string_concat(char *base, const char *str)
{

    string_copy(base + string_length(base), str);

    return base;

}

int string_compare(const char *str1, const char *str2)
{

    for (; *str1 == *str2; ++str1, ++str2)
    {

        if (*str1 == 0)
            return 0;

    }

    return *str1 - *str2;

}

char *string_copy(char *base, const char *str)
{

    char *current = base;

    while ((*current++ = *str++));

    return base;

}

int string_offset(const char *base, char c)
{

    int offset;

    for (offset = 0; *base != c; base++, offset++);

    return offset;

}

int string_length(const char *base)
{

    return string_offset(base, 0);

}

char *string_replace(char *base, char c1, char c2)
{

    char *current = base;

    for (; *current != 0; current++)
    {

        if (*current == c1)
            *current = c2;

    }

    return base;

}

