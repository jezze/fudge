#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[0x4000];

    unsigned int count = file_read(3, 0, 0x4000, buffer);

    if (!count)
        return;

    file_write(FILE_STDOUT, 0, count, buffer);

}

