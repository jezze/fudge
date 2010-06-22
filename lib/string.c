#include <lib/types.h>
#include <lib/string.h>

char *string_concat(char *dest, const char *src)
{

    string_copy(dest + string_length(dest), src);

    return dest;

}

int string_compare(const char *str1, const char *str2)
{

    for (; *str1 == *str2; ++str1, ++str2)
    {

        if (*str1 == 0)
            return 0;

    }

    return *(unsigned char *)str1 < *(unsigned char *)str2 ? -1 : 1;

}

char *string_copy(char *dest, const char *src)
{

    char *save = dest;

    while ((*dest++ = *src++));

    return save;

}

int string_index(const char *str, char c)
{

    int length;

    for (length = 0; *str != c; str++, length++);

    return length;

}

int string_length(const char *str)
{

    return string_index(str, '\0');

}

