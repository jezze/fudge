#include <fudge.h>

#define BUFFER_SIZE 128

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    while (count < BUFFER_SIZE)
    {

        unsigned int length = call_read(FILE_STDIN, 0, BUFFER_SIZE - count, buffer + count);

        if (memory_find(buffer + count, "\n", length, 1))
            break;

        call_write(FILE_STDOUT, 0, length, buffer + count);

        count += length;

    }

    call_write(FILE_STDOUT, 0, 1, "\n");

}

