#include "../define.h"
#include "circular.h"

unsigned int circular_stream_read(struct circular_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (stream->tail + 1) % FUDGE_BSIZE;

        if (stream->head == stream->tail)
            break;

        b[i] = stream->buffer[stream->tail];
        stream->tail = tail;

    }

    return i;

}

unsigned int circular_stream_write(struct circular_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (stream->head + 1) % FUDGE_BSIZE;

        if (head == stream->tail)
            break;

        stream->buffer[stream->head] = b[i];
        stream->head = head;

    }

    return i;

}

void circular_stream_clear(struct circular_stream *stream)
{

    stream->head = 0;
    stream->tail = 0;

}

unsigned int circular_stream_isempty(struct circular_stream *stream)
{

    return stream->head == stream->tail;

}

