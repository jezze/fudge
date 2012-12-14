struct circular_stream
{

    char buffer[FUDGE_BSIZE];
    unsigned int head;
    unsigned int tail;

};

unsigned int circular_stream_read(struct circular_stream *stream, unsigned int count, char *buffer);
unsigned int circular_stream_write(struct circular_stream *stream, unsigned int count, char *buffer);
