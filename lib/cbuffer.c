#include <lib/types.h>
#include <lib/cbuffer.h>

cbuffer_t cbuffer_create(char *buffer, unsigned int size)
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

    if ((cbuffer->head + 1) % cbuffer->size != cbuffer->tail)
    {

        cbuffer->buffer[cbuffer->head] = c;
        cbuffer->head = (cbuffer->head + 1) % cbuffer->size;

    }

}

char cbuffer_read(cbuffer_t *cbuffer)
{

    if (cbuffer->head != cbuffer->tail)
    {

        char c = cbuffer->buffer[cbuffer->tail];
        cbuffer->tail = (cbuffer->tail + 1) % cbuffer->size;

        return c;

    }

    return 0;

}

