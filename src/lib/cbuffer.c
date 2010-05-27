#include <system.h>
#include <string.h>
#include <cbuffer.h>

void cbuffer_write(char *buffer, uint32_t size, uint32_t head, uint32_t tail, char c)
{

    if ((tail + 1) % size != head)
    {

        buffer[tail] = c;
        tail = (tail + 1) % size;

    }

}

char cbuffer_read(char *buffer, uint32_t size, uint32_t head)
{

    char c = buffer[head];
    head = (head + 1) % size;

    return c;

}

