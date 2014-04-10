#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_I0);

    count = call_read(CALL_I0, 0, FUDGE_BSIZE, buffer);

    call_close(CALL_I0);

    while (--count && buffer[count - 1] != '/');

    call_open(CALL_O0);
    call_write(CALL_O0, 0, count, buffer);
    call_close(CALL_O0);

}

