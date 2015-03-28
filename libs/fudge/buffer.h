struct buffer
{

    unsigned int head;
    unsigned int tail;
    unsigned int size;
    unsigned int count;
    unsigned char *memory;

};

unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int count);
void buffer_init(struct buffer *buffer, unsigned int size, unsigned int count, void *memory);
