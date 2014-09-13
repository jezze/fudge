struct buffer
{

    unsigned int head;
    unsigned int tail;
    unsigned int size;
    unsigned char *memory;

};

unsigned int buffer_pushlifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_poplifo(struct buffer *buffer, unsigned int count);
unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory);
void buffer_clear(struct buffer *buffer);
void buffer_init(struct buffer *buffer, unsigned int size, void *memory);
