#include <fudge.h>

#define BUFFER_SIZE 0x100

void main()
{

    char bufferIn[BUFFER_SIZE];
    char bufferCwd[BUFFER_SIZE];
    unsigned int countIn;
    unsigned int countCwd;

    countIn = call_read(FILE_STDIN, 0, BUFFER_SIZE, bufferIn);

    if (!countIn)
        return;

    call_open(3, 8, "/tty/cwd");

    if (memory_match(bufferIn, "/", 1))
    {

        if (!call_open(4, countIn, bufferIn))
            return;

        call_write(3, 0, countIn, bufferIn);

        return;

    }

    else
    {

        countCwd = call_read(3, 0, BUFFER_SIZE, bufferCwd);

        memory_copy(bufferCwd + countCwd, bufferIn, countIn);

        if (!call_open(4, countCwd + countIn, bufferCwd))
            return;

        call_write(3, 0, countCwd + countIn, bufferCwd);

    }

    call_close(3);

}

