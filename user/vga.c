#include <fudge.h>

void main(int argc, char *argv[])
{

    int fd = file_open("dev", "/io");

    file_close(fd);

    call_exit();

}

