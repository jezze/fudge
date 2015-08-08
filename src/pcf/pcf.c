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

static void writechar(char c, unsigned int padding, unsigned char *data)
{

    unsigned short index = pcf_getindex(CALL_P0, c);
    unsigned int offset = pcf_getbitmapoffset(CALL_P0, index);
    struct pcf_metricsdata_normal metrics;
    unsigned int x;
    unsigned int y;

    pcf_readmetrics(CALL_P0, index, &metrics);

    for (y = 0; y < metrics.ascent + metrics.descent; y++)
    {

        for (x = 0; x < padding; x++)
            writebyte(data[offset + y * padding + x]);

        call_write(CALL_PO, 0, 1, 1, "\n");

    }

}

static void writestring(unsigned int count, char *text, unsigned char *data)
{

    unsigned int padding = pcf_getpadding(CALL_P0);
    unsigned int i;

    for (i = 0; i < count; i++)
        writechar(text[i], padding, data);

}

void main(void)
{

    unsigned char data[0x8000];

    call_open(CALL_P0);
    pcf_readdata(CALL_P0, 0x8000, data);
    call_close(CALL_P0);
    writestring(12, "Hello World!", data);

}

