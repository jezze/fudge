struct buffer
{

    unsigned int size;
    unsigned int capacity;
    unsigned int count;
    unsigned char *memory;
    unsigned char *head;
    unsigned char *tail;

};

unsigned int buffer_ecfifo(struct buffer *buffer, unsigned int count);
unsigned int buffer_rcfifo(struct buffer *buffer, unsigned int count, void *memory);
unsigned int buffer_wcfifo(struct buffer *buffer, unsigned int count, void *memory);
void buffer_init(struct buffer *buffer, unsigned int size, unsigned int count, void *memory);
