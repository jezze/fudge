#include <lib/cbuffer.h>

void cbuffer_init(struct cbuffer *cbuffer, char *buffer, unsigned int size)
{

    cbuffer->buffer = buffer;
    cbuffer->size = size;
    cbuffer->head = 0;
    cbuffer->tail = 0;

}

void cbuffer_write(struct cbuffer *cbuffer, char c)
{

    if ((cbuffer->head + 1) % cbuffer->size != cbuffer->tail)
    {

        cbuffer->buffer[cbuffer->head] = c;
        cbuffer->head = (cbuffer->head + 1) % cbuffer->size;

    }

}

char cbuffer_read(struct cbuffer *cbuffer)
{

    if (cbuffer->head != cbuffer->tail)
    {

        char c = cbuffer->buffer[cbuffer->tail];
        cbuffer->tail = (cbuffer->tail + 1) % cbuffer->size;

        return c;

    }

    return 0;

}

