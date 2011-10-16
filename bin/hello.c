#include <fudge.h>

void main(int argc, char *argv[])
{

    file_write_format(FILE_STDOUT, "Hello world!\n");

    call_execute("/shell", 0, 0);

}

