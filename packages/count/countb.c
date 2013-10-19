#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count);

    call_write(CALL_O0, 0, ascii_write_value(buffer, FUDGE_BSIZE, roff + count, 10, 0), buffer);

}

