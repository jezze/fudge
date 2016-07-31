#include <abi.h>
#include <fudge.h>

static unsigned int convert(unsigned char *out, unsigned int ocount, unsigned char *in, unsigned int icount)
{

    unsigned int offset;

    for (offset = 0; offset < icount && offset < ocount; offset++)
        out[offset] = in[offset] * 63 / 255;

    return offset;

}

void main(void)
{

    unsigned char out[FUDGE_BSIZE];
    unsigned char in[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_PO);
    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, in, FUDGE_BSIZE)))
        file_writeall(CALL_PO, out, convert(out, FUDGE_BSIZE, in, count));

    file_close(CALL_PI);
    file_close(CALL_PO);

}

