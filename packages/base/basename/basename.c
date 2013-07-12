#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);
    unsigned int offset = count - 1;

    while (--offset && buffer[offset - 1] != '/');

    count = call_write(CALL_DO, 0, count - offset, buffer + offset);
    call_write(CALL_DO, count, 1, "\n");

}

