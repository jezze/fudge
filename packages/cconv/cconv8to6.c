#include <fudge.h>

static unsigned int convert(unsigned char *out, unsigned int ocount, unsigned char *in, unsigned int icount)
{

    unsigned int offset;

    for (offset = 0; offset < icount && offset < ocount; offset++)
        out[offset] = in[offset] * 63 / 255;

    return offset;

}

void main()
{

    unsigned char out[FUDGE_BSIZE];
    unsigned char in[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, in)); offset += count)
        call_write(CALL_DO, offset, convert(out, FUDGE_BSIZE, in, count), out);

}

