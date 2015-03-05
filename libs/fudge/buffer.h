struct buffer
{

    unsigned int head;
    unsigned int tail;
    unsigned int step;
    unsigned int size;
    unsigned char *memory;

};

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int count);
void buffer_init(struct buffer *buffer, unsigned int step, unsigned int size, void *memory);
