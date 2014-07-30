#include "memory.h"
#include "buffer.h"

unsigned int buffer_read_cfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (cfifo->tail + 1) % cfifo->size;

        if (cfifo->head == cfifo->tail)
            break;

        b[i] = cfifo->buffer[cfifo->tail];
        cfifo->tail = tail;

    }

    return i;

}

unsigned int buffer_write_cfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (cfifo->head + 1) % cfifo->size;

        if (head == cfifo->tail)
            break;

        cfifo->buffer[cfifo->head] = b[i];
        cfifo->head = head;

    }

    return i;

}

void buffer_init_cfifo(struct buffer_cfifo *cfifo, unsigned int size, void *buffer)
{

    memory_clear(cfifo, sizeof (struct buffer_cfifo));

    cfifo->size = size;
    cfifo->buffer = buffer;

}

