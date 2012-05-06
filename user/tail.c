#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[128];
    unsigned int count = 0;

    while (count < 8)
    {

        unsigned int length = call_read(FILE_STDIN, 0, 8, buffer + count);

        call_write(FILE_STDOUT, 0, length, buffer + count);

        count += length;

    }

}

