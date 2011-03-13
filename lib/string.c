#include <lib/memory.h>
#include <lib/string.h>

int string_compare(const char *in1, const char *in2)
{

    return memory_compare(in1, in2, string_length(in2));

}

char *string_concat(char *out, const char *in)
{

    memory_copy(out + string_length(out), in, string_length(in) + 1);

    return out;

}

char *string_copy(char *out, const char *in)
{

    memory_copy(out, in, string_length(in) + 1);

    return out;

}

unsigned int string_index(const char *in, char value, unsigned int skip)
{

    return memory_index(in, value, string_length(in), skip);

}

unsigned int string_index_reversed(const char *in, char value, unsigned int skip)
{

    return memory_index_reversed(in, value, string_length(in), skip);

}

unsigned int string_length(const char *in)
{

    const char *ip = in;

    for (; *ip != '\0'; ip++);

    return ip - in;

}

char *string_replace(char *out, char value1, char value2)
{

    memory_replace(out, value1, value2, string_length(out));

    return out;

}

unsigned int string_split(char *out[], char *in, char value)
{

    if (in[0] == '\0')
        return 0;

    unsigned int count = 1;
    out[0] = in;

    unsigned int i;

    for (i = 1; in[i] != '\0'; i++)
    {

        if (in[i - 1] == value)
        {

            in[i - 1] = '\0';
            out[count] = in + i;
            count++;

        }

    }

    return count;

}

char *string_triml(char *in, char c)
{

    while (*in == c)
        in++;

    return in;

}

char *string_trimr(char *in, char c)
{

    char *end = in + string_length(in) - 1;

    while (*end == c)
        *end-- = '\0';

    return in;

}

char *string_trim(char *in, char c)
{

    return string_triml(string_trimr(in, c), c);

}

void string_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
    {

        out[0] = '0';
        out[1] = '\0';

        return;

    }

    unsigned int i;

    for (i = 0; num; i++)
    {

        out[i] = "0123456789ABCDEF"[num % base];
        num /= base;

    }

    out[i] = '\0';
    unsigned int size = i;

    char temp;

    for (i = 0; i < (size / 2); i++)
    {

        temp = out[size - i - 1];
        out[size - i - 1] = out[i];
        out[i] = temp;

    }

    return;

}

