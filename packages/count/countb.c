#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned char num[32];

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count);

    call_write(CALL_DO, 0, memory_write_number(num, 32, offset + count, 10, 0), num);

}

