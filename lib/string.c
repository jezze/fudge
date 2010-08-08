#include <lib/memory.h>
#include <lib/string.h>

int string_compare(const char *dest, const char *src)
{

    return memory_compare(dest, src, string_length(src));

}

char *string_concat(char *dest, const char *src)
{

    memory_copy(dest + string_length(dest), src, string_length(src));

    return dest;

}

char *string_copy(char *dest, const char *src)
{

    memory_copy(dest, src, string_length(src));

    return dest;

}

unsigned int string_index(const char *dest, char value)
{

    return memory_index(dest, value, string_length(dest));

}

unsigned int string_length(const char *dest)
{

    const char *dp = dest;

    for (; *dp != '\0'; dp++);

    return dp - dest;

}

char *string_replace(char *dest, char value1, char value2)
{

    memory_replace(dest, value1, value2, string_length(dest));

    return dest;

}

unsigned int string_split(char *dest[], char *src, char value)
{

    if (src[0] == '\0')
        return 0;

    unsigned int count = 1;
    dest[0] = src;

    unsigned int i;

    for (i = 1; src[i] != '\0'; i++)
    {

        if (src[i - 1] == value)
        {

            src[i - 1] = '\0';
            dest[count] = src + i;
            count++;

        }

    }

    return count;

}

