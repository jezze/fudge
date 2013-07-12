#include <fudge.h>

static unsigned int convert(void *out, unsigned int ocount, void *in, unsigned int icount)
{

    char *o = out;
    char *i = in;
    unsigned int offset;

    for (offset = 0; offset < icount && offset < ocount; offset++)
        o[offset] = i[offset] * 63 / 255;

    return offset;

}

void main()
{

    char out[FUDGE_BSIZE];
    char in[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, in)); offset += count)
    {

        count = convert(out, FUDGE_BSIZE, in, count);

        call_write(CALL_DO, offset, count, out);

    }

}

