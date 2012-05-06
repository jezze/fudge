#include <fudge.h>

#define BUFFER_SIZE 0x100

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];

    unsigned int id = call_open("/module/rtc/timestamp");

    if (!id)
        return;

    unsigned int count = call_read(id, 0, BUFFER_SIZE, buffer);
    call_close(id);
    call_write(FILE_STDOUT, 0, count, buffer);

}

