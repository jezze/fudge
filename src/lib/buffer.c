#include <system.h>
#include <buffer.h>

void buffer_clear(char *buffer, uint16_t current, uint16_t size)
{

    current = 0;

}

void buffer_write(char *buffer, uint16_t current, uint16_t size, char c)
{

    if (current < size)
    {

        *buffer[current] = c;
        current++;

    }

}

char buffer_read(char *buffer, uint16_t current, uint16_t size)
{

    if (current)
    {

        current--;
        return *buffer[current];

    }

    return -1;

}

