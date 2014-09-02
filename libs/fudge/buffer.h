struct buffer_cfifo
{

    unsigned char *buffer;
    unsigned int head;
    unsigned int tail;
    unsigned int size;

};

unsigned int buffer_rcfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer);
unsigned int buffer_wcfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer);
void buffer_initcfifo(struct buffer_cfifo *cfifo, unsigned int size, void *buffer);
