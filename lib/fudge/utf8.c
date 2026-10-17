#include "utf8.h"

static unsigned int iscontinuation(char c)
{

    return (c & 0xC0) == 0x80;

}

unsigned int utf8_read(char *text, unsigned int length, unsigned int *codepoint)
{

    unsigned char c = text[0];
    unsigned int count;
    unsigned int minimum;
    unsigned int value;
    unsigned int i;

    if (c < 0x80)
    {

        *codepoint = c;

        return 1;

    }

    if (c >= 0xC2 && c < 0xE0)
    {

        count = 2;
        minimum = 0x80;
        value = c & 0x1F;

    }

    else if (c >= 0xE0 && c < 0xF0)
    {

        count = 3;
        minimum = 0x800;
        value = c & 0x0F;

    }

    else if (c >= 0xF0 && c < 0xF5)
    {

        count = 4;
        minimum = 0x10000;
        value = c & 0x07;

    }

    else
    {

        *codepoint = 0xFFFD;

        return 1;

    }

    if (count > length)
    {

        *codepoint = 0xFFFD;

        return 1;

    }

    for (i = 1; i < count; i++)
    {

        if (!iscontinuation(text[i]))
        {

            *codepoint = 0xFFFD;

            return 1;

        }

        value = (value << 6) | (text[i] & 0x3F);

    }

    if (value < minimum || value > 0x10FFFF || (value >= 0xD800 && value < 0xE000))
    {

        *codepoint = 0xFFFD;

        return 1;

    }

    *codepoint = value;

    return count;

}

unsigned int utf8_prev(char *text, unsigned int offset)
{

    unsigned int start = offset - 1;
    unsigned int codepoint;

    if (!offset)
        return 0;

    while (start > 0 && offset - start < 4 && iscontinuation(text[start]))
        start--;

    return (start + utf8_read(text + start, offset - start, &codepoint) == offset) ? start : offset - 1;

}

unsigned int utf8_fit(char *text, unsigned int length, unsigned int limit)
{

    unsigned int start = limit;
    unsigned int codepoint;

    if (limit >= length)
        return length;

    while (start > 0 && limit - start < 3 && iscontinuation(text[start]))
        start--;

    return (start + utf8_read(text + start, length - start, &codepoint) > limit) ? start : limit;

}

unsigned int utf8_write(char *text, unsigned int count, unsigned int codepoint)
{

    if (codepoint && codepoint < 0x80 && count >= 1)
    {

        text[0] = codepoint;

        return 1;

    }

    if (codepoint >= 0x80 && codepoint < 0x800 && count >= 2)
    {

        text[0] = 0xC0 | (codepoint >> 6);
        text[1] = 0x80 | (codepoint & 0x3F);

        return 2;

    }

    if (codepoint >= 0x800 && codepoint < 0x10000 && (codepoint < 0xD800 || codepoint > 0xDFFF) && count >= 3)
    {

        text[0] = 0xE0 | (codepoint >> 12);
        text[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        text[2] = 0x80 | (codepoint & 0x3F);

        return 3;

    }

    if (codepoint >= 0x10000 && codepoint < 0x110000 && count >= 4)
    {

        text[0] = 0xF0 | (codepoint >> 18);
        text[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        text[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        text[3] = 0x80 | (codepoint & 0x3F);

        return 4;

    }

    return 0;

}
