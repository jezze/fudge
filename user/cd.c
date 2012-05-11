#include <fudge.h>

#define BUFFER_SIZE 0x100

void main()
{

    char bufferIn[BUFFER_SIZE];
    char bufferCwd[BUFFER_SIZE];
    unsigned int countIn;
    unsigned int countCwd;
    unsigned int id;

    countIn = call_read(FILE_STDIN, 0, BUFFER_SIZE, bufferIn);

    if (!countIn)
        return;

    id = call_open(FILE_NEW, "/module/tty/cwd");

    if (memory_compare(bufferIn, "/", 1))
    {

        call_write(id, 0, countIn, bufferIn);

        return;

    }

    countCwd = call_read(id, 0, BUFFER_SIZE, bufferCwd);

    memory_copy(bufferCwd + countCwd, bufferIn, countIn);

    call_write(id, 0, countCwd + countIn, bufferCwd); 

}

