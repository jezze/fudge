#include <fudge.h>

void main()
{

    char buffer[0x1000];
    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = call_read(FILE_STDIN, offset, 0x1000, buffer)); offset += 0x1000)
        call_write(FILE_STDOUT, offset, count, buffer);

}

