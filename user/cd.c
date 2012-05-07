#include <fudge.h>

#define BUFFER_SIZE 0x100

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];
    unsigned int id;
    unsigned int count;

    count = call_read(FILE_STDIN, 0, BUFFER_SIZE, buffer);

    id = call_open(FILE_NEW, "/module/tty/cwd");
    call_write(id, 0, count, buffer);

}

