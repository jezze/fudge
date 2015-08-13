#include <abi.h>
#include <fudge.h>
#include "lib.h"

static void writebyte(unsigned char c)
{

    unsigned char i = 0x80;

    do
    {

        if (c & i)
            call_write(CALL_PO, 0, 1, 1, "X");
        else
            call_write(CALL_PO, 0, 1, 1, " ");

    } while (i >>= 1);

}

static void writechar(char c, unsigned int padding, void *buffer)
{

    unsigned short index = pcf_getindex(buffer, c);
    unsigned int offset = pcf_getbitmapoffset(buffer, index);
    unsigned char *data = pcf_getbitmapdata(buffer);
    struct pcf_metricsdata metrics;
    unsigned int x;
    unsigned int y;

    pcf_getmetricsdata(buffer, index, &metrics);

    for (y = 0; y < metrics.ascent + metrics.descent; y++)
    {

        for (x = 0; x < padding; x++)
            writebyte(data[offset + y * padding + x]);

        call_write(CALL_PO, 0, 1, 1, "\n");

    }

}

static void writestring(unsigned int count, char *text, void *buffer)
{

    unsigned int padding = pcf_getpadding(buffer);
    unsigned int i;

    for (i = 0; i < count; i++)
        writechar(text[i], padding, buffer);

}

void main(void)
{

    unsigned char buffer[0x8000];

    call_open(CALL_P0);
    call_read(CALL_P0, 0, 1, 0x8000, buffer);
    call_close(CALL_P0);
    writestring(12, "Hello World!", buffer);

}

