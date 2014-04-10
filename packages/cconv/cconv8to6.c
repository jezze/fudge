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
    unsigned int count, roff, woff = 0;

    call_open(CALL_I0);
    call_open(CALL_O0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, in)); roff += count)
        woff += call_write(CALL_O0, woff, convert(out, FUDGE_BSIZE, in, count), out);

    call_close(CALL_O0);
    call_close(CALL_I0);

}

