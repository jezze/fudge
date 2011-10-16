#include <fudge.h>

void main(int argc, char *argv[])
{

    file_write_format(FILE_STDOUT, "Timer: %d\n", 0);

    call_execute("/shell", 0, 0);

}

