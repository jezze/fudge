#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    file_write_format(FILE_STDOUT, 0, "%s\n", argv[1]);

}

