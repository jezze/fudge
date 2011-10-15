#include <fudge.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/io");

    file_close(fd);

    call_execute("/shell", 1, 0);

}

