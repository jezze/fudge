#include <arch.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);
    unsigned int offset = count;

    while (--offset && buffer[offset - 1] != '/');

    call_write(CALL_DO, 0, count - offset, buffer + offset);

}

