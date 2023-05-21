#include <fudge.h>
#include "net.h"
#include "dns.h"

unsigned int dns_namesize(char *name)
{

    unsigned int i;

    for (i = 0; name[i]; i += name[i] + 1)
    {

        if ((name[i] & 0xC0) == 0xC0)
            return i + 2;

    }

    return i + 1;

}

unsigned int dns_copyname(void *buffer, unsigned int count, char *name)
{

    unsigned char namelen = 0;
    unsigned int valstart = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; i < cstring_length_zero(name); i++)
    {

        if (name[i] == '.' || name[i] == '\0')
        {

            c += buffer_write(buffer, count, &namelen, 1, c);
            c += buffer_write(buffer, count, name + valstart, namelen, c);

            valstart = i + 1;
            namelen = 0;

        }

        else
        {

            namelen++;

        }

    }

    c += cstring_write_zero(buffer, count, c);

    return c;

}

unsigned int dns_writename(void *buffer, unsigned int count, char *name, void *start)
{

    unsigned int offset = 0;
    unsigned int index = 0;

    while (name[index])
    {

        if ((name[index] & 0xC0) == 0xC0)
        {

            unsigned char *temp = start;

            name = (char *)(temp + (((name[index] & 0x2F) << 8) | name[index + 1]));
            index = 0;

        }

        else
        {

            offset += buffer_write(buffer, count, name + index + 1, name[index], offset);
            index += name[index] + 1;

            if (name[index])
                offset += cstring_write(buffer, count, ".", offset);

        }

    }

    return offset;

}

