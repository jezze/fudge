#include <lib/memory.h>
#include <lib/string.h>

int string_compare(const char *in1, const char *in2)
{

    unsigned int count1 = string_length(in1);
    unsigned int count2 = string_length(in2);

    unsigned int count = (count1 > count2) ? count2 : count1;

    return memory_compare(in1, in2, count);

}

unsigned int string_index(const char *in, char value, unsigned int skip)
{

    return memory_index(in, value, string_length(in), skip);

}

unsigned int string_index_reversed(const char *in, char value, unsigned int skip)
{

    return memory_index_reversed(in, value, string_length(in), skip);

}

char *string_find(const char *in, const char *str)
{

    char c = *str++;

    if (!c)
        return (char *)in;

    unsigned int len = string_length(str);

    do
    {

        char sc;

        do
        {

            sc = *in++;

            if (!sc)
                return (char *)0;

        }
        while (sc != c);

    }
    while (memory_compare(in, str, len));

    return (char *)(in - 1);

}

unsigned int string_length(const char *in)
{

    const char *ip = in;

    for (; *ip != '\0'; ip++);

    return ip - in;

}

unsigned int string_read_num(const char *in, unsigned int base)
{

    const char *ip = in;
    int num = 0;

    while ((*ip >= '0' && *ip <= '9') || (*ip >= 'a' && *ip <= 'f'))
    {

        if ((*ip >= '0' && *ip <= '9'))
            num = num * base + *ip++ - '0';

        if ((*ip >= 'a' && *ip <= 'f'))
            num = num * base + *ip++ - 'a' + 10;

    }

    return num;

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

char *string_write(char *out, const char *in)
{

    return memory_copy(out, in, string_length(in) + 1);

}

static char *string_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
        return string_write(out, "0");

    char buffer[32];
    memory_clear(buffer, 32);

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return string_write(out, buffer + i + 1);

}

char *string_write_format(char *out, char *in, ...)
{

    char *ret = out;
    char **arg = (char **)&in;
    arg++;

    char num[32];
    char c;

    while ((c = *in++))
    {

        if (c != '%')
        {

            memory_copy(out, &c, 1);
            out += 1;

            continue;

        }

        c = *in++;

        switch (c)
        {

            case 'c':

                memory_copy(out, (char *)arg, 1);
                out += 1;

                break;

            case 'd':

                string_write_num(num, *(int *)arg, 10);
                string_write(out, num);
                out += string_length(num);

                break;

            case 's':

                string_write(out, *(char **)arg);
                out += string_length(*(char **)arg);

                break;

            case 'x':

                string_write_num(num, *(int *)arg, 16);
                string_write(out, num);
                out += string_length(num);

                break;

        }

        arg++;

    }

    *(out) = '\0';

    return ret;

}

