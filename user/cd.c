#include <fudge.h>

#define BUFFER_SIZE 0x100

void main()
{

    char bufferIn[BUFFER_SIZE];
    char bufferCwd[BUFFER_SIZE];
    unsigned int id;
    unsigned int count;

    count = call_read(FILE_STDIN, 0, BUFFER_SIZE, bufferIn);

    if (!count)
        return;

    id = call_open(FILE_NEW, "/module/tty/cwd");

    if (memory_compare(bufferIn, "/", 1))
    {

        call_write(id, 0, count, bufferIn);

    }

    else
    {

        unsigned int c = call_read(id, 0, BUFFER_SIZE, bufferCwd);

        memory_copy(bufferCwd + c, bufferIn, count);

        call_write(id, 0, c + count, bufferCwd); 

    }

}

