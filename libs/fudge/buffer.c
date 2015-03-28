#include "memory.h"
#include "buffer.h"

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int size, unsigned int count, void *memory)
{

    unsigned char *b = memory;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (buffer->tail + size) % buffer->count;

        if (buffer->head == buffer->tail)
            break;

        memory_copy(b + i * size, buffer->memory + buffer->tail * size, size);

        buffer->tail = tail;

    }

    return i * size;

}

unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int size, unsigned int count, void *memory)
{

    unsigned char *b = memory;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head + size) % buffer->count;

        if (head == buffer->tail)
            break;

        memory_copy(buffer->memory + buffer->head * size, b + i * size, size);

        buffer->head = head;

    }

    return i * size;

}

unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int size, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head - size) % buffer->count;

        if (buffer->head == buffer->tail)
            break;

        buffer->head = head;

    }

    return i * size;

}

void buffer_init(struct buffer *buffer, unsigned int count, void *memory)
{

    memory_clear(buffer, sizeof (struct buffer));

    buffer->count = count;
    buffer->memory = memory;

}

