#include <fudge.h>

#define BUFFER_SIZE 0x1000

void main()
{

    char buffer[BUFFER_SIZE];
    unsigned int id;
    unsigned int count;

    id = call_open(FILE_NEW, "/module/tty/cwd");
    count = call_read(id, 0, BUFFER_SIZE, buffer);
    call_close(id);

    id = call_open(FILE_NEW, buffer);
    count = call_read(id, 0, BUFFER_SIZE, buffer);
    call_close(id);

    call_write(FILE_STDOUT, 0, count, buffer);

}

