#include "memory.h"
#include "buffer.h"

static unsigned int e(struct buffer *buffer)
{

    if (buffer->count == 0)
        return 0;

    if (buffer->head == buffer->memory)
        buffer->head = buffer->memory + buffer->capacity;
    else
        buffer->head -= 1;

    buffer->count--;

    return 1;

}

static unsigned int r(struct buffer *buffer, void *item)
{

    if (buffer->count == 0)
        return 0;

    memory_copy(item, buffer->tail, 1);

    buffer->tail++;

    if (buffer->tail == buffer->memory + buffer->capacity)
        buffer->tail = buffer->memory;

    buffer->count--;

    return 1;

}

static unsigned int w(struct buffer *buffer, void *item)
{

    if (buffer->count == buffer->capacity)
        return 0;

    memory_copy(buffer->head, item, 1);

    buffer->head++;

    if (buffer->head == buffer->memory + buffer->capacity)
        buffer->head = buffer->memory;

    buffer->count++;

    return 1;

}

unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int count)
{

    unsigned int i;
    unsigned int c = 0;

    for (i = 0; i < count; i++)
    {

        c += e(buffer);

    }

    return c;

}

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned int i;
    unsigned int c = 0;
    unsigned char *m = memory;

    for (i = 0; i < count; i++)
    {

        c += r(buffer, m);
        m++;

    }

    return c;

}

unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory)
{

    unsigned int i;
    unsigned int c = 0;
    unsigned char *m = memory;

    for (i = 0; i < count; i++)
    {

        c += w(buffer, m);
        m++;

    }

    return c;

}

void buffer_init(struct buffer *buffer, unsigned int capacity, void *memory)
{

    buffer->capacity = capacity;
    buffer->count = 0;
    buffer->memory = memory;
    buffer->head = memory;
    buffer->tail = memory;

}

