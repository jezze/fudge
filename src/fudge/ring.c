#include "ring.h"

static unsigned int mask(struct ring *ring, unsigned int value)
{

    return value & (ring->capacity - 1);

}

unsigned int ring_count(struct ring *ring)
{

    return ring->head - ring->tail;

}

unsigned int ring_isempty(struct ring *ring)
{

    return ring->head == ring->tail;

}

unsigned int ring_isfull(struct ring *ring)
{

    return ring_count(ring) == ring->capacity;

}

unsigned int ring_skip(struct ring *ring, unsigned int count)
{

    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        ring->tail++;

    }

    return c;

}

unsigned int ring_erase(struct ring *ring, unsigned int count)
{

    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        ring->head--;

    }

    return c;

}

unsigned int ring_read(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        b[c] = ring->buffer[mask(ring, ring->tail++)];

    }

    return c;

}

unsigned int ring_write(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isfull(ring))
            break;

        ring->buffer[mask(ring, ring->head++)] = b[c];

    }

    return c;

}

unsigned int ring_overwrite(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        ring->buffer[mask(ring, ring->head++)] = b[c];

        if (ring_isfull(ring))
            ring->tail++;

    }

    return c;

}

void ring_init(struct ring *ring, unsigned int capacity, void *buffer)
{

    ring->buffer = buffer;
    ring->capacity = capacity;
    ring->head = 0;
    ring->tail = 0;

}

