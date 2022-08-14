#include "base64.h"

static unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned int base64_encode(unsigned char *src, unsigned int len, unsigned char *out)
{

    unsigned char *in = src;
    unsigned char *end = src + len;
    unsigned char *pos = out;
    int line_len = 0;

    while (end - in >= 3)
    {

        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
        line_len += 4;

        if (line_len >= 72)
        {

            *pos++ = '\n';
            line_len = 0;

        }

    }

    if (end - in)
    {

        *pos++ = base64_table[in[0] >> 2];

        if (end - in == 1)
        {

            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';

        }

        else
        {

            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];

        }

        *pos++ = '=';
        line_len += 4;

    }

    if (line_len)
        *pos++ = '\n';

    *pos = '\0';

    return pos - out;

}

unsigned int base64_decode(unsigned char *src, unsigned int len, unsigned char *out)
{

    unsigned char dtable[256];
    unsigned char *pos = 0;
    unsigned char block[4];
    unsigned int count;
    unsigned int i;
    int pad = 0;

    for (i = 0; i < 256; i++)
        dtable[i] = 0x80;

    for (i = 0; i < 64; i++)
        dtable[base64_table[i]] = (unsigned char)i;

    dtable['='] = 0;
    count = 0;

    for (i = 0; i < len; i++)
    {

        if (dtable[src[i]] != 0x80)
            count++;

    }

    if (count == 0 || count % 4)
        return 0;

    count = 0;

    for (i = 0; i < len; i++)
    {

        unsigned char tmp = dtable[src[i]];

        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;

        block[count] = tmp;
        count++;

        if (count == 4)
        {

            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;

            if (pad)
            {

                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else
                    return 0;

                break;

            }

        }

    }

    return pos - out;

}

