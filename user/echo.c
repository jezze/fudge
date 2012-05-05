#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    call_write(FILE_STDOUT, 0, string_length(argv[1]), argv[1]);
    call_write(FILE_STDOUT, 0, 1, "\n");

}

