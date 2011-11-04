#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        call_exit();

    file_write_format(FILE_STDOUT, "%s\n", argv[1]);

    call_exit();
    
}

