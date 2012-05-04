#include <fudge.h>

#define BUFFER_SIZE 0x1000

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];

    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = file_read(3, offset, BUFFER_SIZE, buffer)); offset += BUFFER_SIZE)
    {

        file_write(FILE_STDOUT, 0, count, buffer);

    }

}

