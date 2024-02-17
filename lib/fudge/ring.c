#include "buffer.h"
#include "ring.h"

static unsigned int mask(struct ring *ring, unsigned int value)
{

    return value & (ring->capacity - 1);

}

unsigned int ring_count(struct ring *ring)
{

    return ring->head - ring->tail;

}

unsigned int ring_avail(struct ring *ring)
{

    return ring->capacity - ring_count(ring);

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

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        ring->tail++;

    }

    return c;

}

unsigned int ring_skip_reverse(struct ring *ring, unsigned int count)
{

    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        --ring->head;

    }

    return c;

}

unsigned int ring_read(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        b[c] = ring->buffer[mask(ring, ring->tail++)];

    }

    return c;

}

unsigned int ring_read_all(struct ring *ring, void *buffer, unsigned int count)
{

    if (count > ring_count(ring))
        return 0;

    return ring_read(ring, buffer, count);

}

unsigned int ring_read_reverse(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isempty(ring))
            break;

        b[c] = ring->buffer[mask(ring, --ring->head)];

    }

    return c;

}

unsigned int ring_read_allreverse(struct ring *ring, void *buffer, unsigned int count)
{

    if (count > ring_avail(ring))
        return 0;

    return ring_read_reverse(ring, buffer, count);

}

unsigned int ring_readcopy(struct ring *ring, void *buffer, unsigned int count)
{

    unsigned int head = ring->head;
    unsigned int tail = ring->tail;
    unsigned int total = ring_read(ring, buffer, count);

    ring->head = head;
    ring->tail = tail;

    return total;

}

unsigned int ring_write(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isfull(ring))
            break;

        ring->buffer[mask(ring, ring->head++)] = b[c];

    }

    return c;

}

unsigned int ring_write_all(struct ring *ring, void *buffer, unsigned int count)
{

    if (count > ring_avail(ring))
        return 0;

    return ring_write(ring, buffer, count);

}

unsigned int ring_write_reverse(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        if (ring_isfull(ring))
            break;

        ring->buffer[mask(ring, --ring->tail)] = b[c];

    }

    return c;

}

unsigned int ring_write_allreverse(struct ring *ring, void *buffer, unsigned int count)
{

    if (count > ring_avail(ring))
        return 0;

    return ring_write_reverse(ring, buffer, count);

}

unsigned int ring_overwrite(struct ring *ring, void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int c;

    if (!count)
        return 0;

    for (c = 0; count--; c++)
    {

        ring->buffer[mask(ring, ring->head++)] = b[c];

        if (ring_isfull(ring))
            ring->tail++;

    }

    return c;

}

unsigned int ring_move(struct ring *to, struct ring *from)
{

    char buffer[512];
    unsigned int count;
    unsigned int total = 0;

    while ((count = ring_read(from, buffer, 512)))
        total += ring_write(to, buffer, count);

    return total;

}

unsigned int ring_overmove(struct ring *to, struct ring *from)
{

    char buffer[512];
    unsigned int count;
    unsigned int total = 0;

    while ((count = ring_read(from, buffer, 512)))
        total += ring_overwrite(to, buffer, count);

    return total;

}

unsigned int ring_copy(struct ring *to, struct ring *from)
{

    unsigned int head = from->head;
    unsigned int tail = from->tail;
    unsigned int total = ring_move(to, from);

    from->head = head;
    from->tail = tail;

    return total;

}

unsigned int ring_overcopy(struct ring *to, struct ring *from)
{

    unsigned int head = from->head;
    unsigned int tail = from->tail;
    unsigned int total = ring_overmove(to, from);

    from->head = head;
    from->tail = tail;

    return total;

}

unsigned int ring_each(struct ring *ring, char value)
{

    unsigned int s = ring->tail;
    unsigned int c;

    for (c = 0; s != ring->head; c++)
    {

        if (ring->buffer[mask(ring, s)] == value)
            return c + 1;

        s++;

    }

    return 0;

}

unsigned int ring_find(struct ring *ring, char value)
{

    unsigned int s = ring->tail;
    unsigned int c;

    for (c = 0; s != ring->head; c++)
    {

        if (ring->buffer[mask(ring, s++)] == value)
            return c;

    }

    return s == ring->head ? c : 0;

}

unsigned int ring_find_reverse(struct ring *ring, char value)
{

    unsigned int s = ring->head;
    unsigned int c;

    for (c = 0; s != ring->tail; c++)
    {

        if (ring->buffer[mask(ring, --s)] == value)
            return c;

    }

    return s == ring->tail ? c : 0;

}

void ring_reset(struct ring *ring)
{

    ring->head = 0;
    ring->tail = 0;

}

void ring_init(struct ring *ring, unsigned int capacity, void *buffer)
{

    ring->buffer = buffer;
    ring->capacity = capacity;
    ring->head = 0;
    ring->tail = 0;

}

