struct circular_stream
{

    char buffer[FUDGE_BSIZE];
    unsigned int head;
    unsigned int tail;

};

unsigned int circular_stream_read(struct circular_stream *stream, unsigned int count, void *buffer);
unsigned int circular_stream_write(struct circular_stream *stream, unsigned int count, void *buffer);
void circular_stream_clear(struct circular_stream *stream);
unsigned int circular_stream_isempty(struct circular_stream *stream);
