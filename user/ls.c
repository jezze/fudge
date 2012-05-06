#include <fudge.h>

#define BUFFER_SIZE 0x1000

void main(int argc, char *argv[])
{

    char buffer[0x1000];

    unsigned int id = call_open(FILE_NEW, "/module/tty/pwd");
    unsigned int count = call_read(id, 0, BUFFER_SIZE, buffer);
    call_close(id);
    call_write(FILE_STDOUT, 0, count, buffer);

}

