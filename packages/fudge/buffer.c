#include "memory.h"
#include "buffer.h"

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int size, unsigned int count, void *memory)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (buffer->tail + size) % buffer->count;

        if (buffer->head == buffer->tail)
            break;

        memory_write(memory, count, buffer->memory + buffer->tail, 1, size, i);

        buffer->tail = tail;

    }

    return i;

}

unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int size, unsigned int count, void *memory)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head + size) % buffer->count;

        if (head == buffer->tail)
            break;

        memory_read(buffer->memory + buffer->head, 1, memory, count, size, i);

        buffer->head = head;

    }

    return i;

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

    return i;

}

void buffer_init(struct buffer *buffer, unsigned int count, void *memory)
{

    memory_clear(buffer, sizeof (struct buffer));

    buffer->count = count;
    buffer->memory = memory;

}

