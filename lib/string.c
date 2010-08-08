#include <lib/memory.h>
#include <lib/string.h>

int string_compare(const char *str1, const char *str2)
{

    return memory_compare((void *)str1, (void *)str2, string_length(str1));

}

char *string_concat(char *dest, const char *src)
{

    memory_copy(dest + string_length(dest), src, string_length(src));

    return dest;

}

char *string_copy(char *dest, const char *src)
{

    char *dp = dest;
    const char *sp = src;

    while ((*dp++ = *sp++) != '\0');

    return dest;

}

int string_offset(const char *dest, char c)
{

    int count;

    for (count = 0; *dest != '\0' && *dest != c; dest++, count++);

    return (*dest == c) ? count : -1;

}

int string_length(const char *dest)
{

    return string_offset(dest, '\0');

}

char *string_replace(char *dest, char c1, char c2)
{

    char *dp;

    for (dp = dest; *dp != '\0'; dp++)
    {

        if (*dp == c1)
            *dp = c2;

    }

    return dest;

}

int string_split(char *dest[], char *src, char c)
{

    if (src[0] == '\0')
        return 0;

    unsigned int count = 1;
    dest[0] = src;

    unsigned int i;

    for (i = 1; src[i] != '\0'; i++)
    {

        if (src[i - 1] == c)
        {

            src[i - 1] = '\0';
            dest[count] = src + i;
            count++;

        }

    }

    return count;

}

