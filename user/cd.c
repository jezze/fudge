#include <fudge.h>

#define BUFFER_SIZE 0x100

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];

    unsigned int id = call_open("/module/tty/cwd");

    if (argc == 1)
    {

        unsigned int count = call_read(id, 0, BUFFER_SIZE, buffer);
        call_write(FILE_STDOUT, 0, count, buffer);
        call_write(FILE_STDOUT, 0, 1, "\n");

    }

    if (argc == 2)
    {

        call_write(id, 0, string_length(argv[1]), argv[1]);


    }

    call_close(id);

}

