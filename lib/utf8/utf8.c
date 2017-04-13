#include "utf8.h"

unsigned int utf8_decode(char *s, int *u)
{

    unsigned char c;
    int i;
    int n;
    int r = 1;

    c = *s;

    if (~c & 0x80)
    {

        *u = c;

        return r;

    }

    else if ((c & 0xE0) == 0xC0)
    {

        *u = c & 0x1F;
        n = 1;

    }

    else if ((c & 0xF0) == 0xE0)
    {

        *u = c & 0x0F;
        n = 2;

    }

    else if ((c & 0xF8) == 0xF0)
    {

        *u = c & 0x07;
        n = 3;

    }

    else
    {

        goto invalid;

    }

    for (i = n, ++s; i > 0; --i, ++r, ++s)
    {

        c = *s;

        if ((c & 0xC0) != 0x80)
            goto invalid;

        *u <<= 6;
        *u |= c & 0x3F;

    }

    if ((n == 1 && *u < 0x80) || (n == 2 && *u < 0x800) || (n == 3 && *u < 0x10000) || (*u >= 0xD800 && *u <= 0xDFFF))
        goto invalid;

    return r;

invalid:
    *u = 0xFFFD;

    return r;

}

unsigned int utf8_encode(int *u, char *s)
{

    unsigned char *sp = (unsigned char *)s;
    unsigned int uc = *u;
    int i;
    int n;

    if (uc < 0x80)
    {

        *sp = uc;

        return 1;

    }

    else if (*u < 0x800)
    {

        *sp = (uc >> 6) | 0xC0;
        n = 1;

    }

    else if (uc < 0x10000)
    {

        *sp = (uc >> 12) | 0xE0;
        n = 2;

    }

    else if (uc <= 0x10FFFF)
    {

        *sp = (uc >> 18) | 0xF0;
        n = 3;

    }

    else
    {

        goto invalid;

    }

    for (i = n, ++sp; i > 0; --i, ++sp)
        *sp = ((uc >> 6 * (i - 1)) & 0x3F) | 0x80;

    return n + 1;

invalid:
    *s++ = '\xEF';
    *s++ = '\xBF';
    *s = '\xBD';

    return 3;

}

unsigned int utf8_isfull(char *s, int b)
{

    unsigned char *c1 = (unsigned char *)s;
    unsigned char *c2 = (unsigned char *)++s;
    unsigned char *c3 = (unsigned char *)++s;

    if (b < 1)
        return 0;
    else if ((*c1 & 0xE0) == 0xC0 && b == 1)
        return 0;
    else if ((*c1 & 0xF0) == 0xE0 && ((b == 1) || ((b == 2) && (*c2 & 0xC0) == 0x80)))
        return 0;
    else if ((*c1 & 0xF8) == 0xF0 && ((b == 1) || ((b == 2) && (*c2 & 0xC0) == 0x80) || ((b == 3) && (*c2 & 0xC0) == 0x80 && (*c3 & 0xC0) == 0x80)))
        return 0;
    else
        return 1;

}

unsigned int utf8_size(char *s)
{

    unsigned char c = *s;

    if (~c & 0x80)
        return 1;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else
        return 4;

}

