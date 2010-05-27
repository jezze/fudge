#include <system.h>
#include <string.h>
 
void cbuffer_write(char *buffer, uint32_t size, uint32_t start, uint32_t end, char c)
{

    if ((end + 1) % size != start)
    {

        buffer[end] = c;
        end = (end + 1) % size;

    }

}

char cbuffer_read(char *buffer, uint32_t size, uint32_t start, uint32_t end)
{

    char c = buffer[start];
    start = (start + 1) % size;

    return c;

}

