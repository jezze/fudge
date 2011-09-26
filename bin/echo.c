#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    file_write_string(FILE_STDOUT, argv[1]);

}

