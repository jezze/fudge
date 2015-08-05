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

static void writestring(unsigned int count, char *text, unsigned char *data)
{

    unsigned int i;
    unsigned int padding = pcf_getpadding(CALL_P0);

    for (i = 0; i < count; i++)
    {

        unsigned short index = pcf_getindex(CALL_P0, text[i]);
        unsigned int ascent = pcf_getascent(CALL_P0, index);
        unsigned int descent = pcf_getdescent(CALL_P0, index);
        unsigned int offset = pcf_getbitmapoffset(CALL_P0, index);

        writenum(ascent, 10);
        writenum(descent, 10);
        writenum(padding, 10);
        writedata(padding, (ascent + descent) * padding, data + offset);

    }

}

void main()
{

    unsigned char data[0x8000];

    call_open(CALL_P0);
    pcf_readdata(CALL_P0, 0x8000, data);
    call_close(CALL_P0);
    writestring(3, "A Hello World!", data);

}

