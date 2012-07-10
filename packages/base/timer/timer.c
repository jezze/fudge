#include <fudge.h>

#define BUFFER_SIZE 0x100

void main()
{

    char buffer[BUFFER_SIZE];
    unsigned int id;
    unsigned int count;

    id = call_open(FILE_NEW, "/module/pit/jiffies");

    if (!id)
        return;

    count = call_read(id, 0, BUFFER_SIZE, buffer);

    call_close(id);
    call_write(FILE_STDOUT, 0, count, buffer);

}

