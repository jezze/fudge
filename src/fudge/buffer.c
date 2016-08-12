#include "memory.h"
#include "buffer.h"

static void inchead(struct buffer *buffer)
{

    buffer->head++;
    buffer->count++;

    if (buffer->head == buffer->memory + buffer->capacity)
        buffer->head = buffer->memory;

}

static void inctail(struct buffer *buffer)
{

    buffer->tail++;
    buffer->count--;

    if (buffer->tail == buffer->memory + buffer->capacity)
        buffer->tail = buffer->memory;

}

static void dechead(struct buffer *buffer)
{

    if (buffer->head == buffer->memory)
        buffer->head = buffer->memory + buffer->capacity;

    buffer->head--;
    buffer->count--;

}

unsigned int buffer_skip(struct buffer *buffer, unsigned int count)
{

    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (buffer->count == 0)
            break;

        inctail(buffer);

    }

    return c;

}

unsigned int buffer_erase(struct buffer *buffer, unsigned int count)
{

    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (buffer->count == 0)
            break;

        dechead(buffer);

    }

    return c;

}

unsigned int buffer_read(struct buffer *buffer, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (buffer->count == 0)
            break;

        m[c] = *buffer->tail;

        inctail(buffer);

    }

    return c;

}

unsigned int buffer_write(struct buffer *buffer, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (buffer->count == buffer->capacity)
            break;

        *buffer->head = m[c];

        inchead(buffer);

    }

    return c;

}

unsigned int buffer_overwrite(struct buffer *buffer, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (buffer->count == buffer->capacity)
            inctail(buffer);

        *buffer->head = m[c];

        inchead(buffer);

    }

    return c;

}

unsigned int buffer_copy(struct buffer *buffer, void *memory, unsigned int count)
{

    char *tail = buffer->tail;
    unsigned int c = buffer->count;

    count = buffer_read(buffer, memory, count);
    buffer->tail = tail;
    buffer->count = c;

    return count;

}

void buffer_init(struct buffer *buffer, unsigned int capacity, void *memory)
{

    buffer->capacity = capacity;
    buffer->count = 0;
    buffer->memory = memory;
    buffer->head = memory;
    buffer->tail = memory;

}

