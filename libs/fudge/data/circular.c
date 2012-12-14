#include "../define.h"
#include "circular.h"

unsigned int circular_stream_read(struct circular_stream *stream, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (stream->tail + 1) % FUDGE_BSIZE;

        if (stream->head == stream->tail)
            break;

        buffer[i] = stream->buffer[stream->tail];
        stream->tail = tail;

    }

    return i;

}

unsigned int circular_stream_write(struct circular_stream *stream, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (stream->head + 1) % FUDGE_BSIZE;

        if (head == stream->tail)
            break;

        stream->buffer[stream->head] = buffer[i];
        stream->head = head;

    }

    return i;

}

