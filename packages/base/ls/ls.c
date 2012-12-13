#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

/*
    count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);
*/

    for (offset = 0; (count = call_read(FUDGE_CWD, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(FUDGE_OUT, offset, count, buffer);

}

