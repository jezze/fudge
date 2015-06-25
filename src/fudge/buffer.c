#include "memory.h"
#include "buffer.h"

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned int i;
    char *m = memory;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (buffer->tail + 1) % buffer->count;

        if (buffer->head == buffer->tail)
            break;

        m[i] = buffer->memory[buffer->tail];
        buffer->tail = tail;

    }

    return i;

}

unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned int i;
    char *m = memory;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head + 1) % buffer->count;

        if (head == buffer->tail)
            break;

        buffer->memory[buffer->head] = m[i];
        buffer->head = head;

    }

    return i;

}

unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head - 1) % buffer->count;

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

