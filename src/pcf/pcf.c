#include <abi.h>
#include <fudge.h>
#include "lib.h"

static void writenum(unsigned int value, unsigned int base)
{

    char num[32];

    call_write(CALL_PO, 0, ascii_wvalue(num, 32, value, base, 0), 1, num);
    call_write(CALL_PO, 0, 1, 1, "\n");

}

static void writedata(unsigned int size, unsigned int count, unsigned char *buffer)
{

    char num[32];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        call_write(CALL_PO, 0, ascii_wzerovalue(num, 32, buffer[i], 16, 2, 0), 1, num);

        if (!((i + 1) % size))
            call_write(CALL_PO, 0, 2, 1, "  ");

    }

    call_write(CALL_PO, 0, 1, 1, "\n");

}

static void writestring(unsigned int count, char *text)
{

    unsigned int i;
    unsigned int padding = pcf_getpadding();

    for (i = 0; i < count; i++)
    {

        unsigned short index = pcf_getindex(text[i]);
        unsigned int ascent = pcf_getascent(index);
        unsigned int descent = pcf_getdescent(index);
        void *data = pcf_getdata(index);

        writenum(ascent, 10);
        writenum(descent, 10);
        writenum(padding, 10);
        writedata(padding, (ascent + descent) * padding, data);

    }

}

void main()
{

    pcf_load(CALL_P0);
    writestring(3, "A Hello World!");

}

