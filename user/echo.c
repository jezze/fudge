#include <fudge.h>

#define BUFFER_SIZE 0x1000

void main()
{

    char buffer[BUFFER_SIZE];
    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = call_read(FILE_STDIN, offset, BUFFER_SIZE, buffer)); offset += BUFFER_SIZE)
        call_write(FILE_STDOUT, offset, count, buffer);

}

