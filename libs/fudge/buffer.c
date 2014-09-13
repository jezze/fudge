#include "memory.h"
#include "buffer.h"

unsigned int buffer_pushlifo(struct buffer *buffer, unsigned int count, void *memory)
{

    if (buffer->head + count < buffer->size)
    {

        memory_copy(buffer->memory + buffer->head, memory, count);

        buffer->head += count;

        return count;

    }

    return 0;

}

unsigned int buffer_poplifo(struct buffer *buffer, unsigned int count)
{

    if (buffer->head >= count)
    {

        buffer->head -= count;

        return count;

    }

    return 0;

}

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned char *b = memory;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (buffer->tail + buffer->step) % buffer->size;

        if (buffer->head == buffer->tail)
            break;

        b[i] = buffer->memory[buffer->tail];
        buffer->tail = tail;

    }

    return i;

}

unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned char *b = memory;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (buffer->head + buffer->step) % buffer->size;

        if (head == buffer->tail)
            break;

        buffer->memory[buffer->head] = b[i];
        buffer->head = head;

    }

    return i;

}

void buffer_clear(struct buffer *buffer)
{

    buffer->head = 0;
    buffer->tail = 0;

}

void buffer_init(struct buffer *buffer, unsigned int step, unsigned int size, void *memory)
{

    memory_clear(buffer, sizeof (struct buffer));

    buffer->step = step;
    buffer->size = size;
    buffer->memory = memory;

}

