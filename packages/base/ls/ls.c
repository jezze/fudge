#include <fudge.h>

#define BUFFER_SIZE 0x1000

void main()
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (!call_open(3, 0, "/module/tty/cwd"))
        return;

    memory_clear(buffer, BUFFER_SIZE);

    count = call_read(3, 0, BUFFER_SIZE, buffer);
    call_close(3);

    if (!call_open(3, 0, buffer))
        return;
        
    count = call_read(3, 0, BUFFER_SIZE, buffer);
    call_close(3);

    call_write(FILE_STDOUT, 0, count, buffer);

}

