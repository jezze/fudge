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

unsigned int ring_read(struct ring *ring, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        m[c] = ring->memory[mask(ring, ring->tail++)];

    }

    return c;

}

unsigned int ring_write(struct ring *ring, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        if (ring_isfull(ring))
            break;

        ring->memory[mask(ring, ring->head++)] = m[c];

    }

    return c;

}

unsigned int ring_overwrite(struct ring *ring, void *memory, unsigned int count)
{

    char *m = memory;
    unsigned int c;

    for (c = 0; count--; c++)
    {

        ring->memory[mask(ring, ring->head++)] = m[c];

        if (ring_isfull(ring))
            ring->tail++;

    }

    return c;

}

void ring_init(struct ring *ring, unsigned int capacity, void *memory)
{

    ring->memory = memory;
    ring->capacity = capacity;
    ring->head = 0;
    ring->tail = 0;

}

