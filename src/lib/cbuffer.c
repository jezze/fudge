#include <system.h>
#include <string.h>
#include <cbuffer.h>

cbuffer_t cbuffer_create(char *buffer, uint32_t size)
{

    cbuffer_t cbuffer;
    cbuffer.buffer = buffer;
    cbuffer.size = size;
    cbuffer.head = 0;
    cbuffer.tail = 0;

    return cbuffer;

}

void cbuffer_write(cbuffer_t *cbuffer, char c)
{

/*
    if ((cbuffer->tail + 1) % cbuffer->size != cbuffer->head)
    {

        cbuffer->buffer[cbuffer->tail] = c;
        cbuffer->tail = (cbuffer->tail + 1) % cbuffer->size;

    }
*/

    cbuffer->buffer[cbuffer->head] = c;
    cbuffer->head++;

}

char cbuffer_read(cbuffer_t *cbuffer)
{

/*
    char c = cbuffer->buffer[cbuffer->head];
    cbuffer->head = (cbuffer->head + 1) % cbuffer->size;

    return c;
*/

    if (cbuffer->head > 0)
    {

        char c = cbuffer->buffer[cbuffer->head - 1];
        cbuffer->head--;

        return c;

    }

    return 0;

}

uint8_t cbuffer_full(cbuffer_t *cbuffer)
{

    return cbuffer->head != cbuffer->tail;

}
