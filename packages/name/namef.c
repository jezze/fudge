#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = call_read(CALL_I0, 0, FUDGE_BSIZE, buffer);
    unsigned int offset = count;

    while (--offset && buffer[offset - 1] != '/');

    call_write(CALL_O0, 0, count - offset, buffer + offset);

}

